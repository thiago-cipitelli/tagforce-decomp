// @range 0x088C87A8-0x088C8B78
/* Commands to the presentation about a zone (0x36: a card moves to it, 0x37: an effect of a card,
   0x38: an effect counter). */
#include "duel.h"

void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_effect_kind(unsigned int p, int z, int id, int kind);
unsigned int duel_slot_effect_index(unsigned int p, int z, unsigned int id, unsigned int arg);
void duel_088C8B78(unsigned int p, unsigned short loc, unsigned short t, int a);

void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b)
{
    duel_cmd((p ? 0x8000 : 0) | 0x36, (z & 0xFF) | (a & 0xFF) << 8, id, b);
}

int duel_088C87D8(DuelAct *x, unsigned int p, int z, unsigned short id, int a, unsigned short b)
{
    if (duel_act_immune(x, p, z) != 0) {
        return 0;
    }
    duel_cmd((p ? 0x8000 : 0) | 0x36, (z & 0xFF) | (a & 0xFF) << 8, id, b);
    return 1;
}

void duel_088C8874(unsigned int uid, unsigned short id, int a, unsigned short b)
{
    duel_cmd((uid & 1 ? 0x8000 : 0) | 0x36, (uid >> 1 & 0xFF) | (a & 0xFF) << 8, id | 0x8000, b);
}

void duel_088C88B8(unsigned int p, int z, int v, int c)
{
    if (duel_st_affects(p, z, v & 0xFF, (unsigned short)v >> 8 & 0xFF) != 0) {
        duel_cmd((p ? 0x8000 : 0) | 0x37, (z & 0xFF) | 0xA00, v, c);
    }
}

void duel_088C8940(int q, int sz)
{
    unsigned int p;
    int z;

    for (p = 0; (int)p < 2; p++) {
        for (z = 0; z <= 10; z++) {
            duel_088C88B8(p, z, (unsigned short)((q & 0xFF) | (sz & 0xFF) << 8), 1);
        }
    }
}

void duel_088C89BC(unsigned int p, int z, int id, int c)
{
    if (duel_slot_has_effect(p, z, id) != 0) {
        duel_cmd((p ? 0x8000 : 0) | 0x37, (z & 0xFF) | 0x100, id, c);
    }
}

void duel_088C8A38(unsigned int p, int z, int id, int k, int c)
{
    if (duel_slot_effect_kind(p, z, id, k) != 0) {
        duel_cmd((p ? 0x8000 : 0) | 0x37, (z & 0xFF) | (k & 0xFF) << 8, id, (c & 0xFF) | 0x100);
    }
}

void duel_088C8AD0(unsigned int p, int z, unsigned int id, unsigned int arg)
{
    unsigned int n = duel_slot_effect_index(p, z, id, arg);

    if (n != 0) {
        duel_cmd((p ? 0x8000 : 0) | 0x38, (z & 0xFF) | (n & 0xFF) << 8, 0, 0);
    }
}

void duel_088C8B3C(unsigned int p, int z, int n, unsigned short id, unsigned short c)
{
    duel_cmd((p ? 0x8000 : 0) | 0x38, (z & 0xFF) | (n & 0xFF) << 8, id, c);
}

void duel_088C8B70(unsigned int p, unsigned short loc, unsigned short t)
{
    duel_088C8B78(p, loc, t, 10);
}
