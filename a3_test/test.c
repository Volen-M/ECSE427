
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
    // while(NULL){
    //     printf("In loop\n");

    // }
    // if (NULL){
    //     printf("in if\n");
    // }
    // if(!NULL){
    //     printf("in Not null if\n");
    // }
    // printf("out of loop\n");
    int arr[5];
    int i = sizeof(int);
    printf("%d\n", i);  
    printf("-------\n");
    int * ptr = arr;
    printf("%p\n", ptr);   
    printf("-------\n");
    int *ptr2 = ptr+sizeof(int)/4;
    printf("%p\n", ptr2);    
    printf("-------\n");
    int *ptr3 = &arr[1];
    printf("%p\n", ptr3);    
    printf("-------\n");
    ptr++;
    printf("%p\n", ptr);    
    printf("-------\n");
}