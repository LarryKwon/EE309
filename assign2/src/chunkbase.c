/*--------------------------------------------------------------------*/
/* chunkbase.c                                                        */
/* Author: Bob Dondero                                                */
/*--------------------------------------------------------------------*/

#include "chunkbase.h"
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

/*--------------------------------------------------------------------*/

struct Chunk
{
   /* The number of Units in the Chunk. */
   size_t uiUnits;

   /* The address of the next Chunk. */
   Chunk_T oNextChunk;
};

/*--------------------------------------------------------------------*/

size_t Chunk_getUnitSize(void)

/* Return the number of bytes in a Unit */

{
   return sizeof(struct Chunk);
}

/*--------------------------------------------------------------------*/

size_t Chunk_getUnits(Chunk_T oChunk)

/* Return oChunk's number of Units.
   It is a checked runtime error for oChunk to be NULL. */

{
   assert(oChunk != NULL);
   return oChunk->uiUnits;
}

/*--------------------------------------------------------------------*/

void Chunk_setUnits(Chunk_T oChunk, size_t uiUnits)

/* Set oChunk's number of Units to uiUnits.
   It is a checked runtime error for oChunk to be NULL or
   for uiUnits to be less than MIN_UNITS_PER_CHUNK. */

{
   assert(oChunk != NULL);
   assert(uiUnits >= MIN_UNITS_PER_CHUNK);
   oChunk->uiUnits = uiUnits;
}

/*--------------------------------------------------------------------*/

Chunk_T Chunk_getNextInList(Chunk_T oChunk)

/* Return oChunk's next Chunk in the free list, or NULL if there
   is no next Chunk.
   It is a checked runtime error for oChunk to be NULL. */

{
   assert(oChunk != NULL);
   return oChunk->oNextChunk;
}

/*--------------------------------------------------------------------*/

void Chunk_setNextInList(Chunk_T oChunk, Chunk_T oNextChunk)

/* Set oChunk's next Chunk in the free list to oNextChunk.
   It is a checked runtime error for oChunk to be NULL. */

{
   assert(oChunk != NULL);
   oChunk->oNextChunk = oNextChunk;
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

#ifndef NDEBUG

int Chunk_isValid(Chunk_T oChunk,
                  Chunk_T oHeapStart, Chunk_T oHeapEnd)

/* Return 1 (TRUE) iff oChunk is valid, notably with respect to
   oHeapStart and oHeapEnd.
   It is a checked runtime error for oChunk, oHeapStart, or oHeapEnd
   to be NULL. */

{
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
   return 1;
}

#endif
