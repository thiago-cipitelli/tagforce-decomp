// @range 0x0885FAA8-0x08860DA4
// @asm 0x0885FE04-0x088601A4
// @asm 0x08860C88-0x08860DA4
/* Sound library, playback: object reset, attaching to the per-kind lists, starting a sound (random
   or sequential variants, group data loading, SAS voices), the BGM slots (a new BGM suspends the
   current one, which resumes when it ends), voice clips, stopping with fade-out and the release
   callback, BGM ducking, and stopping by group or class. */
#include "psp.h"
#include "libc.h"
#include "snd.h"

typedef struct SndRandHist {           /* last pick of a sound with variants */
    int unk0;
    unsigned short last;               /* +0x04 random variant */
    signed char seq;                   /* +0x06 sequential variant */
} SndRandHist;

extern SndList snd_idle_list;          /* 0x08C1AFE0 kind 0 */
extern SndList snd_stream_list;        /* 0x08C1AFEC kinds 3-4 */
extern SndList snd_sas_list;           /* 0x08C1AFF8 kinds 1-2 */
extern SndList snd_active;             /* 0x08C1B004 */
extern SndFader snd_bgm_master;        /* 0x08C1B010 */
extern short snd_ch_volume[3];         /* 0x08C1B028 */
extern short snd_cur_slot;             /* 0x08C1B030, followed by the 4 slot objects */
extern int snd_sema;                   /* 0x08C1B044 */
extern int snd_atrac_sema;             /* 0x08C1B048 */
extern int snd_obj_sema;               /* 0x08C1B050 */
extern SndVoiceSlot snd_voice_slots[32]; /* 0x08C1B058 */
extern unsigned char snd_obj_pool[];   /* 0x08C1B1D8, 0x118 per object */
extern int snd_voice_next;             /* 0x08C1D884 */
extern int snd_voice_cur;              /* 0x08C1D888 */
extern unsigned char snd_out_mode[4];  /* 0x08C1D890 */

int snd_group_of(unsigned int id);
int snd_group_data_of(unsigned int id);
unsigned char *snd_desc(unsigned int id);
int snd_desc_b2(unsigned int id);
int snd_entry_prio(unsigned int id);
int snd_entry_b2(unsigned int id);
int snd_list_contains(SndList *l, void *node);
void snd_list_append(SndList *l, void *node);
void snd_list_remove(SndList *l, void *node);
SndRandHist *snd_rand_hist(unsigned int id, int pan);
int snd_obj_alloc(unsigned int id, int pan, int b2, unsigned int flags);
int snd_rand_range(int n);
unsigned int snd_id_handle(unsigned int id);
unsigned int snd_voice_find_free(int n);
void snd_voice_alloc(unsigned int mask, int obj);
void snd_voice_free_one(int v, int reset);
SndObj *snd_cur_slot_get(void);
void snd_out_mode_apply(unsigned char a, unsigned char b);
unsigned int snd_obj_handle(SndObj *o);
SndObj *snd_obj_slot_from_handle(unsigned int h);
void snd_obj_order(SndObj *o, int add);
void snd_slots_update(void);
void *snd_group_load(unsigned int g, void *buf, void *name, unsigned int limit);
int snd_group_unload(unsigned int g, int free);
void snd_fader_init(SndFader *f, short v);
void snd_fader8_init(SndFader8 *f, signed char v);
void snd_fader_set(SndFader *f, short value, int time);
void snd_obj_fade(SndObj *o, int which, short value, int time);
int sceAtracReleaseAtracID(int id);
unsigned int snd_obj_start(unsigned int id, void *name, int fade, unsigned int flags, int pan);
int snd_stop(unsigned int h, int fade);
int snd_stop_fade(unsigned int h, int fade);
int bgm_slot_play(unsigned int id, void *name, int fade_in, int fade_out, unsigned int flags);
void voice_start(unsigned int id, int fade, unsigned int flags);

/* Milliseconds -> fader ticks of 5.8 ms; <0 is the default 0x56. */
#define MS_TICKS(ms) ((ms) < 0 ? 0x56 : (ms) * 10 / 58)

