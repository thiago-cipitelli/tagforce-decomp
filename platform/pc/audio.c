/* sceAudio on ALSA (port step 6, docs/port_survey.md). Each reserved channel is its own 44.1 kHz s16 stereo PCM
   stream on the ALSA default device (PipeWire or dmix mixes the streams, as the PSP hardware mixes its 8 channels);
   a blocking output writes the block to the device off the CPU (host_cpu_leave/enter, kernel.c), so the device
   clock paces the sound thread. Without a device (TAGFORCE_HEADLESS=1, TAGFORCE_AUDIO=0, or the open failing) the
   host clock paces it as before. TAGFORCE_AUDIO=<name> picks the ALSA device (also when headless).
   TAGFORCE_WAV=path dumps the mix of every channel (summed at each channel's own cursor, saturated) as a WAV.
   ponytail: in practice the game writes one channel (wave_hw_ch[2], the snd thread; SceWaveMain only when a wave
   channel plays, which the game never does), so the per-channel streams are PipeWire's problem, not a mixer here. */
#define _GNU_SOURCE
#include <alsa/asoundlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

long long host_now_us(void);
void host_sleep_until(long long deadline);
void host_cpu_leave(void);
void host_cpu_enter(void);

typedef struct {
    int used, len, mono, vol_l, vol_r;
    snd_pcm_t *pcm;
    int pcm_tried;
    long long next;        /* clock pacing: when the last block ends */
    long long cursor;      /* frames written so far (WAV mix position) */
    short buf[0xFFC0 * 2];
} Ch;

static Ch ch[8];
static int wav_fd = -1;
static long long wav_frames;

static const char *device(void)
{
    const char *d = getenv("TAGFORCE_AUDIO");
    const char *h = getenv("TAGFORCE_HEADLESS");

    if (d)
        return strcmp(d, "0") && strcmp(d, "none") ? d : 0;
    return h && !strcmp(h, "1") ? 0 : "default";
}

static void open_pcm(Ch *c)
{
    const char *d = device();
    int r;

    c->pcm_tried = 1;
    if (!d)
        return;
    if ((r = snd_pcm_open(&c->pcm, d, SND_PCM_STREAM_PLAYBACK, 0)) < 0 ||
        (r = snd_pcm_set_params(c->pcm, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, 44100, 1, 120000)) < 0) {
        fprintf(stderr, "audio: %s: %s (no device, clock pacing)\n", d, snd_strerror(r));
        if (c->pcm)
            snd_pcm_close(c->pcm);
        c->pcm = 0;
        return;
    }
    fprintf(stderr, "audio: channel %d on %s\n", (int)(c - ch), d);
}

