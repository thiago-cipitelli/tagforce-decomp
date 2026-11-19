/* Texture replacement mods (optional): texture packs in PPSSPP's format (textures.ini + PNGs), so the packs made for
   the emulator work unchanged on the port.

   Mods live in $TAGFORCE_MODS (default ./mods), one folder per mod with its textures.ini. mods/mods.txt, if present,
   lists the enabled mods one per line (first line = highest priority; '#' comments); without it every folder is
   enabled, in alphabetical order. A folder that is not listed (or a missing mods/) changes nothing.

   The name of a replacement is PPSSPP's (GPU/Common/TextureReplacer.cpp, TextureCacheCommon.cpp):
   %016llx%08x = cache key (texture address << 32 | dimension, xor the CLUT hash for the CLUT formats; the address
   is 0 with ignoreAddress = true) and the hash of the texture data (xxh32/xxh64 with seed 0xBACD7814, the low 32
   bits). The CLUT hash is XXH32(seed 0xC0108888) of the CLUT bytes: the last load plus the start offset, at most the
   largest load so far, xor the whole CLUTFORMAT command word (0xC5 << 24 | data). A 512-row texture hashes only the rows seen by the through-mode draw (maxSeenV, at least
   272). hash = quick (PPSSPP's SSE hash) is not supported.

   The game defines malloc/free, snprintf, sscanf, strtoul and fread itself (they interpose glibc's in the
   executable), so this file allocates with __libc_malloc and parses by hand. */
#define _GNU_SOURCE
#include <dirent.h>
#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

void *__libc_malloc(size_t);
void *__libc_realloc(void *, size_t);
void __libc_free(void *);

typedef uint32_t u32;
typedef uint64_t u64;
typedef uint8_t u8;

/* --- xxHash (XXH32, XXH64; reference algorithm, little endian) --- */

#define P32_1 2654435761u
#define P32_2 2246822519u
#define P32_3 3266489917u
#define P32_4 668265263u
#define P32_5 374761393u
#define P64_1 11400714785074694791ull
#define P64_2 14029467366897019727ull
#define P64_3 1609587929392839161ull
#define P64_4 9650029242287828579ull
#define P64_5 2870177450012600261ull

static u32 rl32(u32 x, int r) { return x << r | x >> (32 - r); }
static u64 rl64(u64 x, int r) { return x << r | x >> (64 - r); }
static u32 rd32(const u8 *p) { u32 v; memcpy(&v, p, 4); return v; }
static u64 rd64(const u8 *p) { u64 v; memcpy(&v, p, 8); return v; }

static u32 xxh32(const u8 *p, u32 n, u32 seed)
{
    const u8 *e = p + n;
    u32 h;

    if (n >= 16) {
        u32 v1 = seed + P32_1 + P32_2, v2 = seed + P32_2, v3 = seed, v4 = seed - P32_1;
        do {
            v1 = rl32(v1 + rd32(p) * P32_2, 13) * P32_1;
            v2 = rl32(v2 + rd32(p + 4) * P32_2, 13) * P32_1;
            v3 = rl32(v3 + rd32(p + 8) * P32_2, 13) * P32_1;
            v4 = rl32(v4 + rd32(p + 12) * P32_2, 13) * P32_1;
            p += 16;
        } while (p + 16 <= e);
        h = rl32(v1, 1) + rl32(v2, 7) + rl32(v3, 12) + rl32(v4, 18);
    } else {
        h = seed + P32_5;
    }
    h += n;
    for (; p + 4 <= e; p += 4)
        h = rl32(h + rd32(p) * P32_3, 17) * P32_4;
    for (; p < e; p++)
        h = rl32(h + *p * P32_5, 11) * P32_1;
    h ^= h >> 15; h *= P32_2;
    h ^= h >> 13; h *= P32_3;
    return h ^ h >> 16;
}

