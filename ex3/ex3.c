/*************************************
* Lab 4 Exercise 3
* Name:
* Student Id: A????????
* Lab Group: B??
*************************************
Note: Duplicate the above and fill in 
for the 2nd member if  you are on a team
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mmalloc.h"

/**********************************************************
 * This is a "global" structure storing heap information
 * The "static" modifier restrict the structure to be
 *  visible only to functions in this file
 *********************************************************/
static heapMetaInfo hmi;


/**********************************************************
 * Quality of life helper functions / macros
 *********************************************************/
#define powOf2(E) (1 << E)

unsigned int log2Ceiling( unsigned int N )
/**********************************************************
 * Find the smallest S, such that 2^S >= N
 * S is returned
 *********************************************************/
{
    unsigned int s = 0, pOf2 = 1;

    while( pOf2 < N){
        pOf2 <<= 1;
        s++;
    }

    return s;
}


unsigned int log2Floor( unsigned int N )
/**********************************************************
 * Find the largest S, such that 2^S <= N
 * S is returned
 *********************************************************/
{
    unsigned int s = 0, pOf2 = 1;

    while( pOf2 <= N){
        pOf2 <<= 1;
        s++;
    }

    return s-1;
}

unsigned int buddyOf( unsigned int addr, unsigned int lvl )
/**********************************************************
 * Return the buddy address of address (addr) at level (lvl)
 *********************************************************/
{
    unsigned int mask = 0xFFFFFFFF << lvl;
    unsigned int buddyBit = 0x0001 << lvl;

    return (addr & mask) ^ buddyBit;
}

unsigned int powerOf(int base, int power) {
    if (power == 1) return base;
    return base * powerOf(base, power - 1); 
}

partInfo* buildPartitionInfo(unsigned int offset)
/**********************************************************
 * Allocate a new partInfo structure and initialize the fields
 *********************************************************/
{
    partInfo *piPtr;

    piPtr = (partInfo*) malloc(sizeof(partInfo));

    piPtr->offset = offset;
	piPtr->nextPart = NULL;

    //Buddy system's partition size is implicit
	//piPtr->size = size;

    //All available partition in buddy system is implicitly free
	//piPtr->status = FREE;

    return piPtr;
}

void printPartitionList(partInfo* piPtr)
/**********************************************************
 * Print a partition linked list
 *********************************************************/
{
	partInfo* current;
    int count = 1;
	
	for ( current = piPtr; current != NULL; 
		current = current->nextPart){
        if (count % 8 == 0){
            printf("\t");
        }
		printf("[+%5d] ", current->offset);
        count++;
        if (count % 8 == 0){
            printf("\n");
        }
	}
    printf("\n");
}

void printHeapMetaInfo()
/**********************************************************
 * Print Heap Internal Bookkeeping Information
 *********************************************************/
{
    int i;

	printf("\nHeap Meta Info:\n");
	printf("===============\n");
	printf("Total Size = %d bytes\n", hmi.totalSize);
	printf("Start Address = %p\n", hmi.base);

    for (i = hmi.maxIdx; i >=0; i--){
        printf("A[%d]: ", i);
        printPartitionList(hmi.A[i] );
    }

}

void printHeap()
/**********************************************************
 * Print the content of the entire Heap 
 *********************************************************/
{
    //Included as last debugging mechanism.
    //Print the entire heap regions as integer values.

    int* array;
    int size, i;
    
    size = hmi.totalSize / sizeof(int);
    array = (int*)hmi.base;

    for ( i = 0; i < size; i++){
        if (i % 4 == 0){
            printf("[+%5d] |", i);
        }
        printf("%8d",array[i]);
        if ((i+1) % 4 == 0){
            printf("\n");
        }
    }
}

void printHeapStatistic()
/**********************************************************
 * Print Heap Usage Statistics
 *********************************************************/
{
    //TODO: Task 4. Calculate and report the various statistics

    printf("\nHeap Usage Statistics:\n");
    printf("======================\n");

    printf("Total Space: %d bytes\n", hmi.totalSize);

   //Remember to preserve the message format!

    printf("Total Space: %d bytes\n", hmi.totalSize);
    
    printf("Total Free Partitions: %d\n", 0);
    printf("Total Free Size: %d bytes\n", 0);

    printf("Total Internal Fragmentation: %d bytes\n", 0);
}

