// @range 0x0882ECAC-0x0882FC3C
// @data 0x088ED900-0x088ED954
/* ehfile: worker thread serving a 16-slot ring of read/load requests, path building from the
   per-context directory table, and the UMD LBN table (ISO9660 directory walk of umd1:) that
   turns "disc0:/..." paths into "disc0:/sce_lbn0x%04x_size0x%04x" for direct sector reads. */
#include "psp.h"
#include "libc.h"
#include "ehfile.h"
#include "allegrex.h"

#define SECTOR 0x800

/* ISO9660 directory record (unaligned fields -> packed) */
#pragma pack(1)
typedef struct IsoDirRec {
    unsigned char len;
    unsigned char ext_attr_len;
    unsigned int extent;
    unsigned int extent_be;
    unsigned int size;
    unsigned int size_be;
    unsigned char date[7];
    unsigned char flags;
    unsigned char unit_size;
    unsigned char gap_size;
    unsigned short volseq, volseq_be;
    unsigned char name_len;
    char name[1];
} IsoDirRec;
#pragma pack()

int gzip_decompress(void *dst, int dst_len, const void *src, int *out_size);
int is_gzip(const void *p);

int ehfile_term(void);
int ehfile_read_async(int dir, const char *name, void *buf, int offset, int size, EhfileCallback cb, void *arg);
int ehfile_load_module(int dir, const char *name);
int ehfile_thread(SceSize args, void *argp);
void ehfile_scan_iso_dir(int fd, const char *prefix, int lbn, int size);
int ehfile_lbn_cmp(const void *a, const void *b);
static int ehfile_read_disc(const char *path, void *buf, int offset, int size);
static int ehfile_open_retry(const char *path, int flags, int mode);
static int ehfile_read_retry(int fd, void *buf, int size);
static SceOff ehfile_lseek(int fd, SceOff offset, int whence);
static int ehfile_getstat_retry(const char *path, SceIoStat *st);
static void ehfile_close(int fd);

int ehfile_init(const char ***dirs)
{
    memset(&ehfile_state, 0, sizeof ehfile_state);
    ehfile_state.dirs = *dirs;
    ehfile_state.thread = sceKernelCreateThread("", ehfile_thread, 0x6f, 0x2000, 0, 0);
    sceKernelStartThread(ehfile_state.thread, 0, 0);
    ehfile_state.sema = sceKernelCreateSema("ehfile_sema", 0, 1, 1, 0);
    ehfile_state.queue_sema = sceKernelCreateSema("ehfile_queue", 0, 0, 16, 0);
    return 1;
}

int ehfile_term(void)
{
    EhfileState *const s = &ehfile_state; /* `const` keeps the base in a register (matches) */

    if (s->queue_sema > 0)
        sceKernelDeleteSema(s->queue_sema);
    if (s->sema > 0)
        sceKernelDeleteSema(s->sema);
    memset(s, 0, sizeof *s);
    return 1;
}

int ehfile_read_file_alias(int dir, const char *name, void *buf, int size)
{
    return ehfile_read_file(dir, name, buf, 0, size);
}

int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, EhfileCallback cb, void *arg)
{
    return ehfile_read_async(dir, name, buf, 0, size, cb, arg);
}

int ehfile_read_file(int dir, const char *name, void *buf, int offset, int size)
{
    char path[128];
    int fd, r;

    ehfile_make_path(path, dir, name, 0);
    if (ehfile_is_disc_path(path))
        return ehfile_read_disc(path, buf, offset, size);
    fd = sceIoOpen(path, 1, 0);
    if (fd < 0)
        return -1;
    if (offset > 0 && sceIoLseek(fd, offset, 0) < 0) {
        sceIoClose(fd);
        return -2;
    }
    r = sceIoRead(fd, buf, size);
    sceIoClose(fd);
    return r;
}

