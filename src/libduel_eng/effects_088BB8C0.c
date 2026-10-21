// @range 0x088BB8C0-0x088BC9E0
// @asm 0x088BC910-0x088BC9E0
/* Card effect handlers: candidate lists for duel_088BFC64 (after effects_088B9CC8.c). */
#include "duel.h"

typedef struct DuelCardF15 {           /* DuelCard with bits 15-16 read as one field */
    unsigned int id : 13;
    unsigned int owner : 1;
    unsigned int b14 : 1;
    unsigned int f15 : 2;
    unsigned int rest : 15;
} DuelCardF15;

int duel_list_card_level(unsigned int p, int i);
int duel_card_dfac8(unsigned int p, int id, int field);
void duel_088BFC64(unsigned int p, int zone, unsigned int i);
int card_is_monster(int id);
int card_get_level(int id);
int card_get_type(int id);
int card_is_in_group_1a6f(int id);
int duel_088A6B5C(unsigned int p, int id);
int duel_088A79F0(unsigned int p, int id);
int duel_088A77A8(unsigned int p, int z, unsigned int i);
int duel_088DFE24(unsigned int p, unsigned int q);
int duel_card_e5b88(int id, int a);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
unsigned int duel_field_type(unsigned int p, int z);
int card_get_prop5_hi(int id);
int duel_act_check_08853CE8(DuelAct *a, int x, int y);
int card_get_atk_q(int id);
int card_get_kind(int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int card_is_in_group_198c(int id);
unsigned int duel_card_b14(DuelCard *c);
int card_same_name(int id, int id2);
extern DuelAct *duel_w_089147DC;

void duel_088BB8C0(unsigned int p)
{
    unsigned int i;
    int id;
    int id2;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        id = duel.players[p & 1].hand[i].id;
        if (card_is_in_group_1a6f(id) && duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i])) < 5 && duel_card_dfac8(p, id, 1)) {
            duel_088BFC64(p, 0xB, i);
        }
    }
    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id2 = duel.players[p & 1].zone_d[i].id;
        if (card_is_in_group_1a6f(id2) && card_get_level(id2) < 5 && duel_card_dfac8(p, id2, 1)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BBA58(unsigned int p, int b, int i)
{
    if (card_is_monster(duel.players[p & 1].hand[i].id)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088BBACC(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_get_type(duel.players[p & 1].zone_d[i].id) == 6) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BBB78(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_is_in_group_1a6f(duel.players[p & 1].grave[i].id)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BBC20(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        duel_088BFC64(p, 0xE, i);
    }
}

void duel_088BBC9C(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_18; i++) {
        id = duel.players[p & 1].zone_c[i].id;
        if (card_get_type(id) == 7 && duel_088A6B5C(p, id)) {
            duel_088BFC64(p, 0xC, i);
        }
    }
}

void duel_088BBD68(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_level(id) == 4 && duel_card_dfac8(p, id, 0)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BBE48(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_in_group_1a6f(id) && duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088BBEE0(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (((DuelCardF15 *)&duel.players[p & 1].grave[i])->f15) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BBF88(unsigned int p, int b, int c)
{
    unsigned int i;
    int id;
    int z;

    if (c == 0) {
        for (i = 0; i < duel.players[p & 1].n_18; i++) {
            id = duel.players[p & 1].zone_c[i].id;
            if (card_is_in_group_1a6f(id) && duel_088A79F0(p, id)) {
                duel_088BFC64(p, 0xC, i);
            }
        }
    } else {
        for (z = 0; z <= 4; z++) {
            if (duel_088A77A8(p, z, 0)) {
                duel_088BFC64(p, z, 0);
            }
        }
        if (duel_088DFE24(p, p)) {
            for (i = 0; i < duel.players[p & 1].n_grave; i++) {
                if (duel_088A77A8(p, 0xE, i)) {
                    duel_088BFC64(p, 0xE, i);
                }
            }
        }
    }
}

void duel_088BC110(unsigned int p, int b, int c)
{
    unsigned int i;
    int id;

    if (c == 0) {
        for (i = 0; i < duel.players[p & 1].n_18; i++) {
            id = duel.players[p & 1].zone_c[i].id;
            if (card_get_type(id) == 1 && duel_088A79F0(p, id)) {
                duel_088BFC64(p, 0xC, i);
            }
        }
    } else {
        duel_088BBF88(p, b, c);
    }
}

void duel_088BC1F0(unsigned int p, int b, int uid)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_get_level(id) < 5 && duel_card_e5b88(id, 5) && uid != CARD_UID(duel.players[p & 1].grave[i]) &&
            duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BC318(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_monster(id) && duel_w_089147DC->w[2] == duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i])) &&
        duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088BC3F4(unsigned int p)
{
    int z;
    int any;
    int id;
    int j;

    int ok[5];
    DuelAct act;
    char *q;
    int n;

    any = 0;
    q = (char *)&act;
    n = 0x18;
    if (q != 0) {
        do {
            *q++ = 0;
        } while (--n);
    }
    act.player = p;
    act.zone = 0xB;
    act.b3_4 = 0;
    for (z = 0; z <= 4; z++) {
        if (duel.players[p & 1].field[z].card.id != 0 && duel.players[p & 1].field[z].faceup != 0 &&
            (unsigned short)duel_field_type(p, z) == 0xF) {
            any = 1;
            ok[z] = 1;
        } else {
            ok[z] = 0;
        }
    }
    if (any) {
        for (z = 0; z < duel.players[p & 1].n_grave; z++) {
            id = duel.players[p & 1].grave[z].id;
            if (card_get_prop5_hi(id) == 3) {
                act.id = id;
                act.uid = CARD_UID(duel.players[p & 1].grave[z]);
                for (j = 0; j <= 4; j++) {
                    if (ok[j] && duel_act_check_08853CE8(&act, p, j) == 1) {
                        duel_088BFC64(p, 0xE, z);
                        break;
                    }
                }
            }
        }
    }
}

void duel_088BC62C(unsigned int p, int b)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_atk_q(id) < 0x5DD && card_get_kind(id) == card_get_kind(b) &&
            card_get_type(id) == card_get_type(b) && duel_card_dfac8(p, id, 1) &&
            !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BC790(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == 0xF) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BC83C(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_is_in_group_198c(id) && duel_card_b14(&duel.players[p & 1].grave[i])) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

#ifdef NON_MATCHING /* a cópia de k para $s1 sai antes do teste do laço; registradores trocados */
void duel_088BC910(unsigned int p, int b, int k)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_same_name(duel.players[p & 1].grave[i].id, k) && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}
#endif
