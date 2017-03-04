#ifndef __VGA_H__
#define __VGA_H__

void k_video_init();
void k_write_char(char ch);
void printk(const char *fmt, ...);

#endif /* __VGA_H__ */
