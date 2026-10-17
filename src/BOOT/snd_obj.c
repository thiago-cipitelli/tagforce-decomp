// @range 0x088615D0-0x08862C5C
// @data 0x088F0428-0x088F04AC
// @asm 0x08861CA0-0x08861D60
// @asm 0x08861F88-0x088624AC
/* Sound library, object side (continued): handles of the object pool, lookup by sound id, start
   order of sounds of a class, the 4 slots of the current sound, group data loading through the
   read/alloc callbacks or the VPL, the 3D model (distance curve, pan from the angle, Doppler pitch),
   faders, and the final SAS volumes of an object. */
#include "psp.h"
#include "libc.h"
#include "allegrex.h"
#include "snd.h"

typedef struct SndGroup {              /* 0x10 per group */
    int data;                          /* loaded data or 0 */
    void *buf;                         /* +0x04 buffer the data is read into */
    int unk8;
    unsigned char state;               /* +0x0C 1 failed, 8 unload when read, 0x20 partial, 0x40 reading, 0x80 loaded */
    unsigned char pad[3];
} SndGroup;

typedef struct SndHandle {
    int data;
    int key;
    unsigned short id;
} SndHandle;

typedef struct SndBankHdr {
    unsigned char pad[0xC];
    unsigned short n_groups_a;         /* +0x0C */
} SndBankHdr;

typedef struct SndCurve {              /* distance attenuation, 0x1C */
    float near;                        /* full volume up to here */
    float mid;
    float far;                         /* silent from here */
    unsigned short vol_mid;            /* +0x0C volume at `mid` */
    unsigned short vol_far;            /* +0x0E volume past `far` */
    int pad[3];
} SndCurve;

typedef struct Snd3D {
    SndCurve *curves;                  /* per-sound curves (snd_desc_b7 picks one), or 0 */
    SndCurve def;                      /* +0x04 */
    float doppler_speed;               /* +0x20 0 disables Doppler */
    float near_dist;                   /* +0x24 inside it the pan swings towards the centre */
    int n_curves;                      /* +0x28 */
} Snd3D;

extern unsigned char snd_obj_pool[];   /* 0x08C1B1D8, 0x118 per object */
extern SndList snd_active;             /* 0x08C1B004 */
extern SndFader snd_bgm_master;        /* 0x08C1B010 */
extern short snd_cur_slot;             /* 0x08C1B030 */
extern SndObj *snd_slot_tab[4];        /* 0x08C1B034 */
extern int snd_bank_sema;              /* 0x08C1B04C */
extern float snd_view_mtx[16];         /* 0x08C1D710 */
extern Snd3D snd_3d;                   /* 0x08C1D750 */
extern SndBankHdr *snd_bank;           /* 0x08C1D88C */
extern short snd_fx_wet_r;             /* 0x08C1D896 */
extern short snd_fx_wet_l;             /* 0x08C1D898 */
extern unsigned short snd_cfg_flags;  /* 0x08C1D894 0x2000: reads take an offset */
extern int snd_vpl;                    /* 0x08C1D89C */
extern unsigned int snd_n_groups;      /* 0x08C1D8AC */
extern int snd_data_base;              /* 0x08C1D8B0 */
extern int (*snd_cb_read)(void *name, void *buf, int size, int off);  /* 0x08C1D8B8 */
extern int (*snd_cb_free)(void *p);    /* 0x08C1D8BC */
extern void *(*snd_cb_alloc)(int size, int align, int tag);           /* 0x08C1D8C0 */

/* Pan law: share of the right channel (0..0x80) for an angle 0..0x80. */
unsigned char snd_pan_table[0x81] = {
    0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11, 0x13, 0x14, 0x16, 0x17, 0x19, 0x1A, 0x1C, 0x1D, 0x1F,
    0x20, 0x21, 0x23, 0x24, 0x26, 0x27, 0x29, 0x2A, 0x2C, 0x2D, 0x2F, 0x30, 0x31, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3D, 0x3E, 0x3E, 0x3F, 0x3F, 0x3F, 0x40, 0x40,
    0x40, 0x40, 0x41, 0x41, 0x41, 0x42, 0x42, 0x43, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A,
    0x4B, 0x4C, 0x4D, 0x4F, 0x50, 0x51, 0x53, 0x54, 0x56, 0x57, 0x59, 0x5A, 0x5C, 0x5D, 0x5F, 0x60,
    0x61, 0x63, 0x64, 0x66, 0x67, 0x69, 0x6A, 0x6C, 0x6D, 0x6F, 0x70, 0x71, 0x73, 0x74, 0x75, 0x76,
    0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7D, 0x7E, 0x7E, 0x7F, 0x7F, 0x7F, 0x80, 0x80, 0x80,
    0x80,
};

