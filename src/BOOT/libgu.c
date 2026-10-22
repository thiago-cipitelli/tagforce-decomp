// @range 0x08804A0C-0x08805EC4
// @asm 0x08804A0C-0x08805838
// @asm 0x08805854-0x088059F4
// @asm 0x08805A10-0x08805ACC
// @asm 0x08805AE8-0x08805EC4
/* libgu of the Sony SDK (GCC; names from the export NIDs): display-list contexts, frame/draw/depth
   buffers, the state bits and the first state commands. Nearly every function reaches the libgu
   globals through %hi/%lo, so they stay in the split asm and the C below is under NON_MATCHING;
   it writes the same GE words, which is what the PC port needs to replace. */
/* not psp.h: it declares sceGuStart as void for its callers, the library returns an int */
typedef int SceUID;
SceUID sceKernelCreateEventFlag(const char *name, int attr, int bits, void *opt);
int sceKernelDeleteEventFlag(SceUID evf);
int sceKernelClearEventFlag(SceUID evf, unsigned int bits);
int sceKernelWaitEventFlag(SceUID evf, unsigned int bits, unsigned int wait, unsigned int *out, unsigned int *timeout);
int sceDisplaySetMode(int mode, int width, int height);
int sceDisplaySetFrameBuf(void *top, int buffer_width, int format, int sync);
void *sceGeEdramGetAddr(void);

int sceGuFinishId(unsigned int id);
void gu_update_stall(void);
int sceGeContinue(void);

#ifdef NON_MATCHING /* GCC: every function loads the libgu globals through %hi/%lo */
typedef struct GuList {                /* 0x088F0D5C + i * 0xFC, one per context (0..4) */
    int pad0;
    unsigned int *start;               /* +0x04 */
    unsigned int *cur;                 /* +0x08 */
    int arg;                           /* +0x0C */
    int pad10[2];
    int fbp_psm;                       /* +0x18 */
    int disp_w;                        /* +0x1C */
    int disp_h;                        /* +0x20 */
    int scissor_on;                    /* +0x24 */
    int scissor[4];                    /* +0x28 x0, y0, x1, y1 */
    int pad38[15];
    int obj_depth;                     /* +0x74 sceGuBeginObject nesting */
    unsigned int *obj_stack[32];       /* +0x78 */
    int next;                          /* +0xF8 */
} GuList;

typedef struct GuSettings {            /* 0x088F0D10 */
    int initialized;                   /* +0x00 */
    int started;                       /* +0x04 first sceGuStart done */
    int display_on;                    /* +0x08 */
    int cid;                           /* +0x0C current context */
    int call_mode;                     /* +0x10 */
    unsigned int edram;                /* +0x14 sceGeEdramGetAddr */
    int ge_list;                       /* +0x18 */
    int send_list;                     /* +0x1C */
    int ge_cb;                         /* +0x20 */
    int psm;                           /* +0x24 */
    int fbw;                           /* +0x28 */
    unsigned int draw_fbp;             /* +0x2C */
    unsigned int disp_fbp;             /* +0x30 */
    unsigned int depth_fbp;            /* +0x34 */
    int depth_w;                       /* +0x38 */
    int width;                         /* +0x3C */
    int height;                        /* +0x40 */
    unsigned int states;               /* +0x44 */
    GuList *list;                      /* +0x48 */
    GuList lists[5];                   /* +0x4C */
} GuSettings;

typedef struct GuCallbacks {           /* 0x088F0CE0 */
    void (*signal)(int);
    void (*finish)(int);
    short signals[16];                 /* +0x08 */
    int signal_count;                  /* +0x28 */
    int event_flag;                    /* +0x2C */
} GuCallbacks;

GuSettings gu;
GuCallbacks gu_cb;
void (*gu_swap_pre)(void);             /* 0x088F1248 */
void (*gu_swap_post)(void);            /* 0x088F124C */
void (*gu_disp_hook)(int psm, int w, int h);       /* 0x088F1250 */
void (*gu_draw_hook)(int psm, unsigned int fbp, int fbw); /* 0x088F1254 */

