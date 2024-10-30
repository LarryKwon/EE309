#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include "heapmgr.h"
#include "chunk.h"

#define INUSE 1
#define FREE 0

typedef struct Header
{
    size_t size;
    int status;
    struct Header *next;
    struct Header *prev;
} Header;

static Header *free_list = NULL;
static Header *heap_end = NULL;

static void HeapMgr_init();
static Header *find_suitable_chunk(size_t uiBytes);
static void coalesce(Header *chunk);

void *HeapMgr_malloc(size_t uiBytes)
{
    if (heap_end == NULL)
    {
        HeapMgr_init();
    }

    size_t totalSize = sizeof(Header) + uiBytes;
    Header *chunk = find_suitable_chunk(totalSize);

    if (chunk != NULL)
    {
        if (chunk->size > totalSize + sizeof(Header))
        {
            Header *newChunk = (Header *)((char *)chunk + totalSize);
            newChunk->size = chunk->size - totalSize;
            newChunk->status = FREE;
            newChunk->next = chunk->next;
            newChunk->prev = chunk;

            chunk->size = totalSize;
            chunk->next = newChunk;
            if (newChunk->next != NULL)
            {
                newChunk->next->prev = newChunk;
            }
            chunk->status = INUSE;
        }
        else
        {
            chunk->status = INUSE;
            if (chunk->prev != NULL)
            {
                chunk->prev->next = chunk->next;
            }
            if (chunk->next != NULL)
            {
                chunk->next->prev = chunk->prev;
            }
            if (chunk == free_list)
            {
                free_list = chunk->next;
            }
        }
        return (void *)(chunk + 1);
    }

    Header *newChunk = (Header *)sbrk(totalSize);
    if (newChunk == (void *)-1)
    {
        return NULL;
    }
    newChunk->size = totalSize;
    newChunk->status = INUSE;
    newChunk->next = NULL;
    newChunk->prev = NULL;

    heap_end = (Header *)((char *)newChunk + totalSize);
    return (void *)(newChunk + 1);
}

void HeapMgr_free(void *pvBytes)
{
    if (pvBytes == NULL)
    {
        return;
    }

    Header *chunk = (Header *)pvBytes - 1;
    chunk->status = FREE;

    if (free_list == NULL)
    {
        free_list = chunk;
        chunk->next = NULL;
        chunk->prev = NULL;
    }
    else
    {
        Header *current = free_list;
        while (current != NULL && current < chunk)
        {
            current = current->next;
        }

        if (current == NULL)
        {
            chunk->next = NULL;
            chunk->prev = heap_end;
            heap_end->next = chunk;
        }
        else
        {
            chunk->next = current;
            chunk->prev = current->prev;
            if (current->prev != NULL)
            {
                current->prev->next = chunk;
            }
            current->prev = chunk;
        }
    }

    coalesce(chunk);
}

static void HeapMgr_init()
{
    free_list = NULL;
    heap_end = NULL;
}

static Header *find_suitable_chunk(size_t uiBytes)
{
    Header *current = free_list;
    while (current != NULL)
    {
        if (current->status == FREE && current->size >= uiBytes)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static void coalesce(Header *chunk)
{
    if (chunk->next != NULL && chunk->next->status == FREE)
    {
        chunk->size += chunk->next->size;
        chunk->next = chunk->next->next;
        if (chunk->next != NULL)
        {
            chunk->next->prev = chunk;
        }
    }
    if (chunk->prev != NULL && chunk->prev->status == FREE)
    {
        chunk->prev->size += chunk->size;
        chunk->prev->next = chunk->next;
        if (chunk->next != NULL)
        {
            chunk->next->prev = chunk->prev;
        }
    }
}
