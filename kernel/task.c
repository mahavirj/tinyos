#include <task.h>
#include <kmalloc.h>
#include <vga.h>
#include <wait_queue.h>

static wq_handle wq_h;

int create_init_task(void (*fn_ptr)(void))
{
	struct task *init = (struct task *) kcalloc(sizeof(struct task));
	if (!init) {
		printk("malloc failed\n");
		return -1;
	}
	int ret = wait_queue_init(&wq_h);
	if (ret != 0) {
		printk("wq init failed\n");
		return ret;
	}

	init->eip = (uint32_t) fn_ptr;
	init->id = 0;
	wait_queue_insert(wq_h, &init->next);
	return 0;
}