void gu_init_contexts(void);
int sceGeSetCallback(void *cb);
int sceGeUnsetCallback(int cbid);
int sceGeListEnQueue(const void *list, void *stall, int cbid, void *arg);
int sceGeListEnQueueHead(const void *list, void *stall, int cbid, void *arg);
int sceGeListSync(int qid, int mode);
int sceGeDrawSync(int mode);
int sceGeListUpdateStallAddr(int qid, void *stall);
int sceGeBreak(int mode, void *p);
int sceKernelCpuSuspendIntr(void);
void sceKernelCpuResumeIntr(int flags);
void gu_ctx_finish(GuList *l, int mode, unsigned int arg);
void gu_ctx_state(GuList *l, int state, int on);
void gu_ctx_matrix(GuList *l, int type, const float *m);
void gu_signal_handler(int id, void *arg);
void gu_finish_handler(int id, void *arg);
void sceGuSetDither(const int *matrix);
void sceGuPatchDivide(unsigned int ulevel, unsigned int vlevel);
void sceGuColorMaterial(int components);
void sceGuTexFlush(void);
void sceGuTexScale(float u, float v);
void sceGuSpecular(float power);
void gu_update_stall_list(void);

extern const char gu_init_list[];      /* 0x08872B78 */
extern int gu_list_arg[];              /* 0x08872EF0 */
extern const int gu_dither_default[16];/* 0x08872F10 */
extern unsigned int gu_break_param[];  /* 0x088F0CD0 */

static void gu_push(unsigned int w)
{
    *gu.list->cur++ = w;
}

int sceGuInit(void)
{
    struct {
        void (*signal)(int, void *);
        void *signal_arg;
        void (*finish)(int, void *);
        void *finish_arg;
    } cb;
    int r;

    if (gu.initialized) {
        return 0x80000020;
    }
    gu.edram = (unsigned int)sceGeEdramGetAddr();
    gu_init_contexts();
    r = sceKernelCreateEventFlag("SceGuSignal", 0x200, 1, 0);
    if (r < 0) {
        return r;
    }
    gu_cb.event_flag = r;
    cb.signal = gu_signal_handler;
    cb.signal_arg = &gu_cb;
    cb.finish = gu_finish_handler;
    cb.finish_arg = &gu_cb;
    r = sceGeSetCallback(&cb);
    if (r < 0) {
        sceKernelDeleteEventFlag(gu_cb.event_flag);
        gu_cb.event_flag = -1;
        return r;
    }
    gu.ge_cb = r;
    r = sceGeListEnQueue(gu_init_list, 0, -1, 0);
    if (r < 0) {
        sceKernelDeleteEventFlag(gu_cb.event_flag);
        sceGeUnsetCallback(gu.ge_cb);
        gu.ge_cb = -1;
        gu_cb.event_flag = -1;
        return r;
    }
    gu.ge_list = r;
    sceGeListSync(r, 0);
    sceGeDrawSync(0);
    gu.initialized = 1;
    return 0;
}

/* Opens context cid on list (cid 0 is the GE list, 3/4 call lists, the others are built
   uncached and cleared to the next 64 bytes). */
int sceGuStart(int cid, void *list, int arg)
{
    GuList *l;
    int flags;
    unsigned int p = (unsigned int)list & 0x1FFFFFFF;

    if (!gu.initialized) {
        gu.list = 0;
        return 0x80000001;
    }
    flags = sceKernelCpuSuspendIntr();
    if (gu.lists[cid].next >= 0) {
        sceKernelCpuResumeIntr(flags);
        gu.list = 0;
        return 0x80000021;
    }
    gu.list = l = &gu.lists[cid];
    l->next = gu.cid;
    gu.cid = cid;
    sceKernelCpuResumeIntr(flags);
    if (cid - 3U < 2) {
        l->arg = arg;
        l->cur = (unsigned int *)p;
        l->start = (unsigned int *)p;
    } else {
        l->arg = arg;
        l->cur = (unsigned int *)(p | 0x40000000);
        l->start = (unsigned int *)(p | 0x40000000);
        if (((unsigned int)list << 1) >> 31 == 0) {
            flags = sceKernelCpuSuspendIntr();
            while ((unsigned int)l->cur & 0x3F) {
                *l->cur++ = 0;
            }
            sceKernelCpuResumeIntr(flags);
        }
    }
    if (gu.cid == 0) {
        int r;

        gu_cb.signal_count = 0;
        r = sceGeListEnQueue(l->start, l->cur, gu.ge_cb, gu_list_arg);
        if (r < 0) {
            return r;
        }
        gu.ge_list = r;
    }
    if (!gu.started) {
        sceGuSetDither(gu_dither_default);
        sceGuPatchDivide(16, 16);
        sceGuColorMaterial(7);
        sceGuSpecular(1.0f);
        sceGuTexScale(1.0f, 1.0f);
        gu.started = 1;
    }
    if (gu.cid != 0 || gu.fbw == 0) {
        return 0;
    }
    l->fbp_psm = gu.psm;
    l->disp_w = gu.width;
    l->disp_h = gu.height;
    gu_push(gu.psm | 0xD2000000);
    gu_push((gu.draw_fbp & 0xFFFFFF) | 0x9C000000);
    gu_push((((gu.draw_fbp & 0xFFFFFFF) >> 24) << 16) | gu.fbw | 0x9D000000);
    return 0;
}

