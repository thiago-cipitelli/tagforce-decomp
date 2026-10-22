// @range 0x0883451C-0x08835650
// @asm 0x08834848-0x08835130
// @data 0x088ED9CC-0x088EDFC4
/* Graphics frame: GE/libgu initialization, end of frame (vsync with a 30 fps cap, flip callback,
   swap), draw targets, screenshots to host0: as BMP or TGA, and the GE state tracker: every
   command pushed through dl_push1 is recorded so that the default state list (one word per GE
   command, enqueued at the end of the frame) can restore it. */
#include "psp.h"
#include "libc.h"
#include "allegrex.h"

typedef struct GfxState {              /* 0x08905084 */
    unsigned int frame;                /* bit 0: displayed buffer */
    int fps;                           /* +0x04 30 or 60 */
} GfxState;

typedef struct GfxFlip {               /* 0x0890508C */
    int hcount;                        /* at the flip */
    int hcount_finish;                 /* +0x04 at the GE finish callback */
    void (*on_flip)(void);             /* +0x08 */
} GfxFlip;

typedef struct BmpInfo {               /* 0x08904908 */
    int size;
    int width;
    int height;
    unsigned short planes;
    unsigned short bits;
    int compression;
    int image_size;
    int xppm;
    int yppm;
    int used;
    int important;
} BmpInfo;

extern GfxState gfx_state;             /* 0x08905084 */
extern GfxFlip gfx_flip;               /* 0x0890508C */
extern int gfx_last_vcount;            /* 0x089048F0 */
extern int gfx_bmp_count;              /* 0x089048F4 */
extern unsigned short gfx_bmp_file[8]; /* 0x089048F8 */
extern BmpInfo gfx_bmp_info;           /* 0x08904908 */
extern unsigned int gfx_bmp_list[0x40]; /* 0x08904940 */
extern int gfx_tga_count;              /* 0x08904A40 */
extern unsigned int gfx_tga_list[0x40]; /* 0x08904A80 */
extern int dl_saved_n;                 /* 0x08904B80 */
extern unsigned int dl_saved[0x100];   /* 0x08904B84 */
extern unsigned char dl_dirty[0x100];  /* 0x08904F84 0xFF never restored, 1 changed this frame */
extern unsigned int gu_init_list[];    /* 0x08874FE8 */
extern unsigned int gfx_fbp_cmd[2];    /* 0x0887535C */
extern unsigned int gfx_fbw_cmd[2];    /* 0x08875364 */

void sceGuInit(void);
void sceGuStart(int cid, void *list, int size);
int sceGuFinish(void);
int sceGuSync(int mode, int what);
void *sceGuSwapBuffers(void);
int sceGuDisplay(int state);
void *sceGuSetCallback(int signal, void (*cb)(int));
void gu_DrawBuffer(int psm, void *fbp, int fbw);
void sceGuDispBuffer(int w, int h, void *fbp, int fbw);
void sceGuDrawBufferList(int psm, void *fbp, int fbw);
void gu_DepthBuffer(void *zbp, int zbw);
void gu_Disable(int state);
void gu_Enable(int state);
void gu_Offset(int x, int y);
void gu_Viewport(int cx, int cy, int w, int h);
void gu_DepthRange(int near, int far);
void gu_Scissor(int x, int y, int w, int h);
void gu_DepthFunc(int f);
void gu_FrontFace(int f);
void gu_set_color_mask(int m);
void gu_set_param_50(int v);
void gu_set_param_4c(int v);
void gu_set_param_494(int v);
void ehsys_08806378(int psm, int sx, int sy, int w, int h, int srcw, void *src, int dx, int dy, int destw, void *dest);
void ehsys_0880847C(void);
int sceGeEdramSetAddrTranslation(int width);
int sceGeListEnQueue(const void *list, void *stall, int cbid, void *arg);
int sceGeListSync(int qid, int mode);
int sceDisplayGetAccumulatedHcount(void);
int sceDisplayGetVcount(void);
int sceDisplayWaitVblankStartCB(void);
int snprintf(char *buf, int size, const char *fmt, ...);
void *dl_get_layer(void);
void dl_set_list(void *p);
void *dl_select_layer(void *layer, unsigned int *size);
void dl_set_cur_list_base(void *p);
void *dl_get_cur_list_base(void);

