/* ThreadManForUser and friends on pthreads (port step 4b, docs/port_survey.md).

   The PSP is single-core with a strict priority scheduler, and the game relies on it (ehscript_yield_to: wake the
   main thread, which preempts the scene thread and suspends it). Model: one global lock is the CPU. A thread runs
   only while it holds the lock; it gives the lock up only in a blocking call, and it returns from one only when it
   is the highest-priority ready, not suspended thread (lowest number). A call that readies a higher-priority thread
   (signal, set, wakeup, start, resume) yields at once, as the PSP preempts.
   ponytail: global lock + one condition variable broadcast on every change (O(threads) per wakeup, fine for ~10
   threads); timers do not preempt a running thread, a delay that expired is seen at the next scheduling point. */
#define _GNU_SOURCE
#include <malloc.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define ERR_TIMEOUT 0x800201A8
#define ERR_NOMSG 0x800201B2
#define ERR_UID 0x800200CB

void *__libc_malloc(size_t n);
void *__libc_memalign(size_t align, size_t n);
void __libc_free(void *p);
int dprintf(int fd, const char *fmt, ...);
void host_gfx_release(void);
unsigned int host_buttons(void);

typedef int (*Entry)(unsigned int args, void *argp);

typedef struct Thr Thr;
struct Thr {
    int used, prio, suspended, done, status, wakeups;
    int (*pred)(Thr *);              /* waiting: the condition that readies it; 0 = running or ready */
    long long deadline;              /* waiting: give up (or, for a delay, be ready) at this time; 0 = never */
    void *obj;
    unsigned int bits;
    int mode;
    Entry entry;
    unsigned int args;
    void *argp;
    pthread_t pt;
    char name[32];
};

typedef struct { int used, count, max; } Sema;
typedef struct { int used; unsigned int bits, init; char name[32]; } Evf;
typedef struct { int used; void **head, **tail; } Mbx;
typedef struct { int used; unsigned int size; void *ptr; } Block;   /* Fpl, Vpl, partition memory */

#define NTHR 64
#define NOBJ 256
static Thr thr[NTHR];
static Sema sema[NOBJ];
static Evf evf[NOBJ];
static Mbx mbx[NOBJ];
static Block blk[NOBJ];
/* uids: kind * 0x1000 + index (index 0 unused) */
enum { K_THR = 1, K_SEMA, K_EVF, K_MBX, K_BLK, K_CB };

static pthread_mutex_t cpu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static __thread Thr *self;
static long long t0;
static int trace_sched;   /* env TAGFORCE_TRACE_SCHED: log every switch */

