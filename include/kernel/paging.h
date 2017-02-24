#ifndef PAGING_H
#define PAGING_H

#include <isr.h>

#define PGSIZE 		4096
#define PGSHIFT		12
#define PTXSHIFT	12
#define PDXSHIFT	22

#define KERNBASE 	0xC0000000
#define PTE_P 		0x001
#define PTE_W 		0x002
#define PTE_PS 		0x080

typedef struct page {
	uint32_t present    : 1;   // Page present in memory
	uint32_t rw         : 1;   // Read-only if clear, readwrite if set
	uint32_t user       : 1;   // Supervisor level only if clear
	uint32_t accessed   : 1;   // Has the page been accessed since last refresh?
	uint32_t dirty      : 1;   // Has the page been written to since last refresh?
	uint32_t unused     : 7;   // Amalgamation of unused and reserved bits
	uint32_t frame      : 20;  // Frame address (shifted right 12 bits)
} page_t;

typedef struct page_table {
	uint32_t pages[1024];
} page_table_t;

typedef struct page_directory {
	/* Array of pointers to pagetables */
	page_table_t *tables[1024];
} page_directory_t;

extern page_directory_t *current_pd;
extern page_directory_t kernel_pd;

static inline void *phys_to_virt(void *phys)
{
	return (void *) ((uintptr_t) phys + KERNBASE);
}

static inline void *virt_to_phys(void *virt)
{
	return (void *) ((uintptr_t) virt - KERNBASE);
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
  Retrieves a pointer to the page required.
  If make == 1, if the page-table in which this page should
  reside isn't created, create it!
 **/
page_t *get_page(uint32_t address, int make, page_directory_t *dir);

/**
  Handler for page faults.
 **/
void page_fault(registers_t *regs);

page_directory_t *clone_directory(page_directory_t *src);

/* Virtual to physical conversion */
void *virt_to_phys(void *addr);

#endif /* PAGING_H */
