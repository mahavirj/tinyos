
# Compiler GCC
CC := gcc
AS := nasm
OBJDUMP := objdump
OBJCOPY := objcopy

# Tiny OS version
VERSION := 0.1

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

c_srcs := $(foreach dir, $(kernel_src_dir), $(wildcard $(dir)/*.c))
asm_srcs := $(foreach dir, $(boot_src_dir), $(wildcard $(dir)/*.s))
c_objs := $(c_srcs:%.c=$(objdir)/%.o)
asm_objs := $(asm_srcs:%.s=$(objdir)/%.o)
syscall_obj := $(objdir)/$(boot_src_dir)/syscall.o

CFLAGS := -g -O2 -m32 -ffreestanding -Wall -Wextra -DVERSION=\"$(VERSION)\"
CFLAGS += -Iinclude/kernel \
	-Iinclude/drivers \

LDSCRIPT = -T ldscript/linker.ld
APP_LDSCRIPT = -T app/linker.ld
LDFLAGS = -nostdlib -Wl,--build-id=none
APP_CFLAGS := -g -O2 -m32 -static -fno-pic -Wall -Wextra -ffreestanding
APP_LDFLAGS := -nostdlib -Ttext 0x100000 -e main -Wl,--build-id=none
ASFLAGS = -f elf

define make-repo
   for dir in $(kernel_src_dir) $(boot_src_dir) $(app_src_dir); \
   do \
        mkdir -p $(objdir)/$$dir; \
   done
endef

all: pre-build $(kernel) $(os_image)

ramfs.obj: $(app_obj_dir)/init $(app_obj_dir)/shell
	$(V)cd $(app_obj_dir) && find . | cpio -o -H newc > ../ramfs.cpio
	$(V)cd $(objdir) && $(OBJCOPY) -I binary -O elf32-i386 -B i386 ramfs.cpio $@

$(app_obj_dir)/init: $(app_src_dir)/init.c
	@echo "  APP   $<"
	$(V)$(CC) $(APP_CFLAGS) $(APP_LDFLAGS) $< $(syscall_obj) -o $@

$(app_obj_dir)/shell: $(app_src_dir)/shell.c
	@echo "  APP   $<"
	$(V)$(CC) $(APP_CFLAGS) $(APP_LDFLAGS) $< $(syscall_obj) -o $@

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