int ehfile_read_async(int dir, const char *name, void *buf, int offset, int size, EhfileCallback cb, void *arg)
{
    EhfileState *s = &ehfile_state;
    SceUInt timeout = 100;
    EhfileReq *req;
    unsigned int next;

    sceKernelWaitSema(s->sema, 1, &timeout);
    req = &s->queue[s->head];
    next = s->head + 1;
    if (next >= 16)
        next = 0;
    s->head = next;
    sceKernelSignalSema(s->sema, 1);
    req->active = 1;
    req->type = 0;
    req->dir = dir;
    req->buf = buf;
    req->offset = offset;
    req->size = size;
    req->cb = cb;
    req->arg = arg;
    strncpy(req->name, name, 31);
    sceKernelSignalSema(s->queue_sema, 1);
    return 1;
}

const char *ehfile_get_dir(int dir)
{
    return ehfile_state.dirs[dir];
}

int ehfile_make_path_alias(char *out, int dir, const char *name)
{
    return ehfile_make_path(out, dir, name, 0);
}

int ehfile_make_path(char *out, int dir, const char *name, int raw)
{
    EhfileLbn *lbn = &ehfile_lbn;
    unsigned int size;
    int sector;

    strcpy(out, ehfile_state.dirs[dir]);
    strcat(out, name);
    if (raw)
        return 1;
    if (ehfile_is_disc_path(out)) {
        if (lbn->ready) {
            sector = ehfile_lookup_lbn(out + 6, &size);
            if (sector >= 0)
                sprintf(out, "disc0:/sce_lbn0x%04x_size0x%04x", sector, size);
        }
    }
    return 1;
}

int ehfile_load_module(int dir, const char *name)
{
    char path[256];

    ehfile_make_path(path, dir, name, 0);
    return sceKernelLoadModule(path, 0, 0);
}

int ehfile_request_module(int dir, const char *name, EhfileCallback cb, void *arg)
{
    EhfileState *s = &ehfile_state;
    SceUInt timeout = 100;
    EhfileReq *req;
    unsigned int next;

    sceKernelWaitSema(s->sema, 1, &timeout);
    req = &s->queue[s->head];
    next = s->head + 1;
    if (next >= 16)
        next = 0;
    s->head = next;
    sceKernelSignalSema(s->sema, 1);
    req->active = 1;
    req->type = 1;
    req->dir = dir;
    req->buf = 0;
    req->offset = 0;
    req->size = 0;
    req->cb = cb;
    req->arg = arg;
    strncpy(req->name, name, 31);
    sceKernelSignalSema(s->queue_sema, 1);
    return 1;
}

int ehfile_get_size(int dir, const char *name)
{
    EhfileLbn *lbn = &ehfile_lbn;
    SceIoStat st;
    char path[128];
    unsigned int size;

    ehfile_make_path(path, dir, name, 1);
    if (ehfile_is_disc_path(path) && lbn->ready) {
        if (ehfile_lookup_lbn(path + 6, &size) >= 0)
            return size;
        if (ehfile_getstat_retry(path, &st) != 0)
            return -1;
        return (int)st.st_size;
    }
    if (sceIoGetstat(path, &st) != 0)
        return -1;
    return (int)st.st_size;
}

static EhfileReq *ehfile_queue_next(void);

int ehfile_thread(SceSize args, void *argp)
{
    EhfileState *s = &ehfile_state;
    EhfileReq *req;
    int r;

    for (;;) {
        do {
            sceKernelWaitSema(s->queue_sema, 1, 0);
            req = ehfile_queue_next();
        } while (req == 0);
        switch (req->type) {
        case 0:
            for (;;) {
                r = ehfile_read_file(req->dir, req->name, req->buf, req->offset, req->size);
                if (req->cb(req->arg, r))
                    break;
            }
            req->active = 0;
            break;
        case 1:
            for (;;) {
                r = ehfile_load_module(req->dir, req->name);
                if (req->cb(req->arg, r))
                    break;
            }
            req->active = 0;
            break;
        }
    }
}

