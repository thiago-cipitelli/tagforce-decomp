// @range 0x088634EC-0x08865260
// @gcc 0x088634EC-0x088640A8
// @asm 0x088640A8-0x08864100
// @gcc 0x08864100-0x088642F8
// @asm 0x088642F8-0x08865260
/* libwave (SDK sample library, "SceWaveMain"): 8 software channels of VAG ADPCM or 16-bit PCM
   (mono or stereo, one-shot, looping, or streamed through a callback) mixed by a thread into one
   448-sample stereo hardware channel, plus 4 directly driven hardware channels (wave_set_sample,
   wave_audio_write...). Channel changes while playing (restart, stop, fade) go through a shadow copy
   applied after a 112-sample fade. Compiled with the SDK's GCC (-O3): the `@gcc` ranges come from
   libwave_gcc.c (this C); the rest stays in asm with its C under NON_MATCHING. Error codes 0x8044xxxx. Outside the
   PSP build the C is used with its own state; sceAudio/sceKernel come from the platform layer. */

int wave_ch_stop(unsigned int ch);

#if !defined(__MWERKS__) || defined(NON_MATCHING)
typedef struct WaveCh {
    const unsigned char *data;         /* VAG blocks or PCM samples (0 for a streamed channel) */
    int size;                          /* +0x04 bytes */
    int vol_l;                         /* +0x08 0..4096 */
    int vol_r;                         /* +0x0C */
    int loop_start;                    /* +0x10 VAG loop start (set by the 6 block flag) */
    int loop_end;                      /* +0x14 VAG position of the last loop end (3 flag) */
    unsigned int flags;                /* +0x18 low byte VAG loop mode; 0x001 PCM loop, 0x100 PCM,
                                          0x200 stereo, 0x400 callback stream, 0x10000/0x20000/
                                          0x40000 shadow copy to apply, 0x100000 fading out */
    int playing;                       /* +0x1C */
    int pos;                           /* +0x20 bytes */
    int unk24;
    int unk28;                         /* +0x28 */
    int unk2c;                         /* +0x2C */
    int hist1;                         /* +0x30 ADPCM history */
    int hist2;                         /* +0x34 */
    int fade_vol;                      /* +0x38 0..127 */
    int fade_len;                      /* +0x3C samples (112) */
    int fade_count;                    /* +0x40 */
    int (*cb)(int ch, void *buf, int size, void *arg); /* +0x44 stream callback, nonzero = end */
    void *cb_arg;                      /* +0x48 */
} WaveCh;

typedef struct WaveThreadArg {
    int thid;
    int main_ch;                       /* +0x04 mixed output channel */
    int prio;                          /* +0x08 */
} WaveThreadArg;

int sceAudioChReserve(int ch, int samples, int format);
int sceAudioChRelease(int ch);
int sceAudioSetChannelDataLen(int ch, int samples);
int sceAudioChangeChannelVolume(int ch, int l, int r);
int sceAudioChangeChannelConfig(int ch, int format);
int sceAudioOutputPanned(int ch, int l, int r, void *buf);
int sceAudioOutputPannedBlocking(int ch, int l, int r, void *buf);
int sceAudioOutputBlocking(int ch, int vol, void *buf);
int sceAudioGetChannelRestLength(int ch);
int sceKernelCreateThread(const char *name, int (*entry)(unsigned int, void *), int prio, int stack, unsigned int attr, void *opt);
int sceKernelStartThread(int thid, unsigned int args, void *argp);
int sceKernelDelayThread(unsigned int us);
int sceKernelExitThread(int status);
int sceKernelWaitThreadEnd(int thid, unsigned int *timeout);
int sceKernelDeleteThread(int thid);

int wave_init(int prio);
int wave_thread(unsigned int args, void *argp);
int wave_mix_block(unsigned char *out);
int vag_decode_block(short *out, const unsigned char *in, int *hist1, int *hist2);
void wave_ch_decode_vag(int ch, unsigned char *pcm);
void wave_ch_copy_pcm(int ch, unsigned char *pcm);
void wave_ch_mix(int ch, short *out, short *pcm);
int wave_ch_is_playing(unsigned int ch);
#if defined(__mips__) && !defined(__MWERKS__)
/* separate objects of the library: the SDK's GCC (-O3) must not inline them into their callers; declared before
   the definitions (GCC 3.3 drops a function whose noinline comes after its body) */
