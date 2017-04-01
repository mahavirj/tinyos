
# Compiler GCC
CC := arm-none-eabi-gcc
CFLAGS := -ggdb -mcpu=cortex-m3 -mthumb -O2 -ffreestanding -Iarch/arm/include/ -I include/
LDFILE := arch/arm/ldscript/gcc_arm.ld
LDFLAGS := -T $(LDFILE) -nostdlib

# Build artifacts
objdir := bin
kernel := $(objdir)/kernel.elf

kernel_src_dir := \
		arch/arm	 	\
		stdlib 	 		\

c_srcs := \
	arch/arm/startup_ARMCM3.c \
	arch/arm/system_ARMCM3.c \
	arch/arm/main.c \
	arch/arm/console.c \
	arch/arm/syscall.c \
	arch/arm/arch.c \
	arch/arm/timer.c \
	arch/arm/task.c \
	stdlib/stdlib.c \
	stdlib/printf.c \
	stdlib/malloc.c \

c_objs := $(c_srcs:%.c=$(objdir)/%.o)

define make-repo
   for dir in $(kernel_src_dir); \
   do \
        mkdir -p $(objdir)/$$dir; \
   done
endef

all: pre-build $(kernel)

pre-build:
	@mkdir -p $(objdir)
	@$(call make-repo)

$(kernel): $(LDFILE) $(c_objs)
	@echo "  LD    $@"
	$(V)$(CC) $(CFLAGS) $(LDFLAGS) $(c_objs) -o $@

qemu: all
	qemu-system-arm -M lm3s6965evb -kernel $(kernel) -serial stdio

qemu_gdb: all
	qemu-system-arm -M lm3s6965evb -kernel $(kernel) -serial stdio -s -S

clean:
	@rm -rf bin/