void snd_obj_reset(SndObj *o, unsigned int id, unsigned int req)
{
    unsigned int kind;
    SndStream *s;
    int i;
    int ch;

    o->state = 0;
    o->b110 = 0;
    o->order = -1;
    o->id = id;
    *(unsigned int *)&o->sid = req;
    o->b113 = 0;
    o->volA4 = 0x1000;
    o->volA6 = 0x1000;
    o->volA0 = 0;
    kind = (id >> 16) & 0xF;
    o->kind = kind;
    o->volA2 = 0;
    o->frames = 0;
    o->on_end = 0;
    for (i = 5; i >= 0; i--) {
        snd_fader_init(&o->fade[i], 0x1000);
    }
    ch = 0;
    if (id & 0x100000) {
        ch = 1;
        if (id & 0x200000) {
            ch = 2;
        }
    }
    snd_fader_init(&o->fade[0], snd_ch_volume[ch]);
    snd_fader_init(&o->fade[6], 0x40);
    snd_fader_init(&o->fade[7], 0x1000);
    o->fade_in = 0;
    o->fade_out = 0;
    switch (kind) {
    case 0:
        o->s.u.v.b4 = 0;
        break;
    case 2:
    case 1:
        o->s.atrac = 0;
        o->s.pos = 0;
        o->s.u.v.send = 0;
        o->s.u.v.b4 = 0;
        break;
    case 3:
    case 4:
        s = &o->s;
        s->atrac = -1;
        s->decoded = 0;
        s->pos = 0;
        s->u.st.end = 0;
        s->u.st.loop2 = -1;
        s->u.st.loop = -1;
        s->u.st.c4 = 0;
        s->u.st.start = 0;
        s->e7 = 0;
        s->e6 = 0;
        snd_fader8_init(&s->fade8, 0);
        s->e5 = 0;
        s->e4 = 0;
        s->sflags = 0;
        break;
    default:
        return;
    }
}


/* Puts `o` in the active list and the list of its kind as pool slot `idx`. */
void snd_obj_attach(SndObj *o, int idx)
{
    if (snd_list_contains(&snd_active, o) == 0) {
        switch (o->kind) {
        case 0:
            snd_list_append(&snd_idle_list, o);
            break;
        case 2:
        case 1:
            snd_list_append(&snd_sas_list, o);
            break;
        case 3:
        case 4:
            snd_list_append(&snd_stream_list, o);
            break;
        default:
            return;
        }
        snd_list_append(&snd_active, o);
        o->index = idx;
    }
}

void snd_obj_detach(SndObj *o)
{
    switch (o->kind) {
    case 0:
        snd_list_remove(&snd_idle_list, o);
        break;
    case 2:
    case 1:
        snd_list_remove(&snd_sas_list, o);
        break;
    case 3:
    case 4:
        snd_list_remove(&snd_stream_list, o);
        break;
    default:
        return;
    }
    snd_list_remove(&snd_active, o);
    o->index = -1;
    *(unsigned int *)&o->sid = 0xFFFFFEFF;
    o->id = 0xFFFFFEFF;
    o->state = 0;
}

/* Starts sound `id` (handle form from snd_id_handle); `name` identifies the group read, `flags`
   0x8000 starts it suspended, 0x4000 marks a restart. Returns the object handle or 0. */
