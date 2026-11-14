/* sceSasCore (port step 6, docs/port_survey.md): the PSP's 32-voice software mixer, after PPSSPP's Core/HW/SasAudio.cpp.
   VAG ADPCM voices (16-byte blocks: filter/shift byte, flag byte, 28 nibbles; flag 7 ends, 6 marks the loop start,
   3 loops when looping is on), linear-interpolated resampling at pitch/0x1000, ADSR envelope of 0..0x40000000
   stepped once per output sample (the "simple" PS1-style ADSR1/ADSR2 words, or rates and curve modes set apart),
   volumes in 1/0x1000. The core writes one grain (256 stereo frames here) per __sceSasCore; CoreWithMix adds it to
   the samples already there. The game ignores the reverb (dry only), so the Rev* calls just answer 0.
   ponytail: no 32-sample key-on delay, no noise voices, no PCM voices, no reverb; the game uses none of them. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { A_LIN_INC, A_LIN_DEC, A_LIN_BENT, A_EXP_DEC, A_EXP_INC, A_DIRECT };
enum { S_OFF, S_ATTACK, S_DECAY, S_SUSTAIN, S_RELEASE };
#define HMAX 0x40000000
#define VOICES 32
#define MAX_GRAIN 2048

typedef struct {
    const unsigned char *vag, *rd;
    int blocks, block, loop, loop_block, loop_next, end, cur;
    int s1, s2;
    short samples[28];
} Vag;

typedef struct {
    int on, playing, paused;
    int pitch, vol_l, vol_r, fx_l, fx_r;
    int a_type, d_type, s_type, r_type, a_rate, d_rate, s_rate, r_rate, s_level;
    int state;
    long long height;
    unsigned int frac;
    short hist[2];
    Vag vag;
} Voice;

static Voice voice[VOICES];
static int grain = 256;
static int trace = -1;   /* env TAGFORCE_TRACE_SAS: key-ons and the first mixed grains */
static int mix[MAX_GRAIN * 2];
static short tmp[MAX_GRAIN * 4 + 8];

static const unsigned char f[16][2] = { { 0, 0 }, { 60, 0 }, { 115, 52 }, { 98, 55 }, { 122, 60 }, { 0, 0 }, { 0, 0 },
    { 52, 0 }, { 55, 2 }, { 60, 125 }, { 0, 0 }, { 0, 91 }, { 0, 0 }, { 2, 216 }, { 125, 6 }, { 0, 151 } };

static int clamp16(int v) { return v < -0x8000 ? -0x8000 : v > 0x7FFF ? 0x7FFF : v; }

/* --- VAG --- */

static void vag_start(Vag *v, const unsigned char *data, int size, int loop)
{
    v->vag = v->rd = data;
    v->blocks = size / 16;
    v->block = -1;
    v->loop = loop;
    v->loop_block = -1;
    v->loop_next = 0;
    v->end = 0;
    v->cur = 28;
    v->s1 = v->s2 = 0;
}

static void vag_block(Vag *v)
{
    const unsigned char *p = v->rd;
    int pred, shift, flags, c1, c2, i, s1 = v->s1, s2 = v->s2;

    if (v->block == v->blocks - 1) {
        v->end = 1;
        return;
    }
    pred = *p++;
    shift = pred & 0xF;
    pred >>= 4;
    flags = *p++;
    if (flags == 7) {
        v->end = 1;
        return;
    }
    if (flags == 6)
        v->loop_block = v->block;
    else if (flags == 3 && v->loop)
        v->loop_next = 1;
    c1 = f[pred][0];
    c2 = -f[pred][1];
    for (i = 0; i < 28; i += 2) {
        unsigned char d = *p++;
        int a = (short)((d & 0xF) << 12) >> shift, b = (short)((d & 0xF0) << 8) >> shift;

        s2 = clamp16(a + ((s1 * c1 + s2 * c2) >> 6));
        s1 = clamp16(b + ((s2 * c1 + s1 * c2) >> 6));
        v->samples[i] = s2;
        v->samples[i + 1] = s1;
    }
    v->s1 = s1;
    v->s2 = s2;
    v->cur = 0;
    v->block++;
    v->rd = p;
}

