#include "micro.h"
#include "hw.h"

int framePeriod; /* period of frame */

/* In boot.S is the initial function called */
int spidy_setup(void *unused)
{
	unsigned int val;

	/* Initializing clock */
	regs[REG_AHBCLKCTRL] |= REG_AHBCLKCTRL_CT32B1;
	regs[REG_AHBCLKCTRL] |= REG_AHBCLKCTRL_IOCON;

	/* enable timer 1, and count at 1,25 ms = 12 *10^6 / (1 / 1,25*10^-3) */
	framePeriod = TIME125;
	regs[REG_TMR32B1TCR] = 1;
	regs[REG_TMR32B1PR] = framePeriod - 1;

	/* First fix pin configuration */
	regs[REG_PIORX] = 1; // 1-6 : rx
	regs[REG_PIOTX] = 1; // 1-7 : tx

	/*
	 * The clock divider must be set before turning on the clock.
	 * We are running at 12MHz with no PLL, so divide by 6.5
	 * (12M / 115200 / 16 = 6.510416)
	 */
	regs[REG_UARTCLKDIV] = 1; /* Don't dived before the internal uart divisor */

	/* Turn on the clock for the uart (and gpio, while we are at it) */
	regs[REG_AHBCLKCTRL] |= REG_AHBCLKCTRL_GPIO;
	regs[REG_AHBCLKCTRL] |= REG_AHBCLKCTRL_UART;

	/* Disable interrupts and clear pending interrupts */
	regs[REG_U0IER] = 0;
	val = regs[REG_U0IIR];
	val = regs[REG_U0RBR]; val = regs[REG_U0LSR];

	/* Set bit rate: enable DLAB bit and then divisor */
	regs[REG_U0LCR] = 0x80;

	/*
	 * This calculation is hairy: we need 6.5, but the fractional
	 * divisor register makes (1 + A/B) where B is 1..15
	 * 6.510416 = 4 * 1.627604 = 4 * (1+5/8)   --- better than 1%
	 */
	val = 4;
	regs[REG_UARTDLL] = val & 0xff;
	regs[REG_UARTDLM] = (val >> 8) & 0xff;
	regs[REG_UARTFDR] = (8 << 4) | 5; /* 1 + 5/8 */

	/* clear DLAB and write mode (8bit, no parity) */
	regs[REG_U0LCR] = REG_8BIT_WORD;
	regs[REG_U0FCR] = REG_FIFO_ON;
	regs[REG_U0FCR] |= (3 << 6);	/* buffer to 14 characters */

	return 0;
}

/* Non blocking getting of single command from buffer */
int getCommand()
{
	return regs[REG_U0RBR];
}

/* Write in UART console a character */
void putc(int c)
{
	if (c == '\n')
		putc('\r');
	while ( !(regs[REG_U0LSR] & REG_U0LSR_THRE) )
		;
	regs[REG_U0THR] = c;
}

/* Write in UART console a string */
void puts(char *s)
{
	while (*s)
		putc (*s++);
}

/* Read from UART console (echoing back what it receive) a character */
int getc(void)
{
	int i = 0;
	while (!(regs[REG_U0LSR] & 1))
		;
	i =  regs[REG_U0RBR];
	if (i == '\r') i = '\n';
	if (i == 0xff) putc(i);
	return i;
}

/* Read from UART console a string */
int gets(char *s, int len)
{
	int i;
	for (i=-1; i<len-1;) {
		i++;
		s[i] = getc();
		if (s[i] == '\n') break;
	}
	s[i++] = '\0';
	return i;
}