int wave_init(int prio) __attribute__((noinline));
int wave_ch_stop(unsigned int ch) __attribute__((noinline));
#endif

#if defined(__MWERKS__) || defined(__mips__) /* PSP (mwcc and the SDK's GCC, libwave_gcc.c): the original objects */
extern volatile int D_08C1E780;                /* 0 none, 1 initialised, 3 thread running */
extern volatile int D_08C1E784;                /* thread stop request */
extern unsigned char D_08C1E788[];     /* decoded block, 0x70 bytes */
extern unsigned char D_08C1E7F8[];     /* one VAG block, 16 bytes */
extern unsigned char D_08C1E808[];     /* two 0x700-byte mix buffers */
extern WaveCh D_08C1F608[8];
extern WaveCh D_08C1F868[8];
extern WaveThreadArg D_08C1FAC8;
extern unsigned char *D_08C1FAD4;
extern unsigned char *D_08C1FAD8;
extern int D_08C1FADC[4];
extern const signed char D_088EAFB8[2][5]; /* VAG predictor coefficients */
#define wave_state D_08C1E780
#define wave_stop_req D_08C1E784
#define wave_pcm_buf D_08C1E788
#define wave_vag_buf D_08C1E7F8
#define wave_mix_buf D_08C1E808
#define wave_ch D_08C1F608
#define wave_shadow D_08C1F868
#define wave_thread_arg D_08C1FAC8
#define wave_vag_ptr D_08C1FAD4
#define wave_pcm_ptr D_08C1FAD8
#define wave_hw_ch D_08C1FADC
#define vag_coef D_088EAFB8
#else
static int wave_state;
static volatile int wave_stop_req;
static unsigned char wave_pcm_buf[0x70];
static unsigned char wave_vag_buf[0x10];
static unsigned char wave_mix_buf[0x1000];
static WaveCh wave_ch[8];
static WaveCh wave_shadow[8];
static WaveThreadArg wave_thread_arg;
static unsigned char *wave_vag_ptr;
static unsigned char *wave_pcm_ptr;
static int wave_hw_ch[4];
static const signed char vag_coef[2][5] = { { 0, 60, 115, 98, 122 }, { 0, 0, -52, -55, -60 } };
#endif

/* Reserves the mixed channel (448 stereo samples) and three hardware channels (256 samples). */
int sceWaveInit_wrap(void)
{
    unsigned int i;
    unsigned char *p;

    if (wave_state != 0) {
        return (int)0x80440001;
    }
    wave_pcm_ptr = wave_pcm_buf;
    wave_vag_ptr = wave_vag_buf;
    p = (unsigned char *)wave_ch;
    for (i = 0; i < sizeof(wave_ch); i++) {
        *p++ = 0;
    }
    p = (unsigned char *)wave_shadow;
    for (i = 0; i < sizeof(wave_shadow); i++) {
        *p++ = 0;
    }
    wave_thread_arg.main_ch = sceAudioChReserve(-1, 0x1C0, 0);
    if (wave_thread_arg.main_ch < 0) {
        return (int)0x80440001;
    }
    wave_hw_ch[0] = sceAudioChReserve(-1, 0x100, 0);
    if (wave_hw_ch[0] < 0) {
        sceAudioChRelease(wave_thread_arg.main_ch);
        return (int)0x80440001;
    }
    wave_hw_ch[1] = sceAudioChReserve(-1, 0x100, 0);
    if (wave_hw_ch[1] < 0) {
        sceAudioChRelease(wave_thread_arg.main_ch);
        sceAudioChRelease(wave_hw_ch[0]);
        return (int)0x80440001;
    }
    wave_hw_ch[2] = sceAudioChReserve(-1, 0x100, 0);
    if (wave_hw_ch[2] < 0) {
        sceAudioChRelease(wave_thread_arg.main_ch);
        sceAudioChRelease(wave_hw_ch[0]);
        sceAudioChRelease(wave_hw_ch[1]);
        return (int)0x80440001;
    }
    wave_state = 1;
    return 0;
}

/* Sets a channel's sound: VAG or PCM data and size (flags as WaveCh.flags), or a callback stream
   (flags 0x400, data and size 0). Volumes reset to full; the channel must be stopped. */