static u64 r64(u64 acc, u64 in) { return rl64(acc + in * P64_2, 31) * P64_1; }
static u64 m64(u64 h, u64 v) { return (h ^ r64(0, v)) * P64_1 + P64_4; }

static u64 xxh64(const u8 *p, u32 n, u64 seed)
{
    const u8 *e = p + n;
    u64 h;

    if (n >= 32) {
        u64 v1 = seed + P64_1 + P64_2, v2 = seed + P64_2, v3 = seed, v4 = seed - P64_1;
        do {
            v1 = r64(v1, rd64(p)); v2 = r64(v2, rd64(p + 8));
            v3 = r64(v3, rd64(p + 16)); v4 = r64(v4, rd64(p + 24));
            p += 32;
        } while (p + 32 <= e);
        h = rl64(v1, 1) + rl64(v2, 7) + rl64(v3, 12) + rl64(v4, 18);
        h = m64(h, v1); h = m64(h, v2); h = m64(h, v3); h = m64(h, v4);
    } else {
        h = seed + P64_5;
    }
    h += n;
    for (; p + 8 <= e; p += 8)
        h = rl64(h ^ r64(0, rd64(p)), 27) * P64_1 + P64_4;
    if (p + 4 <= e) {
        h = rl64(h ^ (u64)rd32(p) * P64_1, 23) * P64_2 + P64_3;
        p += 4;
    }
    for (; p < e; p++)
        h = rl64(h ^ *p * P64_5, 11) * P64_1;
    h ^= h >> 33; h *= P64_2;
    h ^= h >> 29; h *= P64_3;
    return h ^ h >> 32;
}

/* --- mods and their textures.ini --- */

enum { H_XXH32, H_XXH64 };
typedef struct { int hash, ignore_addr, skip128x64, skip128x128; } Mod;
typedef struct { u64 key; u32 hash; int mod; char *path; unsigned gl; int w, h, failed; } Rep;

static Mod mods[32];
static int nmods = -1;
static Rep *reps;
static int nreps, capreps;
static int dbg;

static char *dupstr(const char *s)
{
    size_t n = strlen(s) + 1;
    char *d = __libc_malloc(n);
    memcpy(d, s, n);
    return d;
}

static int hexval(const char *s, int n, u64 *out)
{
    u64 v = 0;
    int i;

    for (i = 0; i < n; i++) {
        int c = s[i], d = c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10
                        : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
        if (d < 0)
            return 0;
        v = v << 4 | d;
    }
    *out = v;
    return 1;
}

static char *trim(char *s)
{
    char *e;

    while (*s == ' ' || *s == '\t')
        s++;
    e = s + strlen(s);
    while (e > s && (e[-1] == ' ' || e[-1] == '\t' || e[-1] == '\r' || e[-1] == '\n'))
        *--e = 0;
    return s;
}

/* `a b c` joined into a fresh string */
static char *join3(const char *a, const char *b, const char *c)
{
    size_t la = strlen(a), lb = strlen(b), lc = strlen(c);
    char *d = __libc_malloc(la + lb + lc + 1);
    memcpy(d, a, la); memcpy(d + la, b, lb); memcpy(d + la + lb, c, lc + 1);
    return d;
}

static void add_rep(u64 key, u32 hash, int mod, char *path)
{
    if (nreps == capreps) {
        capreps = capreps ? capreps * 2 : 1024;
        reps = __libc_realloc(reps, capreps * sizeof *reps);
    }
    memset(&reps[nreps], 0, sizeof *reps);
    reps[nreps].key = key;
    reps[nreps].hash = hash;
    reps[nreps].mod = mod;
    reps[nreps].path = path;
    nreps++;
}

