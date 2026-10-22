/* newlib internals used by the reconstructed stdio/stdlib files (src/BOOT/ungetc.c, vfprintf.c,
   vfscanf.c, wcrtomb.c, wsetup.c): the FILE and reentrancy layouts as the GCC-compiled newlib in
   BOOT uses them, and readable names for the newlib routines below 0x088126E8 that are still unnamed
   in symbols/BOOT.txt (rename the right-hand side when they get names). */
#ifndef NL_STDIO_H
#define NL_STDIO_H

typedef struct NlBuf {
    unsigned char *base;
    int size;
} NlBuf;

typedef struct NlFile {                /* struct __sFILE */
    unsigned char *p;                  /* current position in the buffer */
    int r;                             /* +0x04 read space left */
    int w;                             /* +0x08 write space left */
    short flags;                       /* +0x0C __S* */
    short file;                        /* +0x0E descriptor */
    NlBuf bf;                          /* +0x10 the buffer */
    int lbfsize;                       /* +0x18 0 or -bf.size (line buffered) */
    void *cookie;                      /* +0x1C */
    int (*read)(void *, char *, int);  /* +0x20 */
    int (*write)(void *, const char *, int); /* +0x24 */
    long (*seek)(void *, long, int);   /* +0x28 */
    int (*close)(void *);              /* +0x2C */
    NlBuf ub;                          /* +0x30 ungetc buffer */
    unsigned char *up;                 /* +0x38 saved p while ungetc is active */
    int ur;                            /* +0x3C saved r */
    unsigned char ubuf[3];             /* +0x40 */
    unsigned char nbuf[1];             /* +0x43 */
    NlBuf lb;                          /* +0x44 fgetline buffer */
    int blksize;                       /* +0x4C */
    int offset;                        /* +0x50 */
    struct NlReent *data;              /* +0x54 */
} NlFile;

#define NL_SLBF 0x0001                 /* line buffered */
#define NL_SNBF 0x0002                 /* unbuffered */
#define NL_SRD 0x0004                  /* reading */
#define NL_SWR 0x0008                  /* writing */
#define NL_SRW 0x0010                  /* open for reading and writing */
#define NL_SEOF 0x0020
#define NL_SERR 0x0040
#define NL_SMBF 0x0080                 /* bf.base from malloc */
#define NL_SSTR 0x0200                 /* string stream (sprintf/sscanf) */

typedef struct NlReent {               /* struct _reent */
    int err;                           /* _errno */
    unsigned char pad[0x34];
    int sdidinit;                      /* +0x38 __sdidinit */
} NlReent;

typedef struct NlIov {
    const void *base;
    int len;
} NlIov;

typedef struct NlUio {
    NlIov *iov;
    int iovcnt;
    int resid;
} NlUio;

typedef struct NlMbState {             /* mbstate_t */
    int count;
    union {
        unsigned int wch;
        unsigned char bytes[4];
    } value;
} NlMbState;

#define EILSEQ 138

NlReent *heap_get_current(void);       /* __getreent */
void libc_sinit(NlReent *r);        /* __sinit */
int fflush(NlFile *fp);        /* fflush */
void *_malloc_r(NlReent *r, unsigned int n);            /* _malloc_r */
void *_realloc_r(NlReent *r, void *p, unsigned int n);   /* _realloc_r */
int libc_sfvwrite(NlFile *fp, NlUio *uio);                  /* __sfvwrite */

#define nl_getreent heap_get_current
#define nl_sinit libc_sinit
#define nl_fflush fflush
#define nl_malloc_r _malloc_r
#define nl_realloc_r _realloc_r
#define nl_sfvwrite libc_sfvwrite

#endif