int wave_ch_set_data(unsigned int ch, const unsigned char *data, unsigned int size, unsigned int flags)
{
    WaveCh *c;

    if (ch >= 8) {
        return (int)0x80440005;
    }
    if (size > 0x1800000) {
        return (int)0x80440007;
    }
    if (wave_ch_is_playing(ch) != 0) {
        return (int)0x80440009;
    }
    if (!(flags & 0x400)) {
        if (data == 0) {
            return (int)0x80440006;
        }
        if (size == 0) {
            return (int)0x80440007;
        }
        c = &wave_ch[ch];
        c->data = data;
        c->size = size;
        c->pos = 0;
        c->playing = 0;
        c->vol_l = 4096;
        c->flags = flags;
        c->vol_r = 4096;
        c->fade_len = 112;
        c->hist1 = 0;
        c->hist2 = 0;
        c->unk28 = 0;
        c->unk2c = 0;
        c->unk24 = 0;
        c->loop_start = 0;
        c->loop_end = 0;
        c->fade_count = 0;
    } else {
        if (data != 0) {
            return (int)0x80440006;
        }
        if (size != 0) {
            return (int)0x80440007;
        }
        c = &wave_ch[ch];
        c->data = 0;
        c->size = 0;
        c->pos = 0;
        c->playing = 0;
        c->vol_l = 4096;
        c->flags = flags;
        c->vol_r = 4096;
        c->fade_len = 112;
        c->hist1 = 0;
        c->hist2 = 0;
        c->unk28 = 0;
        c->unk2c = 0;
        c->unk24 = 0;
        c->loop_start = 0;
        c->loop_end = 0;
        c->fade_count = 0;
    }
    return 0;
}

/* volumes 0..127 */
int wave_ch_set_volume(unsigned int ch, unsigned int l, unsigned int r)
{
    if (ch >= 8) {
        return (int)0x80440005;
    }
    if (l >= 128) {
        return (int)0x8044000A;
    }
    if (r >= 128) {
        return (int)0x8044000A;
    }
    wave_ch[ch].vol_l = (int)(l << 12) / 127;
    wave_ch[ch].vol_r = (int)(r << 12) / 127;
    return 0;
}

/* volumes back in 0..127 */
int wave_ch_get_volume(unsigned int ch, int *l, int *r)
{
    int v;

    if (ch >= 8) {
        return (int)0x80440005;
    }
    if (l == 0) {
        return (int)0x8044000A;
    }
    if (r == 0) {
        return (int)0x8044000A;
    }
    v = wave_ch[ch].vol_l;
    if (v <= 0) {
        *l = 0;
    } else if (v >= 128) {
        *l = (v - 1) / 32;
    } else {
        *l = v / 32;
    }
    v = wave_ch[ch].vol_r;
    if (v <= 0) {
        *r = 0;
    } else if (v >= 128) {
        *r = (v - 1) / 32;
    } else {
        *r = v / 32;
    }
    return 0;
}

/* volumes 0..4096 */
int wave_ch_set_volume_raw(unsigned int ch, unsigned int l, unsigned int r)
{
    if (ch >= 8) {
        return (int)0x80440005;
    }
    if (l > 4096) {
        return (int)0x8044000A;
    }
    if (r > 4096) {
        return (int)0x8044000A;
    }
    wave_ch[ch].vol_l = l;
    wave_ch[ch].vol_r = r;
    return 0;
}

int wave_ch_get_volume_raw(unsigned int ch, int *l, int *r)
{
    if (ch >= 8) {
        return (int)0x80440005;
    }
    if (l == 0) {
        return (int)0x8044000A;
    }
    if (r == 0) {
        return (int)0x8044000A;
    }
    *l = wave_ch[ch].vol_l;
    *r = wave_ch[ch].vol_r;
    return 0;
}

/* bytes left to play (0 when stopped or streamed) */
int wave_ch_get_rest(unsigned int ch, int *rest)
{
    WaveCh *c;

    if (ch >= 8) {
        return (int)0x80440005;
    }
    if (rest == 0) {
        return (int)0x8044000C;
    }
    c = &wave_ch[ch];
    if (c->playing != 0 && !(c->flags & 0x400)) {
        *rest = c->size - c->pos;
    } else {
        *rest = 0;
    }
    return 0;
}

