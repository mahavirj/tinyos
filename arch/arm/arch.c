#include <arch.h>

extern void timer_init();
void arch_init()
{
	timer_init();
}

