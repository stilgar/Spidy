#include "micro.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#define __GPIO_BASE(x)		(0x50000000 + ((x) << 16))
#define __GPIO_DIR(x)		(__GPIO_BASE(x) + 0x8000)
#define __GPIO_DAT(x)		(__GPIO_BASE(x) + 0x3ffc)

/* The cfg registers for the various pins are not laid out in order */
#define __GPIO_CFG_BASE		0x40044000
#define __GPIO_CFG_P0_0		0x0c
#define __GPIO_CFG_P0_1		0x10
#define __GPIO_CFG_P0_2		0x1c
#define __GPIO_CFG_P0_3		0x2c
#define __GPIO_CFG_P0_4		0x30
#define __GPIO_CFG_P0_5		0x34
#define __GPIO_CFG_P0_6		0x4c
#define __GPIO_CFG_P0_7		0x50
#define __GPIO_CFG_P0_8		0x60
#define __GPIO_CFG_P0_9		0x64
#define __GPIO_CFG_P0_10	0x68
#define __GPIO_CFG_P0_11	0x74
#define __GPIO_CFG_P1_0		0x78
#define __GPIO_CFG_P1_1		0x7c
#define __GPIO_CFG_P1_2		0x80
#define __GPIO_CFG_P1_3		0x90
#define __GPIO_CFG_P1_4		0x94
#define __GPIO_CFG_P1_5		0xa0
#define __GPIO_CFG_P1_6		0xa4
#define __GPIO_CFG_P1_7		0xa8
#define __GPIO_CFG_P1_8		0x14
#define __GPIO_CFG_P1_9		0x38
#define __GPIO_CFG_P1_10	0x6c
#define __GPIO_CFG_P1_11	0x98
#define __GPIO_CFG_P2_0		0x08
#define __GPIO_CFG_P2_1		0x28
#define __GPIO_CFG_P2_2		0x5c
#define __GPIO_CFG_P2_3		0x8c
#define __GPIO_CFG_P2_4		0x40
#define __GPIO_CFG_P2_5		0x44
#define __GPIO_CFG_P2_6		0x00
#define __GPIO_CFG_P2_7		0x20
#define __GPIO_CFG_P2_8		0x24
#define __GPIO_CFG_P2_9		0x54
#define __GPIO_CFG_P2_10	0x58
#define __GPIO_CFG_P2_11	0x70
#define __GPIO_CFG_P3_0		0x84
#define __GPIO_CFG_P3_1		0x88
#define __GPIO_CFG_P3_2		0x9c
#define __GPIO_CFG_P3_3		0xac
#define __GPIO_CFG_P3_4		0x3c
#define __GPIO_CFG_P3_5		0x48

/* This is the bitmask of the gpio pins whose function "0" is not the gpio one */
#define __GPIO_WEIRDNESS_0	0xc01
#define __GPIO_WEIRDNESS_1	0x00f
#define __GPIO_WEIRDNESS_2	0x000
#define __GPIO_WEIRDNESS_3	0x000

/*
 * This table is needed to turn gpio number to address. The hairy
 * macros are used to shrink the source code.
 */
#define CFG_REG(port, bit)  __GPIO_CFG_P ## port ## _ ## bit
#define __A(port, bit)  [GPIO_NR(port,bit)] = CFG_REG(port,bit)
static uint8_t gpio_addr[] = {
	__A(0, 0), __A(0, 1), __A(0, 2), __A(0, 3), __A(0, 4), __A(0, 5),
	__A(0, 6), __A(0, 7), __A(0, 8), __A(0, 9), __A(0,10), __A(0,11),
	__A(1, 0), __A(1, 1), __A(1, 2), __A(1, 3), __A(1, 4), __A(1, 5),
	__A(1, 6), __A(1, 7), __A(1, 8), __A(1, 9), __A(1,10), __A(1,11),
	__A(2, 0), __A(2, 1), __A(2, 2), __A(2, 3), __A(2, 4), __A(2, 5),
	__A(2, 6), __A(2, 7), __A(2, 8), __A(2, 9), __A(2,10), __A(2,11),
	__A(3, 0), __A(3, 1), __A(3, 2), __A(3, 3), __A(3, 4), __A(3, 5),
};

#define GPIO_MAX  (ARRAY_SIZE(gpio_addr) - 1)

extern uint32_t __gpio_get(int gpio);
extern void __gpio_set(int gpio, uint32_t value);

/* This other table marks the ones where AF0 is swapped with AF1 */
static uint16_t gpio_weird[] = {
	__GPIO_WEIRDNESS_0, __GPIO_WEIRDNESS_1,
	__GPIO_WEIRDNESS_2, __GPIO_WEIRDNESS_3
};

/*
 * What follows is the public interface.
 * Note that only gpio_dir_af() checks the gpio is valid. Other
 * functions are meant to call often and only after setting the mode.
 */
extern void gpio_init(void)
{
	uint32_t reg = readl(0x40048080); /* AHBCLKCTRL */
	writel(reg | 0x1040, 0x40048080);
}

void gpio_dir(int gpio, int output, int value)
{
	int port = GPIO_PORT(gpio);
	int bit = GPIO_BIT(gpio);
	uint32_t reg;

	reg = readl(__GPIO_DIR(port));
	if (output)
		reg |= (1 << bit);
	else
		reg &= ~(1 << bit);

	writel (reg , __GPIO_DIR(port));

	/* After changing the direction we must re-force the value */
	if (output)
		gpio_set(gpio, value);
}


int gpio_dir_af(int gpio, int output, int value, int afnum)
{
	if (gpio > GPIO_MAX || gpio < 0)
		return -1;

	if (afnum < 2) { /* if weird bit, swap AF0 and AF1 */
		int port = GPIO_PORT(gpio);
		int bit = GPIO_BIT(gpio);
		if (gpio_weird[port] & (1 << bit))
			afnum ^= 1;
	}
	/* First set dir to prevent glitches when moving to AF0 */
	gpio_dir(gpio, output, value);
	writel(afnum | 0x80, /* This 0x80 for "digital mode" */
		0x40044000 + gpio_addr[gpio]);
	/* Finally, dir again to force value when moving to gpio-out */
	gpio_dir(gpio, output, value);
	return 0;
}

/* The following functions don't check the gpio value, for speed */
uint32_t __gpio_get(int gpio)
{
	int port = GPIO_PORT(gpio);
	int bit = GPIO_BIT(gpio);

	return readl(__GPIO_DAT(port)) & (1 << bit);
}

int gpio_get(int gpio)
{
	return __gpio_get(gpio) ? 1 : 0;
}

void __gpio_set(int gpio, uint32_t value)
{
	int port = GPIO_PORT(gpio);
	int bit = GPIO_BIT(gpio);

	writel(value, __GPIO_BASE(port) + (0x4 << bit));
}

void gpio_set(int gpio, int value)
{
	__gpio_set(gpio, value << GPIO_BIT(gpio));
}
