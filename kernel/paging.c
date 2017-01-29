
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

void switch_page_directory(page_directory_t *dir)
{
	asm volatile("mov %0, %%cr3":: "r"(&dir->tablesPhysical));
	uint32_t cr0;
	asm volatile("mov %%cr0, %0": "=r"(cr0));
	cr0 |= 0x80000000; // Enable paging!
	asm volatile("mov %0, %%cr0":: "r"(cr0));
}

extern unsigned end;
void init_paging()
{
	page_directory_t *kernel_pd;
	kernel_pd = (page_directory_t *) kmalloc_page(sizeof(page_directory_t));
	if (!kernel_pd)
		printk("Allocation failure\n");
	memset(kernel_pd, 0, sizeof(page_directory_t));
	kernel_pd->tables[0] = (page_table_t *) kmalloc_page(sizeof(page_table_t));
	if (!kernel_pd->tables[0])
		printk("Allocation failure\n");
	memset(kernel_pd->tables[0], 0 , sizeof(page_table_t));

	uint32_t phys, i;
	for (phys = 0, i = 0; phys < (unsigned) &end; phys += 4096, i++) {
		kernel_pd->tables[0]->pages[i].frame = (phys >> 12);
		kernel_pd->tables[0]->pages[i].present = 1;
	}

	kernel_pd->tablesPhysical[0] = (uintptr_t) kernel_pd->tables[0] | 0x1;
	irq_install_handler(14, page_fault);
	switch_page_directory(kernel_pd);
}
