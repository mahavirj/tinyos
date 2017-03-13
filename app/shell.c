#include <tinyos.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main()
{
	int ret;
	int c;
	char buf[32];
	int index = 0;

	printf("\nStarting sh\n");
	printf("Type `help` for commands\n");
	printf("# ");
	for (;;) {
		ret = read(0, &c, 1);
		if (!ret)
			continue;
		buf[index++] = c;
		if (c == '\n') {
			buf[--index] = '\0';
			ret = fork();
			if (ret == 0) {
				if (!strcmp(buf, "help"))
					printf("Available commands 'forktest'\n");
				else
					execve(buf, (char **) &ret, NULL);
				return -1;
			} else {
				waitpid(-1, NULL, 0);
				index = 0;
				printf("# ");
			}
		}
	}
	printf("Bye! Shell exiting\n");
	return 0;
}
