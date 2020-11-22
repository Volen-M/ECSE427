/*
 * =====================================================================================
 *
 *	Filename:  		sma.c
 *
 *  Description:	Base code for Assignment 3 for ECSE-427 / COMP-310
 *
 *  Version:  		1.0
 *  Created:  		6/11/2020 9:30:00 AM
 *  Revised:  		-
 *  Compiler:  		gcc
 *
 *  Author:  		Mohammad Mushfiqur Rahman
 *      
 *  Instructions:   Please address all the "TODO"s in the code below and modify 
 * 					them accordingly. Feel free to modify the "PRIVATE" functions.
 * 					Don't modify the "PUBLIC" functions (except the TODO part), unless
 * 					you find a bug! Refer to the Assignment Handout for further info.
 * =====================================================================================
 */

/* Includes */
#include "sma.h" // Please add any libraries you plan to use inside this file
#include <pthread.h>

/* Definitions*/
#define MAX_TOP_FREE (128 * 1024) // Max top free block size = 128 Kbytes
//	TODO: Change the Header size if required
#define HEADER_SIZE 2 * sizeof(char *) + 2 * sizeof(int) // Size of the Header in a free memory block
//	TODO: Add constants here

#define INITIAL_BREAK_AMOUNT (12 * 1024) // 12 Kbytes
typedef char ALIGN[HEADER_SIZE];

union header
{
	struct
	{
		int size; //Size dissincludes header
		int isFree;
		union header *prev;
		union header *next;
	} s;
	ALIGN x;
};
typedef union header header_t;


typedef enum //	Policy type definition
{
	WORST,
	NEXT
} Policy;

char *sma_malloc_error;
void *freeListHead = NULL;			  //	The pointer to the HEAD of the doubly linked free/allocated memory list
void *freeListTail = NULL;			  //	The pointer to the TAIL of the doubly linked free/allocated memory list
unsigned long totalAllocatedSize = 0; //	Total Allocated memory in Bytes
unsigned long totalFreeSize = 0;	  //	Total Free memory in Bytes in the free memory list
Policy currentPolicy = WORST;		  //	Current Policy
//	TODO: Add any global variables here
void *nextFitCurrentHeader = NULL;
short hasInitialized = 0; //		To check that MAX_TOP_FREE of free spaces has already been prog breaked

/*
 * =====================================================================================
 *	Public Functions for SMA
 * =====================================================================================
 */

/*
 *	Funcation Name: sma_malloc
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates a memory block of input size from the heap, and returns a 
 * 					pointer pointing to it. Returns NULL if failed and sets a global error.
 */
void *sma_malloc(int size)
{
	void *pMemory = NULL;
	if (size <= 0)
	{
		return NULL;
	}
	// Checks if the free list is empty
	if (freeListHead == NULL)
	{
		// Allocate memory by increasing the Program Break
		pMemory = allocate_pBrk(size);
	}
	// If free list is not empty
	else
	{

		// Allocate memory from the free memory list
		pMemory = allocate_freeList(size);

		// If a valid memory could NOT be allocated from the free memory list
		if (pMemory == (void *)-2)
		{	
			// Allocate memory by increasing the Program Break
			pMemory = allocate_pBrk(size);
		}
	}

	// Validates memory allocation
	if (pMemory < 0 || pMemory == NULL)
	{
		sma_malloc_error = "Error: Memory allocation failed!";
		return NULL;
	}
	// pMemory+=ALLOCATED_BLOCK_HEADER_SIZE/4;
	// Updates SMA Info
	totalAllocatedSize += size;
	
	// char str[60];
	// sprintf(str,"malloc: %p, %d, isFree:%d", pMemory, ((header_t*)pMemory)->s.size,((header_t*)pMemory)->s.isFree);
	// puts(str);
	return ((void *)(pMemory + (HEADER_SIZE / 4)));
}

/*
 *	Funcation Name: sma_free
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Deallocates the memory block pointed by the input pointer
 */
void sma_free(void *ptr)
{
	//	Checks if the ptr is NULL
	if (ptr == NULL)
	{
		puts("Error: Attempting to free NULL!");
	}
	//	Checks if the ptr is beyond Program Break
	else if (ptr > sbrk(0))
	{
		puts("Error: Attempting to free unallocated space!");
	}
	else
	{
		//	Adds the block to the free memory list
		add_block_freeList( (void *)(ptr -(HEADER_SIZE/4)) , 1);
	}
}

/*
 *	Funcation Name: sma_mallopt
 *	Input type:		int
 * 	Output type:	void
 * 	Description:	Specifies the memory allocation policy
 */
