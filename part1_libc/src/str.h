#ifndef FORGE_STR_H
#define FORGE_STR_H
#include <stddef.h>

size_t forge_strlen(const char *s);
char  *forge_strcpy(char *dst, const char *src);
char  *forge_strncpy(char *dst, const char *src, size_t n);
int    forge_strcmp(const char *a, const char *b);

#endif
