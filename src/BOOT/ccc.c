// @range 0x08818E64-0x08819BEC
// @gcc 0x08818E64-0x08818F54
// @asm 0x08818F54-0x08818FD0
// @gcc 0x08818FD0-0x08819208
// @asm 0x08819208-0x08819394
// @gcc 0x08819394-0x08819A44
// @gcc 0x08819AE0-0x08819BEC
/* Sony's libccc (sceCcc*, character code conversion): UTF-8, UTF-16 and Shift JIS strings, the JIS
   X 0208/0213 code they decode to (bit 15 = plane 2) and the per-encoding error characters
   substituted for invalid input. JIS <-> UCS-2 mapping uses two tables installed by
   sceCccSetTable. Compiled with GCC; the functions without data references are whole `asm`
   functions, the ones that read the error characters or the tables (bss at 0x088F97C0) stay in asm
   with their C under NON_MATCHING. The string converters take the destination size in bytes, always
   terminate the output and return the number of characters converted. Outside the PSP build the C
   is used. Note: as in the original, sceCccDecodeUTF16 returns a surrogate pair as (high << 16) |
   low, which sceCccEncode* then treat as invalid, and sceCccDecodeSJIS maps lead byte 0xF1 to plane 2
   rows 9-10 instead of 3-4 (its row fix-up only covers 0xF0, 0xF2 and 0xF4). */

typedef unsigned char u8;
typedef unsigned short u16;

unsigned int ccc_decode_utf16(const u16 **s);
void ccc_encode_utf16(u16 **d, unsigned int c);
unsigned int ccc_decode_utf8(const u8 **s);
void ccc_encode_utf8(u8 **d, unsigned int c);
unsigned int ccc_decode_sjis(const u8 **s);
void ccc_encode_sjis(u8 **d, unsigned int jis);
#if defined(__mips__) && !defined(__MWERKS__)
/* separate objects of the library: the SDK's GCC (-O3) must not inline them into their callers; declared before
   the definitions (GCC 3.3 drops a function whose noinline comes after its body) */
unsigned int ccc_decode_utf16(const u16 **s) __attribute__((noinline));
unsigned int ccc_decode_utf8(const u8 **s) __attribute__((noinline));
void ccc_encode_utf16(u16 **d, unsigned int c) __attribute__((noinline));
void ccc_encode_utf8(u8 **d, unsigned int c) __attribute__((noinline));
unsigned int ccc_decode_sjis(const u8 **s) __attribute__((noinline));
void ccc_encode_sjis(u8 **d, unsigned int jis) __attribute__((noinline));
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING)
extern u16 D_088F97C0;                 /* error character for Shift JIS output (a JIS code) */
extern unsigned int D_088F97C4;        /* error character for UTF-8 */
extern unsigned int D_088F97C8;        /* error character for UTF-16 */
extern const u16 *D_088F97CC;          /* JIS -> UCS-2 table */
extern const u16 *D_088F97D0;          /* UCS-2 -> JIS table */
#endif

/* JIS X 0208/0213 code point check (rows 0x21-0x7E, columns 0x21-0x7E; plane 2 only rows 1, 3-5, 8,
   12-15 and 78-94) */
#define JIS_VALID(row, col, plane) ((row) - 0x21 < 94 && (col) >= 0x21 && (col) < 0x7F \
    && (!(plane) || !((row) == 0x22 || (row) == 0x26 || (row) == 0x27 || (row) - 0x29 < 3 || (row) - 0x30 < 62)))

#ifndef __MWERKS__ /* PSP: ccc_gcc.c */
void func_08818E64(void)
{
}
#else
void func_08818E64(void);
#endif

#ifndef __MWERKS__ /* PSP: ccc_gcc.c */
void func_08818E6C(void)
{
}
#else
void func_08818E6C(void);
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: error characters through split lui/lo */
unsigned int ccc_decode_utf16(const u16 **s)
{
    const u16 *p = *s;
    unsigned int c = *p++;
    unsigned int r;

    if ((u16)(c - 0xD800) < 0x400) {
        unsigned int c2 = *p++;

        r = (u16)(c2 - 0xDC00) < 0x400 ? (c << 16 | c2) : D_088F97C8;
    } else {
        r = (u16)(c - 0xDC00) < 0x400 ? D_088F97C8 : c;
    }
    *s = p;
    return r;
}

