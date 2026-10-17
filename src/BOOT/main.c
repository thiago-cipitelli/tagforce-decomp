// @range 0x0883DD58-0x0883E13C
// @data 0x088EE0D8-0x088EE2E8
/* BOOT entry after the C runtime: init order, the frame loop, soft reset and small accessors of
   the game globals. Continues in main2.c (stat_event_record, 0x0883E13C, stays in asm). */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "ehfile.h"
#include "modmgr.h"
#include "game.h"

void on_exit_cleanup(void);
void hold_pool_create(void);
int main_thread_init(void);
int font_init(int);
int font_set_param2c(int);
void dl_set_mode(int);
void gfx_pool_init(void *, int, int, int, int);
int snd_init(void);
void frame_debug_update(void);
void snd_channels_update(void);
void snd_stream_update(void);
void snd_channels_reset(void);
void lang_apply(void);
int sysflag_get2(int id);
void sysflag_set2(int id, int value);
void *dl_select_layer(int layer, unsigned int *size_out);
void dl_set_cur_list_base(void *base);
void *dl_get_cur_list_base(void);
void dl_set_list(void *list);
void gu_set_color_mask(int);
void gu_set_param_50(int);
void gu_set_param_4c(int);
void gu_set_param_494(int);
void ehf_set_frame_alias(void *ehf, int frame);
void ehf_set_anim(void *ehf, int a, int b);
unsigned int *gfx_get_state(void);
void *gfx_begin_2d(void *ctx);
void *ehf_draw(void *ctx, void *ehf, void *frame);
void *gfx_end_2d(void *ctx);
void cfg_set_byte0_bit3(int);
void cfg_set_byte0_bits4_5(int);
void cfg_set_bits_1_3_4(int);
void cfg_set_flag(int which, int on);
void snd_set_volume(int channel, int vol);
void ehp_mount(int slot, void *pack);
void *heap_create(void *mem, int size);
void *ehp_find(void *pack, const char *name, int flags);
extern unsigned char gfx_pool_mem[];   /* 0x08905780 */

static int exit_callback(int arg1, int arg2, void *common);
void sys_check_reboot(void);
void loading_screen_init(void);
void gfx_overlay_enable(int on);
void frame_tick(void);
void loading_overlay_draw(void); /* defined in main2.c */
void cfg_defaults_apply(void); /* also called from newgame_init (asm) */
void rand_seed_from_rtc(void);

const char *dir_table[] = {
    "disc0:/PSP_GAME/USRDIR/module/",
    "disc0:/PSP_GAME/USRDIR/kmodule/",
    "disc0:/PSP_GAME/USRDIR/gmodule/",
    "disc0:/PSP_GAME/USRDIR/",
    "disc0:/PSP_GAME/USRDIR/duel/",
    "disc0:/PSP_GAME/USRDIR/duelsys/",
    "disc0:/PSP_GAME/USRDIR/main/",
    "disc0:/PSP_GAME/USRDIR/snd/",
    0,
};

ModTableEntry game_module_table[] = {
    { 8, 2, "libduel_eng.prx", 0, 0 },
    { 9, 2, "libduel_draw.prx", 0, 0 },
    { 10, 2, "libdeck_swap.prx", 0, 0 },
    { 11, 2, "libtutorial.prx", 0, 0 },
    { 0, 0, 0, 0, 0 },
};

SysInitCfg sys_config = { dir_table, 1, 0, { dl_heap, 0x15C000 }, 0, 30 };

static int exit_callback(int arg1, int arg2, void *common)
{
    on_exit_cleanup();
    sceKernelExitGame();
    return 0;
}

/* L+R+Start+Select with no I/O pending: relaunch the game */
void sys_check_reboot(void)
{
    PadState *pad = pad_get();

    if (ehfile_busy())
        return;
    if ((pad->pressed & 0x309) != 0x309)
        return;
    if (sysvar_get_a90())
        return;
    sceKernelLoadExec("disc0:/PSP_GAME/SYSDIR/BOOT.BIN", 0);
}

void main(void)
{
    SceUID cb;

    free(malloc(0x10));
    hold_pool_create();
    sceUmdActivate(1, "disc0:");
    sceUmdWaitDriveStat(0x20);
    ehfile_build_lbn_table(0x10000);
    sys_init(&sys_config);
    main_thread_init();
    font_init(0);
    font_set_param2c(4);
    dl_set_mode(1);
    gfx_pool_init(gfx_pool_mem, 4, 4, 0x40, 4);
    sys_frame_end();
    modmgr_register_table(game_module_table);
    snd_init();
    loading_screen_init();
    gfx_overlay_enable(1);
    ehscript_start("libtitle", 2, 0);
    cb = sceKernelCreateCallback("EXIT_CALLBACK", exit_callback, 0);
    sceKernelRegisterExitCallback(cb);
    for (;;) {
        ehscript_yield_to(); /* run the scene for one frame */
        frame_tick();
        sys_state_update();
        frame_debug_update();
        sys_frame_end();
        sys_check_reboot();
    }
}

unsigned char *sys_info_get(void)
{
    return g_game.info;
}

unsigned int sysflags_test_mask(unsigned int mask)
{
    return g_game.sysflags & mask;
}

void sysflags_set_mask(int on, unsigned int mask)
{
    if (on)
        g_game.sysflags |= mask;
    else
        g_game.sysflags &= ~mask;
}

int sysflag_test(unsigned int bit)
{
    SysSave *s = sysstate_get();
    return (s->flags & (1 << bit)) != 0;
}

void sysflag_set(unsigned int bit)
{
    SysSave *s = sysstate_get();
    s->flags |= 1 << bit;
}

void frame_counter_reset(void)
{
    g_game.frame_counter = 0;
}

unsigned int frame_counter_get(void)
{
    return g_game.frame_counter;
}

unsigned int rand_next_alias(void)
{
    return rand_next();
}

void rand_seed_from_rtc(void)
{
    unsigned long long tick;

    sceRtcGetCurrentTick(&tick);
    srand((unsigned int)tick);
}

void frame_tick(void)
{
    SysSave *s = sysstate_get();

    if (s->play_counter < 0xFFFFFFFFF) /* 9 F's in the original: promotes to 64-bit */
        s->play_counter++;
    if (!sysflag_get2(0x1B) && s->play_counter >= 0x20F580) /* 20 hours at 30 fps */
        sysflag_set2(0x1B, 2);
    g_game.frame_counter++;
    if (g_game.overlay_enabled)
        loading_overlay_draw();
    snd_channels_update();
    snd_stream_update();
}

void sysvar_set_a98(int v) { g_game.var_a98 = v; }
int sysvar_get_a98(void) { return g_game.var_a98; }
void sysvar_set_a94(int v) { g_game.var_a94 = v; }
int sysvar_get_a94(void) { return g_game.var_a94; }
void sysvar_set_a90(int v) { g_game.var_a90 = v; }
int sysvar_get_a90(void) { return g_game.var_a90; }
void sysvar_set_a8c(int v) { g_game.var_a8c = v; }
int sysvar_get_a8c(void) { return g_game.var_a8c; }

