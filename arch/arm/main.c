#include "list.h"
#include <stdint.h>
#include "task.h"

#include "ARMCM3.h"

static volatile int a = 5;

static void delay()
{
	volatile int a = 0x1000000;
	while (a--);
}

int new_delay()
{
	delay();
}

int new1()
{
	int count = 0;
	printf("Yay, new task\n");
	while(1) {
		printf("new looping %d %d\n", a, count++);
		new_delay();
	}
}

//__attribute__((naked)) void swtch(struct context **old, struct context *new, void *a)
void swtch(struct context **old, struct context *new, void *a)
{
	(void) a;

	__asm volatile("mrs r2, psp \n"
			"stmdb r2!, {r4 - r11, lr} \n"
			"str r2, [r0] \n"
			"ldmia r1!, {r4 - r11, lr} \n"
			"msr psp, r1 \n"
		//	"bx lr \n"
	);
}

int app_init()
{
	int count = 0;
	printf("We in init task\n");
	create_task(new1);
	while(1) {
		printf("Init looping %d %d\n", a, count++);
		delay();
	}
}

int kmain()
{
	printf("ARM CM3 OS\n");
	arch_init();
	create_init_task();
	init_scheduler();
	while(1);
}
