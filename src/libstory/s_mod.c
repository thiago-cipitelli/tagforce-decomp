// @range 0x0880FBE4-0x08810520
// @data 0x0881D608-0x0881D7B0
// @rodata 0x0881C510-0x0881C550
/* Sub-screen modules of the story: the table of the 12 PRX the story loads on demand (libfield,
   libshop and the sub-screens), the switch that unloads the others and loads one, and the
   init/update/term callbacks of each module (the update ones bring the story back to its states). */
#include "libstory.h"

typedef struct StMod {                 /* 0x0881D6F0 module of the story, 0x10 */
    char *name;
    void (*init)(void);
    void (*update)(void);
    void (*term)(void);
} StMod;

int st_mod_loaded(int id);
void st_mod_load(int id, char *name, void (*init)(void), void (*update)(void));
void st_mod_unload(int id, void (*term)(void));
void st_mod_clear_update(void);
void st_bgm_stop(void);
void st_fld_fade_out_wait(void);
void st_req_set_night(int v);
void st_req_set_x10(int v);
void st_req_set_exit(int v);
void st_req_set_lesson(int v);
int st_req_lesson(void);
int vm_var(int i);
int scal_get_wday(void);
void st_lesson_set_a(int v);
void st_lesson_set_b(int v);
void st_lesson_duel_start(void);

void shop_init(int mode, int arg);
int shop_update(void);
void shop_draw(void);
int shop_term(void);
int shop_load_poll(void);
void recipe_viewer_init(int mode, void *p);
int recipe_viewer_update(void);
void recipe_viewer_draw(void);
void password_init(void);
int password_update(void);
void password_draw(void);
void conv_init(void);
int conv_update(void);
void conv_draw(void);
void libselect_card_08804008(int mode, void *cfg, int *n, unsigned short *cards);
int libselect_card_088041C4(void);
void libselect_card_08807630(void);
void cardalbum_init(int mode, void *heap);
int cardalbum_update(void);
void cardalbum_draw(void);
void charalist_init(int mode, void *heap);
int charalist_update(void);
void charalist_draw(void);
void limitlist_init(int mode, void *heap);
int limitlist_update(void);
void limitlist_draw(void);
void tlist_init(int mode, void *heap);
int tlist_update(void);
int tlist_result(void);
void tlist_draw(void);
void duelrecord_init(void *heap);
int duelrecord_update(void);
void duelrecord_draw(void);
void dt_init(int id, void *p);
int dt_update(void);
void dt_draw(void);

void st_fieldmod_init(void);
void st_fieldmod_update(void);
void st_fieldmod_term(void);
void st_shopmod_init(void);
void st_shopmod_update(void);
void st_shopmod_term(void);
void st_recipe_init(void);
void st_recipe_update(void);
void st_password_init(void);
void st_password_update(void);
void st_selcard_init(void);
void st_selcard_update(void);
void st_conv_init(void);
void st_conv_update(void);
void st_album_init(void);
void st_album_update(void);
void st_chlist_init(void);
void st_chlist_update(void);
void st_limit_init(void);
void st_limit_update(void);
void st_tlist_init(void);
void st_tlist_update(void);
void st_dtut_init(void);
void st_dtut_update(void);
void st_record_init(void);
void st_record_update(void);

StMod st_mods[12] = {
    { "libfield.prx", st_fieldmod_init, st_fieldmod_update, st_fieldmod_term },
    { "libshop.prx", st_shopmod_init, st_shopmod_update, st_shopmod_term },
    { "librecipe_viewer.prx", st_recipe_init, st_recipe_update, 0 },
    { "libpassword.prx", st_password_init, st_password_update, 0 },
    { "libselect_card.prx", st_selcard_init, st_selcard_update, 0 },
    { "libconv_machine.prx", st_conv_init, st_conv_update, 0 },
    { "libcardalbum.prx", st_album_init, st_album_update, 0 },
    { "libcharalist.prx", st_chlist_init, st_chlist_update, 0 },
    { "liblimitlist.prx", st_limit_init, st_limit_update, 0 },
    { "libtutoriallist.prx", st_tlist_init, st_tlist_update, 0 },
    { "libdeck_tutorial.prx", st_dtut_init, st_dtut_update, 0 },
    { "libduelrecord.prx", st_record_init, st_record_update, 0 },
};

