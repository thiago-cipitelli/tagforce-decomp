// @range 0x08860DA4-0x088615D0
// @asm 0x088610E0-0x08861304
// @asm 0x088614D0-0x088615A4
/* Sound library, object side: SAS voice allocation mask, per-channel volumes (BGM/SE/voice),
   counters over the active object list, the level meter used for lip sync, and the SAS
   effect/output setup. */
#include "psp.h"
#include "libc.h"
#include "snd.h"

typedef struct SndVoiceTab {           /* 0x08C1B054 */
    unsigned int busy;                 /* mask of SAS voices in use */
    SndVoiceSlot v[32];
} SndVoiceTab;

typedef struct SndVoiceRec {           /* the slot table walked from &snd_voice_tab: record i's obj is at +4 */
    unsigned int prev;
    SndObj *obj;
    unsigned char b8;
    unsigned char flags;
    unsigned char pad[2];
} SndVoiceRec;

typedef struct SndBankHdr {
    unsigned char pad[0x20];
    unsigned short *base_ids;
} SndBankHdr;

extern SndBankHdr *snd_bank;           /* 0x08C1D88C */
extern SndList snd_active;             /* 0x08C1B004 */
extern SndFader snd_master;            /* 0x08C1B01C */
extern unsigned short snd_ch_volume[3]; /* 0x08C1B028 */
extern short snd_cur_slot;             /* 0x08C1B030 */
extern int snd_slot_tab[];             /* 0x08C1B034 */
extern SndVoiceTab snd_voice_tab;      /* 0x08C1B054 */
extern unsigned char snd_obj_pool[];   /* 0x08C1B1D8, 0x118 per object */
extern unsigned short snd_sas_busy;    /* 0x08C1AFF4 */
extern unsigned char snd_out_mode[4];  /* 0x08C1D890 */
extern short snd_fx_wet_r;             /* 0x08C1D896 */
extern short snd_fx_wet_l;             /* 0x08C1D898 */
extern short snd_fx_type;              /* 0x08C1D89A */

int sceAtrac3plus_132F1ECA(void);
void snd_voice_set_adsr(int v, int mask, int a, int d, int s, int r);
void snd_voice_set_curve(int v, int mask, int a, int d, int s, int r);
void snd_voice_key_off(int v);
void snd_fader_set(SndFader *f, int value, int time);
void snd_obj_fade(SndObj *o, int which, short value, int time);
int snd_group_data(unsigned int g);
void snd_fx_set_dry(int l, int r);
void snd_fx_set_wet_on(int l, int r);
int snd_fx_set_type(int type);

/* Bit mask of `n` free SAS voices, 0 when there are not enough. */
unsigned int snd_voice_find_free(int n)
{
    unsigned int free = ~snd_voice_tab.busy;
    unsigned int bit = 1;
    unsigned int mask = 0;

    while (free != 0) {
        if (free & 1) {
            n--;
            mask |= bit;
            if (n <= 0)
                return mask;
        }
        free >>= 1;
        bit <<= 1;
    }
    return 0;
}

void snd_voice_alloc(unsigned int mask, int obj)
{
    SndVoiceRec *s;

    snd_voice_tab.busy |= mask;
    if (mask == 0)
        return;
    s = (SndVoiceRec *)&snd_voice_tab;
    do {
        if (mask & 1) {
            s->obj = (SndObj *)(snd_obj_pool + obj * 0x118);
            s->b8 = 0;
            s->flags = 0;
        }
        mask >>= 1;
        s = (SndVoiceRec *)((unsigned char *)s + 12);
    } while (mask != 0);
}

void snd_voice_free(unsigned int mask, int reset)
{
    SndVoiceRec *s;
    int v;

    snd_voice_tab.busy &= ~mask;
    v = 0;
    if (mask == 0)
        return;
    s = (SndVoiceRec *)&snd_voice_tab;
    do {
        if (mask & 1) {
            if (reset) {
                snd_voice_set_adsr(v, 8, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF);
                snd_voice_set_curve(v, 8, 1, 1, 1, 1);
            }
            snd_voice_key_off(v);
            s->obj = 0;
        }
        mask >>= 1;
        s++;
        v++;
    } while (mask != 0);
}

void snd_voice_free_one(int v, int reset)
{
    unsigned int mask = -1;
    if (v >= 0)
        mask = 1 << v;
    snd_voice_free(mask, reset);
}

int snd_cur_slot_get(void)
{
    if (snd_cur_slot < 0)
        return 0;
    return snd_slot_tab[snd_cur_slot];
}

/* 0 stopped, 1 playing, 0x20 loaded; | 4 looping, | 2 paused. */
unsigned int snd_obj_status(int handle)
{
    SndObj *o;
    unsigned short st;
    unsigned int r = 0;

    o = snd_obj_find(handle);
    if (o != 0 && o->index >= 0) {
        st = o->state;
        if (st & 0x2000)
            r |= 1;
        else
            r = 0x20;
        if (st & 0x80)
            r |= 4;
        if (st & 0x1000)
            r |= 2;
    }
    return r;
}

int snd_obj_exists(int handle)
{
    return snd_obj_status(handle) != 0;
}

int snd_obj_count_bgm(void)
{
    int n = 0;
    SndObj *o = 0;

    for (;;) {
        o = snd_list_next(&snd_active, o);
        if (o == 0)
            break;
        if ((o->id & 0x100000) == 0)
            n++;
    }
    return n;
}

