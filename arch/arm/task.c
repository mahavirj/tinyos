#include <task.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <console.h>
#include <ARMCM3.h>
#include <sched.h>

#define CTRL_REG	(*((volatile uint32_t *) 0xe000ed04))
#define PENDSV_BIT	(1UL << 28UL)

static struct task *init_task;

/* PID of task, will be useful in fork */
static int pid;

void SVC_Handler(void)
{
	/* FIXME: msp can be reset to original value here */
	__asm volatile("mov r0, %0 \n"
			/* Load user saved context */
			"ldmia r0!, {r4-r11} \n"
			/* POP LR in dummy register */
			"ldmia r0!, {r1} \n"
			/* Set PSP from where exception frame will be popped */
			"msr psp, r0 \n"
			/* Make sure will return to thread mode with PSP */
			"orr lr, #0xd \n"
			"bx lr \n"
			: : "r" (init_task->context)
		      );
}

void PendSV_Handler()
{
	__asm volatile("mov r2, %0 \n"
			/* Load current_task address */
			"ldr r1, [r2] \n"
			/* Get context address from current_task */
			"add r1, %1 \n"
			/* Get current PSP */
			"mrs r0, psp \n"
			/* Store current task context */
			"stmdb r0!, {r4-r11, lr} \n"
			/* Store current stack to current context */
			"str r0, [r1] \n"
			/* Save R2 holding base address of current_task */
			"push {r2} \n"
			/* Call scheduling code, that will update current_task */
			"cpsid i \n"
			"isb \n"
			"bl next_to_schedule \n"
			"cpsie i\n"
			"isb \n"
			/* Load base address of current_task */
			"pop {r2} \n"
			/* Load current_task address */
			"ldr r1, [r2] \n"
			/* Get context address from current_task */
			"add r1, %1 \n"
			/* Get actual context stack location for current_task */
			"ldr r0, [r1] \n"
			/* Pop context from thread stack */
			"ldmia r0!, {r4-r11, lr} \n"
			/* Update new PSP */
			"msr psp, r0 \n"
			"bx lr \n"
			: : "r" (&current_task),
				"i" (&((struct task *)0)->context)
		      );
}

struct task *alloctask()
{
	/* Allocate task control block */
	struct task *t = (struct task *) calloc(1, sizeof(struct task));
	if (!t) {
		printf("calloc failed\n");
		return NULL;
	}

	/* Allocate task stack, fixed to 4K for now */
	t->kstack = t->kstack_base =(uint8_t *) calloc(1, STACK_SIZE);
	if (!t->kstack) {
		printf("calloc failed\n");
		return NULL;
	}

	/* Setup stack frame as if task had been interrupted by exception.
         */
	t->id = ++pid;
	char *sp = (char *) ((uint32_t) t->kstack + STACK_SIZE);
	sp -= sizeof(*t->irqf);
	t->irqf = (registers_t *) sp;
	memset(t->irqf, 0, sizeof(*t->irqf));
	sp -= sizeof(*t->context);
	t->context = (struct context *) sp;
	t->kstack += STACK_SIZE;

	/* Initialize wait queue with first task */
	list_add_tail(&t->next, task_list);

	return t;
}

struct task *create_task(void *func)
{
	struct task *t = alloctask();
	if (!t) {
		printf("failed to create task\n");
		return NULL;
	}

	/* Set PC and xPSR registers */
	t->irqf->xpsr = 0x01000000;
	/* As per spec. this value should be in ARM mode (last bit zero),
	 * as it will popped out from stack during exception return, but
         * this is not case with FreeRTOS or other OS'es. Qemu does not work
         * without this.
         */
	t->irqf->pc = (uintptr_t) func & ~(0x1);
	t->context->lr = 0xfffffffd;

	t->state = TASK_READY;
	return t;
}

extern void main();
int create_init_task()
{
	int ret;

	/* Initialize task list for scheduling */
	task_list = (list_head_t *) malloc(sizeof(list_head_t));
	if (!task_list) {
		printf("malloc failed\n");
		return -1;
	}
	INIT_LIST_HEAD(task_list);

	init_task = create_task(main);
	if (!init_task) {
		printf("failed to create init task\n");
		return -1;
	}

	return 0;
}

void task_delete(struct task *new_task)
{
	list_del(&new_task->next);
	free(new_task->kstack_base);
	free(new_task);
}

int create_idle_task()
{
	struct task *idle = create_task(idle_loop);
	if (!idle) {
		printf("failed to create idle task\n");
		return -1;
	}

	return 0;
}

void init_scheduler()
{
	if (!init_task) {
		printf("Scheduler invoked before creating task\n");
		return;
	}
	create_idle_task();
	current_task = init_task;
	current_task->state = TASK_RUNNING;
	__asm volatile("cpsie i \n"
			"svc 0 \n"
		      );
}

void yield()
{
	if (current_task && current_task->state == TASK_RUNNING) {
		current_task->state = TASK_READY;
		CTRL_REG = PENDSV_BIT;
	}
}

void sched()
{
	CTRL_REG = PENDSV_BIT;
}