int snd_entry_kind(unsigned int id);
int snd_entry_flags4(unsigned int id, int mask);
int snd_entry_flags5(unsigned int id, int mask);
int snd_entry_b5(unsigned int id);
int snd_desc_b7(unsigned int id);
int snd_obj_exists(int handle);
SndGroup *snd_group_get(unsigned int g);
int snd_group_data(unsigned int g);
int snd_group_unk8(unsigned int g);
unsigned int snd_bank_offset(unsigned int g);
SndHandle *snd_handle_get(int id, void *key, void *data);
SndHandle *snd_handle_find(void *data);
void snd_handle_release(SndHandle *h);
void snd_group_stop(unsigned int g, int x);
void mat4_transform_vec4(float *out, float *m, float *v);
float sqrtf_vfpu(float x);
float atan2f_vfpu(float y, float x);
void vec3_normalize2(float *out, float *v);
float vec3_dot2(float *a, float *b);
void snd_fader_init(SndFader *f, short v);
void snd_fader8_init(SndFader8 *f, signed char v);
void snd_fader_set(SndFader *f, short value, int time);
void snd_obj_fade(SndObj *o, int which, short value, int time);
int snd_group_unload(unsigned int g, int free);
void snd_group_read_done(void *arg, int result);
void *snd_alloc(int size, int align, int tag);
int snd_free(void *p);
int snd_read(void *name, void *buf, int size, int off);
void snd_group_loaded(unsigned int g, int data);
int snd_curve_volume(float dist, SndCurve *c);

/* Object of a handle (bit 29, index << 16 | sound id), 0 if the slot was reused. */
SndObj *snd_obj_from_handle(unsigned int h)
{
    unsigned int i;
    SndObj *o;

    if (!(h & 0x20000000)) {
        return 0;
    }
    i = (h >> 16) & 0xFF;
    o = (SndObj *)(snd_obj_pool + i * 0x118);
    if (o->index >= 0 && o->index == i && *(unsigned short *)&o->id == (h & 0xFFFF)) {
        return o;
    }
    return 0;
}

/* Same, but a free slot still yields its object. */
SndObj *snd_obj_slot_from_handle(unsigned int h)
{
    unsigned int i;
    SndObj *o;

    if (!(h & 0x20000000)) {
        return 0;
    }
    i = (h >> 16) & 0xFF;
    o = (SndObj *)(snd_obj_pool + i * 0x118);
    if (o->index >= 0 && (o->index != i || *(unsigned short *)&o->id != (h & 0xFFFF))) {
        o = 0;
    }
    return o;
}

/* Sound id -> id with bit 28, the kind in bits 16-19, 0x100000 for sounds past the groups and
   0x200000 for flag 0x20. */
unsigned int snd_id_handle(unsigned int id)
{
    if (!(id & 0x10000000)) {
        id = id | snd_entry_kind(id) << 16 | 0x10000000;
        if (id >= snd_n_groups) {
            id |= 0x100000;
        }
        if (snd_entry_flags4(id, 0x20)) {
            id |= 0x200000;
        }
    }
    return id;
}

/* Object of a handle, a sound id (the earliest started one) or an object pointer. */
SndObj *snd_obj_find(unsigned int h)
{
    SndObj *o;
    SndObj *best;
    unsigned int id;

    if (h & 0x20000000) {
        return snd_obj_from_handle(h);
    }
    if (!(h & 0x10000000)) {
        if (h >= 0x8000000) {
            return (SndObj *)h;
        }
        h = snd_id_handle(h);
    }
    id = h & 0xFFFF;
    best = 0;
    o = 0;
    for (;;) {
        o = snd_list_next(&snd_active, o);
        if (o == 0) {
            break;
        }
        if (o->sid == id) {
            if (best == 0 || o->order < best->order) {
                best = o;
            }
        }
    }
    return best;
}

/* Takes `o` out of the start order of its class (SE or not) and, with `add`, puts it last. */
void snd_obj_order(SndObj *o, int add)
{
    unsigned int cls;
    int ord;
    int n;
    SndObj *p;

    if (o == 0) {
        return;
    }
    cls = o->id & 0x100000;
    ord = o->order;
    if (ord < 0) {
        ord = 0xFF;
    }
    n = 0;
    p = 0;
    for (;;) {
        p = snd_list_next(&snd_active, p);
        if (p == 0) {
            break;
        }
        if ((p->id & 0x100000) == cls) {
            n++;
            if (p->order >= ord) {
                p->order--;
            }
        }
    }
    if (!add) {
        o->order = -1;
    } else {
        o->order = n;
    }
}