/* sceCccIsValidUTF16's test, which the SDK's GCC inlines into ccc_encode_utf16 */
static inline int utf16_ok(unsigned int c)
{
    if (c > 0x10FFFF) {
        return 0;
    }
    if (c - 0xD800 < 0x800) {
        return 0;
    }
    return 1;
}

void ccc_encode_utf16(u16 **d, unsigned int c)
{
    u16 *p = *d;

    if (!utf16_ok(c)) {
        c = D_088F97C8;
    }
    if (c < 0x10000) {
        *p = (u16)c;
    } else {
        c -= 0x10000;
        *p++ = (u16)((c >> 10) + 0xD800);
        *p = (u16)((c & 0x3FF) + 0xDC00);
    }
    *d = p + 1;
}

unsigned int ccc_decode_utf8(const u8 **s)
{
    const u8 *p = *s;
    unsigned int c = *p++;

    if (c & 0x80) {
        if ((u8)(c - 0xC2) < 51) {
            unsigned int k = 0x40;

            c &= 0x7F;
            do {
                c = ((c & (k - 1)) << 6) | (*p++ & 0x3F);
                k <<= 5;
            } while (c >= k);
        } else {
            c = D_088F97C4;
        }
    }
    *s = p;
    return c;
}

void ccc_encode_utf8(u8 **d, unsigned int c)
{
    u8 *p = *d;

    if ((int)c < 0) {
        c = D_088F97C4;
    }
    if (c < 0x80) {
        *p++ = (u8)c;
    } else {
        unsigned int lim = 0x800;
        int n = 1;
        unsigned int mask;
        u8 *q;

        if (c >= 0x800) {
            do {
                lim <<= 5;
                n++;
            } while (c >= lim);
        }
        q = p = p + n + 1;
        mask = ((1u << (7 - n)) - 1) & 0xFF;
        while (n--) {
            *--q = (u8)((c & 0x3F) | 0x80);
            c >>= 6;
        }
        q[-1] = (u8)(~mask | (c & (mask >> 1)));
    }
    *d = p;
}

unsigned int ccc_decode_sjis(const u8 **s)
{
    const u8 *p = *s;
    u8 c = *p++;
    u8 k = c - 0xA0;
    unsigned int r;

    if (c < 0x80 || k < 0x40 || c > 0xFC) {
        r = c;
    } else {
        unsigned int plane = 0;
        int row = 0;
        unsigned int col = 0;

        if ((u8)(c - 0x81) < 0x1F) {
            row = c * 2 - 0x101;
        } else if ((u8)(c - 0xE0) < 0x10) {
            row = c * 2 - 0x181;
        } else if ((u8)(c - 0xF0) < 0x0D) {
            row = c * 2 - 0x1D9;
            if (c >= 0xF4) {
                row = c * 2 - 0x19B;
            }
            plane = 1;
        }
        if (row != 0) {
            c = *p++;
            if ((u8)(c - 0x40) < 0x3F) {
                col = c - 0x3F;
            } else if ((u8)(c + 0x80) < 0x1F) {
                col = c - 0x40;
            } else if ((u8)(c - 0x9F) < 0x5E) {
                col = c - 0x9E;
                row++;
            }
            if (plane) {
                if (row == 7) {
                    row = 1;
                } else if (row == 11) {
                    row = 5;
                } else if (row == 0x4D) {
                    row = 15;
                }
            }
        }
        if (row == 0 || col == 0) {
            r = D_088F97C0;
        } else {
            r = (plane << 15 | (row + 0x20) << 8 | (col + 0x20)) & 0xFFFF;
        }
    }
    *s = p;
    return r;
}

/* sceCccIsValidJIS (ccc_is_valid_jis below), which the SDK's GCC expands inside ccc_encode_sjis */
static inline int ccc_is_valid_jis_inl(unsigned int c)
{
    unsigned int row;
    unsigned int col;
    unsigned int plane;

    c &= 0xFFFF;
    plane = c >> 15;
    row = (c >> 8) & 0x7F;
    col = c & 0x7F;
    if (row - 0x21 >= 94 || col < 0x21 || col >= 0x7F) {
        return 0;
    }
    if (plane) {
        if (row == 0x22 || row == 0x26 || row == 0x27 || row - 0x29 < 3 || row - 0x30 < 62) {
            return 0;
        }
    }
    return 1;
}

