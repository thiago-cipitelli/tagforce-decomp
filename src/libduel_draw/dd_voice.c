// @range 0x08819A70-0x08819C04
// @data 0x08875E34-0x08875E60
// @rodata 0x088700F8-0x0887015C
/* Exports for the duel manager (the first one, 0x088199C8, is at the end of dd_event_cmd.c) around the commentary voice (status 8 of func_08806434) and the
   duel-state sounds: toggle, query, per-state voice and a clamped sound table. */
#include "dd_field_ui.h"

int func_08806434(int a, int b);
void dd_mode_push(int id);
void dd_mode_pop(void);
void dd_chara_busy_clear(void);
void dd_chara_reset(void);
int dd_chara_ready(void);
void dd_chara_select(void);
void dd_chara_set_voice(int v);
void dd_chara_redraw(int state);
void dd_chara_anim(int v, int a);
void func_0883B620(int id);
void func_08819BFC(int v, int a);

/* sound per duel state 0..9 (others: 7) */
int dd_state_se[11] = { 8, 0, 1, 5, 4, 3, 2, 8, 6, 7, 8 };

/* export: commentary active */
int func_08819A70(void)
{
    return func_08806434(8, -1);
}

/* export: commentary idle */
int func_08819A7C(void)
{
    if (func_08806434(8, -1) == 0)
        return 0;
    return !dd_chara_ready();
}

/* export */
void func_08819ABC(void)
{
}

/* export: the commentary line of a duel state */
void func_08819AC4(int a, int state)
{
    int v;

    dd_chara_select();
    dd_chara_redraw(state);
    dd_chara_set_voice(-1);
    v = -1;
    switch (state) {
    case 1:
        v = 0x0;
        break;
    case 2:
        v = 0x2;
        break;
    case 3:
        v = 0x3;
        break;
    case 4:
        v = 0x6;
        break;
    case 5: case 21:
        v = 0x4;
        break;
    case 6: case 20:
        v = 0x4;
        break;
    case 7:
        v = 0x4;
        break;
    case 8:
        v = 0x9;
        break;
    case 9: case 22:
        v = 0x8;
        break;
    case 10: case 23:
        v = 0xA;
        break;
    case 11: case 19:
        v = 0x10;
        break;
    case 12:
        v = 0x12;
        break;
    case 13:
        v = 0x13;
        break;
    case 14:
        v = 0x16;
        break;
    case 15:
        v = 0x17;
        break;
    case 16:
        v = 0x18;
        break;
    case 17:
        v = 0x19;
        break;
    case 18:
        v = 0x1A;
        break;
    case 24:
        v = 0x1C;
        break;
    }
    if (v >= 0)
        func_08819BFC(v, 0);
}

/* export */
void func_08819BC4(int v)
{
    dd_chara_set_voice(v);
}

/* export: sound of duel state n */
void func_08819BCC(int n)
{
    if (n < 0 || n > 9)
        n = 7;
    func_0883B620(dd_state_se[n]);
}

/* export: plays commentary line v */
void func_08819BFC(int v, int a)
{
    dd_chara_anim(v, a);
}