int wave_ch_is_playing(unsigned int ch)
{
    if (ch >= 8) {
        return (int)0x80440005;
    }
    return wave_ch[ch].playing;
}

/* (re)start a channel from the beginning; a playing one fades out into the restart. volumes 0..127 */
int wave_ch_play(unsigned int ch, unsigned int l, unsigned int r)
{
    if (ch >= 8) {
        return (int)0x80440005;
    }
    if (l >= 128) {
        return (int)0x8044000A;
    }
    if (r >= 128) {
        return (int)0x8044000A;
    }
    if (wave_ch[ch].data == 0 && !(wave_ch[ch].flags & 0x400)) {
        return (int)0x80440006;
    }
    if (wave_ch[ch].playing) {
        wave_shadow[ch].vol_l = (int)(l << 12) / 127;
        wave_shadow[ch].vol_r = (int)(r << 12) / 127;
        wave_shadow[ch].unk2c = 0;
        wave_ch[ch].flags |= 0x110000;
        wave_shadow[ch].pos = 0;
        wave_shadow[ch].hist1 = 0;
        wave_shadow[ch].hist2 = 0;
        wave_shadow[ch].unk28 = 0;
        wave_shadow[ch].playing = 1;
    } else {
        wave_ch[ch].playing = 1;
        wave_ch[ch].vol_l = (int)(l << 12) / 127;
        wave_ch[ch].vol_r = (int)(r << 12) / 127;
        wave_ch[ch].pos = 0;
        wave_ch[ch].hist1 = 0;
        wave_ch[ch].hist2 = 0;
        wave_ch[ch].unk28 = 0;
        wave_ch[ch].unk2c = 0;
    }
    return 0;
}

/* volumes 0..4096 */
int wave_ch_play_raw(unsigned int ch, unsigned int l, unsigned int r)
{
    if (ch >= 8) {
        return (int)0x80440005;
    }
    if (l > 4096) {
        return (int)0x8044000A;
    }
    if (r > 4096) {
        return (int)0x8044000A;
    }
    if (wave_ch[ch].data == 0 && !(wave_ch[ch].flags & 0x400)) {
        return (int)0x80440006;
    }
    if (wave_ch[ch].playing) {
        wave_shadow[ch].vol_l = l;
        wave_shadow[ch].vol_r = r;
        wave_shadow[ch].unk2c = 0;
        wave_ch[ch].flags |= 0x110000;
        wave_shadow[ch].pos = 0;
        wave_shadow[ch].hist1 = 0;
        wave_shadow[ch].hist2 = 0;
        wave_shadow[ch].unk28 = 0;
        wave_shadow[ch].playing = 1;
    } else {
        wave_ch[ch].playing = 1;
        wave_ch[ch].vol_l = l;
        wave_ch[ch].vol_r = r;
        wave_ch[ch].pos = 0;
        wave_ch[ch].hist1 = 0;
        wave_ch[ch].hist2 = 0;
        wave_ch[ch].unk28 = 0;
        wave_ch[ch].unk2c = 0;
    }
    return 0;
}

/* stops a channel; a playing one fades out first */
int wave_ch_stop(unsigned int ch)
{
    if (ch >= 8) {
        return (int)0x80440005;
    }
    if (wave_ch[ch].playing) {
        wave_shadow[ch].playing = 0;
        wave_shadow[ch].pos = 0;
        wave_ch[ch].flags |= 0x120000;
        wave_shadow[ch].hist1 = 0;
        wave_shadow[ch].hist2 = 0;
        wave_shadow[ch].unk28 = 0;
        wave_shadow[ch].unk2c = 0;
    } else {
        wave_ch[ch].pos = 0;
        wave_ch[ch].hist1 = 0;
        wave_ch[ch].hist2 = 0;
        wave_ch[ch].unk28 = 0;
        wave_ch[ch].unk2c = 0;
    }
    return 0;
}

/* hardware channel hw (0-3): samples per output, 64..65472 in steps of 64 */
int wave_set_sample(unsigned int hw, unsigned int samples)
{
    int ret;

    if (hw >= 4) {
        return (int)0x80440010;
    }
    if (samples - 0x40 > 0xFF80) {
        return (int)0x80440011;
    }
    if ((samples & 0x3F) != 0) {
        return (int)0x80440011;
    }
    return sceAudioSetChannelDataLen(wave_hw_ch[hw], samples);
}

