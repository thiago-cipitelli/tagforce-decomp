// @range 0x08883114-0x08884118
// @asm 0x08883324-0x088837E4
/* Card effect handlers (DuelAct *a), after effects_r.c. */
#include "duel.h"

extern int duel_step;
extern unsigned short duel_h_0891968E;

int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int duel_field_def(unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
int duel_088D14B8(DuelAct *a, unsigned int p, int z);
void duel_088C9344(DuelAct *a, unsigned int p, int z, int b);
void duel_088C7E68(unsigned int p, int z, int a, int b);
void duel_088C7CF0(void);
void duel_088C7BA4(void);
void duel_088C4508(unsigned int p, int id);
void duel_088C7B90(int n);
void duel_088C09C8(int a, int b);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
int card_get_type(int id);
void duel_088C96EC(unsigned int p, int z, int a);
void duel_088D147C(DuelAct *a, unsigned int p, int z);
void duel_088C87A8(unsigned int p, int z, unsigned short id, unsigned short a, unsigned short b);
int duel_pick_st_zone(unsigned int p);
void duel_088D1C70(DuelAct *a, unsigned int p, int z);
void duel_088D1DA4(DuelAct *a, unsigned int p, int z, unsigned int q, int y);
void duel_088C8B78(unsigned int p, unsigned short loc, unsigned short t, unsigned short k);
void duel_088D13CC(DuelAct *a, unsigned int mask);
void duel_088C4604(DuelAct *a, unsigned int p, int n);
unsigned short duel_slot_e1854(unsigned int p, int z);
void duel_088D1438(unsigned int p, int z, int a, int b);
int duel_card_can_affect();
int duel_slot_immune(unsigned int p, int z, int opp);
int duel_0888A020(DuelAct *a, int b);
void duel_08832AF4(unsigned int p, int z, int a, int b);
void duel_0887DB24(DuelAct *a);
void duel_088C78E0(unsigned int p);
void duel_088328FC(unsigned int p);
DuelCard *duel_0884C7CC(void);
int duel_088CB554(unsigned int p, DuelCard *c);
void duel_0884C588(unsigned int p, int a, int id, int b);
void duel_0884C7C0(int i);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
void duel_0884ACD8(unsigned int p, int msg);
void duel_0887DC94(DuelAct *a);
void duel_088C8DC4(unsigned short loc, unsigned short src);
int duel_0887DDB0(DuelAct *a, int b);

int duel_08883114(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;
    unsigned int id;

    if (a->b4_2) {
        return 0;
    }
    t = duel_act_target(a, 0);
    p = t & 0xFF;
    z = (unsigned short)t >> 8 & 0xFF;
    id = duel.players[p & 1].field[z].card.id;
    switch (duel_step) {
    case 0x80:
        if (duel_088A9008(a, 0, 0) != 0) {
            if (duel.players[p & 1].field[z].pos != 0 && duel.players[p & 1].field[z].faceup == 0) {
                    duel_088C7BA4();
                duel_088C9344(a, p, z, 1);
                duel_088C7E68(p, z, 1, 1);
                return 0x7F;
            }
        }
        return 0;
    case 0x7F:
        duel_088C4508(p, id);
        duel_088C7B90(0x1E);
        duel_088C09C8(-1, 0);
        return 0x7E;
    case 0x7E:
        if (duel_field_def(p, z) > 2000) {
            duel_088C9344(a, p, z, 0);
            duel_088C7E68(p, z, 1, 0);
        } else {
            duel_088D14B8(a, p, z);
        }
        duel_088C7CF0();
        return 1;
    }
    return 0;
}

int duel_088832F8(DuelAct *a)
{
    duel_088C77D4(0, a->id, 1, 0);
    return 0;
}

#ifdef NON_MATCHING /* registradores e desvios */
int duel_08883324(DuelAct *a, DuelAct *b)
{
    unsigned int loc;
    unsigned int q;
    unsigned int z;
    unsigned int id;
    int i;

    if (a->b4_2 || b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    if (a->uid != CARD_UID(duel.players[a->player].field[a->zone].card)) {
        return 0;
    }
    switch (b->id) {
    case 0x1246:
    case 0x131B:
    case 0x131C:
    case 0x1321:
        for (z = 5; z < 11; z++) {
            id = duel.players[a->player & 1].field[z].card.id;
            if (id != 0 && card_get_type(id) == 0x17 && duel.players[a->player & 1].field[z].faceup == 0) {
                duel_088C96EC(a->player, z, 1);
                duel_088C4508(a->player, id);
                duel_088C7B90(0x1E);
                duel_088C96EC(a->player, z, 0);
            }
        }
        b->b4_4 = 1;
        break;
    case 0x11F0:
        for (i = 0; i < 2; i++) {
            duel_088A9008(b, i, &loc);
            q = loc & 0xFF;
            z = (unsigned short)loc >> 8 & 0xFF;
            id = duel.players[loc & 1].field[z].card.id;
            if (id != 0 && card_get_type(id) == 0x17 && duel.players[loc & 1].field[z].faceup == 0) {
                duel_088C96EC(q, z, 1);
                duel_088C4508(a->player, id);
                duel_088C7B90(0x1E);
                duel_088C96EC(q, z, 0);
            }
        }
        b->b4_4 = 1;
        break;
    case 0xFFA:
    case 0x1086:
    case 0x12E6:
    case 0x132D:
    case 0x134C:
    case 0x134D:
    case 0x1364:
    case 0x149B:
    case 0x16B8:
        duel_088A9008(b, 0, &loc);
        q = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        id = duel.players[loc & 1].field[z].card.id;
        if (id != 0 && (q != a->player || z != a->zone)) {
            if (card_get_type(id) == 0x17) {
                if (duel.players[loc & 1].field[z].faceup == 0) {
                    duel_088C96EC(q, z, 1);
                    duel_088C4508(a->player, id);
                    duel_088C7B90(0x1E);
                    duel_088C96EC(q, z, 0);
                }
                duel_088D147C(a, a->player, a->zone);
            }
            b->b4_4 = 1;
        }
        break;
    }
    return 0;
}
#endif

int duel_088837E4(DuelAct *a)
{
    unsigned int loc;
    unsigned int q;
    unsigned int z;

    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].faceup == 0) {
        return 0;
    }
    if (duel_088A9008(a, 0, &loc) != 0) {
        q = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel_0884E440(a, q, z) != 0) {
            duel_088C87A8(q, z, a->player | a->zone << 8, 0x106, 0);
        }
    }
    return 0;
}