void sma_mallopt(int policy)
{
	// Assigns the appropriate Policy
	if (policy == 1)
	{
		nextFitCurrentHeader = NULL;
		currentPolicy = WORST;
	}
	else if (policy == 2)
	{
		currentPolicy = NEXT;
	}
}

/*
 *	Funcation Name: sma_mallinfo
 *	Input type:		void
 * 	Output type:	void
 * 	Description:	Prints statistics about current memory allocation by SMA.
 */
void sma_mallinfo()
{
	//	Finds the largest Contiguous Free Space (should be the largest free block)
	int largestFreeBlock = get_largest_freeBlock();
	char str[60];

	//	Prints the SMA Stats
	sprintf(str, "Total number of bytes allocated: %lu", totalAllocatedSize);
	puts(str);
	sprintf(str, "Total free space: %lu", totalFreeSize);
	puts(str);
	sprintf(str, "Size of largest contigious free space (in bytes): %d", largestFreeBlock);
	puts(str);
}

/*
 *	Funcation Name: sma_realloc
 *	Input type:		void*, int
 * 	Output type:	void*
 * 	Description:	Reallocates memory pointed to by the input pointer by resizing the
 * 					memory block according to the input size.
 */
void *sma_realloc(void *ptr, int size)
{
	// TODO: 	Should be similar to sma_malloc, except you need to check if the pointer address
	//			had been previously allocated.
	// Hint:	Check if you need to expand or contract the memory. If new size is smaller, then
	//			chop off the current allocated memory and add to the free list. If new size is bigger
	//			then check if there is sufficient adjacent free space to expand, otherwise find a new block
	//			like sma_malloc.
	//			Should not accept a NULL pointer, and the size should be greater than 0.

	//USER CODE BEGINS
	void *blockHeader = NULL;

	if (ptr == NULL || size <= 0)
	{
		return sma_malloc(size);
	}
	blockHeader = (void *)(ptr -(HEADER_SIZE/4)) ;
	if (((header_t*)blockHeader)->s.size >= size)
	{
		void *excessFreeBlock; 
		excessFreeBlock = (void*)(blockHeader + ((size + HEADER_SIZE)/4));
		((header_t *)excessFreeBlock)->s.size = ((header_t*)blockHeader)->s.size-size;
		((header_t *)excessFreeBlock)->s.isFree = 1;
		((header_t *)blockHeader)->s.size = size;
		replace_block_freeList(blockHeader,excessFreeBlock);
		return ptr;
	}
	
	add_block_freeList( (void *)(ptr -(HEADER_SIZE/4)) , 1);
	void *pMemory = NULL; 
	pMemory = allocate_freeList(size);

	// If a valid memory could NOT be allocated from the free memory list
	if (pMemory == (void *)-2)
	{	
		// Allocate memory by increasing the Program Break
		pMemory = allocate_pBrk(size);
	}
	totalAllocatedSize += size;
	//memcpy((void *)(pMemory + (HEADER_SIZE / 4)),ptr,((header_t*)blockHeader)->s.size );
	return ((void *)(pMemory + (HEADER_SIZE / 4)));
	//USER CODE ENDS
}

/*
 * =====================================================================================
 *	Private Functions for SMA
 * =====================================================================================
 */



/*
 *	Funcation Name: allocate_pBrk
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory by increasing the Program Break
 */
void *allocate_pBrk(int size)
{
	void *newBlock = NULL;
	int excessSize;

	//Start User Code
	if (hasInitialized == 0)
	{
		excessSize = INITIAL_BREAK_AMOUNT;
		hasInitialized = 1;
	}
	else
	{
		excessSize = 0;
	}
	//End User Code

	//	TODO: 	Allocate memory by incrementing the Program Break by calling sbrk() or brk()
	//	Hint:	Getting an exact "size" of memory might not be the best idea. Why?
	//			Also, if you are getting a larger memory, you need to put the excess in the free list

	//Start User Code
	int total_size = HEADER_SIZE + size + excessSize;
	newBlock = sbrk(total_size);
	if (newBlock == (void *)-1)
	{
		return NULL;
	}

	if (freeListHead == NULL)
	{
		freeListHead = newBlock;
	}

	if (freeListTail != NULL)
	{
		((header_t *)freeListTail)->s.next = newBlock;
		((header_t *)newBlock)->s.prev = freeListTail;
		((header_t *)newBlock)->s.next = freeListHead;
		freeListTail = newBlock;
	}
	else
	{
		freeListTail = newBlock;
		if (freeListHead == newBlock)
		{
			((header_t *)newBlock)->s.prev = newBlock;
			((header_t *)newBlock)->s.next = newBlock;
		}
		else
		{
			puts("This should not happen");
		}
	}
	((header_t *)newBlock)->s.size = size;
	//End User Code

	//	Allocates the Memory Block
	allocate_block(newBlock, size, excessSize, 0);

	return newBlock;
}

