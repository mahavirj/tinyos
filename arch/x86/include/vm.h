#ifndef __VM_H__
#define __VM_H__

#include <isr.h>
#include <stddef.h>

#define PGSIZE 		4096
#define PGSHIFT		12
#define PTXSHIFT	12
#define PDXSHIFT	22

#define HEAPBASE 	0x80000000
#define KERNBASE 	0xC0000000
#define KERNLINK	0xC0100000
#define PHYS_RAM	0x02000000
#define SBRK_LIMIT	(0x00400000 - (2 * STACK_SIZE))
#define PTE_P 		0x001
#define PTE_W 		0x002
#define PTE_U 		0x004
#define PTE_PS 		0x080

#define PDINDEX(x) \
	(((uintptr_t) (x) >> 22) & (0x3ff))

#define PTINDEX(x) \
	(((uintptr_t) (x) >> 12) & (0x3ff))

#define PADDR(x) \
	((uintptr_t) (x) & ~(0xfff))

#define P2V(phys) \
	(void *) ((uintptr_t) (phys) + KERNBASE)

#define V2P(virt) \
	(void *) ((uintptr_t) (virt) - KERNBASE)

#define PG_ALIGN(x) \
		(!(((uintptr_t) (x) & (0xfff))))

#define PG_OFFSET(x) \
		((uintptr_t) (x) & (0xfff))

typedef uint32_t pte_t;

typedef struct {
	/* Array of pointers to ptes */
	pte_t *ptes[1024];
} pde_t;

typedef struct {
	/* Array of pointers to pdes */
	pde_t *pdes[1024];
} pd_t;

extern unsigned etext;

struct kmap_t {
	void *virt;
	void *phys_start;
	void *phys_end;
	int perm;
};

static inline void switch_pgdir(void *pg_dir)
{
	asm volatile("mov %0, %%cr3":: "r"(pg_dir));
}

/**
  Sets up the environment, page directories etc and
  enables paging.
 **/
void init_paging();

/**
  Causes the specified page directory to be loaded into the
  CR3 register.
 **/
void switch_pgdir(void *pg_dir);

/**
  Handler for page faults.
 **/
void page_fault(registers_t *regs);

/** Sets up kernel memory mappings in page directory */
pd_t *setupkvm();

/** Clones parent process tables for user space to child */
int cloneuvm(pd_t *new_pd, pd_t *current_pd);

/** Allocates new memory mapping for given address in virtual space */
int allocuvm(pd_t *new_pd, void *virt, int size);

/** Loads data to newly allocated virtual memory address range */
int loaduvm(pd_t *new_pd, void *virt, void *data, size_t size);

/** Deallocates entire virtual address space and frees up page directory */
int deallocvm(pd_t *pd);

#endif /* __VM_H__ */
