#include <string.h>
#include <task.h>
#include <kmalloc.h>
#include <vga.h>
#include <wait_queue.h>
#include <helper.h>
#include <paging.h>

/* Task list */
static list_head_t *task_list;
/* Current task running, should be in per CPU data */
static struct task *current_task;
/* Per CPU scheduler context */
static struct cpu *cpu;
/* PID of task, will be useful in fork */
static int pid;

extern void task_ret();

int create_task(void (*fn_ptr)(void))
{
	struct task *init;

	/* Allocate task control block */
	init = (struct task *) kcalloc(sizeof(struct task));
	if (!init) {
		printk("malloc failed\n");
		return -1;
	}

	/* Allocate task stack, fixed to 4K for now */
	init->kstack = (uint8_t *) kcalloc_page(4096);
	if (!init->kstack) {
		printk("malloc failed\n");
		return -1;
	}

	/* Initialize wait queue if not already */
	if (!task_list) {
		task_list = (list_head_t *) kmalloc(sizeof(list_head_t));
		if (!task_list) {
			printk("malloc failed\n");
			return -1;
		}
		INIT_LIST_HEAD(task_list);
	}

	/* Clone page directory, kernel code/heap is linked (not copied),
	 * rest copied */
	page_directory_t *new_dir = clone_directory(current_pd);
	if (!new_dir) {
		printk("page dir clone failed\n");
		return -1;
	}

	/* Setup stack frame as if task had been interrupted by exception,
         * with initial return point into `exit` routine of exception
         * handler.
         */
	init->id = ++pid;
	init->pd = virt_to_phys(new_dir);
	char *sp = (char *) ((uint32_t) init->kstack + 4096);
	sp -= sizeof(*init->irqf);
	init->irqf = (registers_t *) sp;
	memset(init->irqf, 0, sizeof(*init->irqf));
	sp -= sizeof(*init->context);
	init->context = (struct context *) sp;
	memset(init->context, 0, sizeof(*init->context));
	init->context->eip = (uint32_t) task_ret;

	/* FIXME: remove code segment, data segment related hardcodings */
	init->irqf->cs = 0x8;
	init->irqf->ds = 0x10;
	init->irqf->eflags = 0x200;
	init->irqf->esp = (uint32_t) init->kstack + 4096;
	init->irqf->eip = (uint32_t) fn_ptr;

	/* Initialize wait queue with first task */
	list_add(&init->next, task_list);

	/* Set state for task */
	init->state = TASK_READY;

	return 0;
}

void task_sleep(void *resource)
{
	cli();
	current_task->wait_resource = resource;
	current_task->state = TASK_SLEEPING;
	sti();
}

void task_wakeup(void *resource)
{
	cli();
	list_head_t *node;
	list_for_each(node, task_list) {
		/* Get task struct */
		struct task *t = list_entry(node, struct task, next);
		if (!t || t->wait_resource != resource)
			continue;
		t->wait_resource = NULL;
		t->state = TASK_READY;
	}
	sti();
}

void sched()
{
	swtch(&current_task->context, cpu->context);
}

void yield()
{
	if (current_task && current_task->state == TASK_RUNNING) {
		current_task->state = TASK_READY;
		swtch(&current_task->context, cpu->context);
	}
}

void tiny_scheduler()
{
	/* Scheduler context */
	cpu = (struct cpu *) kcalloc(sizeof(struct cpu));
	if (!cpu)
		return;

	for (;;) {
		list_head_t *node;
		list_for_each(node, task_list) {
			/* Get task struct */
			struct task *t = list_entry(node, struct task, next);
			if (!t || t->state != TASK_READY)
				continue;
			/* Switch page directory to new task */
			switch_pgdir(t->pd);
			/* Set current task */
			current_task = t;
			/* Set task state to running */
			t->state = TASK_RUNNING;
			/* Switch context */
			swtch(&cpu->context, t->context);
			cli();
			current_task = NULL;
			/* Switch page directory to scheduler code */
			switch_pgdir(virt_to_phys(&kernel_pd));
		}
	}
}
