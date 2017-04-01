#include <sync.h>
#include <task.h>

void acquire(struct spinlock *lock)
{
	spin_lock(&lock->locked);
}

void release(struct spinlock *lock)
{
	spin_unlock(&lock->locked);
}

void sleep(void *resource, struct spinlock *lock)
{
	task_sleep(resource);
	release(lock);
	sched();
	acquire(lock);
}

void wakeup(void *resource)
{
	task_wakeup(resource);
}
