#ifndef __GDT_H__
#define __GDT_H__

#include <stdint.h>

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

#endif	/* __GDT_H__ */
