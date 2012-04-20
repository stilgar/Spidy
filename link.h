/* character used by SENDER and MICRO */
#ifndef __LINK_H__
#define __LINK_H__

#define CHAR_ENDLINE    0x0a    /* => LineFeed (12) */

/* ENGINES */
#define CHAR_ENGINE_FIRST       0x10    /* => motore 01 (16) */
#define CHAR_ENGINE_LAST        0x21    /* => motore 18 (33) */

/* COMMANDS */
#define CHAR_MREADY     0x30    /* => 0 (48) */
#define CHAR_MSTART     0x31    /* => 1 (49) */
#define CHAR_MEND       0x32    /* => 2 (50) */

#define CHAR_CLOOP      0x33    /* => 3 (51) */
#define CHAR_CREAD      0x34    /* => 4 (52) */
#define CHAR_CBUF       0x35    /* => 5 (53) */

#define CHAR_SREADY     0x36    /* => 6 (54) */
#define CHAR_SSTART     0x37    /* => 7 (55) */
#define CHAR_SEND       0x38    /* => 8 (56) */
#define CHAR_SFRAME     0x39    /* => 9 (57) */
#define CHAR_SFRAME_END 0x3A    /* => : (58) */

#endif /* __LINK_H__ */

