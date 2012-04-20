#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "spidy.h"
#include "pc.h"
#include "link.h"
#include "list.h"

FILE *fw;
FILE *fr;
FILE *input;
int engine = 0;
int deg = DEG_NEG90;
int buffer_len;
struct buffer buffer;
struct buffer *tmp;
int debug = 0;

char *parse(char **point);
int closing(int return_request);
int dialogue(void);
FILE *open_stream(char *device, int open_flags, char *typo);

/*
 * INPUT: 00 [#deg] 01 [#deg] 10 [#deg] 12 [#deg] ...
 * OUTPUT: 0 [#deg+90] 1 [#deg+90] 3 [#deg+90] 5 [#deg+90] ...
 *
 * INPUT
 *   first number is engine location:
 *     * first digit to choose leg (0x-1x-2x-3x-4x-5x)
 *     * second digit to choose which motor of each leg (x0-x1-x2)
 *   second number is degree at which it has to move (from -90 to 90)
 *
 * OUTPUT (frames of max 7 couples of numbers)
 *   first number is engine location (from 0 to 18) to choose leg
 *   and this is how it's calculated -->  (input % 10) + (input / 10)*3
 *
 *   second number is the degree but Spidy starts at 0 (from 0 to 180)
 *   so it's converted by --> input + 90
 */
int main (int argc, char **argv)
{
	int out=0, hs=1, c;
	char *dev_usb;

	/*
	 * Accepted Parameters
	 *  -h  [ 0 | 1 (default) ]		- Use Hand Shake or not
	 *  -d  [ 0 (default) | 1 | 2 | 3 ]	- Debug Level
	 *			0 = only errors
	 * 			1 = errors and states
	 * 			2 = errors, states and buffers
	 *			3 = all
	 */
	while ((c = getopt (argc, argv, "h:d:")) != -1) {
		switch (c) {
			case 'd':
				debug=atoi(optarg);
				if (debug < 0)
					debug = 0;
				if (debug > 3)
					debug = 3;
				if (debug > 0)
					printf("Debug Level %i\n", debug);
			break;
			case 'h':
				hs=atoi(optarg);
				if (hs)
					hs = 1;
				else
					hs = 0;
			break;
			default:
			break;
		}
	}

	/* Initialize buffer list */
	INIT_LIST_HEAD(&buffer.list);

	/* Getting USB device from environment or set it to default */
	dev_usb = getenv("SPIDY_USB");
	if (!dev_usb)
		dev_usb = DEV_USB;

	/* Opening input/output streams */
	input = open_stream(DEV_INPUT, O_RDWR | O_NONBLOCK, "r");
	fw = open_stream(dev_usb, O_RDWR | O_NONBLOCK, "w");
	fr = open_stream(dev_usb, O_RDWR | O_NONBLOCK, "r");
	if (!fw || !fr) {
		fprintf(stderr, "ERROR: %s is not accessible\n", dev_usb);
		closing(1);
	}

	/* Handshake with Spidy */
	if (hs) {
		while (out!=CHAR_MSTART) {
			out=fgetc(fr);
			if (debug > 2)
				printf("%i ",out);
		}
		if (debug > 0)
			printf("Received Start -> ");

		/* Start session */
		fputc(CHAR_SSTART, fw);
		fputc(CHAR_ENDLINE, fw);
		if (debug > 0)
			printf("Sent Start\n");
	} else {
		if (debug > 0)
			printf("No Handshake\n");
	}
	dialogue();
	closing(0);
	return 1;
}

/* Parse input */
char *parse(char **point)
{
	int readtot=0;
	while (**point && (**point == ' ' || **point == '\t'))
		*point+=1; /* find next number */
	if ((readtot=sscanf(*point, "%i %i", &engine, &deg)) == 2) {
		while (**point && (**point != ' ' && **point != '\t'))
			*point+=1; /* find next space */
		while (**point && (**point == ' ' || **point == '\t'))
			*point+=1; /* find next number */
		while (**point && (**point != ' ' && **point != '\t'))
			*point+=1; /* find next space */
	} else if (readtot == 1 && engine == -1) { /* accepts only -1 alone */
		while (**point && (**point != ' ' && **point != '\t'))
			*point+=1; /* find next space */
	} else {
		*point = NULL;
	}
	return *point;
}

/* Reads couples of (ENG,DEG) from PIPE
 * and writes them to USB according to the protocol
 */
