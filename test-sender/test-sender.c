#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../link.h"
#include "../spidy.h"

#define DEV "/dev/ttyUSB0"

int main (int argc, char **argv)
{
	FILE *fw = fopen(DEV, "w");
	FILE *fr = fopen(DEV, "r");

	int out=0;
	int i,f=0,end=0;
	int ret;

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
	fputc(0x0a, fw);
	printf("Sent Start\n");

	while (!end) {
		/* USER INPUT */
		printf("0) Send a buffer\n");
		printf("1) Exit\n");
 		scanf("%i",&ret);

		if (ret == 0) {
			/* wait ready */
			while (out!=CHAR_MREADY) {
				out=fgetc(fr);
				printf("%i ",out);
			}
			printf("Received Ready\n");

			/* send ready */
			fputc(CHAR_SREADY, fw);
			fputc(0x0a, fw);
			printf("Sent Ready\n");

			/* send buffer */
			printf("BUFFER --> ");
			fputc(CHAR_SFRAME, fw);

			/* building values and send it */
			for (i=0;i<9;i++) {
				out=DEG_NEG90+f*18;
				printf("%i ",out);
				fputc(out, fw);
			}
			printf("\n");

			fputc(CHAR_SFRAME, fw);
			fputc(0x0a, fw);

		} else if (ret==1) {
			/* end */
			end=1;
		} else {
			printf ("Write only valid numbers!\n");
		}

		f++;  f%=11;	/* from 1 to 10 */
	}
	fclose(fw);
	fclose(fr);

	printf("Exit\n");
	return 0;
}
