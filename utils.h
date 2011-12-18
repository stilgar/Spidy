/* basic utility functions used in several places */
#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>

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

#endif /* __UTILS_H__ */


