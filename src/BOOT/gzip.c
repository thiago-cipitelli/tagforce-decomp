// @range 0x0881F6E4-0x0881FFB0
/* Sony's libdeflt: gzip container (RFC 1952) and raw deflate decoder (RFC 1951). The gzip helpers are
   GCC-compiled C and the decoder is hand-written (one routine with local subroutines called through
   jal/bltzal and the fixed-code tables at D_08874F00), so all of them stay whole `asm` functions here;
   outside the PSP build the C versions below do the same work. Errors are the SDK codes: 0x80000103
   null buffer, 0x80000104 output full, 0x80000108 bad data, 0x80000004 not a deflate stream. */

#define DEFLT_ERR_NULL 0x80000103
#define DEFLT_ERR_OVERFLOW 0x80000104
#define DEFLT_ERR_DATA 0x80000108
#define DEFLT_ERR_METHOD 0x80000004

extern unsigned char D_08874F00[];     /* decoder tables (fixed literal/distance codes) */

int gzip_get_info(const void *buf, const void **extra, const char **name, const char **comment,
                  unsigned short *crc16, const void **data);
const void *gzip_get_data(const void *buf);
int deflate_decompress(void *dst, unsigned int dst_len, const void *src, const void **next);

#ifndef __MWERKS__
typedef struct Inflate {
    const unsigned char *in;
    unsigned int bitbuf;
    int bitcnt;
    unsigned char *out;
    unsigned char *base;
    unsigned char *end;
} Inflate;

typedef struct Huffman {
    short count[16];                   /* codes per length */
    short symbol[288];                 /* symbols in canonical order */
} Huffman;

static unsigned int inf_bits(Inflate *s, int n)
{
    unsigned int v;

    while (s->bitcnt < n) {
        s->bitbuf |= (unsigned int)*s->in++ << s->bitcnt;
        s->bitcnt += 8;
    }
    v = s->bitbuf & ((1u << n) - 1);
    s->bitbuf >>= n;
    s->bitcnt -= n;
    return v;
}

/* builds the canonical code of n lengths; 0 when it is over-subscribed */
static int inf_build(Huffman *h, const unsigned char *len, int n)
{
    short offs[16];
    int left = 1;
    int i;

    for (i = 0; i < 16; i++) {
        h->count[i] = 0;
    }
    for (i = 0; i < n; i++) {
        h->count[len[i]]++;
    }
    for (i = 1; i < 16; i++) {
        left = left * 2 - h->count[i];
        if (left < 0) {
            return 0;
        }
    }
    offs[1] = 0;
    for (i = 1; i < 15; i++) {
        offs[i + 1] = offs[i] + h->count[i];
    }
    for (i = 0; i < n; i++) {
        if (len[i] != 0) {
            h->symbol[offs[len[i]]++] = (short)i;
        }
    }
    return 1;
}

static int inf_decode(Inflate *s, const Huffman *h)
{
    int code = 0;
    int first = 0;
    int index = 0;
    int i;

    for (i = 1; i < 16; i++) {
        int c;

        code |= (int)inf_bits(s, 1);
        c = h->count[i];
        if (code - c < first) {
            return h->symbol[index + (code - first)];
        }
        index += c;
        first = (first + c) << 1;
        code <<= 1;
    }
    return -1;
}

static int inf_codes(Inflate *s, const Huffman *lit, const Huffman *dist)
{
    static const short lbase[29] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
                                     35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258 };
    static const short lext[29] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };
    static const short dbase[30] = { 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769,
                                     1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577 };
    static const short dext[30] = { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };

    for (;;) {
        int sym = inf_decode(s, lit);

        if (sym < 0 || sym > 285) {
            return (int)DEFLT_ERR_DATA;
        }
        if (sym < 256) {
            if (s->out == s->end) {
                return (int)DEFLT_ERR_OVERFLOW;
            }
            *s->out++ = (unsigned char)sym;
        } else if (sym == 256) {
            return 0;
        } else {
            int len;
            int d;
            unsigned int back;

            sym -= 257;
            len = lbase[sym] + (int)inf_bits(s, lext[sym]);
            d = inf_decode(s, dist);
            if (d < 0 || d > 29) {
                return (int)DEFLT_ERR_DATA;
            }
            back = dbase[d] + inf_bits(s, dext[d]);
            if (back > (unsigned int)(s->out - s->base)) {
                return (int)DEFLT_ERR_DATA;
            }
            if ((unsigned int)(s->end - s->out) < (unsigned int)len) {
                return (int)DEFLT_ERR_OVERFLOW;
            }
            while (len-- > 0) {
                *s->out = s->out[-(int)back];
                s->out++;
            }
        }
    }
}

