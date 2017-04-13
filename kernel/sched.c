#include <sched.h>
#include <stdbool.h>
#include <arch.h>
#include <console.h>

/* Task list */
list_head_t *task_list;
/* Current task running, should be in per CPU data */
struct task *current_task;

void idle_loop()
{
	while (1) {
		struct task *new_task;
		list_head_t *node, *_node;

		arch_disable_irq();
		/* Safer version for list traversal, as iterator might get
		 * modified */
		list_for_each_safe(node, _node, task_list) {
			/* Validate task struct */
			new_task = list_entry(node, struct task, next);
			if (!new_task || new_task->state != TASK_EXITED)
				continue;

			/* Free up all dynamic memory allocated */
			task_delete(new_task);

		}
		arch_enable_irq();

		/* Allowed following instruction in previlege mode only */
		arch_halt();
	}
}

void next_to_schedule()
{
	struct task *new_task;
	list_head_t *node;
	bool found = false;

	list_for_each(node, task_list) {
		/* Validate task struct */
		new_task = list_entry(node, struct task, next);
		if (!new_task || new_task == current_task
					 || new_task->state != TASK_READY)
			continue;

		/* Make this last for round robin */
		list_del(&new_task->next);
		list_add_tail(&new_task->next, task_list);
		found = true;
		break;
	}

	if (found)
		/* Set current task */
		current_task = new_task;

	/* Irrespective of scheduler found anything to switch to,
	 * reset current task status.
	 */
	current_task->state = TASK_RUNNING;
}

void task_sleep(void *resource)
{
	if (!resource)
		return;

	arch_disable_irq();
	current_task->wait_resource = resource;
	current_task->state = TASK_SLEEPING;
	arch_enable_irq();
}

void task_wakeup(void *resource)
{
	if (!resource)
		return;

	arch_disable_irq();
	list_head_t *node;
	list_for_each(node, task_list) {
		/* Get task struct */
		struct task *t = list_entry(node, struct task, next);
		if (!t || t->wait_resource != resource)
			continue;
		t->wait_resource = NULL;
		t->state = TASK_READY;
	}
	arch_enable_irq();
}

void trace_tasks()
{
	list_head_t *node;
	struct task *t;

	arch_disable_irq();
	printk("#### Task list ####\n");
	list_for_each(node, task_list) {
		/* Validate task struct */
		t = list_entry(node, struct task, next);
		if (!t) {
			printk("Err! Task is null\n");
			continue;
		}
		printk("Task pid: %d, state: %d\n", t->id, t->state);
	}
	arch_enable_irq();
}