int wave_set_volume(unsigned int hw, unsigned int l, unsigned int r)
{
    if (hw >= 4) {
        return (int)0x80440010;
    }
    if (l > 0x8000) {
        return (int)0x8044000A;
    }
    if (r > 0x8000) {
        return (int)0x8044000A;
    }
    return sceAudioChangeChannelVolume(wave_hw_ch[hw], l, r);
}

int wave_audio_output(unsigned int hw, unsigned int l, unsigned int r, void *buf)
{
    if (hw >= 4) {
        return (int)0x80440010;
    }
    if (l > 0x8000) {
        return (int)0x8044000A;
    }
    if (r > 0x8000) {
        return (int)0x8044000A;
    }
    return sceAudioOutputPanned(wave_hw_ch[hw], l, r, buf);
}

int wave_audio_write(unsigned int hw, unsigned int l, unsigned int r, void *buf)
{
    if (hw >= 4) {
        return (int)0x80440010;
    }
    if (l > 0x8000) {
        return (int)0x8044000A;
    }
    if (r > 0x8000) {
        return (int)0x8044000A;
    }
    return sceAudioOutputPannedBlocking(wave_hw_ch[hw], l, r, buf);
}

/* starts the mixing thread at priority 8..119 */
int wave_start(int prio)
{
    if (wave_state != 1) {
        return (int)0x80440003;
    }
    if ((unsigned int)(prio - 8) >= 0x70) {
        return (int)0x80440012;
    }
    return wave_init(prio);
}

/* stops every channel and the mixing thread */
int wave_stop(void)
{
    int ch;

    if (wave_state != 3) {
        return (int)0x80440004;
    }
    for (ch = 0; ch < 8; ch++) {
        wave_ch_stop(ch);
    }
    wave_stop_req = 1;
    sceKernelWaitThreadEnd(wave_thread_arg.thid, 0);
    sceKernelDeleteThread(wave_thread_arg.thid);
    wave_stop_req = 0;
    wave_state = 1;
    return 0;
}

/* releases the four channels */
int wave_exit(void)
{
    if (wave_state != 1) {
        return (int)0x80440002;
    }
    sceAudioChRelease(wave_thread_arg.main_ch);
    sceAudioChRelease(wave_hw_ch[0]);
    sceAudioChRelease(wave_hw_ch[1]);
    sceAudioChRelease(wave_hw_ch[2]);
    wave_vag_ptr = 0;
    wave_pcm_ptr = 0;
    wave_state = 0;
    return 0;
}

/* VAG loop mode 1 (loop at the 6/3 block flags) or 2 (honour the flags but end at the data end) */
int wave_ch_set_loop(unsigned int ch, unsigned int mode)
{
    if (ch >= 8) {
        return (int)0x80440005;
    }
    if (mode - 1 >= 2) {
        return (int)0x80440008;
    }
    wave_ch[ch].flags = (wave_ch[ch].flags & ~0xFFu) | mode;
    return 0;
}

/* fades a playing channel out and stops it */
int wave_ch_fade_stop(unsigned int ch)
{
    if (ch >= 8) {
        return (int)0x80440005;
    }
    if (wave_ch[ch].playing) {
        wave_shadow[ch].playing = 0;
        wave_ch[ch].flags |= 0x140000;
    }
    return 0;
}

/* marks a stopped channel as playing again, from where it was */
int wave_ch_resume(unsigned int ch)
{
    if (ch >= 8) {
        return (int)0x80440005;
    }
    if (wave_ch[ch].playing == 0) {
        wave_ch[ch].playing = 1;
    }
    return 0;
}
#endif

#ifndef __MWERKS__ /* PSP: libwave_gcc.c */
void wave_ch_stop_all(void)
{
    int ch;

    for (ch = 0; ch < 8; ch++) {
        wave_ch_stop(ch);
    }
}
#else
void wave_ch_stop_all(void);
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: the library state through split lui/lo */
int wave_set_config(unsigned int hw, int format)
{
    if (hw >= 4) {
        return (int)0x80440010;
    }
    return sceAudioChangeChannelConfig(wave_hw_ch[hw], format);
}

