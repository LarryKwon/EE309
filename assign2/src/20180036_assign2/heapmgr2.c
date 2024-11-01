/*--------------------------------------------------------------------*/
/* heapmgr2.c                                                         */
/* Author: Your Name                                                  */
/* Modified for bins with double-linked free lists                    */
/*--------------------------------------------------------------------*/

#include "heapmgr.h"
#include "chunk.h"
#include <stddef.h>
#include <stdio.h> /* For debugging */
#include <assert.h>

// #define __USE_MISC
#include <unistd.h>

#define BINS_SIZE 1024
enum
{
    MIN_UNITS_FROM_OS = 1024
};
/* The minimum number of units to request of the OS. */

/*--------------------------------------------------------------------*/

/* The state of the HeapMgr. */

static Chunk_T oHeapStart = NULL;
/* The address of the start of the heap. */

static Chunk_T oHeapEnd = NULL;
/* The address immediately beyond the end of the heap. */

static Chunk_T oBinsList[BINS_SIZE] = {NULL};
/* The array of bins, each bin is a free list */

static size_t oBinSizeList[BINS_SIZE] = {0};
/* The array of bin sizes */

/*--------------------------------------------------------------------*/

#ifndef NDEBUG

static int
HeapMgr_isValid(void)
/* Return 1 (TRUE) iff the heap manager is in a valid state. */
{
    Chunk_T oChunk;
    Chunk_T oPrevChunk;

    if (oHeapStart == NULL)
    {
        fprintf(stderr, "Uninitialized heap start\n");
        return 0;
    }
    if (oHeapEnd == NULL)
    {
        fprintf(stderr, "Uninitialized heap end\n");
        return 0;
    }

    if (oHeapStart == oHeapEnd)
    {
        for (size_t i = 0; i < BINS_SIZE; i++)
        {
            if (oBinSizeList[i] != 0)
            {
                fprintf(stderr, "Inconsistent empty heap\n");
                return 0;
            }
        }
        return 1;
    }

    /* Traverse memory. */
    for (oChunk = oHeapStart;
         oChunk != NULL;
         oChunk = Chunk_getNextInMem(oChunk, oHeapStart, oHeapEnd))
        if (!Chunk_isValid(oChunk, oHeapStart, oHeapEnd))
            return 0;

    /* Traverse the free lists in all bins. */
    for (size_t i = 0; i < BINS_SIZE; i++)
    {
        if (oBinSizeList[i] != 0)
        {
            Chunk_T bin = oBinsList[i];
            oPrevChunk = NULL;
            for (oChunk = bin;
                 oChunk != NULL;
                 oChunk = Chunk_getNextInList(oChunk))
            {
                if (!Chunk_isValid(oChunk, oHeapStart, oHeapEnd))
                    return 0;
                oPrevChunk = oChunk;
            }
        }
    }

    return 1;
}
#endif
/*--------------------------------------------------------------------*/

static size_t Determine_bin(size_t uiUnits)
{
    if (uiUnits >= BINS_SIZE)
    {
        return BINS_SIZE - 1;
    }
    else
    {
        return uiUnits;
    }
}
/*--------------------------------------------------------------------*/

static void InsertToFreeList(Chunk_T oChunk, size_t bin)
{
    /*
    LIFO Insert
    */
    Chunk_T binList = oBinsList[bin];

    Chunk_setPrevInList(oChunk, NULL);
    Chunk_setNextInList(oChunk, binList);
    if (binList != NULL)
        Chunk_setPrevInList(binList, oChunk);
    oBinsList[bin] = oChunk;
    oBinSizeList[bin] += 1;
}

static void RemoveFromFreeList(Chunk_T oChunk, size_t bin)
{
    Chunk_T oNext = Chunk_getNextInList(oChunk);
    Chunk_T oPrev = Chunk_getPrevInList(oChunk);

    if (oNext != NULL)
        Chunk_setPrevInList(oNext, oPrev);

    if (oPrev != NULL)
        Chunk_setNextInList(oPrev, oNext);
    else
        oBinsList[bin] = oNext;

    oBinSizeList[bin] -= 1;
}
/*--------------------------------------------------------------------*/