void dl_state_init(void);
void dl_state_reset(void);
void gfx_draw_target(unsigned int *list, int psm, void *fbp, unsigned int zbp, unsigned int fbw);
void gfx_finish_cb(int arg);

char gfx_host_dir[] = "host0:./";
char gfx_bmp_fmt[] = "cg%06d.bmp";
unsigned char gfx_tga_header[18] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xE0, 0x01, 0x10, 0x01, 0x18, 0x28 };
char gfx_tga_footer[28] = "\0\0\0\0\0\0\0\0TRUEVISION-XFILE.";
char gfx_tga_fmt[] = "cg%06d.tga";

/* Default GE state: one command per opcode (index = opcode), then the framebuffer setup. */
unsigned int gfx_ge_defaults[359] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x13000000, 0x00000000, 0x15000000, 0x16043DDF, 0x17000000,
    0x18000000, 0x19000000, 0x1A000000, 0x1B000000, 0x1C000000, 0x1D000001,
    0x1E000000, 0x1F000000, 0x20000000, 0x21000001, 0x22000000, 0x23000001,
    0x24000000, 0x25000000, 0x26000000, 0x27000000, 0x28000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x42437000, 0x43C30800, 0x44C69C40, 0x45450000, 0x46450000, 0x4746EA60,
    0x483F8000, 0x493F8000, 0x4A000000, 0x4B000000, 0x4C007100, 0x4D007780,
    0x00000000, 0x00000000, 0x50000000, 0x51000000, 0x00000000, 0x53000007,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x9B000000,
    0x9C000000, 0x9D000000, 0x9E110000, 0x9F000200, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xC0000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xC6000000, 0xC7000101, 0xC8000001, 0xC9000100, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xD2000003, 0x00000000, 0xD4000000, 0xD5043DDF, 0xD6002710, 0xD700C350,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xDE000007, 0xDF000032, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0xE7000000, 0xE8000000, 0xE9000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x55000000, 0x58000000,
    0x14000000, 0x10000000, 0x01000010, 0x08000178, 0x00000000, 0x00100000,
    0x00000110, 0x00000010, 0x00200000, 0x00000110, 0x00000020, 0x00300000,
    0x00000110, 0x00000030, 0x00400000, 0x00000110, 0x00000040, 0x00500000,
    0x00000110, 0x00000050, 0x00600000, 0x00000110, 0x00000060, 0x00700000,
    0x00000110, 0x00000070, 0x00800000, 0x00000110, 0x00000080, 0x00900000,
    0x00000110, 0x00000090, 0x00A00000, 0x00000110, 0x000000A0, 0x00B00000,
    0x00000110, 0x000000B0, 0x00C00000, 0x00000110, 0x000000C0, 0x00D00000,
    0x00000110, 0x000000D0, 0x00E00000, 0x00000110, 0x000000E0, 0x00F00000,
    0x00000110, 0x000000F0, 0x01000000, 0x00000110, 0x00000100, 0x01100000,
    0x00000110, 0x00000110, 0x01200000, 0x00000110, 0x00000120, 0x01300000,
    0x00000110, 0x00000130, 0x01400000, 0x00000110, 0x00000140, 0x01500000,
    0x00000110, 0x00000150, 0x01600000, 0x00000110, 0x00000160, 0x01700000,
    0x00000110, 0x00000170, 0x01800000, 0x00000110, 0x00000180, 0x01900000,
    0x00000110, 0x00000190, 0x01A00000, 0x00000110, 0x000001A0, 0x01B00000,
    0x00000110, 0x000001B0, 0x01C00000, 0x00000110, 0x000001C0, 0x01D00000,
    0x00000110, 0x000001D0, 0x01E00000, 0x00000110, 0xD3000701, 0x12800100,
    0x0406003C, 0xD3000000, 0x13000000, 0x0F000000, 0x0C000000,
};
int gfx_fb_offset[2] = { 0x88000, 0 };

