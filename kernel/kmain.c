int kmain(void)
{
	k_video_init();
	int i, j;

	for (i = 0; i < 25; i++)
		for (j = 0; j < 80; j++)
			k_write_char('A' + i);
	//k_video_init();
	return 0;
}
