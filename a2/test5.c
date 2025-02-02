#include "sut.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void hello1() {
    int i;
    char *str;
    sut_open("0.0.0.0", 12000);
    for (i = 0; i < 10; i++) {
	str = sut_read();
	if (strlen(str) != 0)
	    printf("I am SUT-One, message from server: %s\n", str);
	else
	    printf("ERROR!, empty message received \n");
	sut_yield();
    }
    sut_exit();
}

void hello2() {
    int i;
    for (i = 0; i < 100; i++) {
	//printf("Hello world!, this is SUT-Two \n");
	sut_yield();
    usleep(1000000);
    }
    sut_exit();
}

int main() {
    sut_init();
    sut_create(hello1);
    sut_create(hello2);
    sut_shutdown();
}
