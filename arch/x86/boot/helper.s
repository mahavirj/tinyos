; This will set up our new segment registers. We need to do
; something special in order to set CS. We do what is called a
; far jump. A jump that includes a segment as well as an offset.
; This is declared in C as 'extern void gdt_flush();'
[GLOBAL gdt_flush]     ; Allows the C code to link to this
gdt_flush:
    mov eax, [esp+4]  ; Get the pointer to the GDT, passed as a parameter
    lgdt [eax]        ; Load the GDT pointer
    mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush2   ; 0x08 is the offset to our code segment: Far jump!
flush2:
    ret               ; Returns back to the C code!

; This is declared in C as 'extern void idt_load();'
[GLOBAL idt_load]
idt_load:
    mov eax, [esp+4]
    lidt [eax]
    ret

[GLOBAL tss_flush]    ; Allows our C code to call tss_flush().
tss_flush:
   mov ax, 0x28      ; Load the index of our TSS structure - The index is
                     ; 0x28, as it is the 5th selector
   ltr ax            ; Load ax into the task state register.
   ret
