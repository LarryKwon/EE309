/*--------------------------------------------------------------------*/
/* heapmgrbase.c                                                      */
/* Author: Bob Dondero                                                */
/* Baseline version, similar to K&R version                           */
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
/* The free list is a list of all free Chunks.  It is kept in
   ascending order by memory address. */

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

static Chunk_T HeapMgr_getMoreMemory(Chunk_T oPrevChunk,
                                     size_t uiUnits)

/* Request more memory from the operating system -- enough to store
   uiUnits units.  Create a new chunk, and either append it to the
   free list after oPrevChunk or increase the size of oPrevChunk. */

{
    Chunk_T oChunk;

    if (uiUnits < MIN_UNITS_FROM_OS)
        uiUnits = MIN_UNITS_FROM_OS;

    /* Move the program break. */
    oChunk = (Chunk_T)sbrk(uiUnits * Chunk_getUnitSize());
    if (oChunk == (Chunk_T)-1)
        return NULL;

    /* Determine where the program break is now, and note the
       end of the heap. */
    oHeapEnd = (Chunk_T)sbrk(0);

    /* Set the fields of the new chunk. */
    Chunk_setUnits(oChunk, uiUnits);
    Chunk_setNextInList(oChunk, NULL);

    /* Add the new chunk to the end of the free list. */
    if (oPrevChunk == NULL)
        oFreeList = oChunk;
    else
    {
        Chunk_setNextInList(oPrevChunk, oChunk);
        Chunk_setPrevInList(oChunk, oPrevChunk);
    }

    /* Coalesce the new chunk and the previous one if appropriate. */
    if (oPrevChunk != NULL)
        if (Chunk_getNextInMem(oPrevChunk, oHeapStart, oHeapEnd) == oChunk)
        {
            Chunk_setUnits(oPrevChunk,
                           Chunk_getUnits(oPrevChunk) + uiUnits);
            Chunk_setNextInList(oPrevChunk, NULL);
            oChunk = oPrevChunk;
        }

    return oChunk;
}

/*--------------------------------------------------------------------*/

static Chunk_T HeapMgr_useChunk(Chunk_T oChunk,
                                Chunk_T oPrevChunk, size_t uiUnits)

/* If oChunk is close to the right size (as specified by uiUnits),
   then splice oChunk out of the free list (using oPrevChunk to do
   so), and return oChunk.  If oChunk is too big, split it and return
   the address of the tail end.  */

{
    Chunk_T oNewChunk;
    size_t uiChunkUnits = Chunk_getUnits(oChunk);

    /* If oChunk is close to the right size, then use it. */
    if (uiChunkUnits < uiUnits + (size_t)MIN_UNITS_PER_CHUNK)
    {
        if (oPrevChunk == NULL)
            oFreeList = Chunk_getNextInList(oChunk);
        else
        {
            Chunk_setNextInList(oPrevChunk, Chunk_getNextInList(oChunk));
            Chunk_setPrevInList(Chunk_getNextInList(oChunk), oPrevChunk);
        }
        Chunk_setStatus(oChunk, CHUNK_INUSE);
        return oChunk;
    }

    /* oChunk is too big, so use the tail end of it. */
    Chunk_setUnits(oChunk, uiChunkUnits - uiUnits);
    Chunk_setStatus(oChunk, CHUNK_INUSE);
    oNewChunk = Chunk_getNextInMem(oChunk, oHeapStart, oHeapEnd);
    Chunk_setStatus(oNewChunk, CHUNK_FREE);
    Chunk_setUnits(oNewChunk, uiUnits);
    return oNewChunk;
}

/*--------------------------------------------------------------------*/

void *HeapMgr_malloc(size_t uiBytes)

/* Return a pointer to space for an object of size uiBytes.  Return
   NULL if uiBytes is 0 or the request cannot be satisfied.  The
   space is uninitialized. */

