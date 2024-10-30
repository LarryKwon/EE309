/*--------------------------------------------------------------------*/
/* chunkbase.h                                                        */
/* Author: Bob Dondero                                                */
/*--------------------------------------------------------------------*/

#ifndef CHUNKBASE_INCLUDED
#define CHUNKBASE_INCLUDED

#include <stddef.h>

typedef struct Chunk *Chunk_T;

/* A Chunk is a sequence of Units.

   The first Unit is a header that specifies the number of Units in
   the Chunk and, if the Chunk is free, a pointer to the next Chunk in
   the free list.  The subsequent Units store client data. */

enum {MIN_UNITS_PER_CHUNK = 2};
/* The minimum number of Units that a Chunk can contain. */

size_t Chunk_getUnitSize(void);
/* Return the number of bytes in a Unit. */

size_t Chunk_getUnits(Chunk_T oChunk);
/* Return oChunk's number of Units.
   It is a checked runtime error for oChunk to be NULL. */

void Chunk_setUnits(Chunk_T oChunk, size_t uiUnits);
/* Set oChunk's number of Units to uiUnits.
   It is a checked runtime error for oChunk to be NULL or
   for uiUnits to be less than MIN_UNITS_PER_CHUNK. */

Chunk_T Chunk_getNextInList(Chunk_T oChunk);
/* Return oChunk's next Chunk in the free list, or NULL if there
   is no next Chunk.
   It is a checked runtime error for oChunk to be NULL. */

void Chunk_setNextInList(Chunk_T oChunk, Chunk_T oNextChunk);
/* Set oChunk's next Chunk in the free list to oNextChunk.
   It is a checked runtime error for oChunk to be NULL. */

Chunk_T Chunk_getNextInMem(Chunk_T oChunk,
   Chunk_T oHeapStart, Chunk_T oHeapEnd);
/* Return oChunk's next Chunk in memory, or NULL if there is no
   next Chunk.  Use oHeapEnd to determine if there is no next
   Chunk.  oChunk's number of units must be set properly for this
   function to work.
   It is a checked runtime error for oChunk, oHeapStart, or oHeapEnd
   to be NULL, or for oChunk to be out of the range defined by
   oHeapStart and oHeapEnd. */

#ifndef NDEBUG
int Chunk_isValid(Chunk_T oChunk,
                  Chunk_T oHeapStart, Chunk_T oHeapEnd);
/* Return 1 (TRUE) iff oChunk is valid, notably with respect to
   oHeapStart and oHeapEnd.
   It is a checked runtime error for oChunk, oHeapStart, or oHeapEnd
   to be NULL. */
#endif

#endif
