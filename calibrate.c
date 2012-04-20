#include <stdio.h>
#include <stdlib.h>
#include "master.h"
#include "spidy.h"
#include "pc.h"

void fix_values(int *min, int *zero, int *max);
int save_file(void);
void show_config(void);
int get_engine(void);
int set_value(char *label, int *current_val, int l_bound, uint32_t eng_code);

/* Show a screen with each value in all global arrays */
void show_config(void)
{
	int i;
	printf("\n\t\t\tCURRENT VALUES\n\n");
	printf("                 m   z   M           m   z   M           m   \
z   M\n");
	for (i=5; i>=3; i--) {
		printf("             %02i:%3i %3i %3i  ||  %02i:%3i %3i %3i  \
||  %02i:%3i %3i %3i\n", num2eng(i), min_values[i], zero_values[i],
		max_values[i], num2eng(i+6), min_values[i+6], zero_values[i+6],
		max_values[i+6], num2eng(i+12), min_values[i+12],
		zero_values[i+12], max_values[i+12]);
	}
	printf("          <  ________________________________________________\
_______|\n          <  ______________________________________________________\
_|\n          <                                                         |\n");
	for (i=0; i<3; i++) {
		printf("             %02i:%3i %3i %3i  ||  %02i:%3i %3i %3i  \
||  %02i:%3i %3i %3i\n", num2eng(i), min_values[i], zero_values[i],
		max_values[i], num2eng(i+6), min_values[i+6], zero_values[i+6],
		max_values[i+6], num2eng(i+12), min_values[i+12],
		zero_values[i+12], max_values[i+12]);
	}
	printf("                 m   z   M           m   z   M           m   \
z   M\n");
	printf("\n\n");
}

/*
 * If values are bad, it tries to order them.
 * It they are too bad, it will restore three
 * zero values.
 */
void fix_values(int *min, int *zero, int *max)
{
	if (*min<-90 || *min>90)
		*min = 0;
	if (*zero<*min || *zero>90)
		*zero = *min;
	if (*max<*zero || *max>90)
		*max = *zero;
}

/*
 * It loads configuration file into global arrays
 * Example line ->  "10: -80 10 90"
 * Pattern -> "<engine>: <min_value> <zero_value> <max_value>"
 */
int load_file(void)
{
	FILE *fp;
	char line[1024];
	uint32_t engine_code, engine;
	int min, zero, max;

	printf("Loading configuration file...");
	if (!(fp = fopen(CONFIG_FILE, "r"))) {
		printf(" file not found. Creating it.\n");
                return 0;
        }
        printf("Done!\n");

	/* reads file */
	while (fgets(line, sizeof(line), fp)) {
		if (line[0]!='\n') {
			sscanf(line,"%u: %i %i %i", &engine_code, &min,
								&zero, &max);
			if ((engine = eng2num(engine_code))!= -1) {
				fix_values(&min, &zero, &max);
				min_values[engine] = min;
				zero_values[engine] = zero;
				max_values[engine] = max;
			}
		}
	}
	fclose(fp);
	return 1;
}

/*
 * Write to configuration file from global arrays.
 * Values have been already checked.
 * Example line ->  "10: -80 10 90"
 * Pattern -> "<engine>: <min_value> <zero_value> <max_value>"
 */
int save_file(void)
{
	int i = 0;
	FILE *fp;
	fp = fopen(CONFIG_FILE, "w");
	if (fp == NULL) {
		printf("Error while saving\n");
		return 0;
	}
	for(i=0; i<18; i++)
		fprintf(fp, "%02i: %i %i %i\n", num2eng(i), min_values[i],
					      zero_values[i], max_values[i]);
	fclose(fp);
	return 1;
}

/*
 * Asks which engine to calibrate.
 * Returns: -1 for error, -2 if it has to quit or the engine number (0,17)
 */
int get_engine(void)
{
	char input_tmp[1024];
	char chr;
	uint32_t eng_code;

	printf("[q = quit   f = finish]\n\nEngine Code = ");
	scanf("%s%*[^\n]", &input_tmp[0]);
	if (sscanf(input_tmp, "%u", &eng_code)) {
		return eng2num(eng_code);
	}
	if (sscanf(input_tmp, "%c", &chr) && (chr == 'q' || chr == 'Q')) {
		return -2;
	}
	return -1;
}

/*
 * It asks to change a particular value passed as an address,
 * controlling that it is not below l_bound and sending it into
 * pipe using eng_code as engine.
 * It returns
 *   -1 if it tries a value but it is not confirmed. [NUMBER]
 *   0 if it is pressed q, so no value is passed. Nothing change. [Q]
 *   1 if value passed is also confirmed. [F]
 */
int set_value(char *label, int *current_val, int l_bound, uint32_t eng_code)
{
	char input_tmp[1024];
	int new_val;
	char chr;

	printf("%s %3i -> ", label, *current_val);
	scanf("%s%*[^\n]", &input_tmp[0]);

	if (sscanf(input_tmp,"%c", &chr)) {
		if (chr == 'q')
			return 0;

		if (chr == 'f' && *current_val>=l_bound) {
			send_signal(eng_code, (int)*current_val);
			return 1;
		}
	}

	if (sscanf(input_tmp,"%i", &new_val)) {
		if(new_val>=-90 && new_val<=90 && new_val>=l_bound) {
			*current_val = new_val;
			send_signal(eng_code, new_val);
		}
	}

	return -1;
}

/*
 * Calibration of each engine. It shows a screen with each value
 * for all engines. It allows to choose an engine and set minimum,
 * and maximum degree and a middle value.
 */
int calibrate(void)
{
	int engine=-1, ok=1, i=0, quit=0;
	char *labels[3] = {"MIN=", "ZER=", "MAX="};

	while(!quit) {
		while(engine == -1) {
			load_file();
			system("clear");
			printf("1. CALIBRATION\n");
			show_config(); /* print each global array value */
			init_engines();	/* set engines to middle value */
			engine = get_engine();
			quit = (engine < 0); /* get_engine returns -1 to quit */
		}
		int *values[3] = { &min_values[engine], &zero_values[engine],
							&max_values[engine]};
		if (!quit) {
			for (i=0; i<3 && !quit; ) {
				if (i == 0) {
					ok = set_value(labels[i], values[i],
							-91, num2eng(engine));
				} else {
					ok = set_value(labels[i], values[i],
							*values[i-1], num2eng
							(engine));
				}
				if (ok == 1) {
					i++;
				} else if (ok == 0) {
					quit = !quit;
					break; /* pressed number or q */
				}
			}
			if (ok == 1) {
				if (!save_file()) {
					printf("\nCan't save values into\
file\n");
				} else {
					engine=-1;
				}
			}
		}
	}
	if (engine > 0)
		return 1;
	else
		return engine;
}
