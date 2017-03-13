#include <tinyos.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
	char *argv = NULL;
	printf("User application started\n");
	int ret = fork();
	if (ret == 0) {
		execve("shell", &argv, NULL);
	} else {
		waitpid(-1, NULL, 0);
		printf("Bye! Init exiting\n");
	}
	return 0;
}
