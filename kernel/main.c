#include <vga.h>
#include <gdt.h>
#include <idt.h>
#include <helper.h>
#include <timer.h>
#include <keyboard.h>
#include <kmalloc.h>
#include <paging.h>

extern unsigned end;
int kmain(void)
{
	int i = 0;
	uint32_t *kmem_addr = (uint32_t *) 0xc0000000;

	init_gdt();
	init_idt();
	k_video_init();
	init_timer(100);
	init_keyboard();
	asm("sti");
	init_paging();
	mem_init(kmem_addr, 1U << 24);
	int *p = kmalloc(50);
	int *q = kmalloc_page(50);
	printk("Allocated @%x and @%x\n", p, q);

	while (1) {
		printk("Hello World: %d:\n", i++);
		wait_ms(5000);
	}
}
