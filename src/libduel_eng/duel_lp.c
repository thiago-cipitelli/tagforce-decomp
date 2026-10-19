// @range 0x088C4448-0x088C4864
/* LP changes: damage and recovery with the cards that prevent them, and their commands to the
   presentation (0x25-0x30). */
#include "duel.h"

typedef struct DuelLpEv {              /* event argument of an LP change */
    unsigned short n;
    unsigned char player : 1;
    unsigned char b2_1 : 7;
    unsigned char pad3;
} DuelLpEv;

void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_find_effect_card(unsigned int p, int id, int z);
void duel_08832624(unsigned int p, int a, int b, int c);
void duel_088C321C(unsigned int p);
void duel_088AFD5C(int kind, DuelLpEv ev);
int duel_088C4730(unsigned int p, int n, int f, int id);

void duel_088C4448(unsigned int p, int a, unsigned int uid)
{
    unsigned int id = duel.list[uid].id;

    duel_cmd((p ? 0x8000 : 0) | 0x2E, a, id, uid);
}

void duel_088C448C(int p, int a)
{
    if (p < 0) {
        duel_cmd(0x30, a, 1, 0);
    } else {
        duel_cmd((p ? 0x8000 : 0) | 0x30, a, 0, 0);
    }
}

void duel_088C44E4(unsigned int p, int a)
{
    duel_cmd((p ? 0x8000 : 0) | 0x2F, a, 1, 0);
}

void duel_088C4508(unsigned int p, int a)
{
    duel_cmd((p ? 0x8000 : 0) | 0x2B, a, 1, 0);
}

void duel_088C452C(unsigned int p, int n)
{
    if (n > 0xFFFF) {
        n = 0xFFFF;
    }
    duel_cmd((p ? 0x8000 : 0) | 0x26, n, 0, 0);
}

void duel_088C4564(unsigned int p, int n)
{
    if (n != 0) {
        if (n > 0xFFFF) {
            n = 0xFFFF;
        }
        duel_cmd((p ? 0x8000 : 0) | 0x25, n, 2, 0);
    }
}

void duel_088C45B4(unsigned int p, int n)
{
    if (n != 0) {
        if (n > 0xFFFF) {
            n = 0xFFFF;
        }
        duel_cmd((p ? 0x8000 : 0) | 0x25, n, 3, 0);
    }
}

int duel_088C4604(DuelAct *a, unsigned int p, int n)
{
    if (a->b4_5) {
        switch (a[1].id) {
        case 0x156E:
            if (p == a[1].player) {
                a->b4_5 = 0;
                return duel_088C4604(a, 1 - p, n);
            }
            break;
        case 0x15A7:
            if (p == a[1].player) {
                return duel_088C4604(a, 1 - p, n);
            }
            break;
        case 0x15AB:
            if (p == a[1].player) {
                duel_08832624(1 - p, 1, 0, 1 - p != a->player);
                return 0;
            }
            break;
        case 0x198F:
            return 0;
        }
    }
    return duel_088C4730(p, n, p != (a->player ^ a->opp), a->id);
}

int duel_088C4730(unsigned int p, int n, int f, int id)
{
    DuelLpEv ev;

    if (n == 0) {
        return 0;
    }
    if (f != 0 && duel_slot_has_effect(p, 11, 0x1805) != 0) {
        return 0;
    }
    if (duel_slot_has_effect(p, 11, 0x1850) != 0) {
        return 0;
    }
    if (duel_slot_has_effect(p, 11, 0x1A06) != 0) {
        return 0;
    }
    if (duel_find_effect_card(p, 0x18C4, -1) != 0) {
        return 0;
    }
    if (n > 0xFFFF) {
        n = 0xFFFF;
    }
    duel_cmd((p ? 0x8000 : 0) | 0x25, n, 0, 0);
    if (id != 0x159B) {
        duel_088C321C(p);
    }
    ev.player = p;
    ev.n = n;
    duel_088AFD5C(0x10, ev);
    return 1;
}