int dialogue(void)
{
	int out=0, i;
	char line[1024], *point;
	struct list_head *pos, *q;
	buffer_len=0;

	while (1) {
		/* Reads input (PIPE) */
		while (fgets(line, sizeof(line), input)) {
			point = &line[0];
			while (*point) {
				if ((point = parse(&point))) {
					if (engine > 0
					    && (engine > 52 || (engine%10)>2
					        || deg > 90 || deg < -90)) {
						/* Values not recognized */
						continue;
					} else if (engine == -1) {
						deg = 0;
					}
					/* Add to buffer */
					tmp = NULL;
					tmp = (struct buffer *)malloc
						(sizeof(struct buffer));
					tmp->degree=deg;
					tmp->engine=engine;
					list_add_tail(&(tmp->list),
							&(buffer.list));
					buffer_len++;
				}
			}
			/* if there are couples more than "buffer_len"
			 * stop reading and send them to usb
			 */
			if (buffer_len > 7)
				break;
		}

		/* Write to USB */
		while (buffer_len > 0) {
			/* Wait ready by Spidy */
			while (out!=CHAR_MREADY) {
				out=fgetc(fr);
				if (debug > 2)
					printf("%i ", out);
			}
			if (debug > 0)
				printf("Received Ready\n");

			/* Send buffer
			 * SFRAME #eng #deg .. #eng #deg SFRAME_END ENDLINE
			 */
			tmp=NULL;
			tmp = list_entry(pos, struct buffer, list);
			if (tmp->engine >= 0) {
				if (debug > 1)
					printf("BUFFER --> ");
				fputc(CHAR_SFRAME, fw);

				/* building values and send it */
				tmp=NULL;
				i=0;
				list_for_each_safe(pos, q, &(buffer.list)) {
					if (++i>7)
						break;
					tmp = list_entry(pos, struct buffer,
									list);
					if (tmp->engine < 0)
						break;
					if (debug > 1)
						printf("(%i %i) ", tmp->engine,
								tmp->degree);
					fputc(CHAR_ENGINE_FIRST+eng2num(
							tmp->engine), fw);
					fputc(deg2chr(tmp->degree), fw);
					list_del(pos);
					buffer_len--;
					free(tmp);
				}
				fputc(CHAR_SFRAME_END, fw);
				fputc(CHAR_ENDLINE, fw);
			}
			if (debug > 1)
				printf("\n");

			/* Shutting down
			 * Takes tmp from send buffer header or send buffer
			 * processing. Tmp is segmentation fault only if
			 * send buffer has empty buffer (buffer_len = 0)
			 */
			if (buffer_len != 0 && tmp->engine < 0) {
				if (debug > 0)
					printf("Shutting down sequence \
initiated\n");
				fputc(CHAR_SEND, fw);
				fputc(CHAR_ENDLINE, fw);
				/* The only escape in this function */
				return 1;
			}

			/* Listing remaining commands in buffer */
			tmp = NULL;
			list_for_each_safe(pos, q, &(buffer.list)) {
				tmp = list_entry(pos, struct buffer, list);
				if (debug > 1)
					printf("MEM %i-%i ", tmp->engine,
								tmp->degree);
			}
			if (debug > 1)
				printf("\n");

		 }
	}
	/* This return will never be used */
	return 0;
}

/* Closing down */
int closing(int return_request)
{
	/* Unbuffering */
	tmp=NULL;
	struct list_head *pos, *q;
	list_for_each_safe(pos, q, &(buffer.list)) {
		tmp = list_entry(pos, struct buffer, list);
		list_del(pos);
		free(tmp);
	}
	/* Removing pipe */
	if(remove(DEV_INPUT) != 0) {
		fprintf(stderr, "Error deleting file %s\n", DEV_INPUT);
		if (return_request == 0)
			return_request=1;
	}
	/* Closing streams */
	if (fw)
		fclose(fw);
	if (fr)
		fclose(fr);
	if (input)
		fclose(input);
	if (debug > 0)
		printf("Shutting down\n");
	exit(return_request);
}

/* Open a Stream of a file or create a pipe for it */
FILE *open_stream(char *device, int open_flags, char *typo)
{
	FILE *ret;
	int file_desc = open(device, open_flags);
	if (file_desc < 0) {
		if (mkfifo (device, S_IRUSR | S_IWUSR)) {
			fprintf(stderr, "can't create stream %s\n", device);
			return NULL;
		} else {
			/* FIXME: it has not to be 1000 but a certain user
			 * chown(device, 1000, 1000);
			 */
			file_desc = open(device, open_flags);
		}
	}
	ret = fdopen(file_desc, typo);
	if (!ret) {
		fprintf(stderr, "can't access stream or file %s\n", device);
		return NULL;
	}
	return ret;
}
