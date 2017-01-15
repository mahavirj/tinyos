#include <stdarg.h>
#include <vga.h>

void printk(char *fmt, ...)
{
	va_list ap;
	int d, x;
	char c, *s, *p;

	va_start(ap, fmt);
	for (p = fmt; *p; p++) {
		if (*p != '%') {
			k_write_char(*p);
			continue;
		}
		switch (*++p) {
		case 's':              /* string */
			s = va_arg(ap, char *);
			k_write(s);
			break;
		case 'd':              /* int */
			d = va_arg(ap, int);
			k_write_dec(d);
			break;
		case 'x':		/* hex */
			x = va_arg(ap, int);
			k_write_hex(x);
			break;
		case 'c':              /* char */
			/* need a cast here since va_arg only
			   takes fully promoted types */
			c = (char) va_arg(ap, int);
			k_write_char(c);
			break;
		}
	}
	va_end(ap);
}
