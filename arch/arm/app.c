#include <stdint.h>
#include <task.h>
#include <sync.h>
#include <console.h>

static void delay()
{
	volatile int a = 0x1000000;
	while (a--);
}

#if 0
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
#endif

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
               printf("B. Hello World Iteration %d\n", count++);
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
               printf("A. Hello World Iteration %d\n", count++);
               wakeup(t2);
               release(&lock);
       }
}

int main()
{
	t1();
}
