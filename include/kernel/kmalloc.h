#ifndef __KMALLOC_H__
#define __KMALLOC_H__

#include <stdint.h>
#include <stddef.h>

void *kmalloc_page(size_t size);
void *kmalloc(size_t size);
void kfree(void *addr);
void kfree_page(void *addr);
int mem_init(void *addr, size_t size);

#endif /* __KMALLOC_H__ */
