char a[] = "Hello World Parent!\n";
char b[] = "Hello World Child!\n";

int main(void)
{
	write("User application started\n", 25);
	int ret = fork();
	if (ret == 0) {
		for (;;) {
			write(b, sizeof(b)-1);
			volatile int a = 1 << 20;
			while (a--)
				;
		}
	} else {
		for (;;) {
			write(a, sizeof(a)-1);
			volatile int a = 1 << 20;
			while (a--)
				;
		}
	}
	return 0;
}