static void load_mod(const char *root, const char *name)
{
    char *dir = join3(root, "/", name), *ini = join3(dir, "/textures.ini", ""), line[1024];
    FILE *f = fopen(ini, "r");
    Mod *m = &mods[nmods];
    int sect = 0, n0 = nreps;

    if (!f || nmods == 32) {
        if (f)
            fclose(f);
        __libc_free(ini);
        __libc_free(dir);
        return;
    }
    m->hash = H_XXH64;
    m->ignore_addr = 1;
    m->skip128x64 = m->skip128x128 = 0;
    while (fgets(line, sizeof line, f)) {
        char *s = trim(line), *eq, *k, *v;
        u64 key, hash;
        if (*s == '[') {
            sect = !strncasecmp(s, "[options]", 9) ? 1 : !strncasecmp(s, "[hashes]", 8) ? 2 : 0;
            continue;
        }
        if (!*s || *s == '#' || *s == ';' || !(eq = strchr(s, '=')))
            continue;
        *eq = 0;
        k = trim(s);
        v = trim(eq + 1);
        if (sect == 1) {
            if (!strcasecmp(k, "hash"))
                m->hash = !strcasecmp(v, "xxh32") ? H_XXH32 : !strcasecmp(v, "xxh64") ? H_XXH64 : -1;
            else if (!strcasecmp(k, "ignoreAddress"))
                m->ignore_addr = !strcasecmp(v, "true") || !strcmp(v, "1");
            else if (!strcasecmp(k, "skipLastDXT1Blocks128x64"))
                m->skip128x64 = atoi(v);
            else if (!strcasecmp(k, "skipLastDXT1Blocks128x128"))
                m->skip128x128 = atoi(v);
        } else if (sect == 2 && strlen(k) == 24 && *v && hexval(k, 16, &key) && hexval(k + 16, 8, &hash)) {
            add_rep(key, (u32)hash, nmods, join3(dir, "/", v));
        }
    }
    fclose(f);
    if (m->hash < 0) {
        fprintf(stderr, "texmod: %s: hash = quick is not supported, mod skipped\n", name);
        while (nreps > n0)
            __libc_free(reps[--nreps].path);
    } else {
        fprintf(stderr, "texmod: mod %s, %d textures\n", name, nreps - n0);
        nmods++;
    }
    __libc_free(ini);
    __libc_free(dir);
}

static void init(void)
{
    const char *root = getenv("TAGFORCE_MODS");
    char *list, line[512], *names[256];
    FILE *f;
    int n = 0, i, j;

    nmods = 0;
    dbg = getenv("TAGFORCE_TEXMOD_LOG") != 0;
    if (!root || !*root)
        root = "mods";
    list = join3(root, "/mods.txt", "");
    if ((f = fopen(list, "r"))) {
        while (n < 256 && fgets(line, sizeof line, f)) {
            char *s = trim(line);
            if (*s && *s != '#')
                names[n++] = dupstr(s);
        }
        fclose(f);
    } else {
        DIR *d = opendir(root);
        struct dirent *e;
        while (d && n < 256 && (e = readdir(d)))
            if (e->d_name[0] != '.')
                names[n++] = dupstr(e->d_name);
        if (d)
            closedir(d);
        for (i = 1; i < n; i++)   /* alphabetical */
            for (j = i; j > 0 && strcmp(names[j - 1], names[j]) > 0; j--) {
                char *t = names[j]; names[j] = names[j - 1]; names[j - 1] = t;
            }
    }
    for (i = 0; i < n; i++) {
        load_mod(root, names[i]);
        __libc_free(names[i]);
    }
    __libc_free(list);
}

/* --- lookup --- */

static const u8 bpp[11] = { 16, 16, 16, 32, 4, 8, 16, 32, 4, 8, 8 };
static const u32 alignmask[11] = { 0x7F8, 0x7F8, 0x7F8, 0x7FC, 0x7E0, 0x7F0, 0x7F8, 0x7FC, 0x7FF, 0x7FF, 0x7FF };

