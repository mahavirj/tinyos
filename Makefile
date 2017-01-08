
# Compiler GCC
CC := gcc
AS := nasm

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

CFLAGS := -O2 -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -MMD
CFLAGS += -Iinclude/stdlib \
	-Iinclude/drivers \

LDFLAGS = -T ldscript/linker.ld
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

$(kernel): $(asm_objs) $(c_objs)
	@echo "  LD    $@"
	$(V)$(LD) $(LDFLAGS) $^ -o $@

$(os_image): $(kernel)
	$(V)cp $< iso/boot/
	$(V)genisoimage -R -b boot/grub/grub -no-emul-boot -boot-load-size 4 \
                -A os -input-charset utf8 -quiet -boot-info-table -o $@ iso

run: all
	bochs -qf tools/bochsrc.txt -rc tools/bochsrc.debug

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
