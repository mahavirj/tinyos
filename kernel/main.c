#include <task.h>
//#include <sync.h>
#include <arch.h>
#include <console.h>

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
