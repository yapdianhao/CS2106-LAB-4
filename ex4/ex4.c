/*************************************
 * Lab 4 Exercise 3
 * Name: Yap Dian Hao
 * Student Id: A0184679H
 * Lab Group: B13
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

int internalFragmentationSize = 0;
int allocMinSize;
int allocMaxSize;
int allocatedSize = 0;

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
    int freeSize = 0;
    int freePartitions = 0;
    for (int i = 0; i < hmi.maxIdx + 1; i++) {
        partInfo* curr = hmi.A[i];
        while (curr != NULL) {
            freeSize += powerOf(2, i);
            curr = curr->nextPart;
            freePartitions++;
        }
    }

    printf("\nHeap Usage Statistics:\n");
    printf("======================\n");


    //Remember to preserve the message format!

    printf("Total Space: %d bytes\n", hmi.totalSize);

    printf("Total Free Partitions: %d\n", freePartitions);
    printf("Total Free Size: %d bytes\n", freeSize);

    printf("Total Internal Fragmentation: %d bytes\n", internalFragmentationSize);
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
    if (hmi.A[lvl] == NULL) {
        hmi.A[lvl] = buildPartitionInfo(offset);
    } else {
        partInfo* curr = hmi.A[lvl];
        partInfo* prev = NULL;
        while (curr->nextPart != NULL && curr->nextPart->offset < offset && curr->offset != buddy) {
            prev = curr;
            curr = curr->nextPart;
        }
        if (curr->offset == buddy) {
            if (prev == NULL) hmi.A[lvl] = curr->nextPart;
            else prev->nextPart = curr->nextPart;
            if (buddy > offset) {
                addPartitionAtLevel(lvl + 1, offset);
            } else {
                addPartitionAtLevel(lvl + 1, buddy);
            }
        }
        else if (curr->nextPart == NULL) {
            if (curr->offset > offset) {
                partInfo* new = buildPartitionInfo(offset);
                new->nextPart = curr;
                if (prev == NULL) {
                    hmi.A[lvl] = new;
                } else {
                    prev->nextPart = new;
                }
            } else {
                partInfo* new = buildPartitionInfo(offset);
                curr->nextPart = new;
                if (prev == NULL) {
                    hmi.A[lvl] = curr;
                } else {
                    prev->nextPart = curr;
                }
            }
        } 
        else if (curr->nextPart->offset == buddy) {
            curr->nextPart = curr->nextPart->nextPart;
            if (buddy > offset) {
                addPartitionAtLevel(lvl + 1, offset);
            } else {
                addPartitionAtLevel(lvl + 1, buddy);
            }
        } 
        else if (curr->offset < buddy){
            partInfo* next = curr->nextPart;
            curr->nextPart = buildPartitionInfo(offset);
            curr->nextPart->nextPart = next;
        } else {
            partInfo* new = buildPartitionInfo(offset);
            new->nextPart = curr;
            if (prev != NULL) prev->nextPart = new;
            else hmi.A[lvl] = new;
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
        return NULL;
    } else if (hmi.A[lvl] == NULL) {
        partInfo* prev = removePartitionAtLevel(lvl + 1);
        if (prev != NULL) hmi.A[lvl] = buildPartitionInfo(prev->offset + powerOf(2, lvl));
        return prev;
    } else {
        partInfo* curr = hmi.A[lvl];
        hmi.A[lvl] = hmi.A[lvl]->nextPart;
        return curr;
    }
}

int setupHeap(int initialSize, int minSize, int maxSize)
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

    allocMinSize = minSize;
    allocMaxSize = maxSize;

    hmi.base = base;
    int inputSize = initialSize;

    int adjustedSize = 0;
    int lastAdded = 0;
    while (initialSize > 0) {
        int S = log2Floor(initialSize);
        if (powerOf(2, S) >= maxSize) {
            S = log2Floor(maxSize);
        } else if (powerOf(2, S) <= minSize) {
            S = log2Floor(minSize);
        }
        lastAdded = powerOf(2, S);
        adjustedSize += lastAdded;
        initialSize -= lastAdded;
    }
    if (adjustedSize > inputSize) adjustedSize -= lastAdded;

    hmi.totalSize = adjustedSize;
    hmi.internalFragTotal = 0;

    //TODO: Task 1. Setup the rest of the bookkeeping info:
    //       hmi.A <= an array of partition linked list
    //       hmi.maxIdx <= the largest index for hmi.A[]
    //       
    hmi.maxIdx = adjustedSize > maxSize ? log2Floor(maxSize) : log2Floor(adjustedSize);
    hmi.A = (partInfo**) malloc((hmi.maxIdx + 1) * sizeof(partInfo*));
    for (int i = 0; i < hmi.maxIdx; i++) {
        hmi.A[i] = NULL;
    }
    int lastOffset = 0;

    while (adjustedSize > 0) {
        int S = log2Floor(adjustedSize);
        if (powerOf(2, S) >= maxSize) {
            S = log2Floor(maxSize);
        } else if (powerOf(2, S) <= minSize) {
            S = log2Floor(minSize);
        } 
        if (hmi.A[S] == NULL) {
            hmi.A[S] = buildPartitionInfo(lastOffset);
        } else {
            partInfo* curr = hmi.A[S];
            while (curr->nextPart != NULL) {
                curr = curr->nextPart;
            }
            curr->nextPart = buildPartitionInfo(lastOffset);
        }
       lastOffset += powerOf(2, S);
       adjustedSize -= powerOf(2, S);
    }

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
    if (size < allocMinSize) size = allocMinSize;
    if (size > allocMaxSize) return (void*) 0;
    if (allocatedSize + size > hmi.totalSize) return (void*) 0;
    allocatedSize += size;
    int level = log2Ceiling(size);
    partInfo* chosen = removePartitionAtLevel(level);
    if (chosen != NULL) {
        internalFragmentationSize += powerOf(2, level) - size;
        return (void*) hmi.base + chosen->offset;
    } else return NULL;
}

void myfree(void* address, int size)
    /**********************************************************
     * Mimic the normal "free()":
     *    Attempt to free a previously allocated memory space
     *********************************************************/
{
    //TODO: Task 3. Implement the de allocation using buddy allocator
    if (size < allocMinSize) size = allocMinSize;
    if (size > allocMaxSize) size = allocMaxSize;
    allocatedSize -= size;
    int level = log2Ceiling(size);
    internalFragmentationSize -= powerOf(2, level) - size;
    int addr = (char*) address - (char*)hmi.base;
    addPartitionAtLevel(level, addr);
}