long long host_now_us(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

static void *obj(int uid, int kind, void *base, int size)
{
    int i = uid & 0xFFF;

    if (uid >> 12 != kind || i == 0 || i >= (kind == K_THR ? NTHR : NOBJ) || !*(int *)((char *)base + i * size))
        return 0;
    return (char *)base + i * size;
}
#define OBJ(uid, kind, arr) ((__typeof__(&arr[0]))obj(uid, kind, arr, sizeof arr[0]))

static int alloc(int kind, void *base, int size, int n)
{
    int i;

    for (i = 1; i < n; i++) {
        int *u = (int *)((char *)base + i * size);
        if (!*u) {
            memset(u, 0, size);
            *u = 1;
            return kind << 12 | i;
        }
    }
    dprintf(2, "kernel: out of objects (kind %d)\n", kind);
    abort();
}
#define ALLOC(kind, arr) alloc(kind, arr, sizeof arr[0], sizeof arr / sizeof arr[0])

static int ready(Thr *t)
{
    if (!t->used || t->done || t->suspended)
        return 0;
    if (!t->pred)
        return 1;
    return t->pred(t) || (t->deadline && host_now_us() >= t->deadline);
}

/* the thread that should hold the CPU: the highest priority ready one, the current one on ties */
static Thr *best(void)
{
    Thr *b = ready(self) ? self : 0;
    int i;

    for (i = 1; i < NTHR; i++)
        if (&thr[i] != self && ready(&thr[i]) && (!b || thr[i].prio < b->prio))
            b = &thr[i];
    return b;
}

/* blocks until pred holds (or the deadline passes) and this thread is the one to run; returns 0, or ERR_TIMEOUT */
static int block(int (*pred)(Thr *), long long deadline)
{
    Thr *me = self;
    int r;

    me->pred = pred;
    me->deadline = deadline;
    host_gfx_release();   /* another thread may draw next (window.c) */
    pthread_cond_broadcast(&cv);
    for (;;) {
        if (ready(me) && best() == me)
            break;
        if (me->deadline && !me->pred(me) && host_now_us() < me->deadline) {
            struct timespec ts;
            long long d = me->deadline - host_now_us() + 1;

            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += d / 1000000;
            ts.tv_nsec += d % 1000000 * 1000;
            if (ts.tv_nsec >= 1000000000) {
                ts.tv_sec++;
                ts.tv_nsec -= 1000000000;
            }
            pthread_cond_timedwait(&cv, &cpu, &ts);
        } else {
            pthread_cond_wait(&cv, &cpu);
        }
    }
    r = pred(me) ? 0 : (int)ERR_TIMEOUT;
    if (trace_sched)
        dprintf(2, "sched %lld: run %s\n", host_now_us() - t0, me->name);
    me->pred = 0;
    me->deadline = 0;
    return r;
}

static int p_true(Thr *t) { return 1; }
static int p_false(Thr *t) { return 0; }

/* after readying another thread: give it the CPU if it outranks this one */
static void preempt(void)
{
    pthread_cond_broadcast(&cv);
    if (best() != self)
        block(p_true, 0);
}

static long long deadline_of(unsigned int *timeout)
{
    return timeout ? host_now_us() + (*timeout ? *timeout : 1) : 0;
}

/* --- threads --- */

extern int crt0_thread_prio;   /* user_main's priority (BOOT data) */

__attribute__((constructor)) static void kernel_init(void)
{
    t0 = host_now_us();
    /* every allocation from the brk heap, below 0x80000000: the game keeps addresses in 31-bit fields (HeapEnt) */
    mallopt(M_MMAP_MAX, 0);
    trace_sched = getenv("TAGFORCE_TRACE_SCHED") != 0;
    pthread_mutex_lock(&cpu);
    thr[1].used = 1;
    thr[1].prio = crt0_thread_prio;
    strcpy(thr[1].name, "user_main");
    self = &thr[1];
}

static void thread_end(int status)
{
    self->done = 1;
    self->status = status;
    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&cpu);
    pthread_exit(0);
}

static void *thread_main(void *arg)
{
    Thr *t = arg;

    pthread_mutex_lock(&cpu);
    self = t;
    block(p_true, 0);
    thread_end(t->entry(t->args, t->argp));
    return 0;
}

int sceKernelCreateThread(const char *name, Entry entry, int prio, int stack, unsigned int attr, void *opt)
{
    int uid = ALLOC(K_THR, thr);
    Thr *t = &thr[uid & 0xFFF];

    t->prio = prio;
    t->entry = entry;
    t->done = 1;   /* dormant until started */
    strncpy(t->name, name ? name : "", sizeof t->name - 1);
    return uid;
}

int sceKernelStartThread(int uid, unsigned int args, void *argp)
{
    Thr *t = OBJ(uid, K_THR, thr);
    pthread_attr_t a;

    if (!t)
        return ERR_UID;
    t->args = args;
    t->argp = 0;
    if (argp && args) {   /* the PSP copies the arguments to the new thread's stack */
        t->argp = __libc_malloc(args);
        memcpy(t->argp, argp, args);
    }
    t->done = 0;
    pthread_attr_init(&a);
    pthread_attr_setstacksize(&a, 1 << 20);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);
    pthread_create(&t->pt, &a, thread_main, t);
    pthread_attr_destroy(&a);
    preempt();
    return 0;
}

int sceKernelExitThread(int status)
{
    thread_end(status);
    return 0;
}

