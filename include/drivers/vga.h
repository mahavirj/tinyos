#ifndef __VGA_H__
#define __VGA_H__

void init_vga();
void sys_write_char(char ch);
int sys_write(char *buf, int len);
int printk(const char *fmt, ...);

#endif /* __VGA_H__ */
