// @range 0x0885133C-0x08853A2C
// @data 0x088EF098-0x088EF28C
// @asm 0x088513E0-0x08851BA4
// @asm 0x08851D10-0x08851F4C
// @asm 0x08852070-0x08852144
// @asm 0x08852708-0x088527AC
// @asm 0x088527E8-0x08852900
// @asm 0x08852BCC-0x08852C48
/* PMF movie player (Sony PSMF sample code): movie_run() builds the sceMpeg ring buffer and decoder
   in the block set by movie_set_memory, starts the display/read/sound threads and runs the decode
   loop until the stream ends or the user skips. Control flow between threads uses two event flags
   (MovieCtrl) and per-ring semaphores. */
#define sceIoLseek sceIoLseek_int32 /* psp.h declares the int-returning form used by ehfile.c */
#include "psp.h"
#include "libc.h"
#include "movie.h"
#undef sceIoLseek
long long sceIoLseek(SceUID fd, long long offset, int whence);

int dbg_printf(const char *fmt, ...);
int memcmp2(const void *a, const void *b, unsigned int n);
void vblank_handler_set(int sub, void *handler, void *arg);
int sceWaveInit_wrap(void);
int wave_set_sample(int ch, int samples);
int wave_exit(void);
int wave_audio_write(int ch, int vol_l, int vol_r, void *buf);
int wave_rest_samples(int ch);

/* threads and shared state (bss) */
extern SceUID movie_disp_thread, movie_read_thread, movie_snd_thread;
extern unsigned int pad_prev, pad_cur;
extern MovieCtrl **movie_ctrl_ref;      /* &ctrl of the running movie_run, 0 when idle */
extern unsigned int movie_flags;        /* 1 mute, 2 any key skips, 4 not skippable */
extern unsigned int movie_mem_size, movie_mem_base;
extern int movie_result;                /* 1 restart requested, 10 skipped */
extern int movie_has_sound;
extern int snd_out_idx;
extern unsigned int snd_out[2];
extern int snd_pts_step, snd_delay_count, snd_play_count;
extern int vid_pts_step;
extern void *avc_frame_ptr;
extern void *disp_vram_back, *disp_vram_front;
extern int disp_frame_loss, disp_frame_count;

/* forward declarations (definition order follows the binary) */
int ctrl_abort(MovieCtrl *c);
int ctrl_is_stop_requested(MovieCtrl *c);
int ctrl_is_error(MovieCtrl *c);
int ctrl_init(MovieCtrl *c);
int ctrl_term(MovieCtrl *c);
int ctrl_clear(MovieCtrl *c);
int ctrl_wait_vblank1(MovieCtrl *c);
int ctrl_wait_vblank2(MovieCtrl *c);
int ctrl_wait_start_display(MovieCtrl *c);
int ctrl_wait_start_sound(MovieCtrl *c);
int movie_umd_term(void);
int movie_check_end(MovieVid *vid, PmfReader *reader, MovieCtrl *ctrl);
int movie_main_loop(void *mpeg, void *rb, int packets, MovieAu *au, PmfReader *reader, MovieVid *vid, MovieCtrl *ctrl);
int movie_read_header(void *mpeg, MovieFile *file, int *size, int *offset);
int movie_poll_input(); /* called with a stray argument in movie_main_loop */
int movie_file_init(MovieFile *f, const char *path);
int movie_file_open(MovieFile *f, const char *path, int flags);
int movie_file_close(MovieFile *f);
int movie_file_term(MovieFile *f);
int movie_read_async(MovieFile *f, void *buf, int size);
long long movie_seek(MovieFile *f, long long offset, int whence);
int movie_decode_atrac(void *mpeg, MovieAu *au, MovieVid *vid, int init);
int movie_decode_avc(void *mpeg, MovieAu *au, MovieVid *vid);
int movie_decode_avc_stop(void *mpeg, MovieVid *vid);
int au_init(MovieAu *a, void *avc_stream, void *avc_au, void *atrac_stream, void *atrac_au);
int msnd_init(MovieSnd *s, int bufsize, int nbufs, MovieCtrl *ctrl, unsigned int *mem, int mute);
int snd_reset(MovieSnd *s);
int snd_term(MovieSnd *s, int mute);
void snd_get_write_buf(MovieSnd *s, void **out);
int snd_push_pts(MovieSnd *s, int pts);
int snd_peek_pts(MovieSnd *s, int *out);
int snd_commit(MovieSnd *s);
int snd_free_count(MovieSnd *s);
int snd_count(MovieSnd *s);
int snd_is_stop_requested(MovieSnd *s);
int snd_thread(SceSize args, void *argp);
int snd_output(MovieSnd *s, void *buf);
int snd_play_next(MovieSnd *s);
void msnd_set_volume(int v);
int vid_init(MovieVid *v, MovieDisp *disp, MovieSnd *snd, int n, unsigned int *mem);
int vid_reset(MovieVid *v);
int vid_term(MovieVid *v);
int vid_zero(MovieVid *v);
int vid_push_pts(MovieVid *v, int pts);
int vid_pop_pts(MovieVid *v, int *out);
int vid_sync_state(MovieVid *v);
int ctrl_vblank_handler(int sub, MovieCtrl *c);
int disp_init(MovieDisp *d, int framesize, int nframes, MovieCtrl *ctrl, unsigned int *mem);
int disp_present(void *frame);
int disp_reset(MovieDisp *d);
int disp_term(MovieDisp *d);
void disp_get_write_frame(MovieDisp *d, void **out);
int disp_set_pts(MovieDisp *d, int pts);
int disp_peek_pts(MovieDisp *d, int *out);
int disp_commit(MovieDisp *d);
int disp_free_count(MovieDisp *d);
int disp_count(MovieDisp *d);
int disp_check_stop(MovieDisp *d);
int disp_set_state(MovieDisp *d, int state);
int disp_get_state(MovieDisp *d);
void disp_clear_delay_flag(MovieDisp *d);
int disp_delay_requested(MovieDisp *d);
int disp_thread(SceSize args, void *argp);
int disp_show_next(MovieDisp *d);

