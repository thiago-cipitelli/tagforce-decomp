// @cc gcc
/* Functions of libc_malloc.c (newlib of the Sony SDK libc) that come out of C byte for byte with the SDK's GCC
   (tools/psp_gcc.sh; docs/build.md, "Trilha GCC"); their ranges are the `@gcc` lines of libc_malloc.c. */

extern int libc_charset;               /* 0x088EB5A4 */
extern char libc_lconv[];              /* 0x08873B7C */
int _lseek(int fd, int offset, int whence);
extern int libc_errno;                 /* 0x08E35A34 */
#ifdef __mips__ /* the PC port has its own (libc_malloc.c) */
extern void *libc_global_reent;        /* 0x088EB598 _global_impure_ptr */

/* newlib __getreent as the Sony SDK has it: the thread's reent struct is the word at k0 + 4 (the thread-local
   area of the PSP kernel; crt0's user_main stores it there), the global one when there is none */
void *heap_get_current(void)
{
    register void **k0 __asm__("$26");
    void *r;

    if (k0 != 0 && (r = k0[1]) != 0) {
        return r;
    }
    return libc_global_reent;
}

int strcmp(const char *a, const char *b);
extern char libc_locale_c[];           /* 0x08873BB8 "C" */
extern char libc_lconv_empty[];        /* 0x08873B78 "" (locale.c's literal pool, shared with lconv) */

/* newlib 1.12.0 _setlocale_r without MB_CAPABLE: only "C" and "" are accepted */
char *_setlocale_r(char *p, int category, const char *locale)
{
    if (locale) {
        if (strcmp(locale, libc_locale_c) && strcmp(locale, libc_lconv_empty)) {
            return 0;
        }
        *(int *)(p + 0x30) = category;           /* _current_category */
        *(const char **)(p + 0x34) = locale;     /* _current_locale */
    }
    return libc_locale_c;
}
#endif

int libc_locale_charset(void)
{
    return libc_charset;
}

void *_localeconv_r(void *r)
{
    return libc_lconv;
}

#ifdef __mips__
void *heap_get_current(void);

char *setlocale(int category, const char *locale)
{
    return _setlocale_r(heap_get_current(), category, locale);
}

void *localeconv(void)
{
    return _localeconv_r(heap_get_current());
}
#endif

int _lseek_r(int *r, int fd, int offset, int whence)
{
    int ret;

    libc_errno = 0;
    ret = _lseek(fd, offset, whence);
    if (ret == -1 && libc_errno != 0) {
        *r = libc_errno;
    }
    return ret;
}
