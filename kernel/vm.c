
#include <stdbool.h>
#include <vga.h>
#include <mem.h>
#include <stdint.h>
#include <string.h>
#include <helper.h>
#include <vm.h>

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
	for (;;)
		;
} 

static struct kmap_t kmap[] = {
	/* BIOS and other memory mapped device addres space */
	{
		.virt = (void *) KERNBASE,
		.phys_start = 0,
		.phys_end = (void *) 0x100000,
		.perm = PTE_W,
	},
	/* Kernel TEXT/RODATA, read only mapping */
	{
		.virt = (void *) KERNLINK,
		.phys_start = V2P(KERNLINK),
		.phys_end = V2P(&etext),
		.perm = 0,
	},
	/* Kernel DATA/HEAP, rw mapping */
	{
		.virt = &etext,
		.phys_start = V2P(&etext),
		.phys_end = (void *) PHYS_RAM,
		.perm = PTE_W,
	},
};

static pte_t **pte_walk(pd_t *pd, const void *virt, bool alloc)
{
	pde_t *pde = pd->pdes[PDINDEX(virt)];
	if (!((uintptr_t) pde & PTE_P)) {
		if (alloc) {
			pde = (pde_t *) kcalloc_page(sizeof(pde_t));
			if (!pde)
				return NULL;
			pd->pdes[PDINDEX(virt)] =
				(pde_t *)
				((uintptr_t) V2P(pde) | PTE_P | PTE_W | PTE_U);
		} else {
			return NULL;
		}
	} else {
		pde = P2V(PADDR(pde));
	}
	return &pde->ptes[PTINDEX(virt)];
}

static int map_pages(pd_t *pd, const void *virt, void *phys,
			int size, int perm)
{
	int i;
	pte_t **pte;

	/* Align addresses to nearest page boundary */
	char *v = (char *) virt;
	char *p = (char *) PADDR(phys);
	size += (PGSIZE - 1);
	size &= ~(PGSIZE - 1);

	for (i = 0; i < size; i += PGSIZE, p += PGSIZE, v += PGSIZE) {
		pte = pte_walk(pd, v, true);
		if (*pte) {
			printk("Err...PTE already mapped\n");
			return -1;
		}
		*pte = (pte_t *) ((uintptr_t) p | PTE_P | perm);
	}
	return 0;
}

static void init_kernel_mappings(pd_t *pd)
{
	if (!pd)
		return;

	int i;
	for (i = 0; i < ARR_SIZE(kmap); i++) {
		size_t size = (uintptr_t) kmap[i].phys_end -
				(uintptr_t) kmap[i].phys_start;
		map_pages(pd, kmap[i].virt, kmap[i].phys_start,
				size, kmap[i].perm);
	}
}

void init_paging()
{
	pd_t *init_pd = (pd_t *) kcalloc_page(sizeof(pd_t));
	if (!init_pd) {
		printk("%s: allocation failure\n", __func__);
		return;
	}

	init_kernel_mappings(init_pd);
	irq_install_handler(14, page_fault);
	switch_pgdir(V2P(init_pd));
}

extern unsigned _bin_userapp_end, _bin_userapp_start;
pd_t *setupvm(pd_t *src)
{
	int i;
	pd_t *new_pd = kcalloc_page(sizeof(pd_t));
	if (!new_pd) {
		printk("%s: allocation failure\n", __func__);
		return NULL;
	}
	/* Kernel mode mappings, only linking no clone */
	init_kernel_mappings(new_pd);

	if (!src) {
		map_pages(new_pd, (void *) 0, (void *) V2P(&_bin_userapp_start),
			(unsigned) &_bin_userapp_end - (unsigned) &_bin_userapp_start, PTE_W | PTE_U);
		return new_pd;
	}

	/* We will assume that user mode vma for process would be
	 * well within first 4M boundary, just to simplify clone
	 * process.
	 */
	pde_t *pde = src->pdes[0];
	if ((uintptr_t) pde & PTE_P) {
		/* This will allocate PTE table */
		pte_walk(new_pd, (void *) 0, 1);
		pde = P2V(PADDR(pde));
		for (i = 0; i < 1024; i++) {
			pte_t *pte = pde->ptes[i];
			pte_t **d = pte_walk(new_pd,
				(void *) (i << PTXSHIFT), 0);
			if ((uintptr_t) pte & PTE_P) {
				/* We need to copy 4K page here */
				char *page = kcalloc_page(PGSIZE);
				if (!page) {
					printk("malloc failed\n");
					return NULL;
				}
				pte = P2V(PADDR(pte));
				memcpy(page, pte, PGSIZE);
				*d = (pte_t *) ((uintptr_t) V2P(page)
						| PTE_P | PTE_W | PTE_U);
			}
		}
	}
	return new_pd;
}
