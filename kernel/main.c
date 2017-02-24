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

	while (1) {
		printk("Hello from task1 %d\n", val++);
		wait_ms(1000);
	}
}

extern unsigned end;
int kmain(void)
{
	uint32_t *kmem_addr = (uint32_t *) 0xc0000000;

	init_gdt();
	init_idt();
	k_video_init();
	init_timer(100);
	init_keyboard();
	init_paging();
	mem_init(kmem_addr, 1U << 22);
	create_task(t1);
	create_task(t2);
	tiny_scheduler();
	printk("HALT! Unreachable code\n");
	while (1);
}
