/* sceMpeg (port step 7a, docs/port_survey.md): the PSMF player of BOOT/movie.c at the API level, decoded by the
   system's 64-bit ffmpeg. The game builds a ring buffer whose callback (pmf_read_blocks) reads 2 KB packets of the
   .pmf through a MovieFile whose first word is the sceIo fd, so the file behind the ring is known (io.c); nothing
   in the ring is demuxed here, the ring only keeps the game's reader thread and end-of-stream logic honest: a Put
   calls the callback and counts the packets, each GetAu drains the ring to one packet (the reader keeps feeding
   until the whole stream went through), and once both streams are over the ring reads as empty, which is what the
   main loop waits for ("paused and the ring free"). Video: one ffmpeg subprocess per movie writes raw RGBA frames
   to a pipe (-fps_mode passthrough; 480x272 at 29.97 fps in every PMF of the game), read one frame ahead in
   GetAvcAu and copied into the game's 512-wide 8888 buffer in AvcDecode; no cache (90 s of raw frames is 1.4 GB,
   and ffmpeg decodes this size at hundreds of fps). Audio: the Atrac3+ elementary stream (private stream 1, 4-byte
   sub-header per PES, 8-byte 0x0FD0 header per frame) is demuxed here, wrapped in the RIFF/AT3 header the game's own
   clips use and decoded by atrac.c's cache (ffmpeg once, ~/.cache/tagforce/at3/<hash>.pcm, mmapped); AtracDecode
   hands out 2048 stereo frames per call. Timestamps as PPSSPP: first pts of the header, +3003 per video AU and
   +4180 per audio AU. Without ffmpeg the two streams are empty and the movie ends at once, as with the old stubs.
   ponytail: one movie at a time (the game plays one), the file is identified through the fd, not by demuxing. */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

extern char **environ;
int dprintf(int fd, const char *fmt, ...);
int host_io_fd_path(int fd, char *out, int n);
void *host_at3_decode(const unsigned char *buf, int size, size_t *len);

#define ERR_NO_DATA 0x80618001
#define ERR_INVALID 0x806101FE
#define VIDEO_STEP 3003
#define AUDIO_STEP 4180
#define AUDIO_FRAMES 2048
#define HDR 96   /* RIFF(12) + fmt(8+52) + fact(8+8) + data(8) */

typedef struct {
    int packets, avail;   /* capacity; packets holding data (the firmware's packetsAvail) */
    void *data;
    int (*cb)(void *, int, void *);
    void *arg;
} Rb;

typedef struct {
    Rb *rb;
    int opened, offset, first_pts, w, h, nv, na, stream_v, stream_a;
    pid_t pid;              /* ffmpeg (video) */
    int pipe, veof, have_frame;
    unsigned char *frame;   /* one w*h RGBA frame read ahead */
    const short *pcm;       /* decoded audio, stereo s16 */
    void *map;
    size_t map_len;
    int total_a, pos_a;     /* stereo frames */
} Mpeg;

static Mpeg m;
static unsigned char es_buf[2112];

static unsigned int be32(const unsigned char *p) { return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]; }
static void put16(unsigned char *p, unsigned int v) { p[0] = v; p[1] = v >> 8; }
static void put32(unsigned char *p, unsigned int v) { put16(p, v); put16(p + 2, v >> 16); }

static Mpeg *ctx(void *mpeg) { return mpeg && *(Mpeg **)mpeg == &m ? &m : 0; }

/* --- video: ffmpeg -> pipe --- */

static void open_video(Mpeg *c, const char *path)
{
    char *argv[] = { "ffmpeg", "-nostdin", "-loglevel", "fatal", "-i", (char *)path, "-an", "-f", "rawvideo",
        "-pix_fmt", "rgba", "-fps_mode", "passthrough", "pipe:1", 0 };
    posix_spawn_file_actions_t fa;
    int p[2];

    c->veof = 1;
    if (pipe(p) < 0)
        return;
    posix_spawn_file_actions_init(&fa);
    posix_spawn_file_actions_adddup2(&fa, p[1], 1);
    posix_spawn_file_actions_addclose(&fa, p[0]);
    posix_spawn_file_actions_addclose(&fa, p[1]);
    if (posix_spawnp(&c->pid, argv[0], &fa, 0, argv, environ) != 0) {
        dprintf(2, "mpeg: ffmpeg not found: no video\n");
        c->pid = 0;
        close(p[0]);
    } else {
        fcntl(p[0], F_SETPIPE_SZ, 1 << 20);   /* about two frames ahead */
        c->pipe = p[0];
        c->veof = 0;
    }
    posix_spawn_file_actions_destroy(&fa);
    close(p[1]);
}

