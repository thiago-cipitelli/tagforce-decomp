// @range 0x088056EC-0x088062B4
// @data 0x0882E340-0x0882E3D4
/* Sub-screen modules started from the title menus: each one requests its PRX, the load callback
   starts it, then the state calls its init/update/draw exports until it returns, and goes back to
   the menu (librecipe_viewer, libcharalist, libcardalbum, libduelrecord, liblimitlist,
   libtutoriallist, libdeck_tutorial). */
#include "libtitle.h"

typedef struct TRvParams {             /* recipe_viewer_init argument */
    void *heap;
    void *cip;
    void *deck_pack;
    void (*on_delete)(int slot);
    void *on_select;
} TRvParams;

typedef struct TDtParams {             /* libdeck_tutorial_08804008 argument */
    void *heap;
    void *cip;
} TDtParams;

int title_recipe_loaded(void *arg, int id);
int title_chara_loaded(void *arg, int id);
int title_album_loaded(void *arg, int id);
int title_record_loaded(void *arg, int id);
int title_limit_loaded(void *arg, int id);
int title_tlist_loaded(void *arg, int id);
int title_dtut_loaded(void *arg, int id);
void title_limit_open(short state);
void title_tlist_open(short state, short arg);
void title_dtut_open(short state);

void title_recipe_open(void)
{
    title.sub = 0;
    title.step = 0;
    ehfile_request_module(2, "librecipe_viewer.prx", title_recipe_loaded, 0);
    gfx_overlay_enable(1);
}

int title_recipe_loaded(void *arg, int id)
{
    Title *const t = &title;

    if (title_module_start(arg, id)) {
        t->sub_mod = id;
        gfx_overlay_enable(0);
        t->sub = 1;
        t->sub_shown = 1;
        return 1;
    }
    return 0;
}

void title_recipe_update(void)
{
    Title *const t = &title;
    TRvParams p;

    switch (title.sub) {
    case 1:
        p.heap = t->heap;
        p.cip = 0;
        p.deck_pack = 0;
        recipe_viewer_init(0, &p);
        recipe_viewer_update();
        t->sub = 2;
        break;
    case 2:
        switch (recipe_viewer_update()) {
        case -1:
            break;
        case -2:
            t->sub_shown = 0;
            title_module_stop();
            t->arg = 1;
            title_set_state(9, 5);
            break;
        }
        break;
    }
}

void title_chara_open(void)
{
    title.sub = 0;
    title.step = 0;
    title.next_sub = 1;
    ehfile_request_module(2, "libcharalist.prx", title_chara_loaded, 0);
    gfx_overlay_enable(1);
}

int title_chara_loaded(void *arg, int id)
{
    Title *const t = &title;

    if (title_module_start(arg, id)) {
        t->sub = 1;
        t->sub_mod = id;
        gfx_overlay_enable(0);
        t->sub_shown = 1;
        return 1;
    }
    return 0;
}

void title_chara_update(void)
{
    Title *const t = &title;

    switch (title.sub) {
    case 1:
        charalist_init(0, t->heap);
        charalist_update();
        t->sub = 2;
        break;
    case 2:
        switch (charalist_update()) {
        case 0:
            break;
        case 1:
        case 2:
            t->sub_shown = 0;
            title_module_stop();
            title_set_state(9, 6);
            break;
        }
        break;
    }
}

/* picking a partner/opponent for the free duel (states 0x1D, 0x1E) */
int title_chara_pick_update(int state)
{
    Title *const t = &title;
    int mode = 0;

    switch (title.sub) {
    case 1:
        mode = 0;
        if (state == 29)
            mode = 3;
        else if (state == 30)
            mode = 4;
        charalist_init(mode, t->heap);
        charalist_update();
        t->sub = 2;
        break;
    case 2:
        switch (charalist_update()) {
        case 0:
            break;
        case 1:
            if (state == 29)
                mode = 2;
            else if (state == 30)
                mode = 3;
            title_story_start(mode, charalist_result_id());
            title_module_stop();
            return 40;
        case 2:
            t->sub_shown = 0;
            title_module_stop();
            title_set_state(9, 3);
            break;
        }
        break;
    }
    return state;
}

void title_album_open(void)
{
    title.sub = 0;
    title.step = 0;
    title.next_sub = 1;
    ehfile_request_module(2, "libcardalbum.prx", title_album_loaded, 0);
    gfx_overlay_enable(1);
}

int title_album_loaded(void *arg, int id)
{
    Title *const t = &title;

    if (title_module_start(arg, id)) {
        t->sub_mod = id;
        t->sub = t->next_sub;
        return 1;
    }
    return 0;
}

void title_album_update(void)
{
    Title *const t = &title;

    switch (title.sub) {
    case 0:
        goto done;
    case 1:
        cardalbum_init(0, t->heap);
        t->sub++;
        break;
    }
    if (cardalbum_update() == 0) {
        t->sub_shown = 1;
    } else {
        t->sub_shown = 0;
        title_module_stop();
        t->arg = 1;
        title_set_state(9, 6);
    }
done:
    t->prev_sub = t->sub;
}