int gfx_init(void *arg, int fps)
{
    unsigned int list[0x100];

    gfx_state.frame = 0;
    gfx_state.fps = fps;
    sceGuInit();
    sceGeEdramSetAddrTranslation(0x800);
    dl_state_init();
    sceGuStart(0, list, sizeof(list));
    gu_DrawBuffer(3, 0, 0x200);
    sceGuDispBuffer(480, 272, (void *)0x88000, 0x200);
    gu_DepthBuffer((void *)0x110000, 0x200);
    gu_Disable(6);
    gu_Offset(0x710, 0x778);
    gu_Viewport(0x800, 0x800, 480, 272);
    gu_DepthRange(50000, 10000);
    gu_Disable(4);
    gu_Scissor(0, 0, 480, 272);
    gu_Enable(2);
    gu_DepthFunc(7);
    gu_Enable(1);
    gu_FrontFace(1);
    gu_set_color_mask(0);
    gu_set_param_50(0);
    gu_set_param_4c(0);
    gu_set_param_494(7);
    sceGuFinish();
    sceGuSync(0, 0);
    sceGuDisplay(1);
    sceGuSetCallback(4, gfx_finish_cb);
    return 1;
}

/* Waits for the GE and the vblank (twice at 30 fps when the last flip was less than 2 vblanks
   ago), then flips. */
int gfx_frame_end(void)
{
    GfxFlip *fl = &gfx_flip;
    int h;

    sceDisplayGetAccumulatedHcount();
    sceGuSync(0, 0);
    ehsys_0880847C();
    if (gfx_state.fps == 30) {
        if (sceDisplayGetVcount() - gfx_last_vcount < 2) {
            sceDisplayWaitVblankStartCB();
        }
        gfx_last_vcount = sceDisplayGetVcount();
    }
    sceDisplayWaitVblankStartCB();
    h = sceDisplayGetAccumulatedHcount();
    if (fl->on_flip != 0) {
        fl->on_flip();
    }
    fl->hcount = h;
    sceGuSwapBuffers();
    sceGuSync(0, 0);
    gfx_state.frame++;
    dl_state_reset();
    return 1;
}

void gfx_finish_cb(int arg)
{
    gfx_flip.hcount_finish = sceDisplayGetAccumulatedHcount();
}

GfxState *gfx_get_state(void)
{
    return &gfx_state;
}

void gfx_set_fps(int fps)
{
    gfx_state.fps = fps;
}

/* fbp -1: the depth buffer area; -2: right after the w x h buffer at `base`. */
void gfx_set_draw_target(unsigned int *list, int base, unsigned int fbp, int w, int h, int psm)
{
    if (fbp == 0xFFFFFFFF) {
        fbp = 0x110000;
    } else if (fbp == 0xFFFFFFFE) {
        if (psm == 3) {
            fbp = base + h * (w << 2);
        } else {
            fbp = base + h * (w << 1);
        }
        fbp = fbp + 0x7FFF & 0xFFFF8000;
    }
    gfx_draw_target(list, psm, (void *)base, fbp, w);
}

void gfx_set_default_target(unsigned int *list)
{
    gfx_draw_target(list, 3, (gfx_get_state()->frame & 1) ? (void *)0 : (void *)0x88000, 0x110000, 0x200);
}

/* Next free index of host0:./<fmt>: doubling search, then bisection. */
#define NEXT_FILE(path, name, fmt, count)                                    \
    do {                                                                     \
        int n = 1;                                                           \
        int fd;                                                              \
        for (;;) {                                                           \
            snprintf(name, 0x40, fmt, count + n);                            \
            fd = sceIoOpen(path, 1, 0);                                      \
            if (fd < 0) {                                                    \
                break;                                                       \
            }                                                                \
            sceIoClose(fd);                                                  \
            n <<= 1;                                                         \
        }                                                                    \
        if (n < 2) {                                                         \
            count += n;                                                      \
        } else {                                                             \
            int lo = n >> 1;                                                 \
            int mid = (lo + n) / 2;                                          \
            while (lo < mid) {                                               \
                snprintf(name, 0x40, fmt, count + mid);                      \
                fd = sceIoOpen(path, 1, 0);                                  \
                if (fd >= 0) {                                               \
                    sceIoClose(fd);                                          \
                    lo = mid;                                                \
                } else {                                                     \
                    n = mid;                                                 \
                }                                                            \
                mid = (lo + n) / 2;                                          \
            }                                                                \
            snprintf(name, 0x40, fmt, count + n);                            \
            count += n;                                                      \
        }                                                                    \
    } while (0)