static EhfileReq *ehfile_queue_next(void)
{
    EhfileState *s = &ehfile_state;
    unsigned int head = s->head;
    unsigned int i = head;
    EhfileReq *q;

    do {
        q = &s->queue[i];
        if (q->active)
            break;
        q = 0;
        if (++i >= 16)
            i = 0;
    } while (i != head);
    return q;
}

int ehfile_is_disc_path(const char *path)
{
    return *(const int *)path == *(const int *)"disc";
}

int gzip_decompress_alias(void *dst, int dst_len, const void *src)
{
    int out_size;
    return gzip_decompress(dst, dst_len, src, &out_size);
}

int ehfile_is_gzip(const void *p)
{
    return is_gzip(p) != 0;
}

/* Little-endian u32 ending at p+off: the gzip ISIZE trailer (uncompressed size) of a buffer of
   `off` bytes. The original builds it with the Allegrex `ins` intrinsic. */
unsigned int read_u32_before(const unsigned char *p, int off)
{
    unsigned int v;

    p += off;
    v = p[-4];
    v = ALLEGREX_INS(v, p[-3], 8, 8);
    v = ALLEGREX_INS(v, p[-2], 16, 8);
    v = ALLEGREX_INS(v, p[-1], 24, 8);
    return v;
}

int ehfile_busy(void)
{
    EhfileState *s = &ehfile_state;
    int i;

    for (i = 0; i < 16; i++)
        if (s->queue[i].active)
            return 1;
    return 0;
}

void ehfile_scan_iso_dir(int fd, const char *prefix, int lbn, int size)
{
    EhfileLbn *t = &ehfile_lbn;
    char path[256];
    int prefix_len = strlen(prefix);
    int sub = strlen(prefix); /* path + sub: where the entry name goes */
    SceOff saved;
    unsigned char *sector;
    IsoDirRec *rec;
    char *dst;
    int total;

    saved = ehfile_lseek(fd, 0, 1);
    sector = malloc(SECTOR);
    ehfile_lseek(fd, lbn, 0);
    ehfile_read_retry(fd, sector, 1);
    rec = (IsoDirRec *)sector;
    if (rec->len != 0) {
        do {
            if (rec->flags & 2) {
                if (rec->name[0] != 0 && rec->name[0] != 1) {
                    strcpy(path, prefix);
                    strncpy(path + sub, rec->name, rec->name_len);
                    path[sub + rec->name_len] = '/';
                    path[sub + rec->name_len + 1] = 0;
                    ehfile_scan_iso_dir(fd, path, rec->extent, rec->size);
                }
            } else {
                total = rec->name_len + prefix_len;
                dst = t->names_end - total;
                strcpy(dst, prefix);
                strncpy(dst + prefix_len, rec->name, rec->name_len);
                *t->names_end = 0;
                t->table_end->name = dst;
                t->table_end->lbn = rec->extent;
                t->table_end->size = rec->size;
                t->table_end++;
                t->names_end -= total + 1;
            }
            rec = (IsoDirRec *)((unsigned char *)rec + rec->len);
            if (rec->len == 0) {
                size -= SECTOR;
                if (size <= 0)
                    break;
                ehfile_read_retry(fd, sector, 1);
                rec = (IsoDirRec *)sector;
            }
        } while (rec->len != 0);
    }
    free(sector);
    ehfile_lseek(fd, saved, 0);
}

int ehfile_lbn_cmp(const void *a, const void *b)
{
    return stricmp(*(const char **)a, *(const char **)b);
}

int ehfile_lookup_lbn(const char *name, unsigned int *size_out)
{
    const char *key = name;
    LbnEntry *e;

    e = bsearch(&key, ehfile_lbn.table, ((unsigned int)ehfile_lbn.table_end - (unsigned int)ehfile_lbn.table) / sizeof(LbnEntry),
                sizeof(LbnEntry), ehfile_lbn_cmp);
    if (e == 0)
        return -1;
    if (size_out)
        *size_out = e->size;
    return e->lbn;
}