/* Closes the current context: FINISH/END for the GE list, RET or (call mode 1) a signal for call
   lists, then pops to the previous context. Returns the size in bytes. */
int sceGuFinishId(unsigned int id)
{
    GuList *l = gu.list;
    int size;
    int flags;
    int prev;

    if (l->obj_depth != 0) {
        return 0x80000021;
    }
    switch (gu.cid) {
    case 0: {
        int r;

        gu_push((id & 0xFFFF) | 0x0F000000);
        gu_push(0x0C000000);
        gu_update_stall();
        break;
    }
    case 1:
    case 3:
        if (gu.call_mode == 1) {
            gu_push(0x0E120000);
            gu_push(0x0C000000);
        } else {
            gu_push(0x0B000000);
        }
        break;
    case 2:
    case 4:
        gu_push((id & 0xFFFF) | 0x0F000000);
        gu_push(0x0C000000);
        break;
    default:
        return 0x80000107;
    }
    size = (char *)l->cur - (char *)l->start;
    flags = sceKernelCpuSuspendIntr();
    prev = gu.lists[gu.cid].next;
    gu.lists[gu.cid].next = -1;
    gu.list = prev < 0 ? 0 : &gu.lists[prev];
    gu.cid = prev;
    sceKernelCpuResumeIntr(flags);
    return size;
}

/* Waits until the GE has raised signal id (or 16 signals arrived). */
int sceGuWaitSignal(short id)
{
    int r;

    for (;;) {
        int i;

        for (i = 0; i < gu_cb.signal_count && i < 16; i++) {
            if (gu_cb.signals[i] == id) {
                return 0;
            }
        }
        r = sceKernelWaitEventFlag(gu_cb.event_flag, 1, 0, 0, 0);
        if (r < 0) {
            return r;
        }
        r = sceKernelClearEventFlag(gu_cb.event_flag, 0xFFFFFFFE);
        if (r < 0) {
            return r;
        }
    }
}

/* CALL of a list (call mode 1: through a signal) */
int sceGuCallList(const void *list)
{
    unsigned int p = (unsigned int)list;

    if (gu.call_mode == 1) {
        gu_push(((p & 0xFFFFFFF) >> 16) | 0x0E110000);
        gu_push((p & 0xFFFF) | 0x0C000000);
        gu_update_stall();
    } else {
        gu_push((((p & 0xFFFFFFF) >> 24) << 16) | 0x10000000);
        gu_push((p & 0xFFFFFF) | 0x0A000000);
    }
    return 0;
}

int sceGuSendList(int mode, const void *list, int a2, int a3, int a4)
{
    int arg[4];
    int r;

    arg[0] = 16;
    arg[1] = a2;
    arg[2] = a3;
    arg[3] = a4;
    gu_cb.signal_count = 0;
    if (mode == 1) {
        r = sceGeListEnQueueHead(list, 0, gu.ge_cb, arg);
    } else {
        r = sceGeListEnQueue(list, 0, gu.ge_cb, arg);
    }
    if (r < 0) {
        return r;
    }
    gu.send_list = r;
    return 0;
}

