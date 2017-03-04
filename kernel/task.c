#include <stdbool.h>
#include <string.h>
#include <task.h>
#include <mem.h>
#include <vga.h>
#include <wait_queue.h>
#include <helper.h>
#include <vm.h>

/* Task list */
static list_head_t *task_list;
/* Current task running, should be in per CPU data */
static struct task *current_task, *init_task;
/* PID of task, will be useful in fork */
static int pid;
/* Current task page directory */
static pd_t *current_pd;

extern void task_ret();

int create_task(void (*fn_ptr)(void))
{
	struct task *t;

	/* Allocate task control block */
	t = (struct task *) kcalloc(sizeof(struct task));
	if (!t) {
		printk("malloc failed\n");
		return -1;
	}

	/* Allocate task stack, fixed to 4K for now */
	t->kstack = (uint8_t *) kcalloc_page(STACK_SIZE);
	if (!t->kstack) {
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
	pd_t *new_pd = setupvm(current_pd);
	if (!new_pd) {
		printk("page dir clone failed\n");
		return -1;
	}

	/* Setup stack frame as if task had been interrupted by exception,
         * with initial return point into `exit` routine of exception
         * handler.
         */
	t->id = ++pid;
	t->pd = V2P(new_pd);
	char *sp = (char *) ((uint32_t) t->kstack + STACK_SIZE);
	sp -= sizeof(*t->irqf);
	t->irqf = (registers_t *) sp;
	memset(t->irqf, 0, sizeof(*t->irqf));
	sp -= sizeof(*t->context);
	t->context = (struct context *) sp;
	memset(t->context, 0, sizeof(*t->context));
	t->context->eip = (uint32_t) task_ret;

	/* FIXME: remove code segment, data segment related hardcodings */
	t->irqf->cs = 0x8;
	t->irqf->ds = 0x10;
	t->irqf->eflags = 0x200;
	t->irqf->esp = (uint32_t) t->kstack + STACK_SIZE;
	t->irqf->eip = (uint32_t) fn_ptr;

	/* Initialize wait queue with first task */
	list_add_tail(&t->next, task_list);

	/* Set state for task */
	t->state = TASK_READY;

	if (!init_task)
		init_task = t;

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
	tiny_schedule();
}

void yield()
{
	if (current_task && current_task->state == TASK_RUNNING) {
		current_task->state = TASK_READY;
		tiny_schedule();
	}
}

void trace_tasks()
{
	list_head_t *node;
	struct task *t;
	cli();
	list_for_each(node, task_list) {
		/* Validate task struct */
		t = list_entry(node, struct task, next);
		if (!t) {
			printk("Err! Task is null\n");
			continue;
		}
		printk("Task pid: %d, state: %d\n", t->id, t->state);
	}
	sti();
}

void init_scheduler()
{
	if (!init_task) {
		printk("Scheduler invoked before creating task\n");
		return;
	}
	current_task = init_task;
	load_context(current_task->context);
}

void tiny_schedule()
{
	struct task *new_task, *prev_task;
	list_head_t *node;
	bool found = false;

	/* Globally disable interrupts, can be called from `yield` */
	cli();

	list_for_each(node, task_list) {
		/* Validate task struct */
		new_task = list_entry(node, struct task, next);
		if (!new_task || new_task->state != TASK_READY)
			continue;

		/* Make this last for round robin */
		list_del(&new_task->next);
		list_add_tail(&new_task->next, task_list);
		found = true;
		break;
	}

	if (found) {
		/* Switch page directory to new task */
		switch_pgdir(new_task->pd);

		/* Set current page directory*/
		current_pd = new_task->pd;

		prev_task = current_task;
		/* Set current task */
		current_task = new_task;

		/* Set task state to running */
		new_task->state = TASK_RUNNING;
		/* Switch context */
		swtch(&prev_task->context, new_task->context);
	}

	/* Globally enable interrupts */
	sti();
}
