// @range 0x0880C1C4-0x0880CA9C
// @asm 0x0880C1C4-0x0880C390
/* File pools of the loader (SD characters, room models, sprites, voices): unload by name, size of a
   file before reading it, the typed load requests, cancelling a pool, the reset of the caches and
   the lookups in the tables. */
#include "libfield.h"

void fld_pool_load(int kind, int name, void (*cb)(), void *arg, int flag); /* 0x0880C1C4 */
int fld_chr_index(int id);                     /* 0x0880D4F4 */
int fld_voice_index(int id);                   /* 0x0880F154 */
int fld_req_pending(void);                     /* 0x0880CCF4 */
void fld_res_update(void);
void fld_res_cancel(void);

extern const char str_path_fmt[];              /* "%s/%s" */
extern const char *field_always_ehp[][6];      /* 0x0883C28C */

void fld_unload(int kind, int name)
{
    FieldPoolDesc *d = &field_pools[kind];
    FieldPool *p = d->pool;
    FieldPoolEnt *e = p->e;
    int i = fld_res_find(name, (int)e, d->n, 0x20);

    if (i >= 0) {
        e = &e[i];
        if (e->data == 0 && e->loading) {
            e->discard = 1;
        } else {
            if (e->buf) {
                fld_free(e->buf);
                e->buf = 0;
            }
            memset(e, 0, sizeof(FieldPoolEnt));
            p->count--;
        }
    }
}

int fld_load_size(int kind, int id)
{
    FieldPoolDesc *d = &field_pools[kind];
    char path[0x100];
    int i;

    switch (kind) {
    case 0:
        i = fld_chr_index(id);
        break;
    case 1:
        i = fld_chr_index(id);
        break;
    case 2:
        i = fld_bg_index(id);
        break;
    case 3:
        i = fld_bu_index(id);
        break;
    case 4:
        i = fld_voice_index(id);
        break;
    }
    sprintf(path, str_path_fmt, d->dir, d->names[i]);
    return ehfile_get_size(3, path);
}

void fld_load_chr(int id, int n, void (*cb)(), void *arg)
{
    int k = 0;

    switch (n) {
    case 0:
        k = 0;
        break;
    case 1:
        k = 1;
        break;
    }
    fld_pool_load(k, id, cb, arg, 0);
}

void fld_load_bg(int id, void (*cb)(), void *arg)
{
    fld_pool_load(2, id, cb, arg, 0);
}

void fld_load(int name, void (*cb)(int *file, void *arg), void *arg, int flag)
{
    fld_pool_load(3, name, cb, arg, flag);
}

void fld_load_voice(int name, void (*cb)(), void *arg)
{
    fld_pool_load(4, name, cb, arg, 1);
}

void fld_load_cancel_chr(void)
{
    unsigned int i;
    FieldPoolEnt *e;

    for (i = 0; i < 12; i++) {
        e = &field_pool_chr.e[i];
        if (e) {
            if (e->buf) {
                fld_free(e->buf);
                e->buf = 0;
            }
            memset(e, 0, 4);
        }
    }
    memset(&field_pool_chr, 0, sizeof(field_pool_chr));
    {
        FieldPoolEnt *f;

        for (i = 0; i < 2; i++) {
            f = &field_pool_chr2.e[i];
            if (f) {
                if (f->buf) {
                    fld_free(f->buf);
                    f->buf = 0;
                }
                memset(f, 0, 4);
            }
        }
    }
    memset(&field_pool_chr2, 0, sizeof(field_pool_chr2));
}

void fld_load_cancel_bg(void)
{
    unsigned int i;
    FieldPoolEnt *e;

    i = 0;
    do {
        e = &field_pool_bg.e[i];
        if (e) {
            if (e->buf) {
                fld_free(e->buf);
                e->buf = 0;
            }
            memset(e, 0, 4);
        }
    } while (++i < 1);
    memset(&field_pool_bg, 0, sizeof(field_pool_bg));
}

void fld_load_cancel_all(void)
{
    unsigned int i;
    FieldPoolEnt *e;

    for (i = 0; i < 10; i++) {
        e = &field_pool_misc.e[i];
        if (e) {
            if (e->buf) {
                fld_free(e->buf);
                e->buf = 0;
            }
            memset(e, 0, 4);
        }
    }
    memset(&field_pool_misc, 0, sizeof(field_pool_misc));
}

void fld_req_cancel_all(void)
{
    unsigned int i;
    FieldPoolEnt *e;

    i = 0;
    do {
        e = &field_pool_voice.e[i];
        if (e) {
            if (e->buf) {
                fld_free(e->buf);
                e->buf = 0;
            }
            memset(e, 0, 4);
        }
    } while (++i < 1);
    memset(&field_pool_voice, 0, sizeof(field_pool_voice));
}

void fld_req_init(void)
{
    int region = region_get();
    int lang = cfg_get_language();
    unsigned int i;
    FieldResEnt *e;
    FieldGim *g;

    for (i = 0, e = field_res_gim; i < 9; i++, e++) {
        e->buf = 0;
        e->data = 0;
        e->loading = 0;
        memset(e->waiters, 0, 8);
        e->discard = 0;
    }
    field_res_gim[0].pad04[1] = (int)field_always_ehp[region][lang];
    i = 0;
    g = field_gims;
    do {
        g->gim = 0;
        g++;
    } while (++i < 20);
    memset(&field_pool_chr, 0, sizeof(field_pool_chr));
    memset(&field_pool_chr2, 0, sizeof(field_pool_chr2));
    memset(&field_pool_bg, 0, sizeof(field_pool_bg));
    memset(&field_pool_misc, 0, sizeof(field_pool_misc));
}

int fld_req_idle(void)
{
    if (fld_req_pending() == 0) {
        fld_res_update();
        return 0;
    }
    fld_res_cancel();
    return 1;
}

int fld_res_find(int name, int tab, int n, int size)
{
    int i;
    int v;

    for (i = 0; i < n; i++) {
        v = *(int *)(tab + i * size);
        if (v == name)
            return i;
    }
    return -1;
}

int fld_res_find_free(int name, int tab, int n, int size)
{
    int i;

    for (i = 0; i < n; i++) {
        if (*(int *)(tab + i * size) == 0)
            return i;
    }
    return -1;
}

int fld_req_step(void *pack, int n)
{
    FieldRes *r = &field_res;
    int *q = (int *)&field_res.req;
    int *cur;

    if (n > 0) {
        cur = r->req;
        if (pack)
            ehp_mount(0, pack);
        cur[1]++;
        q[2] = 0;
        return 1;
    }
    return 0;
}
