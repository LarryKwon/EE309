/*--------------------------------------------------------------------*/
/* heapmgrbase.c                                                      */
/* Author: Bob Dondero                                                */
/* Modified for double-linked free list with LIFO policy              */
/*--------------------------------------------------------------------*/

#include "heapmgr.h"
#include "chunk.h"
#include <stddef.h>
#include <stdio.h> /* For debugging */
#include <assert.h>

// #define __USE_MISC
#include <unistd.h>

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

static Chunk_T oFreeList = NULL;
/* The free list is a double-linked list of all free Chunks. */

/*--------------------------------------------------------------------*/

#ifndef NDEBUG

static int HeapMgr_isValid(void)

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
        if (oFreeList == NULL)
            return 1;
        else
        {
            fprintf(stderr, "Inconsistent empty heap\n");
            return 0;
        }
    }

    /* Traverse memory. */
    for (oChunk = oHeapStart;
         oChunk != NULL;
         oChunk = Chunk_getNextInMem(oChunk, oHeapStart, oHeapEnd))
        if (!Chunk_isValid(oChunk, oHeapStart, oHeapEnd))
            return 0;

    /* Traverse the free list. */
    oPrevChunk = NULL;
    for (oChunk = oFreeList;
         oChunk != NULL;
         oChunk = Chunk_getNextInList(oChunk))
    {
        if (!Chunk_isValid(oChunk, oHeapStart, oHeapEnd))
            return 0;
        if (oPrevChunk != NULL)
            if (Chunk_getNextInMem(oPrevChunk, oHeapStart, oHeapEnd) == oChunk)
            {
                fprintf(stderr, "Uncoalesced chunks\n");
                return 0;
            }
        oPrevChunk = oChunk;
    }

    return 1;
}
#endif

/*--------------------------------------------------------------------*/
static void Insert_free(Chunk_T oChunk)
{
    /*
    LIFO Insert
    */
    Chunk_setPrevInList(oChunk, NULL);
    Chunk_setNextInList(oChunk, oFreeList);
    if (oFreeList != NULL)
        Chunk_setPrevInList(oFreeList, oChunk);
    oFreeList = oChunk;
}

static void Remove_free(Chunk_T oChunk)
{
    Chunk_T oNext = Chunk_getNextInList(oChunk);
    Chunk_T oPrev = Chunk_getPrevInList(oChunk);

    if (oNext != NULL)
        Chunk_setPrevInList(oNext, oPrev);

    if (oPrev != NULL)
        Chunk_setNextInList(oPrev, oNext);
    else
        oFreeList = oNext;
}

/*--------------------------------------------------------------------*/

static Chunk_T HeapMgr_getMoreMemory(size_t uiUnits)

/* Request more memory from the operating system -- enough to store
   uiUnits units. Create a new chunk, insert it into the free list,
   and coalesce with the previous chunk if appropriate. */

{
    Chunk_T oChunk;

    if (uiUnits < MIN_UNITS_FROM_OS)
        uiUnits = MIN_UNITS_FROM_OS;

    /* Move the program break */
    oChunk = (Chunk_T)sbrk(uiUnits * Chunk_getUnitSize());
    if (oChunk == (Chunk_T)-1)
        return NULL;

    /* Update oHeapEnd */
    oHeapEnd = (Chunk_T)sbrk(0);

    /* Set the fields of the new chunk. */
    Chunk_setUnits(oChunk, uiUnits);
    Chunk_setStatus(oChunk, CHUNK_FREE);

    /* Add the new chunk to the front of the free list. */
    Insert_free(oChunk);

    /* Coalesce the new chunk and the previous one if appropriate. */
    Chunk_T oPrevChunk = Chunk_getPrevInMem(oChunk, oHeapStart, oHeapEnd);
    if (oPrevChunk != NULL && Chunk_getStatus(oPrevChunk) == CHUNK_FREE)
    {
        Remove_free(oPrevChunk);
        Remove_free(oChunk);

        /* Coalesce them */
        size_t uiTotalUnits = Chunk_getUnits(oPrevChunk) + Chunk_getUnits(oChunk);
        Chunk_setUnits(oPrevChunk, uiTotalUnits);
        Chunk_setStatus(oPrevChunk, CHUNK_FREE);

        /* Insert the coalesced chunk into the free list */
        oChunk = oPrevChunk;
        Insert_free(oChunk);
    }

    return oChunk;
}