/* Called before sceKernelExitGame: aborts a running movie and joins its threads. */
void movie_exit_cleanup(void)
{
    if (movie_ctrl_ref != 0) {
        if (*movie_ctrl_ref != 0)
            ctrl_abort(*movie_ctrl_ref);
        movie_umd_term();
        if (movie_has_sound == 1 && movie_snd_thread > 0)
            sceKernelWaitThreadEnd(movie_snd_thread, 0);
        if (movie_disp_thread > 0)
            sceKernelWaitThreadEnd(movie_disp_thread, 0);
        if (movie_read_thread > 0)
            sceKernelWaitThreadEnd(movie_read_thread, 0);
    }
}

int movie_play_default(const char *path, unsigned int flags, int volume)
{
    return movie_run(path, flags, volume, 32);
}

/* .data objects shared with the functions kept in asm (thread names, volume, device prefixes) */
char movie_name_disp_thread[] = "displayThread";
char movie_name_read_thread[] = "readThread";
char movie_name_snd_thread[] = "soundThread";
int snd_volume = 0x8000;

#define ALIGN64(x) (((x) + 0x3F) & ~0x3F)

/* Plays a .pmf: returns 0 at the end, 1 when skipped. flags: 1 mute, 2 any key skips, 4 no skip. */
#ifdef NON_MATCHING /* structure matches; register allocation and scheduling differ */
int movie_run(const char *path, unsigned int flags, int volume, int packets)
{
    void *mpeg;
    unsigned char rb[0x80];
    unsigned char avc_au[64];
    unsigned char atrac_au[64];
    MovieFile file;
    int stream_size;
    int es_size;
    int out_size;
    int offset;
    PmfReader *reader;
    MovieVid *vid;
    MovieCtrl *ctrl;
    unsigned int mem;
    unsigned int mpeg_data;
    int prio;
    int sampling;
    int result;
    MovieAu *au;
    MovieDisp *disp;
    MovieSnd *snd;
    void *avc_stream, *atrac_stream, *avc_es;
    int rb_packets, rb_size, mpeg_size, r;
    unsigned int rb_data, atrac_es;

    mem = movie_mem_base;
    movie_flags = flags;
    movie_has_sound = 1;
    reader = 0;
    vid = 0;
    ctrl = 0;
    avc_es = 0;
    result = 0;
    avc_stream = 0;
    atrac_stream = 0;
    if (movie_file_init(&file, path) < 0)
        goto done;
    msnd_set_volume(volume);
    movie_ctrl_ref = &ctrl;
    if (sceMpegInit() != 0)
        goto cleanup;
    rb_packets = packets << 2;
    rb_size = sceMpegRingbufferQueryMemSize(rb_packets);
    if (rb_size < 0)
        goto cleanup;
    mpeg_size = sceMpegQueryMemSize(0);
    if (mpeg_size < 0)
        goto cleanup;
    au = (MovieAu *)mem;
    ctrl = (MovieCtrl *)ALIGN64(mem + 0x10);
    disp = (MovieDisp *)ALIGN64((unsigned int)ctrl + 8);
    snd = (MovieSnd *)ALIGN64((unsigned int)disp + 0x28);
    reader = (PmfReader *)ALIGN64((unsigned int)snd + 0x28);
    vid = (MovieVid *)ALIGN64((unsigned int)reader + 0x10);
    rb_data = ALIGN64((unsigned int)vid + 0x20);
    mpeg_data = ALIGN64(rb_data + rb_size);
    atrac_es = ALIGN64(mpeg_data + mpeg_size);
    mem = ALIGN64(atrac_es);
    if (sceMpegRingbufferConstruct(rb, rb_packets, (void *)rb_data, rb_size, pmf_read_blocks, &file) != 0)
        goto cleanup;
    if (sceMpegCreate(&mpeg, (void *)mpeg_data, mpeg_size, rb, 0x200, 0, 0) != 0)
        goto cleanup;
    if (movie_file_open(&file, path, 1) < 0)
        goto cleanup;
    if (movie_read_header(&mpeg, &file, &stream_size, &offset) != 0)
        goto cleanup;
    avc_stream = sceMpegRegistStream(&mpeg, 0, 0);
    if (avc_stream == 0)
        goto cleanup;
    if (movie_has_sound == 1) {
        atrac_stream = sceMpegRegistStream(&mpeg, 1, 0);
        if (atrac_stream == 0)
            goto cleanup;
    }
    avc_es = sceMpegMallocAvcEsBuf(&mpeg);
    if (avc_es == 0)
        goto cleanup;
    if (sceMpegInitAu(&mpeg, avc_es, avc_au) != 0)
        goto cleanup;
    if (movie_has_sound == 1) {
        if (sceMpegQueryAtracEsSize(&mpeg, &es_size, &out_size) != 0)
            goto cleanup;
        mem += es_size;
        mem = ALIGN64(mem);
        if (sceMpegInitAu(&mpeg, (void *)atrac_es, atrac_au) != 0)
            goto cleanup;
    }
    sampling = sceCtrlSetSamplingMode(0);
    au_init(au, avc_stream, avc_au, atrac_stream, atrac_au);
    if (ctrl_init(ctrl) < 0)
        goto free;
    if (disp_init(disp, 0x88000, 2, ctrl, &mem) < 0)
        goto free;
    if (movie_has_sound == 1) {
        if (msnd_init(snd, out_size, 2, ctrl, &mem, flags & 1) < 0)
            goto free;
    }
    if (pmf_reader_attach(reader, rb, ctrl) < 0)
        goto free;
    if (vid_init(vid, disp, snd, 4, &mem) < 0)
        goto free;
    prio = sceKernelGetThreadCurrentPriority();
    sceKernelChangeThreadPriority(sceKernelGetThreadId(), 0x3F);
    movie_disp_thread = sceKernelCreateThread(movie_name_disp_thread, disp_thread, 0x3E, 0x800, 0, 0);
    movie_read_thread = sceKernelCreateThread(movie_name_read_thread, pmf_reader_thread, 0x40, 0x800, 0, 0);
    if (movie_has_sound == 1)
        movie_snd_thread = sceKernelCreateThread(movie_name_snd_thread, snd_thread, 0x3D, 0x800, 0, 0);
    movie_result = 0;
    if (movie_seek(&file, offset, 0) >= 0) {
        if (movie_has_sound == 1)
            snd_reset(snd);
        disp_reset(disp);
        pmf_reader_start(reader, stream_size);
        vid_reset(vid);
        ctrl_clear(ctrl);
        if (movie_has_sound == 1)
            sceKernelStartThread(movie_snd_thread, 4, &vid);
        sceKernelStartThread(movie_disp_thread, 4, &vid);
        sceKernelStartThread(movie_read_thread, 4, &reader);
        r = movie_main_loop(&mpeg, rb, packets, au, reader, vid, ctrl);
        if (r == 0) {
            if (movie_has_sound == 1)
                sceKernelWaitThreadEnd(movie_snd_thread, 0);
            sceKernelWaitThreadEnd(movie_disp_thread, 0);
            sceKernelWaitThreadEnd(movie_read_thread, 0);
        } else if (r >= 0) {
            if (movie_result == 1) {
                if (movie_has_sound == 1)
                    sceKernelWaitThreadEnd(movie_snd_thread, 0);
                sceKernelWaitThreadEnd(movie_disp_thread, 0);
                sceKernelWaitThreadEnd(movie_read_thread, 0);
                if (sceMpegFlushAllStream(&mpeg) == 0)
                    movie_result = 0;
            } else if (movie_result == 10) {
                result = 1;
            }
        }
    }
    if (movie_has_sound == 1)
        sceKernelWaitThreadEndCB(movie_snd_thread, 0);
    sceKernelWaitThreadEndCB(movie_disp_thread, 0);
    sceKernelWaitThreadEndCB(movie_read_thread, 0);
    if (movie_has_sound == 1)
        sceKernelDeleteThread(movie_snd_thread);
    sceKernelDeleteThread(movie_disp_thread);
    sceKernelDeleteThread(movie_read_thread);
free:
    if (reader != 0) {
        pmf_reader_term();
        reader = 0;
    }
    if (vid != 0) {
        vid_term(vid);
        vid = 0;
    }
    if (snd != 0)
        snd_term(snd, flags & 1);
    if (disp != 0)
        disp_term(disp);
    if (ctrl != 0) {
        ctrl_term(ctrl);
        ctrl = 0;
    }
cleanup:
    if (avc_es != 0)
        sceMpegFreeAvcEsBuf(&mpeg, avc_es);
    if (avc_stream != 0)
        sceMpegUnRegistStream(&mpeg, avc_stream);
    if (movie_has_sound == 1 && atrac_stream != 0)
        sceMpegUnRegistStream(&mpeg, atrac_stream);
    movie_file_close(&file);
    sceMpegDelete(&mpeg);
    sceMpegRingbufferDestruct(rb);
    sceMpegFinish();
done:
    movie_file_term(&file);
    sceKernelChangeThreadPriority(sceKernelGetThreadId(), prio);
    sceCtrlSetSamplingMode(sampling);
    memset(sceGeEdramGetAddr(), 0, 0x110000);
    movie_ctrl_ref = 0;
    return result;
}
#endif

