# TinyOS

X86 based Operating system built from scratch for learning purpose:

* KISS (Keep It Simple Stupid) Philosophy
* Bootloader is assumed to be GRUB, kernel has GRUB specific multiboot header

## Features

* Higher half kernel
Kernel set itself up to run from higher half, 3GB region
* Multitasking
 * Basic scheduler with multitasking support, round robin, with same priority
 * Timer interrupt forces context switch
* User mode, Kernel mode distinction
 * GDT have been setup with appropriate DPLs
 * Kernel code runs in ring 0, and user code in ring 3
 * System call happens through `int 64` which has required GATE descriptors for
privilege escalation
* Initramfs
 * Standard cpio format for ramfs (no gzip compression)
 * Standard ELF format for user space applications
* Fork support
 * Clone parent process, no COW support
* Exec support
 * Overwrite address space with new process


# Overview

## Getting Started
To build operating system (assuming gcc, nasm installed),
* $ make

To run (assuming bochs, bochs-sdl installed),
* $ make run

To run under QEMU
* $ make qemu

To run under QEMU with debugging support (GDB)
* $ make qemu_gdb
  (Attach GDB, required commands already provided in .gdbinit file in top level dir)


# Additional Notes
[Technical notes](docs/NOTES.md)

# Contributing
Feel free to fork and send merge request

# References

* [Intel Reference Manual v.3A](http://download.intel.com/design/processor/manuals/253668.pdf)
* [Xv6, unix clone OS](https://pdos.csail.mit.edu/6.828/2016/xv6.html)
 * [Code on github](https://github.com/mit-pdos/xv6-public)
* [Bran's kernel development tutorial] (http://www.osdever.net/bkerndev/Docs/gettingstarted.htm)
* [JamesM's kernel development tutorial] (https://web.archive.org/web/20160311205056/http://www.jamesmolloy.co.uk/index.html)
 * Some known bugs as documented [here](http://wiki.osdev.org/James_Molloy's_Known_Bugs)
* [Little book on operating system] (https://littleosbook.github.io/)

