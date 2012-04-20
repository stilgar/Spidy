#include "micro.h"
#include "hw.h"
#include "link.h"
#include "spidy.h"

#define BUF_LEN 15	/* max number of char on RBR */

#define DEBUG_LEVEL 3	/* 0 = control characters
			 * 1 = only CHAR_CBUF (reading buffer)
			 * 2 = + CHAR_CREAD (correct reading)
			 * 3 = + CHAR_CLOOP (no buffer read)
			 */

int frameIndex;		/* from 0 to 19, index of frames */
int portNow;		/* GPIO number of this frame */
int portLast;		/* GPIO number of last frame */
char tmp[80];		/* debug string */

int motors[18];		/* degrees copied from commands */
int commands[18] = {-1,-1,-1,-1,-1,-1,
		    -1,-1,-1,-1,-1,-1,
		    -1,-1,-1,-1,-1,-1}; /* degrees from sender */

int waitTime = 0;
static uint8_t used_gpio[18] = {
	GPIO_NR(3,1),	GPIO_NR(3,2),	GPIO_NR(3,3),	/* 00-01-02 */
	GPIO_NR(2,6),	GPIO_NR(2,7),	GPIO_NR(3,0),	/* 10-11-12 */
	GPIO_NR(2,5),	GPIO_NR(2,4),	GPIO_NR(2,3),	/* 20-21-22 */
	GPIO_NR(2,2),	GPIO_NR(2,1),	GPIO_NR(2,0),	/* 30-31-32 */
	GPIO_NR(1,3),	GPIO_NR(1,4),	GPIO_NR(1,5),	/* 40-41-42 */
	GPIO_NR(1,2),	GPIO_NR(1,1),	GPIO_NR(1,0),	/* 50-51-52 */
};

/* Handshake */
int spidy_main(void *unused)
{
	int found=0, i=0;

	/* Gpio initialization */
	gpio_init();

        /* MOD GPIO DIR AF */
	for (i=0; i<18; i++)
	        gpio_dir_af(used_gpio[i], 1, 0, 0);
	i=0;

	puts("Your friendly neighborhood Spider-Man\n");

	/* FIXME: spidy_reset(); */

	/* Handshake */
	while (!found) {
		putc(CHAR_MSTART);

		/* read from RBR start character */
		if ( getCommand() == CHAR_SSTART)
			found=1;
	}
	putc(CHAR_MREADY);

	/* Take at least 18 commands --> BLOCKING */
	while (i<18) {
		/* ready to get buffer */
		while (!getBuffer())
			;

		/* if a buffer has been caught send a ready */
		i += saveBuffer();
		putc(CHAR_MREADY);
	}

	frameIndex=0;
	portNow=0;
	portLast=-1;

	return 0;
}

/* In boot.S there is a loop of spidy_step */
void spidy_step(void *unused)
{
	/* Wait till next beginning */
	while (regs[REG_TMR32B1PC] >= 30)
		;

	/* up motor i, frameIndex != 9 && != 19 */
	if (portNow >= 0)
		gpio_dir(used_gpio[portNow], 1, 1);

	/* down motor i-1 if it's > 52 deg, frameIndex != 0 && != 10 */
	if (portLast >= 0) {
		if (motors[portLast] > DEGREELIMIT) {
			waitTime = TIMEDEG * (motors[portLast] - DEGREELIMIT);
			while (regs[REG_TMR32B1PC] < waitTime)
				;
			gpio_dir(used_gpio[portLast], 1, 0);
		}
	} else {
		/* acquiring buffer */
		if (getBuffer()) {
			saveBuffer();
			putc(CHAR_MREADY);	/* to get next burst */
		}
	}

	/* down motor i if it's <= 52 deg, frameIndex != 9 && != 19 */
	if (portNow >= 0) {
		if (motors[portNow] <= DEGREELIMIT) {
			waitTime = (TIMEDEG * (motors[portNow])) + TIME090;
			while(regs[REG_TMR32B1PC] < waitTime)
				;
			gpio_dir(used_gpio[portNow], 1, 0);
		}
	}
	/* setup next motor */
	setPort();
}

/* sets Port for this frame */
void setPort() {
	/* next index */
	frameIndex++;
	frameIndex%=20;
	if (frameIndex < 10)
		portNow=frameIndex;
	else
		portNow = frameIndex-1;
	portLast=portNow-1;
	if (frameIndex == 9 || frameIndex == 19)
		portNow=-1;
	else if (frameIndex == 10)
		portLast=-1;
}

/* Gets couples of characters interleaved by a space (ENGINE_NUM + ENGINE_DEG)
 * All these couples must be surrounded by 2 sync characters (CHAR_SFRAME and
 * CHAR_SFRAME_END)
 * ENGINE_NUM is valid if in [CHAR_ENGINE_FIRST, CHAR_ENGINE_LAST]
 * ENGINE_DEG is valid if in [DEG_NEG90, DEG_POS90]
 *
 * If it's all right a sync character (CHAR_CREADY) is sent to RBR to go on
 * with next burst and 1 is returned.
 *
 * On error CHAR_CLOOP is sent to RBR and 0 is return.
 */
int getBuffer()
{
	int state=0, f=0, engine=0, engine_n=0, empty=1;
	char tmp=0x0;

	/* gets each couple between CHAR_SFRAME */
	while (f<BUF_LEN && state<2) {
		tmp = getCommand();
		if (tmp==CHAR_SFRAME) {
			/* clean commands buffer */
			while (f<18)
				commands[f++] = -1;
			f=0;
			state=1;
		} else if (tmp==CHAR_SFRAME_END) {
			state=2;
		} else if (tmp == CHAR_SEND) {
			putc(CHAR_MEND);
			spidy_reset();
		} else if (state==1) {
			if (tmp != 0) {
				if (DEBUG_LEVEL > 0)
					putc(CHAR_CBUF);
				if (tmp>=CHAR_ENGINE_FIRST
						&& tmp<=CHAR_ENGINE_LAST) {
					engine = 1;
					engine_n = tmp - CHAR_ENGINE_FIRST;
				} else if (engine && tmp >= DEG_NEG90 &&
							     tmp <= DEG_POS90) {
					engine = 0;
					commands[engine_n] = tmp;
					empty=0;
				}
				f++;
			}
		/* send received only outside CHAR_SFRAME */
		} else {
			if (DEBUG_LEVEL > 2)
				putc(CHAR_CLOOP);
			return 0;
		}
	}

	/* there must be at least one complete couple between CHAR_SFRAME */
	if (state==2 && !empty) {
		if (DEBUG_LEVEL > 1)
			putc(CHAR_CREAD);
		return 1;
	} else {	/* otherwise another error */
		if (DEBUG_LEVEL > 2)
			putc(CHAR_CLOOP);
		return 0;
	}
}

/*
 * Copy all the degree values >=0 of the engines read from the RBR
 * Returns saved values amount.
 */
int saveBuffer () {
	int f=0, i=0;

	while (f<18) {
		if (commands[f]>=0) {
			motors[f]=commands[f] - DEG_NEG90;
			i++;
		}
		f++;
	}
	return i;
}

/* FIXME: It has to be a boot.S shortcut that reinitialize everything */
void spidy_reset(void) {
	/* Initialize buffer */
	int i;
	for (i=0; i<18; i++)
		commands[i]=0;
}
