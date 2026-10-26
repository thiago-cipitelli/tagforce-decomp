// @range 0x0880BB98-0x0880C1C4
// @asm 0x0880BBE4-0x0880BDFC
/* File loader of the field: the heap of the module (7.5 MB), the card picture reader, the cache of
   the shared GIM files (story_sys01.gim, ...) and the textures looked up in it. */
#include "libfield.h"

void fld_cip_open(void);
void fld_req_init(void);                       /* 0x0880C860 */
int fld_req_idle(void);                        /* 0x0880C97C */
int fld_res_find(int name, int tab, int n, int size); /* 0x0880C9BC */
void fld_req_add(int kind, FieldResEnt *e, int a, void (*cb)(), void *arg, int b); /* 0x0880CA9C */
void fld_waiter_add(FieldWaitQ *w, void (*cb)(), void *arg); /* 0x0880CBC0 */
void fld_gims_free(void);
void fld_req_cancel_all(void);                 /* 0x0880C7D4 */
void fld_load_cancel_chr(void);
void fld_load_cancel_bg(void);
void fld_load_cancel_all(void);

extern unsigned char field_heap_mem[];         /* 0x088510A0 */
extern unsigned char field_cip_mem[];          /* 0x08849EA0 */
extern int field_cip_closing;                  /* 0x08849E90 */

void fld_res_init(void)
{
    memset(&field_res, 0, sizeof(FieldRes));
    field_res.heap = heap_create(field_heap_mem, 0x780000);
    fld_cip_open();
    fld_req_init();
}

int fld_res_term(void)
{
    FieldRes *r = &field_res;

    if (fld_req_idle() == 0)
        return 0;
    if (r->heap) {
        heap_free_block(r->heap);
        r->heap = 0;
    }
    if (r->cip) {
        if (field_cip_closing == 0) {
            cip_close(r->cip);
            field_cip_closing = 1;
        } else if (cip_is_idle(r->cip) == 0) {
            return 0;
        }
    }
    memset(&field_res, 0, sizeof(FieldRes));
    field_cip_closing = 0;
    return 1;
}

void fld_res_cancel(void)
{
    fld_gims_free();
    fld_load_cancel_chr();
    fld_load_cancel_bg();
    fld_load_cancel_all();
    fld_req_cancel_all();
}

void *fld_alloc(unsigned int size)
{
    return heap_alloc(field_res.heap, size);
}

void fld_free(void *p)
{
    heap_set_level(field_res.heap, p);
}

void *fld_heap_get(void)
{
    return field_res.heap;
}

void fld_cip_open(void)
{
    FieldRes *r = &field_res;

    if (field_res.cip == 0)
        r->cip = cip_open_h(field_cip_mem, 0x7200, 0);
}

void *fld_cip_get(void)
{
    return field_res.cip;
}

void fld_load_gim(int name, void (*cb)(int *file, void *arg), void *arg)
{
    FieldResEnt *e = &field_res_gim[fld_res_find(name, (int)field_res_gim, 9, 0x28)];

    if (e->data) {
        cb((int *)&e->data, arg);
    } else {
        e->discard = 0;
        if (e->loading) {
            fld_waiter_add(&e->waiters, cb, arg);
        } else {
            e->loading = 1;
            fld_req_add(0, e, 0, cb, arg, 0);
        }
    }
}

void fld_gim_free(int name)
{
    int i = fld_res_find(name, (int)field_res_gim, 9, 0x28);
    FieldResEnt *e;

    if (i >= 0) {
        e = &field_res_gim[i];
        if (e->data == 0 && e->loading) {
            e->discard = 1;
        } else {
            if (e->buf) {
                fld_free(e->buf);
                e->buf = 0;
            }
            e->buf = 0;
            e->data = 0;
            e->loading = 0;
            memset(&e->waiters, 0, 8);
            e->discard = 0;
        }
    }
}

void fld_gims_free(void)
{
    unsigned int i;
    FieldResEnt *e;

    for (i = 0, e = field_res_gim; i < 9; i++, e++) {
        if (e) {
            if (e->buf) {
                fld_free(e->buf);
                e->buf = 0;
            }
            e->data = 0;
            e->loading = 0;
            memset(&e->waiters, 0, 8);
            e->discard = 0;
        }
    }
}

/* texture i of the shared GIM pack */
void *fld_gim_get(int i)
{
    FieldResEnt *e = field_res_gim;
    FieldGim *g = &field_gims[i];
    void *r;

    if (field_res_gim[0].data) {
        if ((r = g->gim) != 0)
            return r;
        r = ehp_find(e->data, g->name, 0);
        g->gim = r;
        return r;
    }
    return 0;
}
