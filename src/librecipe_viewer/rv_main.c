// @range 0x0880A248-0x0880A99C
// @rodata 0x0880E544-0x0880E564
// @data 0x0880E8D4-0x0880E8D8
/* Entry points of the recipe viewer sub-screen (init / per-frame update): working heap, the lists
   of existing recipes and saved decks, the load -> fade in -> list / recipe / menu -> fade out ->
   release state machine and the result (selected recipe). */
#include "librecipe_viewer.h"
#include "game.h"
#include "card_db.h"

DeckName *deck_name_get(int i);
void extdata_open(void *heap);
void pad_set_repeat_from_fps(void);
void rv_build_list(RecipeData *d, RvRecipe *r);
void rv_counter_step(void);
int rv_name_update(void);
void rv_info_update(void);
void rv_load_lists(void);
void rv_set_title_anim(void);
void rv_set_tab_anim(void);
void rv_select(void);
void rv_input_list(void);
void rv_input_recipe(void);
void rv_input_menu(void);
int recipe_viewer_release(void);

int module_start(void)
{
    return 0;
}

/* 0x0880A250: export */
void recipe_viewer_init(int mode, RvParams *p)
{
    int i;
    int n;
    RecipeData rd;

    memset(&rv, 0, sizeof(rv));
    if (p) {
        rv.heap = p->heap;
        rv.cip = p->cip;
        rv.deck_pack = p->deck_pack;
        rv.on_delete = p->on_delete;
        rv.on_select = p->on_select;
    }
    rv.mode = mode;
    if (rv.heap == 0) {
        rv.fpl = sceKernelCreateFpl("", 2, 0, 0x400000, 1, 0);
        sceKernelAllocateFpl(rv.fpl, &rv.fpl_mem, 0);
        rv.heap = heap_create(rv.fpl_mem, 0x400000);
    }
    n = 0;
    rv.n_recipes = 0;
    for (i = 1; i < 351; i++) {
        DeckName *d = deck_name_get(i);
        if (recipe_get_id(i)) {
            n++;
            if (d->set) {
                RvRecipe *r = &rv.recipes[rv.n_recipes++];
                r->index = i;
                r->number = n;
                r->kind = d->val;
                recipe_load(i, &rd);
                rv_build_list(&rd, r);
            }
        }
    }
    for (i = 0; i < 50; i++) {
        SaveRec *s = sysstate_rec_get(i);
        RvRecipe *r = &rv.decks[i];
        memset(r, 0, sizeof(*r));
        if (s->used) {
            r->index = i;
            rv_build_list(&s->recipe, r);
        }
    }
    rvres_init();
    if (mode == 0) {
        snd_cmd_fefb();
        bgm_fade(330);
    }
    font_set_flag4(1);
    font_set_flag2(1);
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
        font_set_flag8(0);
    } else {
        font_set_flag1(0);
        font_set_flag8(1);
    }
    if (rv.mode == 1)
        rv.state = 1;
    else
        extdata_open(rv.heap);
}

/* 0x0880A520: export; -1 while running, -2 on cancel, else the selection */
int recipe_viewer_update(void)
{
    if (rv.busy)
        return -1;
    if (rv.info) {
        rv_info_update();
        return -1;
    }
    rv_counter_step();
    if (rv.osk) {
        if (rv_name_update()) {
            fade_start_in(1, 0xFF000000, 16);
            rv.state = 6;
            if (rv.mode == 0)
                bgm_cmd(600);
        }
        return -1;
    }
    switch (rv.state) {
    case 0:
        rv_load_lists();
        if (!rv.loaded)
            rv.loaded = rvres_poll();
        break;
    case 1:
        if (!rv.loaded && !rvres_poll())
            break;
        rv_set_title_anim();
        rv_set_tab_anim();
        rv_select();
        rv.state = 2;
        fade_start(1, 0xFF000000, 16);
        bgm_play(6);
        break;
    case 2:
        if (fade_is_done())
            rv.state = 3;
        break;
    case 3:
        rv_input_list();
        break;
    case 4:
        rv_input_recipe();
        break;
    case 5:
        rv_input_menu();
        break;
    case 6:
        if (fade_is_done())
            rv.state = 7;
        break;
    case 7:
        if (recipe_viewer_release()) {
            if (!rv.cancelled) {
                unsigned int t = rv.tab;
                RvPos *p = &((RecipeViewer *)(char *)&rv)->pos[t];
                int v = p->top + p->cursor;

                if (t == 1)
                    v = rv.recipes[v].index;
                switch (rv.mode) {
                case 1:
                    return v;
                case 2:
                    return (unsigned short)v | t << 16;
                }
            }
            return -2;
        }
        break;
    }
    return -1;
}

int recipe_viewer_release(void)
{
    switch (rv.step) {
    case 0:
        switch (rv.mode) {
        default:
            cardlist_clear();
            cip_close(rv.cip);
            rv.step++;
            break;
        case 2:
        case 1:
            rv.step = 2;
            break;
        }
        break;
    case 1:
        if (cip_is_idle(rv.cip))
            rv.step++;
        break;
    case 2:
        switch (rv.mode) {
        case 2:
        case 1:
            break;
        default:
            rvres_free();
            break;
        }
        if (rv.ext) {
            heap_set_level(rv.heap, rv.ext);
            rv.ext = 0;
        }
        if (rv.fpl > 0) {
            sceKernelFreeFpl(rv.fpl, rv.fpl_mem);
            sceKernelDeleteFpl(rv.fpl);
        }
        pad_set_repeat_from_fps();
        return 1;
    }
    return 0;
}