/* 0 once every ring is drained (or stop was requested), else -1. */
int movie_check_end(MovieVid *vid, PmfReader *reader, MovieCtrl *ctrl)
{
    if (movie_has_sound == 1) {
        if ((disp_free_count(vid->disp) == 0 && snd_free_count(vid->snd) == 0 && pmf_reader_idle(reader) != 0) ||
            ctrl_is_stop_requested(ctrl) == 1) {
            movie_evf_set(ctrl, 1);
            return 0;
        }
        return -1;
    }
    if ((disp_free_count(vid->disp) == 0 && pmf_reader_idle(reader) != 0) || ctrl_is_stop_requested(ctrl) == 1) {
        movie_evf_set(ctrl, 1);
        return 0;
    }
    return -1;
}

int ctrl_is_stop_requested(MovieCtrl *c)
{
    unsigned int st;

    movie_evf_status(c, &st);
    if ((st & 8) == 8)
        return 1;
    return -1;
}

int ctrl_is_error(MovieCtrl *c)
{
    unsigned int st;

    movie_evf_status(c, &st);
    return -(int)((st & 0x800) != 0);
}

int ctrl_abort(MovieCtrl *c)
{
    movie_evf_set(c, 0xFF);
    return 0;
}

/* Decode loop of the main thread. Returns 0 at the end of the stream, 1 when skipped, <0 on error. */
#ifdef NON_MATCHING /* structure matches; register allocation and scheduling differ */
int movie_main_loop(void *mpeg, void *rb, int packets, MovieAu *au, PmfReader *reader, MovieVid *vid, MovieCtrl *ctrl)
{
    int r, res;
    int init = 1;
    int ended = -1;

    while (pmf_reader_idle(reader) != 1) { /* wait for the ring buffer to fill */
        res = ctrl_is_error(ctrl);
        r = res;
        if (res != 0)
            goto fail;
        ctrl_wait_vblank2(ctrl);
        sceKernelDelayThreadCB(1000);
    }
    for (;;) {
        if (ended != 0) {
            ended = movie_check_end(vid, reader, ctrl);
        } else {
            r = movie_poll_input(vid->disp);
            if (r != 0) {
                sceMpeg_4571CC64(mpeg);
                ctrl_abort(ctrl);
                return r;
            }
        }
        if (pmf_reader_is_paused(reader) == 1 && packets << 2 == sceMpegRingbufferAvailableSize(rb))
            break;
        res = ctrl_is_error(ctrl);
        r = res;
        if (res != 0)
            goto fail;
        if (movie_has_sound == 1) {
            res = movie_decode_atrac(mpeg, au, vid, init);
            r = res;
            if (res < 0) {
                if (r != 0x80618001)
                    goto fail;
            } else {
                init = 0;
            }
        }
        res = movie_decode_avc(mpeg, au, vid);
        r = res;
        if (res != 0x80618001 && r < 0)
            goto fail;
        ctrl_wait_vblank2(ctrl);
        sceKernelCheckCallback();
        sceKernelPowerTick(6);
    }
    movie_evf_set(ctrl, 8);
    do {
        res = movie_decode_avc_stop(mpeg, vid);
        r = res;
        if (res < 0)
            goto fail;
        ctrl_wait_vblank2(ctrl);
    } while (r != 0);
    ctrl_abort(ctrl);
    return r;
fail:
    ctrl_abort(ctrl);
    return r;
}
#endif

