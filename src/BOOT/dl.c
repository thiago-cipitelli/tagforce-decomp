// @range 0x08839F50-0x0883CEB0
// @asm 0x0883A4E4-0x0883A6CC
// @asm 0x0883AFAC-0x0883B0DC
// @asm 0x0883B2A0-0x0883B684
// @asm 0x0883BF38-0x0883C040
// @asm 0x0883CBE8-0x0883CDB4
// @data 0x088EE050-0x088EE0A4
// @rodata 0x088753F4-0x0887544C
/* GE display lists: a double-buffered heap with a jump table of 16 layers at the start of each
   half (a layer's list is spliced in by a JUMP from the previous one), and one function per GE
   state or primitive that builds the command words on the stack and appends them with dl_push
   (which records the default of each opcode for dl_state_flush). */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "allegrex.h"

typedef struct DlState {               /* 0x089055F4 */
    unsigned int heap;                 /* uncached */
    int size;                          /* +0x04 both halves */
    unsigned int *sent;                /* +0x08 list handed to the GE */
    int used;                          /* +0x0C bytes of the last frame */
    int half;                          /* +0x10 */
    unsigned int *buf;                 /* +0x14 current half */
    unsigned int *jumps;               /* +0x18 16 BASE/JUMP pairs + FINISH/END */
    int cur;                           /* +0x1C */
    unsigned int *lists[16];           /* +0x20 write position of each layer */
    void *cur_list_base;               /* +0x60 */
    int flag;                          /* +0x64 */
} DlState;

extern int dl_mode;                    /* 0x08905570 */
extern char dl_send_arg[];             /* 0x08905574 */
extern DlState dl;                     /* 0x089055F4 */

#define BASE(p) (((unsigned int)(p) >> 24 & 0xF) << 16 | 0x10000000)
#define ADDR(op, p) (((unsigned int)(p) & 0xFFFFFF) | (op))
#define FBITS(f) (*(unsigned int *)&(f) >> 8)

unsigned int *dl_push(unsigned int *list, unsigned int *words, int n);
unsigned int *dl_push1(unsigned int *list, unsigned int cmd);
int sceGuSendList(int mode, void *list, int a2, int a3, void *arg);
void sceGuStart(int cid, void *list, int size);
int sceGuFinish(void);
void ehf_prim_reset(void);
void sceKernelDcacheWritebackInvalidateAll(void);

int dl_layers_reset(int half);
int dl_layers_link(void);

void dl_set_mode(int mode)
{
    dl_mode = mode;
}

int dl_init(HeapSpec *h)
{
    dl.heap = ALLEGREX_INS((unsigned int)h->addr, 4, 28, 4);
    dl.size = h->size;
    dl_layers_reset(0);
    dl.sent = 0;
    sys_config_init();
    return 1;
}

int dl_submit(void)
{
    dl_layers_link();
    dl.sent = dl.buf;
    sceGuSendList(0, dl.buf, 0, 4, dl_send_arg);
    dl_layers_reset(dl.half == 0);
    return 1;
}

/* Starts half `half` of the heap: empty layers, FINISH/END after the jump table, and the list of
   layer 0 opened right after it. */
int dl_layers_reset(int half)
{
    DlState *const d = &dl;
    unsigned int *j;
    unsigned int *p;
    int i;
    unsigned int *l;

    dl.jumps = dl.buf = (unsigned int *)(dl.heap + dl.size / 2 * half);
    j = dl.jumps;
    for (i = 0, l = (unsigned int *)d; i < 16; i++) {
        j[0] = 0;
        j[1] = 0;
        l[8] = 0;                      /* d->lists[i] */
        j += 2;
        l++;
    }
    j[0] = 0x0F000000;
    j[1] = 0x0C000000;
    j[2] = 0;
    j[3] = 0;
    p = (unsigned int *)((char *)d->buf + ((char *)(j + 4) - (char *)d->jumps));
    d->jumps[0] = BASE(p);
    d->jumps[1] = ADDR(0x08000000, p);
    sceGuStart(2, p, (char *)d->buf + d->size / 2 - (char *)p);
    d->lists[0] = p + (((unsigned int)sceGuFinish() >> 2) - 2);
    d->half = half;
    d->cur = 0;
    ehf_prim_reset();
    return 1;
}

/* Before sending: the END of the current layer becomes the frame size, and every jump-table
   entry jumps over the layers that were never opened. */
int dl_layers_link(void)
{
    DlState *const d = &dl;
    unsigned int *e;
    unsigned int *n;
    unsigned int *l;
    int i;

    dl.used = (char *)(dl.lists[dl.cur] + 2) - (char *)dl.buf;
    for (i = 15; i >= 0; i--) {
        e = d->jumps + i * 2;
        n = e + 2;
        l = d->lists[i];
        if (l == 0) {
            if (*n != 0) {
                e[0] = n[0];
                e[1] = n[1];
            } else {
                e[0] = 0;
                e[1] = 0;
            }
        } else if (*n != 0) {
            l[0] = n[0];
            d->lists[i][1] = n[1];
        } else {
            l[0] = BASE(n);
            d->lists[i][1] = ADDR(0x08000000, n);
        }
    }
    sceKernelDcacheWritebackInvalidateAll();
    return 1;
}

/* Switches to layer `layer`: the current one is closed with a JUMP back into the chain and the new
   one resumes where it stopped (or starts at the jump table). Returns where to write. */
