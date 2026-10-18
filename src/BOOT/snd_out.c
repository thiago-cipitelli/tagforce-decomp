// @range 0x0885D948-0x0885DE90
// @asm 0x0885D948-0x0885DC48
/* Sound library, output: the global effect (stereo widening, echo into its own buffer, box filter)
   applied to the mixed block, and the sound thread that mixes SAS voices and streams into one of two
   0x100-frame buffers and hands it to the audio channel. */
#include "psp.h"
#include "libc.h"
#include "snd.h"

typedef struct SndFx {                 /* 0x08C1AFC8 */
    unsigned char type;                /* 1 stereo widening, 2 echo, 3 box filter */
    unsigned char mode;                /* 0x10 SAS core, 0x20 effect before the mix, 0x30 after */
    unsigned char pad2[2];
    SndFader8 amount;                  /* +0x04 box filter taps */
    int vpl;                           /* +0x0C echo buffer allocation, <0 none */
    short *buf;                        /* +0x10 echo buffer, 0x3000 stereo frames */
    unsigned char lvl;                 /* +0x14 */
    unsigned char delay;               /* +0x15 */
    unsigned short pos;                /* +0x16 in frames */
} SndFx;

extern short snd_widen_hist[4];        /* 0x08BF64A0 */
extern short snd_box_hist[32];         /* 0x08BF64A8 */
extern short snd_mix_buf[2][0x200];    /* 0x08BF6680 */
extern int snd_mix_idx;                /* 0x08BF6E80 */
extern int snd_echo_pos;               /* 0x08BEA49C */
extern short snd_echo_buf[0x6000];     /* 0x08BEA4A0 */
extern SndFx snd_fx;                   /* 0x08C1AFC8 */
extern SndFader snd_master;            /* 0x08C1B01C */
extern int snd_obj_sema;               /* 0x08C1B050 */
extern int snd_idle_evf;               /* 0x08E1FD84 */

void snd_box_filter(short *hist, short *buf, int n, int taps);
void snd_stereo_widen(short *hist, short *buf, int n);
int snd_fader_update(SndFader *f);
int snd_fader8_update(SndFader8 *f);
void snd_faders_update(void);
void snd_voices_update(void);
void snd_streams_decode(int sas);
int snd_sas_core(short *buf);
int snd_sas_core_mix(short *buf, int l, int r);
int wave_audio_write(int ch, int l, int r, void *buf);
int sceKernelDeleteVpl(int vpl);

#ifdef NON_MATCHING /* register allocation of the echo loop */
void snd_fx_apply(short *buf)
{
    int v;
    short *in;
    short *p;
    int pos;
    int lvl;
    int n;
    int i;
    SndFx *fx = &snd_fx;
    unsigned int any;

    switch (fx->type) {
    case 1:
            snd_stereo_widen(snd_widen_hist, buf, 0x100);
        break;
    case 2:
        lvl = fx->lvl * 0x40A0 >> 14;
        pos = ((fx->delay * 0x40A0 >> 14) * 0x2F00 >> 7) + fx->pos;
        in = buf;
        if (pos >= 0x3000) {
            pos -= 0x3000;
        }
        n = 0x200;
        p = fx->buf + pos * 2;
        if (pos + 0x100 > 0x3000) {
            n = (0x3000 - pos) * 2;
            while (--n >= 0) {
                int s = *in * lvl >> 7;

                if (s < 0) {
                    s++;
                }
                s += *p;
                if (s < -0x8000) {
                    s = -0x8000;
                } else if (s > 0x7FFF) {
                    s = 0x7FFF;
                }
                *p = s;
                in++;
                p++;
            }
            n = (pos - 0x2F00) * 2;
            p = fx->buf;
        }
        while (--n >= 0) {
            int s = *in * lvl >> 7;

            if (s < 0) {
                s++;
            }
            s += *p;
            if (s < -0x8000) {
                s = -0x8000;
            } else if (s > 0x7FFF) {
                s = 0x7FFF;
            }
            *p = s;
            p++;
            in++;
        }
        break;
    case 3:
            snd_box_filter(snd_box_hist, buf, 0x100, fx->amount.cur);
        break;
    }
    if (fx->amount.cur != fx->amount.target) {
        snd_fader8_update(&fx->amount);
    }
    if (fx->vpl >= 0) {
        any = 0;
        p = fx->buf + fx->pos * 2;
        for (i = 0; i < 0x200; i++) {
            v = *p >> 1;
            if (v < 0) {
                v++;
            }
            *p = v;
            any |= v;
            v += *buf;
            if (v < -0x8000) {
                v = -0x8000;
            } else if (v > 0x7FFF) {
                v = 0x7FFF;
            }
            *buf = v;
            p++;
            buf++;
        }
        n = fx->pos + 0x100;
        if (n >= 0x3000) {
            n = 0;
        }
        fx->pos = n;
        if (fx->type != 2 && any == 0 && sceKernelDeleteVpl(fx->vpl) == 0) {
            fx->vpl = -1;
        }
    }
}
#endif

void snd_thread_main(void)
{
    unsigned char mode;
    int vol;

    snd_mix_idx = 0;
    snd_echo_pos = 0;
    memset(snd_echo_buf, 0, 0xC000);
    for (;;) {
        SndFader *m = &snd_master;

        if (snd_master.cur != snd_master.target) {
            snd_fader_update(m);
        }
        mode = snd_fx.mode;
        if (mode != 0x10) {
            memset(snd_mix_buf[snd_mix_idx], 0, 0x400);
        }
        sceKernelWaitSema(snd_obj_sema, 1, 0);
        snd_faders_update();
        snd_voices_update();
        if (mode == 0x10) {
            snd_sas_core(snd_mix_buf[snd_mix_idx]);
            snd_fx_apply(snd_mix_buf[snd_mix_idx]);
        }
        snd_streams_decode(mode == 0x10);
        sceKernelSignalSema(snd_obj_sema, 1);
        if (mode != 0x10) {
            if (mode == 0x20) {
                sceKernelWaitSema(snd_obj_sema, 1, 0);
                snd_fx_apply(snd_mix_buf[snd_mix_idx]);
                sceKernelSignalSema(snd_obj_sema, 1);
            }
            snd_sas_core_mix(snd_mix_buf[snd_mix_idx], 0x1000, 0x1000);
            if (mode == 0x30) {
                sceKernelWaitSema(snd_obj_sema, 1, 0);
                snd_fx_apply(snd_mix_buf[snd_mix_idx]);
                sceKernelSignalSema(snd_obj_sema, 1);
            }
        }
        vol = m->cur << 1;
        sceKernelSetEventFlag(snd_idle_evf, 1);
        wave_audio_write(2, vol, vol, snd_mix_buf[snd_mix_idx]);
        if (++snd_mix_idx >= 2) {
            snd_mix_idx = 0;
        }
    }
}
