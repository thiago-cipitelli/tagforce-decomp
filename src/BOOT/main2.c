// @range 0x0883E3A4-0x0883E670
// @data 0x088EE2E8-0x088EE2F4
/* Second half of main.c's unit: save-state byte setter, the loading overlay, config defaults and
   loading_screen_init (which mounts the embedded EHP packs and zeroes the game globals). */
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
void loading_overlay_draw(void);
void cfg_defaults_apply(void); /* also called from newgame_init (asm) */
void rand_seed_from_rtc(void);

#ifdef NON_MATCHING
/* Some cards map to fixed event slots, others to `kind`-relative ones. Functionally equivalent to
   0x0883E13C; the original keeps 264 bytes of dead per-case blocks that mwccpsp will not reproduce
   from any C form tried (see docs/build.md), so the matching build uses the asm. */
void stat_event_record(int card_id, int kind)
{
    SysSave *s = sysstate_get();

    switch (card_id) {
    case 0x18c8: break;
    case 0x1956: kind += 2; break;
    case 0x1943: kind += 4; break;
    case 0x19ef: kind += 6; break;
    case 0x18fa: kind += 8; break;
    case 0x17ef: kind += 10; break;
    case 0x1955: kind += 12; break;
    case 0x17ee: kind += 14; break;
    case 0x198c: kind += 16; break;
    case 0x1951: kind += 18; break;
    case 0x18fc: kind += 20; break;
    case 0x18fd: kind += 22; break;
    case 0x18ab: kind += 24; break;
    case 0x1894: kind += 26; break;
    case 0x1895: kind += 28; break;
    case 0x1788: kind += 30; break;
    case 0x1441: kind += 32; break;
    case 0x19f8: kind += 34; break;
    case 0x19a5: kind += 36; break;
    case 0x19a4: kind += 38; break;
    case 0x19a3: kind += 40; break;
    case 0xFBB: kind = 42; break;
    case 0x1468: kind = 43; break;
    default: return;
    }
    s->event_seen[kind] = 1;
}
#endif

void sysstate_set_byte_3e38(int i)
{
    SysSave *s = sysstate_get();

    s->flag_3e38[i] = 1;
}

/* Static display list that draws the loading animation on layer 0 */
void gfx_overlay_enable(int on)
{
    void *base;
    unsigned int size;
    int n;

    if (on) {
        if (g_game.overlay_enabled)
            return;
        dl_set_cur_list_base(base = dl_select_layer(0, &size)); /* written inside the call: matches */
        sceGuStart(2, base, size);
        gu_set_color_mask(0);
        gu_set_param_50(0);
        gu_set_param_4c(0);
        gu_set_param_494(0x17);
        base = dl_get_cur_list_base();
        n = sceGuFinish();
        dl_set_list((unsigned char *)base + n - 8);
        ehf_set_frame_alias(g_game.loading_ehf, 0);
        ehf_set_anim(g_game.loading_ehf, 0, 0x35);
        g_game.overlay_enabled = 1;
    } else if (g_game.overlay_enabled) {
        g_game.overlay_enabled = 0;
    }
}

void loading_overlay_draw(void)
{
    unsigned int frame[24];
    void *ctx;

    memset(frame, 0, sizeof frame);
    frame[0] = *gfx_get_state();
    ctx = dl_select_layer(4, 0);
    ctx = gfx_begin_2d(ctx);
    ctx = ehf_draw(ctx, g_game.loading_ehf, frame);
    ctx = gfx_end_2d(ctx);
    dl_set_list(ctx);
}

void cfg_defaults_apply(void)
{
    cfg_set_byte0_bit3(1);
    cfg_set_byte0_bits4_5(0);
    cfg_set_bits_1_3_4(1);
    cfg_set_flag(0, 1);
    cfg_set_flag(2, 1);
    cfg_set_flag(3, 0);
    snd_set_volume(0, 12);
    snd_set_volume(1, 12);
    snd_set_volume(2, 12);
}

void loading_screen_init(void)
{
    ehp_mount(0, pack_chr_name);
    ehp_mount(0, pack_sysgfx);
    ehp_mount(0, pack_recipes);
    ehp_mount(0, pack_loading);
    ehp_mount(0, pack_sysmsg);
    memset(&g_game, 0, sizeof g_game);
    snd_channels_reset();
    g_game.heap = heap_create(game_heap_mem, 0x4000);
    rand_seed_from_rtc();
    g_game.rand_state = rand_next();
    g_game.loading_ehf = ehp_find(pack_loading, "loading.ehf", 0);
    lang_apply();
    cfg_defaults_apply();
}
