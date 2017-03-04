
# Compiler GCC
CC := gcc
AS := nasm
OBJDUMP:= objdump

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

c_srcs := $(foreach dir, $(kernel_src_dir), $(wildcard $(dir)/*.c))
asm_srcs := $(foreach dir, $(boot_src_dir), $(wildcard $(dir)/*.s))
c_objs := $(c_srcs:%.c=$(objdir)/%.o)
asm_objs := $(asm_srcs:%.s=$(objdir)/%.o)

CFLAGS := -g -O2 -m32 -ffreestanding -Wall -Wextra -MMD
CFLAGS += -Iinclude/kernel \
	-Iinclude/drivers \

LDFLAGS = -T ldscript/linker.ld -nostdlib -Wl,--build-id=none
ASFLAGS = -f elf

define make-repo
   for dir in $(kernel_src_dir) $(boot_src_dir); \
   do \
        mkdir -p $(objdir)/$$dir; \
   done
endef

all: pre-build $(kernel) $(os_image)

pre-build:
	@mkdir -p $(objdir)
	@$(call make-repo)

$(kernel): ldscript/linker.ld $(asm_objs) $(c_objs)
	@echo "  LD    $@"
	$(V)$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
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