int sceKernelExitDeleteThread(int status)
{
    self->used = 0;
    thread_end(status);
    return 0;
}

int sceKernelDeleteThread(int uid)
{
    Thr *t = OBJ(uid, K_THR, thr);

    if (!t)
        return ERR_UID;
    if (t->done)
        t->used = 0;
    return 0;
}

static int p_end(Thr *t) { return ((Thr *)t->obj)->done; }

int sceKernelWaitThreadEnd(int uid, unsigned int *timeout)
{
    Thr *t = OBJ(uid, K_THR, thr);

    if (!t)
        return ERR_UID;
    self->obj = t;
    return block(p_end, deadline_of(timeout)) ? (int)ERR_TIMEOUT : t->status;
}

int sceKernelWaitThreadEndCB(int uid, unsigned int *timeout) { return sceKernelWaitThreadEnd(uid, timeout); }

int sceKernelGetThreadId(void) { return K_THR << 12 | (int)(self - thr); }
int sceKernelGetThreadCurrentPriority(void) { return self->prio; }

int sceKernelChangeThreadPriority(int uid, int prio)
{
    Thr *t = uid ? OBJ(uid, K_THR, thr) : self;

    if (!t)
        return ERR_UID;
    if (prio < 0 || prio > 126)   /* movie_run passes an uninitialized priority when the movie fails to open */
        return 0x80020193;
    if (prio)
        t->prio = prio;
    preempt();
    return 0;
}

static int p_wakeup(Thr *t) { return t->wakeups > 0; }

int sceKernelSleepThread(void)
{
    block(p_wakeup, 0);
    self->wakeups--;
    return 0;
}

int sceKernelSleepThreadCB(void) { return sceKernelSleepThread(); }

int sceKernelWakeupThread(int uid)
{
    Thr *t = OBJ(uid, K_THR, thr);

    if (!t)
        return ERR_UID;
    t->wakeups++;
    preempt();
    return 0;
}

int sceKernelSuspendThread(int uid)
{
    Thr *t = OBJ(uid, K_THR, thr);

    if (!t)
        return ERR_UID;
    t->suspended = 1;
    if (t == self)
        block(p_true, 0);
    return 0;
}

int sceKernelResumeThread(int uid)
{
    Thr *t = OBJ(uid, K_THR, thr);

    if (!t)
        return ERR_UID;
    t->suspended = 0;
    preempt();
    return 0;
}

int sceKernelDelayThread(unsigned int us)
{
    block(p_false, host_now_us() + (us ? us : 1));
    return 0;
}

int sceKernelDelayThreadCB(unsigned int us) { return sceKernelDelayThread(us); }
int sceKernelRotateThreadReadyQueue(int prio) { block(p_true, 0); return 0; }
int sceKernelChangeCurrentThreadAttr(unsigned int clear, unsigned int set) { return 0; }
int sceKernelCheckCallback(void) { return 0; }

int sceKernelCreateCallback(const char *name, void *fn, void *arg)
{
    static int n;

    return K_CB << 12 | ++n;
}

/* --- semaphores --- */

int sceKernelCreateSema(const char *name, unsigned int attr, int init, int max, void *opt)
{
    int uid = ALLOC(K_SEMA, sema);

    sema[uid & 0xFFF].count = init;
    sema[uid & 0xFFF].max = max;
    return uid;
}

int sceKernelDeleteSema(int uid)
{
    Sema *s = OBJ(uid, K_SEMA, sema);

    if (!s)
        return ERR_UID;
    s->used = 0;
    pthread_cond_broadcast(&cv);
    return 0;
}

static int p_sema(Thr *t) { return ((Sema *)t->obj)->count >= t->mode; }

int sceKernelWaitSema(int uid, int n, unsigned int *timeout)
{
    Sema *s = OBJ(uid, K_SEMA, sema);

    if (!s)
        return ERR_UID;
    self->obj = s;
    self->mode = n;
    if (block(p_sema, deadline_of(timeout)))
        return ERR_TIMEOUT;
    s->count -= n;
    return 0;
}

