// @range 0x0885F05C-0x0885FAA8
// @asm 0x0885F230-0x0885FAA8
// @data 0x088F0114-0x088F03BC
/* Sound library, allocation and start: history of the last variant picked per sound, choice of the
   object slot for a new sound (free slot, suspended BGM or the oldest of the same class), and the
   start of an object's SAS voices (sample, pitch with random spread, envelope) or Atrac stream. */
#include "psp.h"
#include "libc.h"
#include "snd.h"

typedef struct SndRandHist {           /* last pick of a sound with variants, most recent first */
    unsigned int key;                  /* pan << 16 | sound id */
    unsigned short last;               /* +0x04 random variant */
    signed char seq;                   /* +0x06 sequential variant */
    unsigned char pad7;
} SndRandHist;

typedef struct SndDesc {               /* sound descriptor (snd_desc) */
    unsigned char pad00[8];
    int data;                          /* +0x08 offset of the stream in the group data */
    unsigned char e4, e5;              /* +0x0C */
    unsigned char pad0e[4];
    signed char vol_lo, vol_hi;        /* +0x12 random volume spread */
    signed char semi_lo, semi_hi;      /* +0x14 random semitone spread */
    signed char fine_lo, fine_hi;      /* +0x16 random fine pitch spread */
} SndDesc;

typedef struct SndTone {               /* per-voice parameters (snd_data_ptr_18) */
    signed char vol;
    unsigned char angle;
    unsigned char ar;                  /* +0x02 attack */
    unsigned char dr;                  /* +0x03 decay */
    unsigned char sr;                  /* +0x04 sustain rate, or ADSR1 when the bank is newer */
    unsigned char rr;                  /* +0x05 release */
    unsigned char sr_dir;              /* +0x06 */
    unsigned char pad7;
    unsigned char sr_mode;             /* +0x08 */
    unsigned char send;                /* +0x09 */
    unsigned char sl;                  /* +0x0A sustain level */
    unsigned char pad0b;
    unsigned char semi;                /* +0x0C transpose */
    unsigned char fine;                /* +0x0D */
} SndTone;

typedef struct SndSample {             /* snd_data_ptr_10 */
    int size;
    unsigned short pitch;              /* +0x04 */
    unsigned short pad6;
    unsigned char vag[1];              /* +0x08 */
} SndSample;

extern SndRandHist snd_rand_hist_tab[32]; /* 0x08C1D77C */
extern unsigned char snd_obj_pool[];   /* 0x08C1B1D8, 0x118 per object */
extern unsigned char snd_out_mode[4];  /* 0x08C1D890 [2] BGM slots, [3] stream slots */
extern SndVoiceSlot snd_voice_slots[32]; /* 0x08C1B058 */
extern unsigned short *snd_bank;       /* 0x08C1D88C SHDS header */

