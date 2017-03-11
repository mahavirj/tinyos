#include <stdarg.h>
#include <vga.h>

static void write_str(const char *buf)
{
	while (*buf)
		vga_write_char(*buf++);
}

static void to_decimal(const int num)
{
	int quot, i;
	char rem;
	char str[32];

	quot = num;
	i = 0;
	do {
		rem = (quot % 10) + '0';
		quot /= 10;
		str[i++] = rem;
	} while (quot);

	int start, end;
	char temp;
	for (end = i - 1, start = 0; start < end; end--, start++) {
		temp = str[start];
		str[start] = str[end];
		str[end] = temp;
	}

	str[i] = '\0';
	write_str(str);
}

static void to_hex(const int num)
{
	int index = 32;
	int val;

	do {
		index -= 4;
		val = (num >> index) & 0xf;
	} while (!val && index);

	write_str("0x");
	do {
		if (val > 9)
			val = 'A' + (val - 10);
		else
			val += '0';
		vga_write_char(val);
		index -= 4;
		val = (num >> index) & 0xf;
	} while (index >= 0);
}

void printk(const char *fmt, ...)
{
	va_list ap;
	int d, x;
	char c, *s;
	const char *p;

	va_start(ap, fmt);
	for (p = fmt; *p; p++) {
		if (*p != '%') {
			vga_write_char(*p);
			continue;
		}
		switch (*++p) {
		case 's':              /* string */
			s = va_arg(ap, char *);
			write_str(s);
			break;
		case 'd':              /* int */
			d = va_arg(ap, int);
			to_decimal(d);
			break;
		case 'x':		/* hex */
			x = va_arg(ap, int);
			to_hex(x);
			break;
		case 'c':              /* char */
			/* need a cast here since va_arg only
			   takes fully promoted types */
			c = (char) va_arg(ap, int);
			vga_write_char(c);
			break;
		}
	}
	va_end(ap);
}
