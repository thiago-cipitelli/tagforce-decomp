// @range 0x08818E64-0x08819BEC
// @asm 0x08818E74-0x08819394
// @asm 0x08819464-0x08819758
// @asm 0x08819758-0x08819868
// @asm 0x08819924-0x08819968
// @asm 0x08819B94-0x08819BEC
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

#ifdef __MWERKS__
asm void func_08818E64(void)
{
    jr ra
    nop
}
#else
void func_08818E64(void)
{
}
#endif

#ifdef __MWERKS__
asm void func_08818E6C(void)
{
    jr ra
    nop
}
#else
void func_08818E6C(void)
{
}
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

void ccc_encode_utf16(u16 **d, unsigned int c)
{
    u16 *p = *d;

    if (c > 0x10FFFF || c - 0xD800 < 0x800) {
        c = D_088F97C8;
    }
    if (c < 0x10000) {
        *p = (u16)c;
    } else {
        *p++ = (u16)(((c - 0x10000) >> 10) + 0xD800);
        *p = (u16)(((c - 0x10000) & 0x3FF) + 0xDC00);
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
        mask = ((1u << (7 - n)) - 1) & 0xFF;
        q = p = p + n + 1;
        while (n-- > 0) {
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
    unsigned int c = *p++;
    unsigned int plane = 0;
    int row = 0;
    unsigned int col = 0;

    if (c < 0x80 || c - 0xA0 < 0x40 || c > 0xFC) {
        *s = p;
        return c;
    }
    if (c - 0x81 < 0x1F) {
        row = c * 2 - 0x101;
    } else if (c - 0xE0 < 0x10) {
        row = c * 2 - 0x181;
    } else if (c - 0xF0 < 0x0D) {
        row = c >= 0xF4 ? c * 2 - 0x19B : c * 2 - 0x1D9;
        plane = 1;
    }
    if (row != 0) {
        unsigned int c2 = *p++;

        if (c2 - 0x40 < 0x3F) {
            col = c2 - 0x3F;
        } else if (c2 - 0x80 < 0x1F) {
            col = c2 - 0x40;
        } else if (c2 - 0x9F < 0x5E) {
            col = c2 - 0x9E;
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
    *s = p;
    if (row == 0 || col == 0) {
        return D_088F97C0;
    }
    return (plane << 15 | (row + 0x20) << 8 | (col + 0x20)) & 0xFFFF;
}

void ccc_encode_sjis(u8 **d, unsigned int jis)
{
    u8 *p = *d;
    unsigned int c = jis & 0xFFFF;

    if (!(c < 0x80 || (u16)(c - 0xA0) < 0x40)) {
        unsigned int row = (c >> 8) & 0x7F;
        unsigned int col = c & 0x7F;

        if (!JIS_VALID(row, col, c >> 15)) {
            c = D_088F97C0;
        }
    }
    if (c < 0x80 || (u16)(c - 0xA0) < 0x40 || c - 0xFD < 3) {
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
        if (r & 1) {
            *p = (u8)(t < 64 ? t + 63 : t + 64);
        } else {
            *p = (u8)(t - 98);
        }
    }
    *d = p + 1;
}
#endif

/* sceCccUTF8toUTF16 */
#ifdef __MWERKS__
asm int str_to_wide(u16 *dst, unsigned int size, const u8 *src)
{
    addiu sp, sp, -64
    sw s1, 36(sp)
    move s1, a1
    sltiu a1, a1, 2
    sw s3, 44(sp)
    move v1, zero
    addiu s1, s1, -2
    sw s2, 40(sp)
    addiu s3, zero, -1
    move s2, zero
    sw s0, 32(sp)
    move s0, a0
    sw ra, 48(sp)
    beqz a1, L_088193F0
    sw a2, 20(sp)
L_088193D0:
    lw ra, 48(sp)
    lw s3, 44(sp)
    lw s2, 40(sp)
    lw s1, 36(sp)
    lw s0, 32(sp)
    move v0, v1
    jr ra
    addiu sp, sp, 64
L_088193F0:
    jal ccc_decode_utf8
    addiu a0, sp, 20
    beqz v0, L_08819458
    move a1, v0
    addiu a0, sp, 16
    jal ccc_encode_utf16
    sw sp, 16(sp)
    lw v1, 16(sp)
    subu a0, v1, sp
    sra v1, a0, 0x1
    sll a0, v1, 0x1
    sltu a1, s1, a0
    bnez a1, L_08819458
    subu s1, s1, a0
    addiu v1, v1, -1
    beq v1, s3, L_08819450
    move a0, sp
    addiu a1, zero, -1
L_08819438:
    lhu a2, 0(a0)
    addiu v1, v1, -1
    addiu a0, a0, 2
    sh a2, 0(s0)
    bne v1, a1, L_08819438
    addiu s0, s0, 2
L_08819450:
    j L_088193F0
    addiu s2, s2, 1
L_08819458:
    sh zero, 0(s0)
    j L_088193D0
    move v1, s2
}
#else
int str_to_wide(u16 *dst, unsigned int size, const u8 *src)
{
    int n = 0;
    unsigned int room = size - 2;
    unsigned int c;

    if (size < 2) {
        return 0;
    }
    while ((c = ccc_decode_utf8(&src)) != 0) {
        u16 tmp[8];
        u16 *q = tmp;
        int k;
        int i;

        ccc_encode_utf16(&q, c);
        k = q - tmp;
        if (room < (unsigned int)(k * 2)) {
            break;
        }
        room -= k * 2;
        for (i = 0; i < k; i++) {
            *dst++ = tmp[i];
        }
        n++;
    }
    *dst = 0;
    return n;
}
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: tables and error characters by lui/lo */
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
        unsigned int jis = c <= 0xFFFF ? D_088F97D0[c] : 0;
        unsigned int k;
        unsigned int i;

        if (jis == 0) {
            jis = D_088F97C0;
        }
        if (jis == 0) {
            *dst = 0;
            return n;
        }
        ccc_encode_sjis(&q, jis);
        k = q - tmp;
        if (room < k) {
            break;
        }
        room -= k;
        for (i = 0; i < k; i++) {
            *dst++ = tmp[i];
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
    unsigned int u;

    if (size == 0) {
        return 0;
    }
    room = size - 1;
    while ((jis = ccc_decode_sjis(&src)) != 0 && ((u = D_088F97CC[jis]) != 0 || (u = D_088F97C4) != 0)) {
        u8 tmp[16];
        u8 *q = tmp;
        unsigned int k;
        unsigned int i;

        ccc_encode_utf8(&q, u);
        k = q - tmp;
        if (room < k) {
            break;
        }
        room -= k;
        for (i = 0; i < k; i++) {
            *dst++ = tmp[i];
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
        unsigned int u = D_088F97CC[jis];
        int k;
        int i;

        if (u == 0) {
            u = D_088F97C8;
        }
        ccc_encode_utf16(&q, u);
        k = q - tmp;
        if (size < (unsigned int)(k * 2)) {
            break;
        }
        size -= k * 2;
        for (i = 0; i < k; i++) {
            *dst++ = tmp[i];
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
        u16 jis = u <= 0xFFFF ? D_088F97D0[u] : 0;
        unsigned int k;
        unsigned int i;

        if (jis == 0) {
            jis = D_088F97C0;
        }
        if (jis == 0) {
            *dst = 0;
            return n;
        }
        ccc_encode_sjis(&q, jis);
        k = q - tmp;
        if (room < k) {
            break;
        }
        room -= k;
        for (i = 0; i < k; i++) {
            *dst++ = tmp[i];
        }
        n++;
    }
    *dst = 0;
    return n;
}
#endif

/* sceCccUTF16toUTF8 */
#ifdef __MWERKS__
asm int wcs_to_utf8(u8 *dst, unsigned int size, const u16 *src)
{
    addiu sp, sp, -64
    sw s3, 44(sp)
    move v0, zero
    addiu s3, zero, -1
    sw s2, 40(sp)
    move s2, zero
    sw s1, 36(sp)
    addiu s1, a1, -1
    sw s0, 32(sp)
    move s0, a0
    sw ra, 48(sp)
    bnez a1, L_088198B8
    sw a2, 20(sp)
L_0881989C:
    lw ra, 48(sp)
    lw s3, 44(sp)
    lw s2, 40(sp)
    lw s1, 36(sp)
    lw s0, 32(sp)
    jr ra
    addiu sp, sp, 64
L_088198B8:
    jal ccc_decode_utf16
    addiu a0, sp, 20
    beqz v0, L_08819918
    move a1, v0
    addiu a0, sp, 16
    jal ccc_encode_utf8
    sw sp, 16(sp)
    lw v1, 16(sp)
    subu a0, v1, sp
    sltu v0, s1, a0
    bnez v0, L_08819918
    subu s1, s1, a0
    addiu a0, a0, -1
    beq a0, s3, L_08819910
    move v1, sp
    addiu a1, zero, -1
L_088198F8:
    lbu a2, 0(v1)
    addiu a0, a0, -1
    addiu v1, v1, 1
    sb a2, 0(s0)
    bne a0, a1, L_088198F8
    addiu s0, s0, 1
L_08819910:
    j L_088198B8
    addiu s2, s2, 1
L_08819918:
    sb zero, 0(s0)
    j L_0881989C
    move v0, s2
}
#else
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
        unsigned int i;

        ccc_encode_utf8(&q, u);
        k = q - tmp;
        if (room < k) {
            break;
        }
        room -= k;
        for (i = 0; i < k; i++) {
            *dst++ = tmp[i];
        }
        n++;
    }
    *dst = 0;
    return n;
}
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
#ifdef __MWERKS__
asm int ccc_is_valid_ucs4(unsigned int c)
{
    nor v1, zero, a0
    jr ra
    srl v0, v1, 0x1f
}
#else
int ccc_is_valid_ucs4(unsigned int c)
{
    return c < 0x80000000;
}
#endif

/* sceCccIsValidUCS2 */
#ifdef __MWERKS__
asm int ccc_is_valid_ucs2(unsigned int c)
{
    ori a1, zero, 0xffff
    sltu v1, a1, a0
    jr ra
    sltiu v0, v1, 1
}
#else
int ccc_is_valid_ucs2(unsigned int c)
{
    return c <= 0xFFFF;
}
#endif

/* sceCccIsValidJIS */
#ifdef __MWERKS__
asm int ccc_is_valid_jis(unsigned int c)
{
    andi t0, a0, 0xffff
    ext a2, t0, 0x8, 0x7
    addiu a3, a2, -33
    sltiu v1, a3, 94
    andi a1, t0, 0x7f
    xori t1, v1, 0x1
    sltiu t2, a1, 33
    or a3, t1, t2
    bnez a3, L_088199F8
    srl a0, t0, 0xf
    sltiu t3, a1, 127
    beqz t3, L_088199F8
    nop
    beqz a0, L_08819A00
    xori t5, a2, 0x26
    xori v0, a2, 0x22
    sltiu a1, v0, 1
    sltiu t4, t5, 1
    or a0, a1, t4
    bnez a0, L_088199F8
    addiu t6, zero, 39
    beq a2, t6, L_088199F8
    addiu t8, a2, -41
    sltiu t7, t8, 3
    bnez t7, L_088199F8
    addiu t9, a2, -48
    sltiu a2, t9, 62
    beqz a2, L_08819A00
    nop
L_088199F8:
    jr ra
    move v0, zero
L_08819A00:
    jr ra
    addiu v0, zero, 1
}
#else
int ccc_is_valid_jis(unsigned int c)
{
    unsigned int row = (c >> 8) & 0x7F;
    unsigned int col = c & 0x7F;

    return JIS_VALID(row, col, (c & 0xFFFF) >> 15);
}
#endif

/* sceCccIsValidUTF8: any value below 2^31 can be encoded */
#ifdef __MWERKS__
asm int ccc_is_valid_utf8(unsigned int c)
{
    nor v1, zero, a0
    jr ra
    srl v0, v1, 0x1f
}
#else
int ccc_is_valid_utf8(unsigned int c)
{
    return c < 0x80000000;
}
#endif

/* sceCccIsValidUTF16: up to 0x10FFFF, surrogates excluded */
#ifdef __MWERKS__
asm int ccc_is_valid_utf16(unsigned int c)
{
    lui v1, 0x10
    lui t0, 0xffff
    ori a3, v1, 0xffff
    ori a1, t0, 0x2800
    addu a2, a0, a1
    sltu a1, a3, a0
    sltiu v1, a2, 2048
    bnez a1, L_08819A3C
    move a0, zero
    sltiu a0, v1, 1
L_08819A3C:
    jr ra
    move v0, a0
}
#else
int ccc_is_valid_utf16(unsigned int c)
{
    return c <= 0x10FFFF && !(c - 0xD800 < 0x800);
}
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
#ifdef __MWERKS__
asm int ehsys_08819AE0(const u16 *s)
{
    addiu sp, sp, -32
    sw s0, 16(sp)
    move s0, zero
    sw ra, 20(sp)
    sw a0, 0(sp)
L_08819AF4:
    jal ccc_decode_utf16
    move a0, sp
    beqz v0, L_08819B0C
    move v0, s0
    j L_08819AF4
    addiu s0, s0, 1
L_08819B0C:
    lw ra, 20(sp)
    lw s0, 16(sp)
    jr ra
    addiu sp, sp, 32
}
#else
int ehsys_08819AE0(const u16 *s)
{
    int n = 0;

    while (ccc_decode_utf16(&s) != 0) {
        n++;
    }
    return n;
}
#endif

/* sceCccStrlenUTF8 */
#ifdef __MWERKS__
asm int ccc_strlen_utf8(const u8 *s)
{
    addiu sp, sp, -32
    sw s0, 16(sp)
    move s0, zero
    sw ra, 20(sp)
    sw a0, 0(sp)
L_08819B30:
    jal ccc_decode_utf8
    move a0, sp
    beqz v0, L_08819B48
    move v0, s0
    j L_08819B30
    addiu s0, s0, 1
L_08819B48:
    lw ra, 20(sp)
    lw s0, 16(sp)
    jr ra
    addiu sp, sp, 32
}
#else
int ccc_strlen_utf8(const u8 *s)
{
    int n = 0;

    while (ccc_decode_utf8(&s) != 0) {
        n++;
    }
    return n;
}
#endif

/* sceCccStrlenSJIS */
#ifdef __MWERKS__
asm int ehsys_08819B58(const u8 *s)
{
    addiu sp, sp, -32
    sw s0, 16(sp)
    move s0, zero
    sw ra, 20(sp)
    sw a0, 0(sp)
L_08819B6C:
    jal ccc_decode_sjis
    move a0, sp
    beqz v0, L_08819B84
    move v0, s0
    j L_08819B6C
    addiu s0, s0, 1
L_08819B84:
    lw ra, 20(sp)
    lw s0, 16(sp)
    jr ra
    addiu sp, sp, 32
}
#else
int ehsys_08819B58(const u8 *s)
{
    int n = 0;

    while (ccc_decode_sjis(&s) != 0) {
        n++;
    }
    return n;
}
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: the tables through split lui/lo */
/* sceCccUCStoJIS: def when unmapped */
unsigned int ccc_ucs_to_jis(unsigned int ucs, unsigned int def)
{
    unsigned int j;

    if (ucs > 0xFFFF) {
        return def & 0xFFFF;
    }
    j = D_088F97D0[ucs];
    return j ? j : (def & 0xFFFF);
}

/* sceCccJIStoUCS: def when unmapped */
unsigned int ccc_jis_to_ucs(unsigned int jis, unsigned int def)
{
    unsigned int u = D_088F97CC[jis & 0xFFFF];

    return u ? u : def;
}
#endif
