#include <vga.h>
#include <gdt.h>
#include <idt.h>

int kmain(void)
{
	int i;

	init_gdt();
	init_idt();
	k_video_init();

	for (i = 0; i < 24; i++)
		printk("Hello World: %d %x\n", i * 250000, i * 128);

	return 0;
}
