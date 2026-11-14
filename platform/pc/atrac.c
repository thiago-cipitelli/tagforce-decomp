/* sceAtrac without a decoder (port step 5a, docs/port_survey.md): every stream decodes to silence, but with the
   length and loop points of its RIFF header ("fact", "smpl"), so a voice line or a jingle ends when it would on the
   PSP. The duel waits for the end of each character voice (dd_chr_anim_done -> voice_is_playing -> the sound
   object of the clip); with the stubs a clip never ended and the demo duel stopped at the first cut-in.
   ponytail: zeros instead of Atrac3+; decode with libavcodec (atrac3p) when sound comes in. */
#include <string.h>

typedef struct { int used, total, pos, loop_start, loop_end, loops; } At;

static At at[8];

static const unsigned char *chunk(const unsigned char *p, int size, const char *id)
{
    int o = 12;

    while (o + 8 <= size && o < 0x1000) {
        int len = p[o + 4] | p[o + 5] << 8 | p[o + 6] << 16 | p[o + 7] << 24;
        if (!memcmp(p + o, id, 4))
            return p + o + 8;
        o += 8 + ((len + 1) & ~1);
    }
    return 0;
}

static int u32le(const unsigned char *p) { return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24; }

int sceAtracSetDataAndGetID(const unsigned char *buf, int size)
{
    const unsigned char *f, *s;
    int i;

    for (i = 0; i < 8 && at[i].used; i++)
        ;
    if (i == 8)
        return 0x80630006;   /* no free ID */
    memset(&at[i], 0, sizeof at[i]);
    at[i].used = 1;
    at[i].loop_start = at[i].loop_end = -1;
    if (size >= 12 && !memcmp(buf, "RIFF", 4)) {
        if ((f = chunk(buf, size, "fact")))
            at[i].total = u32le(f);
        if ((s = chunk(buf, size, "smpl")) && u32le(s + 28) > 0 && u32le(s + 36 + 12) > u32le(s + 36 + 8)) {
            at[i].loop_start = u32le(s + 36 + 8);
            at[i].loop_end = u32le(s + 36 + 12);
        }
    }
    if (at[i].total <= 0)
        at[i].total = 2048;
    return i;
}

static At *get(int id) { return id >= 0 && id < 8 && at[id].used ? &at[id] : 0; }

int sceAtracReleaseAtracID(int id)
{
    if (get(id))
        at[id].used = 0;
    return 0;
}

int sceAtracGetMaxSample(int id, int *max)
{
    *max = 2048;
    return 0;
}

int sceAtracGetSoundSample(int id, int *end, int *loop_start, int *loop_end)
{
    At *a = get(id);

    *end = a ? a->total - 1 : 0;
    *loop_start = a ? a->loop_start : -1;
    *loop_end = a ? a->loop_end : -1;
    return 0;
}

/* as the firmware: a stream without loop points refuses (and keeps 0, which the mixer reads as "ends") */
int sceAtracSetLoopNum(int id, int n)
{
    At *a = get(id);

    if (!a || a->loop_start < 0)
        return 0x80630021;
    a->loops = n;
    return 0;
}

int sceAtracGetLoopStatus(int id, int *loops, unsigned int *status)
{
    At *a = get(id);

    *loops = a ? a->loops : 0;
    *status = 0;
    return 0;
}

int sceAtracResetPlayPosition(int id, int sample, int a, int b)
{
    if (get(id))
        at[id].pos = sample;
    return 0;
}

/* one frame (2048 stereo samples) of silence per call, as the Atrac3+ decoder does; a looping stream wraps inside
   the frame while loops remain (-1: forever), the last frame of the others is short */
int sceAtracDecodeData(int id, short *out, int *samples, int *end, int *remain)
{
    At *a = get(id);
    int n = 2048;

    *samples = 0;
    *end = 1;
    *remain = -1;
    if (!a)
        return 0x80630000;
    if (a->loops != 0) {
        a->pos += n;
        while (a->pos > a->loop_end) {
            a->pos -= a->loop_end + 1 - a->loop_start;
            if (a->loops > 0 && !--a->loops)
                break;
        }
    } else {
        if (a->pos >= a->total)
            return 0x80630024;   /* all data decoded */
        if (n > a->total - a->pos)
            n = a->total - a->pos;
        a->pos += n;
    }
    memset(out, 0, n * 4);
    *samples = n;
    *end = !a->loops && a->pos >= a->total;
    return 0;
}

int sceAtrac3plus_132F1ECA(void) { return 0; }   /* sceAtracReinit */
