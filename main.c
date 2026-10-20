#include <stdio.h>

extern int fun;

int main() {
	char name[100];

	printf("What is your name? ");

	if (scanf("%99s", name) == 1)
		printf("Hello, %s!\n", name);

	printf("printing value declared in fun.c: %d\n",fun);	
}
