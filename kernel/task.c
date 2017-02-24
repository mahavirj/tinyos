#include <string.h>
#include <task.h>
#include <kmalloc.h>
#include <vga.h>
#include <wait_queue.h>
#include <helper.h>
#include <paging.h>

enum task_state {
	TASK_BLOCKED = 1,
	TASK_RUNNING = 2,
	TASK_READY = 3,
};

/* FIXME: wait queue used for holding tasks in ready queue */
static wq_handle wq_h;
/* Current task running, should be in per CPU data */
struct task *current_task;
/* Per CPU scheduler context */
struct cpu cpu1;
struct cpu *cpu = &cpu1;
/* PID of task, will be useful in fork */
static int pid;

extern void task_ret();

int create_task(void (*fn_ptr)(void))
{
	int ret;
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
	if (!wq_h) {
		ret = wait_queue_init(&wq_h);
		if (ret != 0) {
			printk("wq init failed\n");
			return ret;
		}
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
	wait_queue_insert(wq_h, &init->next);

	/* Set state for task */
	cli();
	init->state = TASK_READY;
	sti();

	return 0;
}

void tiny_scheduler()
{
	for (;;) {
		sti();
		/* Remove first entry from wait queue */
		list_head_t *node = wait_queue_remove(wq_h);
		if (!node)
			/* PANIC, Looks like init task not yet created */
			return;

		/* Insert entry to tail, assuming round robin */
		wait_queue_insert(wq_h, node);

		/* Get task struct */
		struct task *t = list_entry(node, struct task, next);
		if (t->state != TASK_READY)
			continue;
		/* Set current task */
		current_task = t;

		/* Switch page directory to new task */
		switch_pgdir(t->pd);
		/* Switch context */
		swtch(&cpu->context, t->context);
		/* Switch page directory to scheduler code */
		switch_pgdir(virt_to_phys(&kernel_pd));
	}
}
