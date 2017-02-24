
#include <vga.h>
#include <kmalloc.h>
#include <stdint.h>
#include <string.h>
#include <paging.h>

page_directory_t *current_pd;

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

void switch_pgdir(void *pg_dir)
{
	asm volatile("mov %0, %%cr3":: "r"(pg_dir));
	current_pd = phys_to_virt(pg_dir);
}

page_directory_t kernel_pd __attribute__((aligned(4096)));
static page_table_t kernel_pt __attribute__((aligned(4096)));

void init_paging()
{
	/* Map kernel code, data and heap regions from 0 physical to 3G
	 * virtual with size 4M */
	uint32_t phys, i;
	for (phys = 0, i = 0; phys < 0x400000; phys += 4096, i++)
		kernel_pt.pages[i] = phys | PTE_P;

	kernel_pd.tables[768] = (page_table_t *)
			 ((uintptr_t) virt_to_phys(&kernel_pt) | 0x1);

	irq_install_handler(14, page_fault);
	switch_pgdir(virt_to_phys(&kernel_pd));
}

page_directory_t *clone_directory(page_directory_t *src)
{
	page_directory_t *kern_pd = &kernel_pd;
	page_directory_t *new_pd = kcalloc_page(sizeof(page_directory_t));
	if (!new_pd) {
		printk("%s: allocation failure\n", __func__);
		return NULL;
	}

	int i, j;
	for (i = 0; i < 1024; i++) {
		if (kern_pd->tables[i] == src->tables[i]) {
			new_pd->tables[i] = src->tables[i];
		} else {
			page_table_t *virt_pt = kcalloc_page(sizeof(page_table_t));
			if (virt_pt) {
				printk("%s: allocation failure\n", __func__);
				return NULL;
			}
			new_pd->tables[i] = (page_table_t *) ((uintptr_t) virt_to_phys(virt_pt) | PTE_P);
			page_table_t *src_pt = (page_table_t *) ((uintptr_t) src->tables[i] & ~(0xfff));
			for (j = 0; j < 1024; j++) {
				if (!src->tables[i]->pages[j])
					continue;
				uint32_t virt_pg = (uint32_t) kcalloc_page(0x1000);
				if (!virt_pg) {
					printk("%s: allocation failure\n", __func__);
					return NULL;
				}
				printk("MJ %x\n", virt_pg);
				memcpy((void *) virt_pg,
					 phys_to_virt((void *) (src_pt->pages[j] & ~(0xfff))), 0x1000);
				virt_pt->pages[j] = (uintptr_t) virt_to_phys((void *) virt_pg) |
						(src_pt->pages[j] & 0xfff);
			}
		}
	}

	return new_pd;
}