#ifdef NON_MATCHING /* the original keeps `flags` in s4 and spills `name` and `fade`; mwcc spills `flags` here */
unsigned int snd_obj_start(unsigned int id, void *name, int fade, unsigned int flags, int pan)
{
    int data = snd_group_data_of(id);
    unsigned char *d = snd_desc(id);
    SndRandHist *hist;
    unsigned int susp;
    unsigned int st;
    unsigned int kind;
    unsigned int v;
    SndObj *o;
    unsigned int cur;
    int n;
    int r;
    int idx;

    if ((id & 0x100000) && data == 0) {
        return 0;
    }
    cur = id;
    if ((id & 0x100000) && d != 0 && (d[0x10] != 0 || d[0x11] != 0)) {
        hist = snd_rand_hist(id, pan);
        do {
            n = d[0x10];
            r = -1;
            if (n != 0) {
                r = snd_rand_range(n);
                v = (cur & 0xFFFF) + r;
                if (hist->last == v) {
                    r++;
                    v++;
                    if (r >= n) {
                        v = cur & 0xFFFF;
                        r = 0;
                    }
                }
                hist->last = v;
            }
            if (d[0x11] != 0) {
                r = hist->seq + 1;
                if (r >= d[0x11]) {
                    r = 0;
                }
                hist->seq = r;
            }
            if (r < 0) {
                break;
            }
            cur = snd_id_handle(cur + r) | 0x100000;
            d = snd_desc(cur);
        } while (cur != id);
    }
    susp = (flags & 0xFFFF) & 0x8000;
    st = 0x4000;
    if (susp) {
        st |= 0x40;
    }
    kind = (cur >> 16) & 0xF;
    idx = snd_obj_alloc(cur, pan, snd_entry_b2(cur), flags);
    if (idx < 0) {
        return 0;
    }
    o = (SndObj *)(snd_obj_pool + idx * 0x118);
    if (o->index >= 0 && !(o->state & 0x40)) {
        snd_stop((unsigned int)o, 0);
        data = snd_group_data_of(cur);
    }
    if (data == 0 && !(cur & 0x100000) && !susp) {
        if (kind - 3 < 2) {
            if (snd_group_load(cur & 0xFFFF, 0, name, 0x4000) == 0) {
                return 0;
            }
        } else if (snd_group_load(cur & 0xFFFF, 0, name, 0) == 0) {
            return 0;
        }
        st |= 0x1400;
    }
    switch (kind) {
    case 0:
        snd_obj_reset(o, cur, id);
        return 0;
    case 2:
    case 1:
        snd_obj_reset(o, cur, id);
        if (!susp) {
            o->s.pos = snd_voice_find_free(snd_entry_prio(cur));
            snd_voice_alloc(o->s.pos, idx);
        }
        o->state = st;
        break;
    case 3:
    case 4:
        snd_obj_reset(o, cur, id);
        o->s.atrac = -1;
        o->state = st;
        break;
    default:
        return 0;
    }
    o->fade_in = fade;
    o->pan_arg = pan;
    sceKernelWaitSema(snd_obj_sema, 1, 0);
    snd_obj_attach(o, idx);
    snd_obj_order(o, 1);
    sceKernelSignalSema(snd_obj_sema, 1);
    return idx << 16 | (cur & 0xFFFF) | 0x20000000;
}
#endif

/* Retries a start that found no free object once more as suspended. */
unsigned int snd_obj_start_retry(unsigned int id, void *name, int fade, unsigned int flags)
{
    unsigned int h;

    for (;;) {
        h = snd_obj_start(id, name, fade, flags, 0);
        if (h != 0) {
            break;
        }
        if ((unsigned short)flags & 0x8000) {
            break;
        }
        flags = (unsigned short)(flags | 0x8000);
    }
    return h;
}

/* BGM in one of the 4 slots (a suspended one, else the last empty one): replaces the current BGM,
   fading it out over `fade_out` ticks, or becomes the current one when none is playing. */
int bgm_slot_play(unsigned int id, void *name, int fade_in, int fade_out, unsigned int flags)
{
    short *base = &snd_cur_slot;
    SndObj *o;
    int i;
    int cur;
    short *p;
    SndObj *c;
    int slot = -1;
    unsigned int h;

    for (i = 0, p = base; i < 4; i++, p += 2) {
        o = *(SndObj **)(p + 2);
        if (o == 0) {
            slot = i;
        } else if (o->state & 0x40) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        return 0;
    }
    cur = *base;
    if (cur >= 0) {
        c = ((SndObj **)(base + 2))[cur];
        if (c->id == id) {
            return snd_obj_handle(o);
        }
        h = snd_obj_start_retry(id, name, fade_in, flags & 0xFFFF);
        if (h == 0) {
            return 0;
        }
        o = snd_obj_slot_from_handle(h);
        snd_stop((unsigned int)c, fade_out);
        ((SndObj **)(base + 2))[slot] = o;
        snd_slots_update();
        return h;
    }
    h = snd_obj_start_retry(id, name, fade_in, flags & 0xFFFF);
    if (cur < 0 && h != 0) {
        o = snd_obj_slot_from_handle(h);
        if (!(o->state & 0x40)) {
            *(SndObj **)(base + 2) = o;
            *base = 0;
        }
        snd_slots_update();
    }
    return h;
}

