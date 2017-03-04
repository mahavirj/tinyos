#include <isr.h>
#include <helper.h>
#include <task.h>

/* This will keep track of how many ticks that the system
 *  has been running for */
static volatile int timer_ticks;

static void timer_handler(registers_t *r)
{
	(void) r;
	/* Increment our 'tick count' */
	timer_ticks++;

	/* `yield` to invoke scheduler */
	yield();
}

static void timer_phase(int hz)
{
	int divisor = 1193180 / hz;       /* Calculate our divisor */
	outportb(0x43, 0x36);             /* Set our command byte 0x36 */
	outportb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
	outportb(0x40, divisor >> 8);     /* Set high byte of divisor */
}

/* Sets up the system clock by installing the timer handler
 *  into IRQ0 */
void init_timer(int freq)
{
	timer_phase(freq);
	/* Installs 'timer_handler' to IRQ0 */
	irq_install_handler(IRQ0, timer_handler);
}

void wait_ms(int ms)
{
	volatile int start = timer_ticks;
	start += (ms / 10);
	while (start > timer_ticks)
		;
}
