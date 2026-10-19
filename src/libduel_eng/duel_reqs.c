// @range 0x088C87A8-0x088C936C
// @asm 0x088C8B78-0x088C8DC4
// @asm 0x088C8DC4-0x088C8E88
// @asm 0x088C8F04-0x088C914C
// @asm 0x088C914C-0x088C929C
/* Commands to the presentation about a zone (0x36: a card moves to it, 0x37: an effect of a card,
   0x38: an effect counter). */
#include "duel.h"

void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_effect_kind(unsigned int p, int z, int id, int kind);
unsigned int duel_slot_effect_index(unsigned int p, int z, unsigned int id, unsigned int arg);
void duel_088CC178();
void duel_088CC1D0(unsigned int p, int z, int a);
void duel_088C7E68(unsigned int p, int z, int a, int b);
int field_get_card(unsigned int p, int z);
void duel_0884F30C(unsigned int a, int uid, unsigned int b);
void duel_088AFD5C(int kind, unsigned int a);
unsigned short duel_slot_e1854(unsigned int p, int z);
void duel_088D1438(unsigned int p, int z, int a, int b);
int duel_field_attr_is(unsigned int p, int z, int attr);
int card_get_kind(int id);
void duel_088C936C(unsigned int p, int a, int b, int id, int opp);
void duel_088C8B78(unsigned int p, unsigned short loc, unsigned short t, unsigned short k);

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

#ifdef NON_MATCHING /* register allocation and branch layout not reproduced */
void duel_088C8B78(unsigned int p, unsigned short loc, unsigned short t, unsigned short k)
{
    unsigned int tz = t >> 8;
    unsigned int lz = loc >> 8;
    unsigned int tp = t & 0xFF;
    unsigned int q;

    if (k == 10) {
        if (duel.players[loc & 1].field[lz].card.id == 0x13EA) {
            duel_088CC178(loc & 0xFF, lz, 1 - tp);
        }
        duel_088CC1D0(loc & 0xFF, lz, 0);
    }
    duel_cmd((p ? 0x8000 : 0) | 0x42, loc, t, k);
    q = t & 1;
    if (SLOT_ACTIVE(&duel.players[q].field[tz])) {
        int id = field_get_card(tp, tz);

        if (id == 0x186B) {
            duel_088C7E68(tp, tz, 4, 1);
        } else if (id == 0x13C3) {
            duel_0884F30C(t << 31 | 0x30200000 | (tz & 0x1F) << 16 | (field_get_card(tp, tz) & 0xFFFF),
                          CARD_UID(duel.players[q].field[tz].card), loc | t << 16);
        }
    }
    duel_088AFD5C(0x18, loc | t << 16);
}
#endif

void duel_088C8B70(unsigned int p, unsigned short loc, unsigned short t)
{
    duel_088C8B78(p, loc, t, 10);
}

#ifdef NON_MATCHING /* register allocation and branch layout not reproduced */
void duel_088C8DC4(int loc, unsigned short t)
{
    unsigned int p = loc & 0xFF;
    unsigned int z = (unsigned short)loc >> 8 & 0xFF;
    unsigned short s = duel_slot_e1854(p, z);
    int k;

    if (s != 0xFFFF) {
        k = duel_st_affects(s & 0xFF, s >> 8 & 0xFF, p, z);
        duel_088C88B8(s & 0xFF, s >> 8 & 0xFF, (unsigned short)loc, 1);
    } else {
        k = 10;
    }
    duel_088C8B78(p, loc, t, k);
}
#endif

void duel_088C8E88(int q, int sz)
{
    unsigned int p;
    int z;

    for (p = 0; (int)p < 2; p++) {
        for (z = 0; z < 5; z++) {
            duel_088C88B8(p, z, (unsigned short)((q & 0xFF) | (sz & 0xFF) << 8), 1);
        }
    }
}

#ifdef NON_MATCHING /* register allocation and branch layout not reproduced */
void duel_088C8F04(unsigned int p, int z)
{
    unsigned int e = duel.players[p & 1].field[z].effects;

    while (e != 0) {
        DuelEffect *ef = &duel_effects[e];
        unsigned int k;
        int v;

        e = ef->next;
        k = ef->kind & 0xF;
        if (k == 7 || k == 6 || k == 10) {
            unsigned int sp = ef->src & 0xFF;
            unsigned int sz = ef->src >> 8;
            unsigned int q = ef->src & 1;

            if (duel.players[q].field[sz].card.id != 0 && duel.players[q].field[sz].faceup != 0 &&
                !(duel.players[q].field[sz].flags >> 5 & 1)) {
                int id = field_get_card(sp, sz);

                v = sp;
                if (id == 0x18C2 || id == 0x18C1 || id == 0x18C0 || id == 0x18BF) {
                    v = -1;
                    if (duel_field_attr_is(p, z, card_get_kind(field_get_card(sp, sz))) != 0) {
                        v = ef->arg & 1;
                    }
                } else if (id == 0x13EA) {
                    v = duel.players[q].field[sz].link;
                } else if (id != 0x1A9C && id != 0x1877 && id != 0x169A && id != 0x1466 && id != 0x1322) {
                    v = -1;
                }
                if (v >= 0) {
                    duel_088C8B3C(p, z, e, 5, v);
                }
            }
        }
    }
}
#endif

#ifdef NON_MATCHING /* register allocation and branch layout not reproduced */
void duel_088C914C(unsigned int p, int z)
{
    unsigned int e = duel.players[p & 1].field[z].effects;

    while (e != 0) {
        DuelEffect *ef = &duel_effects[e];
        unsigned int k;

        e = ef->next;
        k = ef->kind & 0xF;
        if (k == 0xD || k == 0xC || k == 0xB || k == 10 ||
            (k == 6 && duel.players[ef->src & 1].field[ef->src >> 8].card.id == 0x19ED)) {
            duel_088C88B8(p, z, ef->src, 0);
            duel_088D1438(ef->src & 0xFF, ef->src >> 8, 0, 0);
        }
    }
}
#endif

void duel_088C929C(unsigned int p, int z, unsigned int kind)
{
    unsigned int e = duel.players[p & 1].field[z].effects;

    while (e != 0) {
        DuelEffect *ef = &duel.effects[e];

        e = ef->next;
        if ((ef->kind & 0xF) == kind) {
            duel_088D1438(EFF_PLAYER(ef), EFF_ZONE(ef), 0, 0);
        }
    }
}

void duel_088C9344(DuelAct *a, unsigned int p, int x, int y)
{
    duel_088C936C(p, x, y, a->id, p != a->player);
}
