
# Compiler GCC
CC := gcc
AS := nasm
OBJDUMP := objdump
OBJCOPY := objcopy
AR := ar

# Tiny OS version
VERSION := 0.2

# Verbose build pass verbose=1
ifeq ($(verbose),1)
V :=
else
V := @
endif

# Build artifacts
objdir := bin
os_image := $(objdir)/os.iso
kernel := $(objdir)/kernel.elf

kernel_src_dir := \
		kernel	 \
		drivers	 \
		stdlib 	 \

boot_src_dir :=	boot

app_src_dir := app

app_obj_dir := $(objdir)/$(app_src_dir)

c_srcs := kernel/cpio_parser.c \
	  kernel/gdt.c \
	  kernel/idt.c \
	  kernel/isr.c \
	  kernel/keyboard.c \
	  kernel/main.c \
	  kernel/mem.c \
	  kernel/sync.c \
	  kernel/syscall.c \
	  kernel/task.c \
	  kernel/timer.c \
	  kernel/vm.c \
	  kernel/wait_queue.c \
	  drivers/vga.c \
	  stdlib/stdlib.c \
	  stdlib/printk.c \

asm_srcs := boot/boot.s \
	    boot/helper.s \
	    boot/isr.s \
	    boot/swtch.s \
	    boot/sync.s \

c_objs := $(c_srcs:%.c=$(objdir)/%.o)
asm_objs := $(asm_srcs:%.s=$(objdir)/%.o)

app_lib_srcs := stdlib/crt.c \
		stdlib/stdlib.c \
		stdlib/printf.c \
		stdlib/malloc.c \

app_asm_srcs := boot/syscall.s \

app_lib_objs := $(app_lib_srcs:%.c=$(objdir)/%.o)
app_asm_objs := $(app_asm_srcs:%.s=$(objdir)/%.o)

app_lib := $(objdir)/lib_helper.a

CFLAGS := -g -O2 -m32 -ffreestanding -Wall -Wextra -DVERSION=\"$(VERSION)\"
CFLAGS += -Iinclude/kernel \
	-Iinclude/drivers \

LDSCRIPT = -T ldscript/linker.ld
APP_LDSCRIPT = -T app/linker.ld
LDFLAGS = -nostdlib -Wl,--build-id=none
APP_CFLAGS := -g -O2 -m32 -static -fno-pic -Wall -Wextra -ffreestanding -I app/
APP_LDFLAGS := -nostdlib -Ttext 0x100000 -Wl,--build-id=none
ASFLAGS = -f elf

define make-repo
   for dir in $(kernel_src_dir) $(boot_src_dir) $(app_src_dir); \
   do \
        mkdir -p $(objdir)/$$dir; \
   done
endef

all: pre-build $(kernel) $(os_image)

ramfs.obj: $(app_lib) $(app_obj_dir)/init $(app_obj_dir)/shell $(app_obj_dir)/forktest $(app_obj_dir)/memtest
	$(V)cd $(app_obj_dir) && find . | cpio -o -H newc > ../ramfs.cpio
	$(V)cd $(objdir) && $(OBJCOPY) -I binary -O elf32-i386 -B i386 ramfs.cpio $@

$(app_lib): $(app_lib_objs) $(app_asm_objs)
	$(V)$(AR) cru $@ $^

$(app_obj_dir)/forktest: $(app_src_dir)/forktest.c $(app_lib)
	@echo "  APP   $<"
	$(V)$(CC) $(APP_CFLAGS) $(APP_LDFLAGS) $^ -o $@

$(app_obj_dir)/memtest: $(app_src_dir)/memtest.c $(app_lib)
	@echo "  APP   $<"
	$(V)$(CC) $(APP_CFLAGS) $(APP_LDFLAGS) $^ -o $@

$(app_obj_dir)/init: $(app_src_dir)/init.c $(app_lib)
	@echo "  APP   $<"
	$(V)$(CC) $(APP_CFLAGS) $(APP_LDFLAGS) $^ -o $@

$(app_obj_dir)/shell: $(app_src_dir)/shell.c $(app_lib)
	@echo "  APP   $<"
	$(V)$(CC) $(APP_CFLAGS) $(APP_LDFLAGS) $^ -o $@

pre-build:
	@mkdir -p $(objdir)
	@$(call make-repo)

$(kernel): ldscript/linker.ld $(asm_objs) $(c_objs) ramfs.obj
	@echo "  LD    $@"
	$(V)$(CC) $(CFLAGS) $(LDFLAGS) $(LDSCRIPT) $(asm_objs) $(c_objs) $(objdir)/ramfs.obj -o $@
	$(V)$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(objdir)/kernel.sym

$(os_image): $(kernel)
	$(V)cp $< iso/boot/
	$(V)genisoimage -R -b boot/grub/grub -no-emul-boot -boot-load-size 4 \
                -A os -input-charset utf8 -quiet -boot-info-table -o $@ iso

run: all
	bochs -qf tools/bochsrc.txt -rc tools/bochsrc.debug

qemu: all
	qemu-system-i386 -cdrom bin/os.iso -m 32

qemu_gdb: all
	qemu-system-i386 -cdrom bin/os.iso -m 32 -s -S

$(objdir)/%.o: %.c
	@echo "  CC    $<"
	$(V)$(CC) -c $(CFLAGS) $< -o $@

$(objdir)/%.o: %.s
	@echo "  ASM   $<"
	$(V)$(AS) $(ASFLAGS) $< -o $@

clean:
	@rm -rf bin/
	@rm -f iso/boot/$(kernel)

.PHONY: all pre-build clean run
