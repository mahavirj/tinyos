
#include <vga.h>
#include <kmalloc.h>
#include <stdint.h>
#include <string.h>
#include <paging.h>

void page_fault(registers_t *regs)
{
	// A page fault has occurred.
	// The faulting address is stored in the CR2 register.
	uint32_t faulting_address;
	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

	// The error code gives us details of what happened.
	int present   = !(regs->err_code & 0x1); // Page not present
	int rw = regs->err_code & 0x2;           // Write operation?
	int us = regs->err_code & 0x4;           // Processor was in user-mode?
	int reserved = regs->err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
	// int id = regs->err_code & 0x10;          // Caused by an instruction fetch?

	// Output an error message.
	printk("Page fault @%x\n", faulting_address);
	if (present)
		printk("not present\n");
	if (rw)
		printk("read-only\n");
	if (us)
		printk("user-mode\n");
	if (reserved)
		printk("reserved\n");
} 

// Function to allocate a frame.
void alloc_frame(page_t *page, int is_kernel, int is_writeable)
{
	page->present = 1; // Mark it as present.
	page->rw = (is_writeable) ? 1 : 0; // Should the page be writeable?
	page->user = (is_kernel) ? 0 : 1; // Should the page be user-mode?
}

void switch_page_directory(void *pg_dir)
{
	asm volatile("mov %0, %%cr3":: "r"(pg_dir));
	uint32_t cr0;
	asm volatile("mov %%cr0, %0": "=r"(cr0));
	cr0 |= 0x80000000; // Enable paging!
	asm volatile("mov %0, %%cr0":: "r"(cr0));
}

#define PAGE_PRESENT (0x1)
#define WRITE_MODE   (0x1 << 1)
#define USER_MODE    (0x1 << 2)

static page_table_t kernel_pt __attribute__((aligned(4096)));
static page_table_t kmem_pt __attribute__((aligned(4096)));
static page_directory_t kernel_pd;
extern unsigned end;

void init_paging()
{
	/* Identity mapping for kernel and low memory area */
	uint32_t phys, i;
	for (phys = 0, i = 0; phys < (unsigned) &end; phys += 4096, i++) {
		kernel_pt.pages[i].frame = (phys >> 12);
		kernel_pt.pages[i].present = PAGE_PRESENT;
	}
	kernel_pd.tables[0] = (page_table_t *) ((uintptr_t) &kernel_pt | 0x1);

	/* Heap region reserved for kmalloc */
	uint32_t end_4k = ((uintptr_t) &end + 0xfff) & ~(0xfffU);
	for (i = 0; i < 1024; i++, end_4k += 4096) {
		kmem_pt.pages[i].frame = (end_4k >> 12);
		kmem_pt.pages[i].present = PAGE_PRESENT;
	}
	kernel_pd.tables[768] = (page_table_t *) ((uintptr_t) &kmem_pt | 0x1);

	/* Last page table entry points to page table itself for getting physical addresses */
	kernel_pd.tables[1023] = (page_table_t *) ((uintptr_t) &kernel_pd.tables | 0x1);

	irq_install_handler(14, page_fault);
	switch_page_directory(&kernel_pd);
}

void *virt_to_phys(void *virt)
{
	int pdi = (uint32_t) virt >> 22;
	int pti = ((uint32_t) virt >> 12) & 0x3ff;

	/* Last entry in page directory */
	uint32_t *pde = (uint32_t *) 0xfffff000;
	if ((*pde != 0) && (*pde & 0x1)) {
		uint32_t *pte = (uint32_t *) (0xffc00000 + (0x1000 * pdi));
		if ((*pte != 0))
			return (void *) ((pte[pti] & ~(0xfff)) |
						 ((uint32_t) virt & 0xfff));
	}
	/* FIXME: is it correct return value in failure */
	return NULL;
}

void clone_directory()
{
	page_directory_t *new_pd = kmalloc_page(sizeof(page_directory_t));
	if (!new_pd) {
		printk("%s: allocation failure\n", __func__);
		return;
	}
	memcpy(new_pd, &kernel_pd, sizeof(page_directory_t));
	int *tmp = virt_to_phys(new_pd);
	switch_page_directory(tmp);
}