/* Pitch factors in 4.12: semitones -60..+23, then a fine step of 1/256 semitone (-128..+127). */
unsigned short snd_pitch_semi[84] = {
    0x0001, 0x000F, 0x001F, 0x0030, 0x0043, 0x0056, 0x006A, 0x0080, 0x0096, 0x00AF, 0x00C8, 0x00E3,
    0x0100, 0x010F, 0x011F, 0x0130, 0x0143, 0x0156, 0x016A, 0x0180, 0x0196, 0x01AF, 0x01C8, 0x01E3,
    0x0200, 0x021E, 0x023F, 0x0261, 0x0285, 0x02AB, 0x02D4, 0x02FF, 0x032D, 0x035D, 0x0390, 0x03C7,
    0x0400, 0x043D, 0x047D, 0x04C2, 0x050A, 0x0557, 0x05A8, 0x05FE, 0x0659, 0x06BA, 0x0721, 0x078D,
    0x0800, 0x087A, 0x08FB, 0x0983, 0x0A14, 0x0AAE, 0x0B50, 0x0BFD, 0x0CB3, 0x0D74, 0x0E41, 0x0F1A,
    0x1000, 0x10F4, 0x11F6, 0x1307, 0x1429, 0x155C, 0x16A1, 0x17F9, 0x1966, 0x1AE9, 0x1C82, 0x1E34,
    0x2000, 0x21E7, 0x23EB, 0x260E, 0x2851, 0x2AB7, 0x2D41, 0x2FF2, 0x32CC, 0x35D1, 0x3904, 0x3C68,
};
unsigned short snd_pitch_fine[256] = {
    0x0800, 0x080B, 0x0816, 0x0822, 0x082D, 0x0839, 0x0844, 0x0850, 0x085B, 0x0867, 0x0873, 0x087F,
    0x088B, 0x0897, 0x08A3, 0x08AF, 0x08BB, 0x08C7, 0x08D3, 0x08E0, 0x08EC, 0x08F9, 0x0905, 0x0912,
    0x091F, 0x092B, 0x0938, 0x0945, 0x0952, 0x095F, 0x096C, 0x097A, 0x0987, 0x0994, 0x09A2, 0x09AF,
    0x09BD, 0x09CA, 0x09D8, 0x09E6, 0x09F4, 0x0A02, 0x0A10, 0x0A1E, 0x0A2C, 0x0A3A, 0x0A48, 0x0A57,
    0x0A65, 0x0A74, 0x0A83, 0x0A91, 0x0AA0, 0x0AAF, 0x0ABE, 0x0ACD, 0x0ADC, 0x0AEB, 0x0AFB, 0x0B0A,
    0x0B1A, 0x0B29, 0x0B39, 0x0B48, 0x0B58, 0x0B68, 0x0B78, 0x0B88, 0x0B98, 0x0BA9, 0x0BB9, 0x0BC9,
    0x0BDA, 0x0BEA, 0x0BFB, 0x0C0C, 0x0C1D, 0x0C2E, 0x0C3F, 0x0C50, 0x0C61, 0x0C73, 0x0C84, 0x0C96,
    0x0CA7, 0x0CB9, 0x0CCB, 0x0CDD, 0x0CEF, 0x0D01, 0x0D13, 0x0D25, 0x0D38, 0x0D4A, 0x0D5D, 0x0D70,
    0x0D82, 0x0D95, 0x0DA8, 0x0DBC, 0x0DCF, 0x0DE2, 0x0DF6, 0x0E09, 0x0E1D, 0x0E31, 0x0E44, 0x0E58,
    0x0E6D, 0x0E81, 0x0E95, 0x0EA9, 0x0EBE, 0x0ED3, 0x0EE7, 0x0EFC, 0x0F11, 0x0F26, 0x0F3C, 0x0F51,
    0x0F66, 0x0F7C, 0x0F92, 0x0FA8, 0x0FBD, 0x0FD4, 0x0FEA, 0x1000, 0x1000, 0x1016, 0x102D, 0x1044,
    0x105A, 0x1071, 0x1088, 0x10A0, 0x10B7, 0x10CE, 0x10E6, 0x10FD, 0x1115, 0x112D, 0x1145, 0x115D,
    0x1176, 0x118E, 0x11A7, 0x11C0, 0x11D8, 0x11F1, 0x120B, 0x1224, 0x123D, 0x1257, 0x1271, 0x128A,
    0x12A4, 0x12BE, 0x12D9, 0x12F3, 0x130E, 0x1328, 0x1343, 0x135E, 0x1379, 0x1395, 0x13B0, 0x13CC,
    0x13E7, 0x1403, 0x141F, 0x143B, 0x1458, 0x1474, 0x1491, 0x14AE, 0x14CB, 0x14E8, 0x1505, 0x1523,
    0x1540, 0x155E, 0x157C, 0x159A, 0x15B8, 0x15D7, 0x15F5, 0x1614, 0x1633, 0x1652, 0x1671, 0x1691,
    0x16B0, 0x16D0, 0x16F0, 0x1710, 0x1731, 0x1751, 0x1772, 0x1793, 0x17B4, 0x17D5, 0x17F6, 0x1818,
    0x183A, 0x185C, 0x187E, 0x18A0, 0x18C2, 0x18E5, 0x1908, 0x192B, 0x194E, 0x1972, 0x1995, 0x19B9,
    0x19DD, 0x1A02, 0x1A26, 0x1A4B, 0x1A70, 0x1A95, 0x1ABA, 0x1ADF, 0x1B05, 0x1B2B, 0x1B51, 0x1B77,
    0x1B9E, 0x1BC4, 0x1BEB, 0x1C12, 0x1C3A, 0x1C61, 0x1C89, 0x1CB1, 0x1CD9, 0x1D01, 0x1D2A, 0x1D53,
    0x1D7C, 0x1DA5, 0x1DCF, 0x1DF9, 0x1E23, 0x1E4D, 0x1E77, 0x1EA2, 0x1ECD, 0x1EF8, 0x1F23, 0x1F4F,
    0x1F7B, 0x1FA7, 0x1FD3, 0x2000,
};