/* Drops finished sounds from the 4 slots and makes the latest started one current. */
void snd_slots_update(void)
{
    short *cur = &snd_cur_slot;
    int best;
    int top;
    int i;
    short *p;
    SndObj *o;

    best = top = -1;
    for (i = 0, p = cur; i < 4; i++, p += 2) {
        o = *(SndObj **)(p + 2);
        if (o != 0 && !(o->state & 0x50)) {
            if (snd_obj_exists((int)o) == 0) {
                *(SndObj **)(p + 2) = 0;
                if (*cur == i) {
                    *cur = -1;
                }
            } else if (o->order > top) {
                best = i;
                top = o->order;
            }
        }
    }
    if (best >= 0) {
        *cur = best;
    }
}

void snd_group_loaded(unsigned int g, int data)
{
    SndGroup *gr = snd_group_get(g);

    if (gr != 0 && gr->data == 0 && (gr->state & 0x40)) {
        gr->data = data;
        if (data != 0) {
            gr->state = 0x80;
        }
    }
}

/* Starts reading group `g` into `buf` (or a buffer of its own); `name` identifies the request to
   the read callback, which ends in snd_group_read_done. Returns the buffer or 0. */
void *snd_group_load(unsigned int g, void *buf, void *name, unsigned int limit)
{
    SndGroup *gr;
    void *data;
    unsigned char st;
    unsigned int size;
    int off;

    if (snd_bank == 0) {
        return 0;
    }
    gr = snd_group_get(g);
    if (gr == 0) {
        return 0;
    }
    sceKernelWaitSema(snd_bank_sema, 1, 0);
    if ((data = (void *)snd_group_data(g)) != 0) {
        buf = data;
        goto done;
    }
    if (buf == 0) {
        buf = gr->buf;
        if (buf == 0) {
            buf = snd_alloc(snd_group_unk8(g), 4, g);
            if (buf == 0) {
                goto done;
            }
        }
    }
    st = gr->state;
    if (!(st & ~1)) {
        if (snd_handle_get(g, buf, name) == 0) {
        fail:
            if (buf != 0) {
                if (gr->buf == 0) {
                    snd_free(buf);
                }
            }
            buf = 0;
        } else {
            gr->buf = buf;
            gr->state = 0x40;
            size = snd_group_unk8(g);
            off = snd_data_base + snd_bank_offset(g);
            if (limit != 0 && limit < size) {
                gr->state |= 0x20;
            }
            if (snd_read(name, buf, size, off) != 0) {
                goto done;
            }
            snd_group_read_done(name, -1);
            goto fail;
        }
    } else if (st & 8) {
        gr->state = st & 0xF7;
    }
done:
    sceKernelSignalSema(snd_bank_sema, 1);
    return buf;
}

int snd_group_unload(unsigned int g, int free)
{
    int r = 0;
    SndGroup *gr = snd_group_get(g);
    void *buf;

    if (gr != 0) {
        buf = gr->buf;
        if (free) {
            if (buf != 0) {
                if (gr->state & 0x40) {
                    gr->state |= 8;
                    goto done;
                }
                snd_free(buf);
            } else {
                gr->state = 0;
                goto clear;
            }
        }
        gr->state = 0;
    clear:
        gr->buf = 0;
        gr->data = 0;
    done:
        r = 1;
    }
    return r;
}

int snd_group_release(unsigned int g, int free)
{
    int r;

    snd_group_stop(g, 0);
    sceKernelWaitSema(snd_bank_sema, 1, 0);
    r = snd_group_unload(g, free);
    sceKernelSignalSema(snd_bank_sema, 1);
    return r;
}

/* -0x107 groups of the second set, -0x106 the first set, -0x105 all. */
#ifdef NON_MATCHING /* the loop counter and `free` land in each other's s registers */
int snd_group_release_all(int which, int free)
{
    unsigned int n;
    unsigned int i;

    switch (which) {
    case -0x105:
        n = snd_n_groups;
        i = 0;
        break;
    case -0x106:
        n = snd_bank->n_groups_a;
        i = 0;
        break;
    case -0x107:
        i = snd_bank->n_groups_a;
        n = snd_n_groups;
        break;
    default:
        return 0;
    }
    for (; (int)i < (int)n; i++) {
        snd_group_release(i, free);
    }
    return 1;
}
#endif

