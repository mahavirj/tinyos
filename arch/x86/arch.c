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

extern unsigned end;
void arch_init()
{
	mem_init(&end, PHYS_RAM - (int) V2P(&end));
	printk("Initialized memory allocator\n");
	init_paging();
	printk("Initialized kernel paging\n");
	init_gdt();
	init_idt();
	printk("Initialized descriptors\n");
	init_keyboard();
	init_timer(100);
}
