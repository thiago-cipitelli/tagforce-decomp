// @range 0x08810520-0x08810AE4
// @data 0x0881D7B0-0x0881D7E8
/* Odds and ends of the story mode: item gain routing, what follows the free walk (dorm, store, the
   tournament), the names of the crowd extras, the room-link callbacks, and the end of the story: the
   staff roll movie (its own 5 MB HeapMovie pool, with the duel modules unloaded) and the clear save. */
#include "libstory.h"

int st_pl_room_get(void);
void st_pl_item_add(int a, int b, int c, int d);
void st_pickup_take(int a, int b, int c, int d);
int story_mode_is1(void);
int story_mode_get(void);
int sdate_get_0c(void);
int st_free_duel_ok(void);
int st_sv_alt(void);
int st_ev_run(int id);
void st_req_set_exit(int v);
void st_snd_stop_all(void);
void st_msg_unload(int i);
void st_mod_switch(int id, int load);
void st_fade(int out, int wait, int frames);
void *st_save_buf(void);
int cfg_is_mode1(void);
int charmap_lookup(int c, int flag);
int duel_modules_load();
int duel_modules_unload(void);
int modmgr_none_loading_alias(void);
void movie_set_memory(unsigned int base, unsigned int size);
void movie_play(const char *path, unsigned int flags, int arg);
int ehfile_make_path_alias(char *out, int dir, const char *name);
void sysflag_apply(unsigned int which, unsigned short v);
int msd_open(int mode, void *heap);
int msd_update(void);
void msd_update_dialog(void);
int sceKernelFreeFpl(int fpl, void *data);
int sceKernelDeleteFpl(int fpl);

void st_movie_alloc(void);
void st_movie_wait_load(void);
void st_movie_play(void);

/* item gained: kind 1 comes from a pickup of the map */
void st_fld_item_take(int kind, int a, int b, int c, int d)
{
    if (kind != 1)
        st_pl_item_add(a, b, c, d);
    else
        st_pickup_take(a, b, c, d);
}

/* what the end of the free walk leads to: 3 store, 2 back to the dorm, 1/4 events of the tournament */
int st_after_free(void)
{
    switch (st_pl_room_get()) {
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
        return 2;
    case 27:
        return 3;
    }
    if (story_mode_is1() == 0)
        return 0;
    switch (sdate_get_0c()) {
    case 9:
        if (st_free_duel_ok() == 0)
            return 0;
        if (st_sv_alt() != 0)
            return 0;
        if (cfg_is_mode1()) {
            st_ev_run(0x100);
            return 4;
        }
        if (st_ev_run(0x101))
            return 4;
        return 0;
    case 73:
        return 1;
    }
    return 0;
}

/* random name of a crowd extra, never twice the same in a row */
int st_crowd_pick_name(int *last)
{
    int c;
    int name;

    c = (int)rand_next() % 95 + 41;
    name = charmap_lookup(c, 0);
    if (*last == name)
        name = charmap_lookup(c + 1, 0);
    return *last = name;
}

void st_fld_on_link(void)
{
    st_req_set_exit(9);
}

void st_fld_on_back(int v)
{
    if (v == 0)
        st_req_set_exit(0);
}

/* staff roll: unloads everything, then the steps below run one per frame */
void st_staffroll_start(void)
{
    StoryCtx *c = &story_ctx;
    int i;

    st_snd_stop_all();
    st_msg_unload(1);
    st_msg_unload(2);
    st_msg_unload(0);
    for (i = 0; i < 12; i++)
        st_mod_switch(i, 0);
    c->state = 0x31;
    c->step = st_movie_alloc;
}

void st_movie_alloc(void)
{
    story_ctx.movie_fpl = sceKernelCreateFpl("HeapMovie", 2, 0, 0x500000, 1, 0);
    sceKernelAllocateFpl(story_ctx.movie_fpl, &story_ctx.movie_mem, 0);
    duel_modules_load(0, 1);
    story_ctx.step = st_movie_wait_load;
}

void st_movie_wait_load(void)
{
    StoryCtx *c = &story_ctx;

    if (modmgr_none_loading_alias()) {
        movie_set_memory((unsigned int)c->movie_mem, 0x500000);
        c->step = 0;
        c->state = 0x30;
    }
}

void st_staffroll_play(void)
{
    st_fade(1, 1, 0);
    gfx_overlay_enable(0);
    story_ctx.state = 0x31;
    story_ctx.step = st_movie_play;
}

void st_movie_play(void)
{
    StoryCtx *c = &story_ctx;
    char *name = 0;
    char path[0x100];

    switch (region_get()) {
    case 0:
        name = "movie/staffroll.pmf";
        break;
    case 1:
        name = "movie/staffroll_a.pmf";
        break;
    }
    if (name == 0)
        return;
    ehfile_make_path_alias(path, 3, name);
    movie_play(path, 1, 0x40);
    duel_modules_unload();
    sceKernelFreeFpl(c->movie_fpl, c->movie_mem);
    sceKernelDeleteFpl(c->movie_fpl);
    st_req_set_exit(1);
    c->step = 0;
    c->state = 0x32;
}

/* clear save: unloads everything and opens the save dialog with the cleared stage */
void st_ending_save(void)
{
    StoryCtx *c = &story_ctx;
    int i;
    int stage;

    st_snd_stop_all();
    st_msg_unload(1);
    st_msg_unload(2);
    st_msg_unload(0);
    for (i = 0; i < 12; i++)
        st_mod_switch(i, 0);
    if (msd_open(5, st_save_buf()) == 0)
        c->state = 0x34;
    switch (stage = story_mode_get()) {
    case 3:
    default:
        stage = 1;
        break;
    case 1:
    case 2:
        stage = stage + 1;
        break;
    }
    sysflag_apply(stage, 0);
    gfx_overlay_enable(0);
    st_fade(1, 1, 0);
}

void st_ending_save_poll(void)
{
    StoryCtx *c = &story_ctx;

    switch (msd_update()) {
    case 1:
        break;
    case 2:
    case 3:
        c->state = 0x34;
        break;
    }
    msd_update_dialog();
}

void st_ending_save_done(void)
{
    st_fade(0, 1, 0);
    gfx_overlay_enable(1);
    st_req_set_exit(1);
}