void ccc_encode_sjis(u8 **d, unsigned int jis)
{
    u8 *p = *d;
    unsigned int c = jis & 0xFFFF;

    if ((u16)(c - 0xA0) < 0x40 || c < 0x80) {
    } else if (!ccc_is_valid_jis_inl(c)) {
        c = D_088F97C0;
    }
    if ((u16)(c - 0xA0) < 0x40 || c < 0x80 || c - 0xFD < 3) {
        *p = (u8)c;
    } else {
        unsigned int row = (c >> 8) & 0x7F;
        unsigned int r = row - 0x20;
        unsigned int t = (c & 0x7F) - 0x20;

        if (!(c >> 15)) {
            *p++ = (u8)(r < 63 ? (row + 225) >> 1 : (row + 353) >> 1);
        } else if (r < 16) {
            *p++ = (u8)(((row + 447) >> 1) - 3 * (r >> 3));
        } else {
            *p++ = (u8)((row + 379) >> 1);
        }
        if (!(r & 1)) {
            *p = (u8)(t - 98);
        } else {
            *p = (u8)(t < 64 ? t + 63 : t + 64);
        }
    }
    *d = p + 1;
}
#endif

/* sceCccUTF8toUTF16 */
#ifndef __MWERKS__ /* PSP: ccc_gcc.c */
int str_to_wide(u16 *dst, unsigned int size, const u8 *src)
{
    int n = 0;
    unsigned int c;

    if (size < 2) {
        return n;
    }
    size -= 2;
    while ((c = ccc_decode_utf8(&src)) != 0) {
        u16 tmp[8];
        u16 *q = tmp;
        int k;

        ccc_encode_utf16(&q, c);
        k = q - tmp;
        if (size < (unsigned int)(k * 2)) {
            break;
        }
        size -= k * 2;
        {
            u16 *t = tmp;

            while (k--) {
                *dst++ = *t++;
            }
        }
        n++;
    }
    *dst = 0;
    return n;
}
#else
int str_to_wide(u16 *dst, unsigned int size, const u8 *src);
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: tables and error characters by lui/lo */
/* sceCccUCStoJIS and sceCccJIStoUCS (ccc_ucs_to_jis, ccc_jis_to_ucs below), which the SDK's GCC expands inside the
   string converters */
static inline unsigned int ccc_ucs_to_jis_inl(unsigned int ucs, unsigned int def)
{
    unsigned int j;

    def &= 0xFFFF;
    if (ucs <= 0xFFFF) {
        j = D_088F97D0[ucs];
        if (j) {
            return j;
        }
    }
    return def;
}

static inline unsigned int ccc_jis_to_ucs_inl(unsigned int jis, unsigned int def)
{
    unsigned int u = D_088F97CC[jis & 0xFFFF];

    return u ? u : def;
}

/* sceCccUTF8toSJIS */
int ccc_utf8_to_sjis(u8 *dst, unsigned int size, const u8 *src)
{
    int n = 0;
    unsigned int room;
    unsigned int c;

    if (size == 0) {
        return 0;
    }
    room = size - 1;
    while ((c = ccc_decode_utf8(&src)) != 0) {
        u8 tmp[16];
        u8 *q = tmp;
        u16 jis = ccc_ucs_to_jis_inl(c, D_088F97C0);
        unsigned int k;

        if (jis == 0) {
            break;
        }
        ccc_encode_sjis(&q, jis);
        k = q - tmp;
        if (room < k) {
            break;
        }
        room -= k;
        {
            u8 *t = tmp;

            while (k--) {
                *dst++ = *t++;
            }
        }
        n++;
    }
    *dst = 0;
    return n;
}

/* sceCccSJIStoUTF8 */
int ccc_sjis_to_utf8(u8 *dst, unsigned int size, const u8 *src)
{
    int n = 0;
    unsigned int room;
    unsigned int jis;

    if (size == 0) {
        return 0;
    }
    room = size - 1;
    while ((jis = ccc_decode_sjis(&src)) != 0) {
        u8 tmp[16];
        u8 *q = tmp;
        unsigned int def = D_088F97C4;
        unsigned int u = D_088F97CC[jis];
        unsigned int k;

        u = u ? u : def;
        if (u == 0) {
            break;
        }
        ccc_encode_utf8(&q, u);
        k = q - tmp;
        if (room < k) {
            break;
        }
        room -= k;
        {
            u8 *t = tmp;

            while (k--) {
                *dst++ = *t++;
            }
        }
        n++;
    }
    *dst = 0;
    return n;
}

