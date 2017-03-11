#ifndef __VGA_H__
#define __VGA_H__

void init_vga();
void vga_write_char(char ch);
void vga_write_buf(char *buf, int len);
int printk(const char *fmt, ...);

#endif /* __VGA_H__ */