int duel_08883918(DuelAct *a)
{
    unsigned int loc;
    unsigned int q;
    int z;
    unsigned int p;
    int s;

    if (a->b4_2) {
        return 0;
    }
    if (duel_088A9008(a, 0, &loc) != 0) {
        q = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel_0884E440(a, q, z) != 0) {
            s = duel_pick_st_zone(a->player);
            if (duel.players[a->player & 1].field[a->zone].card.id != 0 && duel.players[a->player & 1].field[a->zone].faceup != 0 && s >= 0) {
                duel_088D1DA4(a, q, z, 1 - q, s);
                if (a->id == 0x15D8) {
                    p = a->player;
                    duel_088C8B78(p, p | (s & 0xFF) << 8, p | a->zone << 8, 10);
                } else {
                    duel_088C7E68(1 - q, s, 8, 1);
                    p = a->player;
                    duel_088C8B78(p, p | (s & 0xFF) << 8, p | a->zone << 8, 0xD);
                }
            } else {
                duel_088D1C70(a, q, z);
            }
        }
    }
    return 0;
}

int duel_08883AF0(DuelAct *a)
{
    int z;
    int n;

    if (a->b4_2) {
        return 0;
    }
    if (a->kind == 2) {
        n = 0;
        for (z = 0; z < 5; z++) {
            if (duel.players[a->player & 1].field[z].card.id != 0) {
                n += duel_field_atk(a->player, z);
            }
        }
        duel_088D13CC(a, 0x1F << (a->player << 4));
        duel_088C4604(a, 1 - a->player, (n + 1) >> 1);
    } else {
        duel_088C7E68(a->player, a->zone, 4, 1);
    }
    return 0;
}

int duel_08883C18(DuelAct *a, int b)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (a->kind == 2) {
        if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
            return 0;
        }
        duel_088C7BA4();
        t = duel_slot_e1854(a->player, a->zone);
        z = (unsigned short)t >> 8 & 0xFF;
        p = t & 0xFF;
        if (a->id == 0x1286) {
            duel_088D1438(a->player, a->zone, 0, 0);
        }
        if (duel_card_can_affect(p, z, a->id, a->zone, 1) != 0 && duel_slot_immune(p, z, p != a->player) == 0) {
            duel_088D1438(p, z, p != a->player, a->id);
        }
        if (a->id == 0x1286) {
            duel_088C4604(a, 1 - a->player, duel_field_atk(p, z));
        }
        duel_088C7CF0();
        return 0;
    }
    return duel_0888A020(a, b);
}

int duel_08883DE4(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    if (duel_088A9008(a, 0, 0) != 0) {
        switch (duel_step) {
        case 0x80:
            duel_08832AF4(a->player, a->zone, 3, 1);
            return 0x7F;
        case 0x7F:
            if (duel_h_0891968E >= 2) {
                duel_0887DB24(a);
            }
            return 0;
        }
    }
    return 0;
}

int duel_08883EA0(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_088BFF78(a->player, a->id, 0) == 0) {
            duel_0884ACD8(a->player, 0x15);
            return 0x78;
        }
        return 0x7F;
    case 0x7F:
        if (a->id == 0x139D) {
            duel_0884C588(a->player, 6, a->id, 0);
        } else {
            duel_0884C7C0(0);
        }
        return 0x7E;
    case 0x7E:
        duel_088CB554(a->player, duel_0884C7CC());
        return 100;
    case 0x78:
        duel_088328FC(1 - a->player);
        return 100;
    case 100:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}

int duel_08883FBC(DuelAct *a)
{
    duel_0887DC94(a);
    return 0;
}

int duel_08883FDC(void)
{
    return 0;
}

int duel_08883FE4(DuelAct *a, int b)
{
    unsigned int t;

    switch (a->w[2]) {
    case 1:
        return duel_0887DDB0(a, b);
    case 2:
        if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
            return 0;
        }
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            if (duel_0884E440(a, t & 0xFF, (unsigned short)t >> 8 & 0xFF) != 0) {
                duel_088C8DC4(t, a->player | a->zone << 8);
            }
        }
        return 0;
    }
    return 0;
}