/* sceCccSJIStoUTF16 */
int mbs_to_wcs(u16 *dst, unsigned int size, const u8 *src)
{
    int n = 0;
    unsigned int jis;

    if (size < 2) {
        return 0;
    }
    size -= 2;
    while ((jis = ccc_decode_sjis(&src)) != 0) {
        u16 tmp[8];
        u16 *q = tmp;
        unsigned int def = D_088F97C8;
        unsigned int u = D_088F97CC[jis];
        int k;

        ccc_encode_utf16(&q, u ? u : def);
        k = q - tmp;
        if (size < (unsigned int)(k * 2)) {
            break;
        }
        size -= k * 2;
        {
            u16 *t = tmp;

            while (k--) {
                *dst++ = *t++;
            }
        }
        n++;
    }
    *dst = 0;
    return n;
}

/* sceCccUTF16toSJIS */
int ccc_utf16_to_sjis(u8 *dst, unsigned int size, const u16 *src)
{
    int n = 0;
    unsigned int room;
    unsigned int u;

    if (size == 0) {
        return 0;
    }
    room = size - 1;
    while ((u = ccc_decode_utf16(&src)) != 0) {
        u8 tmp[16];
        u8 *q = tmp;
        u16 jis = ccc_ucs_to_jis_inl(u, D_088F97C0);
        unsigned int k;

        if (jis == 0) {
            break;
        }
        ccc_encode_sjis(&q, jis);
        k = q - tmp;
        if (room < k) {
            break;
        }
        room -= k;
        {
            u8 *t = tmp;

            while (k--) {
                *dst++ = *t++;
            }
        }
        n++;
    }
    *dst = 0;
    return n;
}
#endif

/* sceCccUTF16toUTF8 */
#ifndef __MWERKS__ /* PSP: ccc_gcc.c */
int wcs_to_utf8(u8 *dst, unsigned int size, const u16 *src)
{
    int n = 0;
    unsigned int room = size - 1;
    unsigned int u;

    if (size == 0) {
        return 0;
    }
    while ((u = ccc_decode_utf16(&src)) != 0) {
        u8 tmp[16];
        u8 *q = tmp;
        unsigned int k;

        ccc_encode_utf8(&q, u);
        k = q - tmp;
        if (room < k) {
            break;
        }
        room -= k;
        {
            u8 *t = tmp;

            while (k--) {
                *dst++ = *t++;
            }
        }
        n++;
    }
    *dst = 0;
    return n;
}
#else
int wcs_to_utf8(u8 *dst, unsigned int size, const u16 *src);
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: the globals through split lui/lo */
/* sceCccSetTable */
void mbs_set_tables(const u16 *jis2ucs, const u16 *ucs2jis)
{
    D_088F97CC = jis2ucs;
    D_088F97D0 = ucs2jis;
}

/* sceCccSetErrorCharSJIS: returns the previous one */
unsigned int ccc_set_error_char_sjis(unsigned int c)
{
    unsigned int old = D_088F97C0;

    D_088F97C0 = (u16)c;
    return old;
}

/* sceCccSetErrorCharUTF8 */
unsigned int ccc_set_error_char_utf8(unsigned int c)
{
    unsigned int old = D_088F97C4;

    D_088F97C4 = c;
    return old;
}

/* sceCccSetErrorCharUTF16 */
unsigned int ccc_set_error_char_utf16(unsigned int c)
{
    unsigned int old = D_088F97C8;

    D_088F97C8 = c;
    return old;
}
#endif

/* sceCccIsValidUCS4: below 2^31 */
#ifndef __MWERKS__ /* PSP: ccc_gcc.c */
int ccc_is_valid_ucs4(unsigned int c)
{
    return c < 0x80000000;
}
#else
int ccc_is_valid_ucs4(unsigned int c);
#endif

/* sceCccIsValidUCS2 */
#ifndef __MWERKS__ /* PSP: ccc_gcc.c */
int ccc_is_valid_ucs2(unsigned int c)
{
    if (c > 0xFFFF) {
        return 0;
    }
    return 1;
}
#else
int ccc_is_valid_ucs2(unsigned int c);
#endif

/* sceCccIsValidJIS */
#ifndef __MWERKS__ /* PSP: ccc_gcc.c */
int ccc_is_valid_jis(unsigned int c)
{
    unsigned int row;
    unsigned int col;
    unsigned int plane;

    c &= 0xFFFF;
    plane = c >> 15;
    row = (c >> 8) & 0x7F;
    col = c & 0x7F;
    if (row - 0x21 >= 94 || col < 0x21 || col >= 0x7F) {
        return 0;
    }
    if (plane) {
        if (row == 0x22 || row == 0x26 || row == 0x27 || row - 0x29 < 3 || row - 0x30 < 62) {
            return 0;
        }
    }
    return 1;
}
#else
int ccc_is_valid_jis(unsigned int c);
#endif