int snd_entry_b3(unsigned int id);
SndDesc *snd_desc(unsigned int id);
unsigned char *snd_group_data_of(unsigned int id);
unsigned short snd_desc_type(unsigned int id);
int snd_entry_flags4(unsigned int id, int mask);
unsigned char *snd_data_ptr_c(unsigned char *grp, SndDesc *d, int i);
SndSample *snd_data_ptr_10(unsigned char *grp, unsigned char *p);
SndTone *snd_data_ptr_18(unsigned char *grp, unsigned char *p);
void snd_voice_clear(SndObj *o);
int snd_rand_range(int n);
void snd_duck_update(void);
int snd_voice_key_off(int v);
int snd_voice_set_vag(int v, void *vag, int size, int loop);
int snd_voice_set_pitch(int v, int pitch);
int snd_voice_set_volume(int v, int l, int r, int el, int er);
int snd_voice_set_simple_adsr(int v, int adsr1, int adsr2);
int sceAtracSetDataAndGetID(void *buf, int size);
int sceAtracGetMaxSample(int id, int *n);
int sceAtracGetSoundSample(int id, int *end, int *loop_start, int *loop_end);
int sceAtracSetLoopNum(int id, int n);

/* Moves the history entry of (id, pan) to the front, or a fresh one (oldest dropped) when absent. */
SndRandHist *snd_rand_hist(unsigned int id, int pan)
{
    SndRandHist e;
    unsigned int key = pan << 16 | (id & 0xFFFF);
    int j = -1;
    SndRandHist *h = snd_rand_hist_tab;
    int i;

    for (i = 0; i < 32; i++, h++) {
        if (h->key == key) {
            j = i;
            break;
        }
        if (h->key == 0) {
            j = i;
        }
    }
    if (j < 0) {
        j = 31;
        e.key = key + 1;
    } else {
        e = snd_rand_hist_tab[j];
    }
    h = snd_rand_hist_tab;
    for (i = 0; i < j; i++) {
        h[i + 1] = h[i];
    }
    h[0] = e;
    if (h[0].key != key) {
        h[0].seq = -1;
        h[0].key = key;
        h[0].last = 0xFFFF;
    }
    return snd_rand_hist_tab;
}

#ifdef NON_MATCHING /* the SE tail returns through branch-likely to the epilogue instead of the original's bltzl + b; 34 words */
/* Object slot for a new sound, <0 none. Voices (kinds 1-2) use slots 0-3 for one-shot sounds and
   4-27 for SE; BGM (4) and streams (3) use 28-33, limited by snd_out_mode. An SE takes the oldest
   instance of itself when `max` are already playing, or the oldest SE of no higher priority. */
