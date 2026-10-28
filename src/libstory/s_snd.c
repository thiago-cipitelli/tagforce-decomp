// @range 0x08817930-0x08817E64
// @data 0x0881D194-0x0881D1E0
/* Story music and sounds: the bgm wanted/playing/volume words, play (stopping the previous one),
   stop and stop-all with their waits, the bgm of the current room (by day, room, night exit or the
   duel just played) and the sound effect of a point delta. */
#include "libstory.h"
#include "duel_setup.h"

typedef struct StBgm {                 /* *st_bgm, 0xC */
    int want;                          /* +0x0 bgm requested (-1 none) */
    int cur;                           /* +0x4 bgm playing (-1 none) */
    int vol;                           /* +0x8 */
} StBgm;

extern StBgm *st_bgm;                  /* 0x08ACBB20 */
extern int st_snd_x;                   /* 0x08ACBB24 */

void bgm_fade(int x);
void se_play(int id);
void snd_load_data(void);
unsigned int bgm_get_status(int id);
void bgm_play(int id);
void snd_cmd_fefb_arg(int arg);
int snd_count_bgm(void);
int snd_count_se(void);
void snd_cmd_fefb(void);
void bgm_cmd(int arg);
void bgm_stop(int x);
void bgm_cmd3(int x);
void bgm_set_pan(int x, int t);
void *game_state2_get(void);
int st_req_is_night(void);
int st_req_exit(void);
int st_sv_alt(void);
int st_sv_get_14(void);
int sdate_get_0c(void);
int st_pl_room_get(void);
int st_remap_8124b0(int id);
int st_remap_81259c(int id);
int st_duel_param5(void);
int st_duel_param6(void);
int st_duel_param7(void);

void st_bgm_play(int id);
void st_bgm_stop(void);
void st_bgm_room(void);
void st_bgm_volume(int v);

void st_init_817930(void)
{
    memset(&st_snd_x, 0, sizeof(st_snd_x));
}

void st_term_817944(void)
{
    memset(&st_snd_x, 0, sizeof(st_snd_x));
}

void st_bgm_attach(void *p)
{
    st_bgm = p;
}

void st_bgm_clear(void)
{
    StBgm *b = st_bgm;

    memset(b, 0, sizeof(StBgm));
    b->want = -1;
    b->cur = -1;
}

void st_bgm_fade(void)
{
    bgm_fade(330);
    bgm_fade(331);
    bgm_fade(333);
    bgm_fade(334);
    bgm_fade(335);
    bgm_fade(328);
    bgm_fade(329);
}

void st_se_play(int id)
{
    se_play(id);
}

/* sound of a point change: three pitches down, three up */
void se_play_delta(int d)
{
    int id;

    if (d < -49)
        id = 0x6222;
    else if (d < -24)
        id = 0x6223;
    else if (d <= -1)
        id = 0x6224;
    else if (d < 25)
        id = 0x6221;
    else if (d < 50)
        id = 0x6220;
    else
        id = 0x621F;
    st_se_play(id);
}

void st_snd_load(void)
{
    snd_load_data();
}

void st_bgm_play(int id)
{
    StBgm *b = st_bgm;

    if (b->want != -1) {
        st_snd_load();
        st_bgm_stop();
    }
    bgm_fade(id);
    while (bgm_get_status(id))
        sys_frame_wait();
    b->want = id;
    b->cur = id;
    bgm_play(id);
}

void st_snd_stop_all(void)
{
    StBgm *b = st_bgm;

    snd_cmd_fefb_arg(1500);
    while (snd_count_bgm() + snd_count_se() != 0)
        sys_frame_wait();
    b->cur = -1;
    snd_cmd_fefb();
}

void st_bgm_stop(void)
{
    StBgm *b = st_bgm;

    if (b->cur == -1)
        return;
    bgm_cmd(1500);
    while (snd_count_bgm())
        sys_frame_wait();
    bgm_stop(b->cur);
    while (snd_count_bgm())
        sys_frame_wait();
    bgm_cmd3(b->cur);
    while (bgm_get_status(b->cur))
        sys_frame_wait();
    b->cur = -1;
}

void st_bgm_nop(void)
{
    snd_cmd_fefb_arg(1500);
}

void st_bgm_play_24(void)
{
    st_bgm_play(24);
}

/* bgm of the place the story is in */
void st_bgm_play_room(void)
{
    StBgm *b = st_bgm;
    int id;

    if (st_req_is_night()) {
        switch (st_req_exit()) {
        case 4:
            switch (((DuelResults *)game_state2_get())->result) {
            case 1:
            default:
                id = st_duel_param5();
                break;
            case 2:
                id = st_duel_param6();
                break;
            case 0:
            case 3:
                id = st_duel_param7();
                break;
            }
            if (id > 0) {
                st_bgm_play(st_remap_8124b0(id));
                return;
            }
            break;
        case 0:
        case 1:
        case 2:
        case 3:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            break;
        }
        if (b->cur == -1)
            st_bgm_play(b->want);
        st_bgm_volume(b->vol);
    } else if (st_sv_alt() && (id = st_sv_get_14(), id >= 0)) {
        if (b->cur != id)
            st_bgm_play(id);
    } else
        st_bgm_room();
}

/* bgm and volume of the current room (lower in rooms 7..15) */
void st_bgm_room(void)
{
    StBgm *b = st_bgm;
    int id;
    int vol = 0x80;

    if (sdate_get_0c() >= 66)
        id = 25;
    else
        id = st_remap_81259c(st_pl_room_get());
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
        vol = 0x60;
        break;
    }
    if (b->cur != id)
        st_bgm_play(id);
    st_bgm_volume(vol);
}

void st_bgm_volume(int v)
{
    st_bgm->vol = v;
    bgm_set_pan(v - 1, 1500);
}
