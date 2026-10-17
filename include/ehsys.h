/* Cross-subsystem pieces of BOOT ("modehsys"): input state, frame scheduling. Grows on demand. */
#ifndef EHSYS_H
#define EHSYS_H

typedef struct PadState {
    unsigned int down;      /* +0x00 buttons currently down (PSP_CTRL_*) */
    unsigned int held;      /* +0x04 buttons down this frame with repeat (menus navigate on it) */
    unsigned int unk8, unkC;
    int analog_x;           /* +0x10 0..255 */
    int analog_y;           /* +0x14 0..255, dead zone 0x62..0x9E */
    unsigned int unk18, unk1C, unk20;
    unsigned int pressed;   /* +0x24 buttons that went down this frame */
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
    int width, height, x, y;          /* +0x00 480x272 */
    int vwidth, vheight, vx, vy;      /* +0x10 */
    int flag;                         /* +0x20 = 1 */
    int unk24[4];
    unsigned int clear_color;         /* +0x34 */
    unsigned int clear_stencil;       /* +0x38 */
    int unk3c;
    int near_z;                       /* +0x40 50000 */
    int far_z;                        /* +0x44 10000 */
    int unk48;
    int unk4c[4];                     /* +0x4C 16,16,16,16 */
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
