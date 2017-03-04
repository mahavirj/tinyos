
#include <stdbool.h>
#include <vga.h>
#include <mem.h>
#include <stdint.h>
#include <string.h>
#include <vm.h>

pd_t *current_pd;

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
	current_pd = P2V(pg_dir);
}

pd_t kernel_pd __attribute__((aligned(4096)));
static pde_t kernel_pde __attribute__((aligned(4096)));
#if 0
extern unsigned etext;

struct kmap_t {
	void *virt;
	void *phys_start;
	void *phys_end;
	int perm;
};

static struct kmap_t kmap[] = {
	{
		.virt = (void *) KERNBASE,
		.phys_start = 0,
		.phys_end = V2P(0x100000),
		.perm = PTE_W,
	},
	{
		.virt = (void *) KERNLINK,
		.phys_start = V2P(KERNLINK),
		.phys_end = V2P(&etext),
		.perm = 0,
	},
	{
		.virt = &etext,
		.phys_start = V2P(&etext),
		.phys_end = (void *) PHYS_RAM,
		.perm = PTE_W,
	},
};

pte_t *walkpgdir(pde_t *pd, const void *virt, bool alloc)
{
	pte_t *pte = pd->ptes[(virt >> 22) & 0x3ff];
	if (!((uintptr_t) pte & PTE_P)) {
		if (alloc) {
			pte = (pte_t *) kcalloc_page(4096);
			if (!pte)
				return NULL;
		} else {
			return NULL;
		}
	}
	uint32_t *page = (uint32_t *) &pte->page[(virt >> 12) & 0x3ff];
}
#endif

void init_paging()
{
	pd_t *init_pd = (pd_t *) kcalloc_page(sizeof(pd_t));
	if (!init_pd) {
		printk("%s: allocation failure\n", __func__);
		return;
	}

	uint32_t phys, i;
	for (phys = 0, i = 0; phys < 0x400000; phys += 4096, i++)
		kernel_pde.ptes[i] = (pte_t *) (phys | PTE_P | PTE_W);

	kernel_pd.pdes[KERNBASE >> PDXSHIFT] = (pde_t *)
			 ((uintptr_t) V2P(&kernel_pde) | PTE_P | PTE_W);

	irq_install_handler(14, page_fault);
	switch_pgdir(V2P(&kernel_pd));
}

pd_t *clone_directory(pd_t *src)
{
	pd_t *kern_pd = &kernel_pd;
	pd_t *new_pd = kcalloc_page(sizeof(pd_t));
	if (!new_pd) {
		printk("%s: allocation failure\n", __func__);
		return NULL;
	}

	int i, j;
	for (i = 0; i < 1024; i++) {
		if (kern_pd->pdes[i] == src->pdes[i]) {
			new_pd->pdes[i] = src->pdes[i];
		} else {
			pde_t *virt_pd = kcalloc_page(sizeof(pde_t));
			if (virt_pd) {
				printk("%s: allocation failure\n", __func__);
				return NULL;
			}
			new_pd->pdes[i] = (pde_t *) ((uintptr_t) V2P(virt_pd) | PTE_P | PTE_W);
			pde_t *src_pd = (pde_t *) ((uintptr_t) src->pdes[i] & ~(0xfff));
			for (j = 0; j < 1024; j++) {
				if (!src->pdes[i]->ptes[j])
					continue;
				uint32_t virt_pg = (uint32_t) kcalloc_page(0x1000);
				if (!virt_pg) {
					printk("%s: allocation failure\n", __func__);
					return NULL;
				}
				memcpy((void *) virt_pg,
					 P2V((uintptr_t) src_pd->ptes[j] & ~(0xfff)), 0x1000);
				virt_pd->ptes[j] = (pte_t *)((uintptr_t) V2P(virt_pg) |
						((uintptr_t) src_pd->ptes[j] & 0xfff));
			}
		}
	}

	return new_pd;
}
