// @range 0x08827F68-0x088282E4
// @asm 0x08827F68-0x08828198
// @data 0x08876508-0x0887653C
/* Sound cues of the effect scripts: play a sound effect (or stop/fade the music) once when the
   frame counter reaches a given frame, tracked by a bit of a flags word; a random hit sound and
   the speed tables of the duel animation setting. */
#include "dd_d3.h"

typedef struct DdRandSeed {            /* BOOT rand3_seeds entry, 12 bytes */
    unsigned int lcg;
    unsigned int b, c;
} DdRandSeed;

extern int rand3_slot;
extern DdRandSeed rand3_seeds[];

int se_play(int id);
int se_play_at(int id, int pan);
int bgm_stop(int x);
int bgm_fade_out(int x, int ms);
unsigned int cfg_get_bits_1_3_4(void);

#ifdef NON_MATCHING /* o original materializa (*flags & bit) != 0 com sltu antes do bnezl; nenhuma forma do teste reproduziu */
int dd_cue_se(int id, int now, int at, unsigned int *flags, unsigned int bit)
{
    if (at < 0)
        return 0;
    if (!(*flags & bit) && now >= at) {
        *flags |= bit;
        return se_play(id);
    }
    return 0;
}

int dd_cue_se_f(int id, float now, float at, unsigned int *flags, unsigned int bit)
{
    if (at < 0.0f)
        return 0;
    if (!(*flags & bit) && now >= at) {
        *flags |= bit;
        return se_play(id);
    }
    return 0;
}

int dd_cue_se_at(int id, int now, int at, int frames, unsigned int *flags, unsigned int bit)
{
    if (at < 0)
        return 0;
    if (!(*flags & bit) && now >= at) {
        *flags |= bit;
        return se_play_at(id, frames * 1000 / 30);
    }
    return 0;
}

int dd_cue_bgm_stop(int x, int now, int at, unsigned int *flags, unsigned int bit)
{
    if (at < 0)
        return 0;
    if (!(*flags & bit) && now >= at) {
        *flags |= bit;
        return bgm_stop(x);
    }
    return 0;
}

int dd_cue_bgm_fade(int x, int now, int at, int frames, unsigned int *flags, unsigned int bit)
{
    if (at < 0)
        return 0;
    if (!(*flags & bit) && now >= at) {
        *flags |= bit;
        return bgm_fade_out(x, frames * 1000 / 30);
    }
    return 0;
}
#endif

int dd_rand_hit_se(void)
{
    int ids[5] = { 0x623D, 0x623E, 0x623F, 0x6240, 0x6241 };
    DdRandSeed *r = &rand3_seeds[rand3_slot];

    r->lcg *= 0x41C64E6D;
    r->lcg += 0x3039;
    return ids[(int)(r->lcg >> 16 & 0x7FFF) % 5];
}

float dd_anim_speed(void)
{
    float speed[4] = { 0.5f, 1.0f, 2.0f, 3.0f };

    return speed[cfg_get_bits_1_3_4()];
}

int dd_anim_step(void)
{
    int step[4] = { 1, 1, 2, 2 };

    return step[cfg_get_bits_1_3_4()];
}