void bgm_start(unsigned int id, void *name, int in_ms, int out_ms, unsigned int flags)
{
    int in = MS_TICKS(in_ms);
    int out = MS_TICKS(out_ms);

    bgm_slot_play(snd_id_handle(id) & ~0x100000, name, in, out, flags);
}

/* Samples played of a stream, -1 for other kinds. */
int snd_obj_stream_pos(unsigned int h)
{
    SndObj *o = snd_obj_find(h);

    if (o == 0) {
        return -1;
    }
    if (o->kind == 3) {
    } else if (o->kind != 4) {
        return -1;
    }
    return o->s.pos - o->s.u.st.start;
}

void se_play_pan(unsigned int id, int fade_ms, unsigned short pan)
{
    int t = MS_TICKS(fade_ms);
    unsigned int h = snd_id_handle(id) | 0x100000;

    snd_obj_start(h, (void *)h, t, 0, pan);
}

/* Voice clip: a new one cuts the previous (which fades out) and waits suspended for it. */
void voice_start(unsigned int id, int fade_ms, unsigned int flags)
{
    int t;
    unsigned int h;
    unsigned int r;

    if (snd_voice_cur != 0) {
        snd_stop_fade(snd_voice_cur, 0x78);
        if (snd_voice_next != 0) {
            snd_stop_fade(snd_voice_next, 0);
            snd_voice_next = 0;
        } else {
            flags = 0x8000;
        }
    }
    t = MS_TICKS(fade_ms);
    h = snd_id_handle(id) | 0x300000;
    r = snd_obj_start(h, (void *)h, t, (unsigned short)flags, 0);
    if (r != 0) {
        if (snd_voice_cur == 0) {
            snd_voice_cur = r;
            snd_voice_next = 0;
        }
        if (flags & 0x8000) {
            snd_voice_next = r;
        }
    }
}

int snd_obj_set_volume(unsigned int h, int vol, int time)
{
    SndObj *o = snd_obj_find(h);

    if (o == 0) {
        return 0;
    }
    snd_obj_fade(o, 3, (unsigned int)(vol * 0x1021) >> 7, time);
    return 1;
}

int snd_cur_set_volume(int vol, int time)
{
    SndObj *o = snd_cur_slot_get();

    if (o == 0) {
        return 0;
    }
    snd_obj_fade(o, 3, (unsigned int)(vol * 0x1021) >> 7, time);
    return 1;
}

/* Releases `o` at once (or after a fade of `fade` ticks): frees its group data, Atrac id or SAS
   voices and calls its release callback. */
void snd_obj_stop(SndObj *o, int fade, int reset)
{
    unsigned short st = o->state;
    unsigned short sid;
    unsigned int h;
    void (*cb)(int, unsigned short);
    SndVoiceSlot *v;
    int i;

    if (st & 0x1000) {
        fade = 0;
    }
    if (st & 0x40) {
        fade = 0;
    }
    if (fade != 0) {
        o->state |= 0x80;
        snd_obj_fade(o, 1, 0, fade);
        return;
    }
    sceKernelWaitSema(snd_obj_sema, 1, 0);
    sid = o->id;
    h = snd_obj_handle(o);
    cb = o->on_end;
    if (o->state & 0x400) {
        snd_group_unload(o->id, 1);
    }
    switch ((o->id >> 16) & 0xF) {
    case 0:
        sceKernelWaitSema(snd_sema, 1, 0);
        sceKernelSignalSema(snd_sema, 1);
        break;
    case 2:
    case 1:
        if (!(st & 0x40)) {
            for (v = snd_voice_slots, i = 0; i < 32; i++, v++) {
                if (v->obj == o) {
                    snd_voice_free_one(i, reset);
                }
            }
        }
        snd_obj_order(o, 0);
        snd_obj_detach(o);
        break;
    case 3:
    case 4:
        sceKernelWaitSema(snd_atrac_sema, 1, 0);
        if (!(st & 0x40) && o->s.atrac >= 0) {
            sceAtracReleaseAtracID(o->s.atrac);
        }
        snd_obj_order(o, 0);
        snd_obj_detach(o);
        sceKernelSignalSema(snd_atrac_sema, 1);
        break;
    }
    sceKernelSignalSema(snd_obj_sema, 1);
    if (cb != 0) {
        cb(h, sid);
    }
    if (h == snd_voice_cur) {
        snd_voice_cur = 0;
    }
}

