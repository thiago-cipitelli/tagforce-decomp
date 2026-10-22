// @range 0x08804000-0x08804AD0
// @data 0x08809290-0x08809664
/* Scene 9 "Labo Debug Menu": a development menu left in the final game that loads each sub-screen
   gmodule on demand (password machine, card shop, album, duelist list, ...) or opens the voice test. */
#include "psp.h"
#include "ehsys.h"

typedef int (*EhfileCallback)(void *arg, int result);

void gfx_overlay_enable(int on);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_is_done(void);
unsigned short region_pick_value(int flag);
int ehfile_request_module(int dir, const char *name, EhfileCallback cb, void *arg);
void scene_pop_to(unsigned short from, unsigned short arg);
void *memset(void *, int, unsigned int);
unsigned int *dl_clear_color(unsigned int *list, unsigned int color);
unsigned int *dl_clear_depth(unsigned int *list, unsigned int z);
unsigned int *dl_clear_stencil(unsigned int *list, unsigned int s);
unsigned int *dl_clear(unsigned int *list, unsigned int flags);
int font_set_layer(char layer);
unsigned int *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(unsigned int *list);
void font_set_size(int w, int h);
unsigned int font_set_color(unsigned int c);
void font_printf(int x, int y, const char *fmt, ...);
unsigned int *gfx_get_state(void);
int fade_apply(void **list);

/* sub-screen gmodules, imported by NID (stub names come from tools/link_map.py):
   init / update (nonzero = finished) / draw */
void password_init(void);
int password_update(void);
void password_draw(void);
void libselect_card_08804008(int a, int b, int c, int d);
int libselect_card_088041C4(void);
void libselect_card_08807630(void);
void libconv_machine_08804008(void);
int libconv_machine_088040BC(void);
void libconv_machine_08804B50(void);
void libcardalbum_08804008(int a, int b);
int libcardalbum_08804130(void);
void libcardalbum_08805CA4(void);
void libcharalist_08804830(int a, int b);
int libcharalist_08804A4C(void);
void libcharalist_08804F48(void);
void libduelrecord_08804008(int a);
int libduelrecord_088040FC(void);
void libduelrecord_08804690(void);
void liblimitlist_088060CC(int a, int b);
int liblimitlist_08806214(void);
void liblimitlist_08806490(void);
void libtutoriallist_08804008(int a, int b);
int libtutoriallist_08804154(void);
void libtutoriallist_088042F4(void);

void voicetest_load(void);
int voicetest_update(void);
void voicetest_draw(void);

typedef struct LaboState {
    int state;     /* 1 = menu, 2..9 = sub-screen (cursor + 2), 10 = voice test, 11 = leaving */
    int load;      /* 0 = loading the gmodule, 1 = loaded (call init), 2 = running */
    int cursor;    /* 0..8, 8 = voice test */
    int modid;     /* loaded gmodule */
    int unused[3];
} LaboState;

extern LaboState labo;            /* .bss 0x0890A624 */
extern char labo_scene_data[];    /* .bss 0x0880A600 */

static const char labo_module_names[9][32];
static void (*const labo_draw_table[12])(void);
static const char labo_menu_items[9][64];
static const char labo_title[16];
static const char labo_cursor_mark[3];

void labo_main(void);
void labo_exit(void);
void labo_draw(void);
void labo_draw_exit(void);
void labo_menu_draw(void);
int labo_module_loaded(void *arg, int modid);

int module_entry(void)
{
    ehscript_register((int (*)(void))labo_main, (int (*)(void))labo_exit, labo_scene_data);
    return 0;
}