unsigned int *dl_select_layer(int layer, int *size)
{
    DlState *const d = &dl;
    unsigned int *p;
    unsigned int *e;

    dl.flag = 1;
    if (dl.cur == layer) {
        return d->lists[layer];
    }
    d->lists[d->cur][0] = 0;
    d->lists[d->cur][1] = 0;
    p = d->lists[d->cur] + 2;
    if (d->lists[layer] == 0) {
        e = d->jumps + layer * 2;
        e[0] = BASE(p);
        e[1] = ADDR(0x08000000, p);
    } else {
        d->lists[layer][0] = BASE(p);
        d->lists[layer][1] = ADDR(0x08000000, p);
    }
    d->cur = layer;
    if (size != 0) {
        *size = d->size / 2 - ((char *)p - (char *)d->buf);
    }
    return p;
}

int dl_set_list(unsigned int *list)
{
    dl.flag = 0;
    dl.lists[dl.cur] = list;
    return 1;
}

void dl_set_cur_list_base(void *p)
{
    dl.cur_list_base = p;
}

void *dl_get_cur_list_base(void)
{
    return dl.cur_list_base;
}

/* Pads with zeros to a 64-byte boundary. */
unsigned int *dl_align64(unsigned int *list)
{
    while ((unsigned int)list & 0x3F) {
        *list++ = 0;
    }
    return list;
}

int dl_get_layer(void)
{
    return dl.cur;
}

int dl_get_flag(void)
{
    return dl.flag;
}

/* Reserves `bytes` inside the list (jumped over) and moves *list past them. */
void dl_alloc_inline(unsigned int **list, int bytes)
{
    unsigned int *l = *list;
    unsigned int *end = l + (bytes + 3) / 4 + 8;
    unsigned int cmd[2] = { 0 };

    cmd[0] = (((unsigned int)end & 0xFF000000) >> 24 & 0xF) << 16 | 0x10000000;
    cmd[1] = ADDR(0x08000000, end);
    dl_push(l, cmd, 2);
    *list = end;
}

#ifdef NON_MATCHING /* the scissor case loads and combines the fields in another order */
unsigned int *dl_enable(unsigned int *list, int state)
{
    SysConf *c = &sys_conf;
    unsigned int cmd[4] = { 0 };
    int op = 0;
    unsigned int y;
    unsigned int x;

    switch (state) {
    case 0:
        op = 0x22;
        break;
    case 1:
        op = 0x23;
        break;
    case 2:
        cmd[3] = c->scissor_y << 10;
        x = c->scissor_x + c->scissor_w - 1;
        y = (c->scissor_y + c->scissor_h - 1) << 10;
        cmd[0] = cmd[3] | 0xD4000000 | c->scissor_x;
        cmd[1] = y | 0xD5000000 | x;
        cmd[2] = 0x15000000;
        cmd[3] = y | 0x16000000 | x;
        list = dl_push(list, cmd, 4);
        c->scissor_on = 1;
        break;
    case 3:
        op = 0x24;
        break;
    case 4:
        op = 0x21;
        break;
    case 5:
        op = 0x1D;
        break;
    case 6:
        op = 0x20;
        break;
    case 7:
        op = 0x1F;
        break;
    case 8:
        op = 0x1C;
        break;
    case 9:
        op = 0x1E;
        break;
    case 10:
        op = 0x17;
        break;
    case 11:
        op = 0x18;
        break;
    case 12:
        op = 0x19;
        break;
    case 13:
        op = 0x1A;
        break;
    case 14:
        op = 0x1B;
        break;
    case 15:
        op = 0x25;
        break;
    case 16:
        op = 0x26;
        break;
    case 17:
        op = 0x27;
        break;
    case 18:
        op = 0x28;
        break;
    case 19:
        op = 0x51;
        break;
    case 20:
        op = 0x38;
        break;
    case 21:
        cmd[0] = c->tfunc_comp << 8 | 0xC9010000 | c->tfunc;
        list = dl_push(list, cmd, 1);
        c->tfunc_double = 1;
        break;
    }
    if (op != 0) {
        cmd[0] = op << 24 | 1;
        list = dl_push(list, cmd, 1);
    }
    return list;
}
#endif

unsigned int *dl_disable(unsigned int *list, int state)
{
    SysConf *c = &sys_conf;
    unsigned int cmd[4] = { 0 };
    unsigned int x;
    int op = 0;
    unsigned int y;

    switch (state) {
    case 0:
        op = 0x22;
        break;
    case 1:
        op = 0x23;
        break;
    case 2:
        x = c->width - 1;
        y = (c->height - 1) << 10;
        cmd[0] = 0xD4000000;
        cmd[1] = y | 0xD5000000 | x;
        cmd[2] = 0x15000000;
        cmd[3] = y | 0x16000000 | x;
        list = dl_push(list, cmd, 4);
        c->scissor_on = 0;
        break;
    case 3:
        op = 0x24;
        break;
    case 4:
        op = 0x21;
        break;
    case 5:
        op = 0x1D;
        break;
    case 6:
        op = 0x20;
        break;
    case 7:
        op = 0x1F;
        break;
    case 8:
        op = 0x1C;
        break;
    case 9:
        op = 0x1E;
        break;
    case 10:
        op = 0x17;
        break;
    case 11:
        op = 0x18;
        break;
    case 12:
        op = 0x19;
        break;
    case 13:
        op = 0x1A;
        break;
    case 14:
        op = 0x1B;
        break;
    case 15:
        op = 0x25;
        break;
    case 16:
        op = 0x26;
        break;
    case 17:
        op = 0x27;
        break;
    case 18:
        op = 0x28;
        break;
    case 19:
        op = 0x51;
        break;
    case 20:
        op = 0x38;
        break;
    case 21:
        cmd[0] = c->tfunc_comp << 8 | 0xC9000000 | c->tfunc;
        list = dl_push(list, cmd, 1);
        c->tfunc_double = 0;
        break;
    }
    if (op != 0) {
        cmd[0] = op << 24;
        list = dl_push(list, cmd, 1);
    }
    return list;
}