int sceKernelWaitSemaCB(int uid, int n, unsigned int *timeout) { return sceKernelWaitSema(uid, n, timeout); }

int sceKernelPollSema(int uid, int n)
{
    Sema *s = OBJ(uid, K_SEMA, sema);

    if (!s)
        return ERR_UID;
    if (s->count < n)
        return 0x800201AD;
    s->count -= n;
    return 0;
}

int sceKernelSignalSema(int uid, int n)
{
    Sema *s = OBJ(uid, K_SEMA, sema);

    if (!s)
        return ERR_UID;
    if (s->count + n > s->max)
        return 0x8002019D;
    s->count += n;
    preempt();
    return 0;
}

/* --- event flags --- */

int sceKernelCreateEventFlag(const char *name, int attr, int bits, void *opt)
{
    int uid = ALLOC(K_EVF, evf);

    evf[uid & 0xFFF].bits = evf[uid & 0xFFF].init = bits;
    strncpy(evf[uid & 0xFFF].name, name ? name : "", 31);
    return uid;
}

int sceKernelDeleteEventFlag(int uid)
{
    Evf *e = OBJ(uid, K_EVF, evf);

    if (!e)
        return ERR_UID;
    e->used = 0;
    return 0;
}

int sceKernelSetEventFlag(int uid, unsigned int bits)
{
    Evf *e = OBJ(uid, K_EVF, evf);

    if (!e)
        return ERR_UID;
    e->bits |= bits;
    preempt();
    return 0;
}

int sceKernelClearEventFlag(int uid, unsigned int bits)
{
    Evf *e = OBJ(uid, K_EVF, evf);

    if (!e)
        return ERR_UID;
    e->bits &= bits;   /* the PSP keeps the bits set in the argument */
    return 0;
}

static int evf_match(Evf *e, unsigned int bits, int mode)
{
    return mode & 1 ? (e->bits & bits) != 0 : (e->bits & bits) == bits;
}

static int p_evf(Thr *t) { return evf_match(t->obj, t->bits, t->mode); }

static void evf_take(Evf *e, unsigned int bits, int mode, unsigned int *out)
{
    if (out)
        *out = e->bits;
    if (mode & 0x10)
        e->bits = 0;
    else if (mode & 0x20)
        e->bits &= ~bits;
}

int sceKernelWaitEventFlag(int uid, unsigned int bits, unsigned int mode, unsigned int *out, unsigned int *timeout)
{
    Evf *e = OBJ(uid, K_EVF, evf);

    if (!e)
        return ERR_UID;
    self->obj = e;
    self->bits = bits;
    self->mode = mode;
    if (block(p_evf, deadline_of(timeout))) {
        if (out)
            *out = e->bits;
        return ERR_TIMEOUT;
    }
    evf_take(e, bits, mode, out);
    return 0;
}

int sceKernelWaitEventFlagCB(int uid, unsigned int bits, unsigned int mode, unsigned int *out, unsigned int *timeout)
{
    return sceKernelWaitEventFlag(uid, bits, mode, out, timeout);
}

int sceKernelPollEventFlag(int uid, unsigned int bits, unsigned int mode, unsigned int *out)
{
    Evf *e = OBJ(uid, K_EVF, evf);

    if (!e)
        return ERR_UID;
    if (!evf_match(e, bits, mode)) {
        if (out)
            *out = e->bits;
        return 0x800201AF;
    }
    evf_take(e, bits, mode, out);
    return 0;
}

int sceKernelReferEventFlagStatus(int uid, void *status)
{
    Evf *e = OBJ(uid, K_EVF, evf);
    unsigned int *s = status;   /* SceKernelEventFlagInfo: size, name[32], attr, init, cur, waiters */

    if (!e)
        return ERR_UID;
    memcpy(s + 1, e->name, 32);
    s[9] = 0;
    s[10] = e->init;
    s[11] = e->bits;
    s[12] = 0;
    return 0;
}

/* --- message boxes: the first word of each message is the link --- */

