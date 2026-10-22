// @range 0x088126E8-0x08813108
// @asm 0x088126E8-0x088126F8
// @asm 0x08812784-0x08812948
// @asm 0x08812998-0x08812E14
// @asm 0x08812E64-0x08813108
/* newlib string to number conversions (stdlib/strtol.c, strtoul.c, strtoll.c, strtoull.c, strtod.c
   wrappers), compiled with GCC. The wrappers fetch the reentrancy structure (heap_get_current is
   newlib's __getreent) and are whole `asm` functions; the _r workers read the ctype table _ctype_
   (D_088739F0) through a split lui/addiu pair, which mwcc's inline assembler cannot relocate, so they
   stay in asm with their C under NON_MATCHING. Outside the PSP build the C is used as is (the errno
   of the reentrancy structure is its first word). */

typedef struct Reent {
    int err;                           /* _errno */
} Reent;

Reent *heap_get_current(void);
double func_088116A4(Reent *r, const char *s, char **end);   /* _strtod_r */
float __truncdfsf2(double d);
long _strtol_r(Reent *r, const char *s, char **end, int base);
unsigned long _strtoul_r(Reent *r, const char *s, char **end, int base);

#if !defined(__MWERKS__) || defined(NON_MATCHING)
#define ERANGE 34
/* the _ctype_ classes the workers test: space, digit, upper, lower */
#define C_SPACE(c) ((c) == ' ' || ((c) >= '\t' && (c) <= '\r'))
#define C_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define C_UPPER(c) ((c) >= 'A' && (c) <= 'Z')
#define C_LOWER(c) ((c) >= 'a' && (c) <= 'z')
#endif

/* 0x088126E8: a cold block of _strtod_r placed here by GCC (it bumps a local and jumps back into
   the function with an absolute `j`); not callable on its own, so it stays in asm and has no C. */

#ifdef __MWERKS__
asm double strtod(const char *s, char **end)
{
    addiu sp, sp, -16
    sw ra, 8(sp)
    sw s1, 4(sp)
    move s1, a1
    sw s0, 0(sp)
    jal heap_get_current
    move s0, a0
    move a0, v0
    move a1, s0
    jal func_088116A4
    move a2, s1
    lw ra, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
double strtod(const char *s, char **end)
{
    return func_088116A4(heap_get_current(), s, end);
}
#endif

#ifdef __MWERKS__
asm float strtof(const char *s, char **end)
{
    addiu sp, sp, -16
    sw ra, 8(sp)
    sw s1, 4(sp)
    move s1, a1
    sw s0, 0(sp)
    jal heap_get_current
    move s0, a0
    move a0, v0
    move a2, s1
    jal func_088116A4
    move a1, s0
    move a0, v0
    jal __truncdfsf2
    move a1, v1
    lw ra, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
float strtof(const char *s, char **end)
{
    return (float)func_088116A4(heap_get_current(), s, end);
}
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: _ctype_ through a split lui/addiu */
long _strtol_r(Reent *r, const char *nptr, char **end, int base)
{
    const char *s = nptr;
    unsigned long acc;
    unsigned long cutoff;
    int c;
    int neg = 0;
    int any;
    int cutlim;

    do {
        c = *s++;
    } while (C_SPACE(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+') {
        c = *s++;
    }
    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0) {
        base = c == '0' ? 8 : 10;
    }
    cutoff = neg ? 0x80000000UL : 0x7FFFFFFFUL;
    cutlim = (int)(cutoff % (unsigned long)base);
    cutoff /= (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (C_DIGIT(c)) {
            c -= '0';
        } else if (C_UPPER(c)) {
            c -= 'A' - 10;
        } else if (C_LOWER(c)) {
            c -= 'a' - 10;
        } else {
            break;
        }
        if (c >= base) {
            break;
        }
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc = acc * base + c;
        }
    }
    if (neg) {
        acc = -acc;
    }
    if (any < 0) {
        r->err = ERANGE;
        acc = neg ? 0x80000000UL : 0x7FFFFFFFUL;
    }
    if (end != 0) {
        *end = (char *)(any ? s - 1 : nptr);
    }
    return (long)acc;
}
#endif

#ifdef __MWERKS__
asm long strtol(const char *s, char **end, int base)
{
    addiu sp, sp, -16
    sw ra, 12(sp)
    sw s2, 8(sp)
    move s2, a2
    sw s1, 4(sp)
    move s1, a1
    sw s0, 0(sp)
    jal heap_get_current
    move s0, a0
    move a0, v0
    move a1, s0
    move a2, s1
    jal _strtol_r
    move a3, s2
    lw ra, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
long strtol(const char *s, char **end, int base)
{
    return _strtol_r(heap_get_current(), s, end, base);
}
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: _ctype_ through a split lui/addiu */
long long _strtoll_r(Reent *r, const char *nptr, char **end, int base)
{
    const char *s = nptr;
    unsigned long long acc;
    unsigned long long cutoff;
    int c;
    int neg = 0;
    int any;
    int cutlim;

    do {
        c = *s++;
    } while (C_SPACE(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+') {
        c = *s++;
    }
    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0) {
        base = c == '0' ? 8 : 10;
    }
    cutoff = neg ? 0x8000000000000000ULL : 0x7FFFFFFFFFFFFFFFULL;
    cutlim = (int)(cutoff % (unsigned long long)(long long)base);
    cutoff /= (unsigned long long)(long long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (C_DIGIT(c)) {
            c -= '0';
        } else if (C_UPPER(c)) {
            c -= 'A' - 10;
        } else if (C_LOWER(c)) {
            c -= 'a' - 10;
        } else {
            break;
        }
        if (c >= base) {
            break;
        }
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc = acc * base + c;
        }
    }
    if (any < 0) {
        acc = neg ? 0x8000000000000000ULL : 0x7FFFFFFFFFFFFFFFULL;
        r->err = ERANGE;
    } else if (neg) {
        acc = -acc;
    }
    if (end != 0) {
        *end = (char *)(any ? s - 1 : nptr);
    }
    return (long long)acc;
}

unsigned long _strtoul_r(Reent *r, const char *nptr, char **end, int base)
{
    const char *s = nptr;
    unsigned long acc;
    unsigned long cutoff;
    int c;
    int neg = 0;
    int any;
    int cutlim;

    do {
        c = *s++;
    } while (C_SPACE(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+') {
        c = *s++;
    }
    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0) {
        base = c == '0' ? 8 : 10;
    }
    cutoff = 0xFFFFFFFFUL / (unsigned long)base;
    cutlim = (int)(0xFFFFFFFFUL % (unsigned long)base);
    for (acc = 0, any = 0;; c = *s++) {
        if (C_DIGIT(c)) {
            c -= '0';
        } else if (C_UPPER(c)) {
            c -= 'A' - 10;
        } else if (C_LOWER(c)) {
            c -= 'a' - 10;
        } else {
            break;
        }
        if (c >= base) {
            break;
        }
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc = acc * base + c;
        }
    }
    if (any < 0) {
        acc = 0xFFFFFFFFUL;
        r->err = ERANGE;
    } else if (neg) {
        acc = -acc;
    }
    if (end != 0) {
        *end = (char *)(any ? s - 1 : nptr);
    }
    return acc;
}
#endif

