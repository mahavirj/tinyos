MEMORY
{
  RAM  (rwx) : ORIGIN = 0x0, LENGTH = 4K
}

ENTRY(main)

SECTIONS {
	.userapp :
	{
		_bin_userapp_start = .;
		*(.text*)
		*(.rodata*)
		. = ALIGN(4);
		*(.data*)
		. = ALIGN(4);
		*(COMMON)
		. = ALIGN(4);
		*(.bss*)
		_bin_userapp_end = .;
	} > RAM

        /DISCARD/ : {
                *(.eh_frame .note.GNU-stack)
        }

}
