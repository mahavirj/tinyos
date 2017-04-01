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

$(objdir)/%.o: %.c
	@echo "  CC    $<"
	$(V)$(CC) -c $(CFLAGS) $< -o $@

$(objdir)/%.o: %.s
	@echo "  ASM   $<"
	$(V)$(AS) $(ASFLAGS) $< -o $@

.PHONY: all pre-build clean run
