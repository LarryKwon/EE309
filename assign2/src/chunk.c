/*--------------------------------------------------------------------*/
/* chunk.c                                                            */
/* Author: Bob Dondero                                                */
/*--------------------------------------------------------------------*/

#include "chunk.h"
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

/*--------------------------------------------------------------------*/

struct Chunk
{
   /* The number of Units in the Chunk.  The high-order bit
      stores the Chunk's status. */
   size_t uiUnits;

   /* The address of an adjacent Chunk. */
   Chunk_T oAdjacentChunk;
};

/*--------------------------------------------------------------------*/

size_t Chunk_getUnitSize(void)

/* Return the number of bytes in a Unit */

{
   return sizeof(struct Chunk);
}

/*--------------------------------------------------------------------*/

enum ChunkStatus Chunk_getStatus(Chunk_T oChunk)

/* Return the status of oChunk.
   It is a checked runtime error for oChunk to be NULL. */

{
   assert(oChunk != NULL);
   return oChunk->uiUnits >> 63;
}

/*--------------------------------------------------------------------*/

void Chunk_setStatus(Chunk_T oChunk, enum ChunkStatus eStatus)

/* Set the status of oChunk to eStatus.
   It is a checked runtime error for oChunk to be NULL or for
   eStatus to be other than CHUNK_FREE or CHUNK_INUSE. */

{
   assert(oChunk != NULL);
   assert((eStatus == CHUNK_FREE) || (eStatus == CHUNK_INUSE));
   oChunk->uiUnits =
      (oChunk->uiUnits & (size_t)0x7fffffffffffffffL) | ((size_t)eStatus << 63);
}

/*--------------------------------------------------------------------*/

size_t Chunk_getUnits(Chunk_T oChunk)

/* Return oChunk's number of units.
   It is a checked runtime error for oChunk to be NULL. */

{
   assert(oChunk != NULL);
   return (oChunk->uiUnits & (size_t)0x7fffffffffffffffL);
}

/*--------------------------------------------------------------------*/

void Chunk_setUnits(Chunk_T oChunk, size_t uiUnits)

/* Set oChunk's number of units to uiUnits.
   It is a checked runtime error for oChunk to be NULL or
   for uiUnits to be less than MIN_UNITS_PER_CHUNK. */

{
   assert(oChunk != NULL);
   assert(uiUnits >= MIN_UNITS_PER_CHUNK);
   assert(uiUnits <= (size_t)0x7fffffffffffffff);

   /* Set the Units in oChunk's header. */
   oChunk->uiUnits =
      (oChunk->uiUnits & (size_t)0x8000000000000000UL) | uiUnits;

   /* Set the Units in oChunk's footer. */
   (oChunk + uiUnits - 1)->uiUnits = uiUnits;
}

/*--------------------------------------------------------------------*/

Chunk_T Chunk_getNextInList(Chunk_T oChunk)

/* Return oChunk's next Chunk in the free list, or NULL if there
   is no next Chunk.
   It is a checked runtime error for oChunk to be NULL. */

{
   assert(oChunk != NULL);
   return oChunk->oAdjacentChunk;
}

/*--------------------------------------------------------------------*/

void Chunk_setNextInList(Chunk_T oChunk, Chunk_T oNextChunk)

/* Set oChunk's next Chunk in the free list to oNextChunk.
   It is a checked runtime error for oChunk to be NULL. */

{
   assert(oChunk != NULL);
   oChunk->oAdjacentChunk = oNextChunk;
}

/*--------------------------------------------------------------------*/

Chunk_T Chunk_getPrevInList(Chunk_T oChunk)

/* Return oChunk's previous Chunk in the free list, or NULL if there
   is no previous Chunk.
   It is a checked runtime error for oChunk to be NULL. */

{
   assert(oChunk != NULL);
   return (oChunk + Chunk_getUnits(oChunk) - 1)->oAdjacentChunk;
}

/*--------------------------------------------------------------------*/

void Chunk_setPrevInList(Chunk_T oChunk, Chunk_T oPrevChunk)

