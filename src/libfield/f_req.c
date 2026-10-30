// @range 0x0880CA9C-0x0880D4F4
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

/* event of the cell (x, z) of the room bg: 1 the shop counter of room 0x14 (before evening), 2 its
   board, 3 and 4 the two doors of room 0x20 */
int fld_bg_event(int bg, int x, int z)
{
    int r = 0;

    switch (bg) {
    case 20:
        if ((x == 1 && z == 4) || (x == 2 && z == 4) || (x == 3 && z == 4) || (x == 4 && z == 4)
            || (x == 5 && z == 4) || (x == 6 && z == 4)) {
            if (fld_day_part() < 2)
                r = 1;
        } else if ((x == 3 && z == 6) || (x == 4 && z == 6)) {
            r = 2;
        }
        break;
    case 32:
        if ((x == 32 && z == 2) || (x == 33 && z == 2))
            r = 3;
        if ((x == 30 && z == 2) || (x == 31 && z == 2))
            r = 4;
        break;
    }
    return r;
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

/* nine-patch: the sprite (u, v, w, h) drawn as a frame of w2 x h2 at (x, y), keeping the corners
   (left/right widths, top/bottom heights) and stretching the edges and the middle */
void *fld_draw_frame(void *list, int x, int y, int u, int v, int w, int h, int w2, int h2, int top,
                     int bottom, int left, int right)
{
    int vb;
    int ur;
    int yb;
    int xr;
    int mw2;
    int mw;
    int um;
    int xm;
    int vm;
    int mh;
    int ym;
    int mh2;

    list = fld_draw_cell1(list, x, y, u, v, left, top);
    vb = v + h - bottom;
    yb = y + h2 - bottom;
    list = fld_draw_cell1(list, x, yb, u, vb, left, bottom);
    ur = u + w - right;
    xr = x + w2 - right;
    list = fld_draw_cell1(list, xr, y, ur, v, right, top);
    list = fld_draw_cell1(list, xr, yb, ur, vb, right, bottom);
    mw2 = w2 - (left + right);
    mw = w - (left + right);
    um = u + left;
    xm = x + left;
    list = fld_draw_cell(list, xm, y, um, v, mw, top, mw2, top);
    list = fld_draw_cell(list, xm, yb, um, vb, mw, bottom, mw2, bottom);
    mh = h - (top + bottom);
    vm = v + top;
    ym = y + top;
    mh2 = h2 - (top + bottom);
    list = fld_draw_cell(list, x, ym, u, vm, left, mh, left, mh2);
    list = fld_draw_cell(list, xr, ym, ur, vm, right, mh, right, mh2);
    return fld_draw_cell(list, xm, ym, um, vm, mw, mh, mw2, mh2);
}