void ehfile_build_lbn_table(int pool_size)
{
    unsigned char *pvd;  /* declaration order drives mwcc's saved-register assignment */
    unsigned char *ptab;
    unsigned int ptab_size;
    int ptab_lbn;
    int fd;

    ehfile_lbn.fpl = sceKernelCreateFpl("", 2, 0, pool_size, 1, 0);
    sceKernelAllocateFpl(ehfile_lbn.fpl, (void **)&ehfile_lbn.buf, 0);
    fd = ehfile_open_retry("umd1:", 1, 0);
    pvd = malloc(SECTOR);
    ehfile_lseek(fd, 16, 0);
    ehfile_read_retry(fd, pvd, 1);
    ptab_size = (*(unsigned int *)(pvd + 0x84) + (SECTOR - 1)) & ~(SECTOR - 1);
    ptab_lbn = *(int *)(pvd + 0x8c);
    ptab = malloc(ptab_size);
    ehfile_lseek(fd, ptab_lbn, 0);
    ehfile_read_retry(fd, ptab, (int)ptab_size / SECTOR);
    ehfile_lbn.names_end = ehfile_lbn.buf + pool_size - 1;
    ehfile_lbn.table_end = (LbnEntry *)ehfile_lbn.buf;
    ehfile_lbn.table = (LbnEntry *)ehfile_lbn.buf;
    ehfile_scan_iso_dir(fd, "/", ((IsoDirRec *)ptab)->extent, SECTOR);
    free(ptab);
    free(pvd);
    ehfile_close(fd);
    qsort(ehfile_lbn.table, ((unsigned int)ehfile_lbn.table_end - (unsigned int)ehfile_lbn.table) / sizeof(LbnEntry),
          sizeof(LbnEntry), ehfile_lbn_cmp);
    ehfile_lbn.ready = 1;
}

static int ehfile_read_disc(const char *path, void *buf, int offset, int size)
{
    int fd, r;

    fd = ehfile_open_retry(path, 1, 0);
    if (fd < 0)
        return -1;
    if (offset > 0 && (int)ehfile_lseek(fd, offset, 0) < 0) {
        ehfile_close(fd);
        return -2;
    }
    r = ehfile_read_retry(fd, buf, size);
    ehfile_close(fd);
    return r;
}

static int ehfile_open_retry(const char *path, int flags, int mode)
{
    int r;

    do {
        if (!(sceUmdGetDriveStat() & 0x20)) {
            r = sceUmdWaitDriveStat(0x20);
            if (r < 0)
                break;
        }
        r = sceIoOpen(path, flags, mode);
    } while (r < 0 && r != (int)0x8001B002 && r != (int)0x80010013 && r != (int)0x8001000C &&
             r != (int)0x80010016 && r != (int)0x8001005B);
    return r;
}

static int ehfile_read_retry(int fd, void *buf, int size)
{
    int r;

    do {
        if (!(sceUmdGetDriveStat() & 0x20)) {
            r = sceUmdWaitDriveStat(0x20);
            if (r < 0)
                break;
        }
        r = sceIoRead(fd, buf, size);
    } while (r < 0 && r != (int)0x80010016 && r != (int)0x80010013 && r != (int)0x8001B003 &&
             r != (int)0x8001000E && r != (int)0x80210002 && r != (int)0x8001B001 && r != (int)0x8001005B);
    return r;
}

static SceOff ehfile_lseek(int fd, SceOff offset, int whence)
{
    return sceIoLseek(fd, offset, whence);
}

static int ehfile_getstat_retry(const char *path, SceIoStat *st)
{
    int r;

    if (path == 0 || st == 0)
        return (int)0x80010016;
    do {
        if (!(sceUmdGetDriveStat() & 0x20)) {
            r = sceUmdWaitDriveStat(0x20);
            if (r < 0)
                break;
        }
        r = sceIoGetstat(path, st);
    } while (r < 0 && r != (int)0x8001B002 && r != (int)0x80010013 && r != (int)0x80010002 &&
             r != (int)0x80010016 && r != (int)0x8001005B);
    return r;
}

static void ehfile_close(int fd)
{
    sceIoClose(fd);
}
