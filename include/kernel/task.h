#ifndef __TASK_H__
#define __TASK_H__

#include <paging.h>
#include <list.h>

enum task_state {
	TASK_RUNNING = 2,
	TASK_READY = 3,
	TASK_SLEEPING = 4,
};

/* Callee saved register context */
struct context {
	uint32_t edi;
	uint32_t esi;
	uint32_t ebx;
	uint32_t ebp;
	uint32_t eip;
};

/* Task control block */
struct task {
	int id;                  // Process ID.
	int state;		 // State of task, running, blocked etc.
	uint8_t *kstack;	 // Kernel stack
	void *wait_resource;	 // Opaque reference to waiting resource
	registers_t *irqf;       // Registers context saved in irq
	struct context *context; // Callee saved register context
	page_directory_t *pd;    // Page directory.
	list_head_t next;        // The next task in a linked list.
};

/* Per CPU scheduler data */
struct cpu {
	struct context *context;
};

#define STACK_SIZE 4096

void trace_tasks();
void init_scheduler();
void tiny_schedule(void);
int create_task(void (*fn_ptr)(void));
void swtch(struct context **old, struct context *new);
void yield();
void sched();
void load_context(struct context *new);
void task_sleep(void *resource);
void task_wakeup(void *resource);

#endif /* __TASK_H__ */
