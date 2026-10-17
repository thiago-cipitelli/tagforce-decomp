// @range 0x08850A70-0x0885133C
// @data 0x088EEF50-0x088EF098
// @rodata 0x088AEBB4-0x088AEBEC
/* Sony sample code kept by the game: the PMF ring-buffer feeder thread (sceMpegRingbufferPut from
   the read thread) and the "DvdUmdRetry*" wrappers that retry sceIo calls while the UMD is not
   ready. Also the codec module load/unload pairs used around a movie. */
#include "psp.h"

typedef struct PmfReader {    /* ring-buffer feeder state shared with the movie module */
    int total;                /* +0 bytes to feed (file size) */
    int state;                /* +4 0 idle, 1 running, 2 error, 3 done */
    void *rb;                 /* +8 SceMpegRingbuffer */
    void *evf;                /* +C owner object with the event flag */
} PmfReader;

int dbg_printf(const char *fmt, ...);
int movie_evf_status(void *obj, unsigned int *out);
void movie_evf_set(void *obj, int bits);
int movie_read(void *file, void *buf, int size);
int modmgr_load_by_id(int id);
int modmgr_update(int id);

int pmf_reader_attach(PmfReader *s, void *rb, void *evf)
{
    s->rb = rb;
    s->evf = evf;
    return 0;
}

int pmf_reader_start(PmfReader *s, int total)
{
    s->state = 1;
    s->total = total;
    return 0;
}

int pmf_reader_noop(void)
{
    return 0;
}

int pmf_reader_get_state(PmfReader *s)
{
    return s->state;
}

int pmf_reader_set_state(PmfReader *s, int state)
{
    s->state = state;
    return 0;
}

int pmf_reader_is_paused(PmfReader *s)
{
    unsigned int st;

    movie_evf_status(s->evf, &st);
    return (st & 4) == 4;
}

int pmf_reader_idle(PmfReader *s)
{
    unsigned int st;
    int n;

    movie_evf_status(s->evf, &st);
    if (pmf_reader_is_paused(s) == 1)
        return 1;
    n = pmf_reader_available(s);
    return n == 0;
}

int pmf_reader_available(PmfReader *s)
{
    return sceMpegRingbufferAvailableSize(s->rb);
}

int pmf_reader_signal_error(PmfReader *s)
{
    movie_evf_set(s->evf, 0xC00);
    return 0;
}

int pmf_reader_signal_done(PmfReader *s)
{
    movie_evf_set(s->evf, 4);
    return 0;
}

int pmf_reader_check_abort(PmfReader *s)
{
    unsigned int st;

    movie_evf_status(s->evf, &st);
    if ((unsigned char)st == 0xFF)
        pmf_reader_set_state(s, 3);
    return 0;
}

/* readThread: keeps the ring buffer full until the whole file has been fed. */
int pmf_reader_thread(SceSize args, void *argp)
{
    PmfReader *s = *(PmfReader **)argp;
    int total = 0;
    int avail, n;

    for (;;) {
        pmf_reader_check_abort(s);
        if (pmf_reader_get_state(s) == 3)
            break;
        avail = sceMpegRingbufferAvailableSize(s->rb);
        if (pmf_reader_get_state(s) == 1) {
            n = sceMpegRingbufferPut(s->rb, 32, avail);
            if (n < 0) {
                dbg_printf("fatal error: sceMpegRingbufferPut 0x%x total = %d\n", n, total);
                pmf_reader_set_state(s, 2);
                pmf_reader_signal_error(s);
                n = 0;
            }
            total += n << 11;
        }
        if (total == s->total && pmf_reader_get_state(s) == 1) {
            pmf_reader_set_state(s, 3);
            pmf_reader_signal_done(s);
            break;
        }
        sceKernelDelayThread(100);
    }
    sceKernelExitThread(0);
    return 0;
}

SceUID umd_io_sema = -1;

int umd_retry_init(void)
{
    SceUID sema;

    if (umd_io_sema >= 0)
        return 0;
    sema = sceKernelCreateSema("UMD_SAMPLE_IO_SEMA", 0, 1, 1, 0);
    if (sema < 0)
        return sema;
    umd_io_sema = sema;
    return 0;
}

int umd_retry_term(void)
{
    SceUID sema;

    if (umd_io_sema >= 0) {
        sceUmdCancelWaitDriveStat();
        sema = umd_io_sema;
        umd_io_sema = -1;
        dbg_printf("POLL SEMA\n");
        sceKernelDeleteSema(sema);
    }
    return 0;
}

