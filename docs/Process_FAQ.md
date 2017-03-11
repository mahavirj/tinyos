# FAQ

* **What's the difference between kernel stack and user stack ?**
In short, nothing - apart from using a different location in memory (and hence a different value for the stackpointer register), and usually different memory access protections. I.e. when executing in user mode, kernel memory (part of which is the kernel stack) will not be accessible even if mapped. Vice versa, without explicitly being requested by the kernel code (in Linux, through functions like copy_from_user()), user memory (including the user stack) is not usually directly accessible.

* **Why is [ a separate ] kernel stack used ?**
Separation of privileges and security. For one, userspace programs can make their stack(pointer) anything they want, and there is usually no architectural requirement to even have a valid one. The kernel therefore cannot trust the userspace stackpointer to be valid nor usable, and therefore will require one set under its own control. Different CPU architectures implement this in different ways; x86 CPUs automatically switch stackpointers when privilege mode switches occur, and the values to be used for different privilege levels are configurable - by privileged code (i.e. only the kernel).

* **If a local variable is declared in an ISR, where will it be stored ?**
On the kernel stack. The kernel (Linux kernel, that is) does not hook ISRs directly to the x86 architecture's interrupt gates but instead delegates the interrupt dispatch to a common kernel interrupt entry/exit mechanism which saves pre-interrupt register state before calling the registered handler(s). The CPU itself when dispatching an interrupt might execute a privilege and/or stack switch, and this is used/set up by the kernel so that the common interrupt entry code can already rely on a kernel stack being present.
That said, interrupts that occur while executing kernel code will simply (continue to) use the kernel stack in place at that point. This can, if interrupt handlers have deeply nested call paths, lead to stack overflows (if a deep kernel call path is interrupted and the handler causes another deep path; in Linux, filesystem / software RAID code being interrupted by network code with iptables active is known to trigger such in untuned older kernels ... solution is to increase kernel stack sizes for such workloads).

* **Does each process have its own kernel stack ?**
Not just each process - each thread has its own kernel stack (and, in fact, its own user stack as well). Remember the only difference between processes and threads (to Linux) is the fact that multiple threads can share an address space (forming a process).

* **How does the process coordinate between both these stacks ?**
Not at all - it doesn't need to. Scheduling (how / when different threads are being run, how their state is saved and restored) is the operating system's task and processes don't need to concern themselves with this. As threads are created (and each process must have at least one thread), the kernel creates kernel stacks for them, while userspace stacks are either explicitly created/provided by whichever mechanism is used to create a thread (functions like makecontext() or pthread_create() allow the caller to specify a memory region to be used for the "child" thread's stack), or inherited (by on-access memory cloning, usually called "copy on write" / COW, when creating a new process).
That said, the process can influence scheduling of its threads and/or influence the context (state, amongst that is the thread's stackpointer)
