/* Tiny mem allocator
 *
 * First fit method for simplicity and less metadata overhead per node
 * Compaction for front and back merge supported in O(1)
 * Malloc/Free complexity of O(n) order for list traversal
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef MEM_DEBUG
#define dbg(...) printf(...)
#else
#define dbg(...) {}
#endif
int printf(const char *fmt, ...);

typedef struct node {
	size_t size;
	struct node *next;
	/*
	 * For best fit case, next would be pointer to next free node as per
	 * size, and previous will also be required for node just previous to
         * ours based on address. This helps to identify merge conditions based
         * on node if free or not, without traversing entire list. Next node
         * based on address could be traced by adding size to current one.
         */
} node_t;

static node_t *start;
static size_t free_size;
const int min_alloc_size = sizeof(node_t);

#define ALLOCATE (1U << 31)

void free(void *addr)
{
	node_t *new;
	node_t *current = start;

	if (!addr)
		return;

	new = (node_t *) ((size_t) addr - sizeof(node_t));
	if (!(new->size & ALLOCATE)) {
		printf("Err...unallocated block\n");
		return;
	}

	dbg("Freeing %p\n", addr);	

	new->size &= ~(ALLOCATE);
	free_size += new->size;

	/* Head null or insert after head condition handling */
	if (current == NULL || new < current) {
		if (((size_t) new + new->size) == (size_t) current) {
			new->size += current->size;
			new->next = current->next;
			start = new;
		} else {
			new->next = current;
			start = new;
		}
	} else {
		while (current->next && (current->next < new))
			current = current->next;

		/* Insert node anyways */
		new->next = current->next;
		current->next = new;

		/* Check for front merge */
		if (((size_t) new + new->size) == (size_t) new->next) {
			new->size += new->next->size;
			new->next = new->next->next;
		}

		/* Check for back merge */
		if (((size_t) current + current->size) == (size_t) new) {
			current->size += new->size;
			current->next = new->next;
		}
	}
}

void *malloc(size_t size)
{
	node_t *current, *prev;

	/* Add block metadata size to incoming size */
	size += sizeof(node_t);

	if (!start) {
		start = sbrk(size);
		if (start == (node_t *) -1) {
			printf("No more mem can be allocated\n");
			return NULL;
		}
		start->size = size;
		start->next = NULL;
		free_size = start->size;
	}

	if (size > free_size) {
		/* Time to query sbrk for increasing system break */
		node_t *tmp = sbrk(size);
		if (tmp == (node_t *) -1) {
			printf("No more mem can be allocated\n");
			return NULL;
		}
		tmp->size = size | ALLOCATE;
		tmp->next = NULL;
		/* Add this node to free list */
		free(tmp + 1);
	}

	current = start;
	prev = current;
	while (current) {
		if (current->size >= size) {
			if (current->size > (size + min_alloc_size)) {
				/* Found large enough block */
				node_t *new = (node_t *) ((size_t) current + size);
				new->size = current->size - size;
				new->next = current->next;
				current->next = new;
			} else {
				/* Size could be more than asked, as per min_alloc_size */
				size = current->size;
			}

			/* Mark as allocated node, size field is used 31st bit */
			current->size = size | ALLOCATE;
			/* See if head needs split */
			if (current == start)
				start = current->next;
			else
				prev->next = current->next;
			free_size -= size;
			dbg("Allocated 0x%x at %p\n", size - sizeof(node_t),
					 (void *) ((size_t) current + sizeof(node_t)));
			return (void *) ((size_t) current + sizeof(node_t));
		}
		prev = current;
		current = current->next;
	}

	dbg("Rquired size unable to allocate %d\n", size - sizeof(node_t));
	return NULL;
}
