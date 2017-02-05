#include <wait_queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <kmalloc.h>

int wait_queue_deinit(wq_handle *handle)
{
	if (!handle || !*handle)
		return -EINVAL;
	kfree((void *)*handle);
	*handle = 0;
	return 0;
}

int wait_queue_init(wq_handle *handle)
{
	if (!handle)
		return -EINVAL;
	wq_t *w = (wq_t *) kmalloc(sizeof(wq_t));
	if (!w)
		return -ENOMEM;
	
	w->obj_cnt = 0;
	INIT_LIST_HEAD(&w->list);

	*handle = (wq_handle) w;
	return 0;
}

int wait_queue_insert(wq_handle handle, list_head_t *node)
{
	if (!handle || !node)
		return -EINVAL;
	wq_t *w = (wq_t *) handle;

	list_add_tail(node, &w->list);
	w->obj_cnt++;
	return 0;
}

list_head_t *wait_queue_remove(wq_handle handle)
{
	list_head_t *node;

	if (!handle)
		return NULL;
	wq_t *w = (wq_t *) handle;
	
	if (list_empty(&w->list))
		return NULL;

	w->obj_cnt--;
	list_for_each(node, &w->list) {
		list_del(node);
		return node;
	}
	return NULL;
}

int wait_queue_objects(wq_handle handle)
{
	if (!handle)
		return -EINVAL;
	wq_t *w = (wq_t *) handle;
	return w->obj_cnt;
}
