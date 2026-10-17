// @range 0x0883CEB0-0x0883D708
// @data 0x088EE0A4-0x088EE0C8
/* System glue: display defaults, the "ehscript" scene thread (one gmodule at a time, cooperative
   with BOOT's main loop), init order and the per-scene state stack. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "ehfile.h"
#include "modmgr.h"

int ehfile_request_module(int dir, const char *name, EhfileCallback cb, void *arg);
int ehscript_thread(SceSize args, void *argp);
static int ehscript_load_done_cb(void *arg, int module_id);

void sys_config_init(void)
{
    memset(&sys_conf, 0, sizeof sys_conf);
    sys_conf.width = 480;
    sys_conf.height = 272;
    sys_conf.x = 0;
    sys_conf.y = 0;
    sys_conf.vwidth = 480;
    sys_conf.vheight = 272;
    sys_conf.vx = 0;
    sys_conf.vy = 0;
    sys_conf.flag = 1;
    sys_conf.unk24[0] = 0;
    sys_conf.unk24[1] = 0;
    sys_conf.unk24[2] = 0;
    sys_conf.unk24[3] = 0;
    sys_conf.clear_color = 0;
    sys_conf.clear_stencil = 0;
    sys_conf.unk3c = 0;
    sys_conf.near_z = 50000;
    sys_conf.far_z = 10000;
    sys_conf.unk48 = 0;
    sys_conf.unk4c[0] = 16;
    sys_conf.unk4c[1] = 16;
    sys_conf.unk4c[2] = 16;
    sys_conf.unk4c[3] = 16;
}

int ehscript_start(const char *name, int dir, void (*static_fn)(void))
{
    EhscriptCtx *c = &ehscript_ctx;
    int prio;

    memset(c, 0, sizeof *c);
    c->dir = dir;
    c->prev_name[0] = 0;
    c->state = 1;
    if (name)
        ehscript_set_module(name);
    else
        ehscript_set_static(static_fn);
    prio = sceKernelGetThreadCurrentPriority();
    c->thread = sceKernelCreateThread("ehscript", ehscript_thread, prio + 1, 0x4000, 0, 0);
    c->main_thread = -1;
    sceKernelStartThread(c->thread, 100, c);
    c->main_thread = sceKernelGetThreadId();
    sceKernelSleepThread();
    sceKernelSuspendThread(c->thread);
    c->main_thread = -1;
    return 1;
}

/* main loop: run the scene thread for one frame */
int ehscript_yield_to(void)
{
    ehscript_ctx.main_thread = sceKernelGetThreadId();
    sceKernelResumeThread(ehscript_ctx.thread);
    sceKernelSleepThread();
    sceKernelSuspendThread(ehscript_ctx.thread);
    ehscript_ctx.main_thread = -1;
    return 1;
}

int ehscript_register(int (*main_fn)(void), int (*exit_fn)(void), void *data)
{
    ehscript_ctx.main_fn = main_fn;
    ehscript_ctx.exit_fn = exit_fn;
    ehscript_ctx.data = data;
    return 1;
}

int ehscript_set_module(const char *name)
{
    strcpy(ehscript_ctx.name, name);
    strcat(ehscript_ctx.name, ".prx");
    return 1;
}

int ehscript_set_static(void (*fn)(void))
{
    EhscriptCtx *c = &ehscript_ctx;

    strcpy(ehscript_ctx.name, "staticmodule");
    if ((*(unsigned int *)fn & 0xFC000000) == 0x08000000) { /* function starting with a `j`: thunk, refuse */
        asm { addi $v0, $ra, 0 } /* stray instruction in the original */
        fn = 0;
    }
    c->static_fn = fn;
    return 1;
}

void ehscript_wake_main(void)
{
    sceKernelWakeupThread(ehscript_ctx.main_thread);
}

int ehscript_thread(SceSize args, void *argp)
{
    EhscriptCtx *c;
    int start_status;
    int stop_status;

    for (;;) {
        c = &ehscript_ctx;
        switch (c->state) {
        case 1:
            c->state = 2;
            if (strcmp(c->name, "staticmodule") == 0) {
                c->static_fn();
            } else {
                c->module_id = 0;
                ehfile_request_module(c->dir, c->name, ehscript_load_done_cb, c);
            }
            break;
        case 2:
            if (c->module_id == 0)
                break;
            sceKernelStartModule(c->module_id, 0, 0, &start_status, 0);
            /* fallthrough */
        case 3:
            strcpy(c->prev_name, c->name);
            c->state = 5;
            c->name[0] = 0;
            c->static_fn = 0;
            c->main_fn(); /* the whole scene runs inside this call */
            goto running;  /* falls into case 5; written this way so case 4 keeps its place (matches) */
        case 4:
            break;
        case 5:
        running:
            if (c->name[0] != 0) { /* a next scene was requested */
                c->exit_fn();
                c->prev_name[0] = 0;
                c->main_fn = 0;
                c->exit_fn = 0;
                c->data = 0;
                if (c->module_id > 0) {
                    sceKernelStopModule(c->module_id, 0, 0, &stop_status, 0);
                    sceKernelUnloadModule(c->module_id);
                    c->module_id = 0;
                }
                c->state = 1;
            }
            break;
        }
        sceKernelWakeupThread(c->main_thread);
    }
}

static int ehscript_load_done_cb(void *arg, int module_id)
{
    if (module_id <= 0)
        return 0;
    ((EhscriptCtx *)arg)->module_id = module_id;
    return 1;
}

int sys_init(SysInitCfg *cfg)
{
    gfx_init(cfg->gfx_mode, cfg->fps == 30 ? 30 : 60);
    ehfile_init(&cfg->dirs);
    modmgr_init(cfg->modmgr_arg1, cfg->modmgr_arg2);
    dl_init(&cfg->dl_heap);
    pad_init();
    vram_init();
    return 1;
}

int sys_frame_end(void)
{
    gfx_frame_end();
    dl_submit();
    pad_update();
    return 1;
}

int sys_state_update(void)
{
    SysStateStack *ss = &sys_state;

    if (ss->st[ss->index].update)
        ss->st[ss->index].update();
    return ss->st[ss->index].id;
}

int sys_state_set_callbacks(int id, int (*update)(void), int (*exit)(void))
{
    SysStateStack *ss = &sys_state;
    int prev = ss->st[ss->index].id;

    if (ss->st[ss->index].exit)
        ss->st[ss->index].exit();
    ss->st[ss->index].id = id;
    ss->st[ss->index].update = update;
    ss->st[ss->index].exit = exit;
    return prev;
}

int sys_state_push(int id, int (*update)(void), int (*exit)(void))
{
    int prev = sys_state.st[sys_state.index].id;

    sys_state.index++;
    sys_state.st[sys_state.index].id = id;
    sys_state.st[sys_state.index].update = update;
    sys_state.st[sys_state.index].exit = exit;
    return prev;
}

int sys_state_pop(void)
{
    SysStateStack *ss = &sys_state;
    int prev = ss->st[ss->index].id;

    if (ss->st[ss->index].exit)
        ss->st[ss->index].exit();
    ss->st[ss->index].id = -1;
    ss->st[ss->index].update = 0;
    ss->st[ss->index].exit = 0;
    ss->index--;
    return prev;
}

int sys_state_get_id(void)
{
    return sys_state.st[sys_state.index].id;
}

int (*sys_state_get_update(void))(void)
{
    return sys_state.st[sys_state.index].update;
}
