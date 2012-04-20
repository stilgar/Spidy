/* devices, structs and functions shared between MASTER and SENDER  */
#ifndef __PC_H__
#define __PC_H__

#include "list.h"

#define DEV_INPUT "pipe"
#define DEV_INPUT2 "pipe_sim"
#define DEV_USB "/dev/ttyUSB0"

int eng2num(int engine);
int num2eng(int number);
int deg2chr(int degree);
int chr2deg(int character);

struct buffer {
	int engine;
	int degree;
	struct list_head list;
};

#endif /* __PC_H__ */