void labo_main(void)
{
    PadState *pad;

    gfx_overlay_enable(0);
    sys_state_set_callbacks(9, (int (*)(void))labo_draw, (int (*)(void))labo_draw_exit);
    fade_start(1, 0xFF000000, 16);
    memset(&labo, 0, sizeof labo);
    labo.state = 1;
    for (;;) {
        sys_frame_wait();
        switch (labo.state) {
        case 1:
            pad = pad_get();
            if (pad->b.trig & region_pick_value(1)) {
                fade_start_in(1, 0xFF000000, 16);
                labo.state = 11;
            } else if (pad->b.trig & region_pick_value(0)) {
                int arg = 1;
                if (labo.cursor == 8) {
                    voicetest_load();
                    labo.state = 10;
                    labo.load = 2;
                } else {
                    labo.state = 0;
                    labo.load = 0;
                    gfx_overlay_enable(arg);
                    if (ehfile_request_module(2, labo_module_names[labo.cursor], labo_module_loaded, 0) == 0)
                        return;
                    while (labo.load == 0)
                        sys_frame_wait();
                    sceKernelStartModule(labo.modid, 4, &arg, 0, 0);
                    labo.state = labo.cursor + 2;
                }
            } else if (pad->b.dir_rep & 0x10) {
                labo.cursor = (labo.cursor + 8) % 9;
            } else if (pad->b.dir_rep & 0x40) {
                labo.cursor = (labo.cursor + 1) % 9;
            }
            break;
        case 2:
            switch (labo.load) {
            case 1:
                password_init();
                labo.load = 2;
                break;
            case 2:
                if (password_update()) {
                    sceKernelStopModule(labo.modid, 0, 0, 0, 0);
                    sceKernelUnloadModule(labo.modid);
                    fade_start(1, 0xFF000000, 16);
                    gfx_overlay_enable(0);
                    labo.state = 1;
                    labo.load = 0;
                    labo.modid = 0;
                }
                break;
            }
            break;
        case 3:
            switch (labo.load) {
            case 1:
                libselect_card_08804008(0, 0, 0, 0);
                labo.load = 2;
                break;
            case 2:
                if (libselect_card_088041C4()) {
                    sceKernelStopModule(labo.modid, 0, 0, 0, 0);
                    sceKernelUnloadModule(labo.modid);
                    fade_start(1, 0xFF000000, 16);
                    gfx_overlay_enable(0);
                    labo.state = 1;
                    labo.load = 0;
                    labo.modid = 0;
                }
                break;
            }
            break;
        case 4:
            switch (labo.load) {
            case 1:
                libconv_machine_08804008();
                labo.load = 2;
                break;
            case 2:
                if (libconv_machine_088040BC()) {
                    sceKernelStopModule(labo.modid, 0, 0, 0, 0);
                    sceKernelUnloadModule(labo.modid);
                    fade_start(1, 0xFF000000, 16);
                    gfx_overlay_enable(0);
                    labo.state = 1;
                    labo.load = 0;
                    labo.modid = 0;
                }
                break;
            }
            break;
        case 5:
            switch (labo.load) {
            case 1:
                libcardalbum_08804008(0, 0);
                labo.load = 2;
                break;
            case 2:
                if (libcardalbum_08804130()) {
                    sceKernelStopModule(labo.modid, 0, 0, 0, 0);
                    sceKernelUnloadModule(labo.modid);
                    fade_start(1, 0xFF000000, 16);
                    gfx_overlay_enable(0);
                    labo.state = 1;
                    labo.load = 0;
                    labo.modid = 0;
                }
                break;
            }
            break;
        case 6:
            switch (labo.load) {
            case 1:
                libcharalist_08804830(0, 0);
                labo.load = 2;
                break;
            case 2:
                if (libcharalist_08804A4C()) {
                    sceKernelStopModule(labo.modid, 0, 0, 0, 0);
                    sceKernelUnloadModule(labo.modid);
                    fade_start(1, 0xFF000000, 16);
                    gfx_overlay_enable(0);
                    labo.state = 1;
                    labo.load = 0;
                    labo.modid = 0;
                }
                break;
            }
            break;
        case 7:
            switch (labo.load) {
            case 1:
                libduelrecord_08804008(0);
                labo.load = 2;
                break;
            case 2:
                if (libduelrecord_088040FC()) {
                    sceKernelStopModule(labo.modid, 0, 0, 0, 0);
                    sceKernelUnloadModule(labo.modid);
                    fade_start(1, 0xFF000000, 16);
                    gfx_overlay_enable(0);
                    labo.state = 1;
                    labo.load = 0;
                    labo.modid = 0;
                }
                break;
            }
            break;
        case 8:
            switch (labo.load) {
            case 1:
                liblimitlist_088060CC(0, 0);
                labo.load = 2;
                break;
            case 2:
                if (liblimitlist_08806214()) {
                    sceKernelStopModule(labo.modid, 0, 0, 0, 0);
                    sceKernelUnloadModule(labo.modid);
                    fade_start(1, 0xFF000000, 16);
                    gfx_overlay_enable(0);
                    labo.state = 1;
                    labo.load = 0;
                    labo.modid = 0;
                }
                break;
            }
            break;
        case 9:
            switch (labo.load) {
            case 1:
                libtutoriallist_08804008(0, 0);
                labo.load = 2;
                break;
            case 2:
                if (libtutoriallist_08804154()) {
                    sceKernelStopModule(labo.modid, 0, 0, 0, 0);
                    sceKernelUnloadModule(labo.modid);
                    fade_start(1, 0xFF000000, 16);
                    gfx_overlay_enable(0);
                    labo.state = 1;
                    labo.load = 0;
                    labo.modid = 0;
                }
                break;
            }
            break;
        case 10:
            switch (labo.load) {
            case 1:
                voicetest_load();
                labo.load = 2;
                break;
            case 2:
                if (voicetest_update()) {
                    fade_start(1, 0xFF000000, 16);
                    gfx_overlay_enable(0);
                    labo.state = 1;
                    labo.load = 0;
                    labo.modid = 0;
                }
                break;
            }
            break;
        case 11:
            if (fade_is_done()) {
                sys_state_set_callbacks(0, 0, 0);
                gfx_overlay_enable(1);
                sys_frame_wait();
                scene_pop_to(9, 0);
                return;
            }
            break;
        }
    }
}