/*
 *	Funcation Name: allocate_freeList
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory from the free memory list
 */
void *allocate_freeList(int size) //Size passed dissincludes header
{
	void *pMemory = NULL;

	if (currentPolicy == WORST)
	{
		// Allocates memory using Worst Fit Policy
		pMemory = allocate_worst_fit(size);
	}
	else if (currentPolicy == NEXT)
	{
		// Allocates memory using Next Fit Policy
		pMemory = allocate_next_fit(size);
	}
	else
	{
		pMemory = NULL;
	}

	return pMemory;
}

/*
 *	Funcation Name: allocate_worst_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Worst Fit from the free memory list
 */
void *allocate_worst_fit(int size) //Size passed dissincludes header
{
	void *worstBlock = NULL;
	int excessSize;
	int blockFound = 0;
	char str[80];


	//	TODO: 	Allocate memory by using Worst Fit Policy
	//	Hint:	Start off with the freeListHead and iterate through the entire list to get the largest block

	//User Code Start
	void *studiedBlock = freeListHead;
	if(((header_t *)studiedBlock)->s.isFree != 1){
		for(;;){
			studiedBlock = ((header_t *)studiedBlock)->s.next;
			if(((header_t *)studiedBlock)->s.isFree== 1){
				break;
			}

			if(studiedBlock == freeListTail){
				break;
			}
		}
	}
	worstBlock = studiedBlock;

	int firstPass = 0; //To avoid if there is only one free spot which points at itself
	if (studiedBlock != NULL && ((header_t *)studiedBlock)->s.isFree == 1)
	{
		for (;;)
		{
			studiedBlock = ((header_t *)studiedBlock)->s.next;
			if (((header_t *)studiedBlock)->s.isFree == 1 && ((header_t *)studiedBlock)->s.size > ((header_t *)worstBlock)->s.size)
			{
				worstBlock = studiedBlock;
			}
			if(studiedBlock == freeListTail){
				break;
			}
		}
	}
	if (((header_t *)worstBlock)->s.size >= size && ((header_t *)worstBlock)->s.isFree == 1)
	{
		blockFound = 1;
	}
	//User Code End

	//	Checks if appropriate block is found.
	if (blockFound)
	{
		//	Allocates the Memory Block
		excessSize = ((header_t *)worstBlock)->s.size - size;
		allocate_block(worstBlock, size, excessSize, 1);
	}
	else
	{
		//	Assigns invalid address if appropriate block not found in free list
		worstBlock = (void*)-2;
	}

	return worstBlock;
}

/*
 *	Funcation Name: allocate_next_fit
 *	Input type:		int
 * 	Output type:	void*
 * 	Description:	Allocates memory using Next Fit from the free memory list
 */
void *allocate_next_fit(int size) //Size passed dissincludes header
{
	void *nextBlock = NULL;
	int excessSize;
	int blockFound = 0;

	//	TODO: 	Allocate memory by using Next Fit Policy
	//	Hint:	Start off with the freeListHead, and keep track of the current position in the free memory list.
	//			The next time you allocate, it should start from the current position.

	//User Code Start
	if (nextFitCurrentHeader == NULL)
	{
		nextFitCurrentHeader = freeListHead;
	}
	nextBlock = nextFitCurrentHeader;
	if(((header_t *)nextBlock)->s.isFree != 1 ||  ((header_t *)nextBlock)->s.size < size){
		for (;;)
		{	
			nextBlock = ((header_t *)nextBlock)->s.next;
			if (((header_t *)nextBlock)->s.isFree == 1 &&  ((header_t *)nextBlock)->s.size >= size)
			{
				nextFitCurrentHeader = nextBlock;
				break;
			}
			if (nextBlock == nextFitCurrentHeader){
				break;
			}
		}
	}

	if (((header_t *)nextBlock)->s.size  >= size && ((header_t *)nextBlock)->s.isFree == 1)
	{
		blockFound = 1;
	}
	//User Code End

	//	Checks if appropriate found is found.
	if (blockFound)
	{
		//	Allocates the Memory Block
		excessSize = ((header_t *)nextBlock)->s.size - size;
		allocate_block(nextBlock, size, excessSize, 1);
	}
	else
	{
		//	Assigns invalid address if appropriate block not found in free list
		nextBlock=(void*)-2;
	}


	return nextBlock;
}

