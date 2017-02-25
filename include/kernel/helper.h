#ifndef __HELPER_H__
#define __HELPER_H__

/* Enable interrupts on local processor */
static inline void sti()
{
	asm volatile("sti");
}

/* Disable interrupts on local processor */
static inline void cli()
{
	asm volatile("cli");
}

static inline unsigned char inportb (unsigned short _port)
{
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

static inline void outportb (unsigned short _port, unsigned char _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

extern void spin_lock();
extern void spin_unlock();

#endif /* __HELPER_H__ */
