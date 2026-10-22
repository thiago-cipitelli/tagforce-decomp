// @range 0x08862D28-0x088634EC
// @asm 0x08862D28-0x088634A8
// @asm 0x088634C4-0x088634EC
/* Sony's libsas (user side of the SAS hardware mixer): sceSas* wrappers that check the library was
   initialised and pass the shared SasCore buffer (D_08C1D940, 64-byte aligned) to the __sceSas*
   system calls. Compiled with GCC; every wrapper reads the init flag (D_08C1D900) and the core
   through split lui/lo pairs, so they stay in asm with their C under NON_MATCHING; only
   sceSasInit's default-grain variant is a whole `asm` function. Error codes are the SDK ones
   (0x80420100 not initialised, 0x80420101 already initialised, 0x80420005 bad buffer, 0x80420010 bad
   voice, 0x80420012 bad pitch). As in the original, sceSasInit configures the 32 voices through the
   checked wrappers before setting the init flag, so that loop does nothing. Outside the PSP build
   the C is used with its own state and the __sceSas* calls come from the platform layer. */

int snd_sas_open(int grain);

#if !defined(__MWERKS__) || defined(NON_MATCHING)
int __sceSasInit(void *core, int grain, int max_voices, int out_mode, int freq);
int __sceSasCore(void *core, void *out);
int __sceSasCoreWithMix(void *core, void *inout, int l, int r);
int __sceSasSetKeyOn(void *core, int voice);
int __sceSasSetKeyOff(void *core, int voice);
int __sceSasSetPause(void *core, unsigned int mask, int pause);
int __sceSasGetPauseFlag(void *core);
int __sceSasSetVolume(void *core, int voice, int l, int r, int el, int er);
int __sceSasSetPitch(void *core, int voice, int pitch);
int __sceSasSetVoice(void *core, int voice, const void *vag, int size, int loop);
int __sceSasSetNoise(void *core, int voice, int freq);
int __sceSasSetADSR(void *core, int voice, int flag, int a, int d, int s, int r);
int __sceSasSetADSRmode(void *core, int voice, int flag, int a, int d, int s, int r);
int __sceSasSetSL(void *core, int voice, int level);
int __sceSasSetSimpleADSR(void *core, int voice, int adsr1, int adsr2);
int __sceSasGetEndFlag(void *core);
int __sceSasGetEnvelopeHeight(void *core, int voice);
int __sceSasRevVON(void *core, int dry, int wet);
int __sceSasRevEVOL(void *core, int l, int r);
int __sceSasRevType(void *core, int type);
int __sceSasRevParam(void *core, int delay, int feedback);
int snd_voice_set_pitch(int voice, int pitch);
int snd_voice_set_simple_adsr(int voice, int adsr1, int adsr2);
int snd_voice_set_volume(int voice, int l, int r, int el, int er);

#ifdef __MWERKS__
extern int D_08C1D900;                 /* initialised flag */
extern char D_08C1D940[];              /* SasCore work area */
#define sas_ready D_08C1D900
#define sas_core ((void *)D_08C1D940)
#else
static int sas_ready;
static unsigned int sas_core_buf[0x390] __attribute__((aligned(64)));
#define sas_core ((void *)sas_core_buf)
#endif

#define SAS_NOT_INIT ((int)0x80420100)

/* sceSasCore: mixes one grain into out (64-byte aligned) */
int snd_sas_core(void *out)
{
    if (!sas_ready) {
        return SAS_NOT_INIT;
    }
    if (out == 0 || ((unsigned int)out & 0x3F) != 0) {
        return (int)0x80420005;
    }
    return __sceSasCore(sas_core, out);
}

/* sceSasCoreWithMix: mixes one grain onto the samples already in inout */
int snd_sas_core_mix(void *inout, int l, int r)
{
    if (!sas_ready) {
        return SAS_NOT_INIT;
    }
    if (inout == 0 || ((unsigned int)inout & 0x3F) != 0) {
        return (int)0x80420005;
    }
    return __sceSasCoreWithMix(sas_core, inout, l, r);
}

/* sceSasInit: 32 voices at 44.1 kHz */
int snd_sas_open(int grain)
{
    int r;
    int v;

    if (sas_ready) {
        return (int)0x80420101;
    }
    r = __sceSasInit(sas_core, grain, 32, 0, 44100);
    if (r != 0) {
        return r;
    }
    for (v = 0; v < 32; v++) {
        snd_voice_set_pitch(v, 0x1000);
        snd_voice_set_simple_adsr(v, 0xF, 0x5FC0);
        snd_voice_set_volume(v, 0, 0, 0, 0);
    }
    sas_ready = 1;
    return 0;
}

/* sceSasSetKeyOn */
int snd_voice_key_on(int voice)
{
    return sas_ready ? __sceSasSetKeyOn(sas_core, voice) : SAS_NOT_INIT;
}