/* Set oChunk's previous Chunk in the free list to oPrevChunk.
   It is a checked runtime error for oChunk to be NULL. */

{
   assert(oChunk != NULL);
   (oChunk + Chunk_getUnits(oChunk) - 1)->oAdjacentChunk = oPrevChunk;
}

/*--------------------------------------------------------------------*/

Chunk_T Chunk_getNextInMem(Chunk_T oChunk,
   Chunk_T oHeapStart, Chunk_T oHeapEnd)

/* Return oChunk's next Chunk in memory, or NULL if there is no
   next Chunk.  Use oHeapEnd to determine if there is no next
   Chunk.  oChunk's number of units must be set properly for this
   function to work.
   It is a checked runtime error for oChunk, oHeapStart, or oHeapEnd
   to be NULL, or for oChunk to out be of the range defined by
   oHeapStart and oHeapEnd. */

{
   Chunk_T oNextChunk;
   assert(oChunk != NULL);
   assert(oHeapStart != NULL);
   assert(oHeapEnd != NULL);
   assert(oChunk >= oHeapStart);
   assert(oChunk < oHeapEnd);
   oNextChunk = oChunk + Chunk_getUnits(oChunk);
   assert(oNextChunk <= oHeapEnd);
   if (oNextChunk == oHeapEnd)
      return NULL;
   return oNextChunk;
}

/*--------------------------------------------------------------------*/

Chunk_T Chunk_getPrevInMem(Chunk_T oChunk,
   Chunk_T oHeapStart, Chunk_T oHeapEnd)

/* Return oChunk's previous Chunk in memory, or NULL if there is no
   previous Chunk.  Use oHeapStart to determine if there is no
   previous Chunk.  The previous Chunk's number of units must be set
   properly for this function to work.
   It is a checked runtime error for oChunk, oHeapStart, or oHeapEnd
   to be NULL, or for oChunk to be out of the range defined by
   oHeapStart and oHeapEnd. */

{
   Chunk_T oPrevChunk;
   assert(oChunk != NULL);
   assert(oHeapStart != NULL);
   assert(oHeapEnd != NULL);
   assert(oChunk >= oHeapStart);
   assert(oChunk < oHeapEnd);
   if (oChunk == oHeapStart)
      return NULL;
   oPrevChunk = oChunk - ((oChunk - 1)->uiUnits);
   assert(oPrevChunk >= oHeapStart);
   return oPrevChunk;
}

/*--------------------------------------------------------------------*/

#ifndef NDEBUG

static size_t Chunk_getFooterUnits(Chunk_T oChunk)

/* Return the number of Units in oChunk's footer. */

{
   assert(oChunk != NULL);
   return (oChunk + Chunk_getUnits(oChunk) - 1)->uiUnits;
}

#endif

/*--------------------------------------------------------------------*/

#ifndef NDEBUG

int Chunk_isValid(Chunk_T oChunk,
                  Chunk_T oHeapStart, Chunk_T oHeapEnd)

/* Return TRUE iff oChunk is valid, notably with respect to
   oHeapStart and oHeapEnd.
   It is a checked runtime error for oChunk, oHeapStart, or oHeapEnd
   to be NULL. */

{
   // fprintf(stderr, "Chunk_isValid(%p)\n", (void*)oChunk);
   assert(oChunk != NULL);
   assert(oHeapStart != NULL);
   assert(oHeapEnd != NULL);

   if (oChunk < oHeapStart)
      {fprintf(stderr, "Bad heap start\n"); return 0; }
   if (oChunk >= oHeapEnd)
      {fprintf(stderr, "Bad heap end\n"); return 0; }
   if (Chunk_getUnits(oChunk) == 0)
      {fprintf(stderr, "Zero units\n"); return 0; }
   if (Chunk_getUnits(oChunk) < MIN_UNITS_PER_CHUNK)
      {fprintf(stderr, "Bad size\n"); return 0; }
   if (Chunk_getUnits(oChunk) != Chunk_getFooterUnits(oChunk))
      {fprintf(stderr, "Inconsistent sizes\n"); return 0; }
   return 1;
}

#endif