#ifdef __MWERKS__
asm unsigned long strtoul(const char *s, char **end, int base)
{
    addiu sp, sp, -16
    sw ra, 12(sp)
    sw s2, 8(sp)
    move s2, a2
    sw s1, 4(sp)
    move s1, a1
    sw s0, 0(sp)
    jal heap_get_current
    move s0, a0
    move a0, v0
    move a1, s0
    move a2, s1
    jal _strtoul_r
    move a3, s2
    lw ra, 12(sp)
    lw s2, 8(sp)
    lw s1, 4(sp)
    lw s0, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
unsigned long strtoul(const char *s, char **end, int base)
{
    return _strtoul_r(heap_get_current(), s, end, base);
}
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: _ctype_ through a split lui/addiu */
unsigned long long _strtoull_r(Reent *r, const char *nptr, char **end, int base)
{
    const char *s = nptr;
    unsigned long long acc;
    unsigned long long cutoff;
    int c;
    int neg = 0;
    int any;
    int cutlim;

    do {
        c = *s++;
    } while (C_SPACE(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+') {
        c = *s++;
    }
    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0) {
        base = c == '0' ? 8 : 10;
    }
    cutoff = 0xFFFFFFFFFFFFFFFFULL / (unsigned long long)(long long)base;
    cutlim = (int)(0xFFFFFFFFFFFFFFFFULL % (unsigned long long)(long long)base);
    for (acc = 0, any = 0;; c = *s++) {
        if (C_DIGIT(c)) {
            c -= '0';
        } else if (C_UPPER(c)) {
            c -= 'A' - 10;
        } else if (C_LOWER(c)) {
            c -= 'a' - 10;
        } else {
            break;
        }
        if (c >= base) {
            break;
        }
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
        } else {
            any = 1;
            acc = acc * base + c;
        }
    }
    if (any < 0) {
        acc = 0xFFFFFFFFFFFFFFFFULL;
        r->err = ERANGE;
    } else if (neg) {
        acc = -acc;
    }
    if (end != 0) {
        *end = (char *)(any ? s - 1 : nptr);
    }
    return acc;
}
#endif
