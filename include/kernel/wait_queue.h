

#ifndef __WAIT_QUEUE_H__
#define __WAIT_QUEUE_H__

#include <stdint.h>
#include <list.h>

typedef int wq_handle;

typedef struct {
	uint32_t obj_cnt;
	list_head_t list;
} wq_t;

/**
 * Initialize wait queue
 * 
 * @param[out] handle Wait queue handle will be retuned in this
 * @return 0 on SUCCESS
 */
int wait_queue_init(wq_handle *handle);

/**
 * Insert object in wait queue
 * @param[in] handle Wait queue handle which was already created 
 * @param[in] node Object to be inserted in wait queue
 * @return 0 on SUCCESS
 */
int wait_queue_insert(wq_handle handle, list_head_t *node);

/**
 * Remove object from wait queue
 * @param[in] handle Wait queue handle which was already created
 * @return Object removed from wait queue
 */
list_head_t *wait_queue_remove(wq_handle handle);

/**
 * Get number of object waiting in wait queue
 * @return Count of no. of objects
 */
int wait_queue_objects(wq_handle handle);

/**
 * Deinitialize wait queue
 * 
 * @param[in] handle Wait queue handle which was already created
 * @return 0 on SUCCESS
 */
int wait_queue_deinit(wq_handle *handle);

#endif