int wave_rest_samples(unsigned int hw)
{
    if (hw >= 4) {
        return (int)0x80440010;
    }
    return sceAudioGetChannelRestLength(wave_hw_ch[hw]);
}

/* creates and starts "SceWaveMain" */
int wave_init(int prio)
{
    int r;

    wave_stop_req = 0;
    wave_thread_arg.thid = sceKernelCreateThread("SceWaveMain", wave_thread, prio, 0x1000, 0, 0);
    if (wave_thread_arg.thid < 0) {
        return (int)0x80440003;
    }
    wave_thread_arg.prio = prio;
    r = sceKernelStartThread(wave_thread_arg.thid, sizeof(wave_thread_arg), &wave_thread_arg);
    if (r != 0) {
        return (int)0x80440003;
    }
    sceKernelDelayThread(10000);
    wave_state = 3;
    return 0;
}

/* mixes 16 blocks (448 samples) per buffer, double buffered, while any channel plays */
int wave_thread(unsigned int args, void *argp)
{
    WaveThreadArg *a = argp;
    int idx = 0;

    while (wave_stop_req == 0) {
        int off = idx * 0x700;

        if (wave_mix_block(wave_mix_buf + off) == 0) {
            sceKernelDelayThread(10000);
        } else {
            int i;

            for (i = 1; i < 16; i++) {
                wave_mix_block(wave_mix_buf + off + i * 0x70);
            }
            sceAudioOutputBlocking(a->main_ch, 0x8000, wave_mix_buf + off);
            idx = (idx + 1) % 2;
        }
    }
    sceAudioOutputBlocking(a->main_ch, 0, 0);
    sceKernelExitThread(0);
    return 0;
}

/* one 28-sample stereo block of every playing channel into out; returns the playing count */
int wave_mix_block(unsigned char *out)
{
    unsigned int i;
    int active = 0;
    int ch;

    for (i = 0; i < 0x70; i++) {
        out[i] = 0;
    }
    for (ch = 0; ch < 8; ch++) {
        WaveCh *c = &wave_ch[ch];

        if (c->playing == 0) {
            continue;
        }
        active++;
        if ((c->flags & 0x400) == 0) {
            if ((c->flags & 0x100) == 0) {
                if (c->data != 0) {
                    wave_ch_decode_vag(ch, wave_pcm_ptr);
                }
            } else if (c->data != 0) {
                wave_ch_copy_pcm(ch, wave_pcm_ptr);
            }
        } else if (c->cb != 0) {
            for (i = 0; i < 0x70; i++) {
                wave_pcm_ptr[i] = 0;
            }
            if ((c->flags & 0x100) == 0) {
                if (c->cb(ch, wave_vag_ptr, 0x10, c->cb_arg) != 0) {
                    c->playing = 0;
                }
                vag_decode_block((short *)wave_pcm_ptr, wave_vag_ptr, &c->hist1, &c->hist2);
            } else if (c->cb(ch, wave_pcm_ptr, 0x70, c->cb_arg) != 0) {
                c->playing = 0;
            }
        }
        wave_ch_mix(ch, (short *)out, (short *)wave_pcm_ptr);
    }
    return active;
}

/* One VAG ADPCM block (16 bytes: shift/predictor, flags, 14 data bytes) into 28 samples; returns
   the block flags (7 = end marker, nothing decoded). */
int vag_decode_block(short *out, const unsigned char *in, int *hist1, int *hist2)
{
    int shift = in[0] & 0xF;
    int pred = in[0] >> 4;
    int flags = in[1];
    int c0 = vag_coef[0][pred];
    int c1 = vag_coef[1][pred];
    int s1 = *hist1;
    int s2 = *hist2;
    int i;

    if (flags == 7) {
        return 7;
    }
    for (i = 2; i < 16; i++) {
        int b = (signed char)in[i];

        s2 = (((b << 28) >> (shift + 10)) + s1 * c0 + s2 * c1) >> 6;
        if (s2 > 0x7FFF) {
            s2 = 0x7FFF;
        }
        if (s2 < -0x8000) {
            s2 = -0x8000;
        }
        *out++ = (short)s2;
        s1 = (((b & ~0xF) << (14 - shift)) + s2 * c0 + s1 * c1) >> 6;
        if (s1 > 0x7FFF) {
            s1 = 0x7FFF;
        }
        if (s1 < -0x8000) {
            s1 = -0x8000;
        }
        *out++ = (short)s1;
    }
    *hist1 = s1;
    *hist2 = s2;
    return flags;
}