/*--------------------------------------------------------------------*/

static Chunk_T HeapMgr_useChunk(Chunk_T oChunk, size_t uiUnits)

/* Use the given chunk oChunk for allocation. If the chunk is close
   in size to uiUnits, allocate it directly. If it is larger, split
   it and return the front part. */

{
    Chunk_T oNewChunk;
    size_t uiChunkUnits = Chunk_getUnits(oChunk);

    /* Remove the chunk from the free list */
    // Remove_free(oChunk);

    if (uiChunkUnits < uiUnits + MIN_UNITS_PER_CHUNK)
    {
        /* Chunk is close in size, use it directly */
        Remove_free(oChunk);
        Chunk_setStatus(oChunk, CHUNK_INUSE);
        return oChunk;
    }

    /* Split the chunk */
    Remove_free(oChunk);
    Chunk_setUnits(oChunk, uiUnits);
    Chunk_setStatus(oChunk, CHUNK_INUSE);
    // Chunk_setStatus(oChunk, CHUNK_INUSE);

    /* Create the tail chunk */
    oNewChunk = Chunk_getNextInMem(oChunk, oHeapStart, oHeapEnd);
    Chunk_setUnits(oNewChunk, uiChunkUnits - uiUnits);
    Chunk_setStatus(oNewChunk, CHUNK_FREE);
    Insert_free(oNewChunk);

    /* Insert the tail chunk into the free list */
    // Insert_free(oTailChunk);

    return oChunk;
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

    /* Search the free list for a chunk that is big enough */
    for (oChunk = oFreeList; oChunk != NULL; oChunk = Chunk_getNextInList(oChunk))
    {
        if (Chunk_getUnits(oChunk) >= uiUnits)
        {
            oChunk = HeapMgr_useChunk(oChunk, uiUnits);
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

    /* Now allocate from the new chunk */
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

    /* Set status to FREE */
    Chunk_setStatus(oChunk, CHUNK_FREE);

    /* Insert oChunk into free list */
    Insert_free(oChunk);

    /* Coalesce the given chunk and the previous one if appropriate. */
    Chunk_T oPrevChunk = Chunk_getPrevInMem(oChunk, oHeapStart, oHeapEnd);
    if (oPrevChunk != NULL && Chunk_getStatus(oPrevChunk) == CHUNK_FREE)
    {
        Remove_free(oPrevChunk);
        Remove_free(oChunk);

        size_t uiTotalUnits = Chunk_getUnits(oPrevChunk) + Chunk_getUnits(oChunk);
        Chunk_setUnits(oPrevChunk, uiTotalUnits);
        Chunk_setStatus(oPrevChunk, CHUNK_FREE);

        oChunk = oPrevChunk;
        Insert_free(oChunk);
    }

    /* Coalesce the given chunk and the next one if appropriate. */
    Chunk_T oNextChunk = Chunk_getNextInMem(oChunk, oHeapStart, oHeapEnd);
    if (oNextChunk != NULL && Chunk_getStatus(oNextChunk) == CHUNK_FREE)
    {
        Remove_free(oNextChunk);
        Remove_free(oChunk);

        size_t uiTotalUnits = Chunk_getUnits(oChunk) + Chunk_getUnits(oNextChunk);
        Chunk_setUnits(oChunk, uiTotalUnits);
        Chunk_setStatus(oChunk, CHUNK_FREE);

        Insert_free(oChunk);
    }

    assert(HeapMgr_isValid());
}

/*--------------------------------------------------------------------*/