int snd_obj_alloc(unsigned int id, int pan, int max, unsigned short flags)
{
    int end;
    int free;
    int susp;
    unsigned int se = id & 0x100000;
    int prio;
    SndObj *o;
    int i;
    int cls;
    int avail;
    int same;
    int same_order;
    int low;
    int low_order;
    unsigned int kind;

    if (se == 0) {
        max = 1;
    }
    susp = -1;
    free = -1;
    switch ((id >> 16) & 0xF) {
    case 2:
    case 1:
        if (se != 0) {
            i = 4;
            end = 0x1C;
        } else {
            i = 0;
            end = 4;
        }
        cls = -1;
        avail = 0xFF;
        break;
    case 3:
        cls = 3;
        end = 0x22;
        avail = snd_out_mode[3];
        i = 0x1C;
        break;
    case 4:
        cls = 4;
        end = 0x22;
        avail = snd_out_mode[2];
        i = 0x1C;
        break;
    default:
        return susp;
    }
    prio = snd_entry_b3(id);
    o = (SndObj *)(snd_obj_pool + i * 0x118);
    same = -1;
    same_order = 0xFF;
    low = -1;
    low_order = 0xFF;
    for (; i < end; i++, o++) {
        if (o->index < 0) {
            free = i;
            continue;
        }
        kind = (o->id >> 16) & 0xF;
        if (cls == kind) {
            avail--;
        }
        if (se != 0 && pan != (unsigned short)o->pan_arg) {
            continue;
        }
        if (cls == kind && (o->state & 0x40)) {
            susp = i;
            continue;
        }
        if (prio < snd_entry_b3(o->id)) {
            continue;
        }
        if (o->id == id) {
            if (o->order < same_order) {
                same_order = o->order;
                same = i;
            }
            max--;
        }
        if (se == (o->id & 0x100000) && o->order < low_order) {
            if (o->order < low_order) {
                low_order = o->order;
                low = i;
            }
        }
    }
    if (flags & 0x4000) {
        return susp;
    }
    if (se == 0) {
        if ((flags & 0x8000) && susp >= 0) {
            return susp;
        }
        if (avail <= 0 && !(flags & 0x8000)) {
            return -1;
        }
        return free;
    }
    if (free < 0) {
        if (same < 0) {
            same = low;
        }
        return same;
    }
    if (avail <= 0) {
        if (same < 0) {
            same = low;
        }
        return same;
    }
    if (max > 0) {
        same = free;
    }
    return same;
}
#endif

#ifdef NON_MATCHING /* same shape; the voice slot, sample and tone pointers rotate through $s0/$s2/$s3; 47 words */
/* Starts the SAS voices of `o` (kinds 1-2, one per bit of the voice mask) or its Atrac stream
   (kinds 3-4). Returns 1, -1 on bad data, 0 when no Atrac id is free. */
