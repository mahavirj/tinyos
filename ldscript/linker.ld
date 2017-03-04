/* the name of the entry label */
ENTRY(_start)

SECTIONS {
	. = 0xC0100000;          /* Kernel will execute from 3G + 1M */

	.text ALIGN (0x1000) : AT(0x100000)   /* align at 4 KB */
	{
		code = .;
		*(.text*)             /* all text sections from all files */
		*(.rodata*)          /* all read-only data sections from all files */
	}

	.data ALIGN (0x1000) :   /* align at 4 KB */
	{
		data = .;
		*(.data*)             /* all data sections from all files */
	}

	.bss ALIGN (0x1000) :    /* align at 4 KB */
	{
		bss = .;
		*(COMMON)            /* all COMMON sections from all files */
		*(.bss*)              /* all bss sections from all files */
	}

	end = .;

}
