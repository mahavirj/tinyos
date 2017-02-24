; Context switch
;
;   void swtch(struct context **old, struct context *new);
; 
; Save current register context in old
; and then load register context from new.

[GLOBAL swtch]
swtch:
  mov eax, [esp + 4] ;4(%esp), %eax
  mov edx, [esp + 8] ;8(%esp), %edx

  ; Save old callee-save registers
  push ebp
  push ebx
  push esi
  push edi

  ; Switch stacks
  mov [eax], esp ;%esp, (%eax)
  mov esp, edx ;%edx, %esp

  ; Load new callee-save registers
  pop edi
  pop esi
  pop ebx
  pop ebp
  ret
