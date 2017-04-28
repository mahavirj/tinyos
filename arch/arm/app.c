#include <stdint.h>
#include <task.h>
#include <sync.h>
#include <console.h>

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
	return 0;
}