static void vag_read(Vag *v, short *out, int n)
{
    int i;

    for (i = 0; i < n; i++) {
        if (v->end) {
            memset(out + i, 0, (n - i) * 2);
            return;
        }
        if (v->cur == 28) {
            if (v->loop_next) {
                v->rd = v->vag + 16 * v->loop_block + 16;
                v->block = v->loop_block;
                v->loop_next = 0;
            }
            vag_block(v);
            if (v->end) {
                memset(out + i, 0, (n - i) * 2);
                return;
            }
        }
        out[i] = v->samples[v->cur++];
    }
}

/* --- ADSR --- */

static int simple_rate(int n)
{
    int r;

    n &= 0x7F;
    if (n == 0x7F)
        return 0;
    r = ((7 - (n & 3)) << 26) >> (n >> 2);
    return r ? r : 1;
}

static int exp_rate(int n)
{
    int r;

    n &= 0x7F;
    if (n == 0x7F)
        return 0;
    r = ((7 - (n & 3)) << 24) >> (n >> 2);
    return r ? r : 1;
}

static void simple_adsr(Voice *v, unsigned int e1, unsigned int e2)
{
    int n;

    v->a_rate = simple_rate(e1 >> 8);
    v->a_type = e1 & 0x8000 ? A_LIN_BENT : A_LIN_INC;
    n = (e1 >> 4) & 0xF;
    v->d_rate = n == 0 ? 0x7FFFFFFF : (int)(0x80000000u >> n);
    v->d_type = A_EXP_DEC;
    v->s_type = (e2 >> 14) & 3;
    v->s_rate = v->s_type == A_EXP_DEC ? exp_rate(e2 >> 6) : simple_rate(e2 >> 6);
    v->r_type = e2 & 0x20 ? A_EXP_DEC : A_LIN_DEC;
    n = e2 & 0x1F;
    if (n == 31)
        v->r_rate = 0;
    else if (v->r_type == A_LIN_DEC)
        v->r_rate = n == 30 ? 0x40000000 : n == 29 ? 1 : 0x10000000 >> n;
    else
        v->r_rate = n == 0 ? 0x7FFFFFFF : (int)(0x80000000u >> n);
    v->s_level = ((e1 & 0xF) + 1) << 26;
}

static void walk(Voice *v, int type, int rate)
{
    long long d;

    switch (type) {
    case A_LIN_INC: v->height += rate; break;
    case A_LIN_DEC: v->height -= rate; break;
    case A_LIN_BENT: v->height += v->height <= (long long)HMAX * 3 / 4 ? rate : rate / 4; break;
    case A_EXP_DEC:
        d = v->height - HMAX;
        d += (-d * rate) >> 32;
        v->height = d + HMAX - (rate + 3ULL) / 4ULL;
        break;
    case A_EXP_INC:
        d = v->height - HMAX;
        d += (-d * rate) >> 32;
        v->height = d + 0x4000 + HMAX;
        break;
    case A_DIRECT: v->height = rate; break;
    }
}

static void set_state(Voice *v, int s)
{
    if (v->height > HMAX)
        v->height = HMAX;
    v->state = s;
}

static void env_step(Voice *v)
{
    switch (v->state) {
    case S_ATTACK:
        walk(v, v->a_type, v->a_rate);
        if (v->height >= HMAX || v->height < 0)
            set_state(v, S_DECAY);
        break;
    case S_DECAY:
        walk(v, v->d_type, v->d_rate);
        if (v->height < v->s_level)
            set_state(v, S_SUSTAIN);
        break;
    case S_SUSTAIN:
        walk(v, v->s_type, v->s_rate);
        if (v->height <= 0) {
            v->height = 0;
            set_state(v, S_RELEASE);
        }
        break;
    case S_RELEASE:
        walk(v, v->r_type, v->r_rate);
        if (v->height <= 0) {
            v->height = 0;
            set_state(v, S_OFF);
        }
        break;
    }
}

/* --- mixing --- */

