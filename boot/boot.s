; Multiboot macros to make a few lines later more readable
MULTIBOOT_HEADER_MAGIC	equ 	0x1BADB002
MULTIBOOT_HEADER_FLAGS	equ 	0x0
MULTIBOOT_CHECKSUM	equ 	-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

[BITS 32]

mboot:
	; This is the GRUB Multiboot header. A boot signature
	dd MULTIBOOT_HEADER_MAGIC
	dd MULTIBOOT_HEADER_FLAGS
	dd MULTIBOOT_CHECKSUM

[GLOBAL _start]
[EXTERN kmain]
_start equ (_entry - 0xC0000000)

[EXTERN entrypgdir]
; Kernel main entry point
_entry:
	mov eax, cr4
	or eax, 0x00000010
	mov cr4, eax

	mov eax, (entrypgdir - 0xC0000000)
	mov cr3, eax

	mov eax, cr0
	or eax, 0x80000000
	mov cr0, eax

	mov esp, _sys_stack
	mov eax, kmain
	jmp eax
	jmp $

; Here is the definition of our BSS section. Right now, we'll use
; it just to store the stack. Remember that a stack actually grows
; downwards, so we declare the size of the data before declaring
; the identifier '_sys_stack'
SECTION .bss
align 4
	resb 8192               ; This reserves 8KBytes of memory here
_sys_stack:
