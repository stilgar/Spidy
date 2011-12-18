#include "../spidy.h"
#include "../hw.h"
#include "../utils.h"
#include "../io.h"
#include "../gpio.h"

#define COM_LEN 15

int motors[18];
int saveIndex;		/* from 0 to 17, index of motors */
int frameIndex;		/* from 0 to 19, index of frames */
int portNow;            /* GPIO number of this frame */
int portLast;           /* GPIO number of last frame */
char tmp[80];		/* debug string */
int commands[COM_LEN] = {0,0,0,0,0,
			 0,0,0,0,0,
			 0,0,0,0,0};
int waitTime = 0;
static uint8_t used_gpio[18] = {
        GPIO_NR(1,9),   GPIO_NR(1,8),   GPIO_NR(1,10),
        GPIO_NR(1,11),  GPIO_NR(2,0),   GPIO_NR(2,1),
        GPIO_NR(2,2),   GPIO_NR(2,3),   GPIO_NR(2,4),
        GPIO_NR(2,5),   GPIO_NR(2,6),   GPIO_NR(2,7),
        GPIO_NR(2,8),   GPIO_NR(2,9),   GPIO_NR(2,10),
        GPIO_NR(2,11),  GPIO_NR(3,0),   GPIO_NR(3,1),
};

/* Handshake */
int spidy_main(void *unused)
{
	int found=0, i=0;
	saveIndex=0;

        /* Gpio initialization */
        gpio_init();

	puts("Your friendly neighborhood Spider-Man\n");

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
	puts("# FRAME ");
	itoa(frameIndex, tmp);puts(tmp);
	putc('\n');
	puts("PN,PL (");
	itoa(portNow, tmp);puts(tmp);
	putc(',');
	itoa(portLast, tmp);puts(tmp);
	puts(")\n");

	/* up motor i - frameIndex != 9 && != 19 */
	if (portNow >= 0) {
		puts("UP ");
		itoa(portNow, tmp);puts(tmp);
		putc('\n');
	}

	/* finishing i-1 if it is above 52 deg - frameIndex != 0 && != 10 */
	if (portLast >= 0) {
		if (motors[portLast] > 52) {
			waitTime = TIMEDEG * (motors[portLast] - 52);
			puts("WAIT 1 ");
			itoa(portLast, tmp);puts(tmp);
			puts(" - ");
			itoa(motors[portLast], tmp);puts(tmp);
			puts(" deg = ");
			itoa(waitTime, tmp);puts(tmp);
			putc('\n');
			puts("DOWN ");
			itoa(portLast, tmp);puts(tmp);
			putc('\n');
		}
	} else {
		puts("GET BUFFER\n");
	}

	/* finishing i if it below or equal 52 deg - frameIndex != 9 && != 19 */
	if (portNow >= 0) {
		if (motors[portNow] <= 52) {
			waitTime = (TIMEDEG * (motors[portNow])) + TIME090;
			puts("WAIT 2 ");
			itoa(portNow, tmp);puts(tmp);
			puts(" - ");
			itoa(motors[portNow], tmp);puts(tmp);
			puts(" deg = ");
			itoa(waitTime, tmp);puts(tmp);
			putc('\n');
			puts("DOWN ");
			itoa(portNow, tmp);puts(tmp);
			putc('\n');
		}
	}
	/* recognize next motor */
	setPort();

	putc(CHAR_TEST_MEND);
	while ( getCommand() != CHAR_TEST_SEND)
		;

}

/* sets Port and Bit for this frame */
void setPort() {
	/* next index */
	frameIndex++;
	frameIndex%=20;			/* frameIndex from 0 to 19 */
	if (frameIndex < 10)
		portNow=frameIndex;
	else
		portNow=frameIndex-1;
	portLast=portNow-1;
	if (frameIndex == 9 || frameIndex == 19)
		portNow=-1;
	else if (frameIndex == 10)
		portLast=-1;
}

/* It's really rigid structure. I get 9 characters surrounded by two sync
 * characters (CHAR_MFRAME). So it will be 11 characters and 12th must be
 * a CHAR_ENDLINE.
 * If this is not the structure I got, this will return 0, otherwise 1.
 * This is so rigid, because I need to be speedy in this structure.
 */
int getBuffer()
{
        int state=0,f=0;
        char tmp=0x0;
        commands[0]=0;

        /* gets each command between frame chars, otherwise return 0 */
        while (f<COM_LEN && state<2) {
                tmp = getCommand();
                if (tmp == CHAR_SFRAME) {
                        state++;
                } else if (state == 1) {
			if (tmp != 0) { /* pause between FRAME and BUFFER */
				if (DEBUG_LEVEL > 0)
					putc(CHAR_CBUF);
				putc(f);
				putc(tmp);
                	        commands[f++]=tmp;
			}
                } else {
			if (DEBUG_LEVEL > 2)
				putc(CHAR_CLOOP);
                        return 0;
                }
        }

        /* frame may be below 9 characters lenght, so it puts 0 at the end */
        if (state==2) {
                while (f<COM_LEN)
                        commands[f++]=0;
		if (DEBUG_LEVEL > 1)
			putc(CHAR_CREAD);
                return 1;
        } else {        /* otherwise another error */
		if (DEBUG_LEVEL > 2)
			putc(CHAR_CLOOP);
                return 0;
        }
}

/* Saves buffer of 9 commands received at most. Returns saved values amount. */
int saveBuffer () {
	int f=0;
	while (f<9 && commands[f]!=0) {
		motors[saveIndex++]=commands[f++]-DEG_0;
		saveIndex %= 18;
	}
	return f;
}

