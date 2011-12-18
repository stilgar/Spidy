/* basic IO utilities used in several places */
#ifndef __IO_H__
#define __IO_H__

extern void putc(int c);
extern void puts(char *s);
extern int getc(void);
extern int pingc(void);
extern int gets(char *s, int len);
extern int spidy_setup(void *unused);
extern int getCommand();


#endif /* __IO_H__ */
