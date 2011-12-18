#include <omp.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "spidy.h"

#define DEV "/dev/ttyUSB0"

/* debug_level -> 0 = no output, 1 = medium output, 2+ = verbose */
int _percent=0;

void receive (int print)
{
	int n=0;
	char ret[1024];
	while(++n<100) {
		fgets(ret, sizeof(ret), stdin);
		if (print > 0)
			printf("[REC] %s\n", ret);
	}
}

int send_init (int print, FILE *fr, FILE *fw)
{
	int out=0;
	char frase[10000], chr[1024];

	/* HANDSHAKE */
	strcpy(frase, "RECEIVE --> ");
	while (out!=CHAR_MSTART) {
		out=fgetc(fr);
		sprintf(chr, "%i ", out);
		strcat(frase, chr);
	}
	if (print > 0)
		printf("[SND] %s\n", frase);
	if (print > 1)
		printf("[SND] Received Start\n");

	/* START SESSION */
	fputc(CHAR_SSTART, fw);
	fputc(CHAR_ENDLINE, fw);
	if (print > 0)
		printf("[SND] Sent Start\n");

	return 1;
}

int send_work (int print, FILE *fr, FILE *fw)
{
	int out=0;
	char frase[10000], chr[1024];
	int i=0;

	/* wait ready */
	strcpy(frase, "RECEIVE --> ");
	while (out!=CHAR_MREADY) {
		out=fgetc(fr);
		sprintf(chr, "%i ", out);
		strcat(frase, chr);
	}
	if (print > 1)
		printf("[SND] %s\n", frase);
	if (print > 0)
		printf("[SND] Received Ready\n");

	/* send buffer */
	fputc(CHAR_SFRAME, fw);

	/* building values and send it */
	strcpy(frase, "BUFFER --> ");
	for (i=0;i<9;i++) {
		out=DEG_0+_percent*18;
		fputc(out, fw);
		sprintf(chr, "%i ", out);
		strcat(frase, chr);
	}
	if (print > 1)
		printf("[SND] %s\n", frase);

	fputc(CHAR_SFRAME, fw);
	fputc(CHAR_ENDLINE, fw);

	_percent++;  _percent%=11;	/* from 1 to 10 */

	sleep(1);

	return 1;
}


int main (int argc, char **argv)
{
	FILE *fw = fopen(DEV, "w");
	FILE *fr = fopen(DEV, "r");
	int i;

	/* controlling streams */
	if (!fw || !fr) {
		fprintf(stderr,	"[MAIN] /dev/ttyUSB0 is not accessible\n");
		exit(1);
	}

	printf("[MAIN] Found device!\n");


	#pragma omp sections
	{
		#pragma omp section
		{
			printf("Start Section Read\n");
			send_init(2, fr, fw);
			for (i=0; i<20 || 1;i++)
				send_work(2, fr, fw);
		}
		#pragma omp section
		{
			printf("Start Section Receive\n");
			receive(2);
		}
	}

	fclose(fw);
	fclose(fr);

	return 0;
}



