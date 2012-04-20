#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "master.h"
#include "pc.h"
#include "spidy.h"

int menu(void);
int closing(int return_request);

/*
 * Master Controller is a program that uses sender
 * to build frames of a walk pattern. It allows also
 * to calibrate each engine of Spidy.
 */
int main (int argc, char **argv)
{
	int i;
	char choice[80];
	/* It must be first action, to avoid segmentation fault */
	load_file();
	for (i=0; i<18; i++)
		leg_negatives[i] = pow(-1, (int)(i/3));

	while(1) {
		choice[0] = '\0';
		switch(menu()) {
			case 1:		/* Calibrate engines */
			calibrate();
			break;

			case 2:		/* Test engines */
			test();
			break;

			case 3:		/* Calc values to walk */
			walk();
			break;

			case 4:		/* Quit */
			return closing(0);
			break;
		}
		printf("\nExit? [y/n] ");
		scanf("%s%*[^\n]", &choice[0]);
		if (choice[0] == 'y' || choice[0] == 'Y')
			return closing(0);
	}
	return closing(0);
}

/*
 * Send to each engine a middle value.
 */
void init_engines(void)
{
        int i;
        for(i=0; i<18; i++)
                send_signal(num2eng(i), zero_values[i]);
}

/*
 * Ask user what he wants to do, returns number of the question
 */
int menu(void)
{
	char choice[80];
	int choice_value = 0;
	while(choice_value < 1 || choice_value > 4) {
		system("clear");
		printf("\t\tSPIDY MASTER CONTROLLER\n");
		printf("1.Calibrate\n2.Test\n3.Walk\n4.Quit\n\nMake \
your choice: ");
		scanf("%s%*[^\n]*", &choice[0]);
		if (choice[0] == 'q' || choice[0] == 'Q')
			choice_value = 4;
		else
			choice_value = atoi(choice);
		printf("You choose %i\n", choice_value);
	}
	return choice_value;
}

/* Closing down */
int closing(int return_request)
{
	return return_request;
}

/* TODO: move in a separate file test.c */
int test(void) {
	return 0;
}

/*
 * Send a particular value to an engine, or wait for pipe becoming available
 */
void _send_signal(uint32_t engine_code, int value, char *device)
{
	FILE *named_pipe;
	int file_desc;
	int warning=1;
	do {	/* pipe has to be created outside.
		 * It only waits it's available.
		 */
		file_desc=open(device, O_RDWR | O_NONBLOCK);
		named_pipe = fdopen(file_desc, "w");
		if (warning-->0 && !named_pipe)
			printf("Can't send value into %s, trying again\n",
								device);
	} while (!named_pipe);
	if (named_pipe) {
		fprintf(named_pipe, "%02u %i\n", engine_code, value);
		fclose(named_pipe);
	}
}

/*
 * Sends not calibrated values to pipe_sim and calibrated ones to pipe
 */
void send_signal(uint32_t engine_code, int value)
{
	/* to pipe sim without calibration */
	_send_signal(engine_code, value, DEV_INPUT2);

	/* update last value without calibration */
	int engine = eng2num(engine_code);
	last_values[engine]=value;

	/* calibration */
	value += zero_values[engine];
	if (value > max_values[engine])
		value = max_values[engine];
	if (value < min_values[engine])
		value = min_values[engine];

	/* correction of angle direction */
	value *= leg_negatives[engine];

       /* to pipe with calibration*/
	_send_signal(engine_code, value, DEV_INPUT);
}