/* VTYPE, the index buffer when the vertex type has one, the vertex buffer and PRIM. */
unsigned int *dl_draw_array(unsigned int *list, int prim, unsigned int vtype, int count, void *indices, void *vertices)
{
    unsigned int cmd[6] = { 0 };
    int n;

    n = 0;
    cmd[n++] = ADDR(0x12000000, vtype);
    if (vtype & 0x1800) {
        cmd[n++] = BASE(indices);
        cmd[n++] = ADDR(0x02000000, indices);
    }
    cmd[n++] = BASE(vertices);
    cmd[n++] = ADDR(0x01000000, vertices);
    cmd[n++] = prim << 16 | 0x04000000 | count;
    return dl_push(list, cmd, n);
}

/* Same with `times` PRIM commands over consecutive vertices. */
unsigned int *dl_draw_array_n(unsigned int *list, int prim, unsigned int vtype, int count, int times, void *indices, void *vertices)
{
    unsigned int cmd[5] = { 0 };
    int n;
    int i;
    unsigned int *p;

    n = 0;
    cmd[n++] = ADDR(0x12000000, vtype);
    if (vtype & 0x1800) {
        cmd[n++] = BASE(indices);
        cmd[n++] = ADDR(0x02000000, indices);
    }
    cmd[n++] = BASE(vertices);
    cmd[n++] = ADDR(0x01000000, vertices);
    p = dl_push(list, cmd, n);
    for (i = 0; i < times; i++) {
        p = dl_push1(p, prim << 16 | 0x04000000 | count);
    }
    return p;
}

/* Bezier patch (BEZIER). */
unsigned int *dl_draw_bezier(unsigned int *list, unsigned int vtype, int ucount, int vcount, void *indices, void *vertices)
{
    unsigned int cmd[6] = { 0 };
    int n;

    n = 0;
    cmd[n++] = ADDR(0x12000000, vtype);
    if (vtype & 0x1800) {
        cmd[n++] = BASE(indices);
        cmd[n++] = ADDR(0x02000000, indices);
    }
    cmd[n++] = BASE(vertices);
    cmd[n++] = ADDR(0x01000000, vertices);
    cmd[n++] = vcount << 8 | 0x05000000 | ucount;
    return dl_push(list, cmd, n);
}

/* Spline surface (SPLINE). */
unsigned int *dl_draw_spline(unsigned int *list, unsigned int vtype, int ucount, int vcount, int uedge, int vedge, void *indices, void *vertices)
{
    unsigned int cmd[6] = { 0 };
    int n;

    n = 0;
    cmd[n++] = ADDR(0x12000000, vtype);
    if (vtype & 0x1800) {
        cmd[n++] = BASE(indices);
        cmd[n++] = ADDR(0x02000000, indices);
    }
    cmd[n++] = BASE(vertices);
    cmd[n++] = ADDR(0x01000000, vertices);
    cmd[n++] = ucount | (vcount << 8 | (vedge << 18 | 0x06000000 | uedge << 16));
    return dl_push(list, cmd, n);
}

unsigned int *dl_patch_divide(unsigned int *list, int u, int v)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = v << 8 | 0x36000000 | u;
    return dl_push(list, cmd, 1);
}

unsigned int *dl_patch_prim(unsigned int *list, int prim)
{
    unsigned int cmd[1] = { 0 };

    if (prim == 0) {
        cmd[0] = 0x37000002;
    } else if (prim == 2) {
        cmd[0] = 0x37000001;
    } else {
        cmd[0] = 0x37000000;
    }
    return dl_push(list, cmd, 1);
}

typedef struct SpriteVtx {
    unsigned short u, v;
    short x, y, z;
} SpriteVtx;

/* One sprite (two vertices inlined after a JUMP), the texture rectangle mirrored by flip_uv
   (1 u, 2 v, 3 both) and the screen rectangle by flip_xy (1 y, 2 both, 3 x); sizes come from
   draw_set_offsets. */
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy)
{
    SpriteVtx vt[2];
    unsigned int cmd[11] = { 0 };
    unsigned short t;
    short ts;

    vt[0].u = u;
    vt[0].v = v;
    vt[1].u = u + sys_conf.sprite_size[0];
    vt[1].v = v + sys_conf.sprite_size[1];
    vt[0].x = x;
    vt[0].y = y;
    vt[1].x = x + sys_conf.sprite_size[2];
    vt[1].y = y + sys_conf.sprite_size[3];
    vt[0].z = z;
    vt[1].z = z;
    if (flip_uv == 0) {
    } else if (flip_uv == 1) {
        t = vt[0].u;
        vt[0].u = vt[1].u;
        vt[1].u = t;
    } else if (flip_uv == 2) {
        t = vt[0].v;
        vt[0].v = vt[1].v;
        vt[1].v = t;
    } else if (flip_uv == 3) {
        t = vt[0].u;
        vt[0].u = vt[1].u;
        vt[1].u = t;
        t = vt[0].v;
        vt[0].v = vt[1].v;
        vt[1].v = t;
    }
    if (flip_xy == 0) {
    } else if (flip_xy == 1) {
        ts = vt[0].y;
        vt[0].y = vt[1].y;
        vt[1].y = ts;
    } else if (flip_xy == 2) {
        ts = vt[0].x;
        vt[0].x = vt[1].x;
        vt[1].x = ts;
        ts = vt[0].y;
        vt[0].y = vt[1].y;
        vt[1].y = ts;
    } else if (flip_xy == 3) {
        ts = vt[0].x;
        vt[0].x = vt[1].x;
        vt[1].x = ts;
    }
    cmd[0] = BASE((char *)list + 0x1C);
    cmd[1] = ADDR(0x08000000, (char *)list + 0x1C);
    memcpy(&cmd[2], vt, 0x14);
    cmd[7] = 0x12800102;
    cmd[8] = BASE((char *)list + 8);
    cmd[9] = ADDR(0x01000000, (char *)list + 8);
    cmd[10] = 0x04060002;
    return dl_push(list, cmd, 11);
}

unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h)
{
    sys_conf.sprite_size[0] = tw;
    sys_conf.sprite_size[1] = th;
    sys_conf.sprite_size[2] = w;
    sys_conf.sprite_size[3] = h;
    return list;
}

#ifdef NON_MATCHING /* scheduling of the vertex type fields differs */
/* Opens a primitive whose vertex count is patched by dl_prim_end. */
unsigned int *dl_prim_begin(unsigned int *list, unsigned int vtype, int prim, void *indices, void *vertices)
{
    SysConf *c = &sys_conf;
    unsigned int cmd[8] = { 0 };
    unsigned int ib = vtype >> 11 & 3;
    int n;

    n = 0;
    cmd[n++] = ib << 11 | 0x12000000 | (vtype >> 7 & 3) << 7;
    if (ib != 0) {
        cmd[n++] = BASE(indices);
        cmd[n++] = ADDR(0x02000000, indices);
    }
    cmd[n++] = BASE(vertices);
    cmd[n++] = ADDR(0x01000000, vertices);
    cmd[n++] = prim | 0x07000000;
    c->prim = &cmd[n++];
    *c->prim = 0x10000000;
    cmd[n++] = 0x09000000;
    return dl_push(list, cmd, n);
}
#endif

unsigned int *dl_prim_end(unsigned int *list)
{
    unsigned int *p = sys_conf.prim;

    p[0] = ((unsigned int)list >> 24 & 0xF) << 16 | 0x10000000;
    p[1] = ADDR(0x09000000, list);
    sys_conf.prim = 0;
    return list;
}

/* Bone matrix `index` (BOFS + 12 BONE). */
unsigned int *dl_bone_matrix(unsigned int *list, int index, float *m)
{
    unsigned int cmd[14] = { 0xFF000000 };
    unsigned int *u = (unsigned int *)m;

    cmd[1] = index * 12 | 0x2A000000;
    cmd[2] = (u[0] >> 8 & 0xFFFFFF) | 0x2B000000;
    cmd[3] = (u[1] >> 8 & 0xFFFFFF) | 0x2B000000;
    cmd[4] = (u[2] >> 8 & 0xFFFFFF) | 0x2B000000;
    cmd[5] = (u[4] >> 8 & 0xFFFFFF) | 0x2B000000;
    cmd[6] = (u[5] >> 8 & 0xFFFFFF) | 0x2B000000;
    cmd[7] = (u[6] >> 8 & 0xFFFFFF) | 0x2B000000;
    cmd[8] = (u[8] >> 8 & 0xFFFFFF) | 0x2B000000;
    cmd[9] = (u[9] >> 8 & 0xFFFFFF) | 0x2B000000;
    cmd[10] = (u[10] >> 8 & 0xFFFFFF) | 0x2B000000;
    cmd[11] = (u[12] >> 8 & 0xFFFFFF) | 0x2B000000;
    cmd[12] = (u[13] >> 8 & 0xFFFFFF) | 0x2B000000;
    cmd[13] = (u[14] >> 8 & 0xFFFFFF) | 0x2B000000;
    return dl_push(list, cmd, 14);
}

unsigned int *dl_morph_weight(float w, unsigned int *list, int index)
{
    unsigned int cmd[2] = { 0xFF000000 };

    cmd[1] = (index + 0x2C) << 24 | (*(unsigned int *)&w >> 8 & 0xFFFFFF);
    return dl_push(list, cmd, 2);
}

#ifdef NON_MATCHING /* the write pointer and the GE opcode land in other registers */
/* Loads a matrix straight into the list: 0 projection (4x4), 1 world, 2 view, 3 texture (4x3). */
unsigned int *dl_set_matrix(unsigned int *list, int type, float *m)
{
    unsigned int *u = (unsigned int *)m;
    unsigned int *p;
    unsigned int op;
    int i;

    switch (type) {
    default:
        return list;
    case 0:
        *list = 0x3E000000;
        p = list + 1;
        for (i = 0; i < 16; i++) {
            *p++ = *u++ >> 8 | 0x3F000000;
        }
        return p;
    case 1:
        *list = 0x3C000000;
        p = list + 1;
        op = 0x3D;
        break;
    case 2:
        *list = 0x3A000000;
        p = list + 1;
        op = 0x3B;
        break;
    case 3:
        *list = 0x40000000;
        p = list + 1;
        op = 0x41;
        break;
    }
    op <<= 24;
    p[0] = op | u[0] >> 8;
    p[1] = op | u[1] >> 8;
    p[2] = op | u[2] >> 8;
    p[3] = op | u[4] >> 8;
    p[4] = op | u[5] >> 8;
    p[5] = op | u[6] >> 8;
    p[6] = op | u[8] >> 8;
    p[7] = op | u[9] >> 8;
    p[8] = op | u[10] >> 8;
    p[9] = op | u[12] >> 8;
    p[10] = op | u[13] >> 8;
    p[11] = op | u[14] >> 8;
    return p + 12;
}
#endif

