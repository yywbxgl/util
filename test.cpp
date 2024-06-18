#include <iostream>
#include <unistd.h>
#include "hello.h"


int main()
{
	printf("---hello\n");

	while(1) {
		hello_world(123);
		sleep(1);
	}
	return 0;
}
