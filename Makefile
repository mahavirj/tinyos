
V := @
OUT_DIR := bin
OS_IMAGE := $(OUT_DIR)/os.iso
KERNEL := $(OUT_DIR)/kernel.elf
ASM_SRCS := boot/loader.s
C_SRCS := kernel/kmain.c

ASM_OBJS = $(patsubst %.s,$(OUT_DIR)/%.o, $(notdir $(ASM_SRCS)))
C_OBJS = $(patsubst %.c,$(OUT_DIR)/%.o, $(notdir $(C_SRCS)))

CC = gcc
CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	 -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -c
LDFLAGS = -T ldscript/linker.ld -melf_i386
AS = nasm
ASFLAGS = -f elf
LD = ld

all: pre-build $(KERNEL) $(OS_IMAGE)

pre-build:
	@mkdir -p $(OUT_DIR)

$(KERNEL): $(ASM_OBJS) $(C_OBJS)
	@echo "  LD    $@"
	$(V)$(LD) $(LDFLAGS) $^ -o $@

$(OS_IMAGE): $(KERNEL)
	$(V)cp $< iso/boot/
	$(V)genisoimage -R                          \
		-b boot/grub/grub    		\
		-no-emul-boot                   \
		-boot-load-size 4               \
		-A os                           \
		-input-charset utf8             \
		-quiet                          \
		-boot-info-table                \
		-o $@	 			\
		iso

run: all
	bochs -f tools/bochsrc.txt -q

$(C_OBJS): $(C_SRCS)
	@echo "  CC    $<"
	$(V)$(CC) $(CFLAGS)  $< -o $@

$(ASM_OBJS): $(ASM_SRCS)
	@echo "  ASM   $<"
	$(V)$(AS) $(ASFLAGS) $< -o $@

clean:
	@rm -rf bin/
	@rm -f iso/boo/$(KERNEL)