/*
 *	Funcation Name: allocate_block
 *	Input type:		void*, int, int, int
 * 	Output type:	void
 * 	Description:	Performs routine operations for allocating a memory block
 */
void allocate_block(void *newBlock, int size, int excessSize, int fromFreeList) //Size passed dissincludes header
{
	void *excessFreeBlock; //	pointer for any excess free block
	int addFreeBlock;

	// 	Checks if excess free size is big enough to be added to the free memory list
	//	Helps to reduce external fragmentation

	//	TODO: Adjust the condition based on your Head and Tail size (depends on your TAG system)
	//	Hint: Might want to have a minimum size greater than the Head/Tail sizes
	addFreeBlock = excessSize > HEADER_SIZE;

	//USER CODE BEGIN
	((header_t *)newBlock)->s.isFree = 0;
	//USER CODE END

	//	If excess free size is big enough
	if (addFreeBlock)
	{
		//	TODO: Create a free block using the excess memory size, then assign it to the Excess Free Block
		
		//User Code Starts
		((header_t *)newBlock)->s.size = size;
		excessFreeBlock = (void*)(newBlock + ((((header_t *)newBlock)->s.size + HEADER_SIZE)/4));
		((header_t *)excessFreeBlock)->s.size = excessSize-HEADER_SIZE;
		((header_t *)excessFreeBlock)->s.isFree = 1;
		//User Code Ends

		//	Checks if the new block was allocated from the free memory list

		if (fromFreeList)
		{
			//	Removes new block and adds the excess free block to the free list
			replace_block_freeList(newBlock, excessFreeBlock);
		}
		else
		{
			//	Adds excess free block to the free list
			add_block_freeList(excessFreeBlock, 0);
			remove_block_freeList(newBlock);
		}
	}
	//	Otherwise add the excess memory to the new block
	else
	{
		//	TODO: Add excessSize to size and assign it to the new Block
		((header_t *)newBlock)->s.size = size + excessSize;
		//	Checks if the new block was allocated from the free memory list
		if (fromFreeList)
		{
			//	Removes the new block from the free list
			remove_block_freeList(newBlock);
		}
	}
}

/*
 *	Funcation Name: replace_block_freeList
 *	Input type:		void*, void*
 * 	Output type:	void
 * 	Description:	Replaces old block with the new block in the free list (it actually just adds it after)
 */
void replace_block_freeList(void *oldBlock, void *newBlock)
{
	//	TODO: Replace the old block with the new block

	//User Code Start
	((header_t *)newBlock)->s.next = ((header_t *)oldBlock)->s.next;
	((header_t *)oldBlock)->s.next = newBlock;
	((header_t *)newBlock)->s.prev = oldBlock;
	((header_t *)((header_t *)newBlock)->s.next)->s.prev = newBlock;
	//User Code End
	check_for_possible_free_merges();
	//	Updates SMA info
	totalAllocatedSize += (get_blockSize(oldBlock) - get_blockSize(newBlock));
	totalFreeSize += (get_blockSize(newBlock) - get_blockSize(oldBlock));
}

/*
 *	Funcation Name: add_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Adds a memory block to the the free memory list
 */
void add_block_freeList(void *block, int fromSmaFree)
{
	//	TODO: 	Add the block to the free list
	//	Hint: 	You could add the free block at the end of the list, but need to check if there
	//			exits a list. You need to add the TAG to the list.
	//			Also, you would need to check if merging with the "adjacent" blocks is possible or not.
	//			Merging would be tideous. Check adjacent blocks, then also check if the merged
	//			block is at the top and is bigger than the largest free block allowed (128kB).

	//User Code Start

	((header_t *)block)->s.isFree = 1;
	if(fromSmaFree == 0){
		if (freeListHead == NULL)
		{
			freeListHead = block;
		}

		if (freeListTail != NULL)
		{
			((header_t *)freeListTail)->s.next = block;
			((header_t *)block)->s.prev = freeListTail;
			((header_t *)block)->s.next = freeListHead;
			((header_t *)freeListTail)->s.prev = block;
			freeListTail = block;
		}
		else
		{
			freeListTail = block;
			if (freeListHead == block)
			{
				((header_t *)block)->s.prev = block;
				((header_t *)block)->s.next = block;
			}
			else
			{
				puts("This should not happen");
			}
		}
	}
	if(block == nextFitCurrentHeader){
		nextFitCurrentHeader == NULL;
	}
	check_for_possible_free_merges();
	//User Code End

	//	Updates SMA info
	if (fromSmaFree==1){
		totalAllocatedSize -= get_blockSize(block);
	}
	totalFreeSize += get_blockSize(block);
}

