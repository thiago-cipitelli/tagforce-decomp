// @range 0x088BC9E0-0x088BE638
// @asm 0x088BD150-0x088BD348
// @asm 0x088BD4E0-0x088BD9A4
/* Card effect handlers: candidate lists for duel_088BFC64 (after effects_088BB8C0.c). */
#include "duel.h"

int duel_list_card_level(unsigned int p, int i);
int duel_card_dfac8(unsigned int p, int id, int field);
void duel_088BFC64(unsigned int p, int zone, unsigned int i);
int card_is_monster(int id);
int card_get_level(int id);
int card_get_type(int id);
int card_is_in_group_1973(int id);
int card_is_in_group_19bf(int id);
int card_is_attribute8(int id);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
int card_get_prop5_hi(int id);
int duel_act_check_08853CE8(DuelAct *a, int x, int y);
int card_get_atk_q(int id);
int duel_088DFCC8(unsigned int uid);
int card_same_name(int id, int id2);
int duel_count_active(int id);
int duel_find_effect_card(unsigned int p, int id, int z);
int card_is_in_group_19bb(int id);
int duel_088B5F74(int n, int x);
int duel_card_e59b0(int id);
int duel_pick_st_zone(unsigned int p);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
unsigned int duel_card_locate(unsigned int uid);
int card_is_in_group_1a6x(int id);
int card_is_in_group_1a6f(int id);
int card_get_attribute_icon(int id);
extern unsigned int duel_w_089147E4;
extern unsigned short duel_w_089147E8[];

void duel_088BC9E0(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_is_in_group_1973(id) && card_get_level(id) < 5) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BCAB8(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_is_monster(duel.players[p & 1].grave[i].id)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BCB60(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_in_group_19bf(id) && duel_card_dfac8(p, id, 1)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BCC2C(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_is_in_group_1973(id) && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BCD08(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_is_in_group_1973(id)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BCDCC(unsigned int p)
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
        if (duel.players[p & 1].field[z].card.id != 0 && duel.players[p & 1].field[z].faceup != 0) {
            ok[z] = 1;
            any = 1;
        } else {
            ok[z] = 0;
        }
    }
    if (any) {
        for (z = 0; z < duel.players[p & 1].n_grave; z++) {
            id = duel.players[p & 1].grave[z].id;
            if (card_is_attribute8(id)) {
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

void duel_088BCFE0(unsigned int p, int b, unsigned int c)
{
    unsigned int i;
    int id;

    if (c & 0x10000000) {
        for (i = 0; i < duel.players[p & 1].n_grave; i++) {
            id = duel.players[p & 1].grave[i].id;
            if (card_is_monster(id) && card_get_atk_q(id) <= (c & 0xFFFFFFF)) {
                duel_088BFC64(p, 0xE, i);
            }
        }
    } else {
        if (card_get_prop5_hi(duel.players[p & 1].hand[c].id) == 3 && duel_088DFCC8(CARD_UID(duel.players[p & 1].hand[c]))) {
            duel_088BFC64(p, 0xB, c);
        }
    }
}

void duel_088BD148(unsigned int p, int b, unsigned int i)
{
    duel_088BFC64(p, 0xB, i);
}

#ifdef NON_MATCHING /* o original testa duel_count_active(0x159D) com um beqz para a instrução seguinte; sem o teste no fonte o mwcc não emite o desvio */
void duel_088BD150(unsigned int p, int b, int k)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_same_name(duel.players[p & 1].zone_d[i].id, k) && duel_card_dfac8(p, k, 0)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
    duel_count_active(0x159D); /* the original tests the result but branches to the next instruction either way */
    if (duel_find_effect_card(p, 0x158B, -1)) {
        for (i = 0; i < duel.players[p & 1].n_grave; i++) {
            if (card_same_name(duel.players[p & 1].grave[i].id, k) && duel_grave_card_usable(p, p, i)) {
                duel_088BFC64(p, 0xE, i);
            }
        }
    }
    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        if (card_same_name(duel.players[p & 1].hand[i].id, k) && duel_card_dfac8(p, k, 0)) {
            duel_088BFC64(p, 0xB, i);
        }
    }
}
#endif

void duel_088BD348(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[(1 - p) & 1].n_grave; i++) {
        if (card_get_type(duel.players[(1 - p) & 1].grave[i].id) == 0x16) {
            duel_088BFC64(1 - p, 0xE, i);
        }
    }
}

void duel_088BD3F8(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_monster(id) && card_get_type(id) == 7 && duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i])) < 3 &&
        duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

#ifdef NON_MATCHING /* o original guarda p e max na pilha, usa $s8 e recarrega duel_w_089147E4 a cada escrita no anel; frame de 128 bytes contra 112 */
void duel_088BD4E0(unsigned int p, int b, unsigned int c)
{
    unsigned int i;
    int id;
    int lv;
    int j;
    int k;
    int n;
    int max;
    int cnt[13];

    max = c >> 16;
    if (max == 0) {
        id = duel.players[p & 1].hand[c & 0xFFFF].id;
        if (!card_is_monster(id)) {
            return;
        }
        if (!card_is_in_group_19bb(id)) {
            return;
        }
        lv = duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[c & 0xFFFF]));
        if (lv < 5) {
            return;
        }
        max = lv * 2;
    }
    for (lv = 1; lv < 13; lv++) {
        cnt[lv] = 0;
    }
    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_is_in_group_19bb(id)) {
            cnt[card_get_level(id)]++;
        }
    }
    for (lv = 12; lv > 0; lv--) {
        if (cnt[lv]) {
            n = 0;
            for (j = 12; j > 0; j--) {
                for (k = lv == j; k < cnt[j]; k++) {
                    duel_w_089147E8[(duel_w_089147E4 + n) & 0x7F] = j;
                    n++;
                }
            }
            if (!duel_088B5F74(n - 1, max - lv)) {
                cnt[lv] = 0;
            }
        }
    }
    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_is_in_group_19bb(id) && cnt[card_get_level(id)]) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}
