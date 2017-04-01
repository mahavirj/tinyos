#include <stdint.h>
#include <task.h>
#include <ARMCM3.h>

static volatile int timer_ticks;

void SysTick_Handler(void)
{
	timer_ticks++;
	yield();
}
	
void timer_init()
{
	volatile uint32_t clock = 1000000;
	SysTick->LOAD = clock - 1;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
			SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk;
}