/* load != 0: unloads every other module (stopping the bgm unless the field keeps it) and loads id;
   load == 0: unloads id */
void st_mod_switch(int id, int load)
{
    int i;
    StMod *m;

    if (load) {
        for (i = 0, m = st_mods; i < 12; i++, m++) {
            if (id == i)
                continue;
            if (st_mod_loaded(i) == 0)
                continue;
            switch (id) {
            case 0:
                if (i != 11 && i != 6)
                    st_bgm_stop();
                break;
            case 6:
            case 11:
                break;
            default:
                st_bgm_stop();
                break;
            }
            st_mod_unload(i, m->term);
        }
        st_mod_load(id, st_mods[id].name, st_mods[id].init, st_mods[id].update);
    } else {
        st_mod_unload(id, st_mods[id].term);
    }
}

void st_fieldmod_init(void)
{
    fld_sys_init();
    sys_frame_wait();
    gfx_overlay_enable(1);
}

void st_fieldmod_update(void)
{
    fld_sys_update();
    fld_sys_draw();
}

void st_fieldmod_term(void)
{
    st_fld_fade_out_wait();
    st_mod_clear_update();
    sys_frame_wait();
    while (!fld_sys_term())
        sys_frame_wait();
    st_req_set_night(1);
}

/* the shop: script variable 0 picks the shop (0 = 0x1C, 0x1D = the other one) */
void st_shopmod_init(void)
{
    int kind;
    int mode;

    kind = vm_var(0);
    if (kind == 0)
        kind = 0x1C;
    switch (kind) {
    case 0x1C:
    default:
        mode = 1;
        break;
    case 0x1D:
        mode = 0;
        break;
    }
    shop_init(mode, (unsigned short)scal_get_wday());
    while (!shop_load_poll())
        sys_frame_wait();
}

void st_shopmod_update(void)
{
    StoryCtx *c = &story_ctx;

    if (shop_update()) {
        c->prev = 1;
        c->state = 0xF;
        st_mod_clear_update();
    } else {
        shop_draw();
    }
}

void st_shopmod_term(void)
{
    while (!shop_term())
        sys_frame_wait();
    st_req_set_x10(0);
}

void st_recipe_init(void)
{
    recipe_viewer_init(0, 0);
}

void st_recipe_update(void)
{
    StoryCtx *c = &story_ctx;

    switch (recipe_viewer_update()) {
    case -1:
        recipe_viewer_draw();
        break;
    default:
        c->prev = 2;
        st_req_set_x10(0);
        c->state = 4;
        st_mod_clear_update();
        break;
    }
}

void st_password_init(void)
{
    password_init();
}

void st_password_update(void)
{
    StoryCtx *c = &story_ctx;

    switch (password_update()) {
    case 0:
        password_draw();
        break;
    default:
        c->prev = 3;
        st_req_set_x10(0);
        c->state = 4;
        st_mod_clear_update();
        break;
    }
}

void st_selcard_init(void)
{
    libselect_card_08804008(0, 0, 0, 0);
}

void st_selcard_update(void)
{
    StoryCtx *c = &story_ctx;

    switch (libselect_card_088041C4()) {
    case 0:
        libselect_card_08807630();
        break;
    default:
        c->prev = 4;
        st_req_set_x10(0);
        c->state = 0x18;
        st_mod_clear_update();
        break;
    }
}

void st_conv_init(void)
{
    conv_init();
}

