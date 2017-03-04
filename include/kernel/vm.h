#ifndef PAGING_H
#define PAGING_H

#include <isr.h>

#define PGSIZE 		4096
#define PGSHIFT		12
#define PTXSHIFT	12
#define PDXSHIFT	22

#define KERNBASE 	0xC0000000
#define KERNLINK	0xC0100000
#define PHYS_RAM	0x02000000
#define PTE_P 		0x001
#define PTE_W 		0x002
#define PTE_PS 		0x080

typedef uint32_t pte_t;

typedef struct {
	pte_t *ptes[1024];
} pde_t;

typedef struct {
	/* Array of pointers to page directories */
	pde_t *pdes[1024];
} pd_t;

extern pd_t *current_pd;
extern pd_t kernel_pd;

#define P2V(phys) \
	(void *) ((uintptr_t) (phys) + KERNBASE)

#define V2P(virt) \
	(void *) ((uintptr_t) (virt) - KERNBASE)

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

pd_t *clone_directory(pd_t *src);

/* Virtual to physical conversion */
void *virt_to_phys(void *addr);

#endif /* PAGING_H */
