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

	init_gdt();
	init_idt();
	k_video_init();
	init_timer(100);
	init_keyboard();
	asm("sti");
	mem_init(&end, 1U << 24);
	init_paging();

	while (1) {
		printk("Hello World: %d:\n", i++);
		wait_ms(5000);
	}
}
