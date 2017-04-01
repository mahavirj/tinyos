#ifndef __TASK_H__
#define __TASK_H__

#include <list.h>
#include <stdint.h>

#define STACK_SIZE 4096

enum task_state {
	TASK_RUNNING = 2,
	TASK_READY = 3,
	TASK_SLEEPING = 4,
	TASK_EXITED = 5,
};

/* Callee saved register context */
struct context {
	uint32_t r4, r5, r6, r7, r8, r9, r10, r11, lr;
};

typedef struct {
	uint32_t r0, r1, r2, r3, r12, lr, pc, xpsr;
} registers_t;

/* Task control block */
struct task {
	int id;                  // Process ID
	int state;		 // State of task, running, blocked etc.
	uint8_t *kstack_base;	 // Kernel stack base
	uint8_t *kstack;	 // Kernel stack
	void *wait_resource;	 // Opaque reference to waiting resource
	registers_t *irqf;       // Registers context saved in irq
	struct context *context; // Callee saved register context
	struct task *parent;	 // Parent Task
	list_head_t next;        // The next task in a linked list
};

extern struct task init;

#endif /* __TASK_H__ */
