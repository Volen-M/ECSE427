#include "sut.h"
#include <stdio.h>

void hello1() {
    int i;
    for (i = 0; i < 100; i++) {
	printf("Hello world!, this is SUT-One: %d \n", i);
	sut_yield();
    }
    sut_exit();
}

void hello2() {
    int i;
    for (i = 0; i < 100; i++) {
	printf("Hello world!, this is SUT-Two: %d \n", i);
	sut_yield();
    }
    sut_exit();
}

int main() {
    printf("In test 1 main\n");
    sut_init();
    printf("Back In test 1 main\n");
    sut_create(hello1);
    sut_create(hello2);
    printf("right before shutdown call\n");
    sut_shutdown();
}
