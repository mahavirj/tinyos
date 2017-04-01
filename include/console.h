#ifndef __CONSOLE_H__
#define __CONSOLE_H__

void console_init(void);
int printk(const char *fmt, ...);
int printf(const char *fmt, ...);

#endif /* __CONSOLE_H__ */
