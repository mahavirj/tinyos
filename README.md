# TinyOS

X86 based Operating system built from scratch for learning purpose

- KISS (Keep It Simple Stupid) Philosophy
- Bootloader is assumed to be GRUB, kernel has GRUB specific multiboot header

## Features

- Higher half Kernel, Kernel sets itself up to run from higher half, 3GB region
- Multitasking
  - Basic scheduler with multitasking support, round robin, with same priority
  - Timer interrupt forces context switch
- User mode, Kernel mode distinction
  - GDT have been setup with appropriate DPLs
  - Kernel code runs in ring 0, and user code in ring 3
  - System call happens through `int 64` which has required GATE descriptors for
privilege escalation
- Initramfs
  - Standard cpio format for ramfs (no gzip compression)
  - Standard ELF format for user space applications
- Fork support
  - Clone parent process, no COW support
- Exec support
  - Overwrite address space with new process

# Overview

## Getting Started
To build operating system (assuming gcc, nasm installed),
```bash
make
```

To run under QEMU
```bash
make qemu
```

To run (assuming bochs, bochs-sdl installed),
```bash
make run
```

To run under QEMU with debugging support (GDB)
``` bash
make qemu_gdb
```
(Attach GDB, required commands already provided in .gdbinit file in top level dir)


# Additional Notes

## Bootup

BIOS starts processor in 16-bit real mode, GRUB initiates 32 bit protected
mode. Kernel ELF image is provided with multiboot header as per GRUB
specification, if GRUB finds this header in first 512 bytes of image, then it
loads ELF at 0x100000 location, lower memory belongs to BIOS, and other
hardware/IO mappings like VGA. Bootup code, executes from `_start` entry point
of kernel, and (note BSS is already zero initialized by GRUB),

- Sets up two 4MB (size-extended) PTE's to map kernel image in,
**0x0-0x400000** and **0xC0000000-0xC0400000** range respectively.
- Former being identity map as EIP is still in lower memory range and later
serving as higher half mapping.
- Stack pointer is set up in higher half range, earlier being set by GRUB below 1M
- Once paging is enabled, program does long jump to higher half, thus starting
execution from higher half.

## Memory Management

Simple `first-fit` strategy memory allocator, allocates memory from kernel
space, during `free` it also manages compaction of adjacent free blocks.

For userspace malloc/free are provided which internally used `sbrk` system call
to increase system break (if required). Kernel does required page table setups and
returns increases system break limit.

## Paging and VM

This is divided into kernel space memory mapping and user space memory
mappings. Kernel space mappings remains constant and are part of every process
address space, only linked not cloned, as changes from one process in kernel
space should be visible ot other processes as well.

User space mappings depends on `exec` call, and every process has its own
kernel as well as user stack.

During context switch, CR3 register gets loaded with current process page
directory base address and that also internally invalidates TLB (Translation
Lookaside Buffer).

## Init Process

User space applications are stored in `initramfs`, a cpio format archieve in
standard ELF format. During `exec` kernel finds and parses ELF image, sets up its
page tables accordingly. `Init` process only starts `shell` then hangs in there
forever.

## Scheduling

Scheduler runs on behalf of currently executing process, mainly in two cases,
- In case of timer IRQ, process will be swapped out
- In case of relinquishing CPU if process blocks on something or yields

## Kernel Space vs User Space

Kernel mode CODE/DATA segments are separate than user mode CODE/DATA segments with
privilege levels programmed accordingly. While returning from exception, hardware
pops up CS (code segment) and SS (stack segment) from stack to return to lower privilege level.

We program this stack frame accordingly while creating new task,

```c
        /* Task will start in CPL = 3, i.e. user mode */
        task->irqf->cs = (SEG_UCODE << 3) | DPL_USER;
        task->irqf->ds = (SEG_UDATA << 3) | DPL_USER;
        task->irqf->eflags = 0x200;
        task->irqf->ss = (SEG_UDATA << 3) | DPL_USER;
```

X86 hardware has built-in support for task switching, basically switching from
user stack to kernel stack along with other segmentation parameters. For this
every task needs to setup with its own TSS, which gets modified during context
switching. We will not be using hardware task switching, but will be doing same
in software itself. In any case we need to set up at-least one TSS which has valid
kernel mode SS (stack segment) and ESP (stack pointer) for current task.

## Synchronization

[Random FAQ](docs/NOTES.md)

# Contributing
Feel free to fork and send merge request

# Credits/References

- [Intel Reference Manual v.3A](http://download.intel.com/design/processor/manuals/253668.pdf)
- [Xv6, unix clone OS](https://pdos.csail.mit.edu/6.828/2016/xv6.html)
  - [Code on github](https://github.com/mit-pdos/xv6-public)
- [OSDev](http://wiki.osdev.org/Main_Page)
- [Bran's kernel development tutorial](http://www.osdever.net/bkerndev/Docs/gettingstarted.htm)
- [JamesM's kernel development tutorial](https://web.archive.org/web/20160311205056/http://www.jamesmolloy.co.uk/index.html)
  - Some known bugs as documented [here](http://wiki.osdev.org/James_Molloy's_Known_Bugs)
- [Little book on operating system](https://littleosbook.github.io/)