/* Read callback result: 0 done, -1 failed to start, -2 failed. */
void snd_group_read_done(void *name, int result)
{
    SndHandle *h;
    SndGroup *gr;
    int data;
    int unload;

    sceKernelWaitSema(snd_bank_sema, 1, 0);
    h = snd_handle_find(name);
    if (h != 0) {
        gr = snd_group_get(h->id);
        data = 0;
        switch (result) {
        case -2:
        case -1:
            goto shared;
        case 0:
            data = h->key;
        shared:
            unload = gr->state & 8;
            break;
        default:
            goto out;
        }
        snd_group_loaded(h->id, data);
        snd_handle_release(h);
        if (gr != 0) {
            if (unload && data != 0) {
                snd_group_unload(h->id, 1);
            }
            if (result != 0) {
                gr->state = 1;
            }
        }
    }
out:
    sceKernelSignalSema(snd_bank_sema, 1);
}

void *snd_alloc(int size, int align, int tag)
{
    void *p;

    if (snd_vpl >= 0) {
        if (sceKernelTryAllocateVpl(snd_vpl, size, &p) != 0) {
            return 0;
        }
        return p;
    }
    if (snd_cb_alloc == 0) {
        return 0;
    }
    return snd_cb_alloc(size, align, tag);
}

int snd_free(void *p)
{
    int r;

    if (snd_vpl >= 0) {
        r = sceKernelFreeVpl(snd_vpl, p) == 0;
    } else if (snd_cb_free == 0) {
        r = 0;
    } else {
        r = snd_cb_free(p);
    }
    return r;
}

int snd_read(void *name, void *buf, int size, int off)
{
    if (snd_cb_read != 0) {
        if (off < 0) {
            off = 0;
        }
        if (!(snd_cfg_flags & 0x2000)) {
            off = -1;
        }
        if (snd_cb_read(name, buf, size, off) != 0) {
            return 1;
        }
    }
    return 0;
}

/* Volume (0x1000 = full) at distance `dist`: full up to near, linear to vol_mid at mid, then to
   vol_far at far. */
#ifdef NON_MATCHING /* the original loads vol_far before the compare instead of in its delay slot */
int snd_curve_volume(float dist, SndCurve *c)
{
    int v = 0x1000;
    int base;
    int d;

    if (dist > c->near) {
        base = c->vol_far;
        v = base;
        if (dist < c->far) {
            if (dist < c->mid) {
                base = c->vol_mid;
                d = (0x1000 - base) * (c->mid - dist) / (c->mid - c->near);
                if (d < 0) {
                    d = -d;
                }
                v = d + base;
            } else if (c->mid != c->far) {
                d = (c->vol_mid - base) * (c->far - dist) / (c->far - c->mid);
                if (d < 0) {
                    d = -d;
                }
                v = d + base;
            }
        }
    }
    return v;
}
#endif