int snd_obj_count_se(void)
{
    int n = 0;
    SndObj *o = 0;

    for (;;) {
        o = snd_list_next(&snd_active, o);
        if (o == 0)
            break;
        if (o->id & 0x100000)
            n++;
    }
    return n;
}

int snd_seq_entry(unsigned int g)
{
    return snd_group_data(g);
}

int snd_obj_base_id(int obj)
{
    return snd_bank->base_ids[obj];
}

#ifdef NON_MATCHING /* branch layout of the kind test and the meter arithmetic */
/* Arms the level meter of a streamed sound (kind 3/4); returns its current level, -1 if none. */
int snd_obj_meter_level(int handle)
{
    SndObj *o = snd_obj_find(handle);
    unsigned int kind;
    unsigned short st;

    if (o == 0)
        return -1;
    kind = (o->id & 0xFFFFF) >> 16;
    if (kind == 3)
        st = o->state;
    else if (kind == 4)
        st = o->state;
    else
        return -1;
    if ((st & 0x2000) == 0)
        return 0;
    if ((st & 0x20) == 0) {
        o->s.sflags |= 0x10;
        o->s.level = 0;
        o->s.meter = 0;
        o->s.acc = 0;
        o->s.shown = 0;
        o->s.peak = 0;
        o->s.tick = 0;
        o->s.step = 0;
        o->state |= 0x20;
        return 0;
    }
    return o->s.level;
}

/* Smoothed mouth level for lip sync, advanced by `dt` ticks. */
int snd_obj_query(int handle, signed char dt)
{
    SndObj *o = snd_obj_find(handle);
    unsigned int lv;
    int v;

    if (o == 0)
        return -1;
    if (snd_obj_meter_level((int)o) < 0)
        return -1;
    if ((o->state & 0x20) == 0)
        return 0;
    lv = (int)(o->s.level << 12) >> 14;
    if (lv < 0x1000) {
        v = 0x1000 - lv;
        v = 0x1000 - (v * v >> 12);
    } else {
        v = 0x1000;
    }
    if (o->s.peak > 0x3FF && v < 0x200)
        v = 0;
    if (v < 0x100) {
        if (o->s.peak == 0 && o->s.tick < 4) {
            o->s.acc = 0;
        } else {
            o->s.tick = 5;
            o->s.acc = 0;
        }
    } else {
        o->s.acc = (v + (unsigned short)o->s.acc) >> 1;
    }
    dt = o->s.tick + dt;
    o->s.tick = dt;
    if (dt < 6) {
        if (dt > 3)
            return o->s.peak;
        o->s.shown += o->s.step;
    } else {
        o->s.peak = o->s.acc;
        o->s.acc = 0;
        o->s.tick = 0;
        o->s.step = (o->s.peak - o->s.shown) >> 2;
    }
    return o->s.shown;
}

#endif

int snd_set_master_volume(int vol, int time)
{
    int old = snd_master.target << 1;
    snd_fader_set(&snd_master, (short)((unsigned int)(vol * 0x8103) >> 8), time);
    return old;
}

/* Volume 0..127 of channel 0 BGM, 1 SE, 2 voice; ramps every active sound of the channel. */
int snd_set_channel_volume(int ch, unsigned int vol, int time)
{
    int old = snd_ch_volume[ch];
    int v;
    unsigned int match;
    SndObj *o;

    if (vol > 0x7F)
        vol = 0x7F;
    v = vol * 0x1021 >> 7;
    snd_ch_volume[ch] = v;
    if (time >= 0) {
        match = -1;
        switch (ch) {
        case 0:
            match = 0;
            break;
        case 1:
            match = 0x100000;
            break;
        case 2:
            match = 0x300000;
            break;
        }
        o = 0;
        for (;;) {
            o = snd_list_next(&snd_active, o);
            if (o == 0)
                break;
            if ((o->id & 0x300000) != match)
                continue;
            snd_obj_fade(o, 0, v, time);
        }
    }
    return old;
}

void snd_out_mode_apply(unsigned char a, unsigned char b)
{
    if (sceAtrac3plus_132F1ECA() == 0) {
        snd_out_mode[3] = a;
        snd_out_mode[2] = b;
    }
}

int snd_sas_init(unsigned char a, unsigned char b)
{
    if (snd_sas_busy == 0) {
        snd_out_mode[1] = a;
        snd_out_mode[0] = b;
        snd_out_mode_apply(a, b);   /* returns whatever the apply left in v0 */
    } else {
        return 1;
    }
}

#ifdef NON_MATCHING /* likely vs plain branch on the set-type result */
/* SAS effect (reverb) type 0..; -1 selects 4. Returns the previous type. */
int snd_fx_setup(int type, int wet_l, int wet_r)
{
    int r;

    if (snd_fx_type < 0) {
        snd_fx_set_dry(0x1000, 0x1000);
        snd_fx_set_wet_on(1, 1);
    }
    if (type < 0)
        type = 4;
    if (type == 0) {
        wet_r = 0;
        wet_l = 0;
    }
    r = snd_fx_set_type(type);
    if (r == 0) {
        snd_fx_wet_l = wet_l * 0x1021 >> 7;
        snd_fx_wet_r = wet_r * 0x1021 >> 7;
        r = snd_fx_type;
        snd_fx_type = type;
    }
    return r;
}

#endif

/* Public handle of an object: index << 16 | id, with bit 29 set. */
unsigned int snd_obj_handle(SndObj *o)
{
    if (o == 0)
        return 0;
    return (o->index << 16 | (unsigned short)o->id) | 0x20000000;
}
