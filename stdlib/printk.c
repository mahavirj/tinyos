#include <stdarg.h>
#include <vga.h>

extern int hex_to_str(const int num, char buf[]);
extern int dec_to_str(const int num, char buf[]);

static void write_str(const char *buf)
{
	while (*buf)
		sys_write_char(*buf++);
}

int printk(const char *fmt, ...)
{
	va_list ap;
	int d, x;
	char c, *s;
	const char *p;
	void *pi;
	char tmp_buf[16];

	va_start(ap, fmt);
	for (p = fmt; *p; p++) {
		if (*p != '%') {
			sys_write_char(*p);
			continue;
		}
		switch (*++p) {
		case 's':              /* string */
			s = va_arg(ap, char *);
			write_str(s);
			break;
		case 'd':              /* int */
			d = va_arg(ap, int);
			dec_to_str(d, tmp_buf);
			write_str(tmp_buf);
			break;
		case 'x':		/* hex */
			x = va_arg(ap, int);
			hex_to_str(x, tmp_buf);
			write_str(tmp_buf);
			break;
		case 'p':		/* pointer */
			pi = va_arg(ap, void *);
			if (pi) {
				hex_to_str((int) pi, tmp_buf);
				write_str(tmp_buf);
			} else {
				write_str("NULL");
			}
			break;
		case 'c':              /* char */
			/* need a cast here since va_arg only
			   takes fully promoted types */
			c = (char) va_arg(ap, int);
			sys_write_char(c);
			break;
		}
	}
	va_end(ap);
	return 0;
}