static int inf_dynamic(Inflate *s, Huffman *lit, Huffman *dist)
{
    static const unsigned char order[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
    unsigned char len[320];
    int nlen = (int)inf_bits(s, 5) + 257;
    int ndist = (int)inf_bits(s, 5) + 1;
    int ncode = (int)inf_bits(s, 4) + 4;
    int i;

    if (nlen > 286 || ndist > 30) {
        return (int)DEFLT_ERR_DATA;
    }
    for (i = 0; i < 19; i++) {
        len[order[i]] = i < ncode ? (unsigned char)inf_bits(s, 3) : 0;
    }
    if (!inf_build(lit, len, 19)) {
        return (int)DEFLT_ERR_DATA;
    }
    for (i = 0; i < nlen + ndist;) {
        int sym = inf_decode(s, lit);
        int rep;
        unsigned char v = 0;

        if (sym < 0) {
            return (int)DEFLT_ERR_DATA;
        }
        if (sym < 16) {
            len[i++] = (unsigned char)sym;
            continue;
        }
        if (sym == 16) {
            if (i == 0) {
                return (int)DEFLT_ERR_DATA;
            }
            v = len[i - 1];
            rep = 3 + (int)inf_bits(s, 2);
        } else if (sym == 17) {
            rep = 3 + (int)inf_bits(s, 3);
        } else {
            rep = 11 + (int)inf_bits(s, 7);
        }
        if (i + rep > nlen + ndist) {
            return (int)DEFLT_ERR_DATA;
        }
        while (rep-- > 0) {
            len[i++] = v;
        }
    }
    if (!inf_build(lit, len, nlen) || !inf_build(dist, len + nlen, ndist)) {
        return (int)DEFLT_ERR_DATA;
    }
    return inf_codes(s, lit, dist);
}

static int inf_fixed(Inflate *s, Huffman *lit, Huffman *dist)
{
    unsigned char len[288];
    int i;

    for (i = 0; i < 288; i++) {
        len[i] = i < 144 ? 8 : i < 256 ? 9 : i < 280 ? 7 : 8;
    }
    inf_build(lit, len, 288);
    for (i = 0; i < 30; i++) {
        len[i] = 5;
    }
    inf_build(dist, len, 30);
    return inf_codes(s, lit, dist);
}
#endif

/* Decompresses the gzip file at src into dst (at most dst_len bytes); returns the size (checked
   against the trailer's ISIZE) or an error. *crc32 (if crc32 != 0) receives the trailer's CRC-32,
   which is not verified. */
#ifdef __MWERKS__
asm int gzip_decompress(void *dst, unsigned int dst_len, const void *src, unsigned int *crc32)
{
    addiu sp, sp, -64
    sw s2, 40(sp)
    move s2, a0
    move a0, a2
    sw s3, 44(sp)
    move s3, a3
    sw s1, 36(sp)
    move s1, a1
    sw s0, 32(sp)
    sw ra, 48(sp)
    jal gzip_get_data
    move s0, a2
    lui v1, 0x8000
    beqz v0, L_0881F7A0
    ori t0, v1, 0x108
    lbu t2, 2(s0)
    move a2, v0
    addiu t1, zero, 8
    lui v0, 0x8000
    move a0, s2
    move a1, s1
    addiu a3, sp, 16
    bne t2, t1, L_0881F7A0
    ori t0, v0, 0x4
    jal deflate_decompress
    nop
    move a0, v0
    bltz v0, L_0881F7A0
    move t0, v0
    lw t3, 16(sp)
    lui v1, 0x8000
    ori t0, v1, 0x108
    lwl v1, 3(t3)
    lwr v1, 0(t3)
    lwl a1, 7(t3)
    lwr a1, 4(t3)
    swl v1, 3(sp)
    swr v1, 0(sp)
    swl a1, 7(sp)
    swr a1, 4(sp)
    lw a1, 4(sp)
    bne v0, a1, L_0881F7A4
    lw ra, 48(sp)
    beqz s3, L_0881F79C
    lw a2, 0(sp)
    sw a2, 0(s3)
L_0881F79C:
    move t0, a0
L_0881F7A0:
    lw ra, 48(sp)
L_0881F7A4:
    lw s3, 44(sp)
    lw s2, 40(sp)
    lw s1, 36(sp)
    lw s0, 32(sp)
    move v0, t0
    jr ra
    addiu sp, sp, 64
}
#else
int gzip_decompress(void *dst, unsigned int dst_len, const void *src, unsigned int *crc32)
{
    const unsigned char *data = gzip_get_data(src);
    const unsigned char *tail;
    int n;

    if (data == 0) {
        return (int)DEFLT_ERR_DATA;
    }
    if (((const unsigned char *)src)[2] != 8) {
        return (int)DEFLT_ERR_METHOD;
    }
    n = deflate_decompress(dst, dst_len, data, (const void **)&tail);
    if (n < 0) {
        return n;
    }
    if ((unsigned int)n != (tail[4] | tail[5] << 8 | tail[6] << 16 | (unsigned int)tail[7] << 24)) {
        return (int)DEFLT_ERR_DATA;
    }
    if (crc32 != 0) {
        *crc32 = tail[0] | tail[1] << 8 | tail[2] << 16 | (unsigned int)tail[3] << 24;
    }
    return n;
}
#endif

/* Parses the gzip header at buf: each non-null output receives the FEXTRA field (pointing at its
   XLEN; as in the original the skip adds XLEN without the 2 XLEN bytes), the FNAME and FCOMMENT
   strings, the FHCRC value and the start of the compressed data. Returns 0 or an error. */
#ifdef __MWERKS__
asm int gzip_get_info(const void *buf, const void **extra, const char **name, const char **comment,
                      unsigned short *crc16, const void **data)
{
    addiu sp, sp, -16
    beqz a1, L_0881F7D0
    addiu t2, a0, 10
    sw zero, 0(a1)
L_0881F7D0:
    bnezl a2, L_0881F7D8
    sw zero, 0(a2)
L_0881F7D8:
    bnezl a3, L_0881F7E0
    sw zero, 0(a3)
L_0881F7E0:
    bnezl t0, L_0881F7E8
    sh zero, 0(t0)
L_0881F7E8:
    bnezl t1, L_0881F7F0
    sw zero, 0(t1)
L_0881F7F0:
    lui v1, 0x8000
    beqz a0, L_0881F850
    ori v0, v1, 0x103
    lwl v0, 3(a0)
    lwr v0, 0(a0)
    lwl v1, 7(a0)
    lwr v1, 4(a0)
    lb t3, 8(a0)
    lb t4, 9(a0)
    swl v0, 3(sp)
    swr v0, 0(sp)
    swl v1, 7(sp)
    swr v1, 4(sp)
    sb t3, 8(sp)
    sb t4, 9(sp)
    addiu t3, zero, 31
    lbu v0, 0(sp)
    bne v0, t3, L_0881F848
    lbu t5, 1(sp)
    addiu t4, zero, 139
    beq t5, t4, L_0881F858
    lbu t3, 3(sp)
L_0881F848:
    lui v1, 0x8000
    ori v0, v1, 0x108
L_0881F850:
    jr ra
    addiu sp, sp, 16
L_0881F858:
    andi t6, t3, 0x4
    beqzl t6, L_0881F884
    andi a1, t3, 0x8
    lbu t8, 1(t2)
    lbu t7, 10(a0)
    sll a0, t8, 0x8
    beqz a1, L_0881F87C
    or v1, t7, a0
    sw t2, 0(a1)
L_0881F87C:
    addu t2, t2, v1
    andi a1, t3, 0x8
L_0881F884:
    beqz a1, L_0881F8B0
    andi t4, t3, 0x10
    bnezl a2, L_0881F894
    sw t2, 0(a2)
L_0881F894:
    lbu a2, 0(t2)
    beqz a2, L_0881F8AC
    addiu t2, t2, 1
L_0881F8A0:
    lbu t9, 0(t2)
    bnez t9, L_0881F8A0
    addiu t2, t2, 1
L_0881F8AC:
    andi t4, t3, 0x10
L_0881F8B0:
    beqz t4, L_0881F8DC
    andi v1, t3, 0x2
    bnezl a3, L_0881F8C0
    sw t2, 0(a3)
L_0881F8C0:
    lbu a3, 0(t2)
    beqz a3, L_0881F8D8
    addiu t2, t2, 1
L_0881F8CC:
    lbu t5, 0(t2)
    bnez t5, L_0881F8CC
    addiu t2, t2, 1
L_0881F8D8:
    andi v1, t3, 0x2
L_0881F8DC:
    beqz v1, L_0881F904
    nop
    beqzl t0, L_0881F904
    addiu t2, t2, 2
    lbu a0, 1(t2)
    lbu t6, 0(t2)
    sll t7, a0, 0x8
    or t3, t6, t7
    sh t3, 0(t0)
    addiu t2, t2, 2
L_0881F904:
    bnezl t1, L_0881F90C
    sw t2, 0(t1)
L_0881F90C:
    j L_0881F850
    move v0, zero
}
#else
int gzip_get_info(const void *buf, const void **extra, const char **name, const char **comment,
                  unsigned short *crc16, const void **data)
{
    const unsigned char *h = buf;
    const unsigned char *p;
    unsigned char flags;

    if (extra != 0) {
        *extra = 0;
    }
    if (name != 0) {
        *name = 0;
    }
    if (comment != 0) {
        *comment = 0;
    }
    if (crc16 != 0) {
        *crc16 = 0;
    }
    if (data != 0) {
        *data = 0;
    }
    if (h == 0) {
        return (int)DEFLT_ERR_NULL;
    }
    if (h[0] != 0x1F || h[1] != 0x8B) {
        return (int)DEFLT_ERR_DATA;
    }
    flags = h[3];
    p = h + 10;
    if (flags & 4) {
        unsigned int xlen = p[0] | p[1] << 8;

        if (extra != 0) {
            *extra = p;
        }
        p += xlen;
    }
    if (flags & 8) {
        if (name != 0) {
            *name = (const char *)p;
        }
        while (*p++ != 0) {
        }
    }
    if (flags & 0x10) {
        if (comment != 0) {
            *comment = (const char *)p;
        }
        while (*p++ != 0) {
        }
    }
    if (flags & 2) {
        if (crc16 != 0) {
            *crc16 = (unsigned short)(p[0] | p[1] << 8);
        }
        p += 2;
    }
    if (data != 0) {
        *data = p;
    }
    return 0;
}
#endif

/* start of the compressed data, 0 on a bad header */
#ifdef __MWERKS__
asm const void *gzip_get_data(const void *buf)
{
    addiu sp, sp, -32
    move a1, zero
    move a2, zero
    move a3, zero
    move t0, zero
    sw ra, 16(sp)
    jal gzip_get_info
    move t1, sp
    slti a0, v0, 0
    lw v0, 0(sp)
    lw ra, 16(sp)
    movn v0, zero, a0
    jr ra
    addiu sp, sp, 32
}
#else
const void *gzip_get_data(const void *buf)
{
    const void *p;

    return gzip_get_info(buf, 0, 0, 0, 0, &p) < 0 ? 0 : p;
}
#endif

/* 1 when buf starts with the gzip magic 1F 8B */
#ifdef __MWERKS__
asm int is_gzip(const void *buf)
{
    lbu v0, 0(a0)
    addiu a2, zero, 31
    beq v0, a2, L_0881F968
    addiu a1, zero, 1
    move a1, zero
L_0881F960:
    jr ra
    move v0, a1
L_0881F968:
    lbu a2, 1(a0)
    addiu a0, zero, 139
    bnel a2, a0, L_0881F960
    move a1, zero
    j L_0881F960
    nop
}
#else
int is_gzip(const void *buf)
{
    const unsigned char *h = buf;

    return h[0] == 0x1F && h[1] == 0x8B;
}
#endif

/* FNAME of the header, 0 when absent or on a bad header */
#ifdef __MWERKS__
asm const char *gzip_get_name(const void *buf)
{
    addiu sp, sp, -32
    move a1, zero
    move a2, sp
    move a3, zero
    move t0, zero
    sw ra, 16(sp)
    jal gzip_get_info
    move t1, zero
    slti a0, v0, 0
    lw v0, 0(sp)
    lw ra, 16(sp)
    movn v0, zero, a0
    jr ra
    addiu sp, sp, 32
}
#else
const char *gzip_get_name(const void *buf)
{
    const char *p;

    return gzip_get_info(buf, 0, &p, 0, 0, 0) < 0 ? 0 : p;
}
#endif

/* FCOMMENT of the header, 0 when absent or on a bad header */
#ifdef __MWERKS__
asm const char *gzip_get_comment(const void *buf)
{
    addiu sp, sp, -32
    move a1, zero
    move a2, zero
    move a3, sp
    move t0, zero
    sw ra, 16(sp)
    jal gzip_get_info
    move t1, zero
    slti a0, v0, 0
    lw v0, 0(sp)
    lw ra, 16(sp)
    movn v0, zero, a0
    jr ra
    addiu sp, sp, 32
}
#else
const char *gzip_get_comment(const void *buf)
{
    const char *p;

    return gzip_get_info(buf, 0, 0, &p, 0, 0) < 0 ? 0 : p;
}
#endif

/* Decodes the raw deflate stream at src into dst (at most dst_len bytes); returns the size or an
   error. On success *next (if next != 0) receives the first byte after the stream. */
#ifdef __MWERKS__
asm int deflate_decompress(void *dst, unsigned int dst_len, const void *src, const void **next)
{
    addiu sp, sp, -896
    sw s4, 776(sp)
    addu a1, a0, a1
    move t6, a0
    la t7, D_08874F00
    sw ra, 784(sp)
L_0881FA0C:
    andi t9, a2, 0x3
    subu a2, a2, t9
    lw t8, 0(a2)
    sll t9, t9, 0x3
    addiu t9, t9, -32
L_0881FA20:
    addiu t9, t9, 3
    blez t9, L_0881FA44
    rotrv v0, t8, t9
    addiu a2, a2, 4
    lw t8, 0(a2)
    sllv v0, v0, t9
    addiu t9, t9, -32
    ins v0, t8, 0x0, 0x10
    rotrv v0, v0, t9
L_0881FA44:
    ext v1, v0, 0x1e, 0x2
    beqz v1, L_0881FF40
    ext t2, v0, 0x1d, 0x1
    addiu t0, v1, -2
    beqz t0, L_0881FADC
    sh t2, 788(sp)
    bgtz t0, L_0881FF98
    addiu t2, sp, 0
    addiu s4, t7, 108
L_0881FA68:
    lw t3, 0(t7)
    addiu t2, t2, 4
    addiu t7, t7, 4
    bne s4, t7, L_0881FA68
    sw t3, -4(t2)
    addiu t1, zero, 0
    addiu s4, zero, 144
L_0881FA84:
    sh t1, 0(t2)
    addiu t0, t1, 144
    addiu t1, t1, 1
    sh t0, 304(t2)
    bne t1, s4, L_0881FA84
    addiu t2, t2, 2
    addiu s4, t7, 16
L_0881FAA0:
    lw t3, 0(t7)
    addiu t2, t2, 4
    addiu t7, t7, 4
    bne s4, t7, L_0881FAA0
    sw t3, -4(t2)
    addiu s4, t7, 64
L_0881FAB8:
    lw t3, 0(t7)
    addiu t2, t2, 4
    addiu t7, t7, 4
    bne s4, t7, L_0881FAB8
    sw t3, 280(t2)
    addiu t7, t7, -188
    sh t3, 666(sp)
    j L_0881FBCC
    sh t3, 636(sp)
L_0881FADC:
    addiu t9, t9, 14
    blez t9, L_0881FB00
    rotrv v0, t8, t9
    addiu a2, a2, 4
    lw t8, 0(a2)
    sllv v0, v0, t9
    addiu t9, t9, -32
    ins v0, t8, 0x0, 0x10
    rotrv v0, v0, t9
L_0881FB00:
    sw v0, 860(sp)
    ext s4, v0, 0x1c, 0x4
    addiu t3, sp, 760
    addiu t0, t7, -4
    addu s4, s4, t7
L_0881FB14:
    beq t0, s4, L_0881FB5C
    addiu t0, t0, 1
    lb t2, 9(t0)
    addiu t9, t9, 3
    blez t9, L_0881FB44
    rotrv v0, t8, t9
    addiu a2, a2, 4
    lw t8, 0(a2)
    sllv v0, v0, t9
    addiu t9, t9, -32
    ins v0, t8, 0x0, 0x10
    rotrv v0, v0, t9
L_0881FB44:
    srl v0, v0, 0x1d
    beqz v0, L_0881FB14
    ins t2, v0, 0x9, 0x6
    addiu t3, t3, 2
    j L_0881FB14
    sh t2, 58(t3)
L_0881FB5C:
    jal L_0881FEB4
    addiu s4, sp, 760
    lw t5, 860(sp)
    beqz t0, L_0881FF98
    addiu s4, sp, 0
    ext t5, t5, 0x12, 0x5
    jal L_0881FDA8
    addiu t5, t5, 257
    beqz t0, L_0881FF98
    lw t5, 860(sp)
L_0881FB84:
    lbu t2, 61(s4)
    lbu t1, 60(s4)
    addiu s4, s4, 2
    beqz t2, L_0881FB9C
    ins t7, t1, 0x1, 0x5
    lh t1, 60(t7)
L_0881FB9C:
    bne s4, t3, L_0881FB84
    sh t1, 58(s4)
    addiu s4, sp, 636
    jal L_0881FDA4
    ext t5, t5, 0x17, 0x5
L_0881FBB0:
    lh t1, 60(s4)
    addiu s4, s4, 2
    ins t7, t1, 0x1, 0x5
    lh t1, 124(t7)
    bne s4, t3, L_0881FBB0
    sh t1, 58(s4)
    ins t7, zero, 0x1, 0x5
L_0881FBCC:
    lb t4, 0(sp)
L_0881FBD0:
    lhu v1, 30(sp)
    jal L_0881FD48
    addiu t0, sp, 2
    addiu t0, sp, 638
    bgez v0, L_0881FC60
    bitrev t2, v0
    sra s4, v0, 0x4
    bltzal t2, L_0881FC8C
    lhu v1, 666(sp)
    lb t2, 636(sp)
    jal L_0881FD08
    subu at, a0, s4
    sltu s4, a1, at
    bnez s4, L_0881FFA4
    clz t2, v0
    addiu t2, t2, -30
    bltzal t2, L_0881FC70
    subu s4, a0, v0
    sltu t2, t6, s4
    beqz t2, L_0881FF98
    addiu a0, a0, 1
L_0881FC24:
    lbu t0, -1(s4)
    addiu s4, s4, 1
    beq at, a0, L_0881FBD0
    sb t0, -1(a0)
    j L_0881FC24
    addiu a0, a0, 1
L_0881FC3C:
    addiu a2, a2, 4
    lw t8, 0(a2)
    sllv v0, v0, t9
    addiu t9, t9, -32
    ins v0, t8, 0x0, 0x10
    rotrv v0, v0, t9
    srlv v0, v0, t2
    jr ra
    subu s4, s4, v0
L_0881FC60:
    beq a0, a1, L_0881FFA4
    addiu a0, a0, 1
    j L_0881FBD0
    sb v0, -1(a0)
L_0881FC70:
    bltz v0, L_0881FF98
    subu t9, t9, t2
    bgtz t9, L_0881FC3C
    rotrv v0, t8, t9
    srlv v0, v0, t2
    jr ra
    subu s4, s4, v0
L_0881FC8C:
    sra t2, t2, 0x1c
    beq s4, t2, L_0881FCAC
    subu t9, t9, t2
    bgtz t9, L_0881FC3C
    rotrv v0, t8, t9
    srlv v0, v0, t2
    jr ra
    subu s4, s4, v0
L_0881FCAC:
    addiu v0, v0, 1
    beqz v0, L_0881FF98
    lh t0, 788(sp)
    beqz t0, L_0881FA20
    addu t9, t9, t2
L_0881FCC0:
    beqz a3, L_0881FCD8
    subu v0, a0, t6
    addiu t9, t9, 39
    srl t1, t9, 0x3
    addu a2, a2, t1
    sw a2, 0(a3)
L_0881FCD8:
    lw ra, 784(sp)
    lw s4, 776(sp)
    jr ra
    addiu sp, sp, 896
L_0881FCE8:
    addiu a2, a2, 4
    lw t8, 0(a2)
    sllv v0, v0, t9
    addiu t9, t9, -32
    ins v0, t8, 0x0, 0x10
    rotrv v0, v0, t9
    j L_0881FD18
    ins v0, zero, 0x0, 0x10
L_0881FD08:
    srlv v0, t8, t9
    subu t9, t9, t2
    bgtz t9, L_0881FCE8
    sllv v0, v0, t2
L_0881FD18:
    bitrev v0, v0
    sltu t2, v0, v1
    beqz t2, L_0881FD68
    addu v0, v0, v0
L_0881FD28:
    addu v0, v0, t0
    jr ra
    lh v0, 58(v0)
L_0881FD34:
    lhu t2, 0(t0)
    addu v0, v0, t0
    subu v0, v0, t2
    jr ra
    lh v0, 60(v0)
L_0881FD48:
    srlv v0, t8, t9
    subu t9, t9, t4
    bgtz t9, L_0881FCE8
    sllv v0, v0, t4
    bitrev v0, v0
    sltu t2, v0, v1
    bnez t2, L_0881FD28
    addu v0, v0, v0
L_0881FD68:
    beqz t9, L_0881FD90
    lhu v1, 30(t0)
    srlv t2, t8, t9
    ins v0, t2, 0x0, 0x1
    addiu t9, t9, 1
L_0881FD7C:
    sltu v1, v0, v1
    bnez v1, L_0881FD34
    addu v0, v0, v0
    j L_0881FD68
    addiu t0, t0, 2
L_0881FD90:
    addiu a2, a2, 4
    lw t8, 0(a2)
    addiu t9, zero, -31
    j L_0881FD7C
    ins v0, t8, 0x0, 0x1
L_0881FDA4:
    addiu t5, t5, 1
L_0881FDA8:
    sw zero, 56(s4)
    move at, zero
    addiu t1, zero, 7
    sw zero, 44(s4)
    move t3, s4
    mthi ra
    lb t4, 760(sp)
    sw zero, 52(s4)
L_0881FDC8:
    andi t0, at, 0x1ff
L_0881FDCC:
    subu t0, t0, t5
    bgez t0, L_0881FEA8
    lhu v1, 790(sp)
    jal L_0881FD48
    addiu t0, sp, 762
    addiu t0, v0, -16
    blez t0, L_0881FE24
    min t2, t0, t1
    addu t9, t9, t2
    blez t9, L_0881FE10
    rotrv v0, t8, t9
    addiu a2, a2, 4
    lw t8, 0(a2)
    sllv v0, v0, t9
    addiu t9, t9, -32
    ins v0, t8, 0x0, 0x10
    rotrv v0, v0, t9
L_0881FE10:
    negu t2, t2
    srlv v0, v0, t2
    addu t0, t0, v0
L_0881FE1C:
    j L_0881FDC8
    addu at, at, t0
L_0881FE24:
    beqz t0, L_0881FE40
    addiu t2, t3, 2
    ins at, v0, 0x9, 0x6
    sh at, 60(t3)
    addiu at, at, 1
    j L_0881FDC8
    movn t3, t2, v0
L_0881FE40:
    blez at, L_0881FF98
    addiu t9, t9, 2
    blez t9, L_0881FE68
    rotrv v0, t8, t9
    addiu a2, a2, 4
    lw t8, 0(a2)
    sllv v0, v0, t9
    addiu t9, t9, -32
    ins v0, t8, 0x0, 0x10
    rotrv v0, v0, t9
L_0881FE68:
    srl v0, v0, 0x1e
    andi t0, at, 0x1ff
    beq t0, at, L_0881FE1C
    addiu t0, v0, 3
    addu t0, t0, at
L_0881FE7C:
    sh at, 60(t3)
    addiu at, at, 1
    bne at, t0, L_0881FE7C
    addiu t3, t3, 2
    j L_0881FDCC
    andi t0, at, 0x1ff
L_0881FE94:
    srlv v0, at, t0
    bnez v0, L_0881FF98
    addiu at, zero, -1
    jr ra
    sh at, 28(t1)
L_0881FEA8:
    sw zero, 48(s4)
    bnez t0, L_0881FF98
    mfhi ra
L_0881FEB4:
    sw zero, 32(s4)
    move t0, zero
    move t5, s4
    sw zero, 36(s4)
    addiu t1, s4, 2
    addiu at, zero, -1
    sw zero, 40(s4)
L_0881FED0:
    beq t3, t5, L_0881FE94
    addiu t4, t5, 2
    addiu at, at, 1
    lh v0, 60(t5)
L_0881FEE0:
    lh v1, 60(t4)
    beq t3, t4, L_0881FEFC
    addiu t4, t4, 2
    max t2, v1, v0
    sh t2, 58(t4)
    j L_0881FEE0
    min v0, v1, v0
L_0881FEFC:
    srl t4, v0, 0x9
    andi v0, v0, 0x1ff
    sh v0, 60(t5)
    beq t0, t4, L_0881FED0
    addiu t5, t5, 2
    subu t2, t0, t4
    beqz t0, L_0881FF38
    move t0, t4
    sh at, 28(t1)
    rotrv at, at, t2
    addu t2, t2, t2
    subu t1, t1, t2
    subu t2, t5, at
    subu t2, t2, at
    subu t2, t1, t2
L_0881FF38:
    j L_0881FED0
    sh t2, -2(t1)
L_0881FF40:
    negu t0, t9
    srl t0, t0, 0x3
    subu a2, a2, t0
    lwl t0, 7(a2)
    lwr t0, 4(a2)
    andi v1, t0, 0xffff
    addu v1, v1, a0
    sltu t1, a1, v1
    bnez t1, L_0881FFA4
    nor t1, t0, zero
    rotr t1, t1, 16
    bne t1, t0, L_0881FF98
    nop
L_0881FF74:
    lbu t1, 8(a2)
    addiu a2, a2, 1
    addiu a0, a0, 1
    bne v1, a0, L_0881FF74
    sb t1, -1(a0)
    bnez t2, L_0881FCC0
    addiu t9, zero, 32
    j L_0881FA0C
    addiu a2, a2, 8
L_0881FF98:
    lui v0, 0x8000
    j L_0881FCD8
    ori v0, v0, 0x108
L_0881FFA4:
    lui v0, 0x8000
    j L_0881FCD8
    ori v0, v0, 0x104
}
#else
int deflate_decompress(void *dst, unsigned int dst_len, const void *src, const void **next)
{
    Inflate s;
    Huffman lit;
    Huffman dist;
    int last;
    int r;

    s.in = src;
    s.bitbuf = 0;
    s.bitcnt = 0;
    s.base = s.out = dst;
    s.end = s.base + dst_len;
    do {
        last = (int)inf_bits(&s, 1);
        switch (inf_bits(&s, 2)) {
        case 0: {
            unsigned int len;

            s.bitbuf = 0;
            s.bitcnt = 0;
            len = s.in[0] | s.in[1] << 8;
            if ((unsigned int)(s.in[2] | s.in[3] << 8) != (~len & 0xFFFF)) {
                return (int)DEFLT_ERR_DATA;
            }
            s.in += 4;
            if ((unsigned int)(s.end - s.out) < len) {
                return (int)DEFLT_ERR_OVERFLOW;
            }
            while (len-- > 0) {
                *s.out++ = *s.in++;
            }
            r = 0;
            break;
        }
        case 1:
            r = inf_fixed(&s, &lit, &dist);
            break;
        case 2:
            r = inf_dynamic(&s, &lit, &dist);
            break;
        default:
            r = (int)DEFLT_ERR_DATA;
            break;
        }
        if (r < 0) {
            return r;
        }
    } while (!last);
    if (next != 0) {
        *next = s.in - s.bitcnt / 8;
    }
    return (int)(s.out - s.base);
}
#endif
