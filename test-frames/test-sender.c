#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "../spidy.h"

#define DEV "/dev/ttyUSB0"

int main (int argc, char **argv)
{
	FILE *fw = fopen(DEV, "w");
	FILE *fr = fopen(DEV, "r");

	int out=0;
	int h=0,i,f=0;

	if (!fw || !fr) {
		fprintf(stderr, "/dev/ttyUSB0 is not accessible\n");
		exit(1);
	}

	/* HANDSHAKE */
	while (out!=CHAR_MSTART) {
		out=fgetc(fr);
		printf("%i ",out);
	}
	printf("Received Start -> ");

	/* START SESSION */
	fputc(CHAR_SSTART, fw);
	fputc(CHAR_ENDLINE, fw);
	printf("Sent Start\n");

	printf("# Sending Motors\n");

	/* wait ready */
	while (out!=CHAR_MREADY) {
		out=fgetc(fr);
		printf("%i ", out);
	}
	printf("Received Ready\n");

	while (h<2) {
		/* send buffer */
		printf("BUFFER --> ");
		fputc(CHAR_SFRAME, fw);

		/* building values and send it */
		for (i=1+f*9;i<=9+f*9;i++) {
			out=DEG_0+i*10;
			printf("%i ",out);
			fputc(out, fw);
		}
		printf("\n");

		fputc(CHAR_SFRAME, fw);
		fputc(CHAR_ENDLINE, fw);

		f++;  f%=2;	/* from 1 to 10 */
		h++;

		sleep(1);

		/* wait ready */
		while (out!=CHAR_MREADY) {
			out=fgetc(fr);
			printf("%i ", out);
		}
		printf("Received Ready\n");
	}
	printf("# Testing Frames\n");
	for (i=0;i<40;i++) {
		while(out != CHAR_TEST_MEND) {
			out=fgetc(fr);
			printf("%c", out);
		}
		sleep(1);
		printf("\n# Next Frame\n");
		fputc(CHAR_TEST_SEND, fw);
		fputc(CHAR_ENDLINE, fw);
		out=0;
	}
	printf("# Exiting\n");


	fclose(fw);
	fclose(fr);
	return 0;
}

