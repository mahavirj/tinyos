; Multiboot macros to make a few lines later more readable
MULTIBOOT_PAGE_ALIGN	equ 	1<<0
MULTIBOOT_MEMORY_INFO	equ 	1<<1
MULTIBOOT_HEADER_MAGIC	equ 	0x1BADB002
MULTIBOOT_HEADER_FLAGS	equ 	MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM	equ 	-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

[BITS 32]

[EXTERN code]
[EXTERN bss]
[EXTERN end]

mboot:
	; This is the GRUB Multiboot header. A boot signature
	dd MULTIBOOT_HEADER_MAGIC
	dd MULTIBOOT_HEADER_FLAGS
	dd MULTIBOOT_CHECKSUM

	; The linker script fills in the data for these ones!
	dd mboot
	dd code
	dd bss
	dd end
	dd _start

[GLOBAL _start]
[EXTERN kmain]

; Kernel main entry point
_start:
	mov esp, _sys_stack
	push ebx

	cli
	call kmain
	jmp $

; Here is the definition of our BSS section. Right now, we'll use
; it just to store the stack. Remember that a stack actually grows
; downwards, so we declare the size of the data before declaring
; the identifier '_sys_stack'
SECTION .bss
	resb 8192               ; This reserves 8KBytes of memory here
_sys_stack:
