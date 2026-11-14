/* sceAtrac (port step 6, docs/port_survey.md): Atrac3+ decoded by the system's ffmpeg as a subprocess. The game
   hands sceAtracSetDataAndGetID a whole RIFF/AT3 file from its sound bank; it is hashed (FNV-1a), decoded once to
   s16le stereo 44.1 kHz in $XDG_CACHE_HOME/tagforce/at3/<hash>.pcm (ffmpeg -i <hash>.at3), and the PCM file is
   mmapped. Decoding follows the header as before (step 5a): the length from "fact", the loop points from "smpl",
   2048 frames per sceAtracDecodeData, a looping stream wrapping inside the frame while loops remain (-1: forever),
   the last frame of the others short. Without ffmpeg (or on a failed decode) the stream is silence of that length.
   ponytail: one synchronous ffmpeg per new clip (a few hundred ms, once per clip per machine); a decode thread or
   ffmpeg's stdin/stdout pipes would hide that, add them when the first play of a track is noticeably late. */
#define _GNU_SOURCE
#include <fcntl.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;

typedef struct {
    int used, total, pos, loop_start, loop_end, loops;
    const short *pcm;   /* total stereo frames, or 0 for silence */
    void *map;
    size_t map_len;
} At;

static At at[8];
static char cache_dir[1024];
static int no_ffmpeg;

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

static const char *cache(void)
{
    const char *x = getenv("XDG_CACHE_HOME"), *h = getenv("HOME");

    if (cache_dir[0])
        return cache_dir;
    if (x && *x)
        snprintf(cache_dir, sizeof cache_dir, "%s/tagforce", x);
    else
        snprintf(cache_dir, sizeof cache_dir, "%s/.cache/tagforce", h ? h : ".");
    mkdir(cache_dir, 0755);
    strcat(cache_dir, "/at3");
    mkdir(cache_dir, 0755);
    return cache_dir;
}

static int run(char *const argv[])
{
    pid_t pid;
    int st;

    if (posix_spawnp(&pid, argv[0], 0, 0, argv, environ))
        return -1;
    if (waitpid(pid, &st, 0) < 0)
        return -1;
    return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}

/* the decoded clip, from the cache or through ffmpeg; 0 (silence) without ffmpeg */
static void *decode(const unsigned char *buf, int size, size_t *len)
{
    unsigned long long h = 0xcbf29ce484222325ULL;
    char pcm[1200], at3[1200], tmp[1200];
    struct stat st;
    int i, fd;
    void *p;

    for (i = 0; i < size; i++)
        h = (h ^ buf[i]) * 0x100000001b3ULL;
    snprintf(pcm, sizeof pcm, "%s/%016llx.pcm", cache(), h);
    if (stat(pcm, &st) != 0) {
        char *argv[] = { "ffmpeg", "-nostdin", "-loglevel", "fatal", "-y", "-i", at3, "-f", "s16le", "-ac", "2",
            "-ar", "44100", tmp, 0 };
        int r;

        if (no_ffmpeg)
            return 0;
        snprintf(at3, sizeof at3, "%s/%016llx.at3", cache(), h);
        snprintf(tmp, sizeof tmp, "%s/%016llx.tmp", cache(), h);
        if ((fd = open(at3, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
            return 0;
        write(fd, buf, size);
        close(fd);
        r = run(argv);
        if (!getenv("TAGFORCE_TRACE_ATRAC"))   /* tracing keeps the clip, to compare with ffmpeg by hand */
            unlink(at3);
        if (r != 0 || rename(tmp, pcm) != 0) {
            unlink(tmp);
            if (r < 0)
                no_ffmpeg = 1;
            fprintf(stderr, "atrac: ffmpeg %s (%d): silence\n", r < 0 ? "not found" : "failed", r);
            return 0;
        }
        if (stat(pcm, &st) != 0)
            return 0;
    }
    if (st.st_size < 4 || (fd = open(pcm, O_RDONLY)) < 0)
        return 0;
    p = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (p == MAP_FAILED)
        return 0;
    *len = st.st_size;
    return p;
}

int sceAtracSetDataAndGetID(const unsigned char *buf, int size)
{
    const unsigned char *f = 0, *s;
    int i, fact = 0, fact_len = 0, skip;

    for (i = 0; i < 8 && at[i].used; i++)
        ;
    if (i == 8)
        return 0x80630006;   /* no free ID */
    memset(&at[i], 0, sizeof at[i]);
    at[i].used = 1;
    at[i].loop_start = at[i].loop_end = -1;
    if (size >= 12 && !memcmp(buf, "RIFF", 4)) {
        if ((f = chunk(buf, size, "fact"))) {
            at[i].total = fact = u32le(f);
            fact_len = u32le(f - 4);
        }
        if ((s = chunk(buf, size, "smpl")) && u32le(s + 28) > 0 && u32le(s + 36 + 12) > u32le(s + 36 + 8)) {
            at[i].loop_start = u32le(s + 36 + 8);
            at[i].loop_end = u32le(s + 36 + 12);
        }
    }
    if (at[i].total <= 0)
        at[i].total = 2048;
    /* the game's sample 0 is decoder sample 368 + fact[1] (PPSSPP's FirstSampleOffsetFull for Atrac3+: the
       encoder delay); the "smpl" loop points are decoder samples, sceAtracGetSoundSample gives them from sample 0 */
    skip = fact_len >= 8 ? 368 + u32le(f + 4) : 0;
    if (at[i].loop_start >= 0) {
        at[i].loop_start -= skip;
        at[i].loop_end -= skip;
    }
    at[i].map = decode(buf, size, &at[i].map_len);
    if (at[i].map) {
        int decoded = at[i].map_len / 4;

        if (decoded < skip + at[i].total)   /* not the delay we expected: keep the length, counted from the end */
            skip = decoded > at[i].total ? decoded - at[i].total : 0;
        at[i].pcm = (const short *)at[i].map + skip * 2;
        if (decoded - skip < at[i].total)
            at[i].total = decoded - skip;
        if (getenv("TAGFORCE_TRACE_ATRAC"))
            fprintf(stderr, "atrac %d: %d bytes, fact %d (%d bytes: %d), decoded %d frames, skip %d, loop %d..%d\n", i,
                size, fact, fact_len, fact_len >= 8 ? u32le(f + 4) : 0, decoded, skip, at[i].loop_start, at[i].loop_end);
    }
    if (at[i].loop_end >= at[i].total)
        at[i].loop_end = at[i].total - 1;
    if (at[i].loop_end >= 0 && at[i].loop_start >= at[i].loop_end)
        at[i].loop_start = at[i].loop_end = -1;
    return i;
}

static At *get(int id) { return id >= 0 && id < 8 && at[id].used ? &at[id] : 0; }

int sceAtracReleaseAtracID(int id)
{
    At *a = get(id);

    if (a) {
        if (a->map)
            munmap(a->map, a->map_len);
        a->used = 0;
    }
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

    if (!a || a->loop_end < 0)
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
    At *s = get(id);

    if (s)
        s->pos = sample < 0 ? 0 : sample > s->total ? s->total : sample;
    return 0;
}

/* one frame (2048 stereo samples) per call, as the Atrac3+ decoder does; a looping stream wraps inside the frame
   while loops remain (-1: forever), the last frame of the others is short */
int sceAtracDecodeData(int id, short *out, int *samples, int *end, int *remain)
{
    At *a = get(id);
    int n = 2048, done = 0;

    *samples = 0;
    *end = 1;
    *remain = -1;
    if (!a)
        return 0x80630000;
    if (a->loops == 0 && a->pos >= a->total)
        return 0x80630024;   /* all data decoded */
    while (done < n) {
        int lim = a->loops != 0 ? a->loop_end + 1 : a->total, k = lim - a->pos;

        if (k > n - done)
            k = n - done;
        if (k > 0) {
            if (a->pcm)
                memcpy(out + done * 2, a->pcm + a->pos * 2, k * 4);
            else
                memset(out + done * 2, 0, k * 4);
            a->pos += k;
            done += k;
        }
        if (a->loops != 0 && a->pos > a->loop_end) {
            a->pos = a->loop_start < 0 ? 0 : a->loop_start;   /* -368 for a loop from the start (see SetData) */
            if (a->loops > 0 && !--a->loops)
                break;
        } else if (a->loops == 0 && a->pos >= a->total) {
            break;
        }
    }
    *samples = done;
    *end = !a->loops && a->pos >= a->total;
    return 0;
}

int sceAtrac3plus_132F1ECA(void) { return 0; }   /* sceAtracReinit */