void labo_exit(void)
{
}

/* per-frame draw: the active screen's draw function, or a black clear while a gmodule loads */
void labo_draw(void)
{
    unsigned int *dl;

    if (labo_draw_table[labo.state]) {
        labo_draw_table[labo.state]();
    } else {
        dl = dl_select_layer(0, 0);
        dl = dl_clear_color(dl, 0xFF000000);
        dl = dl_clear_depth(dl, 0);
        dl = dl_clear_stencil(dl, 0);
        dl = dl_clear(dl, 0x17);
        dl_set_list(dl);
    }
}

void labo_draw_exit(void)
{
}

void labo_menu_draw(void)
{
    unsigned int *dl;
    int i;
    const char *item;
    int y;

    dl = dl_select_layer(0, 0);
    dl = dl_clear_color(dl, 0xFF404000);
    dl = dl_clear_depth(dl, 0);
    dl = dl_clear_stencil(dl, 0);
    dl = dl_clear(dl, 0x17);
    dl_set_list(dl);
    font_set_layer(4);
    font_set_size(20, 20);
    font_set_color(0xFFFFFFFF);
    font_printf(24, 2, labo_title);
    if ((*gfx_get_state() >> 3) & 1)  /* cursor blink */
        font_printf(0, labo.cursor * 24 + 48, labo_cursor_mark);
    for (i = 0, item = labo_menu_items[0], y = 48; i < 9; i++, item += 64, y += 24)
        font_printf(24, y, item);
    dl = dl_select_layer(4, 0);
    fade_apply((void **)&dl);
    dl_set_list(dl);
}

/* ehfile_request_module callback: the gmodule is loaded */
int labo_module_loaded(void *arg, int modid)
{
    int loaded = modid > 0;

    if (loaded) {
        labo.load = 1;
        labo.modid = modid;
    }
    return loaded;
}

static const char labo_module_names[9][32] = {
    "libpassword.prx", "libselect_card.prx", "libconv_machine.prx", "libcardalbum.prx",
    "libcharalist.prx", "libduelrecord.prx", "liblimitlist.prx", "libtutoriallist.prx", "",
};

static void (*const labo_draw_table[12])(void) = {
    0, labo_menu_draw, password_draw, libselect_card_08807630, libconv_machine_08804B50,
    libcardalbum_08805CA4, libcharalist_08804F48, libduelrecord_08804690, liblimitlist_08806490,
    libtutoriallist_088042F4, voicetest_draw, labo_menu_draw,
};

static const char labo_menu_items[9][64] = {
    "Password Machine", "Rental", "Card Convert Machine", "Card Album", "List of Duelists",
    "Duel Ranking", "Forbidden/Limited Card List", "Tutorial", "Japanese Only",
};

static const char labo_title[16] = "Labo Debug Menu";
static const char labo_cursor_mark[3] = "\x81\x9f";
