// @range 0x08859824-0x08859DF0
/* Screen fades: a 16-byte state (type, colour, frame counter, duration, in/out flag) started by a
   scene and applied once per frame to the display list: alpha fade (1), quadratic alpha fade (2)
   or a diagonal wipe drawn as 2-pixel lines (3). */
#include "psp.h"
#include "libc.h"
#include "allegrex.h"

typedef struct FadeState {             /* 0x08BEA43C */
    unsigned char type : 6;            /* 0 none, 1 linear, 2 quadratic, 3 wipe */
    unsigned char hi : 2;
    unsigned char pad1[3];
    unsigned int color;                /* +4 ABGR; alpha scaled by the progress */
    unsigned short counter;            /* +8 frames elapsed */
    unsigned short duration;           /* +A frames (16 when 0 is passed) */
    unsigned char in : 2;              /* +C 1 fade in (progress counts down), 0 fade out */
    unsigned char pad2 : 6;
    unsigned char pad3[3];
} FadeState;

extern FadeState fade_state;
extern int rand3_slot;
extern unsigned int rand3_seeds[][3];

void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dl_tex_func(void *list, int func, int rgba);
void *dl_ambient_color(void *list, unsigned int color);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);

void *fade_begin(void *list, unsigned int color);
void *fade_end(void *list);
void *fade_fill(void *list, unsigned int color);
void *fade_wipe_line(void *list, int x0, int y, int x1);

void fade_start(unsigned char type, unsigned int color, unsigned short frames)
{
    memset(&fade_state, 0, sizeof(FadeState));
    fade_state.type = type;
    fade_state.color = color;
    if (frames == 0)
        fade_state.duration = 16;
    else
        fade_state.duration = frames;
    fade_state.in = 0;
}

void fade_start_in(unsigned char type, unsigned int color, unsigned short frames)
{
    memset(&fade_state, 0, sizeof(FadeState));
    fade_state.type = type;
    fade_state.color = color;
    if (frames == 0)
        fade_state.duration = 16;
    else
        fade_state.duration = frames;
    fade_state.in = 1;
}

/* Nonzero once the counter has passed the duration. */
int fade_is_done(void)
{
    return fade_state.counter > fade_state.duration;
}

void fade_finish(void)
{
    if (fade_state.counter < fade_state.duration)
        fade_state.counter = fade_state.duration + 1;
}

/* Draws the current fade step into *list and advances the counter; returns 1 when the fade is over. */
int fade_apply(void **list)
{
    int t = ALLEGREX_MIN(fade_state.counter, fade_state.duration);
    int a, w, i, y, x, j;
    unsigned int rgb, alpha;

    switch (fade_state.type) {
    case 1:
        a = (fade_state.color >> 24) & 0xFF;
        rgb = fade_state.color & 0xFFFFFF;
        if (fade_state.in == 0)
            t = fade_state.duration - t;
        alpha = (unsigned char)(a * t / fade_state.duration);
        *list = fade_fill(*list, (alpha << 24) | rgb);
        break;
    case 2:
        a = (fade_state.color >> 24) & 0xFF;
        rgb = fade_state.color & 0xFFFFFF;
        if (fade_state.in == 0)
            t = fade_state.duration - t;
        alpha = (unsigned char)(a * (t * t) / (fade_state.duration * fade_state.duration));
        *list = fade_fill(*list, (alpha << 24) | rgb);
        break;
    case 3:
        *list = fade_begin(*list, fade_state.color);
        if (fade_state.in == 0)
            t = fade_state.duration - t;
        w = t * 480 / fade_state.duration;
        rand3_seeds[rand3_slot][0] = 0;
        for (i = 0, y = 0; i < 136; i++, y += 2)
            *list = fade_wipe_line(*list, 480 - w, y, 480);
        for (j = 0, x = 0; j < 136; j++, x += 2)   /* separate variables: the register pair swaps (s0/s1) */
            *list = fade_wipe_line(*list, 0, 271 - x, w);
        *list = fade_end(*list);
        break;
    }
    if (fade_state.counter > fade_state.duration)
        return 1;
    fade_state.counter++;
    return 0;
}

void *fade_begin(void *list, unsigned int color)
{
    list = dl_disable(list, 9);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_tex_func(list, 4, 0);
    return dl_ambient_color(list, color);
}

void *fade_end(void *list)
{
    list = dl_enable(list, 9);
    list = dl_tex_func(list, 0, 1);
    return dl_ambient_color(list, 0xFFFFFFFF);
}

/* Full-screen quad in `color` (skipped when the alpha is 0). */
void *fade_fill(void *list, unsigned int color)
{
    if (((color >> 24) & 0xFF) == 0)
        return list;
    list = fade_begin(list, color);
    list = draw_set_offsets(list, 0, 0, 480, 272);
    list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
    return fade_end(list);
}

/* One-pixel-high line from x0 to x1 (clipped to the screen) at row y. */
void *fade_wipe_line(void *list, int x0, int y, int x1)
{
    if (x1 <= x0)
        return list;
    if (x0 >= 480)
        return list;
    if (x0 < 0)
        x0 = 0;
    if (480 < x1)
        x1 = 480;
    x1 -= x0;
    list = draw_set_offsets(list, 0, 0, x1, 1);
    return draw_sprite(list, x0, y, 0, 0, 0, 0, 0);
}