/* Lowers the BGM master by the strongest ducking level among the playing sounds. */
void snd_duck_update(void)
{
    SndObj *cur;
    int min = 0x100;
    SndObj *o = snd_list_next(&snd_active, 0);
    int d;

    for (;;) {
        if (o == 0) {
            break;
        }
        cur = o;
        o = snd_list_next(&snd_active, o);
        d = snd_desc_b2(cur->id);
        if (d > 0 && d < min) {
            min = d;
        }
    }
    {
        int v = 0x1000;

        if (min < 0x100) {
            v = (0xF - min) * 0x1112 >> 4;
        }
        snd_fader_set(&snd_bgm_master, v, 0x96);
    }
}

int snd_stop(unsigned int h, int fade)
{
    SndObj *o = snd_obj_find(h);

    if (o == 0) {
        return 0;
    }
    snd_obj_handle(o);
    snd_obj_stop(o, fade, 1);
    snd_duck_update();
    return 1;
}

/* Stops `h`; when it was released, resumes the suspended BGM and voice clips. */
int snd_stop_fade(unsigned int h, int fade)
{
    SndObj *o = snd_obj_find(h);
    unsigned int id;
    int i;
    short *p;
    SndObj *s;
    SndObj *next;

    if (o == 0) {
        return 0;
    }
    id = o->id;
    if (snd_stop(h, fade) == 0) {
        return 0;
    }
    if (o->index >= 0) {
        return 1;
    }
    snd_slots_update();
    if (snd_out_mode[3] != snd_out_mode[1] || snd_out_mode[2] != snd_out_mode[0]) {
        snd_out_mode_apply(snd_out_mode[1], snd_out_mode[0]);
    }
    if (!(id & 0x100000)) {
        for (p = &snd_cur_slot, i = 0; i < 4; i++, p += 2) {
            s = *(SndObj **)(p + 2);
            if (s != 0 && (s->state & 0x40)) {
                bgm_slot_play(s->id, s->name, s->fade_in, s->fade_out, 0x4000);
            }
        }
    }
    if ((id & 0x300000) == 0x300000) {
        next = snd_list_next(&snd_active, 0);
        for (;;) {
            if (next == 0) {
                break;
            }
            s = next;
            next = snd_list_next(&snd_active, next);
            if ((s->state & 0x40) && (s->id & 0x300000) == 0x300000) {
                voice_start(s->id & 0xFFFF, 0, 0x4000);
            }
        }
    }
    return 1;
}

int snd_group_stop(int g, int fade)
{
    SndObj *cur;
    SndObj *o = snd_list_next(&snd_active, 0);

    for (;;) {
        if (o == 0) {
            break;
        }
        cur = o;
        o = snd_list_next(&snd_active, o);
        if (g == snd_group_of(cur->id)) {
            snd_stop_fade((unsigned int)cur, fade);
        }
    }
    return 1;
}

/* -0x109 SE of channel 1 only, -0x108 voices, -0x107 SE, -0x106 BGM, -0x105 everything. */
#ifdef NON_MATCHING /* one compare takes its operands in the other order */
int snd_group_cmd(unsigned int cmd, int fade)
{
    unsigned int val;
    unsigned int mask;
    SndObj *cur;
    SndObj *o;

    if (cmd < 0xFFFFFEF0) {
        return 0;
    }
    if (cmd >= 0xFFFFFF00) {
        return 0;
    }
    val = -1;
    mask = 0x300000;
    switch (cmd) {
    case 0xFFFFFEFB:
        break;
    case 0xFFFFFEFA:
        val = 0;
        break;
    case 0xFFFFFEF9:
        val = 0x100000;
        break;
    case 0xFFFFFEF8:
        val = 0x300000;
        break;
    case 0xFFFFFEF7:
        mask = 0x100000;
        val = 0x100000;
        break;
    default:
        return 0;
    }
    o = snd_list_next(&snd_active, 0);
    for (;;) {
        if (o == 0) {
            break;
        }
        cur = o;
        o = snd_list_next(&snd_active, o);
        if ((int)val < 0 || (cur->id & mask) == val) {
            snd_stop_fade((unsigned int)cur, fade);
        }
    }
    return 1;
}
#endif
