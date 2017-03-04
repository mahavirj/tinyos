# TinyOS

X86 based Operating system built from scratch for learning purpose

* KISS (Keep It Simple Stupid) Philosophy
* Bootloader is imported from GRUB to avoid going down legacy X86 stuff

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


# References

* [Xv6, unix clone OS](https://pdos.csail.mit.edu/6.828/2016/xv6.html)
 * [Code on github](https://github.com/mit-pdos/xv6-public)
* [Bran's kernel development tutorial] (http://www.osdever.net/bkerndev/Docs/gettingstarted.htm)
* [JamesM's kernel development tutorial] (https://web.archive.org/web/20160311205056/http://www.jamesmolloy.co.uk/index.html)
 * Some known bugs as documented [here](http://wiki.osdev.org/James_Molloy's_Known_Bugs)
* [Little book on operating system] (https://littleosbook.github.io/)

