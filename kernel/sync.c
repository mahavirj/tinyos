#include <helper.h>
#include <sync.h>

void acquire(struct spinlock *lock)
{
	spin_lock(&lock->locked);
}

void release(struct spinlock *lock)
{
	spin_unlock(&lock->locked);
}
