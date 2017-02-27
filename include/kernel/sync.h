#ifndef __SYNC_H__
#define __SYNC_H__

struct spinlock {
	int locked;
};

void acquire(struct spinlock *lock);
void release(struct spinlock *lock);

#endif /* __SYNC_H__ */