{
    Chunk_T oChunk;
    Chunk_T oPrevChunk;
    Chunk_T oPrevPrevChunk;
    size_t uiUnits;
    size_t uiUnitSize;

    if (uiBytes == 0)
        return NULL;

    /* Initialize the heap manager if this is the first call. */
    if (oHeapStart == NULL)
    {
        oHeapStart = (Chunk_T)sbrk(0);
        oHeapEnd = oHeapStart;
    }

    assert(HeapMgr_isValid());

    /* Determine the number of units the new chunk should contain. */
    uiUnitSize = Chunk_getUnitSize();
    while (uiBytes % uiUnitSize != 0)
        uiBytes++;
    uiUnits = uiBytes / uiUnitSize;
    uiUnits++; /* Allow room for a header. */
    uiUnits++; /* Allow room for a footer */

    oPrevPrevChunk = NULL;
    oPrevChunk = NULL;

    for (oChunk = oFreeList;
         oChunk != NULL;
         oChunk = Chunk_getNextInList(oChunk))
    {
        /* If oChunk is big enough, then use it. */
        if (Chunk_getUnits(oChunk) >= uiUnits)
        {
            oChunk = HeapMgr_useChunk(oChunk, oPrevChunk, uiUnits);
            assert(HeapMgr_isValid());
            return (void *)((char *)oChunk + uiUnitSize);
        }

        oPrevPrevChunk = oPrevChunk;
        oPrevChunk = oChunk;
    }

    /* Ask the OS for more memory, and create a new chunk (or expand
       the existing chunk) at the end of the free list. */
    oChunk = HeapMgr_getMoreMemory(oPrevChunk, uiUnits);
    if (oChunk == NULL)
    {
        assert(HeapMgr_isValid());
        return NULL;
    }

    /* If the new large chunk was coalesced with the previous chunk,
       then reset the previous chunk. */
    if (oChunk == oPrevChunk)
        oPrevChunk = oPrevPrevChunk;

    /* oChunk is big enough, so use it. */
    oChunk = HeapMgr_useChunk(oChunk, oPrevChunk, uiUnits);
    assert(HeapMgr_isValid());
    return (void *)((char *)oChunk + uiUnitSize);
}

/*--------------------------------------------------------------------*/

void HeapMgr_free(void *pvBytes)

/* Deallocate the space pointed to by pvBytes.  Do nothing if pvBytes
   is NULL.  It is an unchecked runtime error for pvBytes to be a
   pointer to space that was not previously allocated by
   HeapMgr_malloc(). */

{
    Chunk_T oChunk;
    Chunk_T oNextChunk;
    Chunk_T oPrevChunk;

    assert(HeapMgr_isValid());

    if (pvBytes == NULL)
        return;

    oChunk = (Chunk_T)((char *)pvBytes - Chunk_getUnitSize());
    assert(Chunk_isValid(oChunk, oHeapStart, oHeapEnd));

    /* Splice the given chunk into the free list at the correct spot.
       The free list is kept in increasing order by memory address. */
    oPrevChunk = NULL;
    oNextChunk = oFreeList;
    for (;;)
    {
        if ((oNextChunk == NULL) || (oNextChunk > oChunk))
        {
            if (oPrevChunk == NULL)
                oFreeList = oChunk;
            else
                Chunk_setNextInList(oPrevChunk, oChunk);
            Chunk_setNextInList(oChunk, oNextChunk);
            break;
        }
        oPrevChunk = oNextChunk;
        oNextChunk = Chunk_getNextInList(oNextChunk);
    }

    /* Coalesce the given chunk and the previous one if appropriate. */
    if (oPrevChunk != NULL)
        if (Chunk_getNextInMem(oPrevChunk, oHeapStart, oHeapEnd) == oChunk)
        {
            Chunk_setUnits(oPrevChunk,
                           Chunk_getUnits(oPrevChunk) + Chunk_getUnits(oChunk));
            Chunk_setNextInList(oPrevChunk,
                                Chunk_getNextInList(oChunk));
            oChunk = oPrevChunk;
        }

    /* Coalesce the given chunk and the next one if appropriate. */
    if (oNextChunk != NULL)
        if (Chunk_getNextInMem(oChunk, oHeapStart, oHeapEnd) == oNextChunk)
        {
            Chunk_setUnits(oChunk,
                           Chunk_getUnits(oChunk) + Chunk_getUnits(oNextChunk));
            Chunk_setNextInList(oChunk,
                                Chunk_getNextInList(oNextChunk));
        }

    assert(HeapMgr_isValid());
}