/* sceCccIsValidUTF8: any value below 2^31 can be encoded */
#ifndef __MWERKS__ /* PSP: ccc_gcc.c */
int ccc_is_valid_utf8(unsigned int c)
{
    return c < 0x80000000;
}
#else
int ccc_is_valid_utf8(unsigned int c);
#endif

/* sceCccIsValidUTF16: up to 0x10FFFF, surrogates excluded */
#ifndef __MWERKS__ /* PSP: ccc_gcc.c */
int ccc_is_valid_utf16(unsigned int c)
{
    if (c > 0x10FFFF) {
        return 0;
    }
    if (c - 0xD800 < 0x800) {
        return 0;
    }
    return 1;
}
#else
int ccc_is_valid_utf16(unsigned int c);
#endif

/* sceCccIsValidSJIS: ASCII, half-width kana (0xA0-0xDF) or a valid JIS code */
#ifdef __MWERKS__
asm int ccc_is_valid_sjis(unsigned int c)
{
    andi a0, a0, 0xffff
    addiu a1, a0, -160
    sltiu v1, a1, 64
    sltiu a2, a0, 128
    or a1, a2, v1
    bnez a1, L_08819AD8
    addiu v0, zero, 1
    ext a2, a0, 0x8, 0x7
    addiu t3, a2, -33
    andi a1, a0, 0x7f
    sltiu t2, t3, 94
    xori t0, t2, 0x1
    sltiu t1, a1, 33
    or a3, t0, t1
    bnez a3, L_08819AD0
    srl v1, a0, 0xf
    sltiu a0, a1, 127
    beqz a0, L_08819AD0
    nop
    beqz v1, L_08819AD8
    xori t7, a2, 0x26
    xori v0, a2, 0x22
    sltiu t5, v0, 1
    sltiu t6, t7, 1
    or t4, t5, t6
    bnez t4, L_08819AD0
    addiu t8, zero, 39
    beq a2, t8, L_08819AD0
    addiu v1, a2, -41
    sltiu t9, v1, 3
    bnez t9, L_08819AD0
    addiu a1, a2, -48
    sltiu a2, a1, 62
    beqzl a2, L_08819AD8
    addiu v0, zero, 1
L_08819AD0:
    jr ra
    move v0, zero
L_08819AD8:
    jr ra
    nop
}
#else
int ccc_is_valid_sjis(unsigned int c)
{
    c &= 0xFFFF;
    if (c - 0xA0 < 0x40 || c < 0x80) {
        return 1;
    }
    return JIS_VALID((c >> 8) & 0x7F, c & 0x7F, c >> 15);
}
#endif

/* sceCccStrlenUTF16: characters before the terminator */
#ifndef __MWERKS__ /* PSP: ccc_gcc.c */
int ccc_strlen_utf16(const u16 *s)
{
    int n = 0;

    while (ccc_decode_utf16(&s) != 0) {
        n++;
    }
    return n;
}
#else
int ccc_strlen_utf16(const u16 *s);
#endif

/* sceCccStrlenUTF8 */
#ifndef __MWERKS__ /* PSP: ccc_gcc.c */
int ccc_strlen_utf8(const u8 *s)
{
    int n = 0;

    while (ccc_decode_utf8(&s) != 0) {
        n++;
    }
    return n;
}
#else
int ccc_strlen_utf8(const u8 *s);
#endif

/* sceCccStrlenSJIS */
#ifndef __MWERKS__ /* PSP: ccc_gcc.c */
int ccc_strlen_sjis(const u8 *s)
{
    int n = 0;

    while (ccc_decode_sjis(&s) != 0) {
        n++;
    }
    return n;
}
#else
int ccc_strlen_sjis(const u8 *s);
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: the tables through split lui/lo */
/* sceCccUCStoJIS: def when unmapped */
unsigned int ccc_ucs_to_jis(unsigned int ucs, unsigned int def)
{
    unsigned int j;

    def &= 0xFFFF;
    if (ucs <= 0xFFFF) {
        j = D_088F97D0[ucs];
        if (j) {
            return j;
        }
    }
    return def;
}

/* sceCccJIStoUCS: def when unmapped */
unsigned int ccc_jis_to_ucs(unsigned int jis, unsigned int def)
{
    unsigned int u = D_088F97CC[jis & 0xFFFF];

    return u ? u : def;
}
#endif
