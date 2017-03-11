#include <isr.h>
#include <vga.h>
#include <helper.h>
#include <task.h>

static void *irq_handlers[256];

/* This is a simple string array. It contains the message that
 *  corresponds to each and every exception. We get the correct
 *  message by accessing like:
 *  exception_message[interrupt_number] */
const char *exception_messages[] =
{
	"Division By Zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint Exception",
	"Into Detected Overflow Exception",
	"Out of Bounds Exception",
	"Invalid Opcode Exception",
	"No Coprocessor Exception",
	"Double Fault Exception",
	"Coprocessor Segment Overrun Exception",
	"Bad TSS Exception",
	"Segment Not Present Exception",
	"Stack Fault Exception",
	"General Protection Fault Exception",
	"Page Fault Exception",
	"Unknown Interrupt Exception",
	"Coprocessor Fault Exception",
	"Alignment Check Exception",
	"Machine Check Exception",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
};

/* This gets called from our ASM interrupt handler stub */
void isr_handler(registers_t *r)
{
	/* This is a blank function pointer */
	void (*handler)(registers_t *r);

	handler = irq_handlers[r->int_no];
	if (handler)
		handler(r);
	if (r->int_no < 0x20) {
		printk("system generated exception, HALT! %d\n", r->int_no);
		printk("%s\n", exception_messages[r->int_no]);
		while(1);
	}
}

/* This installs a custom IRQ handler for the given IRQ */
void irq_install_handler(int irq, void (*handler)(registers_t *r))
{
	irq_handlers[irq] = handler;
}

/* This clears the handler for a given IRQ */
void irq_uninstall_handler(int irq)
{
	irq_handlers[irq] = 0;
}

static void syscall_handler(registers_t *r)
{
	/* write system call */
	if (r->eax == 0) {
		/* First argument is fd, ignore for now */
		/* buffer to print on VGA */
		char *buf = (char *) (*(int *)(r->useresp + 8));
		/* length of buffer */
		int len = *(int *) (r->useresp + 12);
		vga_write_buf(buf, len);
	/* fork system call */
	} else if (r->eax == 1) {
		/* FIXME: do we need to set current task `irqf` to `r` here? */
		r->eax = sys_fork();
	} else if (r->eax == 2) {
		char *fname = (char *) (*(int *)(r->useresp + 4));
		r->eax = sys_exec(fname);
	}
}

// This gets called from our ASM interrupt handler stub.
void irq_handler(registers_t *r)
{
	/* This is a blank function pointer */
	void (*handler)(registers_t *r);

	if (r->int_no == IRQ_SYSCALL) {
		syscall_handler(r);
		return;
	}

	/* If the IDT entry that was invoked was greater than 40
	 *  (meaning IRQ8 - 15), then we need to send an EOI to
	 *  the slave controller */
	if (r->int_no >= 40)
		outportb(0xA0, 0x20);

	/* In either case, we need to send an EOI to the master
	 *  interrupt controller too */
	outportb(0x20, 0x20);

	/* Find out if we have a custom handler to run for this
	 *  IRQ, and then finally, run it */
	handler = irq_handlers[r->int_no];
	if (handler)
		handler(r);
}