void st_conv_update(void)
{
    StoryCtx *c = &story_ctx;

    switch (conv_update()) {
    case 0:
        conv_draw();
        break;
    default:
        c->prev = 5;
        st_req_set_x10(0);
        c->state = 4;
        st_mod_clear_update();
        break;
    }
}

void st_album_init(void)
{
    cardalbum_init(1, 0);
}

void st_album_update(void)
{
    StoryCtx *c = &story_ctx;

    switch (cardalbum_update()) {
    case 0:
        cardalbum_draw();
        break;
    default:
        c->prev = 6;
        st_req_set_x10(0);
        c->state = 4;
        st_mod_clear_update();
        break;
    }
}

void st_chlist_init(void)
{
    charalist_init(1, 0);
}

void st_chlist_update(void)
{
    StoryCtx *c = &story_ctx;

    switch (charalist_update()) {
    case 0:
        charalist_draw();
        break;
    default:
        c->prev = 7;
        st_req_set_x10(0);
        c->state = 4;
        st_mod_clear_update();
        break;
    }
}

void st_limit_init(void)
{
    limitlist_init(1, 0);
}

/* 2: the duel lesson of the list was picked */
void st_limit_update(void)
{
    StoryCtx *c = &story_ctx;

    switch (limitlist_update()) {
    case 0:
        limitlist_draw();
        break;
    case 2:
        st_req_set_x10(0);
        c->state = 12;
        st_mod_clear_update();
        st_req_set_exit(3);
        break;
    default:
        c->prev = 8;
        st_req_set_x10(0);
        c->state = 4;
        st_mod_clear_update();
        break;
    }
}

void st_tlist_init(void)
{
    tlist_init(st_req_lesson(), 0);
}

/* lessons 0..7 are read in the story (state 40); 8..23 are duel lessons */
void st_tlist_update(void)
{
    StoryCtx *c = &story_ctx;
    int r;

    switch (tlist_update()) {
    case 0:
        tlist_draw();
        break;
    case 1:
        r = tlist_result();
        if (r >= 0 && r < 8) {
            st_req_set_lesson(r);
            c->state = 40;
        } else if (r >= 8 && r < 24) {
            st_req_set_lesson(r);
            switch (r) {
            default:
            case 8: r = 1; break;
            case 9: r = 2; break;
            case 10: r = 3; break;
            case 11: r = 4; break;
            case 12: r = 5; break;
            case 13: r = 6; break;
            case 14: r = 7; break;
            case 15: r = 8; break;
            case 16: r = 9; break;
            case 17: r = 10; break;
            case 18: r = 11; break;
            case 19: r = 12; break;
            case 20: r = 13; break;
            case 21: r = 14; break;
            case 22: r = 15; break;
            case 23: r = 16; break;
            }
            st_lesson_set_a(0);
            st_lesson_set_b(r);
            st_lesson_duel_start();
            c->state = 12;
            st_req_set_exit(7);
        } else {
            st_req_set_lesson(-1);
        }
        st_req_set_x10(0);
        st_mod_clear_update();
        break;
    default:
        c->prev = 9;
        st_req_set_x10(0);
        c->state = 4;
        st_mod_clear_update();
        break;
    }
}

void st_dtut_init(void)
{
    dt_init(st_req_lesson(), 0);
}

void st_dtut_update(void)
{
    StoryCtx *c = &story_ctx;

    switch (dt_update()) {
    case 0:
        dt_draw();
        break;
    case 1:
    default:
        c->prev = 10;
        st_req_set_x10(0);
        c->state = 0x25;
        st_mod_clear_update();
        break;
    }
}

void st_record_init(void)
{
    duelrecord_init(0);
}

void st_record_update(void)
{
    StoryCtx *c = &story_ctx;

    switch (duelrecord_update()) {
    case 0:
        duelrecord_draw();
        break;
    default:
        c->prev = 11;
        st_req_set_x10(0);
        c->state = 4;
        st_mod_clear_update();
        break;
    }
}
