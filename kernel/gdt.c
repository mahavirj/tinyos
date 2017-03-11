#include <string.h>
#include <gdt.h>

/* Our GDT, with 6 entries, and finally our special GDT pointer */
static struct gdt_entry gdt[6];
static struct gdt_ptr gp;
static tss_entry_t tss_entry;

void set_kernel_stack(uint32_t stack)
{
	tss_entry.esp0 = stack;
	/* FIXME: why below line results in general prot. fault? */
	/* tss_flush(); */
}

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
	/* Setup the descriptor base address */
	gdt[num].base_low = (base & 0xFFFF);
	gdt[num].base_middle = (base >> 16) & 0xFF;
	gdt[num].base_high = (base >> 24) & 0xFF;

	/* Setup the descriptor limits */
	gdt[num].limit_low = (limit & 0xFFFF);
	gdt[num].granularity = ((limit >> 16) & 0x0F);

	/* Finally, set up the granularity and access flags */
	gdt[num].granularity |= (gran & 0xF0);
	gdt[num].access = access;
}

/* Should be called by main. This will setup the special GDT
 *  pointer, set up the first 3 entries in our GDT, and then
 *  finally call gdt_flush() in our assembler file in order
 *  to tell the processor where the new GDT is and update the
 *  new segment registers */
void init_gdt()
{
	/* Setup the GDT pointer and limit */
	gp.limit = sizeof(gdt) - 1;
	gp.base = (uint32_t) &gdt;

	/* Our NULL descriptor */
	gdt_set_gate(SEG_NULL, 0, 0, 0, 0);

	/* The second entry is our Code Segment. The base address
	 *  is 0, the limit is 4GBytes, it uses 4KByte granularity,
	 *  uses 32-bit opcodes, and is a Code Segment descriptor.
	 *  Please check the table above in the tutorial in order
	 *  to see exactly what each value means */
	gdt_set_gate(SEG_KCODE, 0, 0xFFFFFFFF, 0x9A, 0xCF);

	/* The third entry is our Data Segment. It's EXACTLY the
	 *  same as our code segment, but the descriptor type in
	 *  this entry's access byte says it's a Data Segment */
	gdt_set_gate(SEG_KDATA, 0, 0xFFFFFFFF, 0x92, 0xCF);

	/* User mode code segment */
	gdt_set_gate(SEG_UCODE, 0, 0xFFFFFFFF, 0xFA, 0xCF);
	/* User mode data segment */
	gdt_set_gate(SEG_UDATA, 0, 0xFFFFFFFF, 0xF2, 0xCF);

	// Firstly, let's compute the base and limit of our entry into the GDT.
	uint32_t base = (uint32_t) &tss_entry;
	uint32_t limit = base + sizeof(tss_entry);

	tss_entry.ss0  = SEG_KDATA << 3;  // Set the kernel stack segment
	// Now, add our TSS descriptor's address to the GDT
	gdt_set_gate(SEG_TSS, base, limit, 0xE9, 0x00);

	/* Flush out the old GDT and install the new changes! */
	gdt_flush(&gp);
	tss_flush();
}

