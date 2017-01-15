#ifndef __IDT_H__
#define __IDT_H__

#include <stdint.h>

/* Defines an IDT entry */
struct idt_entry
{
	uint16_t base_lo;
	uint16_t sel;        /* Our kernel segment goes here! */
	uint8_t always0;     /* This will ALWAYS be set to 0! */
	uint8_t flags;       /* Set using the above table! */
	uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr
{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

/* This exists in 'helper.s', and is used to load our IDT */
void idt_load();

void init_idt();

void isr0();
void isr1();
void isr2();
void isr3();
void isr4();
void isr5();
void isr6();
void isr7();
void isr8();
void isr9();
void isr10();
void isr11();
void isr12();
void isr13();
void isr14();
void isr15();
void isr16();
void isr17();
void isr18();
void isr19();
void isr20();
void isr21();
void isr22();
void isr23();
void isr24();
void isr25();
void isr26();
void isr27();
void isr28();
void isr29();
void isr30();
void isr31();

void irq0();
void irq1();
void irq2();
void irq3();
void irq4();
void irq5();
void irq6();
void irq7();
void irq8();
void irq9();
void irq10();
void irq11();
void irq12();
void irq13();
void irq14();
void irq15();

#endif /* __IDT_H__ */