int sceKernelCreateMbx(const char *name, unsigned int attr, void *opt) { return ALLOC(K_MBX, mbx); }

int sceKernelDeleteMbx(int uid)
{
    Mbx *m = OBJ(uid, K_MBX, mbx);

    if (!m)
        return ERR_UID;
    m->used = 0;
    return 0;
}

int sceKernelSendMbx(int uid, void *msg)
{
    Mbx *m = OBJ(uid, K_MBX, mbx);

    if (!m)
        return ERR_UID;
    *(void **)msg = 0;
    if (m->tail)
        *m->tail = msg;
    else
        m->head = msg;
    m->tail = msg;
    preempt();
    return 0;
}

int sceKernelPollMbx(int uid, void **msg)
{
    Mbx *m = OBJ(uid, K_MBX, mbx);

    if (!m)
        return ERR_UID;
    if (!m->head)
        return ERR_NOMSG;
    *msg = m->head;
    m->head = *m->head;
    if (!m->head)
        m->tail = 0;
    return 0;
}

/* --- memory: Fpl/Vpl/partition blocks from the host heap (ponytail: no pool limits) --- */

static int blk_new(unsigned int size, void *ptr)
{
    int uid = ALLOC(K_BLK, blk);

    blk[uid & 0xFFF].size = size;
    blk[uid & 0xFFF].ptr = ptr;
    return uid;
}

static int blk_free(int uid)
{
    Block *b = OBJ(uid, K_BLK, blk);

    if (!b)
        return ERR_UID;
    __libc_free(b->ptr);
    b->used = 0;
    return 0;
}

int sceKernelCreateFpl(const char *name, int part, int attr, unsigned int size, unsigned int blocks, void *opt)
{
    return blk_new(size, 0);
}

int sceKernelAllocateFpl(int uid, void **data, unsigned int *timeout)
{
    Block *b = OBJ(uid, K_BLK, blk);

    if (!b)
        return ERR_UID;
    *data = __libc_memalign(64, b->size);
    memset(*data, 0, b->size);
    return 0;
}

int sceKernelAllocateFplCB(int uid, void **data, unsigned int *timeout) { return sceKernelAllocateFpl(uid, data, timeout); }
int sceKernelTryAllocateFpl(int uid, void **data) { return sceKernelAllocateFpl(uid, data, 0); }
int sceKernelFreeFpl(int uid, void *data) { __libc_free(data); return 0; }
int sceKernelDeleteFpl(int uid) { Block *b = OBJ(uid, K_BLK, blk); if (b) b->used = 0; return 0; }

int sceKernelCreateVpl(const char *name, int part, int attr, int size, void *opt) { return blk_new(size, 0); }

int sceKernelTryAllocateVpl(int uid, unsigned int size, void **data)
{
    *data = __libc_memalign(64, size);
    return 0;
}

int sceKernelAllocateVpl(int uid, unsigned int size, void **data, unsigned int *timeout)
{
    return sceKernelTryAllocateVpl(uid, size, data);
}

int sceKernelFreeVpl(int uid, void *data) { __libc_free(data); return 0; }
int sceKernelDeleteVpl(int uid) { Block *b = OBJ(uid, K_BLK, blk); if (b) b->used = 0; return 0; }

int sceKernelAllocPartitionMemory(int part, const char *name, int type, unsigned int size, void *addr)
{
    void *p = __libc_memalign(64, size);

    memset(p, 0, size);
    return blk_new(size, p);
}

void *sceKernelGetBlockHeadAddr(int uid)
{
    Block *b = OBJ(uid, K_BLK, blk);

    return b ? b->ptr : 0;
}

int sceKernelFreePartitionMemory(int uid) { return blk_free(uid); }
int sceKernelMaxFreeMemSize(void) { return 24 << 20; }
int sceKernelTotalFreeMemSize(void) { return 24 << 20; }

/* --- time --- */

unsigned int sceKernelGetSystemTimeLow(void) { return (unsigned int)(host_now_us() - t0); }
long long sceKernelGetSystemTimeWide(void) { return host_now_us() - t0; }
int sceKernelLibcClock(void) { return (int)(host_now_us() - t0); }