/* Reads the first 2 KB to get the stream offset and size, then rewinds. */
int movie_read_header(void *mpeg, MovieFile *file, int *size, int *offset)
{
    unsigned char buf[0x800];
    int r;

    if (movie_seek(file, 0, 0) < 0)
        r = -1;
    else if (movie_read_async(file, buf, 0x800) < 0x800)
        r = -1;
    else if (sceMpegQueryStreamOffset(mpeg, buf, offset) != 0)
        r = -1;
    else if (sceMpegQueryStreamSize(buf, size) != 0)
        r = -1;
    else if (movie_seek(file, 0, 0) < 0)
        r = -1;
    else
        r = 0;
    return r;
}

/* 1 when the user skipped the movie (movie_result = 10). */
#ifdef NON_MATCHING /* structure matches; register allocation and scheduling differ */
int movie_poll_input(void)
{
    unsigned int pad[4];
    unsigned int pressed;

    pad_prev = pad_cur;
    sceCtrlReadBufferPositive(pad, 1);
    pad_cur = pad[1];
    if (movie_flags & 4)
        return 0;
    if ((movie_flags & 2) && (pad_cur & ~pad_prev) != 0) {
        movie_result = 10;
        return 1;
    }
    pressed = pad_cur & ~pad_prev;
    if (pressed & 1) {
        movie_result = 10;
        return 1;
    }
    if (pressed & 8) {
        movie_result = 10;
        return 1;
    }
    return 0;
}
#endif

int msnd_init(MovieSnd *s, int bufsize, int nbufs, MovieCtrl *ctrl, unsigned int *mem, int mute)
{
    int r;
    int i;

    s->bufs = 0;
    s->pts = 0;
    s->bufsize = bufsize;
    s->nbufs = nbufs;
    s->ctrl = ctrl;
    snd_out[0] = 0;
    snd_out[1] = 0;
    r = 0;
    if (mute == 0 && sceWaveInit_wrap() != 0) {
        r = -1;
        dbg_printf("fatal error : sceWaveInit()\n");
    } else if (wave_set_sample(0, 0x800) != 0) {
        r = -1;
    } else {
        s->sema = sceKernelCreateSema("sound_lockSema", 0, 1, 1, 0);
        s->bufs = (void **)*mem;
        *mem = (*mem + s->nbufs * 4 + 0x3F) & ~0x3F;
        for (i = 0; i < s->nbufs; i++)
            s->bufs[i] = 0;
        for (i = 0; i < s->nbufs; i++) {
            s->bufs[i] = (void *)*mem;
            *mem = (*mem + s->bufsize + 0x3F) & ~0x3F;
        }
        s->pts = (int *)*mem;
        *mem = (*mem + s->nbufs * 4 + 0x3F) & ~0x3F;
        snd_out[0] = *mem;
        *mem = (*mem + s->bufsize + 0x3F) & ~0x3F;
        snd_out[1] = *mem;
        *mem = (*mem + s->bufsize + 0x3F) & ~0x3F;
        snd_pts_step = 0x1053;
        goto done;
    }
    if (s->bufs != 0) {
        for (i = 0; i < s->nbufs; i++) {
            if (s->bufs[i] != 0)
                s->bufs[i] = 0;
        }
        s->bufs = 0;
    }
    if (s->pts != 0)
        s->pts = 0;
    for (i = 0; i < 2; i++) {
        if (snd_out[i] != 0)
            snd_out[i] = 0;
    }
done:
    return r;
}

int snd_reset(MovieSnd *s)
{
    snd_play_count = 0;
    snd_delay_count = 0;
    s->wr = 0;
    s->rd = 0;
    s->count = 0;
    return 0;
}

int snd_term(MovieSnd *s, int mute)
{
    int i;

    if (mute == 0)
        wave_exit();
    sceKernelDeleteSema(s->sema);
    if (s->bufs != 0) {
        for (i = 0; i < s->nbufs; i++) {
            if (s->bufs[i] != 0)
                s->bufs[i] = 0;
        }
        s->bufs = 0;
    }
    if (s->pts != 0)
        s->pts = 0;
    for (i = 0; i < 2; i++) {
        if (snd_out[i] != 0)
            snd_out[i] = 0;
    }
    return 0;
}

void snd_get_write_buf(MovieSnd *s, void **out)
{
    if (snd_free_count(s) > 0)
        *out = s->bufs[s->wr];
    else
        *out = 0;
}

int snd_push_pts(MovieSnd *s, int pts)
{
    if (pts == -1) {
        s->last_pts += (unsigned int)snd_pts_step;
        pts = s->last_pts;
    } else {
        s->last_pts = pts;
    }
    s->pts[s->wr] = pts;
    return 0;
}

int snd_peek_pts(MovieSnd *s, int *out)
{
    *out = s->pts[s->rd];
    return 0;
}

int snd_commit(MovieSnd *s)
{
    sceKernelWaitSema(s->sema, 1, 0);
    s->count++;
    sceKernelSignalSema(s->sema, 1);
    s->wr = (s->wr + 1) % s->nbufs;
    return 0;
}

int snd_free_count(MovieSnd *s)
{
    return s->nbufs - s->count;
}

int snd_count(MovieSnd *s)
{
    return s->count;
}

int snd_is_stop_requested(MovieSnd *s)
{
    unsigned int st;

    movie_evf_status(s->ctrl, &st);
    return (st & 8) == 8;
}

/* soundThread: feeds decoded Atrac frames to the wave output. */
int snd_thread(SceSize args, void *argp)
{
    MovieSnd *s = (*(MovieVid **)argp)->snd;

    ctrl_wait_start_sound(s->ctrl);
    for (;;) {
        if (snd_is_stop_requested(s) == 1 && snd_count(s) == 0)
            break;
        if (snd_count(s) > 0) {
            snd_play_next(s);
            sceKernelWaitSema(s->sema, 1, 0);
            s->count--;
            sceKernelSignalSema(s->sema, 1);
            snd_play_count++;
        } else {
            snd_delay_count++;
            sceKernelDelayThread(1000);
        }
    }
    while (wave_rest_samples(0) != 0)
        sceKernelDelayThread(1000);
    dbg_printf("sound count = %d\n", snd_play_count);
    dbg_printf("sound delay = %d\n", snd_delay_count);
    movie_evf_set(s->ctrl, 0x20);
    sceKernelExitThread(0);
    return 0;
}

#ifdef NON_MATCHING /* structure matches; register allocation and scheduling differ */
int snd_output(MovieSnd *s, void *buf)
{
    memcpy((void *)snd_out[snd_out_idx], buf, s->bufsize);
    s->rd = (s->rd + 1) % s->nbufs;
    wave_audio_write(0, snd_volume, snd_volume, (void *)snd_out[snd_out_idx]);
    snd_out_idx ^= 1;
    return 0;
}
#endif

int snd_play_next(MovieSnd *s)
{
    snd_output(s, s->bufs[s->rd]);
    return 0;
}

void msnd_set_volume(int v)
{
    snd_volume = v;
}

char movie_dev_host0[] = "host0:";
char movie_dev_disc0[] = "disc0:";
char movie_msg_umd_activate[] = "fatal error : sceUmdActivate() 0x%x\n";
char movie_dev_ms0[] = "ms0:";
char movie_msg_bad_device[] = "fatal error : non-supported device\n";

/* Records the device prefix of path; activates the UMD for disc0:. */
#ifdef NON_MATCHING /* structure matches; register allocation and scheduling differ */
int movie_file_init(MovieFile *f, const char *path)
{
    int r;

    if (memcmp2(path, movie_dev_host0, 6) == 0) {
        strcpy(f->dev, movie_dev_host0);
        r = 0;
    } else if (memcmp2(path, movie_dev_disc0, 6) == 0) {
        strcpy(f->dev, movie_dev_disc0);
        r = sceUmdActivate(1, movie_dev_disc0);
        if (r < 0) {
            dbg_printf(movie_msg_umd_activate, r);
            return r;
        }
        umd_retry_init();
        r = 0;
    } else if (memcmp2(path, movie_dev_ms0, 4) == 0) {
        strcpy(f->dev, movie_dev_ms0);
        r = 0;
    } else {
        dbg_printf(movie_msg_bad_device);
        r = -1;
        f->fd = r;
    }
    return r;
}
#endif

int movie_file_open(MovieFile *f, const char *path, int flags)
{
    int r = 0;
    int fd, ret;
    long long res;

    if (memcmp2(path, movie_dev_host0, 6) == 0 || memcmp2(path, movie_dev_ms0, 4) == 0) {
        fd = sceIoOpenAsync(path, flags, 0);
        if (fd < 0) {
            f->fd = fd;
        } else {
            ret = sceIoWaitAsyncCB(fd, &res);
            fd = (ret == 0) ? (int)res : ret;
            f->fd = fd;
        }
        if (fd < 0) {
            dbg_printf("fatal error : sceIoOpen() 0x%x\n", fd);
            r = f->fd;
        }
    } else if (memcmp2(path, movie_dev_disc0, 6) == 0) {
        fd = DvdUmdRetryOpenCB(path, flags, 0);
        f->fd = fd;
        if (fd < 0) {
            dbg_printf("fatal error : sceIoOpen() 0x%x\n", fd);
            r = f->fd;
        }
    } else {
        dbg_printf(movie_msg_bad_device);
        r = -1;
        f->fd = r;
    }
    return r;
}

int movie_file_close(MovieFile *f)
{
    if (f->fd >= 0)
        return sceIoClose(f->fd);
    return -1;
}

int movie_file_term(MovieFile *f)
{
    if (strcmp(f->dev, movie_dev_disc0) == 0)
        umd_retry_term();
    return 0;
}

int movie_read(MovieFile *f, void *buf, int size)
{
    if (strcmp(f->dev, movie_dev_disc0) == 0)
        return DvdUmdRetryRead(f->fd, buf, size);
    return sceIoRead(f->fd, buf, size);
}

int movie_read_async(MovieFile *f, void *buf, int size)
{
    int r;
    SceUID fd;
    long long res;

    if (strcmp(f->dev, movie_dev_disc0) == 0)
        return DvdUmdRetryReadCB(f->fd, buf, size);
    fd = f->fd;
    r = sceIoReadAsync(fd, buf, size);
    if (r < 0)
        return r;
    r = sceIoWaitAsyncCB(fd, &res);
    if (r == 0)
        return (int)res;
    return r;
}

long long movie_seek(MovieFile *f, long long offset, int whence)
{
    return sceIoLseek(f->fd, offset, whence);
}

int movie_umd_term(void)
{
    umd_retry_term();
    return 0;
}

#ifdef NON_MATCHING /* structure matches; register allocation and scheduling differ */
int vid_init(MovieVid *v, MovieDisp *disp, MovieSnd *snd, int n, unsigned int *mem)
{
    v->disp = disp;
    v->snd = snd;
    v->pts = 0;
    v->pts = (int *)*mem;
    *mem = (*mem + n * 4 + 0x3F) & ~0x3F;
    v->n = n;
    vid_pts_step = (int)((90000.0f / sceDisplayGetFramePerSec()) * 2.0f);
    return 0;
}
#endif

int vid_reset(MovieVid *v)
{
    v->last_pts = 0;
    vid_zero(v);
    return 0;
}

int vid_term(MovieVid *v)
{
    if (v->pts != 0)
        v->pts = 0;
    return 0;
}

int vid_zero(MovieVid *v)
{
    v->wr = 0;
    v->rd = 0;
    v->count = 0;
    return 0;
}

int vid_push_pts(MovieVid *v, int pts)
{
    if (pts == -1) {
        v->last_pts += (unsigned int)vid_pts_step;
        pts = v->last_pts;
    } else {
        v->last_pts = pts;
    }
    v->pts[v->wr] = pts;
    v->count++;
    v->wr = (v->wr + 1) % v->n;
    return 0;
}

int vid_pop_pts(MovieVid *v, int *out)
{
    *out = v->pts[v->rd];
    v->count--;
    v->rd = (v->rd + 1) % v->n;
    return 0;
}

/* A/V sync: 1 in sync, 2 video ahead of the sound (hold), 4 video behind (drop). */
int vid_sync_state(MovieVid *v)
{
    int spts;
    int vpts;
    int d;

    if (v->snd == 0)
        return 1;
    if (snd_count(v->snd) == 0 || disp_count(v->disp) == 0)
        return 1;
    snd_peek_pts(v->snd, &spts);
    disp_peek_pts(v->disp, &vpts);
    d = spts - vpts;
    if (d >= -vid_pts_step * 2 && d <= vid_pts_step * 2)
        return 1;
    if (vid_pts_step * 2 < d)
        return 2;
    return 4;
}

int ctrl_vblank_handler(int sub, MovieCtrl *c)
{
    return sceKernelSetEventFlag(c->wait_flag, 3);
}

int ctrl_init(MovieCtrl *c)
{
    c->control_flag = sceKernelCreateEventFlag("controlFlag", 0x200, 0, 0);
    c->wait_flag = sceKernelCreateEventFlag("waitFlag", 0x200, 0, 0);
    vblank_handler_set(0, ctrl_vblank_handler, c);
    return 0;
}

