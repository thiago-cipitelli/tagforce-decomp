// @range 0x088AF390-0x088AFD5C
// @data 0x08911A38-0x08911A68
/* Card effect handlers (DuelAct *a): target selection on duel_w_08914908, after effects_088AE378.c. */
#include "duel.h"

extern int duel_w_08914908;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern unsigned short duel_h_0891968E;

void duel_088A8B94(DuelAct *a, unsigned short loc);
void duel_088A8BB0(DuelAct *a, unsigned int p, int z);
void duel_088A8D40(DuelAct *a, unsigned int p, int z);
void duel_088A8EA0(DuelAct *a, unsigned int p, int z, int uid);
int duel_088AA488(DuelAct *a, int b);
int duel_088AA844(DuelAct *a, unsigned int p, int z);
int duel_0887A96C(DuelAct *a, int b);
int duel_grave_find_name(unsigned int p, int id);
DuelCard *duel_0884C7CC(void);
int duel_0884C804(void);
DuelCard *duel_0884C7EC(int i);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
int duel_0884E5E0(DuelAct *a);
int duel_0884E9E8(DuelAct *a, int b);
int duel_08835AC0(void);
void duel_088359E8(unsigned int p, int id, int (*fn)(DuelAct *a, unsigned int p, int z));
void duel_08835A54(unsigned int p, unsigned short id, int (*fn)(DuelAct *a, unsigned int p, int z));
void duel_0884ACD8(unsigned int p, void *msg);
void duel_0884AD18(int msg);
int duel_0885E154(DuelAct *a, unsigned int p, int z);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
void duel_088C96EC(unsigned int p, int z, int a);
void duel_088C7B90(int n);
void duel_088329D0(unsigned int p, int a, int b);
int duel_st_zone_free(unsigned int p, int z);
void duel_088CB938(unsigned int p, int a);

int duel_088AF390(DuelAct *a, int b)
{
    int i;
    unsigned int p;

    switch (duel_w_08914908) {
    case 0:
        if (duel_0887A96C(a, b) == 0) {
            return 1;
        }
    case 1:
        return duel_088AA488(a, b);
    case 2:
        i = duel_grave_find_name(a->player, 0x12E5);
        if (i >= 0) {
            p = a->player;
            duel_088A8EA0(a, p, 0xE, CARD_UID(duel.players[p & 1].grave[i]));
            duel_088A8EA0(a, duel_0884C7CC()->owner, duel_0884C804(), CARD_UID(*duel_0884C7CC()));
        }
        break;
    }
    return 1;
}

int duel_088AF4EC(DuelAct *a)
{
    int p;
    int z;

    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        for (p = 0; p < 2; p++) {
            for (z = 0; z <= 4; z++) {
                if (!duel_0884E3E4(a, p, z)) {
                    continue;
                }
                duel_088A8D40(a, p, z);
                duel_w_08914908++;
                return 0;
            }
        }
        return 1;
    case 1:
        duel_0884ACD8(a->player, (void *)0x7B);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_088359E8(a->player, a->id, duel_0885E154);
        duel_w_08914908++;
        return 0;
    case 3:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            return 1;
        }
        duel_w_08914908 -= 2;
        return 0;
    }
    return 1;
}

int duel_088AF678(DuelAct *a, int b)
{
    int i;

    if (duel_0884E9E8(a, b) == 0) {
        return -1;
    }
    duel_088BFF78(a->player, a->id, 0x194F);
    for (i = 0; i < 2; i++) {
        duel_088A8EA0(a, a->player, 0xE, CARD_UID(*duel_0884C7EC(i)));
    }
    duel_088BFF78(a->player, a->id, 0x1950);
    duel_088A8EA0(a, a->player, 0xE, CARD_UID(*duel_0884C7EC(0)));
    return 1;
}

int duel_088AF780(DuelAct *a)
{
    duel_088A8BB0(a, duel_special.player[1], duel_special.zone[1]);
    return 1;
}

int duel_088AF7AC(DuelAct *a)
{
    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        if (a->player != a->b15_3) {
            if (a->player == a->b15_1 && duel_0884E5E0(a) != 0) {
                if (duel_player_type[a->player] == 1) {
                    duel_w_08919624 = 1;
                } else {
                    duel_0884AD18(0x13A);
                }
                duel_w_08914908++;
                return 0;
            }
        }
        return 1;
    case 1:
    case 4:
        if (duel_w_08919624 == 0) {
            return 1;
        }
        duel_088359E8(a->player, a->id, duel_088AA844);
        duel_w_08914908++;
        return 0;
    case 2:
    case 5:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            if (duel_0884E5E0(a) >= 2) {
                if (a->n7 < 2) {
                    duel_w_08914908++;
                    return 0;
                }
            }
            return 1;
        }
        duel_w_08914908 -= 2;
        return 0;
    case 3:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 1;
        } else {
            duel_0884AD18(0x13B);
        }
        duel_w_08914908++;
        return 0;
    }
    return 0;
}

int duel_088AF9DC(void)
{
    if (duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].faceup == 0) {
        duel_088C96EC(duel_special.player[1], duel_special.zone[1], 1);
        duel_088C7B90(0x1E);
        duel_088C96EC(duel_special.player[1], duel_special.zone[1], 0);
    }
    return 1;
}

int duel_088AFA64(DuelAct *a)
{
    duel_088A8BB0(a, a->src.player, a->src.zone);
    return 1;
}

int duel_088AFA94(DuelAct *a)
{
    if (a->kind == 5) {
        switch (duel_w_08914908) {
        case 0:
            duel_088329D0(1 - a->player, a->id, 0);
            duel_w_08914908++;
            return 0;
        case 1:
            duel_088A8EA0(a, a->player, 0xE, duel_h_0891968E);
            return 1;
        }
    }
    return 1;
}

int duel_088AFB34(DuelAct *a, int p, int z)
{
    int i;

    if (a->player == p) {
        return 0;
    }
    if (z < 5 || z >= 10) {
        return 0;
    }
    if (duel_st_zone_free(p, z) == 0) {
        return 0;
    }
    for (i = 0; i < a->n7; i++) {
        if (a->w[i] == (unsigned short)((unsigned char)p | (unsigned char)z << 8)) {
            return 0;
        }
    }
    return 1;
}

int duel_088AFBFC(DuelAct *a)
{
    unsigned int p;
    int z;

    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        duel_0884ACD8(a->player, (void *)0x144);
    case 2:
    case 4:
        duel_08835A54(a->player, a->id, (void *)duel_088AFB34);
        duel_w_08914908++;
        return 0;
    case 1:
    case 3:
    case 5:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_088CB938(p, z);
            duel_088A8B94(a, (unsigned char)p | (unsigned char)z << 8);
            duel_w_08914908++;
            return 0;
        }
        duel_w_08914908--;
        return 0;
    }
    return 1;
}

int duel_088AFD30(DuelAct *a)
{
    duel_088A8D40(a, duel_special.player[1], duel_special.zone[1]);
    return 1;
}