void title_album_draw(void)
{
    if (title.sub_shown)
        cardalbum_draw();
}

void title_record_open(void)
{
    title.sub = 0;
    title.step = 0;
    title.next_sub = 1;
    ehfile_request_module(2, "libduelrecord.prx", title_record_loaded, 0);
    gfx_overlay_enable(1);
}

int title_record_loaded(void *arg, int id)
{
    Title *const t = &title;

    if (title_module_start(arg, id)) {
        t->sub_mod = id;
        t->sub = t->next_sub;
        return 1;
    }
    return 0;
}

void title_record_update(void)
{
    Title *const t = &title;

    switch (title.sub) {
    case 0:
        goto done;
    case 1:
        duelrecord_init(t->heap);
        t->sub++;
        break;
    }
    if (duelrecord_update() == 0) {
        t->sub_shown = 1;
    } else {
        t->sub_shown = 0;
        title_module_stop();
        t->arg = 3;
        title_set_state(9, 6);
    }
done:
    t->prev_sub = t->sub;
}

void title_record_draw(void)
{
    if (title.sub_shown)
        duelrecord_draw();
}

void title_limit_open(short state)
{
    title.state = state;
    title.sub = 0;
    title.step = 0;
    title.next_sub = 1;
    ehfile_request_module(2, "liblimitlist.prx", title_limit_loaded, 0);
    gfx_overlay_enable(1);
}

int title_limit_loaded(void *arg, int id)
{
    Title *const t = &title;

    if (title_module_start(arg, id)) {
        t->sub_mod = id;
        t->sub = t->next_sub;
        return 1;
    }
    return 0;
}

int title_limit_update(void)
{
    Title *const t = &title;

    switch (title.sub) {
    case 0:
        goto done;
    case 1:
        limitlist_init(0, t->heap);
        t->sub++;
        break;
    }
    switch (limitlist_update()) {
    case 0:
        t->sub_shown = 1;
        break;
    case 2:
        t->sub_shown = 0;
        title_module_stop();
        title_set_state(20, 2);
        t->prev_sub = t->sub;
        return 40;
    case 1:
        t->sub_shown = 0;
        title_module_stop();
        t->arg = 4;
        title_set_state(9, 6);
        break;
    }
done:
    t->prev_sub = t->sub;
    return t->state;
}

void title_limit_draw(void)
{
    if (title.sub_shown)
        limitlist_draw();
}

void title_tlist_open(short state, short arg)
{
    Title *t = &title;

    title.state = state;
    title.sub = 0;
    title.step = 0;
    title.next_sub = 1;
    if (arg < 0)
        t->arg = (int)arg;
    else
        t->arg = (unsigned int)arg;

    ehfile_request_module(2, "libtutoriallist.prx", title_tlist_loaded, 0);
    gfx_overlay_enable(1);
}

int title_tlist_loaded(void *arg, int id)
{
    Title *const t = &title;

    if (title_module_start(arg, id)) {
        t->sub_mod = id;
        t->sub = t->next_sub;
        return 1;
    }
    return 0;
}

int title_tlist_update(int state)
{
    Title *const t = &title;

    switch (title.sub) {
    case 0:
        break;
    case 1:
        tlist_init(t->arg, t->heap);
        t->sub++;
    case 2:
        switch (tlist_update()) {
        case 0:
            t->sub_shown = 1;
            break;
        case 1:
            t->sub_shown = 0;
            state = tlist_result();
            t->arg = state;
            title_module_stop();
            if (state < 8)
                state = title_set_state(39, 0);
            else
                state = title_set_state(38, 0);
            break;
        case 2:
            t->sub_shown = 0;
            title_module_stop();
            t->arg = 2;
            title_set_state(9, 6);
            break;
        }
        break;
    case 3:
        break;
    }
    t->prev_sub = t->sub;
    return state;
}

void title_tlist_draw(void)
{
    if (title.sub_shown)
        tlist_draw();
}

void title_dtut_open(short state)
{
    title.state = state;
    title.sub = 0;
    title.step = 0;
    title.next_sub = 1;
    ehfile_request_module(2, "libdeck_tutorial.prx", title_dtut_loaded, 0);
    gfx_overlay_enable(1);
}

int title_dtut_loaded(void *arg, int id)
{
    Title *const t = &title;

    if (title_module_start(arg, id)) {
        t->sub_mod = id;
        t->sub = t->next_sub;
        return 1;
    }
    return 0;
}

void title_dtut_update(void)
{
    Title *const t = &title;
    TDtParams p;

    switch (title.sub) {
    case 0:
        goto done;
    case 1:
        p.heap = t->heap;
        p.cip = 0;
        libdeck_tutorial_08804008(t->arg, &p);
        t->sub++;
        break;
    }
    if (libdeck_tutorial_0880418C() == 0) {
        t->sub_shown = 1;
    } else {
        t->sub_shown = 0;
        title_module_stop();
        title_set_state(24, (short)t->arg);
    }
done:
    t->prev_sub = t->sub;
}

void title_dtut_draw(void)
{
    if (title.sub_shown)
        libdeck_tutorial_08804554();
}
