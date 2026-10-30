// @range 0x0885C688-0x0885D948
// @asm 0x0885CCE0-0x0885CEC4
// @asm 0x0885D078-0x0885D948
/* Sound library, mixer: per-frame fader update and SAS voice parameters, key on and end detection
   of the SAS voices, the echo ring buffer (0x3000 stereo frames) the streams feed and the output
   reads back at half level, stream filters, ordering of the active streams, the level of a block and
   the Atrac stream decoder that fills each stream's PCM ring and mixes it into the output block. */
#include "psp.h"
#include "libc.h"
#include "snd.h"

typedef struct SndStreamRef {          /* snd_streams_sorted output */
    SndObj *obj;
    int left;                          /* samples still decodable (+0x4000 when not looping) */
} SndStreamRef;

extern SndList snd_stream_list;        /* 0x08C1AFEC */
extern SndList snd_sas_list;           /* 0x08C1AFF8 */
extern SndList snd_active;             /* 0x08C1B004 */
extern SndFader snd_bgm_master;        /* 0x08C1B010 */
extern SndVoiceSlot snd_voice_slots[32]; /* 0x08C1B058 */
extern int snd_echo_pos;               /* 0x08BEA49C in frames */
extern short snd_echo_buf[0x6000];     /* 0x08BEA4A0 */
extern short snd_filter_hist[][16][2]; /* 0x08BF64E8 per stream */
extern short snd_mix_buf[2][0x200];    /* 0x08BF6680 */
extern int snd_mix_idx;                /* 0x08BF6E80 */
extern unsigned char snd_voice_pcm[][0x6000]; /* 0x08BF6EC0 per stream: 0x1000-frame ring + overflow */
extern int snd_atrac_sema;             /* 0x08C1B048 */

int snd_fader_update(SndFader *f);
int snd_obj_volume(SndObj *o, int sq);
int snd_obj_pitch(SndObj *o);
void snd_obj_pan_lr(SndObj *o, short *l, short *r, int ang);
void snd_obj_send_lr(SndObj *o, short *l, short *r);
int snd_voice_set_volume(int v, int l, int r, int el, int er);
int snd_voice_set_pitch(int v, int pitch);
int snd_voice_key_on(int v);
unsigned int snd_sas_end_flags(void);
int snd_fader8_update(SndFader8 *f);
int sceAtracResetPlayPosition(int id, int sample, int a, int b);
int sceAtracSetLoopNum(int id, int n);
int sceAtracGetLoopStatus(int id, int *loops, int *status);
int sceAtracDecodeData(int id, short *out, int *n, int *end, int *remain);
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

/* Widens the stereo image: adds the L-R difference of the frame two back; `hist` carries the last two
   differences across blocks. Each sum is clamped to 16 bits on the side its sign can overflow. */
void snd_stereo_widen(short *hist, short *buf, int n)
{
    short save[2];
    short *p;
    int i;
    short *s;
    int d;
    int a;
    int b;

    for (s = buf + (n - 2) * 2, i = 0; i < 2; i++, s += 2) {
        a = s[0] - s[1];
        if (a < -0x8000)
            a = -0x8000;
        else if (a > 0x7FFF)
            a = 0x7FFF;
        save[i] = a;
    }
    for (p = buf + (n - 1) * 2; p >= buf + 4; p -= 2) {
        d = p[-4] - p[-3];
        a = p[0] + d;
        b = p[1] - d;
        if (d >= 0) {
            if (a > 0x7FFF)
                a = 0x7FFF;
            p[0] = a;
            if (b < -0x8000)
                b = -0x8000;
            p[1] = b;
        } else {
            if (a < -0x8000)
                a = -0x8000;
            p[0] = a;
            if (b > 0x7FFF)
                b = 0x7FFF;
            p[1] = b;
        }
    }
    for (i = 1; i >= 0; i--, p -= 2) {
        a = p[0] + hist[i];
        if (a < -0x8000)
            a = -0x8000;
        else if (a > 0x7FFF)
            a = 0x7FFF;
        p[0] = a;
        b = p[1] - hist[i];
        if (b < -0x8000)
            b = -0x8000;
        else if (b > 0x7FFF)
            b = 0x7FFF;
        p[1] = b;
        hist[i] = save[i];
    }
}