int snd_obj_begin(SndObj *o)
{
    unsigned int id;
    SndDesc *d;
    unsigned char *grp;
    int loop;
    unsigned int mask;
    int at;
    unsigned char *p;
    unsigned char *e;
    int n;
    SndSample *smp;
    SndTone *t;
    int pitch;
    int x;
    int a1;
    int a2;
    int *hdr;
    SndVoiceSlot *slot;
    SndStream *s;
    int v;
    int max;

    id = o->id;
    d = snd_desc(id);
    grp = snd_group_data_of(id);
    if (grp == 0 || d == 0) {
        return -1;
    }
    o->volA0 = snd_desc_type(id);
    loop = 0;
    if (snd_entry_flags4(id, 0x80)) {
        loop = 1;
    }
    switch ((id >> 16) & 0xF) {
    case 2:
    case 1:
        mask = o->s.pos;
        slot = snd_voice_slots;
        n = 0;
        p = snd_data_ptr_c(grp, d, 0);
        for (v = 0; mask != 0; mask >>= 1, v++, slot++) {
            if (!(mask & 1)) {
                continue;
            }
            snd_voice_key_off(v);
            smp = snd_data_ptr_10(grp, p);
            t = snd_data_ptr_18(grp, p);
            snd_voice_set_vag(v, smp->vag, smp->size, loop);
            pitch = smp->pitch;
            if (t->semi != 0) {
                pitch = pitch * snd_pitch_semi[t->semi + 60] >> 12;
            }
            x = d->semi_hi - d->semi_lo;
            if (x != 0) {
                x = d->semi_lo + snd_rand_range(x + 1) + 60;
                if (x < 0) {
                    x = 0;
                }
                if (x >= 0x54) {
                    x = 0x53;
                }
                pitch = pitch * snd_pitch_semi[x] >> 12;
            }
            x = d->fine_hi - d->fine_lo;
            if (x != 0) {
                x = d->fine_lo + snd_rand_range(x) + 0x80;
                pitch += (snd_pitch_semi[t->fine + 1] - snd_pitch_semi[t->fine]) * snd_pitch_fine[x] >> 12;
            }
            if (pitch <= 0) {
                pitch = 1;
            }
            if (pitch > 0x4000) {
                pitch = 0x4000;
            }
            o->volA4 = pitch;
            snd_voice_set_pitch(v, o->volA4);
            x = t->send * 0x1021 >> 7;
            o->s.u.v.send = x;
            if (x != 0x1000) {
                x = x * x >> 12;
            }
            o->s.u.v.b8 = x;
            o->s.u.v.send_pan = 0x40;
            snd_voice_set_volume(v, 0, 0, 0, 0);
            if (snd_bank[4] <= 0x20) {
                a1 = t->ar << 8 | (t->dr >> 3) << 4 | t->sl >> 3;
                if (t->sr_dir != 0) {
                    a1 |= 0x8000;
                }
                a2 = (0x7F - t->sr) << 6 | t->rr >> 2;
                if (t->send != 1) {
                    a2 |= 0x20;
                }
                switch (t->sr_mode) {
                case 1:
                    a2 |= 0x4000;
                    break;
                case 2:
                    a2 |= 0x2000;
                    break;
                case 3:
                    a2 |= 0xC000;
                    break;
                }
                snd_voice_set_simple_adsr(v, a1, a2);
            } else {
                snd_voice_set_simple_adsr(v, *(unsigned short *)&t->sr, *(unsigned short *)&t->sr_dir);
            }
            slot->obj = o;
            slot->b8 = n;
            slot->angle = t->angle;
            slot->voice = v;
            slot->pad7 = t->vol;
            x = d->vol_hi - d->vol_lo;
            if (x != 0) {
                x = t->vol + (d->vol_lo + snd_rand_range(x));
                if (x < 0) {
                    x = 0;
                }
                if (x >= 0x80) {
                    slot->pad7 = 0x7F;
                } else {
                    slot->pad7 = x;
                }
            }
            slot->flags = 0;
            p += 8;
            n++;
        }
        break;
    case 3:
    case 4:
        s = &o->s;
        hdr = (int *)(grp + *(int *)(grp + 0x14) + d->data);
        e = &d->e4;
        at = sceAtracSetDataAndGetID(hdr + 1, *hdr);
        if (at < 0) {
            return -(at != (int)0x80630003);
        }
        sceAtracGetMaxSample(at, &max);
        s->u.st.c4 = max;
        sceAtracGetSoundSample(at, &s->u.st.end, &s->u.st.loop, &s->u.st.loop2);
        s->u.st.end++;
        if (s->u.st.loop2 >= 0) {
            s->u.st.loop2++;
            if (s->u.st.loop < 0) {
                s->u.st.loop = 0;
            }
            if (s->u.st.loop2 < s->pos) {
                s->pos = s->decoded = s->u.st.loop;
            }
        } else {
            s->u.st.loop = -1;
            if (s->u.st.end < s->pos) {
                s->pos = s->decoded = s->u.st.end;
            }
        }
        sceAtracSetLoopNum(at, -1);
        s->e4 = e[0];
        s->e5 = e[1];
        s->atrac = at;
        *(int **)(o->pad00 + 0x10) = hdr + 1;
        snd_voice_clear(o);
        break;
    default:
        return -1;
    }
    o->fade[5].cur = 0;
    o->fade[5].count = o->fade_in;
    snd_duck_update();
    return 1;
}
#endif