/*
 *	Funcation Name: remove_block_freeList
 *	Input type:		void*
 * 	Output type:	void
 * 	Description:	Removes a memory block from the the free memory list
 */
void remove_block_freeList(void *block)
{
	//	TODO: 	Remove the block from the free list
	//	Hint: 	You need to update the pointers in the free blocks before and after this block.
	//			You also need to remove any TAG in the free block.

	//User Code Start
	((header_t *)block)->s.isFree = 0;
	//User Code End

	//	Updates SMA info
	totalAllocatedSize += get_blockSize(block);
	totalFreeSize -= get_blockSize(block);
}

/*
 *	Funcation Name: get_blockSize
 *	Input type:		void*
 * 	Output type:	int
 * 	Description:	Extracts the Block Size
 */
int get_blockSize(void *ptr)
{
	// int *pSize;

	// //	Points to the address where the Length of the block is stored
	// pSize = (int *)ptr;
	// pSize--;

	//User Code Begins
	int *pSize = ptr;
	//Use Code Ends

	//	Returns the deferenced size
	return *(int *)pSize;
}

/*
 *	Funcation Name: get_largest_freeBlock
 *	Input type:		void
 * 	Output type:	int
 * 	Description:	Extracts the largest Block Size
 */
int get_largest_freeBlock()
{

	//	TODO: Iterate through the Free Block List to find the largest free block and return its size

	//User Code Starts
	void *worstBlock = NULL;
	void *studiedBlock = freeListHead;
	if(((header_t *)studiedBlock)->s.isFree != 1){
		for(;;){
			studiedBlock = ((header_t *)studiedBlock)->s.next;
			if(((header_t *)studiedBlock)->s.isFree== 1){
				break;
			}

			if(studiedBlock == freeListTail){
				break;
			}
		}
	}
	worstBlock = studiedBlock;

	if (studiedBlock != NULL && ((header_t *)studiedBlock)->s.isFree == 1)
	{
		for (;;)
		{
			studiedBlock = ((header_t *)studiedBlock)->s.next;
			if (((header_t *)studiedBlock)->s.isFree == 1 && ((header_t *)studiedBlock)->s.size > ((header_t *)worstBlock)->s.size)
			{
				worstBlock = studiedBlock;
			}
			if(studiedBlock == freeListTail){
				break;
			}
		}
	}
	if (((header_t *)worstBlock)->s.isFree == 1)
	{
		return ((header_t *)worstBlock)->s.size;
	}
	//User Code Ends

	return 0;
}
/*
 *	Funcation Name: check_for_possible_free_merges
 * 	Output type:	void
 * 	Description:	Merges all adjacent free blocks
 */ 
void check_for_possible_free_merges()
{	
	void *studiedBlock = freeListHead;
	void* nextBlock;
	if(((header_t *)studiedBlock)->s.isFree != 1){
		for(;;){
			studiedBlock = ((header_t *)studiedBlock)->s.next;
			if(((header_t *)studiedBlock)->s.isFree== 1){
				break;
			}

			if(studiedBlock == freeListTail){
				break;
			}
		}
	}

	int firstPass = 0; //To avoid if there is only one free spot which points at itself
	if (studiedBlock != NULL && ((header_t *)studiedBlock)->s.isFree == 1)
	{
		for (;;)
		{	
			nextBlock = ((header_t *)studiedBlock)->s.next;
			if(((header_t *)studiedBlock)->s.isFree == 1){
				for(;;){
					if(nextBlock < sbrk(0) && ((header_t *)nextBlock)->s.isFree == 1){
						((header_t *)studiedBlock)->s.size += ((header_t *)nextBlock)->s.size+HEADER_SIZE;
						((header_t *)studiedBlock)->s.next = ((header_t *)nextBlock)->s.next;
						((header_t *)((header_t *)studiedBlock)->s.next)->s.prev = studiedBlock;
						if(nextBlock == freeListTail){
							freeListTail = studiedBlock;
							break;
						}
					}
					else
					{
						break;
					}
					nextBlock = ((header_t *)studiedBlock)->s.next;
				}
			}
			studiedBlock = ((header_t *)studiedBlock)->s.next;
			if(studiedBlock == freeListTail){
				break;
			}
		}
	}
}