static void close_video(Mpeg *c)
{
    if (c->pid) {
        kill(c->pid, SIGKILL);
        waitpid(c->pid, 0, 0);
        c->pid = 0;
    }
    if (c->pipe > 0)
        close(c->pipe);
    c->pipe = 0;
    c->veof = 1;
}

/* the next frame into c->frame; 0 at the end of the stream */
static int read_frame(Mpeg *c)
{
    int size = c->w * c->h * 4, got = 0, n;

    if (c->veof)
        return 0;
    if (!c->frame)
        c->frame = malloc(size);
    while (got < size) {
        n = read(c->pipe, c->frame + got, size - got);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            break;
        got += n;
    }
    if (got < size) {
        c->veof = 1;
        return 0;
    }
    return 1;
}

/* --- audio: PSMF private stream 1 -> RIFF/AT3 -> atrac.c --- */

static void open_audio(Mpeg *c, const char *path)
{
    /* the fmt chunk of the game's own AT3 clips (WAVE_FORMAT_EXTENSIBLE, Atrac3+ GUID), block align patched */
    static const unsigned char fmt[52] = { 0xfe, 0xff, 2, 0, 0x44, 0xac, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x22, 0,
        0, 8, 3, 0, 0, 0, 0xbf, 0xaa, 0x23, 0xe9, 0x58, 0xcb, 0x71, 0x44, 0xa1, 0x19, 0xff, 0xfa, 0x01, 0xe4,
        0xce, 0x62, 1, 0, 0x28, 0x2e, 0, 0, 0, 0, 0, 0, 0, 0 };
    struct stat st;
    unsigned char *d, *es, *out, *p;
    size_t size, n_es = 0, pos, o, n_out, frames = 0, fs, ba = 0, rate;
    int fd;

    if ((fd = open(path, O_RDONLY)) < 0)
        return;
    if (fstat(fd, &st) != 0 || (d = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
        close(fd);
        return;
    }
    close(fd);
    size = st.st_size;
    es = malloc(size);
    /* MPEG-PS pack walk: BA pack header (14 + stuffing), everything else a PES with a 16-bit length */
    for (pos = c->offset; pos + 6 <= size && d[pos] == 0 && d[pos + 1] == 0 && d[pos + 2] == 1;) {
        size_t len = d[pos + 4] << 8 | d[pos + 5];
        if (d[pos + 3] == 0xBA) {
            pos += 14 + (d[pos + 13] & 7);
            continue;
        }
        if (d[pos + 3] == 0xBD && pos + 9 <= size) {
            size_t hl = d[pos + 8], start = pos + 9 + hl + 4, end = pos + 6 + len;
            if (end > size)
                end = size;
            if (start < end) {
                memcpy(es + n_es, d + start, end - start);
                n_es += end - start;
            }
        }
        pos += 6 + len;
    }
    munmap(d, size);
    /* frames: 0F D0 ss ss 00 00 00 00, size = (ss & 0x3FF) * 8 + 16 with the header; the decoder wants them bare */
    out = malloc(n_es + 128);
    p = out + HDR;
    n_out = 0;
    for (o = 0; o + 8 <= n_es && es[o] == 0x0F && es[o + 1] == 0xD0; o += fs) {
        fs = (((es[o + 2] & 3) << 8 | es[o + 3]) * 8) + 16;
        if (o + fs > n_es)
            break;
        if (!ba)
            ba = fs - 8;
        if (fs - 8 != ba)
            break;
        memcpy(p + n_out, es + o + 8, ba);
        n_out += ba;
        frames++;
    }
    free(es);
    if (!frames) {
        free(out);
        return;
    }
    rate = ba * 44100 / AUDIO_FRAMES;
    p = out;
    memcpy(p, "RIFF", 4);
    put32(p + 4, HDR - 8 + n_out);
    memcpy(p + 8, "WAVEfmt ", 8);
    put32(p + 16, sizeof fmt);
    memcpy(p + 20, fmt, sizeof fmt);
    put32(p + 28, rate);
    put16(p + 32, ba);
    p += 20 + sizeof fmt;
    memcpy(p, "fact", 4);
    put32(p + 4, 8);
    put32(p + 8, frames * AUDIO_FRAMES);
    put32(p + 12, AUDIO_FRAMES);
    memcpy(p + 16, "data", 4);
    put32(p + 20, n_out);
    c->map = host_at3_decode(out, (int)(HDR + n_out), &c->map_len);
    free(out);
    if (c->map) {
        c->pcm = c->map;
        c->total_a = (int)(c->map_len / 4);
    } else {
        c->total_a = (int)(frames * AUDIO_FRAMES);   /* silence of the same length: the game's A/V logic goes on */
    }
}

static void close_audio(Mpeg *c)
{
    if (c->map)
        munmap(c->map, c->map_len);
    c->map = 0;
    c->pcm = 0;
    c->total_a = c->pos_a = 0;
}

static void prepare(Mpeg *c)
{
    char path[1024];
    int fd = c->rb && c->rb->arg ? *(int *)c->rb->arg : -1;   /* the game's MovieFile: fd first */

    c->opened = 1;
    if (fd < 0 || host_io_fd_path(fd, path, sizeof path) < 0) {
        dprintf(2, "mpeg: no file behind the ring buffer\n");
        return;
    }
    if (getenv("TAGFORCE_PMF"))   /* test knob: play this host file instead (any PMF through the game's player) */
        snprintf(path, sizeof path, "%s", getenv("TAGFORCE_PMF"));
    open_video(c, path);
    open_audio(c, path);
    dprintf(2, "mpeg: %s %dx%d, audio %d frames\n", path, c->w, c->h, c->total_a / AUDIO_FRAMES);
}

/* a GetAu drains the ring to one packet so the reader goes on; empty once both streams ended (the end test) */
static void consume(Mpeg *c)
{
    if (!c->rb)
        return;
    if (c->veof && !c->have_frame && c->pos_a >= c->total_a)
        c->rb->avail = 0;
    else if (c->rb->avail > 1)
        c->rb->avail = 1;
}

static void set_au(unsigned int *au, int pts)
{
    au[0] = 0;
    au[1] = pts;
    au[2] = 0;
    au[3] = pts;
}

/* --- API --- */

int sceMpegInit(void) { return 0; }
int sceMpegFinish(void) { return 0; }
int sceMpegQueryMemSize(int mode) { return 0x10000; }
int sceMpegRingbufferQueryMemSize(int packets) { return packets * (2048 + 104); }

int sceMpegRingbufferConstruct(void *rb, int packets, void *data, int size, int (*cb)(void *, int, void *), void *arg)
{
    Rb *r = rb;

    memset(r, 0, sizeof *r);
    r->packets = packets;
    r->data = data;
    r->cb = cb;
    r->arg = arg;
    return 0;
}

int sceMpegRingbufferDestruct(void *rb) { return 0; }

int sceMpegRingbufferAvailableSize(void *rb)
{
    Rb *r = rb;
    return r->packets - r->avail;
}

int sceMpegRingbufferPut(void *rb, int packets, int available)
{
    Rb *r = rb;
    int n;

    if (packets > available)
        packets = available;
    if (packets > r->packets - r->avail)
        packets = r->packets - r->avail;
    if (packets <= 0 || !r->cb)
        return 0;
    n = r->cb(r->data, packets, r->arg);   /* the data is not kept: nothing here demuxes the ring */
    if (n > 0)
        r->avail += n;
    return n;
}

int sceMpegCreate(void *mpeg, void *data, int size, void *rb, int frame_width, int mode, int ddrtop)
{
    memset(&m, 0, sizeof m);
    m.rb = rb;
    m.w = 480;
    m.h = 272;
    *(Mpeg **)mpeg = &m;
    return 0;
}

int sceMpegDelete(void *mpeg)
{
    Mpeg *c = ctx(mpeg);

    if (!c)
        return ERR_INVALID;
    close_video(c);
    close_audio(c);
    free(c->frame);
    c->frame = 0;
    c->rb = 0;
    *(Mpeg **)mpeg = 0;
    return 0;
}

int sceMpegQueryStreamOffset(void *mpeg, void *buf, int *offset)
{
    Mpeg *c = ctx(mpeg);
    const unsigned char *h = buf;

    if (!c || memcmp(h, "PSMF", 4) != 0)
        return ERR_INVALID;
    *offset = c->offset = be32(h + 8);
    c->first_pts = be32(h + 0x56);
    if (h[0x8E] && h[0x8F]) {   /* first stream entry (video): width/16, height/16 */
        c->w = h[0x8E] * 16;
        c->h = h[0x8F] * 16;
    }
    return 0;
}

int sceMpegQueryStreamSize(void *buf, int *size)
{
    const unsigned char *h = buf;

    if (memcmp(h, "PSMF", 4) != 0)
        return ERR_INVALID;
    *size = be32(h + 0xC);
    return *size & 2047 ? ERR_INVALID : 0;
}

void *sceMpegRegistStream(void *mpeg, int type, int num)
{
    Mpeg *c = ctx(mpeg);

    if (!c)
        return 0;
    return type == 0 ? &c->stream_v : &c->stream_a;
}

int sceMpegUnRegistStream(void *mpeg, void *stream) { return 0; }
void *sceMpegMallocAvcEsBuf(void *mpeg) { return es_buf; }
int sceMpegFreeAvcEsBuf(void *mpeg, void *es) { return 0; }

int sceMpegInitAu(void *mpeg, void *es, void *au)
{
    unsigned int *a = au;

    a[0] = a[1] = a[2] = a[3] = 0xFFFFFFFF;
    a[4] = (unsigned int)es;
    a[5] = es == es_buf ? 2048 : 2112;
    return 0;
}

int sceMpegQueryAtracEsSize(void *mpeg, int *es_size, int *out_size)
{
    *es_size = 2112;
    *out_size = AUDIO_FRAMES * 4;
    return 0;
}

int sceMpegGetAvcAu(void *mpeg, void *stream, void *au, int *attr)
{
    Mpeg *c = ctx(mpeg);

    if (!c)
        return ERR_INVALID;
    if (!c->opened)
        prepare(c);
    if (!c->have_frame)
        c->have_frame = read_frame(c);
    consume(c);
    if (!c->have_frame)
        return ERR_NO_DATA;
    set_au(au, c->first_pts + c->nv++ * VIDEO_STEP);
    if (attr)
        *attr = 1;
    return 0;
}

int sceMpegAvcDecode(void *mpeg, void *au, int frame_width, void **buffer, int *got)
{
    Mpeg *c = ctx(mpeg);
    unsigned char *dst = buffer ? *buffer : 0;
    int y;

    if (!c)
        return ERR_INVALID;
    *got = 0;
    if (!c->have_frame || !dst)
        return 0;
    for (y = 0; y < c->h; y++)
        memcpy(dst + y * frame_width * 4, c->frame + y * c->w * 4, c->w * 4);
    c->have_frame = 0;
    *got = 1;
    return 0;
}

int sceMpegAvcDecodeStop(void *mpeg, int frame_width, void **buffer, int *got)
{
    *got = 0;
    return 0;
}

int sceMpeg_4571CC64(void *mpeg) { return 0; }   /* sceMpegAvcDecodeFlush */
int sceMpegFlushAllStream(void *mpeg) { return 0; }

int sceMpegGetAtracAu(void *mpeg, void *stream, void *au, int *attr)
{
    Mpeg *c = ctx(mpeg);

    if (!c)
        return ERR_INVALID;
    if (!c->opened)
        prepare(c);
    consume(c);
    if (c->pos_a >= c->total_a)
        return ERR_NO_DATA;
    set_au(au, c->first_pts + c->na++ * AUDIO_STEP);
    if (attr)
        *attr = 0;
    return 0;
}

int sceMpegAtracDecode(void *mpeg, void *au, void *buffer, int init)
{
    Mpeg *c = ctx(mpeg);
    int n;

    if (!c)
        return ERR_INVALID;
    n = c->total_a - c->pos_a;
    if (n > AUDIO_FRAMES)
        n = AUDIO_FRAMES;
    if (n < 0)
        n = 0;
    if (c->pcm)
        memcpy(buffer, c->pcm + c->pos_a * 2, n * 4);
    else
        n = 0;
    memset((char *)buffer + n * 4, 0, (AUDIO_FRAMES - n) * 4);
    c->pos_a += AUDIO_FRAMES;   /* the init call's output is discarded by the game: the encoder's lead-in */
    return 0;
}
