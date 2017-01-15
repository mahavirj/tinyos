#ifndef __ISR_H__
#define __ISR_H__

#include <stdint.h>

typedef struct {
	uint32_t ds;                  // Data segment selector
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
	uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
	uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;

void irq_install_handler(int irq, void (*handler)(registers_t *r));
#endif /* __ISR_H__ */
