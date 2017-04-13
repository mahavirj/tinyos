#ifndef __ARCH_H__
#define __ARCH_H__

void arch_init(void);

static inline void arch_enable_irq()
{
#ifdef CONFIG_X86
	__asm volatile("sti");
#elif CONFIG_ARM
	__asm volatile("cpsie i" : : : "memory");
#endif
}

static inline void arch_disable_irq()
{
#ifdef CONFIG_X86
	__asm volatile("cli");
#elif CONFIG_ARM
	__asm volatile("cpsid i" : : : "memory");
#endif
}

static inline void arch_halt()
{
#ifdef CONFIG_X86
	__asm volatile("hlt");
#elif CONFIG_ARM
	__asm volatile("wfi");
#endif
}

#endif /* __ARCH_H__ */