int DvdUmdRetryOpenCB(const char *path, int flags, int mode)
{
    int ret;

    for (;;) {
        if ((sceUmdGetDriveStat() & 0x20) == 0) {
            dbg_printf("WAITTING: %s(%d)\n", __FUNCTION__, 401);
            ret = sceUmdWaitDriveStatCB(0x20, 0);
            if (ret < 0)
                break;
        }
        if ((ret = sceKernelWaitSemaCB(umd_io_sema, 1, 0)) < 0)
            break;
        ret = sceIoOpen(path, flags, mode);
        sceKernelSignalSema(umd_io_sema, 1);
        if (ret >= 0)
            break;
        if (ret == 0x8001B002 || ret == 0x80010013 || ret == 0x8001000C || ret == 0x80010016 || ret == 0x8001005B)
            dbg_printf("ERROR: CRITICAL ERROR [0x%x] !!\n", ret);
    }
    return ret;
}

int DvdUmdRetryRead(SceUID fd, void *buf, SceSize size)
{
    int ret, result;

    for (;;) {
        if ((sceUmdGetDriveStat() & 0x20) == 0) {
            dbg_printf("WAITTING: %s(%d)\n", __FUNCTION__, 459);
            ret = sceUmdWaitDriveStat(0x20);
            if (ret < 0)
                break;
        }
        result = sceKernelWaitSema(umd_io_sema, 1, 0);
        ret = result;
        if (result < 0) {
            dbg_printf("CANCEL WAIT: %s(%x)\n", __FUNCTION__, result);
            break;
        }
        ret = sceIoRead(fd, buf, size);
        sceKernelSignalSema(umd_io_sema, 1);
        if (ret >= 0)
            break;
        if (ret == 0x80010016 || ret == 0x80010013 || ret == 0x8001B003 || ret == 0x8001000E || ret == 0x80210002 ||
            ret == 0x8001B001 || ret == 0x8001005B)
            dbg_printf("ERROR: CRITICAL ERROR [0x%x] !!\n", ret);
        dbg_printf("*** %s(0x%08x) *** \n", __FUNCTION__, ret);
        if (ret == 0x80010009)
            dbg_printf("WARNING: INVALID DISC ACCESS !!\n");
    }
    return ret;
}

int DvdUmdRetryReadCB(SceUID fd, void *buf, SceSize size)
{
    int ret, result;
    long long res;

    for (;;) {
        if ((sceUmdGetDriveStat() & 0x20) == 0) {
            dbg_printf("WAITTING: %s(%d)\n", __FUNCTION__, 525);
            ret = sceUmdWaitDriveStatCB(0x20, 0);
            if (ret < 0) {
                dbg_printf("CANCEL WAIT: %s(%x)\n", __FUNCTION__, ret);
                break;
            }
        }
        result = sceKernelWaitSemaCB(umd_io_sema, 1, 0);
        ret = result;
        if (result < 0) {
            dbg_printf("CANCEL SEMA: %s(%x)\n", __FUNCTION__, result);
            break;
        }
        result = sceIoReadAsync(fd, buf, size);
        if (result < 0) {
            ret = result;
        } else {
            result = sceIoWaitAsyncCB(fd, &res);
            if (result == 0)
                ret = (int)res;
            else
                ret = result;
        }
        sceKernelSignalSema(umd_io_sema, 1);
        if (ret >= 0)
            break;
        if (ret == 0x80010016 || ret == 0x80010013 || ret == 0x8001B003 || ret == 0x8001000E || ret == 0x80210002 ||
            ret == 0x8001B001 || ret == 0x8001005B)
            dbg_printf("ERROR: CRITICAL ERROR [0x0x%x] !!\n", ret);
        dbg_printf("*** %s(0x%08x) *** \n", __FUNCTION__, ret);
        if (ret == 0x80010009)
            dbg_printf("WARNING: INVALID FILE DESCRIPTOR !!\n");
    }
    return ret;
}

/* Reads whole 2 KB blocks; returns the block count, or -1 when a partial block came back. */
int pmf_read_blocks(void *buf, int blocks, void *file)
{
    int n = movie_read(file, buf, blocks << 11);

    if (n < 0)
        return n;
    if (n % 0x800 != 0)
        return -1;
    return n / 0x800;
}

int duel_modules_load(void)
{
    modmgr_load_by_id(2);
    modmgr_load_by_id(3);
    modmgr_load_by_id(4);
    modmgr_load_by_id(5);
    return 0;
}

int modmgr_none_loading(void);

int modmgr_none_loading_alias(void)
{
    return modmgr_none_loading();
}

int duel_modules_unload(void)
{
    modmgr_update(5);
    modmgr_update(4);
    modmgr_update(3);
    modmgr_update(2);
    return 0;
}

extern int cardview_a, cardview_b;

void cardview_set_ab(int a, int b)
{
    cardview_a = a;
    cardview_b = b;
}
