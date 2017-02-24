#include <vga.h>
#include <gdt.h>
#include <idt.h>
#include <helper.h>
#include <timer.h>
#include <keyboard.h>
#include <kmalloc.h>
#include <paging.h>
#include <task.h>

static void t2()
{
	int val = 1000;

	while (1) {
		printk("Hello from task2 %d\n", val++);
		wait_ms(5000);
	}
}

static void t1()
{
	int val = 500;
	create_task(t2);

	while (1) {
		printk("Hello from task1 %d\n", val++);
		wait_ms(1000);
	}
}

extern unsigned end;
int kmain(void)
{
	init_gdt();
	init_idt();
	k_video_init();
	init_timer(100);
	init_keyboard();
	init_paging();
	mem_init(&end, 1U << 20);
	create_task(t1);

	tiny_scheduler();
	printk("HALT! Unreachable code\n");
	while (1);
}

__attribute__((aligned(PGSIZE))) page_directory_t entrypgdir = {
	/* Identiy mapping for first 4M memory */
	.tables[0] = (page_table_t *) ((0) | PTE_P | PTE_W | PTE_PS),
	/* Higher address mapping for 4M */
	.tables[KERNBASE >> PDXSHIFT] =
		 (page_table_t *) ((0) | PTE_P | PTE_W | PTE_PS),
};
