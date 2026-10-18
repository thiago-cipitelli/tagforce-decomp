// @range 0x0885C688-0x0885D078
// @asm 0x0885CB30-0x0885CEC4
/* Sound library, mixer: per-frame fader update and SAS voice parameters, key on and end detection
   of the SAS voices, the echo ring buffer (0x3000 stereo frames) the streams feed and the output
   reads back at half level, stream filters, ordering of the active streams and the level of a block. */
#include "psp.h"
#include "libc.h"
#include "snd.h"

typedef struct SndStreamRef {          /* snd_streams_sorted output */
    SndObj *obj;
    int left;                          /* samples still decodable (+0x4000 when not looping) */
} SndStreamRef;

extern SndList snd_stream_list;        /* 0x08C1AFEC */
extern SndList snd_active;             /* 0x08C1B004 */
extern SndFader snd_bgm_master;        /* 0x08C1B010 */
extern SndVoiceSlot snd_voice_slots[32]; /* 0x08C1B058 */
extern int snd_echo_pos;               /* 0x08BEA49C in frames */
extern short snd_echo_buf[0x6000];     /* 0x08BEA4A0 */

int snd_fader_update(SndFader *f);
int snd_obj_volume(SndObj *o, int sq);
int snd_obj_pitch(SndObj *o);
void snd_obj_pan_lr(SndObj *o, short *l, short *r, int ang);
void snd_obj_send_lr(SndObj *o, short *l, short *r);
int snd_voice_set_volume(int v, int l, int r, int el, int er);
int snd_voice_set_pitch(int v, int pitch);
int snd_voice_key_on(int v);
unsigned int snd_sas_end_flags(void);
void snd_voice_free(unsigned int mask, int reset);

void snd_faders_update(void)
{
    SndObj *o;
    int master = 0;
    SndFader *m = &snd_bgm_master;
    SndFader *f;
    unsigned int upd;
    int i;

    if (snd_bgm_master.cur != snd_bgm_master.target) {
        snd_fader_update(m);
        master = 1;
    }
    o = 0;
    for (;;) {
        o = snd_list_next(&snd_active, o);
        if (o == 0) {
            break;
        }
        upd = o->state & 0x100;
        if (master && !(o->id & 0x100000)) {
            upd = 0x100;
        }
        if (upd) {
            for (f = o->fade, i = 0; i < 8; i++, f++) {
                snd_fader_update(f);
            }
            o->volA2 = snd_obj_volume(o, 0);
            o->volA6 = snd_obj_pitch(o);
        }
    }
}

void snd_voice_apply(SndObj *o, SndVoiceSlot *v)
{
    short l;
    short r;
    short el;
    short er;

    snd_obj_pan_lr(o, &l, &r, v->angle);
    snd_obj_send_lr(o, &el, &er);
    if (!(o->state & 0x10)) {
        snd_voice_set_volume(v->voice, l, r, el, er);
        snd_voice_set_pitch(v->voice, snd_obj_pitch(o));
    }
}

/* Keys on newly started voices and frees the ones the SAS reports finished. */
void snd_voices_update(void)
{
    SndVoiceSlot *v;
    int i;
    unsigned int bit;
    SndObj *o;
    unsigned int end;

    bit = 1;
    for (v = snd_voice_slots, i = 0; i < 32; i++, v++, bit <<= 1) {
        o = v->obj;
        if (o != 0 && (o->state & 0x2000) && !(o->state & 0x10)) {
            if (!(v->flags & 0x40)) {
                v->flags |= 0x40;
                snd_voice_apply(o, v);
                snd_voice_key_on(i);
            } else {
                o->frames++;
                if (o->state & 0x100) {
                    snd_voice_apply(o, v);
                }
                end = snd_sas_end_flags();
                if (end & bit) {
                    o->s.atrac |= bit;
                    snd_voice_free(bit, 1);
                }
            }
        }
    }
}

/* Adds 0x100 frames of `in` to the echo buffer at the object's delay and level. */
void snd_echo_add(SndObj *o, short *in)
{
    int lvl = o->s.e4 * 0x40A0 >> 14;
    int pos = (o->s.e5 * 0x40A0 >> 14) * 0x2F00 >> 7;
    short *p;
    int n;

    pos += snd_echo_pos;
    if (pos >= 0x3000) {
        pos -= 0x3000;
    }
    p = &snd_echo_buf[pos * 2];
    n = 0x200;
    if (pos + 0x100 > 0x3000) {
        n = (0x3000 - pos) * 2;
        while (--n >= 0) {
            int v = *in * lvl >> 7;

            if (v < 0) {
                v++;
            }
            v += *p;
            if (v >= 0x8000) {
                v = 0x7FFF;
            } else if (v < -0x8000) {
                v = -0x8000;
            }
            *p = v;
            in++;
            p++;
        }
        n = (pos - 0x2F00) * 2;
        p = snd_echo_buf;
    }
    while (--n >= 0) {
        int v = *in * lvl >> 7;

        if (v < 0) {
            v++;
        }
        v += *p;
        if (v >= 0x8000) {
            v = 0x7FFF;
        } else if (v < -0x8000) {
            v = -0x8000;
        }
        *p = v;
        in++;
        p++;
    }
}

