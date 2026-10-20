#include <stdio.h>

extern int fun;

int main() {
	printf("%d\n",fun);	
	char name[100];

	printf("What is your name? ");

	if (scanf("%99s", name) == 1)
		printf("Hello, %s!\n", name);
}
