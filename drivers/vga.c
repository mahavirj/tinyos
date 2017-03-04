#include <stdint.h>
#include <string.h>
#include <helper.h>
#include <vm.h>

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

void k_write_char(char c)
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

void k_write(const char *buf)
{
	while (*buf)
		k_write_char(*buf++);
}

void k_write_dec(const int num)
{
	int quot, i;
	char rem;
	char str[32];

	quot = num;
	i = 0;
	do {
		rem = (quot % 10) + '0';
		quot /= 10;
		str[i++] = rem;
	} while (quot);

	int start, end;
	char temp;
	for (end = i - 1, start = 0; start < end; end--, start++) {
		temp = str[start];
		str[start] = str[end];
		str[end] = temp;
	}

	str[i] = '\0';
	k_write(str);
}

void k_write_hex(const int num)
{
	int index = 32;
	int val;

	do {
		index -= 4;
		val = (num >> index) & 0xf;
	} while (!val && index);

	k_write("0x");
	do {
		if (val > 9)
			val = 'A' + (val - 10);
		else
			val += '0';
		k_write_char(val);
		index -= 4;
		val = (num >> index) & 0xf;
	} while (index >= 0);
}

void k_video_init()
{
	fb = (uint16_t *) (KERNBASE + 0xB8000);
	cls();
}
