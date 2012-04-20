#ifndef __MICRO_H__
#define __MICRO_H__

#include <stdint.h>

/* We have 32 gpio bits per "port", this is hardwired */
#define GPIO_NR(port, bit)	((port) * 32 + (bit))
#define GPIO_PORT(nr)           ((nr) / 32)
#define GPIO_BIT(nr)            ((nr) % 32)

#define DEBUG_LEVEL 3	/* 0 = control characters
			 * 1 = only CHAR_CBUF (reading buffer)
			 * 2 = + CHAR_CREAD (correct reading)
			 * 3 = + CHAR_CLOOP (no buffer read)
			 */

#define TIME085 10200	/* Last down of motor i-1 */
#define TIME090 10800	/* First down of motor i */
#define TIME125 15000	/* Up of motor i+1 */
#define TIME120 14400	/* Period */
#define TIMEDEG 80	/* 12 * 1000 * 1000 / 1,2 * 10^-3 / 180 */

#define DEGREELIMIT 52	/* Last value managed by period i. Bigger values
						* are managed by period i+1.
						*/

extern void gpio_init(void);
extern int gpio_dir_af(int gpio, int output, int value, int afnum);
extern void gpio_dir(int gpio, int output, int value);
extern int gpio_get(int gpio);
extern void gpio_set(int gpio, int value);

extern void putc(int c);
extern void puts(char *s);
extern int getc(void);
extern int gets(char *s, int len);
extern int spidy_setup(void *unused);
extern int getCommand();

extern int spidy_main(void *unused);
extern void spidy_step(void *unused);
extern void spidy_reset(void);
extern int saveBuffer();
extern void setPort();
extern int getBuffer();

/* basic utility functions used in several places */
int pow(int base, int n);
int atoi(char s[]);
void itoa(int n, char s[]);
void reverse(char s[]);
int strlen(char s[]);
int strcmp(char* a, char* b);
char *inthex(unsigned val);
unsigned hexint(char *s);

/* Let's add readl/writel for portability */
static inline uint32_t readl(unsigned long addr)
{
	return *(volatile uint32_t *)addr;
}
static inline void writel(uint32_t val, unsigned long addr)
{
	*(volatile uint32_t *)addr = val;
}

#endif /* __MICRO_H__ */
