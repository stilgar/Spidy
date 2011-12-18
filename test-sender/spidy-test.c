#include "../spidy.h"
#include "../hw.h"
#include "../utils.h"
#include "../io.h"
#include "../gpio.h"

#define COMMANDS_LEN 20

int commands[COMMANDS_LEN]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void sleep(int n)
{
	int i, now;
	for(i=0; i<n; i++){
		/* wait just a period (to do not create so much traffic) */
		now=jiffies;
		while (now==jiffies)
			;
	}
}

/* Handshake */
int spidy_main(void *unused)
{
	int found = 0;

	puts("Your friendly neighborhood Spider-Man\n");

	while (!found) {
		putc(CHAR_MSTART);

		/* read from RBR start character */
		if (getCommand() == CHAR_SSTART)
			found = 1;

		sleep(1);
	}

	putc(CHAR_MREADY);

	return 0;
}

/* In boot.S there is a loop of spidy_step (never ending story) */
void spidy_step(void *unused)
{
	int i;

	if (getCommand() == CHAR_SREADY) {

		for(i=0; i<COMMANDS_LEN; i++)
			putc(commands[i]);

		/* ready to get buffer */
		while (!getBuffer())
			;

		for(i=0; i<COMMANDS_LEN; i++)
			putc(commands[i]);

		putc(CHAR_MREADY);
	}
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
	int tmp=0x0;
	commands[0]=0;

	/* gets each command between frame chars, otherwise return 0 */
	while (f<COMMANDS_LEN && state<2) {
		tmp = getCommand();
		if (tmp == CHAR_SFRAME) {
			state++;
		} else if (state==1) {
			commands[f++]=tmp;
		} else {
			return 0;
		}
	}

	/* frame may be below 9 characters lenght, so it puts 0 at the end */
	if (state==2) {
		while (f<COMMANDS_LEN)
			commands[f++]=0;
		return 1;
	} else {	/* otherwise another error */
		return 0;
	}
}