/* 3D: distance volume (fader 5), pan from the angle (6) and Doppler pitch (7) of a positioned sound. */
#ifdef NON_MATCHING /* the SndPos pointer lands in a2 instead of a1 and the Doppler product swaps its operands */
void snd_obj_3d(SndObj *o)
{
    Snd3D *g = &snd_3d;
    SndCurve *c;
    int pitch;
    unsigned int flags;
    unsigned short pf;
    SndPos *p;
    float *pv;
    int vol;
    int k;
    int pan;
    float dist;
    float ang;
    float near;
    float sp;
    float in[4];
    float d[4];
    float v[4];
    float dir[4];
    float n[4];

    c = &snd_3d.def;
    if (g->curves != 0) {
        c = g->curves;
        k = snd_desc_b7(o->id);
        if (k > 0) {
            if (k >= g->n_curves) {
                k = 0;
            }
            c += k;
        }
    }
    flags = snd_entry_b5(o->id);
    d[3] = 0.0f;
    pf = o->p3d.flags;
    p = &o->p3d;
    vol = -1;
    pitch = 0x1000;
    if (pf & 0x40) {
        if (!(pf & 0x80)) {
            p->flags = pf | 0x80;
            vol = 0;
        } else {
            d[0] = p->src[0] - p->pos[0];
            d[1] = p->src[1] - p->pos[1];
            d[2] = p->src[2] - p->pos[2];
            d[3] = 1.0f;
        }
        p->pos[0] = p->src[0];
        p->pos[1] = p->src[1];
        p->pos[2] = p->src[2];
    }
    if (p->flags & 0x20) {
        pv = v;
        v[0] = p->pos[0];
        v[1] = p->pos[1];
        v[2] = p->pos[2];
        v[3] = 1.0f;
    } else {
        pv = v;
        in[0] = p->pos[0];
        in[1] = p->pos[1];
        in[2] = p->pos[2];
        in[3] = 1.0f;
        mat4_transform_vec4(pv, snd_view_mtx, in);
    }
    dist = sqrtf_vfpu(pv[2] * pv[2] + (pv[0] * pv[0] + pv[1] * pv[1]));
    ang = atan2f_vfpu(pv[2], pv[0]);
    if (!(flags & 8) && d[3] != 0.0f && g->doppler_speed != 0.0f) {
        sp = sqrtf_vfpu(d[2] * d[2] + (d[0] * d[0] + d[1] * d[1]));
        sp = sp / g->doppler_speed;
        dir[0] = v[0] / dist;
        dir[1] = v[1] / dist;
        dir[2] = v[2] / dist;
        dir[3] = 1.0f / dist;
        vec3_normalize2(n, d);
        pitch = -(int)(vec3_dot2(dir, n) * 4096.0f * sp);
        if (pitch > 0x1000) {
            pitch = 0x2000;
        } else {
            pitch += 0x1000;
            if (pitch < -1) {
                pitch = 1;
                vol = 0;
            }
        }
    }
    if (vol < 0) {
        vol = snd_curve_volume(dist, c);
    }
    if (ang < 0.0f) {
        ang = -ang;
    }
    near = g->near_dist;
    if (dist < near && ((o->id >> 16) & 0xF) - 1 > 1) {
        ang = ang + (float)(1.5707963705062866 - ang) * (near - dist) / near;
    }
    pan = 128.0 * (3.1415927f - ang) / 3.1415927410125732;
    if (pan >= 0x80) {
        pan = 0x7F;
    }
    if (!(flags & 2)) {
        snd_obj_fade(o, 5, (short)vol, 0);
    }
    if (!(flags & 4)) {
        snd_obj_fade(o, 6, (short)pan, 0);
    }
    if (!(flags & 8)) {
        snd_obj_fade(o, 7, (short)pitch, 0);
    }
}
#endif

void snd_fader_init(SndFader *f, short v)
{
    f->target = v;
    f->cur = v;
    f->frac = 0x80;
    f->count = 0;
    f->step = 0;
}

void snd_fader8_init(SndFader8 *f, signed char v)
{
    f->target = v;
    f->cur = v;
    f->frac = 0x80;
    f->count = 0;
    f->step = 0;
}

/* Ramps to `value` over `time` ms (ticks of 5.8 ms), at once for 0, the default for <0. */
void snd_fader_set(SndFader *f, short value, int time)
{
    if (time != 0) {
        if (time < 0) {
            time = 0x56;
        } else {
            time = time * 10 / 58;
        }
    }
    f->count = time;
    f->target = value;
    f->step = 0;
}

void snd_obj_fade(SndObj *o, int which, short value, int time)
{
    snd_fader_set(&o->fade[which], value, time);
    o->state |= 0x100;
}

/* One tick; 0 when the fader is at rest. */
int snd_fader_update(SndFader *f)
{
    int target;
    int v;
    int step;

    if (f->target != f->cur || f->count != 0) {
        if (f->count == 0) {
            snd_fader_init(f, f->target);
            return 1;
        }
        v = f->cur << 8 | f->frac;
        target = f->target << 8;
        if (f->step == 0) {
            step = (target - v) / f->count;
            if (step == 0) {
                f->step = 1;
            } else {
                f->step = step;
            }
        }
        v += f->step;
        if (--f->count < 0) {
            f->count = 0;
            v = target;
        }
        if (f->step < 0) {
            if (v < target) {
                v = target;
                f->step = 0;
            }
        } else if (v >= target) {
            v = target;
            f->step = 0;
        }
        f->frac = v;
        f->cur = v >> 8 & 0xFFFF;
        return 1;
    }
    return 0;
}

