// @range 0x08833420-0x0883451C
// @asm 0x088338C8-0x08833A30
// @data 0x088ED9A8-0x088ED9CC
/* EHP packs: name lookup (binary search over the sorted entry table), relocation after loading
   (ptrs.txt pointer lists, TGMA/TGMT models, TMS sets, EHF animations with their GIM textures,
   GIM images, and handlers registered by extension), the pending load callback, the table of
   mounted pack directories, plus the 2D projection push/pop. */
#include "psp.h"
#include "libc.h"
#include "ehfile.h"

typedef struct EhpEntry {              /* 8 bytes, after the 0x10-byte header */
    int name;                          /* offset of the name */
    int data;                          /* offset of the data */
} EhpEntry;

typedef struct EhpPack {
    int magic;                         /* "EHP" + version in the high byte */
    unsigned int size;                 /* +0x04 */
    unsigned int bind;                 /* +0x08 "BIND" once relocated */
    int count;                         /* +0x0C */
    EhpEntry e[1];                     /* +0x10 zero-terminated */
} EhpPack;

typedef void (*EhpHandler)(int load, void *data, int size, EhpPack *pack);

typedef struct EhpState {              /* 0x089048A4 */
    void (*pending)(EhpPack *pack);    /* called once the pack being loaded is relocated */
    int n;                             /* +0x04 */
    unsigned int ext[8];               /* +0x08 extension as a little-endian word (".abc") */
    EhpHandler handler[8];             /* +0x28 */
    EhpPack *cur;                      /* +0x48 pack whose TMS set is being relocated */
} EhpState;

typedef struct EhpMount {              /* 0x104 */
    EhpPack *pack;
    char path[0x100];
} EhpMount;


typedef struct EhfTexName {            /* texture entry before the GIM is bound: its name */
    int unk0;
    void *gim;
    int size;
} EhfTexName;

#define EHP_VERSION(p) ((p)->magic >> 24 & 0xFF)
#define EXT4(p) ((p)[3] * 0x1000000 + ((p)[0] + (p)[1] * 0x100 + (p)[2] * 0x10000))

extern EhpState ehp_state;             /* 0x089048A4 */
extern int ehp_mount_n;                /* 0x08903860 */
extern EhpMount ehp_mount_tab[16];     /* 0x08903864 */
extern int gfx_saved_mtx_mode;         /* 0x0890385C */

int mtx_get_mode(void);
void mtx_set_mode(int mode);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_ortho(float l, float r, float b, float t, float n, float f);
void *mtx_commit(void *list);
int ehfile_read_file_alias(int dir, const char *name, void *buf, int size);
int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
int ehfile_read_async(int dir, const char *name, void *buf, int offset, int size, int (*cb)(void *, int), void *arg);
void mem_move(void *dst, void *src, unsigned int n);
char *strrchr_str(char *s, int c);
void tgma_relocate(void *p);
void tgm_relocate(void *p);
void ehsys_0882B650(void *(*fn)(char *name));
void ehsys_0882B65C(void *p);
void gim_reset(void *gim);
int ehf_tex_foreach(void *f, unsigned char *flags, int (*cb)(EhpPack *pack, EhfTexName *t), void *arg);
void *ehf_inst_init(float w, float h, float cx, float cy, void *f, void *e);

int ehp_is_valid(EhpPack *p);
EhpEntry *ehp_find_entry(EhpPack *p, const char *name);
int ehp_entry_size(EhpPack *p, EhpEntry *e);
int ehp_loaded(EhpPack *p, int result);
int ehp_relocate(EhpPack *p);
char *str_ext(char *name);
int ehp_tex_bind(EhpPack *p, EhfTexName *t);
void *ehp_tms_find(char *name);

/* ORs one byte of `flags` into the flags of each texture entry of an EHF file. */
int ehf_tex_or_flags(char *f, unsigned char *flags)
{
    int *h;
    char *t;
    char *end;
    int i;

    i = 0;
    h = (int *)(f + 0x10);
    if (h[8] == 0) {
        return 1;
    }
    t = f + h[8];
    end = t + h[9];
    for (; t < end; t += 0x10) {
        *(unsigned int *)(t + 0xC) |= flags[i];
        i++;
    }
    return 1;
}