#ifdef NON_MATCHING /* lógica corrigida (quociente negativo +1, laço interno do-while); 31 palavras: só registradores (o original dá t1 ao k externo, t0 a i e t4/t5/t6 a idx/r/l), nenhuma ordem de declaração resolve */
/* Box filter of `taps` frames over `n` stereo frames; `hist` holds the 16 frames before the block. */
void snd_box_filter(short *hist, short *buf, int n, int taps)
{
    short save[32];
    int k;
    int idx;
    int l;
    int r;
    int i;

    for (i = 0, k = (n - 16) * 2; i < 32; i++, k++) {
        save[i] = buf[k];
    }
    if (taps != 0) {
        for (k = (n - 1) * 2; k >= 0; k -= 2) {
            l = 0;
            r = 0;
            i = 0;
            idx = k;
            do { /* no test before the first pass: one frame even when taps < 0 */
                if (idx >= 0) {
                    l += buf[idx];
                    r += buf[idx + 1];
                } else {
                    short *h = &hist[idx + 32];

                    l += h[0];
                    r += h[1];
                }
                i++;
                idx -= 2;
            } while (i < taps);
            /* quotients below zero are bumped by one (as the original) */
            r /= taps;
            if (r < 0)
                r++;
            l /= taps;
            if (l < 0)
                l++;
            buf[k] = l;
            buf[k + 1] = r;
        }
    }
    for (i = 0; i < 32; i++) {
        hist[i] = save[i];
    }
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

#ifdef NON_MATCHING /* the original keeps $t7/$t8/$v0 live across the calls to snd_stereo_widen and
   snd_box_filter (which leave them untouched); mwcc build 219 with our flags always saves them in
   $s registers or on the stack, so the register allocation and frame cannot match */
/* Decodes the active Atrac streams into their PCM rings (0x1000 frames, decoded ahead by up to
   `budget` frames, less when many SAS voices are active) and mixes 0x100 frames of each into the
   current output block: copied when it is the only stream of the first pass, else added with
   saturation. Handles the loop points and the end of the stream. */
void snd_streams_decode(int mix)
{
    SndStreamRef list[7];
    int loops;
    int tmp;
    short r;
    short l;
    int remain;
    int end;
    int decoded;
    int n;
    int budget;
    SndStreamRef *e;
    SndObj *o;
    SndStream *s;
    int id;
    int v;
    unsigned int *pcm;
    unsigned int *pcm_hi;
    short (*hist)[2];
    int loop_end;
    int rd0;
    int filled;
    unsigned int *dst;
    unsigned int *a;
    unsigned int *b;
    int cnt;
    int wrap;
    int rd;
    int x;
    int lo;

    sceKernelWaitSema(snd_atrac_sema, 1, 0);
    tmp = snd_sas_list.count;
    budget = 0xC00;
    if (tmp >= 4) {
        budget = 0x800;
        if (tmp >= 8) {
            budget = 0;
        }
    }
    n = snd_streams_sorted(list);
    for (e = list; (o = e->obj) != 0; e++) {
        if (o->state & 0x10) {
            continue;
        }
        v = o->index - 0x1C;
        snd_obj_pan_lr(o, &l, &r, 0x40);
        id = o->s.atrac;
        s = &o->s;
        if (!(s->sflags & 0x80)) {
            s->sflags |= 0x80;
            if (s->u.st.end < s->pos) {
                tmp = s->u.st.loop;
                if (tmp < 0) {
                    tmp = 0;
                }
                s->pos = s->decoded = tmp;
            }
            sceAtracResetPlayPosition(id, s->pos, 0, 0);
            sceAtracSetLoopNum(id, -1);
            s->u.st.start = 0;
            s->wr = s->u.st.rd = 0x800;
        }
        if (s->e7) {
            s->e7 = 0;
            sceAtracSetLoopNum(id, 0);
        }
        sceAtracGetLoopStatus(id, &loops, &tmp);
        loop_end = s->u.st.end;
        if (loops < 0) {
            loop_end = s->u.st.loop2;
        }
        if (s->pos >= s->u.st.end) {
            continue;
        }
        if (s->fade8.cur != s->fade8.target) {
            snd_fader8_update(&s->fade8);
        }
        rd0 = s->u.st.rd;
        filled = 0;
        pcm = (unsigned int *)snd_voice_pcm[v];
        pcm_hi = (unsigned int *)(snd_voice_pcm[v] + 0x4000);
        hist = snd_filter_hist[v];
        do {
            decoded = 0;
            if (s->sflags & 1) {
                goto out;
            }
            if (e->left < 0x100 || budget - s->u.st.c4 > 0) {
                tmp = s->decoded - s->pos;
                if ((s->sflags & 0x40) && s->u.st.c4 < tmp) {
                    goto out;
                }
                v = s->wr;
                dst = pcm + v;
                tmp = sceAtracDecodeData(id, (short *)dst, &decoded, &end, &remain);
                if (tmp != 0 && tmp != (int)0x80630024) {
                    goto next;
                }
                if (o->state & 0x20) {
                    tmp = snd_block_level((short *)dst);
                    if (s->sflags & 0x10) {
                        s->sflags &= ~0x10;
                        s->meter = tmp;
                    } else {
                        tmp += s->meter;
                        s->meter = tmp >> 1;
                    }
                }
                s->decoded += decoded;
                v += decoded;
                budget = -1;
                if (v > 0x1000) {
                    v -= 0x1000;
                    memcpy(pcm, pcm_hi, v * 4);
                    s->sflags |= 0x40;
                }
                s->wr = v;
            }
        out:
            if (!(s->sflags & 0x40)) {
                goto next;
            }
            rd = s->u.st.rd;
            b = pcm + rd;
            dst = (unsigned int *)snd_mix_buf[snd_mix_idx] + filled;
            tmp = 0x100 - filled;
            cnt = s->decoded - s->pos;
            a = b;
            wrap = 0;
            if (tmp < cnt) {
                cnt = tmp;
            }
            rd += cnt;
            s->pos += cnt;
            filled += cnt;
            if (rd >= 0x1000) {
                rd -= 0x1000;
                b = pcm;
                wrap = rd;
                cnt -= rd;
            }
            s->u.st.rd = rd;
            if (s->e6 == 2) {
                snd_stereo_widen((short *)hist, (short *)a, cnt);
            } else if (s->e6 == 1 && s->fade8.cur != 0) {
                snd_box_filter((short *)hist, (short *)a, cnt, s->fade8.cur);
            }
            if (r != 0 || l != 0) {
                if (mix == 0 && n == 1) {
                    if (r == 0x1000 && l == 0x1000) {
                        while (--cnt >= 0) {
                            *dst++ = *a++;
                        }
                        while (--wrap >= 0) {
                            *dst++ = *b++;
                        }
                    } else {
                        while (--cnt >= 0) {
                            x = *a;
                            lo = (short)x * l >> 12;
                            if (lo < 0) {
                                lo++;
                            }
                            tmp = (x >> 16) * r >> 12;
                            if (tmp < 0) {
                                tmp++;
                            }
                            *dst = (lo & 0xFFFF) | tmp << 16;
                            dst++;
                            a++;
                        }
                        while (--wrap >= 0) {
                            x = *b;
                            lo = (short)x * l >> 12;
                            if (lo < 0) {
                                lo++;
                            }
                            tmp = (x >> 16) * r >> 12;
                            if (tmp < 0) {
                                tmp++;
                            }
                            *dst = (lo & 0xFFFF) | tmp << 16;
                            dst++;
                            b++;
                        }
                    }
                } else {
                    while (--cnt >= 0) {
                        x = *a;
                        lo = (short)x * l >> 12;
                        if (lo < 0) {
                            lo++;
                        }
                        lo += ((short *)dst)[0];
                        if (lo < -0x8000) {
                            lo = -0x8000;
                        } else if (lo > 0x7FFF) {
                            lo = 0x7FFF;
                        }
                        tmp = (x >> 16) * r >> 12;
                        if (tmp < 0) {
                            tmp++;
                        }
                        tmp += ((short *)dst)[1];
                        if (tmp < -0x8000) {
                            tmp = -0x8000;
                        } else if (tmp > 0x7FFF) {
                            tmp = 0x7FFF;
                        }
                        *dst = (lo & 0xFFFF) | tmp << 16;
                        dst++;
                        a++;
                    }
                    while (--wrap >= 0) {
                        x = *b;
                        lo = (short)x * l >> 12;
                        if (lo < 0) {
                            lo++;
                        }
                        lo += ((short *)dst)[0];
                        if (lo < -0x8000) {
                            lo = -0x8000;
                        } else if (lo > 0x7FFF) {
                            lo = 0x7FFF;
                        }
                        tmp = (x >> 16) * r >> 12;
                        if (tmp < 0) {
                            tmp++;
                        }
                        tmp += ((short *)dst)[1];
                        if (tmp < -0x8000) {
                            tmp = -0x8000;
                        } else if (tmp > 0x7FFF) {
                            tmp = 0x7FFF;
                        }
                        *dst = (lo & 0xFFFF) | tmp << 16;
                        dst++;
                        b++;
                    }
                }
            }
            tmp = s->decoded - s->u.st.start - loop_end;
            if (tmp >= 0) {
                if (loops == 0) {
                    s->sflags |= 1;
                } else {
                    s->u.st.start = s->u.st.loop2 - s->u.st.loop;
                }
            }
            if (s->pos >= s->u.st.end && loops == 0 && filled < 0x100) {
                do {
                    *dst++ = 0;
                } while (++filled < 0x100);
            }
        } while (filled < 0x100);
        o->frames++;
        if (s->decoded >= loop_end && s->pos >= loop_end) {
            tmp = s->u.st.start;
            s->pos -= tmp;
            s->decoded -= tmp;
            s->u.st.start = 0;
        }
        if (s->e4) {
            snd_echo_add(o, (short *)(pcm + rd0));
        }
    next:;
    }
    sceKernelSignalSema(snd_atrac_sema, 1);
    snd_echo_mix(snd_mix_buf[snd_mix_idx]);
}
#endif
