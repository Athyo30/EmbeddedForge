#ifndef FORGE_ALLOC_H
#define FORGE_ALLOC_H
#include <stddef.h>

/* You are given a fixed byte arena to manage. No sbrk, no mmap. This
 * is exactly the constraint you hit on bare metal: a static blob of
 * RAM and nothing else. */

/* ---- Stage A: bump allocator ------------------------------------
 * Hands out memory linearly, never frees individual blocks. Trivial
 * but teaches alignment. Do this first. */
void  bump_init(void *arena, size_t size);
void *bump_alloc(size_t size);   /* returns 8-byte-aligned ptr or NULL */
void  bump_reset(void);          /* free everything at once */

/* ---- Stage B: free-list allocator -------------------------------
 * Real malloc/free over the same arena, with a singly-linked free
 * list, splitting on alloc and coalescing adjacent blocks on free.
 * This is the data-structure-design rep. Keep the node minimal. */
void  fl_init(void *arena, size_t size);
void *fl_malloc(size_t size);
void  fl_free(void *ptr);

#endif
