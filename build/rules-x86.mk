
# Compiler GCC
CC := gcc
AS := nasm
OBJDUMP := objdump
OBJCOPY := objcopy
AR := ar

# Build artifacts
objdir := bin
os_image := $(objdir)/os.iso
kernel := $(objdir)/kernel.elf

kernel_src_dir := \
		arch/x86/	 	\
		arch/x86/drivers/ 	\
		kernel/			\
		stdlib/	 		\

boot_src_dir :=	arch/x86/boot

app_src_dir := app

app_obj_dir := $(objdir)/$(app_src_dir)

c_srcs := \
	  arch/x86/arch.c \
	  arch/x86/cpio_parser.c \
	  arch/x86/gdt.c \
	  arch/x86/idt.c \
	  arch/x86/isr.c \
	  arch/x86/keyboard.c \
	  arch/x86/mem.c \
	  arch/x86/syscall.c \
	  arch/x86/task.c \
	  arch/x86/timer.c \
	  arch/x86/vm.c \
	  arch/x86/drivers/vga.c \
	  kernel/main.c \
	  kernel/sync.c \
	  stdlib/stdlib.c \
	  stdlib/printk.c \
	  stdlib/wait_queue.c \

asm_srcs := arch/x86/boot/boot.s \
	    arch/x86/boot/helper.s \
	    arch/x86/boot/isr.s \
	    arch/x86/boot/swtch.s \
	    arch/x86/boot/sync.s \

c_objs := $(c_srcs:%.c=$(objdir)/%.o)
asm_objs := $(asm_srcs:%.s=$(objdir)/%.o)

app_lib_srcs := stdlib/crt.c \
		stdlib/stdlib.c \
		stdlib/printf.c \
		stdlib/malloc.c \

app_asm_srcs := arch/x86/boot/syscall.s \

app_lib_objs := $(app_lib_srcs:%.c=$(objdir)/%.o)
app_asm_objs := $(app_asm_srcs:%.s=$(objdir)/%.o)

app_lib := $(objdir)/lib_helper.a

CFLAGS := -g -O2 -m32 -ffreestanding -Wall -Wextra -DVERSION=\"$(VERSION)\"
CFLAGS += -Iinclude/ \
	-Iarch/x86/include \
	-Iarch/x86/include/drivers \

LDFILE = arch/x86/ldscript/linker.ld
LDSCRIPT = -T $(LDFILE)
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
	$(V)rm -f $(objdir)/ramfs.cpio $(app_lib)

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

$(kernel): $(LDFILE) $(asm_objs) $(c_objs) ramfs.obj
	@echo "  LD    $@"
	$(V)$(CC) $(CFLAGS) $(LDFLAGS) $(LDSCRIPT) $(asm_objs) $(c_objs) $(objdir)/ramfs.obj -o $@
	$(V)$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(objdir)/kernel.sym
	$(V)rm -f $(objdir)/ramfs.obj

$(os_image): $(kernel)
	$(V)cp $< iso/boot/
	$(V)genisoimage -R -b boot/grub/grub -no-emul-boot -boot-load-size 4 \
                -A os -input-charset utf8 -quiet -boot-info-table -o $@ iso

run: all
	bochs -qf tools/bochsrc.txt -rc tools/bochsrc.debug

qemu: all
	qemu-system-i386 -cdrom bin/os.iso -m 32 -monitor stdio

qemu_gdb: all
	qemu-system-i386 -cdrom bin/os.iso -m 32 -s -S -monitor stdio

clean:
	@rm -rf bin/
	@rm -f iso/boot/$(kernel)
