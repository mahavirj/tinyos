#ifndef __TASK_H__
#define __TASK_H__

#include <paging.h>
#include <list.h>

struct task {
	int id;                // Process ID.
	uint32_t esp, ebp;       // Stack and base pointers.
	uint32_t eip;            // Instruction pointer.
	page_directory_t *pd; // Page directory.
	list_head_t next;     // The next task in a linked list.
};

#endif /* __TASK_H__ */
