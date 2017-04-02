#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <task.h>

void next_to_schedule(list_head_t *task_list, struct task **current_task);

#endif /* __SCHED_H__ */
