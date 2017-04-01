#include <stdint.h>

extern char end[];
void *sbrk(int size)
{
	/* FIXME: check for upper limit */
	static void *limit = end;

	if (size)
		limit = (void *)((uintptr_t) limit + size);

	return limit;
}
