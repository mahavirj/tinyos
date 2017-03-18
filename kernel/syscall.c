#include <isr.h>
#include <vga.h>
#include <helper.h>
#include <task.h>
#include <keyboard.h>

static registers_t *irqf;

static void *sys_handlers[] = {
	sys_write,
	sys_fork,
	sys_exec,
	sys_read,
	sys_exit,
	sys_waitpid,
	sys_sbrk,
};

int argint(int n, int *p)
{
	/* FIXME: Need some error checking here */
	*p = *(int *)(irqf->useresp + 4 + 4*n);
	return 0;
}

int argstr(int n, char **p)
{
	/* FIXME: Need some error checking here */
	*p = *(char **)(irqf->useresp + 4 + 4*n);
	return 0;
}

void syscall_handler(registers_t *r)
{
	int num = r->eax;
	int (*func) (void);

	irqf = r;
	if (num < ARR_SIZE(sys_handlers) && sys_handlers[r->eax]) {
		func = sys_handlers[r->eax];
		r->eax = func();
	} else {
		printk("Unsupported syscall %d\n", num);
	}
	irqf = NULL;
}
