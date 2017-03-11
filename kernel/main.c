#include <vga.h>
#include <gdt.h>
#include <idt.h>
#include <helper.h>
#include <timer.h>
#include <keyboard.h>
#include <mem.h>
#include <vm.h>
#include <task.h>
#include <sync.h>

#if 0
static int count = 1;
static struct spinlock lock;
static void t1();
static void t2();

static void t2()
{
	while (1) {
		acquire(&lock);
		if (count % 2 != 0)
			sleep(t2, &lock);
		printk("B. Hello World Iteration %d\n", count++);
		wakeup(t1);
		release(&lock);
	}
}

static void t1()
{
	create_task(t2);

	while (1) {
		acquire(&lock);
		if (count % 2 == 0)
			sleep(t1, &lock);
		printk("A. Hello World Iteration %d\n", count++);
		wakeup(t2);
		release(&lock);
	}
}
#endif

static void print_banner()
{
	printk("\n#########################\n");
	printk(" Welcome to Tiny OS v%s\n", VERSION);
	printk("#########################\n\n");
}

extern unsigned end;
int kmain(void)
{
	init_vga();
	print_banner();
	mem_init(&end, PHYS_RAM - (int) V2P(&end));
	printk("Initialized memory allocator\n");
	init_paging();
	printk("Initialized kernel paging\n");
	init_gdt();
	init_idt();
	printk("Initialized gdt/idt settings\n");
	init_keyboard();
	init_timer(100);
	create_init_task();
	init_scheduler();
	printk("HALT! Unreachable code\n");
	for (;;)
		;
}

__attribute__((aligned(PGSIZE))) pd_t entrypgdir = {
	/* Identiy mapping for first 4M memory */
	.pdes[0] = (pde_t *) ((0) | PTE_P | PTE_W | PTE_PS),
	/* Higher address mapping for 4M memory */
	.pdes[KERNBASE >> PDXSHIFT] =
		 (pde_t *) ((0) | PTE_P | PTE_W | PTE_PS),
};
