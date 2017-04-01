#include <sync.h>
#include <ARMCM3.h>

void spin_lock(void *locked)
{
	__disable_irq();
}

void spin_unlock(void *locked)
{
	__enable_irq();
}
