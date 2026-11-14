/* Port side of the game's newlib (src/BOOT/libc_*.c, crt0.c; declarations in include/libc.h).
   The game defines malloc/free/printf... itself, which interposes glibc's: its allocator ends here and uses glibc's
   own entry points, which keep working under the interposition. */
#include <stddef.h>

void *__libc_malloc(size_t n);
void __libc_free(void *p);
void *__libc_realloc(void *p, size_t n);
void *__libc_memalign(size_t align, size_t n);

void *platform_malloc(size_t n) { return __libc_malloc(n); }
void platform_free(void *p) { __libc_free(p); }
void *platform_realloc(void *p, size_t n) { return __libc_realloc(p, n); }
void *platform_memalign(size_t align, size_t n) { return __libc_memalign(align, n); }

/* crt0 (_init/_fini of the SDK): no constructors in the game's C */
void libc_init(void) {}
void libc_fini(void) {}

/* the calling thread's reent block (k0 + 4 on the PSP); 0 = the global one */
char *libc_thread_reent(void) { return 0; }