/* sceSasSetKeyOff */
int snd_voice_key_off(int voice)
{
    return sas_ready ? __sceSasSetKeyOff(sas_core, voice) : SAS_NOT_INIT;
}

/* pause or resume one voice */
int snd_voice_set_pause(unsigned int voice, int pause)
{
    if (!sas_ready) {
        return SAS_NOT_INIT;
    }
    if (voice >= 32) {
        return (int)0x80420010;
    }
    return __sceSasSetPause(sas_core, 1u << voice, pause);
}

/* sceSasSetPause: pause or resume the voices in mask */
int snd_sas_set_pause(unsigned int mask, int pause)
{
    return sas_ready ? __sceSasSetPause(sas_core, mask, pause) : SAS_NOT_INIT;
}

/* sceSasGetPauseFlag: mask of the paused voices */
int snd_sas_get_pause_flag(void)
{
    return sas_ready ? __sceSasGetPauseFlag(sas_core) : 0;
}

/* sceSasSetVolume */
int snd_voice_set_volume(int voice, int l, int r, int el, int er)
{
    return sas_ready ? __sceSasSetVolume(sas_core, voice, l, r, el, er) : SAS_NOT_INIT;
}

/* sceSasSetPitch: pitch 0 is rejected */
int snd_voice_set_pitch(int voice, int pitch)
{
    if (!sas_ready) {
        return SAS_NOT_INIT;
    }
    if (pitch == 0) {
        return (int)0x80420012;
    }
    return __sceSasSetPitch(sas_core, voice, pitch);
}

/* sceSasSetVoice: VAG data, size, loop flag */
int snd_voice_set_vag(int voice, const void *vag, int size, int loop)
{
    return sas_ready ? __sceSasSetVoice(sas_core, voice, vag, size, loop) : SAS_NOT_INIT;
}

/* sceSasSetNoise */
int snd_voice_set_noise(int voice, int freq)
{
    return sas_ready ? __sceSasSetNoise(sas_core, voice, freq) : SAS_NOT_INIT;
}

/* sceSasSetADSR */
int snd_voice_set_adsr(int voice, int flag, int a, int d, int s, int r)
{
    return sas_ready ? __sceSasSetADSR(sas_core, voice, flag, a, d, s, r) : SAS_NOT_INIT;
}

/* sceSasSetADSRmode */
int snd_voice_set_curve(int voice, int flag, int a, int d, int s, int r)
{
    return sas_ready ? __sceSasSetADSRmode(sas_core, voice, flag, a, d, s, r) : SAS_NOT_INIT;
}

/* sceSasSetSL: sustain level */
int snd_voice_set_sl(int voice, int level)
{
    return sas_ready ? __sceSasSetSL(sas_core, voice, level) : SAS_NOT_INIT;
}

/* sceSasSetSimpleADSR */
int snd_voice_set_simple_adsr(int voice, int adsr1, int adsr2)
{
    return sas_ready ? __sceSasSetSimpleADSR(sas_core, voice, adsr1, adsr2) : SAS_NOT_INIT;
}

/* sceSasGetEndFlag: mask of the voices that finished */
int snd_sas_end_flags(void)
{
    return sas_ready ? __sceSasGetEndFlag(sas_core) : 0;
}

/* sceSasGetEnvelopeHeight */
int snd_voice_get_envelope(int voice)
{
    return sas_ready ? __sceSasGetEnvelopeHeight(sas_core, voice) : SAS_NOT_INIT;
}

/* sceSasRevVON: dry / wet switches */
int snd_fx_set_wet_on(int dry, int wet)
{
    return sas_ready ? __sceSasRevVON(sas_core, dry, wet) : SAS_NOT_INIT;
}

/* sceSasRevEVOL: effect volume */
int snd_fx_set_dry(int l, int r)
{
    return sas_ready ? __sceSasRevEVOL(sas_core, l, r) : SAS_NOT_INIT;
}

/* sceSasRevType */
int snd_fx_set_type(int type)
{
    return sas_ready ? __sceSasRevType(sas_core, type) : SAS_NOT_INIT;
}

/* sceSasRevParam: delay and feedback */
int snd_fx_set_param(int delay, int feedback)
{
    return sas_ready ? __sceSasRevParam(sas_core, delay, feedback) : SAS_NOT_INIT;
}
#endif

/* sceSasInit with the default grain of 256 samples */
#ifdef __MWERKS__
asm int snd_sas_open_default(void)
{
    addiu sp, sp, -16
    sw ra, 0(sp)
    jal snd_sas_open
    addiu a0, zero, 256
    lw ra, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
int snd_sas_open_default(void)
{
    return snd_sas_open(0x100);
}
#endif

#if !defined(__MWERKS__) || defined(NON_MATCHING) /* GCC: the init flag through a split lui/lo */
/* sceSasExit */
int snd_sas_close(void)
{
    if (sas_ready != 1) {
        return SAS_NOT_INIT;
    }
    sas_ready = 0;
    return 0;
}
#endif