static void wav_open(void)
{
    const char *p = getenv("TAGFORCE_WAV");
    static const unsigned char hdr[44] = { 'R','I','F','F', 0,0,0,0, 'W','A','V','E', 'f','m','t',' ', 16,0,0,0, 1,0, 2,0,
        0x44,0xAC,0,0, 0x10,0xB1,2,0, 4,0, 16,0, 'd','a','t','a', 0,0,0,0 };

    if (!p || wav_fd >= 0)
        return;
    wav_fd = open(p, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (wav_fd < 0)
        return;
    write(wav_fd, hdr, 44);
}

/* adds a block at a channel's cursor: the file is the running mix of every channel */
static void wav_add(Ch *c, const short *s, int n)
{
    static short old[0xFFC0 * 2];
    long long off = 44 + c->cursor * 4;
    int i, got;

    if (wav_fd < 0)
        return;
    got = pread(wav_fd, old, n * 4, off);
    if (got < 0)
        got = 0;
    memset((char *)old + got, 0, n * 4 - got);
    for (i = 0; i < n * 2; i++) {
        int v = old[i] + s[i];
        old[i] = v < -0x8000 ? -0x8000 : v > 0x7FFF ? 0x7FFF : v;
    }
    pwrite(wav_fd, old, n * 4, off);
    if (c->cursor + n > wav_frames) {
        unsigned int d = (wav_frames = c->cursor + n) * 4, r = d + 36;
        unsigned char b[4];

        b[0] = r; b[1] = r >> 8; b[2] = r >> 16; b[3] = r >> 24;
        pwrite(wav_fd, b, 4, 4);
        b[0] = d; b[1] = d >> 8; b[2] = d >> 16; b[3] = d >> 24;
        pwrite(wav_fd, b, 4, 40);
    }
}

static Ch *get(int i) { return i >= 0 && i < 8 && ch[i].used ? &ch[i] : 0; }

int sceAudioChReserve(int i, int samples, int format)
{
    if (i < 0)
        for (i = 0; i < 8 && ch[i].used; i++)
            ;
    if (i >= 8)
        return (int)0x80260003;
    memset(&ch[i], 0, sizeof ch[i]);
    ch[i].used = 1;
    ch[i].len = samples;
    ch[i].mono = format == 1;
    ch[i].vol_l = ch[i].vol_r = 0x8000;
    wav_open();
    return i;
}

int sceAudioChRelease(int i)
{
    Ch *c = get(i);

    if (!c)
        return (int)0x80260001;
    if (c->pcm)
        snd_pcm_close(c->pcm);
    c->used = 0;
    return 0;
}

int sceAudioSetChannelDataLen(int i, int samples)
{
    Ch *c = get(i);

    if (!c)
        return (int)0x80260001;
    c->len = samples;
    return 0;
}

int sceAudioChangeChannelConfig(int i, int format)
{
    Ch *c = get(i);

    if (!c)
        return (int)0x80260001;
    c->mono = format == 1;
    return 0;
}

int sceAudioChangeChannelVolume(int i, int l, int r)
{
    Ch *c = get(i);

    if (!c)
        return (int)0x80260001;
    c->vol_l = l;
    c->vol_r = r;
    return 0;
}

int sceAudioGetChannelRestLength(int i) { return 0; }

/* one block: scaled copy, WAV mix, then the device write off the CPU or the clock wait */
static int output(int i, int l, int r, const short *buf)
{
    Ch *c = get(i);
    int n, k;

    if (!c)
        return (int)0x80260001;
    n = c->len;
    if (!buf) {   /* "wait for the channel to drain" */
        if (c->pcm)
            snd_pcm_drain(c->pcm);
        return 0;
    }
    for (k = 0; k < n; k++) {
        int sl = c->mono ? buf[k] : buf[k * 2], sr = c->mono ? buf[k] : buf[k * 2 + 1];

        c->buf[k * 2] = sl * l >> 15;
        c->buf[k * 2 + 1] = sr * r >> 15;
    }
    wav_add(c, c->buf, n);
    c->cursor += n;
    if (!c->pcm_tried)
        open_pcm(c);
    if (c->pcm) {
        host_cpu_leave();
        for (k = 0; k < n;) {
            snd_pcm_sframes_t w = snd_pcm_writei(c->pcm, c->buf + k * 2, n - k);

            if (w < 0) {
                if (snd_pcm_recover(c->pcm, w, 1) < 0) {
                    fprintf(stderr, "audio: write: %s\n", snd_strerror(w));
                    break;
                }
                continue;
            }
            k += w;
        }
        host_cpu_enter();
    } else {
        long long now = host_now_us();

        if (c->next < now)
            c->next = now;
        c->next += (long long)n * 1000000 / 44100;
        host_sleep_until(c->next);
    }
    return n;
}

int sceAudioOutputPannedBlocking(int i, int l, int r, void *buf) { return output(i, l, r, buf); }
int sceAudioOutputBlocking(int i, int vol, void *buf) { return output(i, vol, vol, buf); }
int sceAudioOutputPanned(int i, int l, int r, void *buf) { return output(i, l, r, buf); }   /* blocking too */
