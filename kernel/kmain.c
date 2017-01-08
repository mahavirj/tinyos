#include <vga.h>

int kmain(void)
{
	int i;

	k_video_init();
	for (i = 0; i < 24; i++) {
		k_write("Hello World: ");
		k_write_dec(i * 250000);
		k_write("  ");
		k_write_hex(i * 128);
		k_write("\n");
	}
	return 0;
}
