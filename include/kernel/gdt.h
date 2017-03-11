#ifndef __GDT_H__
#define __GDT_H__

#include <stdint.h>

#define SEG_NULL 	0
#define SEG_KCODE 	1
#define SEG_KDATA 	2
#define SEG_UCODE 	3
#define SEG_UDATA 	4
#define SEG_TSS		5

#define DPL_KERN	0
#define DPL_USER	3

// A struct describing a Task State Segment.
typedef struct{
   uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
   uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
   uint32_t ss0;        // The stack segment to load when we change to kernel mode.
   uint32_t esp1;       // Unused...
   uint32_t ss1;
   uint32_t esp2;
   uint32_t ss2;
   uint32_t cr3;
   uint32_t eip;
   uint32_t eflags;
   uint32_t eax;
   uint32_t ecx;
   uint32_t edx;
   uint32_t ebx;
   uint32_t esp;
   uint32_t ebp;
   uint32_t esi;
   uint32_t edi;
   uint32_t es;         // The value to load into ES when we change to kernel mode.
   uint32_t cs;         // The value to load into CS when we change to kernel mode.
   uint32_t ss;         // The value to load into SS when we change to kernel mode.
   uint32_t ds;         // The value to load into DS when we change to kernel mode.
   uint32_t fs;         // The value to load into FS when we change to kernel mode.
   uint32_t gs;         // The value to load into GS when we change to kernel mode.
   uint32_t ldt;        // Unused...
   uint16_t trap;
   uint16_t iomap_base;
} __attribute__((packed)) tss_entry_t;

/* Defines a GDT entry. We say packed, because it prevents the
 *  compiler from doing things that it thinks is best: Prevent
 *  compiler "optimization" by packing */
struct gdt_entry
{
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} __attribute__((packed));

/* Special pointer which includes the limit: The max bytes
 *  taken up by the GDT, minus 1. Again, this NEEDS to be packed */
struct gdt_ptr
{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

/* This will be a function in start.asm. We use this to properly
 *  reload the new segment registers */
void gdt_flush();

/* Install GDT */
void init_gdt();

void tss_flush();

void set_kernel_stack(uint32_t stack);

#endif	/* __GDT_H__ */