/* Converts the 480x272 RGBA frame copied to `p` into BGR in place. */
#define RGBA_TO_BGR(p)                                                       \
    do {                                                                     \
        unsigned char *d;                                                    \
        unsigned char *s;                                                    \
        int i;                                                               \
        p[0] = p[2];                                                         \
        p[2] = p[0];                                                         \
        d = p + 3;                                                           \
        s = p + 4;                                                           \
        for (i = 1; i < 480 * 272; i++) {                                    \
            d[0] = s[2];                                                     \
            d[1] = s[1];                                                     \
            d[2] = s[0];                                                     \
            d += 3;                                                          \
            s += 4;                                                          \
        }                                                                    \
    } while (0)

#ifdef NON_MATCHING /* the doubling search jumps back through a trampoline block in the original */
/* Screenshot of the displayed buffer as host0:./cgNNNNNN.bmp (`buf`: 0x5FA00 bytes of work). */
void gfx_save_bmp(unsigned int buf)
{
    char path[0x40];
    char *name;
    int fd;
    unsigned char *p;
    int fb;

    strcpy(path, gfx_host_dir);
    name = path + strlen(path);
    NEXT_FILE(path, name, gfx_bmp_fmt, gfx_bmp_count);
    if ((fd = sceIoOpen(path, 0x202, 0x2666)) < 0) {
        return;
    }
    p = (unsigned char *)(buf & ~0xF);
    gfx_bmp_file[0] = 0x4D42;
    gfx_bmp_file[1] = 0x36;
    gfx_bmp_file[2] = 0x40;
    gfx_bmp_file[3] = 0;
    gfx_bmp_file[4] = 0;
    gfx_bmp_file[5] = 0x36;
    gfx_bmp_file[6] = 0;
    gfx_bmp_info.size = 0x28;
    gfx_bmp_info.width = 480;
    gfx_bmp_info.height = -272;
    gfx_bmp_info.planes = 1;
    gfx_bmp_info.bits = 24;
    gfx_bmp_info.compression = 0;
    gfx_bmp_info.image_size = 0x5FA00;
    gfx_bmp_info.xppm = 1;
    gfx_bmp_info.yppm = 1;
    gfx_bmp_info.used = 0;
    gfx_bmp_info.important = 0;
    sceIoWrite(fd, gfx_bmp_file, 0xE);
    sceIoWrite(fd, &gfx_bmp_info, 0x28);
    fb = (gfx_get_state()->frame & 1) ? 0x88000 : 0;
    sceKernelDcacheWritebackInvalidateAll();
    sceGuStart(0, gfx_bmp_list, 0x100);
    ehsys_08806378(3, 0, 0, 480, 272, 0x200, (char *)sceGeEdramGetAddr() + fb, 0, 0, 480, p);
    sceGuFinish();
    sceGuSync(0, 0);
    RGBA_TO_BGR(p);
    sceIoWrite(fd, p, 0x5FA00);
    sceIoClose(fd);
}
#endif

#ifdef NON_MATCHING /* same as gfx_save_bmp */
/* Same as a TGA (uncompressed true color, top-down, with the TRUEVISION footer). */
void gfx_save_tga(unsigned int buf)
{
    char path[0x40];
    char *name;
    int fd;
    unsigned char *p;
    int fb;

    strcpy(path, gfx_host_dir);
    name = path + strlen(path);
    NEXT_FILE(path, name, gfx_tga_fmt, gfx_tga_count);
    if ((fd = sceIoOpen(path, 0x202, 0x2666)) < 0) {
        return;
    }
    p = (unsigned char *)(buf & ~0xF);
    sceIoWrite(fd, gfx_tga_header, 0x12);
    fb = (gfx_get_state()->frame & 1) ? 0x88000 : 0;
    sceKernelDcacheWritebackInvalidateAll();
    sceGuStart(0, gfx_tga_list, 0x100);
    ehsys_08806378(3, 0, 0, 480, 272, 0x200, (char *)sceGeEdramGetAddr() + fb, 0, 0, 480, p);
    sceGuFinish();
    sceGuSync(0, 0);
    RGBA_TO_BGR(p);
    sceIoWrite(fd, p, 0x5FA00);
    sceIoWrite(fd, gfx_tga_footer, 0x1C);
    sceIoClose(fd);
}
#endif

