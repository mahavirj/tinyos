#include <console.h>

#define UART_BASE 0x4000C000

void console_init()
{
	/* Nothing to do for now */
}

void write(int fd, const char *buf, int len)
{
	(void) fd;

	volatile int *uart0 = (volatile int *) UART_BASE;
	while (len--)
		*uart0 = *buf++;
}