#ifdef NON_MATCHING /* register allocation of the float conversions differs */
/* Depth range (sceGuDepthRange): viewport Z scale/center and the min/max Z clamp. */
unsigned int *dl_depth_range(unsigned int *list, int near, int far)
{
    unsigned int cmd[4] = { 0 };
    float center;
    float scale;
    int sum;
    int half;

    sum = far + near;
    half = sum / 2;
    center = half;
    scale = far - (half + sum % 2);
    cmd[1] = (FBITS(center) & 0xFFFFFF) | 0x47000000;
    cmd[2] = ADDR(0xD6000000, ALLEGREX_MIN(near, far));
    cmd[0] = (FBITS(scale) & 0xFFFFFF) | 0x44000000;
    cmd[3] = ADDR(0xD7000000, ALLEGREX_MAX(near, far));
    list = dl_push(list, cmd, 4);
    sys_conf.near_z = near;
    sys_conf.far_z = far;
    return list;
}
#endif

#ifdef NON_MATCHING /* register allocation of the float conversions differs */
/* Same range with the center moved by `bias`. */
unsigned int *dl_depth_range_i(unsigned int *list, int bias)
{
    unsigned int cmd[4] = { 0 };
    float center;
    float scale;
    int near = sys_conf.near_z;
    int far = sys_conf.far_z;
    int sum;
    int half;

    sum = far + near;
    half = sum / 2;
    center = half;
    scale = far - (half + sum % 2);
    cmd[2] = ADDR(0xD6000000, ALLEGREX_MIN(near, far));
    cmd[0] = (FBITS(scale) & 0xFFFFFF) | 0x44000000;
    center += bias;
    cmd[3] = ADDR(0xD7000000, ALLEGREX_MAX(near, far));
    cmd[1] = (FBITS(center) & 0xFFFFFF) | 0x47000000;
    return dl_push(list, cmd, 4);
}
#endif

unsigned int *dl_viewport(unsigned int *list, int cx, int cy, int w, int h)
{
    float x = cx;
    float y = cy;
    float sx = w / 2;
    float sy = -h / 2;
    unsigned int cmd[4] = { 0 };

    cmd[0] = FBITS(sx) | 0x42000000;
    cmd[1] = FBITS(sy) | 0x43000000;
    cmd[2] = FBITS(x) | 0x45000000;
    cmd[3] = FBITS(y) | 0x46000000;
    return dl_push(list, cmd, 4);
}

unsigned int *dl_offset(unsigned int *list, int x, int y)
{
    unsigned int cmd[2] = { 0 };

    cmd[0] = x << 4 | 0x4C000000;
    cmd[1] = y << 4 | 0x4D000000;
    return dl_push(list, cmd, 2);
}

/* Material ambient (with alpha), diffuse and specular from one color. */
unsigned int *dl_color(unsigned int *list, unsigned int color)
{
    unsigned int cmd[4] = { 0 };
    unsigned int rgb = color & 0xFFFFFF;

    cmd[0] = rgb | 0x55000000;
    cmd[1] = color >> 24 | 0x58000000;
    cmd[2] = rgb | 0x56000000;
    cmd[3] = rgb | 0x57000000;
    return dl_push(list, cmd, 4);
}

/* Light `n`: position/direction and type (sceGuLight). */
unsigned int *dl_light(unsigned int *list, int n, int type, int comp, float *pos)
{
    unsigned int cmd[4] = { 0 };
    unsigned int *u = (unsigned int *)pos;
    int k = n * 3;
    unsigned int c;

    cmd[0] = u[0] >> 8 | (k + 0x63) << 24;
    cmd[1] = u[1] >> 8 | (k + 0x64) << 24;
    cmd[2] = u[2] >> 8 | (k + 0x65) << 24;
    cmd[3] = (n + 0x5F) << 24;
    switch (comp) {
    case 2:
        c = 0;
        break;
    case 6:
        c = 1;
        break;
    case 8:
        c = 2;
        break;
    }
    cmd[3] |= type << 8 | c;
    return dl_push(list, cmd, 4);
}

/* Light `n` ambient (1), diffuse (2) or specular (4) color (sceGuLightColor). */
unsigned int *dl_light_color(unsigned int *list, int n, int comp, unsigned int color)
{
    unsigned int cmd[1] = { 0 };
    int op;

    op = n * 3 + 0x8F;
    cmd[0] = color & 0xFFFFFF;
    switch (comp) {
    case 1:
        break;
    case 2:
        op += 1;
        break;
    case 4:
        op += 2;
        break;
    }
    cmd[0] |= op << 24;
    return dl_push(list, cmd, 1);
}

/* Light `n` spot direction, exponent and cutoff (sceGuLightSpot). */
unsigned int *dl_light_spot(float exponent, float cutoff, unsigned int *list, int n, float *dir)
{
    unsigned int cmd[5] = { 0 };
    unsigned int *u = (unsigned int *)dir;

    cmd[0] = FBITS(exponent);
    cmd[1] = FBITS(cutoff);
    cmd[2] = u[0] >> 8;
    cmd[3] = u[1] >> 8;
    cmd[4] = u[2] >> 8;
    switch (n) {
    case 0:
        cmd[0] |= 0x87000000;
        cmd[1] |= 0x8B000000;
        cmd[2] |= 0x6F000000;
        cmd[3] |= 0x70000000;
        cmd[4] |= 0x71000000;
        break;
    case 1:
        cmd[0] |= 0x88000000;
        cmd[1] |= 0x8C000000;
        cmd[2] |= 0x72000000;
        cmd[3] |= 0x73000000;
        cmd[4] |= 0x74000000;
        break;
    case 2:
        cmd[0] |= 0x89000000;
        cmd[1] |= 0x8D000000;
        cmd[2] |= 0x75000000;
        cmd[3] |= 0x76000000;
        cmd[4] |= 0x77000000;
        break;
    case 3:
        cmd[0] |= 0x8A000000;
        cmd[1] |= 0x8E000000;
        cmd[2] |= 0x78000000;
        cmd[3] |= 0x79000000;
        cmd[4] |= 0x7A000000;
        break;
    }
    return dl_push(list, cmd, 5);
}