static Chunk_T HeapMgr_getMoreMemory(size_t uiUnits)
/* Request more memory from the operating system -- enough to store
   uiUnits units. Create a new chunk, insert it into the free list,
   and coalesce with the previous chunk if appropriate. */
{
    Chunk_T oChunk;

    size_t uiRequestUnits = uiUnits;
    if (uiRequestUnits < MIN_UNITS_FROM_OS)
        uiRequestUnits = MIN_UNITS_FROM_OS;

    /* Move the program break */
    oChunk = (Chunk_T)sbrk(uiRequestUnits * Chunk_getUnitSize());
    if (oChunk == (Chunk_T)-1)
        return NULL;

    /* Determine where the program break is now, and note the
       end of the heap. */
    oHeapEnd = (Chunk_T)sbrk(0);

    /* Set the fields of the new chunk. */
    Chunk_setUnits(oChunk, uiRequestUnits);
    Chunk_setStatus(oChunk, CHUNK_FREE);

    /* Coalesce the new chunk and the previous one if appropriate. */
    Chunk_T oPrevChunk = Chunk_getPrevInMem(oChunk, oHeapStart, oHeapEnd);
    if (oPrevChunk != NULL && Chunk_getStatus(oPrevChunk) == CHUNK_FREE)
    {
        size_t prevBin = Determine_bin(Chunk_getUnits(oPrevChunk));
        RemoveFromFreeList(oPrevChunk, prevBin);

        /* Coalesce */
        size_t uiTotalUnits = Chunk_getUnits(oPrevChunk) + Chunk_getUnits(oChunk);
        Chunk_setUnits(oPrevChunk, uiTotalUnits);
        Chunk_setStatus(oPrevChunk, CHUNK_FREE);

        oChunk = oPrevChunk;
    }

    /* Insert the new/coalesced chunk into the proper bin */
    size_t bin = Determine_bin(Chunk_getUnits(oChunk));
    InsertToFreeList(oChunk, bin);

    return oChunk;
}

/*--------------------------------------------------------------------*/

static Chunk_T HeapMgr_useChunk(Chunk_T oChunk, size_t uiUnits)
{
    size_t uiChunkUnits = Chunk_getUnits(oChunk);
    size_t oldBin = Determine_bin(uiChunkUnits);

    /* uiUnits : the size we want to allocate */
    if (uiChunkUnits < uiUnits + MIN_UNITS_PER_CHUNK)
    {
        /* If oChunk is close to the right size, then use it. */
        RemoveFromFreeList(oChunk, oldBin);
        Chunk_setStatus(oChunk, CHUNK_INUSE);
        return oChunk;
    }
    else
    {
        /* oChunk is too big, so use the tail end of it. */
        /* Split */
        RemoveFromFreeList(oChunk, oldBin);
        Chunk_setUnits(oChunk, uiUnits);
        Chunk_setStatus(oChunk, CHUNK_INUSE);

        /* Create the tail */
        size_t newChunkSize = uiChunkUnits - uiUnits;
        Chunk_T oTailChunk = Chunk_getNextInMem(oChunk, oHeapStart, oHeapEnd);
        Chunk_setUnits(oTailChunk, newChunkSize);
        Chunk_setStatus(oTailChunk, CHUNK_FREE);

        /* Insert the tail chunk into the proper bin */
        size_t newBin = Determine_bin(newChunkSize);
        InsertToFreeList(oTailChunk, newBin);

        return oChunk;
    }
}
/*--------------------------------------------------------------------*/

void *HeapMgr_malloc(size_t uiBytes)

/* Return a pointer to space for an object of size uiBytes. Return
   NULL if uiBytes is 0 or the request cannot be satisfied. The
   space is uninitialized. */