/* display mode and (when the display is on) the displayed frame buffer */
void gu_set_display_mode(int psm, int width, int height, unsigned int dispbp, int fbw)
{
    if (gu_disp_hook != 0) {
        gu_disp_hook(psm, width, height);
    }
    gu.psm = psm;
    gu.fbw = fbw;
    gu.disp_fbp = dispbp;
    gu.width = width;
    gu.height = height;
    sceDisplaySetMode(0, width, height);
    if (gu.display_on == 1) {
        sceDisplaySetFrameBuf((void *)(gu.edram + gu.disp_fbp), fbw, psm, 1);
    }
}

/* swaps draw and display buffers; returns the new draw buffer */
unsigned int sceGuSwapBuffers(void)
{
    unsigned int t;

    if (gu_swap_pre != 0) {
        gu_swap_pre();
    }
    t = gu.disp_fbp;
    gu.disp_fbp = gu.draw_fbp;
    gu.draw_fbp = t;
    if (gu.display_on == 1) {
        sceDisplaySetFrameBuf((void *)(gu.edram + gu.disp_fbp), gu.fbw, gu.psm, 0);
    }
    if (gu_swap_post != 0) {
        gu_swap_post();
    }
    if (gu_draw_hook != 0) {
        gu_draw_hook(gu.psm, gu.draw_fbp, gu.fbw);
    }
    return gu.draw_fbp;
}

/* sceGuDrawBuffer: FRAMEBUFPIXFORMAT, FRAMEBUFPTR, FRAMEBUFWIDTH; copied into every context */
void gu_DrawBuffer(int psm, unsigned int fbp, int fbw)
{
    int i;

    if (gu_draw_hook != 0) {
        gu_draw_hook(psm, fbp, fbw);
    }
    gu_push(psm | 0xD2000000);
    gu_push((fbp & 0xFFFFFF) | 0x9C000000);
    gu_push((((fbp & 0xFFFFFFF) >> 24) << 16) | fbw | 0x9D000000);
    for (i = 0; i < 5; i++) {
        gu.lists[i].fbp_psm = psm;
        gu.lists[i].disp_w = gu.width;
        gu.lists[i].disp_h = gu.height;
    }
    gu.psm = psm;
    gu.draw_fbp = fbp;
    gu.fbw = fbw;
    if (gu.depth_fbp == 0 && gu.height != 0) {
        gu.depth_fbp = fbp + gu.height * fbw * 4;
    }
    if (gu.depth_w == 0) {
        gu.depth_w = fbw;
    }
}

/* sceGuSetAllStatus: one bit per state 0..21 */
void sceGuSetAllStatus(unsigned int status)
{
    int i;

    for (i = 0; i < 22; i++) {
        if ((status >> i) & 1) {
            gu_ctx_state(gu.list, i, 1);
            gu.states |= 1 << i;
        } else {
            gu_ctx_state(gu.list, i, 0);
            gu.states &= ~(1 << i);
        }
    }
}

/* DITH0..3: a 4x4 matrix of 4-bit values */
void sceGuSetDither(const int *m)
{
    int i;

    for (i = 0; i < 4; i++) {
        gu_push((m[i * 4 + 3] & 0xF) << 12 | (m[i * 4 + 2] & 0xF) << 8 | (m[i * 4 + 1] & 0xF) << 4 |
                (m[i * 4] & 0xF) | (0xE2000000 + (i << 24)));
    }
}

/* closes the innermost sceGuBeginObject: patches its BASE/BJUMP to the current position */
int sceGuEndObject(void)
{
    GuList *l = gu.list;
    unsigned int cur;
    unsigned int *p;
    int i = l->obj_depth - 1;

    if (i < 0) {
        return 0x80000025;
    }
    cur = (unsigned int)l->cur;
    p = l->obj_stack[i];
    l->obj_depth = i;
    p[1] = (cur & 0xFFFFFF) | 0x09000000;
    p[0] = (((cur & 0xFFFFFFF) >> 24) << 16) | 0x10000000;
    if (gu.cid == 0) {
        int r = sceGeListUpdateStallAddr(gu.ge_list, (void *)cur);

        if (r < 0) {
            return r;
        }
    }
    return 0;
}

void sceGuPatchDivide(unsigned int ulevel, unsigned int vlevel)
{
    gu_push(vlevel << 8 | ulevel | 0x36000000);
}

void sceGuColorMaterial(int components)
{
    gu_push((components & 7) | 0x53000000);
}

