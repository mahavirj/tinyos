#include <tinyos.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define N 32

void forktest(void)
{
	int n, pid;

	printf("fork test\n");

	for (n=0; n<N; n++) {
		pid = fork();
		if(pid < 0) {
			break;
		}
		if(pid == 0) {
			exit(0);
		}
	} 

	if (n == N) {
		printf("fork claimed to work N times!\n", N);
	}
}

int main(void)
{
	forktest();
	waitpid(-1, NULL, 0);
	return 0;
}