static void mix_voice(Voice *v, int keyed)
{
    int need, pos = 2, i, end;
    unsigned int frac = v->frac;

    if (!v->vag.vag)
        return;
    tmp[0] = v->hist[0];
    tmp[1] = v->hist[1];
    need = (frac + v->pitch * grain) >> 12;
    if (keyed) {
        pos = 0;
        need += 2;
    }
    vag_read(&v->vag, tmp + pos, need);
    end = pos + need;
    for (i = 0; i < grain; i++) {
        const short *s = tmp + (frac >> 12);
        int sample = s[0] - (((s[0] - s[1]) * (int)(frac & 0xFFF)) >> 12), e;

        frac += v->pitch;
        e = v->height < 0 ? 0 : (int)v->height;
        env_step(v);
        e = (e + (1 << 14)) >> 15;
        sample = (sample * e + (1 << 14)) >> 15;
        mix[i * 2] += sample * v->vol_l >> 12;
        mix[i * 2 + 1] += sample * v->vol_r >> 12;
    }
    v->hist[0] = tmp[end - 2];
    v->hist[1] = tmp[end - 1];
    v->frac = frac - (end - 2) * 0x1000;
    if (v->vag.end) {
        v->state = S_OFF;
        v->height = 0;
    }
    if (v->state == S_OFF)
        v->playing = v->on = 0;
}

static void core(short *out, const short *in, int l, int r)
{
    int i;

    memset(mix, 0, grain * 8);
    for (i = 0; i < VOICES; i++)
        if (voice[i].playing && !voice[i].paused) {
            int keyed = voice[i].on == 2;

            voice[i].on = 1;
            mix_voice(&voice[i], keyed);
        }
    if (trace > 0) {
        static int shown;
        int pk = 0;

        for (i = 0; i < grain * 2; i++)
            pk = abs(mix[i]) > pk ? abs(mix[i]) : pk;
        if (pk && shown < 20) {
            shown++;
            fprintf(stderr, "sas: grain peak %d\n", pk);
        }
    }
    for (i = 0; i < grain * 2; i += 2) {
        int sl = mix[i], sr = mix[i + 1];

        if (in) {
            sl += in[i] * l >> 12;
            sr += in[i + 1] * r >> 12;
        }
        out[i] = clamp16(sl);
        out[i + 1] = clamp16(sr);
    }
}

/* --- API --- */

#define BAD_VOICE ((int)0x80420010)
#define BAD_PITCH ((int)0x80420012)
#define BAD_VOLUME ((int)0x80420016)
#define BAD_SIZE ((int)0x80420014)

static Voice *vget(int i) { return i >= 0 && i < VOICES ? &voice[i] : 0; }

int __sceSasInit(void *c, int g, int max_voices, int out_mode, int freq)
{
    int i;

    grain = g > 0 && g <= MAX_GRAIN ? g : 256;
    memset(voice, 0, sizeof voice);
    for (i = 0; i < VOICES; i++) {
        voice[i].pitch = 0x1000;
        voice[i].vol_l = voice[i].vol_r = voice[i].fx_l = voice[i].fx_r = 0x1000;
    }
    return 0;
}

int __sceSasCore(void *c, void *out)
{
    core(out, 0, 0, 0);
    return 0;
}

int __sceSasCoreWithMix(void *c, void *inout, int l, int r)
{
    core(inout, inout, l, r);
    return 0;
}

int __sceSasSetVoice(void *c, int i, const void *vag, int size, int loop)
{
    Voice *v = vget(i);

    if (!v)
        return BAD_VOICE;
    if (size <= 0 || (size & 0xF))
        return BAD_SIZE;
    v->vag.vag = vag;
    v->vag.blocks = size / 16;
    v->vag.loop = loop;
    return 0;
}

int __sceSasSetKeyOn(void *c, int i)
{
    Voice *v = vget(i);

    if (!v)
        return BAD_VOICE;
    if (!v->vag.vag)
        return 0;
    vag_start(&v->vag, v->vag.vag, v->vag.blocks * 16, v->vag.loop);
    if (trace < 0)
        trace = getenv("TAGFORCE_TRACE_SAS") != 0;
    if (trace)
        fprintf(stderr, "sas: key on %d: %d blocks loop %d pitch %#x vol %d/%d adsr a%d/%d d%d/%d s%d/%d r%d/%d sl %#x\n", i,
            v->vag.blocks, v->vag.loop, v->pitch, v->vol_l, v->vol_r, v->a_type, v->a_rate, v->d_type, v->d_rate, v->s_type,
            v->s_rate, v->r_type, v->r_rate, v->s_level);
    v->height = 0;
    v->state = S_ATTACK;
    v->playing = 1;
    v->on = 2;   /* first grain: the resampler primes its history from the data */
    v->paused = 0;
    v->frac = 0;
    v->hist[0] = v->hist[1] = 0;
    return 0;
}