/* Light `n` attenuation (sceGuLightAtt). */
unsigned int *dl_light_att(float a, float b, float c, unsigned int *list, int n)
{
    unsigned int cmd[3] = { 0 };

    n *= 3;
    cmd[0] = FBITS(a) | (n + 0x7B) << 24;
    cmd[1] = FBITS(b) | (n + 0x7C) << 24;
    cmd[2] = FBITS(c) | (n + 0x7D) << 24;
    return dl_push(list, cmd, 3);
}

unsigned int *dl_light_mode(unsigned int *list, int mode)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = mode | 0x5E000000;
    return dl_push(list, cmd, 1);
}

unsigned int *dl_ambient_light(unsigned int *list, unsigned int color)
{
    unsigned int cmd[2] = { 0 };

    cmd[0] = color & 0xFFFFFF | 0x5C000000;
    cmd[1] = color >> 24 | 0x5D000000;
    return dl_push(list, cmd, 2);
}

unsigned int *dl_ambient_color(unsigned int *list, unsigned int color)
{
    unsigned int cmd[2] = { 0 };

    cmd[0] = color & 0xFFFFFF | 0x55000000;
    cmd[1] = color >> 24 | 0x58000000;
    return dl_push(list, cmd, 2);
}

unsigned int *dl_specular_power(float p, unsigned int *list)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = FBITS(p) | 0x5B000000;
    return dl_push(list, cmd, 1);
}

/* Material ambient (1, with alpha), diffuse (2) or specular (4) color (sceGuMaterial). */
unsigned int *dl_material(unsigned int *list, int comp, unsigned int color)
{
    unsigned int cmd[2] = { 0 };
    int n;
    unsigned int rgb;

    rgb = color & 0xFFFFFF;
    cmd[1] = color >> 24 | 0x58000000;
    cmd[0] = rgb;
    n = 1;
    switch (comp) {
    case 1:
        n = 2;
        cmd[0] = rgb | 0x55000000;
        break;
    case 2:
        cmd[0] = rgb | 0x56000000;
        break;
    case 4:
        cmd[0] = rgb | 0x57000000;
        break;
    }
    return dl_push(list, cmd, n);
}

unsigned int *dl_material_colors(unsigned int *list, unsigned int emissive, unsigned int ambient, unsigned int diffuse, unsigned int specular)
{
    unsigned int cmd[4] = { 0 };

    cmd[0] = emissive & 0xFFFFFF | 0x54000000;
    cmd[1] = diffuse & 0xFFFFFF | 0x56000000;
    cmd[2] = ambient & 0xFFFFFF | 0x55000000;
    cmd[3] = specular & 0xFFFFFF | 0x57000000;
    return dl_push(list, cmd, 4);
}

unsigned int *dl_color_material(unsigned int *list, int comps)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = comps | 0x53000000;
    return dl_push(list, cmd, 1);
}

unsigned int *dl_shade_model(unsigned int *list, int mode)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = mode | 0x50000000;
    return dl_push(list, cmd, 1);
}

/* Front face (sceGuFrontFace: 0x9B gets 1 for counter-clockwise). */
unsigned int *dl_front_face(unsigned int *list, int cw)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = !cw | 0x9B000000;
    return dl_push(list, cmd, 1);
}

#ifdef NON_MATCHING /* the original builds the level 0 words first and moves them to level `mip`; scheduling differs */
/* Texture image of mip level `mip`: address, buffer width, log2 of the size, TFLUSH. */
unsigned int *dl_tex_image(unsigned int *list, int mip, int w, int h, int tbw, void *addr)
{
    unsigned int cmd[4] = { 0 };
    int lw;
    int lh;

    for (lw = 0; lw < 10; lw++) {
        if (w == 1 << lw) {
            break;
        }
    }
    for (lh = 0; lh < 10; lh++) {
        if (h == 1 << lh) {
            break;
        }
    }
    cmd[0] = (unsigned int)addr & 0xFFFFFF | 0xA0000000;
    cmd[1] = ((unsigned int)addr >> 24 & 0xFF) << 16 | 0xA8000000 | tbw;
    cmd[2] = lh << 8 | 0xB8000000 | lw;
    cmd[3] = 0xCB000000;
    cmd[0] = cmd[0] & 0xFFFFFF | (mip + 0xA0) << 24;
    cmd[1] = cmd[1] & 0xFFFFFF | (mip + 0xA8) << 24;
    cmd[2] = cmd[2] & 0xFFFFFF | (mip + 0xB8) << 24;
    return dl_push(list, cmd, 4);
}
#endif

unsigned int *dl_tex_mode(unsigned int *list, int psm, int maxmips, int a2, int swizzle)
{
    unsigned int cmd[3] = { 0, 0, 0xCB000000 };

    cmd[0] = swizzle | (maxmips << 16 | 0xC2000000 | a2 << 8);
    cmd[1] = psm | 0xC3000000;
    list = dl_push(list, cmd, 3);
    sys_conf.tex_psm = psm;
    return list;
}

unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = mag << 8 | 0xC6000000 | min;
    return dl_push(list, cmd, 1);
}

unsigned int *dl_tex_func(unsigned int *list, int func, int comp)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = func | (sys_conf.tfunc_double << 16 | 0xC9000000 | comp << 8);
    list = dl_push(list, cmd, 1);
    sys_conf.tfunc = func;
    sys_conf.tfunc_comp = comp;
    return list;
}

unsigned int *dl_tex_wrap(unsigned int *list, int u, int v)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = v << 8 | 0xC7000000 | u;
    return dl_push(list, cmd, 1);
}