/* Projection for 2D drawing: ortho 480x272, previous matrix mode saved. */
void gfx_begin_2d(void *list)
{
    gfx_saved_mtx_mode = mtx_get_mode();
    mtx_set_mode(0);
    mtx_push();
    mtx_identity();
    mtx_ortho(0.0f, 480.0f, 272.0f, 0.0f, -1000.0f, 1000.0f);
    mtx_commit(list);
}

void *gfx_end_2d(void *list)
{
    mtx_set_mode(0);
    mtx_pop();
    list = mtx_commit(list);
    if (gfx_saved_mtx_mode != -1) {
        mtx_set_mode(gfx_saved_mtx_mode);
        gfx_saved_mtx_mode = -1;
    }
    return list;
}

/* Registers a handler for entries with extension `ext` (".abc"); 0 if already registered. */
int ehp_add_handler(unsigned char *ext, EhpHandler fn)
{
    EhpState *st = &ehp_state;
    unsigned int key = EXT4(ext);
    int i;

    for (i = 0; i < st->n; i++) {
        if (st->ext[i] == key) {
            return 0;
        }
    }
    st->ext[st->n] = key;
    st->handler[st->n] = fn;
    st->n++;
    return 1;
}

/* Loads `dir`.ehp into `buf` (a trailing slash is replaced by the extension). Synchronous when
   `cb` is 0, otherwise asynchronous with `cb` called once relocated. */
int ehp_load(int dev, const char *dir, void (*cb)(EhpPack *p), void *buf, int size)
{
    char path[0x80];

    if (ehp_state.pending != 0) {
        return 0;
    }
    if (buf == 0) {
        return 0;
    }
    strcpy(path, dir);
    if (path[strlen(path) - 1] == '/' || path[strlen(path) - 1] == '\\') {
        strcpy(path + strlen(path) - 1, ".ehp");
    } else {
        strcat(path, ".ehp");
    }
    if (cb != 0) {
        ehp_state.pending = cb;
        ehfile_read_async_alias(dev, path, buf, size, (int (*)(void *, int))ehp_loaded, buf);
        return 1;
    }
    do {
    } while (ehfile_read_file_alias(dev, path, buf, size) <= 0);
    ehp_loaded(buf, 1);
    return 1;
}

int ehp_mount(void (*cb)(EhpPack *p), EhpPack *pack)
{
    if (ehp_state.pending != 0) {
        return 0;
    }
    ehp_state.pending = cb;
    ehp_loaded(pack, 1);
    return 1;
}

#ifdef NON_MATCHING /* the loop entry schedules the entry pointer before the branch instead of in its delay slot */
/* Calls the handlers of the pack's entries with load 0, then marks the pack invalid. */
int ehp_unload(EhpPack *p)
{
    EhpState *st = &ehp_state;
    EhpEntry *e;
    unsigned int *size;
    unsigned char *ext;
    unsigned int key;
    int data;
    char *ptr;
    int i;
    int len;

    e = p->e;
    size = &p->size;
    while (e->name != 0) {
        ext = (unsigned char *)str_ext((char *)p + *(int *)e);
        if (ext != 0) {
            key = EXT4(ext);
            if (key != 0x6D69672E && key != 0x6668652E && key != 0x736D742E && key != 0x746D742E && key != 0x616D742E) {
                data = e->data;
                ptr = (char *)p + data;
                for (i = 0; i < st->n; i++) {
                    if (key == st->ext[i]) {
                        if (st->handler[i] != 0) {
                            if (e[1].name == 0) {
                                len = *size - data;
                            } else {
                                len = e[1].data - data;
                            }
                            st->handler[i](0, ptr, len, p);
                        }
                        break;
                    }
                }
            }
        }
        e++;
    }
    *(unsigned int *)p = 0;
    return 1;
}
#endif

/* Load completion: relocates the pack once (version 2+ packs are marked BIND) and calls the
   pending callback. */
int ehp_loaded(EhpPack *p, int result)
{
    void (*cb)(EhpPack *p);
    int reloc;
    unsigned int *bind;

    if (result < 0) {
        return 0;
    }
    reloc = 1;
    if (EHP_VERSION(p) >= 2) {
        bind = &p->bind;
        if (*bind != 0x444E4942) {
            *bind = 0x444E4942;
        } else {
            reloc = 0;
        }
    }
    if (reloc) {
        ehp_relocate(p);
    }
    cb = ehp_state.pending;
    ehp_state.pending = 0;
    if (cb != 0) {
        cb(p);
    }
    return 1;
}

