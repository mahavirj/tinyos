#include <stdarg.h>
#include <string.h>
#include <unistd.h>

extern int hex_to_str(const int num, char buf[]);
extern int dec_to_str(const int num, char buf[]);

int printf(const char *fmt, ...)
{
	va_list ap;
	int d, x;
	char c, *s;
	const char *p;
	void *pi;
	char buf[96];
	int index = 0;
	int ret;

	va_start(ap, fmt);
	for (p = fmt; *p; p++) {
		if (*p != '%') {
			buf[index++] = *p;
			continue;
		}
		switch (*++p) {
		case 's':              /* string */
			s = va_arg(ap, char *);
			strncpy(&buf[index], s, strlen(s));
			index += strlen(s);
			break;
		case 'd':              /* int */
			d = va_arg(ap, int);
			ret = dec_to_str(d, &buf[index]);
			index += ret;
			break;
		case 'x':		/* hex */
			x = va_arg(ap, int);
			ret = hex_to_str(x, &buf[index]);
			index += ret;
			break;
		case 'p':		/* pointer */
			pi = va_arg(ap, void *);
			if (pi) {
				ret = hex_to_str((int) pi, &buf[index]);
				index += ret;
			} else {
				strncpy(&buf[index], "NULL", strlen("NULL"));
				index += strlen("NULL");
			}
			break;
		case 'c':              /* char */
			/* need a cast here since va_arg only
			   takes fully promoted types */
			c = (char) va_arg(ap, int);
			buf[index++] = c;
			break;
		}
	}
	va_end(ap);
	return write(1, buf, index);
}