/* Mipmap level mode and bias (sceGuTexLevelMode). */
unsigned int *dl_tex_level_mode(float bias, unsigned int *list, int mode)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = (int)(bias * 16.0f) << 16 | 0xC8000000 | mode;
    return dl_push(list, cmd, 1);
}

/* Texture LOD slope (sceGuTexSlope). */
unsigned int *dl_tex_slope(float f, unsigned int *list)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = FBITS(f) | 0xD0000000;
    return dl_push(list, cmd, 1);
}

unsigned int *dl_tex_map_mode(unsigned int *list, int mode, int a, int b)
{
    unsigned int cmd[2] = { 0 };

    cmd[0] = sys_conf.tex_proj << 8 | 0xC0000000 | mode;
    cmd[1] = b << 8 | 0xC1000000 | a;
    list = dl_push(list, cmd, 2);
    sys_conf.tex_map = mode;
    return list;
}

unsigned int *dl_tex_proj_map_mode(unsigned int *list, int mode)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = mode << 8 | 0xC0000000 | sys_conf.tex_map;
    list = dl_push(list, cmd, 1);
    sys_conf.tex_proj = mode;
    return list;
}

unsigned int *dl_tex_offset(float u, float v, unsigned int *list)
{
    unsigned int cmd[2] = { 0 };

    cmd[0] = FBITS(u) | 0x4A000000;
    cmd[1] = FBITS(v) | 0x4B000000;
    return dl_push(list, cmd, 2);
}

unsigned int *dl_tex_scale(float u, float v, unsigned int *list)
{
    unsigned int cmd[2] = { 0 };

    cmd[0] = FBITS(u) | 0x48000000;
    cmd[1] = FBITS(v) | 0x49000000;
    return dl_push(list, cmd, 2);
}

unsigned int *dl_tex_env_color(unsigned int *list, unsigned int slope)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = slope & 0xFFFFFF | 0xCA000000;
    return dl_push(list, cmd, 1);
}

/* TSYNC */
unsigned int *dl_tex_sync(unsigned int *list)
{
    unsigned int cmd[1] = { 0xCC000000 };

    return dl_push(list, cmd, 1);
}

/* TFLUSH */
unsigned int *dl_tex_flush(unsigned int *list)
{
    unsigned int cmd[1] = { 0xCB000000 };

    return dl_push(list, cmd, 1);
}

/* CLUT address and load of `n` blocks of 8 entries (sceGuClutLoad). */
unsigned int *dl_clut_load(unsigned int *list, int n, void *clut)
{
    unsigned int cmd[3] = { 0 };

    cmd[0] = ADDR(0xB0000000, clut);
    cmd[1] = ((unsigned int)clut >> 24 & 0xFF) << 16 | 0xB1000000;
    cmd[2] = n | 0xC4000000;
    return dl_push(list, cmd, 3);
}

unsigned int *dl_clut_mode(unsigned int *list, int psm, int shift, int mask, int start)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = psm | (shift << 2 | (start << 16 | 0xC5000000 | mask << 8));
    return dl_push(list, cmd, 1);
}

/* Fog (sceGuFog): far, 1 / (far - near), color. */
unsigned int *dl_fog(float near, float far, unsigned int *list, unsigned int color)
{
    unsigned int cmd[3] = { 0 };
    float d;

    d = 1.0f / (far - near);
    cmd[0] = color & 0xFFFFFF | 0xCF000000;
    cmd[1] = FBITS(far) | 0xCD000000;
    cmd[2] = FBITS(d) | 0xCE000000;
    return dl_push(list, cmd, 3);
}

/* Scissor and region; only emitted while the scissor test is on (dl_enable 2 applies it). */
unsigned int *dl_scissor(unsigned int *list, int x, int y, int w, int h)
{
    unsigned int cmd[4] = { 0 };
    unsigned int y1;
    unsigned int x1;

    cmd[0] = y << 10 | 0xD4000000 | x;
    x1 = x + w - 1;
    y1 = (y + h - 1) << 10;
    cmd[1] = y1 | 0xD5000000 | x1;
    cmd[2] = y << 10 | 0x15000000 | x;
    cmd[3] = y1 | 0x16000000 | x1;
    sys_conf.scissor_x = x;
    sys_conf.scissor_y = y;
    sys_conf.scissor_w = w;
    sys_conf.scissor_h = h;
    if (sys_conf.scissor_on != 0) {
        list = dl_push(list, cmd, 4);
    }
    return list;
}

/* Color test function, reference and mask (sceGuColorFunc). */
unsigned int *dl_color_func(unsigned int *list, int func, unsigned int color, unsigned int mask)
{
    unsigned int cmd[3] = { 0 };

    cmd[0] = func | 0xD8000000;
    cmd[1] = color & 0xFFFFFF | 0xD9000000;
    cmd[2] = mask & 0xFFFFFF | 0xDA000000;
    return dl_push(list, cmd, 3);
}

unsigned int *dl_alpha_test(unsigned int *list, int func, int ref, int mask)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = func | (mask << 16 | 0xDB000000 | ref << 8);
    return dl_push(list, cmd, 1);
}

unsigned int *dl_stencil_op(unsigned int *list, int fail, int zfail, int zpass)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = fail | (zpass << 16 | 0xDC000000 | zfail << 8);
    return dl_push(list, cmd, 1);
}

unsigned int *dl_stencil_func(unsigned int *list, int func, int ref, int mask)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = func | (mask << 16 | 0xDD000000 | ref << 8);
    return dl_push(list, cmd, 1);
}

unsigned int *dl_depth_func(unsigned int *list, int func)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = func | 0xDE000000;
    return dl_push(list, cmd, 1);
}

