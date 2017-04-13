
#include <stdbool.h>
#include <vga.h>
#include <mem.h>
#include <stdint.h>
#include <string.h>
#include <helper.h>
#include <vm.h>
#include <errno.h>

void page_fault(registers_t *regs)
{
	// A page fault has occurred.
	// The faulting address is stored in the CR2 register.
	uint32_t faulting_address;
	__asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

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
	size = ALIGN(size, PGSIZE);

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
	int i;
	for (i = 0; i < ARR_SIZE(kmap); i++) {
		size_t size = (uintptr_t) kmap[i].phys_end -
				(uintptr_t) kmap[i].phys_start;
		map_pages(pd, kmap[i].virt, kmap[i].phys_start,
				size, kmap[i].perm);
	}
}

pd_t *setupkvm()
{
	pd_t *new_pd = kcalloc_page(sizeof(pd_t));
	if (!new_pd) {
		printk("%s: allocation failure\n", __func__);
		return NULL;
	}
	/* Kernel mode mappings, only linking no clone */
	init_kernel_mappings(new_pd);

	return new_pd;
}

int cloneuvm(pd_t *new_pd, pd_t *src)
{
	int i;

	/* We will assume that user mode vma for process would be
	 * well within first 4M boundary, just to simplify clone
	 * process.
	 */
	pde_t *pde = src->pdes[0];
	if ((uintptr_t) pde & PTE_P) {
		/* This will allocate PTE table */
		pte_walk(new_pd, (void *) 0, true);
		pde = P2V(PADDR(pde));
		for (i = 0; i < 1024; i++) {
			pte_t *pte = pde->ptes[i];
			pte_t **d = pte_walk(new_pd,
				(void *) (i << PTXSHIFT), false);
			if ((uintptr_t) pte & PTE_P) {
				/* We need to copy 4K page here */
				char *page = kcalloc_page(PGSIZE);
				if (!page) {
					printk("malloc failed\n");
					return -1;
				}
				pte = P2V(PADDR(pte));
				memcpy(page, pte, PGSIZE);
				*d = (pte_t *) ((uintptr_t) V2P(page)
						| PTE_P | PTE_W | PTE_U);
			}
		}
	}
	return 0;
}

int allocuvm(pd_t *new_pd, void *virt, int size)
{
	int i;
	/* Align size to page boundary */
	size = ALIGN(size, PGSIZE);

	for (i = 0; i < size; i += PGSIZE, virt = PTRINC(virt, PGSIZE)) {
		pte_t **pte = pte_walk(new_pd, virt, true);
		if (!pte) {
			printk("page table walk failed\n");
			return -1;
		} else if ((uintptr_t) *pte & PTE_P) {
			continue;
		}
		void *page = kcalloc_page(PGSIZE);
		if (!page) {
			printk("malloc failed\n");
			return -1;
		}

		*pte = (pte_t *) ((uintptr_t) V2P(page) |
					PTE_P | PTE_W | PTE_U);
	}
	return 0;
}

int loaduvm(pd_t *new_pd, void *virt, void *data, size_t size)
{
	size_t copy_size;
	size_t curr_size = 0;
	int offset;

	if (!new_pd || !data)
		return -EINVAL;

	copy_size = PGSIZE;
	while (curr_size < size) {
		if (!PG_ALIGN(virt)) {
			/* Virtual address is not aligned, set offset */
			offset = PG_OFFSET(virt);
		} else {
			offset = 0;
		}

		copy_size -= offset;
		if ((size - curr_size) < copy_size)
			copy_size = (size - curr_size);

		pte_t **pte = pte_walk(new_pd, virt, false);
		if (!*pte) {
			printk("pte doesn't exists\n");
			return -1;
		}

		void *src_addr = P2V(PADDR(*pte));
		src_addr = PTRINC(src_addr, offset);
		memcpy(src_addr, data, copy_size);

		data = PTRINC(data, copy_size);
		curr_size += copy_size;
		virt = PTRINC(virt, copy_size);
	}
	return 0;
}

int deallocvm(pd_t *pd)
{
	int i;
	pde_t *pde;

	for (i = (KERNBASE >> PDXSHIFT); i < 1024; i++) {
		pde = pd->pdes[i];
		if ((uintptr_t) pde & PTE_P)
			kfree_page(P2V(PADDR(pde)));
	}

	/* We will assume that user mode vma for process would be
	 * well within first 4M boundary, just to simplify clone
	 * process.
	 */
	pde = pd->pdes[0];
	if ((uintptr_t) pde & PTE_P) {
		pde = P2V(PADDR(pde));
		for (i = 0; i < 1024; i++) {
			pte_t *pte = pde->ptes[i];
			if ((uintptr_t) pte & PTE_P)
				kfree_page(P2V(PADDR(pte)));
		}
		kfree_page(pde);
	}
	kfree_page(pd);
	return 0;
}

void init_paging()
{
	pd_t *init_pd = setupkvm();
	if (!init_pd) {
		printk("%s: setupkvm failure\n", __func__);
		return;
	}

	printk("Kernel Memory Map:\n");
	int i;
	for (i = 0; i < ARR_SIZE(kmap); i++) {
		size_t size = (uintptr_t) kmap[i].phys_end -
				(uintptr_t) kmap[i].phys_start;
		printk("[virt: %p] [phy: %p] [size: %x]\n",
				kmap[i].virt, kmap[i].phys_start, size);
	}

	irq_install_handler(14, page_fault);
	switch_pgdir(V2P(init_pd));
}

__attribute__((aligned(PGSIZE))) pd_t entrypgdir = {
	/* Identiy mapping for first 4M memory */
	.pdes[0] = (pde_t *) ((0) | PTE_P | PTE_W | PTE_PS),
	/* Higher address mapping for 4M memory */
	.pdes[KERNBASE >> PDXSHIFT] =
		 (pde_t *) ((0) | PTE_P | PTE_W | PTE_PS),
};