int sceKernelLibcTime(int *t)
{
    int r = (int)time(0);

    if (t)
        *t = r;
    return r;
}

int sceKernelLibcGettimeofday(void *tp, void *tzp)
{
    struct timeval tv;

    gettimeofday(&tv, 0);
    if (tp) {
        ((int *)tp)[0] = (int)tv.tv_sec;
        ((int *)tp)[1] = (int)tv.tv_usec;
    }
    return 0;
}

int sceRtcGetCurrentTick(unsigned long long *tick)
{
    struct timeval tv;
    const char *fixed = getenv("TAGFORCE_RTC");   /* the game seeds rand() with it: fixed = repeatable runs */

    if (fixed) {
        *tick = strtoull(fixed, 0, 0);
        return 0;
    }

    gettimeofday(&tv, 0);
    *tick = (unsigned long long)tv.tv_sec * 1000000 + tv.tv_usec + 62135596800000000ULL;   /* from 0001-01-01 */
    return 0;
}

/* --- display, input: 60 Hz from the host clock, buttons from the keyboard (window.c) --- */

#define VBLANK_US 16683   /* 59.94 Hz */

int sceDisplayGetVcount(void) { return (int)((host_now_us() - t0) / VBLANK_US); }

int sceDisplayWaitVblankStart(void)
{
    long long n = (host_now_us() - t0) / VBLANK_US + 1;

    block(p_false, t0 + n * VBLANK_US);
    return 0;
}

int sceDisplayWaitVblankStartCB(void) { return sceDisplayWaitVblankStart(); }
int sceDisplayWaitVblank(void) { return sceDisplayWaitVblankStart(); }
int sceDisplayWaitVblankCB(void) { return sceDisplayWaitVblankStart(); }
float sceDisplayGetFramePerSec(void) { return 59.94005f; }

static void pad_fill(unsigned int *pad, int count)
{
    unsigned int b = host_buttons();
    int i;

    for (i = 0; i < count; i++, pad += 4) {   /* SceCtrlData: time, buttons, lx, ly, reserved[6] */
        pad[0] = sceKernelGetSystemTimeLow();
        pad[1] = b;
        pad[2] = 0x8080;
        pad[3] = 0;
    }
}

int sceCtrlPeekBufferPositive(void *pad, int count)
{
    pad_fill(pad, count);
    return count;
}

int sceCtrlReadBufferPositive(void *pad, int count)
{
    sceDisplayWaitVblankStart();   /* the PSP blocks until the next sample (one per vblank) */
    pad_fill(pad, count);
    return count;
}

/* --- audio output: no sound yet, the blocking calls pace the sound thread at 44.1 kHz --- */

static int audio_len[8];
static long long audio_next[8];

int sceAudioChReserve(int ch, int samples, int format)
{
    static int n;

    if (ch < 0)
        ch = n++ & 7;
    audio_len[ch & 7] = samples;
    return ch;
}

int sceAudioSetChannelDataLen(int ch, int samples)
{
    audio_len[ch & 7] = samples;
    return 0;
}

static int audio_block(int ch)
{
    long long now = host_now_us();
    long long *next = &audio_next[ch & 7];

    if (*next < now)
        *next = now;
    *next += (long long)audio_len[ch & 7] * 1000000 / 44100;
    block(p_false, *next);
    return audio_len[ch & 7];
}

int sceAudioOutputPannedBlocking(int ch, int l, int r, void *buf) { return audio_block(ch); }
int sceAudioOutputBlocking(int ch, int vol, void *buf) { return audio_block(ch); }

/* no SAS mixing yet: every voice has ended at once (a stub's 0 = "all voices playing" left the duel waiting
   forever for a voice clip to end) */
int __sceSasGetEndFlag(void *core) { return -1; }

/* --- exit --- */

int sceKernelExitGame(void)
{
    dprintf(2, "sceKernelExitGame\n");
    _Exit(0);
}