int ctrl_term(MovieCtrl *c)
{
    vblank_handler_set(0, 0, 0);
    sceKernelDeleteEventFlag(c->control_flag);
    sceKernelDeleteEventFlag(c->wait_flag);
    return 0;
}

int ctrl_clear(MovieCtrl *c)
{
    sceKernelClearEventFlag(c->control_flag, 0);
    sceKernelClearEventFlag(c->wait_flag, 0);
    return 0;
}

int ctrl_wait_vblank1(MovieCtrl *c)
{
    sceKernelWaitEventFlag(c->wait_flag, 1, 0x21, 0, 0);
    return 0;
}

int ctrl_wait_vblank2(MovieCtrl *c)
{
    sceKernelWaitEventFlag(c->wait_flag, 2, 0x21, 0, 0);
    return 0;
}

int movie_evf_set(MovieCtrl *c, unsigned int bits)
{
    return sceKernelSetEventFlag(c->control_flag, bits);
}

int movie_evf_status(MovieCtrl *c, unsigned int *out)
{
    unsigned int info[13]; /* SceKernelEventFlagInfo: size, name[32], attr, init, current, waiting */

    info[0] = 0x34;
    sceKernelReferEventFlagStatus(c->control_flag, info);
    *out = info[11];
    return 0;
}

int ctrl_wait_start_display(MovieCtrl *c)
{
    sceKernelWaitEventFlag(c->control_flag, 1, 0, 0, 0);
    return 0;
}

int ctrl_wait_start_sound(MovieCtrl *c)
{
    sceKernelWaitEventFlag(c->control_flag, 2, 0, 0, 0);
    return 0;
}

int au_init(MovieAu *a, void *avc_stream, void *avc_au, void *atrac_stream, void *atrac_au)
{
    a->avc_stream = avc_stream;
    a->atrac_stream = atrac_stream;
    a->avc_au = avc_au;
    a->atrac_au = atrac_au;
    return 0;
}

int movie_decode_atrac(void *mpeg, MovieAu *au, MovieVid *vid, int init)
{
    int ret;
    int attr;
    void *buf;

    if (snd_free_count(vid->snd) == 0) {
        ret = 1;
    } else {
        if ((ret = sceMpegGetAtracAu(mpeg, au->atrac_stream, au->atrac_au, &attr)) == 0) {
            snd_get_write_buf(vid->snd, &buf);
            if ((ret = sceMpegAtracDecode(mpeg, au->atrac_au, buf, init)) != 0) {
                dbg_printf("fatal error : sceMpegAtracDecode() 0x%x\n", ret);
            } else {
                snd_push_pts(vid->snd, ((int *)au->atrac_au)[1]);
                if (init == 0)
                    snd_commit(vid->snd);
                ret = 0;
            }
        }
    }
    return ret;
}

int movie_decode_avc(void *mpeg, MovieAu *au, MovieVid *vid)
{
    int ret;
    void *frame;
    int pts;
    int got;
    int attr;

    if (disp_free_count(vid->disp) == 0) {
        ret = 1;
    } else {
        if ((ret = sceMpegGetAvcAu(mpeg, au->avc_stream, au->avc_au, &attr)) == 0) {
            disp_get_write_frame(vid->disp, &frame);
            avc_frame_ptr = frame;
            if ((ret = sceMpegAvcDecode(mpeg, au->avc_au, 0x200, &avc_frame_ptr, &got)) != 0) {
                dbg_printf("fatal error : sceMpegAvcDecode() 0x%x\n", ret);
            } else {
                vid_push_pts(vid, ((int *)au->avc_au)[1]);
                if (got == 1) {
                    vid_pop_pts(vid, &pts);
                    disp_set_pts(vid->disp, pts);
                    disp_commit(vid->disp);
                }
                ret = 0;
            }
        }
    }
    return ret;
}

int movie_decode_avc_stop(void *mpeg, MovieVid *vid)
{
    int ret;
    void *frame;
    int pts;
    int got;

    if (disp_free_count(vid->disp) == 0) {
        ret = 1;
    } else {
        disp_get_write_frame(vid->disp, &frame);
        avc_frame_ptr = frame;
        if ((ret = sceMpegAvcDecodeStop(mpeg, 0x200, &avc_frame_ptr, &got)) != 0) {
            dbg_printf("fatal error : sceMpegGetAvcDecodeStop() 0x%x\n", ret);
        } else {
            if (got >= 1) {
                vid_pop_pts(vid, &pts);
                disp_set_pts(vid->disp, pts);
                disp_commit(vid->disp);
            }
            ret = 0;
        }
    }
    return ret;
}

int disp_init(MovieDisp *d, int framesize, int nframes, MovieCtrl *ctrl, unsigned int *mem)
{
    int r, i;
    unsigned int vram;

    d->frames = 0;
    d->pts = 0;
    d->nframes = nframes;
    d->ctrl = ctrl;
    r = sceDisplaySetMode(0, 480, 272);
    if (r >= 0) {
        d->sema = sceKernelCreateSema("display_lockSema", 0, 1, 1, 0);
        d->delay_flag = sceKernelCreateEventFlag("display_delay_flag", 0x200, 0, 0);
        d->frames = (void **)*mem;
        *mem = (*mem + d->nframes * 4 + 0x3F) & ~0x3F;
        for (i = 0; i < d->nframes; i++)
            d->frames[i] = 0;
        for (i = 0; i < d->nframes; i++) {
            d->frames[i] = (void *)*mem;
            *mem = (*mem + framesize + 0x3F) & ~0x3F;
        }
        d->pts = (int *)*mem;
        *mem = (*mem + d->nframes * 4 + 0x3F) & ~0x3F;
        vram = (unsigned int)sceGeEdramGetAddr();
        disp_vram_back = (void *)(vram + 0x88000);
        disp_vram_front = (void *)(vram + 0x110000);
    } else {
        if (d->frames != 0) {
            for (i = 0; i < d->nframes; i++) {
                if (d->frames[i] != 0)
                    d->frames[i] = 0;
            }
            d->frames = 0;
        }
        if (d->pts != 0)
            d->pts = 0;
    }
    return r;
}

