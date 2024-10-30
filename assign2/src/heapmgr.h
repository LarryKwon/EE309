/*--------------------------------------------------------------------*/
/* heapmgr.h                                                          */
/* Author: Bob Dondero                                                */
/*--------------------------------------------------------------------*/

#ifndef HEAPMGR_INCLUDED
#define HEAPMGR_INCLUDED

#include <stddef.h>

void *HeapMgr_malloc(size_t uiBytes);
/* Return a pointer to space for an object of size uiBytes.  Return
   NULL if uiBytes is 0 or the request cannot be satisfied.  The
   space is uninitialized. */

void HeapMgr_free(void *pvBytes);
/* Deallocate the space pointed to by pvBytes.  Do nothing if pvBytes
   is NULL.  It is an unchecked runtime error for pvBytes to be a
   a pointer to space that was not previously allocated by
   HeapMgr_malloc(). */

#endif
