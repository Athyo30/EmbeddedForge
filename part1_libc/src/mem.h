#ifndef FORGE_MEM_H
#define FORGE_MEM_H
#include <stddef.h>

/* Reimplement the standard mem* family. Prototypes match <string.h>. */
void *forge_memset(void *dst, int c, size_t n);
void *forge_memcpy(void *dst, const void *src, size_t n);
void *forge_memmove(void *dst, const void *src, size_t n);
int   forge_memcmp(const void *a, const void *b, size_t n);

#endif