int __sceSasSetKeyOff(void *c, int i)
{
    Voice *v = vget(i);

    if (!v)
        return BAD_VOICE;
    v->on = 0;
    set_state(v, S_RELEASE);
    return 0;
}

int __sceSasSetPause(void *c, unsigned int mask, int pause)
{
    int i;

    for (i = 0; i < VOICES; i++)
        if (mask >> i & 1)
            voice[i].paused = pause != 0;
    return 0;
}

int __sceSasGetPauseFlag(void *c)
{
    unsigned int m = 0;
    int i;

    for (i = 0; i < VOICES; i++)
        if (voice[i].paused)
            m |= 1u << i;
    return m;
}

int __sceSasGetEndFlag(void *c)
{
    unsigned int m = 0;
    int i;

    for (i = 0; i < VOICES; i++)
        if (!voice[i].playing)
            m |= 1u << i;
    return m;
}

int __sceSasSetVolume(void *c, int i, int l, int r, int el, int er)
{
    Voice *v = vget(i);

    if (!v)
        return BAD_VOICE;
    if (l < -0x1000 || l > 0x1000 || r < -0x1000 || r > 0x1000 || el < -0x1000 || el > 0x1000 || er < -0x1000 || er > 0x1000)
        return BAD_VOLUME;
    v->vol_l = l;
    v->vol_r = r;
    v->fx_l = el;
    v->fx_r = er;
    return 0;
}

int __sceSasSetPitch(void *c, int i, int pitch)
{
    Voice *v = vget(i);

    if (!v)
        return BAD_VOICE;
    if (pitch < 1 || pitch > 0x4000)
        return BAD_PITCH;
    v->pitch = pitch;
    return 0;
}

int __sceSasSetSimpleADSR(void *c, int i, int e1, int e2)
{
    Voice *v = vget(i);

    if (!v)
        return BAD_VOICE;
    if (e2 >> 13 & 1)
        return (int)0x80420013;
    simple_adsr(v, e1 & 0xFFFF, e2 & 0xFFFF);
    return 0;
}

int __sceSasSetADSR(void *c, int i, int flag, int a, int d, int s, int r)
{
    Voice *v = vget(i);

    if (!v)
        return BAD_VOICE;
    if (flag & 1) v->a_rate = a;
    if (flag & 2) v->d_rate = d;
    if (flag & 4) v->s_rate = s;
    if (flag & 8) v->r_rate = r;
    return 0;
}

int __sceSasSetADSRmode(void *c, int i, int flag, int a, int d, int s, int r)
{
    Voice *v = vget(i);

    if (!v)
        return BAD_VOICE;
    if (flag & 1) v->a_type = a;
    if (flag & 2) v->d_type = d;
    if (flag & 4) v->s_type = s;
    if (flag & 8) v->r_type = r;
    return 0;
}

int __sceSasSetSL(void *c, int i, int level)
{
    Voice *v = vget(i);

    if (!v)
        return BAD_VOICE;
    v->s_level = level;
    return 0;
}

int __sceSasGetEnvelopeHeight(void *c, int i)
{
    Voice *v = vget(i);

    if (!v)
        return BAD_VOICE;
    return v->height > HMAX ? HMAX : v->height < 0 ? 0 : (int)v->height;
}

int __sceSasSetNoise(void *c, int i, int freq) { return 0; }
int __sceSasRevVON(void *c, int dry, int wet) { return 0; }
int __sceSasRevEVOL(void *c, int l, int r) { return 0; }
int __sceSasRevType(void *c, int type) { return 0; }
int __sceSasRevParam(void *c, int delay, int feedback) { return 0; }
