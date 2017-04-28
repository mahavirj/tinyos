#include <sync.h>
#include <ARMCM3.h>

void spin_lock(void *locked)
{
	(void) locked;
	__disable_irq();
}

void spin_unlock(void *locked)
{
	(void) locked;
	__enable_irq();
}
