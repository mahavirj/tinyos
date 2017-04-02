# Tiny OS version
VERSION := 0.2

# Architecture
ARCH ?= x86

# Verbose build pass verbose=1
ifeq ($(verbose),1)
V :=
else
V := @
endif

include build/rules-$(ARCH).mk

define make-repo
   for dir in $(src_dirs); \
   do \
        mkdir -p $(objdir)/$$dir; \
   done
endef

$(objdir)/%.o: %.c
	@echo "  CC    $<"
	$(V)$(CC) -c $(CFLAGS) $< -o $@

$(objdir)/%.o: %.s
	@echo "  ASM   $<"
	$(V)$(AS) $(ASFLAGS) $< -o $@

.PHONY: pre-build all post-build clean run qemu qemu_gdb