unsigned int *dl_depth_mask(unsigned int *list, int mask)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = mask | 0xE7000000;
    return dl_push(list, cmd, 1);
}

unsigned int *dl_blend_func(unsigned int *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix)
{
    unsigned int cmd[3] = { 0 };

    cmd[0] = src | (op << 8 | 0xDF000000 | dst << 4);
    cmd[1] = srcfix & 0xFFFFFF | 0xE0000000;
    cmd[2] = dstfix & 0xFFFFFF | 0xE1000000;
    return dl_push(list, cmd, 3);
}

/* 4x4 dither matrix (sceGuSetDither). */
unsigned int *dl_dither(unsigned int *list, int *m)
{
    unsigned int cmd[4] = { 0 };

    cmd[0] = m[0] & 0xF | ((m[1] & 0xF) << 4 | ((m[3] & 0xF) << 12 | 0xE2000000 | (m[2] & 0xF) << 8));
    cmd[1] = m[4] & 0xF | ((m[5] & 0xF) << 4 | ((m[7] & 0xF) << 12 | 0xE3000000 | (m[6] & 0xF) << 8));
    cmd[2] = m[8] & 0xF | ((m[9] & 0xF) << 4 | ((m[11] & 0xF) << 12 | 0xE4000000 | (m[10] & 0xF) << 8));
    cmd[3] = m[12] & 0xF | ((m[13] & 0xF) << 4 | ((m[15] & 0xF) << 12 | 0xE5000000 | (m[14] & 0xF) << 8));
    return dl_push(list, cmd, 4);
}

unsigned int *dl_logic_op(unsigned int *list, int op)
{
    unsigned int cmd[1] = { 0 };

    cmd[0] = op | 0xE6000000;
    return dl_push(list, cmd, 1);
}

unsigned int *dl_pixel_mask(unsigned int *list, unsigned int mask)
{
    unsigned int cmd[2] = { 0 };

    cmd[0] = mask & 0xFFFFFF | 0xE8000000;
    cmd[1] = mask >> 24 | 0xE9000000;
    return dl_push(list, cmd, 2);
}

unsigned int *dl_clear_color(unsigned int *list, unsigned int color)
{
    sys_conf.clear_color = color;
    return list;
}

unsigned int *dl_clear_stencil(unsigned int *list, unsigned int s)
{
    sys_conf.clear_stencil = s;
    return list;
}

unsigned int *dl_clear_depth(unsigned int *list, unsigned int z)
{
    sys_conf.clear_depth = z;
    return list;
}

#ifdef NON_MATCHING /* loop strength reduction and register allocation differ */
/* Clears with sprites in clear mode (sceGuClear): one full-screen sprite, or 8 strips of 64
   pixels with the fast flag (0x10). */
unsigned int *dl_clear(unsigned int *list, unsigned int flags)
{
    SysConf *c = &sys_conf;
    int nv = 2;
    unsigned int cmd[64];
    unsigned int *p;
    unsigned int *v;
    unsigned int col;
    unsigned int st;
    unsigned int a;
    int x;
    int i;

    for (i = 0; i < 64; i++) {
        cmd[i] = 0;
    }
    if (flags & 0x10) {
        nv = 0x10;
    }
    a = (unsigned int)(list + nv * 3) + 8;
    cmd[0] = a >> 8 & 0xFF0000 | 0x10000000;
    cmd[1] = a & 0xFFFFFF | 0x08000000;
    if (flags & 0x10) {
        col = c->clear_depth << 24 | c->clear_color & 0xFFFFFF;
        st = (unsigned short)c->clear_stencil;
        v = cmd;
        x = 0;
        for (i = 0; i < 8; ) {
            v[2] = col;
            v[3] = x;
            i++;
            v[4] = st;
            v[5] = col;
            v[6] = i << 6 | 0x01100000;
            v[7] = st;
            v += 6;
            x += 0x40;
        }
    } else {
        cmd[4] = (unsigned short)c->clear_stencil;
        cmd[7] = cmd[4];
        cmd[2] = c->clear_depth << 24 | c->clear_color & 0xFFFFFF;
        cmd[3] = 0;
        cmd[5] = cmd[2];
        cmd[6] = 0x011001E0;
    }
    p = cmd + nv * 3;
    p[2] = (flags & 7) << 8 | 0xD3000001;
    p[3] = 0x1280011C;
    p[4] = (unsigned int)(list + 2) >> 8 & 0xFF0000 | 0x10000000;
    p[5] = (unsigned int)(list + 2) & 0xFFFFFF | 0x01000000;
    p[6] = nv | 0x04060000;
    p[7] = 0xD3000000;
    return dl_push(list, cmd, nv * 3 + 8);
}
#endif

/* Block transfer (sceGuCopyImage). */
unsigned int *dl_copy_image(unsigned int *list, int psm, int sx, int sy, int w, int h, int srcw, void *src, int dx, int dy, int dstw, void *dst)
{
    unsigned int cmd[8] = { 0 };

    cmd[0] = ADDR(0xB2000000, src);
    cmd[1] = ((unsigned int)src >> 24 & 0xFF) << 16 | 0xB3000000 | srcw;
    cmd[2] = sy << 10 | 0xEB000000 | sx;
    cmd[3] = ADDR(0xB4000000, dst);
    cmd[4] = ((unsigned int)dst >> 24 & 0xFF) << 16 | 0xB5000000 | dstw;
    cmd[5] = dy << 10 | 0xEC000000 | dx;
    cmd[6] = (h - 1) << 10 | 0xEE000000 | w - 1;
    cmd[7] = (psm == 3) | 0xEA000000;
    return dl_push(list, cmd, 8);
}
