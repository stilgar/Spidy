#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "list.h"
#include "master.h"
#include "pc.h"

/* Time from computer power on (in ms) */
#define NOW		((ts.tv_sec * 1000) + (ts.tv_nsec / (1000*1000)))

/* Period in millis, from one sending to another (min 40 ms) => USB rate */
#define ENABLE_PERIOD	50

/* samples per step */
#define SAMPLES_STEP	1024

/* Spidy's speed (how many samples per enable) */
#define MAX_SAMPLES	20	/* 100% -> 1,536 s  */
#define MIN_SAMPLES	5	/*   1% -> 6,144 s  */

/*
 * XXX: use walk.cfg
 * 3 engines per leg
 * 7 couples sample-value
 */
int walk_cfg[3][7][2] = {
     {{0, 20},{85, 20},{427,-20},{512,-20},{597,-20},{939, 20},{1024, 20}},
     {{0, 45},{85, 10},{427, 10},{512, 45},{597, 45},{939, 45},{1024, 45}},
     {{0, 45},{85, 45},{427, 45},{512, 45},{597, 45},{939, 45},{1024, 45}}
		     	};

struct timespec ts;
struct buffer buffer;	/* commands list [engine,degree] */
int effective_steps = 0;
int speed;		/* samples per enable (by user)*/
uint32_t start_time;	/* when the walk starts */

int stage(int *group, int n_elements, uint32_t start_sample);

/*
 * Action from main menu
 * Makes a walk in N steps with a fixed speed
 */
int walk(void)
{
	char choice[80];
	int steps = 0;
	int group[2][3] = { {0, 3, 4},
			    {1, 2, 5} }; /* number of legs */
	int loops=0;

	/* exit on QUIT character */
	while(1) {
		/* Ask to user for STEPS and SPEED */
		while ((steps - effective_steps) <= 0) {
			system("clear");
			printf("How many steps? ");
			scanf("%s%*[^\n]*", &choice[0]);
			if (choice[0] == 'q' || choice[0] == 'Q')
				return 0;
			else
				steps = atoi(choice);
			effective_steps=0;
			speed = 0;
			while(speed<1 || speed>100) {
				printf("Speed (1-100)? ");
				scanf("%s%*[^\n]*", &choice[0]);
				if (choice[0] == 'q' || choice[0] == 'Q') {
					return 0;
				} else {
					speed = atoi(choice);
				}
			}
			/* 100 : MAX_SAMPLES-MIN_SAMPLES = speed : x */
			speed = MIN_SAMPLES + (speed*(MAX_SAMPLES-
							MIN_SAMPLES))/100;
			loops = 0;
			if (steps > 0) {
				printf("\n\n\tLet's Walk!\n");
				INIT_LIST_HEAD(&buffer.list);
				stage_getUp(); /* init all engines */
				clock_gettime(CLOCK_MONOTONIC, &ts);
				start_time = NOW;
			}
		} /* starts only if steps is bigger than 0 */

		/* calculate new command list */
		stage(group[0], 3, 0); /* first must have start_sample=0 */
		stage(group[1], 3, 512);

		/* waiting for enable */
		/* FIXME: maybe usleep could decrese CPU load */
		clock_gettime(CLOCK_MONOTONIC, &ts);
		while(NOW < (loops*ENABLE_PERIOD + start_time))
			clock_gettime(CLOCK_MONOTONIC, &ts);

		/* send commands list to pipe */
		struct list_head *pos, *q;
		struct buffer *tmp;
		list_for_each_safe(pos, q, &(buffer.list)) {
			tmp = list_entry(pos, struct buffer, list);
			send_signal(tmp->engine, tmp->degree);
			list_del(pos);
			free(tmp);
		}

		loops++;
	}
	return 1;
}

/*
 * Gets a group of 3 legs and a sample to start to calculate from
 *
 * group 	-> pointer to the legs' array
 * n_elements 	-> size of array pointed by group
 * start_sample -> number of sample [0,1024]
 * negative 	-> fix leg motor angles to be coherent to robot
 */
int stage(int *group, int n_elements, uint32_t start_sample)
{
	int i, engine, sample;
	uint32_t min_tmp=0, max_tmp=0, diff, n_samples, curr_sample;
	int min_deg=0, max_deg=0;
	struct buffer *tmp;
	int tmp_eng, tmp_deg;

	/* calculates time elapsed from global start_time */
	clock_gettime(CLOCK_MONOTONIC, &ts);
	diff = NOW - start_time;

	/* converts elapsed time in elapsed samples */
	n_samples = (diff / ENABLE_PERIOD) * speed;

	/* and calculates in which sample of the step we are */
	curr_sample = (start_sample + n_samples) % SAMPLES_STEP;

	/* first call of stage must absolutely have start_sample = 0 */
	if (start_sample == 0)
		effective_steps = n_samples / SAMPLES_STEP;

	/* cycle on the group's legs */
	for(i=0; i<n_elements; i++) {
		/* cycle on the leg's engines */
		for(engine=0; engine<3; engine++) {
			/* cycle on the engine's samples */
			for(sample=0; sample<7; sample++) {
				/* find intervals of samples and values
						 in which curr_sample is */
				if (walk_cfg[engine][sample][0]<=curr_sample) {
					min_deg = walk_cfg[engine][sample][1];
					min_tmp = walk_cfg[engine][sample][0];
				} else {
					max_deg = walk_cfg[engine][sample][1];
					max_tmp = walk_cfg[engine][sample][0];
					sample = 7;
				}
			}
			/* get couple */
			tmp_eng = group[i]*10 + engine;
			tmp_deg = ((curr_sample-min_tmp)*abs(max_deg-min_deg)
							/(max_tmp-min_tmp));

			/* suppose linear function */
			if (min_deg < max_deg)
				tmp_deg = min_deg + tmp_deg;
			else
				tmp_deg = min_deg - tmp_deg;

			/* don't send if it is already in this position */
			if (last_values[eng2num(tmp_eng)]!=tmp_deg) {
				/* add couple (engine-value) to commands list*/
				tmp = NULL;
				tmp = (struct buffer *)malloc(sizeof(struct
								buffer));
				tmp->engine = tmp_eng;
				tmp->degree = tmp_deg;
				list_add_tail(&(tmp->list),&(buffer.list));
			}
		}
	}
	return 1;
}

/* TODO: It has to move Spidy to a get up position */
int stage_getUp(void)
{
	return 1;
}
