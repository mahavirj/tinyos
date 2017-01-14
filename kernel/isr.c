#include <isr.h>
#include <vga.h>

/* This gets called from our ASM interrupt handler stub */
void isr_handler(registers_t *regs)
{
	k_write("recieved interrupt: ");
	k_write_dec(regs->int_no);
	k_write("\n");
}