int disp_present(void *frame)
{
    void *t;

    sceDmacMemcpy(disp_vram_back, frame, 0x88000);
    sceDisplaySetFrameBuf(disp_vram_back, 0x200, 3, 1);
    t = disp_vram_back;
    disp_vram_back = disp_vram_front;
    disp_vram_front = t;
    return 0;
}

int disp_reset(MovieDisp *d)
{
    int r;

    disp_frame_count = 0;
    disp_frame_loss = 0;
    d->wr = 0;
    d->rd = 0;
    d->count = 0;
    disp_set_state(d, 1);
    sceKernelClearEventFlag(d->delay_flag, 0);
    r = sceDisplaySetMode(0, 480, 272);
    if (r < 0)
        return r;
    sceDisplaySetFrameBuf(0, 0x200, 3, 1);
    return 0;
}

int disp_term(MovieDisp *d)
{
    int i;

    if (d->frames != 0) {
        for (i = 0; i < d->nframes; i++) {
            if (d->frames[i] != 0)
                d->frames[i] = 0;
        }
        d->frames = 0;
    }
    if (d->pts != 0)
        d->pts = 0;
    sceKernelDeleteSema(d->sema);
    sceKernelDeleteEventFlag(d->delay_flag);
    return 0;
}

void disp_get_write_frame(MovieDisp *d, void **out)
{
    if (disp_free_count(d) > 0)
        *out = d->frames[d->wr];
    else
        *out = 0;
}

int disp_set_pts(MovieDisp *d, int pts)
{
    d->pts[d->wr] = pts;
    return 0;
}

int disp_peek_pts(MovieDisp *d, int *out)
{
    *out = d->pts[d->rd];
    return 0;
}

int disp_commit(MovieDisp *d)
{
    sceKernelWaitSema(d->sema, 1, 0);
    d->count++;
    d->wr = (d->wr + 1) % d->nframes;
    sceKernelSignalSema(d->sema, 1);
    return 0;
}

int disp_free_count(MovieDisp *d)
{
    return d->nframes - d->count;
}

int disp_count(MovieDisp *d)
{
    return d->count;
}

int disp_check_stop(MovieDisp *d)
{
    unsigned int st;
    int r;

    movie_evf_status(d->ctrl, &st);
    if ((st & 8) == 8) {
        disp_set_state(d, 0xFF);
        return 1;
    }
    return 0;
}

int disp_set_state(MovieDisp *d, int state)
{
    d->state = state;
    return 0;
}

int disp_get_state(MovieDisp *d)
{
    return d->state;
}

void disp_clear_delay_flag(MovieDisp *d)
{
    sceKernelClearEventFlag(d->delay_flag, 0);
}

int disp_delay_requested(MovieDisp *d)
{
    unsigned int info[13];
    int r = 0;

    info[0] = 0x34;
    sceKernelReferEventFlagStatus(d->delay_flag, info);
    if (info[11] == 1)
        r = 1;
    return r;
}

/* displayThread: shows one decoded frame per vblank, dropping or holding to follow the sound. */
int disp_thread(SceSize args, void *argp)
{
    MovieVid *vid = *(MovieVid **)argp;
    MovieDisp *d = vid->disp;

    ctrl_wait_start_display(d->ctrl);
    for (;;) {
        if (disp_check_stop(d) == 1 && disp_count(d) == 0)
            break;
        if (disp_delay_requested(d) == 1) {
            sceKernelDelayThread(1000000);
            disp_clear_delay_flag(d);
        }
        disp_set_state(d, vid_sync_state(vid));
        if (disp_count(d) > 0) {
            if (disp_get_state(d) != 4) {
                disp_show_next(d);
                sceKernelWaitSema(d->sema, 1, 0);
                d->count--;
                sceKernelSignalSema(d->sema, 1);
                disp_frame_count++;
            }
        } else {
            disp_frame_loss++;
        }
        movie_evf_set(d->ctrl, 2);
        ctrl_wait_vblank1(d->ctrl);
    }
    sceDisplaySetFrameBuf(0, 0x200, 3, 1);
    dbg_printf("frame count = %d\n", disp_frame_count);
    dbg_printf("frame loss  = %d\n", disp_frame_loss);
    movie_evf_set(d->ctrl, 0x10);
    sceKernelExitThread(0);
    return 0;
}

int disp_show_next(MovieDisp *d)
{
    if (disp_get_state(d) == 1 || disp_get_state(d) == 0xFF) {
        disp_present(d->frames[d->rd]);
        d->rd = (d->rd + 1) % d->nframes;
        ctrl_wait_vblank1(d->ctrl);
    } else {
        d->rd = (d->rd + 1) % d->nframes;
    }
    return 0;
}