/* SPECULAR power (float24) */
void sceGuSpecular(float power)
{
    union {
        float f;
        unsigned int i;
    } u;

    u.f = power;
    gu_push(u.i >> 8 | 0x5B000000);
}

void sceGuTexScale(float u, float v)
{
    union {
        float f;
        unsigned int i;
    } a, b;

    a.f = u;
    b.f = v;
    gu_push(a.i >> 8 | 0x48000000);
    gu_push(b.i >> 8 | 0x49000000);
}

/* installs the four display hooks and calls the two buffer ones with the current settings */
void gu_set_display_hooks(void (*pre)(void), void (*post)(void), void (*disp)(int, int, int),
                          void (*draw)(int, unsigned int, int))
{
    gu_swap_pre = pre;
    gu_swap_post = post;
    gu_disp_hook = disp;
    gu_draw_hook = draw;
    disp(gu.psm, gu.width, gu.height);
    draw(gu.psm, gu.draw_fbp, gu.fbw);
}

int sceGuTerm(void)
{
    if (!gu.initialized) {
        return 0x80000001;
    }
    sceKernelDeleteEventFlag(gu_cb.event_flag);
    sceGeUnsetCallback(gu.ge_cb);
    gu_cb.event_flag = -1;
    gu.ge_cb = -1;
    gu.list = 0;
    gu.initialized = 0;
    return 0;
}
#endif

