To run tests write:
    > make sma
    > gcc sma.o a3_test.c 
    > ./a.out

To simply compile using MakeFile:
    > make sma

To compile yourself and run tests directly:
    > gcc a3_test.c sma.c -o testSma
    > ./testSma

    
-------------- TEST INFO -------------------
NO TESTS WERE MODIFIED ONLY PUT STATEMENTS HAVE BEEN ADDED
Only test 5 does not pass.
Test were run in a controlled environment on a Linux Virtual Machine.
An output file with all the print has been included "output.txt"


-------------- CODE INFO --------------------
All internally passed pointers are header pointers. get_blockSize() was as such changed to fit the standard. 
The SMA Statistics tests only show the space not covered by headers.
An extra function called void check_for_possible_free_merges() was created to merge the freed blocks.
Both allocated and freed blocks share the same headers. 
The doubly linked list includes both allocated and free blocks which are differentiate by an isFree int.
The first memory creates a space of 12Kbytes as demanded.