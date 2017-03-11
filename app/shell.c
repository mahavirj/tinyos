#include <tinyos.h>

int a = 0xa5a5;
int main()
{
	if (a == 0xa5a5)
		printf("I am shell\n");
	else
		printf("I lost you\n");
	for(;;);
}
