#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <task.h>

void next_to_schedule();
void idle_loop();
void task_delete(struct task *new_task);
extern list_head_t *task_list;
extern struct task *current_task;

#endif /* __SCHED_H__ */
