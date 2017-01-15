#ifndef __VGA_H__
#define __VGA_H__

void k_video_init();
void k_write(const char *str);
void k_write_char(char ch);
void k_write_dec(const int num);
void k_write_hex(const int num);
void printk(const char *fmt, ...);

#endif /* __VGA_H__ */
