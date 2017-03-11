#include <stdint.h>
#include <string.h>
#include <helper.h>
#include <vm.h>
#include <syscall.h>

extern void memsetw(void *src, uint16_t value, size_t size);

#define FB_CMD_PORT 0x3d4
#define FB_DATA_PORT 0x3d5
#define FB_CMD_HIGH 14
#define FB_CMD_LOW 15
#define WHITE_COLOR 0xf
#define BLACK_COLOR 0x0
#define COLOR_ATTR (BLACK_COLOR << 4 | WHITE_COLOR)

static uint16_t *fb;
static int _x, _y;

/* If screen is full, empty last line */
static void scroll()
{
	uint16_t blank;

	blank = 0x20 | (COLOR_ATTR << 8);
	if (_y >= 25) {
		/* Ideally overlapping regions are not supported by memcpy, but here source
		 * is greater than destination and hence there is no source overwrite issue
		 */
		memcpy(fb, fb + 80, (24 * 80 * 2));
		/* Set last line as blank */
		memsetw(fb + (24 * 80), blank, 80);
		_y = 24;
	}
}

static void move_cursor(int x, int y)
{
	int pos = y * 80 + x;

	outportb(FB_CMD_PORT, FB_CMD_HIGH);
	outportb(FB_DATA_PORT, (pos >> 8 & 0xff));
	outportb(FB_CMD_PORT, FB_CMD_LOW);
	outportb(FB_DATA_PORT, (pos & 0xff));
}

/* Clears the screen */
static void cls()
{
	uint16_t blank;
	int i;

	/* Again, we need the 'short' that will be used to
	 * represent a space with color */
	blank = 0x20 | (COLOR_ATTR << 8);

	/* Sets the entire screen to spaces in our current
	 * color */
	for (i = 0; i < 25; i++)
		memsetw (fb + i * 80, blank, 80);

	/* Update out virtual cursor, and then move the
	 * hardware cursor */
	_x = 0;
	_y = 0;
	move_cursor(_x, _y);
}

void sys_write_char(char c)
{
	uint16_t *loc = fb + (_y * 80 + _x);
	uint8_t attr = COLOR_ATTR;

	switch (c) {
	case '\n':
		_x = 0;
		_y++;
		break;
	case '\r':
		_x = 0;
		break;
	case ' ':
		_x++;
		break;
	case 0x8:
		_x--;
		break;
	default:
		*loc = c | (attr << 8);
		_x++;
		break;
	}

	if (_x >= 80) {
		_x = 0;
		_y++;
	}

	scroll();
	move_cursor(_x, _y);
}

int sys_write()
{
	const char *buf;
	int len;

	argstr(1, (char **) &buf);
	argint(2, &len);

	if (!buf || !len)
		return 0;

	int i;
	for (i = 0; i < len; i++)
		sys_write_char(buf[i]);
	return len;
}

void init_vga()
{
	fb = (uint16_t *) (KERNBASE + 0xB8000);
	cls();
}
