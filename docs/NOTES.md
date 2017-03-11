# Random Notes

* `pusha` and `popa` instructions considers esp as general purpose register,
but do not restore/modify it during `popa` sequence.
http://x86.renejeschke.de/html/file_module_x86_id_249.html

* Setting up TSS (at-least one) is mandatory, even if hardware assisted task
switching is not used, X86 needs TSS for user to kernel mode transition.
https://web.archive.org/web/20160311214501/http://www.jamesmolloy.co.uk/tutorial_html/10.-User%20Mode.html

* **User to kernel mode transition:**
This requires TSS (task state segment) already set up in GDT (global descriptor
table). TSS has information for mainly SS, ESP and other registers that will be
loaded in hardware context upon kernel mode transition (CPL = 0). Note that,
interrupt handler needs to be set with GATE privilege such that `int $x`
instruction can work from lower privilege levels.
ESP in TSS is kernel mode stack of task, that gets updated in every context switch.
Transition includes: 
 * First extract SS (stack segment) and ESP from TSS
 * Save 5 registers on to stack, SS, ESP, EFLAGS, CS, EIP (instruction after int)
 * Process interrupt handler
 * On exit, pop 5 registers from stack, that loads up correct stack and IP.

* `Double fault` occurs if something goes wrong in handling exception by processor,
e.g. stack overflow, or incorrect entry in IDT. There is CPU generated exception for
double fault. If something goes wrong in handling double fault, then it generates
triple fault (mostly rare, may result in system RESET).

* **CPL/DPL/RPL**
An interrupt can never transfer control from a more-privileged to a less-privileged ring.
Privilege must either stay the same (when the kernel itself is interrupted) or be elevated
(when user-mode code is interrupted). In either case, the resulting CPL will be equal to
to the DPL of the destination code segment; if the CPL changes, a stack switch also occurs.
If an interrupt is triggered by code via an instruction like int n, one more check
takes place: the gate DPL must be at the same or lower privilege as the CPL. This
prevents user code from triggering random interrupts. If these checks fail – you
guessed it – a general-protection exception happens. All Linux interrupt handlers
end up running in ring zero.
http://duartes.org/gustavo/blog/post/cpu-rings-privilege-and-protection/
