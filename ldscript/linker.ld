/* the name of the entry label */
ENTRY(_start)

SECTIONS {
	. = 0xC0100000;          /* Kernel will execute from 3G + 1M */

	.text ALIGN (0x1000) : AT(0x100000)   /* align at 4 KB */
	{
		*(.text*)             /* all text sections from all files */
		*(.rodata*)          /* all read-only data sections from all files */
	}
	/* Align on page boundary, read only page mapping */
	. = ALIGN(0x1000);
	etext = .;

	.data ALIGN (0x1000) :   /* align at 4 KB */
	{
		*(.data*)             /* all data sections from all files */
	}

	.bss ALIGN (0x1000) :    /* align at 4 KB */
	{
		*(COMMON)           /* all COMMON sections from all files */
		*(.bss*)             /* all bss sections from all files */
	}

	. = ALIGN(4);
	end = .;
}