#endif

void duel_088BD9A4(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_get_type(id) == 0x16 && !duel_card_e59b0(id) && (duel_pick_st_zone(p) >= 0 || card_get_prop5_hi(id) == 2)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BDA90(unsigned int p, int b, unsigned int c)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_atk_q(id) <= c && duel_card_dfac8(p, id, 1) &&
            !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BDBB4(unsigned int p, int b)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_get_type(id) == card_get_type(b) && card_get_atk_q(id) < 0x5DD &&
            !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BDCD4(unsigned int p)
{
    unsigned int i;
    unsigned int j;
    int id;
    int n;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id)) {
            n = 0;
            for (j = 0; j < duel.players[p & 1].n_grave; j++) {
                if (card_is_monster(id) && card_same_name(id, duel.players[p & 1].grave[j].id)) {
                    n++;
                }
            }
            if (n == 3 && duel_grave_card_usable(p, p, i)) {
                duel_088BFC64(p, 0xE, i);
            }
        }
    }
}

void duel_088BDE24(unsigned int p, int b, int k)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_same_name(k, duel.players[p & 1].zone_d[i].id) &&
            !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]))) {
            duel_088BFC64(p, 0xD, i);
        }
    }
    for (i = 0; i < duel.players[p & 1].n_18; i++) {
        if (card_same_name(k, duel.players[p & 1].zone_c[i].id)) {
            duel_088BFC64(p, 0xC, i);
        }
    }
}

void duel_088BDF80(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        switch (duel.players[p & 1].zone_d[i].id) {
        case 0xFE0:
        case 0x18AA:
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BE03C(unsigned int p, unsigned int b, int z)
{
    unsigned int e = duel.players[p & 1].field[z].effects;
    unsigned int loc;

    while (e != 0) {
        DuelEffect *ef = &duel.effects[e];

        e = ef->next;
        if ((ef->kind & 0xF) < 6 && ef->src == b) {
            loc = duel_card_locate(ef->arg);
            if (((unsigned short)loc >> 8 & 0xFF) == 0xE && duel_grave_card_usable(p, loc & 0xFF, loc >> 16 & 0xFFFF)) {
                duel_088BFC64(loc & 0xFF, 0xE, loc >> 16 & 0xFFFF);
            }
        }
    }
}

void duel_088BE144(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_is_in_group_1a6x(id) && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BE220(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_is_in_group_1a6f(duel.players[p & 1].zone_d[i].id)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BE2C8(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_get_attribute_icon(id) == 0 && card_is_in_group_1a6f(id) && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BE3B4(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && card_get_attribute_icon(id) == 0 && card_is_in_group_1a6f(id) && duel_card_dfac8(p, id, 0)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BE4A0(unsigned int p)
{
    unsigned int i;
    int id;
    int id2;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        id = duel.players[p & 1].hand[i].id;
        if (card_is_in_group_1a6x(id) && duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i])) < 5 && duel_card_dfac8(p, id, 1)) {
            duel_088BFC64(p, 0xB, i);
        }
    }
    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id2 = duel.players[p & 1].zone_d[i].id;
        if (card_is_in_group_1a6x(id2) && card_get_level(id2) < 5 && duel_card_dfac8(p, id2, 1)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}