{
    Chunk_T oChunk;
    size_t uiUnits;
    size_t uiUnitSize;

    if (uiBytes == 0)
        return NULL;

    /* Initialize the heap manager if this is the first call */
    if (oHeapStart == NULL)
    {
        oHeapStart = (Chunk_T)sbrk(0);
        oHeapEnd = oHeapStart;
    }

    assert(HeapMgr_isValid());

    /* Determine the number of units the new chunk should contain */
    uiUnitSize = Chunk_getUnitSize();
    while (uiBytes % uiUnitSize != 0)
        uiBytes++;
    uiUnits = uiBytes / uiUnitSize;
    uiUnits++; /* Allow room for a header. */
    uiUnits++; /* Allow room for a footer */

    for (size_t i = uiUnits; i < BINS_SIZE; i++)
    {
        if (oBinSizeList[i] > 0 && oBinsList[i] != NULL)
        {
            Chunk_T oChunkInBin = oBinsList[i];
            oChunk = HeapMgr_useChunk(oChunkInBin, uiUnits);
            assert(HeapMgr_isValid());
            return (void *)((char *)oChunk + uiUnitSize);
        }
    }

    /* Ask the OS for more memory, and create a new chunk (or expand
    the existing chunk) at the end of the free list. */
    oChunk = HeapMgr_getMoreMemory(uiUnits);
    if (oChunk == NULL)
    {
        assert(HeapMgr_isValid());
        return NULL;
    }

    oChunk = HeapMgr_useChunk(oChunk, uiUnits);
    assert(HeapMgr_isValid());
    return (void *)((char *)oChunk + uiUnitSize);
}

/*--------------------------------------------------------------------*/
void HeapMgr_free(void *pvBytes)

/* Deallocate the space pointed to by pvBytes. Do nothing if pvBytes
   is NULL. It is an unchecked runtime error for pvBytes to be a
   pointer to space that was not previously allocated by
   HeapMgr_malloc(). */

{
    Chunk_T oChunk;

    assert(HeapMgr_isValid());

    if (pvBytes == NULL)
        return;

    oChunk = (Chunk_T)((char *)pvBytes - Chunk_getUnitSize());
    assert(Chunk_isValid(oChunk, oHeapStart, oHeapEnd));
    Chunk_setStatus(oChunk, CHUNK_FREE);

    /* Insert oChunk into proper bin */
    size_t bin = Determine_bin(Chunk_getUnits(oChunk));
    InsertToFreeList(oChunk, bin);

    /* Coalesce the given chunk and the previous one if appropriate. */
    Chunk_T oPrevChunk = Chunk_getPrevInMem(oChunk, oHeapStart, oHeapEnd);
    if (oPrevChunk != NULL && Chunk_getStatus(oPrevChunk) == CHUNK_FREE)
    {
        size_t prevBin = Determine_bin(Chunk_getUnits(oPrevChunk));
        RemoveFromFreeList(oPrevChunk, prevBin);
        RemoveFromFreeList(oChunk, bin);

        size_t uiTotalUnits = Chunk_getUnits(oPrevChunk) + Chunk_getUnits(oChunk);
        Chunk_setUnits(oPrevChunk, uiTotalUnits);
        Chunk_setStatus(oPrevChunk, CHUNK_FREE);

        oChunk = oPrevChunk;
        bin = Determine_bin(uiTotalUnits);
        InsertToFreeList(oChunk, bin);
    }

    /* Coalesce the given chunk and the next one if appropriate. */
    Chunk_T oNextChunk = Chunk_getNextInMem(oChunk, oHeapStart, oHeapEnd);
    if (oNextChunk != NULL && Chunk_getStatus(oNextChunk) == CHUNK_FREE)
    {
        size_t nextBin = Determine_bin(Chunk_getUnits(oNextChunk));
        RemoveFromFreeList(oNextChunk, nextBin);
        RemoveFromFreeList(oChunk, bin);

        size_t uiTotalUnits = Chunk_getUnits(oChunk) + Chunk_getUnits(oNextChunk);
        Chunk_setUnits(oChunk, uiTotalUnits);
        Chunk_setStatus(oChunk, CHUNK_FREE);

        bin = Determine_bin(uiTotalUnits);
        InsertToFreeList(oChunk, bin);
    }

    assert(HeapMgr_isValid());
}

/*--------------------------------------------------------------------*/