static void copy_bytes(unsigned char *dst, const unsigned char *src, unsigned int n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        *dst++ = *src++;
    }
}

/* next VAG block of a channel into pcm (28 mono samples), following the loop mode */
void wave_ch_decode_vag(int ch, unsigned char *pcm)
{
    const unsigned char *data = wave_ch[ch].data;
    unsigned int i;
    int r;

    {
        unsigned char *p = pcm;

        for (i = 0; i < 0x70; i++) {
            *p++ = 0;
        }
    }
    switch (wave_ch[ch].flags & 0xFF) {
    case 0:
        if (wave_ch[ch].pos + 16 <= wave_ch[ch].size) {
            copy_bytes(wave_vag_ptr, data + wave_ch[ch].pos, 16);
            wave_ch[ch].pos += 16;
            vag_decode_block((short *)pcm, wave_vag_ptr, &wave_ch[ch].hist1, &wave_ch[ch].hist2);
            return;
        }
        wave_ch[ch].playing = 0;
        wave_ch[ch].pos = 0;
        return;
    case 1:
        if (wave_ch[ch].size < 16) {
            return;
        }
        if (wave_ch[ch].pos + 16 <= wave_ch[ch].size) {
            copy_bytes(wave_vag_ptr, data + wave_ch[ch].pos, 16);
        } else {
            wave_ch[ch].pos = wave_ch[ch].loop_start;
            copy_bytes(wave_vag_ptr, data + wave_ch[ch].pos, 16);
        }
        break;
    case 2:
        if (wave_ch[ch].size < 16) {
            return;
        }
        if (wave_ch[ch].size < wave_ch[ch].pos + 16) {
            wave_ch[ch].playing = 0;
            wave_ch[ch].pos = 0;
            return;
        }
        copy_bytes(wave_vag_ptr, data + wave_ch[ch].pos, 16);
        break;
    default:
        return;
    }
    r = vag_decode_block((short *)pcm, wave_vag_ptr, &wave_ch[ch].hist1, &wave_ch[ch].hist2);
    if (r == 3) {
        wave_ch[ch].loop_end = wave_ch[ch].pos;
        wave_ch[ch].pos = wave_ch[ch].loop_start;
    } else if (r == 6) {
        wave_ch[ch].loop_start = wave_ch[ch].pos;
        wave_ch[ch].pos += 16;
    } else {
        wave_ch[ch].pos += 16;
    }
}

/* wave_ch_copy_pcm, once per block size (0x38 mono, 0x70 stereo): the SDK's GCC expands the two helpers in place for
   each constant, so the original has four copies of the code */
#if defined(__mips__) && !defined(__MWERKS__)
#define WAVE_ALWAYS_INLINE __attribute__((always_inline))
#else
#define WAVE_ALWAYS_INLINE
#endif
static inline void wave_pcm_once(int ch, unsigned char *pcm, const unsigned char *data, int blk) WAVE_ALWAYS_INLINE;
static inline void wave_pcm_once(int ch, unsigned char *pcm, const unsigned char *data, int blk)
{
    unsigned int i;

    if (wave_ch[ch].size >= wave_ch[ch].pos + blk) {
        copy_bytes(pcm, data + wave_ch[ch].pos, blk);
        wave_ch[ch].pos += blk;
        return;
    }
    {
        unsigned char *p = pcm;

        for (i = 0; i < 0x70; i++) {
            *p++ = 0;
        }
    }
    copy_bytes(pcm, data + wave_ch[ch].pos, wave_ch[ch].size - wave_ch[ch].pos);
    wave_ch[ch].pos = 0;
    wave_ch[ch].playing = 0;
}

