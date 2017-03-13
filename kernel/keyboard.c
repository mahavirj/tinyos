#include <isr.h>
#include <helper.h>
#include <keyboard.h>
#include <vga.h>
#include <sync.h>
#include <mem.h>
#include <syscall.h>
#include <string.h>

/* KBDUS means US Keyboard Layout. This is a scancode table
 *  used to layout a standard US keyboard. I have left some
 *  comments in to give you an idea of what key is what, even
 *  though I set it's array index to 0. You can change that to
 *  whatever you want using a macro, if you wish! */
unsigned char kbdus[128] =
{
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
	'9', '0', '-', '=', '\b',	/* Backspace */
	'\t',			/* Tab */
	'q', 'w', 'e', 'r',	/* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
	0,			/* 29   - Control */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
	'\'', '`',   0,		/* Left shift */
	'\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
	'm', ',', '.', '/',   0,				/* Right shift */
	'*',
	0,	/* Alt */
	' ',	/* Space bar */
	0,	/* Caps lock */
	0,	/* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0,
	0,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0,	/* Left Arrow */
	0,
	0,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */
};

#define MAX_SIZE 32

#define WRAP(x) \
	((x + 1) & (MAX_SIZE - 1))

#define CIRC_BUF_EMPTY(x) \
	(x->rindex == x->windex)

#define CIRC_BUF_FULL(x) \
	(x->rindex == WRAP(x->windex + 1))

struct circ_buf {
	struct spinlock lock;
	char buf[MAX_SIZE];
	int rindex;
	int windex;
};

static struct circ_buf *circ_buf;

/* Handles the keyboard interrupt */
void keyboard_handler(registers_t *r)
{
	(void) r;
	unsigned char scancode;

	/* Read from the keyboard's data buffer */
	scancode = inportb(0x60);

	/* If the top bit of the byte we read from the keyboard is
	 *  set, that means that a key has just been released */
	if (scancode & 0x80) {
		/* Special keyboard chars */
	} else {
		/* Here, a key was just pressed. Please note that if you
		 *  hold a key down, you will get repeated key press
		 *  interrupts. */
		sys_write_char(kbdus[scancode]);
		if (!CIRC_BUF_FULL(circ_buf)) {
			circ_buf->buf[circ_buf->windex] = kbdus[scancode];
			circ_buf->windex = WRAP(circ_buf->windex);
			wakeup(&circ_buf->lock);
		}
	}
}

void init_keyboard()
{
	circ_buf = kcalloc(sizeof(struct circ_buf));
	if (!circ_buf) {
		printk("malloc failed\n");
		return;
	}
	/* Installs 'keyboard_handler' to IRQ1 */
	irq_install_handler(IRQ1, keyboard_handler);
}

int sys_read()
{
	char *buf;
	int len;
	int read_bytes = 0;

	argstr(1, &buf);
	argint(2, &len);

	if (CIRC_BUF_EMPTY(circ_buf))
		sleep(circ_buf, &circ_buf->lock);

	while (!CIRC_BUF_EMPTY(circ_buf) && (read_bytes < len)) {
		buf[read_bytes++]  = circ_buf->buf[circ_buf->rindex];
		circ_buf->rindex = WRAP(circ_buf->rindex);
	}
	return read_bytes;
}
