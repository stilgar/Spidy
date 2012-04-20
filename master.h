/* Everything used only by master */
#ifndef __MASTER_H__
#define __MASTER_H__

#include <stdint.h>

#define CONFIG_FILE "config_file"

int min_values[18];
int zero_values[18];
int max_values[18];
int last_values[18];
/* values to correct engine angles to be real */
int leg_negatives[18];

int calibrate(void);
int walk(void);
int test(void); /* TODO */
int load_file(void);
int save_file(void);
void send_signal(uint32_t engine_code, int value);
int stage_getUp(void);
void init_engines(void);

#endif /* __MASTER_H__ */