/* Mixes the echo at the read position into `out` (0x100 frames), halving what stays in the buffer. */
void snd_echo_mix(short *out)
{
    int i;
    int v;
    short *p = &snd_echo_buf[snd_echo_pos * 2];

    for (i = 0; i < 0x200; i++) {
        v = *p >> 1;
        if (v < 0) {
            v++;
        }
        *p = v;
        v += *out;
        if (v >= 0x8000) {
            v = 0x7FFF;
        } else if (v < -0x8000) {
            v = -0x8000;
        }
        *out = v;
        p++;
        out++;
    }
    i = snd_echo_pos + 0x100;
    if (i >= 0x3000) {
        i = 0;
    }
    snd_echo_pos = i;
}

#ifdef NON_MATCHING /* clamp branches written per sign in the original */
static int clamp16(int v)
{
    return v < -0x8000 ? -0x8000 : v > 0x7FFF ? 0x7FFF : v;
}

/* Widens the stereo image: adds the L-R difference of the frame two back; `hist` carries the last two
   differences across blocks. */
void snd_stereo_widen(short *hist, short *buf, int n)
{
    short save[2];
    short *p;
    int i;
    int d;

    for (i = 0, p = buf + (n - 2) * 2; i < 2; i++, p += 2) {
        save[i] = clamp16(p[0] - p[1]);
    }
    for (p = buf + (n - 1) * 2; p >= buf + 4; p -= 2) {
        d = p[-4] - p[-3];
        p[0] = clamp16(p[0] + d);
        p[1] = clamp16(p[1] - d);
    }
    for (i = 1; i >= 0; i--, p -= 2) {
        p[0] = clamp16(p[0] + hist[i]);
        p[1] = clamp16(p[1] - hist[i]);
        hist[i] = save[i];
    }
}

/* Box filter of `taps` frames over `n` stereo frames; `hist` holds the 16 frames before the block. */
void snd_box_filter(short *hist, short *buf, int n, int taps)
{
    short save[32];
    int k;
    int j;
    int idx;
    int l;
    int r;

    memcpy(save, buf + (n - 16) * 2, sizeof save);
    if (taps != 0) {
        for (k = (n - 1) * 2; k >= 0; k -= 2) {
            l = 0;
            r = 0;
            for (j = 0, idx = k; j < taps; j++, idx -= 2) {
                if (idx < 0) {
                    l += hist[idx + 32];
                    r += hist[idx + 33];
                } else {
                    l += buf[idx];
                    r += buf[idx + 1];
                }
            }
            buf[k] = l / taps;
            buf[k + 1] = r / taps;
        }
    }
    memcpy(hist, save, sizeof save);
}
#endif

/* Playing streams ordered by samples left, 0-terminated; returns their number. */
int snd_streams_sorted(SndStreamRef *out)
{
    int n = 0;
    SndObj *o = 0;
    SndStreamRef *r = out;
    SndObj *t;
    int left;
    int i;
    int j;

    for (;;) {
        o = snd_list_next(&snd_stream_list, o);
        if (o == 0) {
            break;
        }
        if (o->index >= 0 && (o->state & 0x2000)) {
            r->obj = o;
            r->left = o->s.decoded - o->s.pos;
            if (!(o->s.sflags & 0x40)) {
                r->left += 0x4000;
            }
            r++;
            n++;
        }
    }
    switch (n) {
    case 0:
    case 1:
        break;
    case 2:
        left = out[0].left;
        if (out[1].left < left) {
            t = out[0].obj;
            out[0].obj = out[1].obj;
            out[0].left = out[1].left;
            out[1].obj = t;
            out[1].left = left;
        }
        break;
    default:
        for (i = 0; i < n - 1; i++) {
            for (j = i + 1; j < n; j++) {
                left = out[i].left;
                if (out[j].left < left) {
                    t = out[i].obj;
                    out[i].obj = out[j].obj;
                    out[i].left = out[j].left;
                    out[j].obj = t;
                    out[j].left = left;
                }
            }
        }
        break;
    }
    out[n].obj = 0;
    return n;
}

/* Mean absolute level of the left channel of 0x100 stereo frames. */
int snd_block_level(short *p)
{
    int sum;
    int v;
    int n;

    n = 0x100;
    sum = 0;
    do {
        v = *p;
        if (v < 0) {
            v = -v;
        }
        n--;
        sum += v;
        p += 2;
    } while (n > 0);
    return sum >> 8;
}