void addPartitionAtLevel( unsigned int lvl, unsigned int offset )
/**********************************************************
 * There is just a suggested approach. You are free NOT to use this.
 *    This function adds a new free partition with "offset" at hmi.A[lvl]
 *    If buddy is found, recursively (or repeatedly) perform merging and insert
 *      at higher level
 *********************************************************/
{
    int buddy = buddyOf(offset,lvl);
    printf("buddy of %d is %d\n", offset, buddy);
    if (hmi.A[lvl] == NULL) {
      hmi.A[lvl] = buildPartitionInfo(offset);
    } else {
        partInfo* curr = hmi.A[lvl];
        while (curr->offset < offset && curr->offset != buddy) {
            curr = curr->nextPart;
        }
        if (curr->offset == buddy) {
            hmi.A[lvl] = curr->nextPart;
            if (buddy > offset) {
                addPartitionAtLevel(lvl + 1, offset);
            } else {
                addPartitionAtLevel(lvl + 1, buddy);
            }
        } else if (curr->offset > offset) {
            partInfo* new = buildPartitionInfo(offset);
            new->nextPart = curr;
            hmi.A[lvl] = new;
        }
    }
}

partInfo* removePartitionAtLevel(unsigned int lvl)
/**********************************************************
 * There is just a suggested approach. You are free NOT to use this.
 *    This function remove a free partition at hmi.A[lvl]
 *    Perform the "upstream" search if this lvl is empty AND perform
 *      the repeated split from higher level back to this level.
 * 
 * Return NULL if cannot find such partition (i.e. cannot sastify request)
 * Return the Partition Structure if found.
 *********************************************************/
{
    if (lvl > hmi.maxIdx) {
        //printf("here1\n");
        return NULL;
    } else if (hmi.A[lvl] == NULL) {
        //printf("here2\n");
        //partInfo* curr = removePartitionAtLevel(lvl + 1);
        //hmi.A[lvl] = removePartitionAtLevel(lvl + 1);;
        partInfo* prev = removePartitionAtLevel(lvl + 1);
        hmi.A[lvl] = buildPartitionInfo(prev->offset + powerOf(2, lvl));
        return prev;
        //return hmi.A[lvl];
    } else {
        //printf("here3\n");
        partInfo* curr = hmi.A[lvl];
        hmi.A[lvl] = hmi.A[lvl]->nextPart;
        //return buildPartitionInfo(powerOf(2, lvl - 1));
        return curr;
    }
}

int setupHeap(int initialSize)
/**********************************************************
 * Setup a heap with "initialSize" bytes
 *********************************************************/
{
	void* base;

	base = sbrk(0);
	if(	sbrk(initialSize) == (void*)-1){
		printf("Cannot set break! Behavior undefined!\n");
		return 0;
	}

    hmi.base = base;

	hmi.totalSize = initialSize;
    hmi.internalFragTotal = 0;
	
    //TODO: Task 1. Setup the rest of the bookkeeping info:
    //       hmi.A <= an array of partition linked list
    //       hmi.maxIdx <= the largest index for hmi.A[]
    //       
    hmi.maxIdx = log2Floor(initialSize); //change this!
    hmi.A = (partInfo**) malloc((hmi.maxIdx + 1) * sizeof(partInfo*));
    for (int i = 0; i < hmi.maxIdx; i++) {
        hmi.A[i] = NULL;
    }
    hmi.A[hmi.maxIdx] = buildPartitionInfo(0);

    return 1;
}


void* mymalloc(int size)
/**********************************************************
 * Mimic the normal "malloc()":
 *    Attempt to allocate a piece of free heap of (size) bytes
 *    Return the memory addres of this free memory if successful
 *    Return NULL otherwise 
 *********************************************************/
{
    //TODO: Task 2. Implement the allocation using buddy allocator
    int level = log2Floor(size);
    partInfo* chosen = removePartitionAtLevel(level);
    return (void*) hmi.base + chosen->offset;
}

void myfree(void* address, int size)
/**********************************************************
 * Mimic the normal "free()":
 *    Attempt to free a previously allocated memory space
 *********************************************************/
{
    //TODO: Task 3. Implement the de allocation using buddy allocator
    int level = log2Floor(size);
    int addr = (char*) address - (char*)hmi.base;
    addPartitionAtLevel(level, addr);
    printf("calculated address is : %d\n", addr);
}