/* Binary search of the entry table, by name (case-insensitive) or by data pointer. */
EhpEntry *ehp_search(EhpPack *p, const void *key, int by_name)
{
    EhpEntry *tbl = (EhpEntry *)&p->bind;
    int lo;
    int hi;
    int mid;
    int c;
    EhpEntry *e;

    hi = p->count + 1;
    lo = 0;
    mid = hi / 2;
    if (mid > 0) {
        do {
            e = &tbl[mid];
            if (by_name) {
                c = stricmp((char *)p + e->name, key);
            } else {
                c = (char *)p + e->data - (char *)key;
            }
            if (c == 0) {
                return e;
            }
            if (c < 0) {
                lo = mid;
            } else {
                hi = mid;
            }
            mid = (lo + hi) / 2;
        } while (lo < mid);
    }
    return 0;
}

EhpEntry *ehp_find_entry(EhpPack *p, const char *name)
{
    return ehp_search(p, name, 1);
}

EhpEntry *ehp_find_data(EhpPack *p, void *data)
{
    return ehp_search(p, data, 0);
}

void *ehp_find(EhpPack *p, const char *name, int *size)
{
    EhpEntry *e;

    if (p == 0 || ehp_is_valid(p) == 0) {
        return 0;
    }
    if ((e = ehp_find_entry(p, name)) != 0) {
        if (size != 0) {
            *size = ehp_entry_size(p, e);
        }
        return (char *)p + e->data;
    }
    return 0;
}

char *ehp_name_of(EhpPack *p, void *data)
{
    EhpEntry *e = ehp_find_data(p, data);

    if (e != 0) {
        return (char *)p + e->name;
    }
    return 0;
}

int ehp_is_valid(EhpPack *p)
{
    return (*(unsigned int *)p & 0xFFFFFF) == 0x504845;
}

/* Relocates the entries of a freshly loaded pack by extension. */
int ehp_relocate(EhpPack *p)
{
    char buf[0x100];
    EhpState *st = &ehp_state;
    int len = strlen("ptrs.txt");
    EhpEntry *e = p->e;
    int *data;
    char *name;
    unsigned char *ext;
    unsigned int key;
    int i;

    while (e->name != 0) {
        name = (char *)p + e->name;
        data = (int *)((char *)p + e->data);
        if (stricmp(name + strlen(name) - len, "ptrs.txt") == 0) {
            while (*data != -1) {
                if (*data != 0) {
                    *data += (int)p;
                }
                data++;
            }
        } else {
            ext = (unsigned char *)str_ext(name);
            if (ext != 0) {
                key = EXT4(ext);
                switch (key) {
                case 0x6D69672E:
                    gim_reset(data);
                    break;
                case 0x6668652E:
                    strcpy(buf, name);
                    strcpy(buf + strlen(name) - 4, ".tmf");
                    ehf_tex_foreach(data, ehp_find(p, buf, 0), ehp_tex_bind, p);
                    ehf_inst_init(480.0f, 272.0f, 240.0f, 136.0f, data, 0);
                    break;
                case 0x736D742E:
                    st->cur = p;
                    ehsys_0882B650(ehp_tms_find);
                    ehsys_0882B65C(data);
                    break;
                case 0x746D742E:
                    tgm_relocate(data);
                    break;
                case 0x616D742E:
                    tgma_relocate(data);
                    break;
                default:
                    for (i = 0; i < st->n; i++) {
                        if (key == st->ext[i]) {
                            st->handler[i](1, data, ehp_entry_size(p, e), p);
                            break;
                        }
                    }
                    break;
                }
            }
        }
        e++;
    }
    return 1;
}

int ehp_entry_size(EhpPack *p, EhpEntry *e)
{
    unsigned char *s;

    if (EHP_VERSION(p) >= 3) {
        s = (unsigned char *)p + e->name;
        s += strlen((char *)s) + 1;
        return EXT4(s);
    }
    if (e[1].name == 0) {
        return p->size - e->data;
    }
    return e[1].data - e->data;
}