/* Emits FBP/FBW for the depth buffer, then a sub-list with the draw buffer. */
void gfx_draw_target(unsigned int *list, int psm, void *fbp, unsigned int zbp, unsigned int fbw)
{
    void *layer;
    unsigned int size;
    int n;

    list[0] = (zbp & 0xFFFFFF) | 0x9E000000;
    list[1] = ((int)zbp >> 24 & 0xFF) << 16 | 0x9F000000 | fbw;
    layer = dl_get_layer();
    dl_set_list(list + 2);
    dl_set_cur_list_base(layer = dl_select_layer(layer, &size));
    sceGuStart(2, layer, size);
    sceGuDrawBufferList(psm, fbp, fbw);
    layer = dl_get_layer();
    list = dl_get_cur_list_base();
    n = sceGuFinish();
    dl_set_list((char *)list + n - 8);
    dl_select_layer(layer, 0);
}

void dl_state_init(void)
{
    int i;

    sceGeListSync(sceGeListEnQueue(gu_init_list, 0, -1, 0), 0);
    for (i = 0; i < 0x100; i++) {
        if (gfx_ge_defaults[i] != 0) {
            dl_dirty[i] = 0;
        } else {
            dl_dirty[i] = 0xFF;
        }
    }
    dl_saved_n = 0;
}

/* Pushes a GE command, remembering the default of its opcode the first time in the frame. */
unsigned int *dl_push1(unsigned int *list, unsigned int cmd)
{
    if (dl_dirty[cmd >> 24] == 0) {
        dl_dirty[cmd >> 24] = 1;
        dl_saved[dl_saved_n] = gfx_ge_defaults[cmd >> 24];
        dl_saved_n++;
        *list = cmd;
    } else {
        *list = cmd;
    }
    return list + 1;
}

unsigned int *dl_push(unsigned int *list, unsigned int *words, int n)
{
    int i;

    for (i = 0; i < n; i++) {
        list = dl_push1(list, words[i]);
    }
    return list;
}

/* Appends the saved defaults (restoring the state changed since the last flush). */
char *dl_state_flush(char *list)
{
    int n = dl_saved_n;
    int i;

    dl_saved_n = 0;
    for (i = 0; i < n; i++) {
        dl_dirty[dl_saved[i] >> 24] = 0;
    }
    memcpy(list, dl_saved, n * 4);
    return list + n * 4;
}

void dl_state_reset(void)
{
    unsigned int fr = gfx_state.frame;
    unsigned int *u = (unsigned int *)ALLEGREX_INS((unsigned int)gfx_ge_defaults, 4, 28, 4);
    int b = (fr & 1) * 4;
    int i = 0;
    unsigned char *f;

    u[0x9C] = *(unsigned int *)((char *)gfx_fbp_cmd + b);
    u[0x9D] = *(unsigned int *)((char *)gfx_fbw_cmd + b);
    dl_saved_n = 0;
    f = dl_dirty;
    for (; i < 0x100; i++) {
        if (*f != 0xFF) {
            *f = 0;
        }
        f++;
    }
    sceGeListSync(sceGeListEnQueue(gfx_ge_defaults, 0, -1, 0), 0);
}

/* Copies the displayed frame (or `src`) to `dst` (default: the displayed frame). */
void gfx_copy_frame(void *dst, void *src)
{
    unsigned int b = gfx_state.frame & 1;
    void *fb = (void *)(gfx_fb_offset[b] + (int)sceGeEdramGetAddr());

    if (src == 0) {
        src = fb;
    } else if (dst == 0) {
        dst = fb;
    }
    sceKernelDcacheWritebackInvalidateAll();
    ehsys_08806378(3, 0, 0, 480, 272, 0x200, src, 0, 0, 0x200, dst);
}

void *gfx_set_flip_cb(void (*cb)(void))
{
    void *old = gfx_flip.on_flip;

    gfx_flip.on_flip = cb;
    return old;
}
