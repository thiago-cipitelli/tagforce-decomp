// @range 0x0880CA9C-0x0880D4F4
// @asm 0x0880CF74-0x0880D0F8
// @asm 0x0880D2B4-0x0880D4F4
/* Queue of the load requests and the callbacks waiting for each file, the asynchronous reads, the
   field context (frame counter), the time of day and small helpers (lengths, color lerp, sprites). */
#include "libfield.h"

int fld_req_step(void *pack, int n);
void *fld_draw_cell(void *list, int x, int y, int u, int v, int sw, int sh, int dw, int dh);

extern const char str_path_fmt[];              /* "%s/%s" */
extern unsigned char field_dot0;               /* "." */
extern unsigned char field_dot1;

int fld_req_add(int kind, void *ent, void *ent2, void (*cb)(), void *arg, int front)
{
    FieldReqQ *q = &field_res.q;
    FieldReq *r = fld_alloc(sizeof(FieldReq));

    if (r == 0)
        return 0;
    r->kind = kind;
    r->step = 0;
    r->ent = ent;
    r->ent2 = ent2;
    r->cb = cb;
    r->arg = arg;
    r->next = 0;
    if (q->head == 0) {
        q->head = r;
        q->tail = r;
    } else if (!front) {
        q->tail->next = r;
        q->tail = r;
    } else {
        r->next = q->head->next;
        q->head->next = r;
    }
    return 1;
}

void fld_req_pop(void)
{
    FieldReqQ *q = &field_res.q;
    FieldReq *r = q->head;

    if (r) {
        if (r->next)
            q->head = r->next;
        else
            q->head = 0;
        if (r)
            fld_free(r);
    }
}

int fld_waiter_add(FieldWaitQ *w, void (*cb)(), void *arg)
{
    FieldWaiter *n = fld_alloc(sizeof(FieldWaiter));

    if (n == 0)
        return 0;
    n->cb = cb;
    n->arg = arg;
    n->next = 0;
    if (w->head == 0) {
        w->head = n;
        w->tail = n;
    } else {
        w->tail->next = n;
        w->tail = n;
    }
    return 1;
}

void fld_waiter_pop(FieldWaitQ *w, void (**cb)(), void **arg)
{
    FieldWaiter *n;

    *cb = 0;
    *arg = 0;
    n = w->head;
    if (n) {
        *cb = n->cb;
        *arg = w->head->arg;
        if (w->head->next)
            w->head = w->head->next;
        else
            w->head = 0;
        if (n)
            fld_free(n);
    }
}

void fld_waiters_call(FieldWaitQ *w, void *file)
{
    void (*cb)();
    void *arg;

    for (;;) {
        fld_waiter_pop(w, &cb, &arg);
        if (cb == 0)
            break;
        cb(file, arg);
    }
}

int fld_req_pending(void)
{
    FieldReqQ *q = &field_res.q;

    if (q->busy)
        return 0;
    return !q->head;
}

int fld_req_read_gim(FieldResEnt *e)
{
    FieldReqQ *q = &field_res.q;
    char path[0x100];
    int size;

    sprintf(path, str_path_fmt, e->dir, e->file);
    size = ehfile_get_size(3, path);
    if (size <= 0)
        return 0;
    e->buf = fld_alloc(size);
    if (e->buf == 0)
        return 0;
    if (e->pack)
        ehfile_read_async_alias(3, path, e->buf, size, fld_req_step, e->buf);
    else
        ehfile_read_async_alias(3, path, e->buf, size, fld_req_step, 0);
    q->busy = 1;
    return 1;
}

int fld_req_read_pool(FieldPoolDesc *d, void **buf, int i)
{
    FieldReqQ *q = &field_res.q;
    char path[0x80];
    int size;

    sprintf(path, str_path_fmt, d->dir, d->names[i]);
    size = ehfile_get_size(3, path);
    if (size <= 0)
        return 0;
    *buf = fld_alloc(size);
    if (*buf == 0)
        return 0;
    if (d->x10)
        ehfile_read_async_alias(3, path, *buf, size, fld_req_step, *buf);
    else
        ehfile_read_async_alias(3, path, *buf, size, fld_req_step, 0);
    q->busy = 1;
    return 1;
}

void fld_ctx_init(void)
{
    memset(&field_ctx, 0, sizeof(FieldCtx));
}

FieldCtx *fld_ctx_get(void)
{
    return &field_ctx;
}

typedef struct FieldClock {
    unsigned char lo : 1;
    unsigned char part : 3;
} FieldClock;

/* part of the day from the saved clock */
int fld_day_part(void)
{
    union {
        short s;
        FieldClock c;
    } u;
    short t[2];

    deck_cur_get_u16_0(t);
    u.s = t[0];
    return u.c.part;
}

/* 0 before 16:00, 1 before 19:00, 2 at night */
int fld_time_index(int h)
{
    if (h < 16)
        return 0;
    if (h < 19)
        return 1;
    return 2;
}

float fld_stick_axis(int v)
{
    int d;

    if (v - 0x80 > 0) {
        d = v - 0x98;
        d = d > 0 ? d : 0;
    } else {
        d = v - 0x68;
        d = d < 0 ? d : 0;
    }
    return d / 103.0f;
}

float fld_vec3_len(Vec3 *v)
{
    return fabsf_vfpu(sqrtf_vfpu(v->z * v->z + (v->x * v->x + v->y * v->y)));
}

/* the text after the first "." of s */
char *fld_after_dot(char *s)
{
    char d[2];

    d[0] = field_dot0;
    d[1] = field_dot1;
    return strstr(s, d) + 1;
}

unsigned int fld_color_lerp(float t, unsigned int a, unsigned int b)
{
    float ca[4];
    float cb[4];
    float c[4];

    color_unpack(ca, a);
    color_unpack(cb, b);
    vec4_lerp2(t, c, ca, cb);
    return color_pack(c);
}

void *fld_draw_cell1(void *list, int x, int y, int u, int v, int w, int h)
{
    return fld_draw_cell(list, x, y, u, v, w, h, w, h);
}

void *fld_draw_cell(void *list, int x, int y, int u, int v, int sw, int sh, int dw, int dh)
{
    list = draw_set_offsets(list, sw, sh, dw, dh);
    return draw_sprite(list, x, y, 0, u, v, 0, 0);
}
