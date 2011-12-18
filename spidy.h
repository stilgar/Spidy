/* basic utility functions used in several places */
#ifndef __SPIDY_H__
#define __SPIDY_H__

#include <stdint.h>

#define DEBUG_LEVEL 1	/* 0 = control characters
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
#define CHAR_MREADY	0x30	/* = 0 (48) */
#define CHAR_MSTART	0x31	/* = 1 (49) */
#define CHAR_MEND	0x32	/* = 2 (50) */

#define CHAR_CLOOP	0x33	/* = 3 (51) */
#define CHAR_CREAD	0x34	/* = 4 (52) */
#define CHAR_CBUF	0x35	/* = 5 (53) */

#define CHAR_SREADY	0x36	/* = 6 (54) */
#define CHAR_SSTART	0x37	/* = 7 (55) */
#define CHAR_SEND	0x38	/* = 8 (56) */
#define CHAR_SFRAME	0x39	/* = 9 (57) */

#define CHAR_ENDLINE	0x0a	/* = LF */
#define DEG_0		0x4B	/* First character corresponding 0 deg [K] */
#define DEG_180		0xFF	/* Last character corresponding 180 deg */

#define CHAR_TEST_MEND	0x2	/* used only in tests */
#define CHAR_TEST_SEND	0x3

extern int spidy_main(void *unused);
extern void spidy_step(void *unused);
extern int getCommand();
extern int saveBuffer();
extern void setPort();
extern int getBuffer();

#endif /* __SPIDY_H__ */
