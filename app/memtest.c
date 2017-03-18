#include <tinyos.h>
#include <stdlib.h>

int main()
{
	int *m1, *m2;
	int cnt = 0;

	m2 = NULL;
	/* Allocated as much as we can, and create linked list */
	while ((m1 = malloc(10001))) {
		*(int **) m1 = m2;
		m2 = m1;
		cnt++;
	}

	/* Free all allocated memory */
	while (m2) {
		m1 = *(int **) m2;
		free(m2);
		m2 = m1;
	}
	/* Confirm that allocation can succeed now */
	m1 = malloc(10001);
	if (!m1)
		printf("mem test fail!\n");
	else
		free(m1);

	printf("mem test ok! %d\n", cnt);

	return 0;
}
