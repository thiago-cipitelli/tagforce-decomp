// @range 0x08839A8C-0x08839F50
// @asm 0x08839AC8-0x08839CFC
// @rodata 0x08875390-0x0887539C
/* Controller: one sample per frame (the previous one is reused when reading fails), edge and
   auto-repeat masks per button, the stick folded into d-pad bits with its own repeat, repeat
   timings scaled to the frame rate, and the stick (or d-pad) direction as an angle. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"

typedef struct GfxState {
    unsigned int frame;
    int fps;
} GfxState;

extern PadState pad_state;             /* 0x08905534 */
extern signed char pad_repeat_rate;    /* 0x08905530 */
extern signed char pad_repeat_delay;   /* 0x08905531 */
extern signed char pad_repeat_count[32]; /* 0x08905510 */

int sceCtrlSetSamplingMode(int mode);
int sceCtrlPeekBufferPositive(PadCtrl *data, int count);
GfxState *gfx_get_state(void);
int int_abs(int x);
float atan2f(float y, float x);
float nanf_value(void);

unsigned int pad_repeat(unsigned int down);
void pad_set_repeat_from_fps(void);

int pad_init(void)
{
    memset(&pad_state, 0, sizeof(pad_state));
    sceCtrlSetSamplingMode(1);
    pad_set_repeat_from_fps();
    return 1;
}

#ifdef NON_MATCHING /* the original keeps the new state in a stack struct copied with lwc1/swc1; mwcc 3.0.1 keeps it in registers */
int pad_update(void)
{
    PadCtrl ctrl;
    PadBits p;

    if (sceCtrlPeekBufferPositive(&ctrl, 1) <= 0) {
        ctrl = pad_state.last;
    }
    if (pad_state.ignore_hold != 0 && (ctrl.buttons & 0x20000)) {
        ctrl.ly = 0x80;
        ctrl.lx = 0x80;
    }
    p.analog_x = ctrl.lx;
    p.analog_y = ctrl.ly;
    p.release = ~ctrl.buttons & pad_state.b.down;
    p.trig = ctrl.buttons & ~pad_state.b.down;
    p.down = ctrl.buttons;
    p.rep = p.trig | pad_repeat(p.down);
    p.dir = p.down;
    if (p.analog_x < 0x21) {
        p.dir = p.down | 0x80;
    } else if (p.analog_x >= 0xDF) {
        p.dir = p.down | 0x20;
    }
    if (p.analog_y < 0x21) {
        p.dir |= 0x10;
    } else if (p.analog_y >= 0xDF) {
        p.dir |= 0x40;
    }
    p.dir_trig = p.dir & ~pad_state.b.dir;
    p.dir_release = ~p.dir & pad_state.b.dir;
    if (p.dir == 0) {
        p.dir_rep = 0;
        pad_state.dir_count = 0;
    } else if (pad_state.dir_count == 0) {
        p.dir_rep = p.dir;
        pad_state.dir_count = 1;
    } else if (p.dir != pad_state.b.dir) {
        p.dir_rep = p.dir;
        pad_state.dir_count = 1;
    } else {
        if (pad_state.dir_count < 0xF) {
            p.dir_rep = 0;
        } else if ((pad_state.dir_count & 3) == 0) {
            p.dir_rep = p.dir;
        } else {
            p.dir_rep = 0;
        }
        pad_state.dir_count++;
    }
    pad_state.b = p;
    pad_state.last = ctrl;
    return 1;
}
#endif

PadState *pad_get(void)
{
    return &pad_state;
}

/* Buttons whose repeat counter elapsed this frame (first repeat after the delay). */
unsigned int pad_repeat(unsigned int down)
{
    unsigned int out;
    signed char rate;
    unsigned int bit;
    int start;
    unsigned int i;
    signed char *c;

    out = 0;
    rate = pad_repeat_rate;
    i = 0;
    bit = 1;
    start = pad_repeat_delay - pad_repeat_rate;
    c = pad_repeat_count;
    do {
        if (!(down & bit)) {
            *c = -start;
        } else if (++*c >= rate) {
            *c = 0;
            out |= bit;
        }
        i++;
        c++;
        bit <<= 1;
    } while (i < 32);
    return out;
}

void pad_set_repeat_from_fps(void)
{
    int fps = gfx_get_state()->fps;

    pad_repeat_delay = fps * 30 / 60;
    pad_repeat_rate = fps * 8 / 60;
}

void pad_set_repeat(int delay, int rate)
{
    if (delay >= 0) {
        pad_repeat_delay = delay;
    }
    if (rate > 0) {
        pad_repeat_rate = rate;
    }
}

unsigned int pad_get_repeat(void)
{
    return pad_repeat_delay << 16 | pad_repeat_rate & 0xFFFF;
}

/* Direction of the stick (atan2 of the offsets from the center), or of the d-pad in steps of
   pi/4 when the stick is centered; NaN when nothing is pressed. */
float pad_angle(PadBits *p)
{
    static const signed char dir_steps[9] = { -3, -4, 3, -2, 0, 2, -1, 0, 1 };
    int dx = p->analog_x - 0x80;
    int dy = p->analog_y - 0x80;
    int moved;
    int d;

    moved = !(int_abs(dx) < 0x20 && int_abs(dy) < 0x20);
    if (moved) {
        return atan2f(dx, dy);
    }
    d = 4;
    if (p->down & 0x40) {
        d += 3;
    } else if (p->down & 0x10) {
        d -= 3;
    }
    if (p->down & 0x20) {
        d++;
    } else if (p->down & 0x80) {
        d--;
    }
    if (d == 4) {
        return nanf_value();
    }
    return dir_steps[d] * 0.7853982f;
}