static inline void wave_pcm_loop(int ch, unsigned char *pcm, const unsigned char *data, int blk) WAVE_ALWAYS_INLINE;
static inline void wave_pcm_loop(int ch, unsigned char *pcm, const unsigned char *data, int blk)
{
    if (wave_ch[ch].size >= blk) {
        unsigned int rem = wave_ch[ch].size - wave_ch[ch].pos;

        if (wave_ch[ch].pos + blk < wave_ch[ch].size) {
            copy_bytes(pcm, data + wave_ch[ch].pos, blk);
            wave_ch[ch].pos += blk;
            return;
        }
        copy_bytes(pcm, data + wave_ch[ch].pos, rem);
        wave_ch[ch].pos = 0;
        copy_bytes(pcm + rem, data, blk - rem);
        wave_ch[ch].pos = blk - rem;
    } else {
        unsigned int done = wave_ch[ch].size - wave_ch[ch].pos;
        int need = blk - done;

        copy_bytes(pcm, data + wave_ch[ch].pos, done);
        wave_ch[ch].pos = 0;
        while (need > wave_ch[ch].size) {
            copy_bytes(pcm + done, data, wave_ch[ch].size);
            need -= wave_ch[ch].size;
            done += wave_ch[ch].size;
        }
        copy_bytes(pcm + done, data, need);
        wave_ch[ch].pos += need;
    }
}

/* next PCM block of a channel into pcm: 0x38 bytes (28 mono samples) or 0x70 (28 stereo) */
void wave_ch_copy_pcm(int ch, unsigned char *pcm)
{
    const unsigned char *data = wave_ch[ch].data;
    unsigned int flags = wave_ch[ch].flags;

    if ((flags & 1) == 0) {
        if (!(flags & 0x200)) {
            wave_pcm_once(ch, pcm, data, 0x38);
        } else {
            wave_pcm_once(ch, pcm, data, 0x70);
        }
    } else {
        if (!(flags & 0x200)) {
            wave_pcm_loop(ch, pcm, data, 0x38);
        } else {
            wave_pcm_loop(ch, pcm, data, 0x70);
        }
    }
}

/* adds 28 samples of a channel to the stereo mix, with its volumes and fade; at the end of a fade
   the pending shadow state (restart, stop or fade stop) is applied */
void wave_ch_mix(int ch, short *out, short *pcm)
{
    WaveCh *c = &wave_ch[ch];
    WaveCh *s = &wave_shadow[ch];
    short *mono = pcm;
    int i;

    for (i = 0; i < 28; i++) {
        unsigned int flags = c->flags;
        int l;
        int r;
        int vl;
        int vr;

        if ((flags & 0x100000) == 0) {
            c->fade_vol = 127;
        } else {
            int len = c->fade_len;

            c->fade_vol = ((len - c->fade_count) * 127) / len;
            if (c->fade_vol < 0) {
                c->fade_vol = 0;
            }
            c->fade_count++;
            if (c->fade_count >= len) {
                c->flags = flags & ~0x100000;
                c->fade_count = 0;
                if (flags & 0x10000) {
                    c->flags = flags & ~0x110000;
                    c->playing = s->playing;
                    c->pos = s->pos;
                    c->vol_l = s->vol_l;
                    c->vol_r = s->vol_r;
                    c->hist1 = s->hist1;
                    c->hist2 = s->hist2;
                    c->unk28 = s->unk28;
                    c->unk2c = s->unk2c;
                } else if (flags & 0x20000) {
                    c->flags = flags & ~0x120000;
                    c->playing = s->playing;
                    c->pos = s->pos;
                    c->hist1 = s->hist1;
                    c->hist2 = s->hist2;
                    c->unk28 = s->unk28;
                    c->unk2c = s->unk2c;
                } else if (flags & 0x40000) {
                    c->flags = flags & ~0x140000;
                    c->playing = s->playing;
                }
            }
        }
        vl = (c->vol_l * c->fade_vol) / 127;
        vr = (c->vol_r * c->fade_vol) / 127;
        if ((c->flags & 0x100) && (c->flags & 0x200)) {
            l = vl * pcm[i * 2];
            r = vr * pcm[i * 2 + 1];
        } else {
            r = vr * *mono;
            l = vl * *mono;
        }
        mono++;
        l = out[i * 2] + l / 4096;
        r = out[i * 2 + 1] + r / 4096;
        if (l > 0x7FFE) {
            l = 0x7FFF;
        }
        if (r > 0x7FFE) {
            r = 0x7FFF;
        }
        if (l < -0x8000) {
            l = -0x8000;
        }
        if (r < -0x8000) {
            r = -0x8000;
        }
        out[i * 2] = (short)l;
        out[i * 2 + 1] = (short)r;
    }
}
#endif
