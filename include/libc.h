/* MSL libc functions statically linked in BOOT (names resolve through symbols/BOOT.txt). */
#ifndef LIBC_H
#define LIBC_H
typedef unsigned int size_t;
void *memset(void *, int, size_t);
void *memcpy(void *, const void *, size_t);
char *strcpy(char *, const char *);
char *strncpy(char *, const char *, size_t);
char *strcat(char *, const char *);
size_t strlen(const char *);
int stricmp(const char *, const char *);
int sprintf(char *, const char *, ...);
void *malloc(size_t);
void free(void *);
void qsort(void *, size_t, size_t, int (*)(const void *, const void *));
void *bsearch(const void *, const void *, size_t, size_t, int (*)(const void *, const void *));
/* allocator of the PC port (platform/): the port C of the newlib malloc family sits on it */
void *platform_malloc(size_t n);
void platform_free(void *p);
void *platform_realloc(void *p, size_t n);
void *platform_memalign(size_t align, size_t n);
#endif
