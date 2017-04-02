#include <sched.h>
#include <stdbool.h>

void next_to_schedule(list_head_t *task_list, struct task **current_task)
{
	struct task *new_task;
	list_head_t *node;
	bool found = false;

	list_for_each(node, task_list) {
		/* Validate task struct */
		new_task = list_entry(node, struct task, next);
		if (!new_task || new_task == *current_task
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
		*current_task = new_task;

	/* Irrespective of scheduler found anything to switch to,
	 * reset current task status.
	 */
	(*current_task)->state = TASK_RUNNING;
}