int ehp_file_size(EhpPack *p, const char *name)
{
    EhpEntry *e = ehp_find_entry(p, name);

    if (e != 0) {
        return ehp_entry_size(p, e);
    }
    return -1;
}

int ehp_get_size_cur(EhpPack *p, void *data)
{
    EhpEntry *e = ehp_find_data(p, data);

    if (e != 0) {
        return ehp_entry_size(p, e);
    }
    return -1;
}

void *ehp_get_name_by_index(EhpPack *p, int index, char *out)
{
    EhpEntry *e = p->e;
    int i;

    for (i = 0; e->name != 0 && i < index; i++, e++) {
    }
    if (e->name == 0) {
        return 0;
    }
    strcpy(out, (char *)p + e->name);
    return (char *)p + e->data;
}

/* Last '.' of `name` (not the first character), or 0. */
char *str_ext(char *name)
{
    char *s = name + strlen(name) - 1;

    while (s > name && *s != '.') {
        s--;
    }
    if (s == name) {
        return 0;
    }
    return s;
}

/* ehf_tex_foreach callback: binds "<name>.gim" of the pack to a texture entry. */
int ehp_tex_bind(EhpPack *p, EhfTexName *t)
{
    char buf[0x100];
    int size;
    void *gim;

    strcpy(buf, (char *)t);
    strcat(buf, ".gim");
    gim = ehp_find(p, buf, &size);
    t->unk0 = -1;
    t->gim = gim;
    t->size = size;
    return 1;
}

void ehp_mounts_clear(void)
{
    memset(ehp_mount_tab, 0, sizeof(ehp_mount_tab));
    ehp_mount_n = 0;
}

int ehp_mounts_add(const char *dir, EhpPack *pack)
{
    char path[0x3C];

    if (ehp_mount_n >= 16) {
        return 0;
    }
    strcpy(path, dir);
    if (path[strlen(path) - 1] == '/' || path[strlen(path) - 1] == '\\') {
        strcpy(path + strlen(path) - 1, ".ehp");
    } else {
        strcat(path, ".ehp");
    }
    strcpy(ehp_mount_tab[ehp_mount_n].path, path);
    ehp_mount_tab[ehp_mount_n].pack = pack;
    ehp_mount_n++;
    return 1;
}

void ehp_mounts_remove(EhpPack *pack)
{
    int i;
    int n;

    for (i = 0; i < ehp_mount_n; i++) {
        if (ehp_mount_tab[i].pack == pack) {
            break;
        }
    }
    n = ehp_mount_n - 1 - i;
    if (n > 0) {
        mem_move(&ehp_mount_tab[i], &ehp_mount_tab[i + 1], (unsigned int)(n * sizeof(EhpMount)) >> 2);
    }
    ehp_mount_n--;
}

/* Reads one entry of a mounted pack from disc into `buf` (asynchronously when `cb` is set). */
int ehp_read_entry(int dev, EhpPack *pack, const void *key, int by_name, void *buf, void *arg, int (*cb)(void *, int))
{
    int ret = 0;
    int i;
    EhpEntry *e;
    int size;
    EhpMount *m;

    for (i = 0, m = ehp_mount_tab; i < ehp_mount_n; i++) {
        if (m->pack == pack) {
            break;
        }
        m++;
    }
    if ((e = ehp_search(pack, key, by_name)) != 0) {
        size = ehp_entry_size(pack, e);
        if (cb == 0) {
            while (ehfile_read_file(dev, m->path, buf, e->data, size) <= 0) {
            }
            ret = 1;
        } else {
            ehfile_read_async(dev, m->path, buf, e->data, size, cb, arg);
            ret = 1;
        }
    }
    return ret;
}

/* TMS callback: the texture "<name>.gim" (name up to 0x20 chars) of the pack being relocated. */
void *ehp_tms_find(char *name)
{
    EhpState *st = &ehp_state;
    char buf[0x20];
    char *dot;

    dot = strrchr_str(name, '.');
    if ((unsigned int)(dot + 5 - name) >= 0x20) {
        return 0;
    }
    strcpy(buf, name);
    dot = strrchr_str(buf, '.');
    if (dot != 0) {
        strcpy(dot, ".gim");
    }
    return ehp_find(st->cur, buf, 0);
}
