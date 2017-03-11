#include <tinyos.h>
#include <unistd.h>

char a[] = "Hello World Parent!\n";
char b[] = "Hello World Child!\n";

int main(void)
{
	char *argv = NULL;
	printf("User application started\n");
	int ret = fork();
	if (ret == 0) {
		execve("shell", &argv, NULL);
	} else {
		for (;;) {
			printf("%s", a);
			volatile int a = 1 << 23;
			while (a--)
				;
		}
	}
	return 0;
}