/* PPSSPP's TextureReplacer::ComputeHash */
static u32 data_hash(const Mod *m, const u8 *p, u32 fmt, u32 w, u32 h, u32 bufw)
{
    if (bufw <= w) {
        u32 size = bpp[fmt] * (bufw * h + (w - bufw)) / 8;
        if (fmt == 8 && w == 128 && h == 64 && m->skip128x64 && 8u * m->skip128x64 < size)
            size -= 8 * m->skip128x64;
        if (fmt == 8 && w == 128 && h == 128 && m->skip128x128 && 8u * m->skip128x128 < size)
            size -= 8 * m->skip128x128;
        return m->hash == H_XXH32 ? xxh32(p, size, 0xBACD7814) : (u32)xxh64(p, size, 0xBACD7814);
    } else {
        u32 line = bpp[fmt] * w / 8, stride = bpp[fmt] * bufw / 8, r = 0, y;
        for (y = 0; y < h; y++, p += stride)
            r = r * 11 ^ (m->hash == H_XXH32 ? xxh32(p, line, 0xBACD7814) : (u32)xxh64(p, line, 0xBACD7814));
        return r;
    }
}

u32 texmod_clut_hash(const u8 *clut, u32 n)
{
    return xxh32(clut, n, 0xC0108888);
}

/* The replacement of the texture (0: none). addr = PSP address, p its bytes, fmt/w/h/bufw_reg the TEX* registers,
   low = dimension (xor the CLUT hash), maxv = largest through-mode v of the draw (0: unknown). Fills the GL
   texture of the replacement through upload() the first time. */
unsigned texmod_find(u32 addr, const u8 *p, u32 fmt, u32 w, u32 h, u32 bufw_reg, u32 low, u32 maxv,
                     unsigned (*upload)(const void *rgba, int w, int h))
{
    u32 hs[32], bufw;
    int i, k, have[32] = { 0 };

    if (nmods < 0)
        init();
    if (!nreps || fmt > 10)
        return 0;
    bufw = bufw_reg & alignmask[fmt];
    if (!bufw && fmt <= 10)
        bufw = 128 / bpp[fmt];
    if (h == 512 && maxv && maxv < 512)
        h = maxv < 272 ? 272 : maxv;
    for (i = 0; i < nreps; i++) {   /* ponytail: linear scan per decoded texture; a hash table if packs grow */
        Rep *r = &reps[i];
        const Mod *m = &mods[r->mod];
        u64 key = (m->ignore_addr ? 0 : (u64)(addr & 0x3FFFFFFF) << 32) | low;
        if (r->key != key)
            continue;
        k = r->mod;
        if (!have[k]) {
            hs[k] = data_hash(m, p, fmt, w, h, bufw);
            have[k] = 1;
        }
        if (r->hash != hs[k])
            continue;
        /* first match = highest priority mod (reps are in mod order) */
        if (!r->gl && !r->failed) {
            png_image img;
            void *buf;
            memset(&img, 0, sizeof img);
            img.version = PNG_IMAGE_VERSION;
            if (png_image_begin_read_from_file(&img, r->path)) {
                img.format = PNG_FORMAT_RGBA;
                buf = __libc_malloc(PNG_IMAGE_SIZE(img));
                if (buf && png_image_finish_read(&img, 0, buf, 0, 0)) {
                    r->w = img.width;
                    r->h = img.height;
                    r->gl = upload(buf, r->w, r->h);
                }
                __libc_free(buf);
            }
            png_image_free(&img);
            if (!r->gl) {
                r->failed = 1;
                fprintf(stderr, "texmod: cannot read %s\n", r->path);
            } else if (dbg) {
                fprintf(stderr, "texmod: %016llx%08x -> %s (%dx%d)\n", (unsigned long long)key, r->hash, r->path,
                        r->w, r->h);
            }
        }
        return r->gl;
    }
    if (dbg)   /* the name an xxh64 / ignoreAddress pack would use */
        fprintf(stderr, "texmod: miss %016llx%08x fmt %u %ux%u bufw %u\n", (unsigned long long)low,
                data_hash(&(Mod){ H_XXH64, 1, 0, 0 }, p, fmt, w, h, bufw), fmt, w, h, bufw);
    return 0;
}
