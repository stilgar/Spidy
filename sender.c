#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "spidy.h"

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

	while (h<10) {
		/* wait ready */
		while (out!=CHAR_MREADY) {
			out=fgetc(fr);
			printf("%i ", out);
		}
		printf("Received Ready\n");

		/* send buffer */
		printf("BUFFER --> ");
		fputc(CHAR_SFRAME, fw);

		/* building values and send it */
		for (i=0;i<9;i++) {
			out=DEG_0+f*18;
			printf("%i ",out);
			fputc(out, fw);
		}
		printf("\n");

		fputc(CHAR_SFRAME, fw);
		fputc(CHAR_ENDLINE, fw);

		f++;  f%=11;	/* from 1 to 10 */
		h++;
		sleep(1);
	}
	fclose(fw);
	fclose(fr);

	printf("Exit\n");
	return 0;
}