int snd_fader8_update(SndFader8 *f)
{
    int target;
    int v;
    int step;

    if (f->target != f->cur || f->count != 0) {
        if (f->count == 0) {
            snd_fader8_init(f, f->target);
            return 1;
        }
        v = f->cur << 8 | f->frac;
        target = f->target << 8;
        if (f->step == 0) {
            step = (target - v) / f->count;
            if (step == 0) {
                f->step = 1;
            } else {
                f->step = step;
            }
        }
        v += f->step;
        if (--f->count < 0) {
            f->count = 0;
            v = target;
        }
        if (f->step < 0) {
            if (v < target) {
                v = target;
                f->step = 0;
            }
        } else if (v >= target) {
            v = target;
            f->step = 0;
        }
        f->frac = v;
        f->cur = v >> 8 & 0xFFFF;
        return 1;
    }
    return 0;
}

/* SAS pitch of an object: base pitch scaled by fader 7, 1..0x4000. */
int snd_obj_pitch(SndObj *o)
{
    int p = o->volA4 * o->fade[7].cur >> 12;

    if (p < 1) {
        p = 1;
    }
    if (p > 0x4000) {
        p = 0x4000;
    }
    return p;
}

/* Volume of an object through the BGM master (non-SE) and faders 5..0; `sq` squares it. */
int snd_obj_volume(SndObj *o, int sq)
{
    int v = o->volA0 * 0x1021 >> 7;
    int i;
    SndFader *f;

    if (!(o->id & 0x100000)) {
        v = v * snd_bgm_master.cur >> 12;
    }
    i = 5;
    f = &o->fade[5];
    do {
        i--;
        v = v * f->cur >> 12;
        f--;
    } while (i >= 0);
    if (sq) {
        long long x = (long long)v * v;
        v = x >> 12;
        if (v == 0 && x != 0) {
            v = 1;
        }
    }
    return v;
}

/* Squares a 0x1000-based volume (SAS volumes follow the power), keeping it non-zero. */
#define VOL_SQ(v)                                                \
    do {                                                         \
        long long sq_ = (long long)(v) * (v);                    \
        (v) = sq_ >> 12;                                         \
        if ((v) == 0 && sq_ != 0) {                              \
            (v) = 1;                                             \
        }                                                        \
    } while (0)

/* Left/right volumes of an object for pan angle `ang` (0..0x7F), through the object's pan fader. */
void snd_obj_pan_lr(SndObj *o, short *l, short *r, int ang)
{
    int vol;
    int pan;
    int p;
    int t;

    ang = ang * 0x40A0 >> 14;
    pan = o->fade[6].cur * 0x40A0 >> 14;
    vol = o->volA2;
    if (pan >= 64) {
        ang = 0x80 - ang;
    }
    p = snd_pan_table[ang * pan >> 6];
    if (p == 0x40) {
        t = vol;
        VOL_SQ(t);
        *r = t;
        *l = t;
    } else {
        t = vol * (0x80 - p) >> 6;
        if (vol < t) {
            t = vol;
        }
        if (t < -vol) {
            t = -vol;
        }
        VOL_SQ(t);
        *l = t;
        t = vol * p >> 6;
        if (vol < t) {
            t = vol;
        }
        if (t < -vol) {
            t = -vol;
        }
        VOL_SQ(t);
        *r = t;
    }
}

/* Effect send levels of an object; sounds without flag 0x10 add the global wet levels. */
void snd_obj_send_lr(SndObj *o, short *l, short *r)
{
    int dry = snd_entry_flags5(o->id, 0x10);
    int vol;
    int p;
    int t;

    *r = 0;
    *l = 0;
    if (((o->id >> 16) & 0xF) - 1 <= 1) {
        return;
    }
    vol = o->send * o->volA2 >> 12;
    if (vol == 0) {
        return;
    }
    p = snd_pan_table[o->send_pan * 0x40A0 >> 14];
    if (p == 0x40) {
        t = vol;
        VOL_SQ(t);
        *r = t;
        *l = t;
    } else {
        t = vol * (0x80 - p) >> 6;
        if (vol < t) {
            t = vol;
        }
        if (t < -vol) {
            t = -vol;
        }
        VOL_SQ(t);
        if (!dry) {
            t += snd_fx_wet_l;
            if (t < 0) {
                t = 0;
            } else if (t > 0x1000) {
                t = 0x1000;
            }
        }
        *l = t;
        t = vol * p >> 6;
        if (vol < t) {
            t = vol;
        }
        if (t < -vol) {
            t = -vol;
        }
        VOL_SQ(t);
        if (!dry) {
            t += snd_fx_wet_r;
            if (t < 0) {
                t = 0;
            } else if (t > 0x1000) {
                t = 0x1000;
            }
        }
        *r = t;
    }
}