#ifdef __MWERKS__
asm int sceGuFinish(void)
{
    addiu sp, sp, -16
    sw ra, 0(sp)
    jal sceGuFinishId
    move a0, zero
    lw ra, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
int sceGuFinish(void)
{
    return sceGuFinishId(0);
}
#endif

#ifdef NON_MATCHING /* GCC: libgu globals through %hi/%lo */
/* reserves size bytes in the list behind a JUMP */
void *sceGuGetMemory(int size)
{
    unsigned int *p = gu.list->cur;
    unsigned int next = (unsigned int)p + ((size + 3) & ~3) + 8;

    gu.list->cur = (unsigned int *)next;
    p[0] = (((next & 0xFFFFFFF) >> 24) << 16) | 0x10000000;
    p[1] = (next & 0xFFFFFF) | 0x08000000;
    return p + 2;
}

/* aligned block in the list: BASE/JUMP written at the old position (it overlaps the block when
   the position is already aligned) */
void *gu_get_memory_aligned(int align, int size)
{
    unsigned int *cur = gu.list->cur;
    unsigned int p = (((unsigned int)cur - 1) | (align - 1)) + 1;
    unsigned int next = p + ((size + 3) & ~3);

    cur[1] = (next & 0xFFFFFF) | 0x08000000;
    gu.list->cur = (unsigned int *)next;
    cur[0] = (((next >> 24) & 0xF) << 16) | 0x10000000;
    return (void *)p;
}

/* bytes written in the current list */
int sceGuCheckList(void)
{
    return (char *)gu.list->cur - (char *)gu.list->start;
}

/* current write position; *left = bytes left before the list's arg limit */
unsigned int *gu_get_list(int *left)
{
    if (left != 0) {
        *left = gu.list->arg - ((char *)gu.list->cur - (char *)gu.list->start);
    }
    return gu.list->cur;
}

void gu_set_list(unsigned int *cur)
{
    gu.list->cur = cur;
}

/* SIGNAL behavior/value through the context, then stall update */
void sceGuSignal(int behavior, int signal)
{
    gu_ctx_finish(gu.list, behavior, signal);
    gu_update_stall();
}

/* sceGuSetCallback: 1 = signal, 4 = finish; returns the previous one */
void *sceGuSetCallback(int which, void (*cb)(int))
{
    void (*old)(int) = 0;

    if (which == 1) {
        old = gu_cb.signal;
        gu_cb.signal = cb;
    } else if (which == 4) {
        old = gu_cb.finish;
        gu_cb.finish = cb;
    }
    return (void *)old;
}

int sceGuBreak(int mode)
{
    return sceGeBreak(mode, gu_break_param);
}
#endif

#ifdef __MWERKS__
asm int sceGuContinue(void)
{
    addiu sp, sp, -16
    sw ra, 0(sp)
    jal sceGeContinue
    nop
    lw ra, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
int sceGuContinue(void)
{
    return sceGeContinue();
}
#endif

#ifdef NON_MATCHING /* GCC: libgu globals through %hi/%lo */
void sceGuCallMode(int mode)
{
    gu.call_mode = mode;
}

/* BASE + JUMP to addr; returns the position after them */
unsigned int *gu_jump(unsigned int addr)
{
    unsigned int *p = gu.list->cur;

    gu.list->cur = p + 2;
    p[0] = ((addr >> 24) & 0xF) << 16 | 0x10000000;
    p[1] = (addr & 0xFFFFFF) | 0x08000000;
    return p + 2;
}

/* 0: draw sync; 3: GE list sync; 4: send list sync */
int sceGuSync(int mode, int what)
{
    switch (mode) {
    case 0:
        return sceGeDrawSync(what);
    case 3:
        return sceGeListSync(gu.ge_list, what);
    case 4:
        return sceGeListSync(gu.send_list, what);
    default:
        return 0;
    }
}
#endif

/* stall-address update of the GE list */
#ifdef __MWERKS__
asm void gu_flush(void)
{
    addiu sp, sp, -16
    sw ra, 0(sp)
    jal gu_update_stall
    nop
    lw ra, 0(sp)
    jr ra
    addiu sp, sp, 16
}
#else
void gu_flush(void)
{
    gu_update_stall();
}
#endif

#ifdef NON_MATCHING /* GCC: libgu globals through %hi/%lo */
/* display on/off; returns the previous state */
int sceGuDisplay(int on)
{
    int old;

    if (on) {
        sceDisplaySetFrameBuf((void *)(gu.edram + gu.disp_fbp), gu.fbw, gu.psm, 1);
    } else {
        sceDisplaySetFrameBuf(0, 0, 0, 1);
    }
    old = gu.display_on;
    gu.display_on = on;
    return old;
}

void sceGuDispBuffer(int width, int height, unsigned int dispbp, int fbw)
{
    int i;

    gu_set_display_mode(gu.psm, width, height, dispbp, fbw);
    for (i = 0; i < 5; i++) {
        gu.lists[i].fbp_psm = gu.psm;
        gu.lists[i].disp_w = gu.width;
        gu.lists[i].disp_h = gu.height;
    }
}

/* sceGuDrawBuffer for the current context only */
void gu_draw_buffer_cur(int psm, unsigned int fbp, int fbw)
{
    GuList *l = gu.list;

    if (gu_draw_hook != 0) {
        gu_draw_hook(psm, fbp, fbw);
    }
    l->fbp_psm = psm;
    l->disp_w = gu.width;
    l->disp_h = gu.height;
    gu_push(psm | 0xD2000000);
    gu_push((fbp & 0xFFFFFF) | 0x9C000000);
    gu_push((((fbp & 0xFFFFFFF) >> 24) << 16) | fbw | 0x9D000000);
}

void sceGuDrawBufferList(int psm, unsigned int fbp, int fbw)
{
    gu_push(psm | 0xD2000000);
    gu_push((fbp & 0xFFFFFF) | 0x9C000000);
    gu_push((((fbp & 0xFFFFFFF) >> 24) << 16) | fbw | 0x9D000000);
}

/* sceGuDepthBuffer */
void gu_DepthBuffer(unsigned int zbp, int zbw)
{
    gu_push((zbp & 0xFFFFFF) | 0x9E000000);
    gu_push((((zbp & 0xFFFFFFF) >> 24) << 16) | zbw | 0x9F000000);
    gu.depth_w = zbw;
    gu.depth_fbp = zbp;
}

/* sceGuEnable */
void gu_Enable(int state)
{
    gu_ctx_state(gu.list, state, 1);
    gu.states |= 1 << state;
}

/* sceGuDisable */
void gu_Disable(int state)
{
    gu_ctx_state(gu.list, state, 0);
    gu.states &= ~(1 << state);
}

void sceGuSetStatus(int state, int on)
{
    if (on) {
        gu_Enable(state);
    } else {
        gu_Disable(state);
    }
}

int sceGuGetStatus(int state)
{
    if (state < 22) {
        return (gu.states >> state) & 1;
    }
    return 0;
}

unsigned int sceGuGetAllStatus(void)
{
    return gu.states;
}

void sceGuSetMatrix(int type, const float *m)
{
    gu_ctx_matrix(gu.list, type, m);
}
#endif
