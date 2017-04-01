#include <task.h>
#include <arch.h>
#include <console.h>

static void print_banner()
{
	printk("\n#########################\n");
	printk(" Welcome to Tiny OS v%s\n", VERSION);
	printk("#########################\n\n");
}

int kmain(void)
{
	console_init();
	print_banner();
	arch_init();
	create_init_task();
	init_scheduler();
	printk("HALT! Unreachable code\n");
	for (;;)
		;
}
