/* Cross-subsystem pieces of BOOT ("modehsys"): input state, frame scheduling. Grows on demand. */
#ifndef EHSYS_H
#define EHSYS_H

typedef struct PadBits {
    unsigned int down;      /* +0x00 buttons currently down (PSP_CTRL_*) */
    unsigned int trig;      /* +0x04 buttons that went down this frame */
    unsigned int release;   /* +0x08 buttons that went up this frame */
    unsigned int rep;       /* +0x0C trig plus per-button auto-repeat */
    int analog_x;           /* +0x10 0..255 */
    int analog_y;           /* +0x14 0..255 */
    unsigned int dir;       /* +0x18 down plus the stick as d-pad bits (dead zone 0x21..0xDE) */
    unsigned int dir_trig;  /* +0x1C */
    unsigned int dir_release; /* +0x20 */
    unsigned int dir_rep;   /* +0x24 dir_trig plus repeat after 15 frames, every 4 */
} PadBits;

typedef struct PadCtrl {    /* SceCtrlData */
    unsigned int time;
    unsigned int buttons;
    unsigned char lx, ly;
    unsigned char rsrv[6];
} PadCtrl;

typedef struct PadState {   /* pad_state 0x08905534 */
    PadBits b;
    PadCtrl last;           /* +0x28 last sample, reused when reading fails */
    int dir_count;          /* +0x38 */
    int ignore_hold;        /* +0x3C when set, HOLD centers the stick */
} PadState;

PadState *pad_get(void);
int pad_init(void);
int pad_update(void);

/* sys_init configuration block (BOOT main passes 0x088EE288) */
typedef struct HeapSpec {
    void *addr;
    unsigned int size;
} HeapSpec;

typedef struct SysInitCfg {
    const char **dirs;    /* +0x00 ehfile directory table */
    int modmgr_arg1;      /* +0x04 replaces -1 in the static module table */
    int modmgr_arg2;      /* +0x08 replaces -2 */
    HeapSpec dl_heap;     /* +0x0C display-list heap */
    int gfx_mode;         /* +0x14 */
    int fps;              /* +0x18 30 or 60 */
} SysInitCfg;

/* display/engine parameters, zeroed and defaulted by sys_config_init (0x0890565C, 0x5C bytes) */
typedef struct SysConf {
    int width, height;                /* +0x00 480x272 */
    int scissor_x, scissor_y;         /* +0x08 */
    int scissor_w, scissor_h;         /* +0x10 */
    int scissor_on;                   /* +0x18 */
    int tfunc;                        /* +0x1C texture function (dl_tex_func) */
    int tfunc_comp;                   /* +0x20 = 1 (rgba) */
    int tfunc_double;                 /* +0x24 color doubling (dl_enable 21) */
    int tex_map;                      /* +0x28 dl_tex_map_mode */
    int tex_proj;                     /* +0x2C */
    unsigned int clear_depth;         /* +0x30 */
    unsigned int clear_color;         /* +0x34 */
    unsigned int clear_stencil;       /* +0x38 */
    int tex_psm;                      /* +0x3C */
    int near_z;                       /* +0x40 50000 */
    int far_z;                        /* +0x44 10000 */
    unsigned int *prim;               /* +0x48 vertex count to patch (dl_prim_begin) */
    int sprite_size[4];               /* +0x4C 16,16,16,16 (draw_set_offsets) */
} SysConf;

typedef struct SysState {
    int id;
    int (*update)(void);
    int (*exit)(void);
} SysState;

typedef struct SysStateStack {
    SysState st[8];       /* 0x0890571C */
    int index;            /* +0x60 */
} SysStateStack;

/* scene thread context (0x089056B8, 100 bytes) */
typedef struct EhscriptCtx {
    int dir;                 /* +0x00 ehfile dir index of gmodule/ */
    char name[32];           /* +0x04 next module name (+".prx") or "staticmodule" */
    char prev_name[32];      /* +0x24 */
    int (*main_fn)(void);    /* +0x44 registered by the scene (ehscript_register) */
    int (*exit_fn)(void);    /* +0x48 */
    void *data;              /* +0x4C */
    int module_id;           /* +0x50 */
    void (*static_fn)(void); /* +0x54 */
    int state;               /* +0x58 1 request, 2 loading, 3 loaded, 5 running */
    int thread;              /* +0x5C */
    int main_thread;         /* +0x60 -1 while main is not sleeping on us */
} EhscriptCtx;

extern SysConf sys_conf;
extern EhscriptCtx ehscript_ctx;
extern SysStateStack sys_state;

void sys_config_init(void);
int sys_init(SysInitCfg *cfg);
int sys_frame_end(void);
int sys_state_update(void);
int sys_state_set_callbacks(int id, int (*update)(void), int (*exit)(void));
int sys_state_push(int id, int (*update)(void), int (*exit)(void));
int sys_state_pop(void);
int sys_state_get_id(void);
int (*sys_state_get_update(void))(void);

int ehscript_start(const char *name, int dir, void (*static_fn)(void));
int ehscript_yield_to(void);
int ehscript_register(int (*main_fn)(void), int (*exit_fn)(void), void *data);
int ehscript_set_module(const char *name);
int ehscript_set_static(void (*fn)(void));
void ehscript_wake_main(void);   /* sceKernelWakeupThread(main): hands the frame back to BOOT's main loop */
void sys_frame_wait(void);       /* called by scenes once per frame */

int gfx_init(int mode, int fps);
int gfx_frame_end(void);
int dl_init(HeapSpec *heap);
int dl_submit(void);
void vram_init(void);

#endif
