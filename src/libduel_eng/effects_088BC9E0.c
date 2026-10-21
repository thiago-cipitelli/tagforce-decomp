// @range 0x088BC9E0-0x088C004C
// @asm 0x088BD150-0x088BD348
// @asm 0x088BD4E0-0x088BD9A4
// @asm 0x088BECC0-0x088BEDEC
// @asm 0x088BFB48-0x088BFC64
/* Card effect handlers: candidate lists for duel_088BFC64 (after effects_088BB8C0.c), and the list itself:
   duel_088BFC64 appends a card and its zone to duel_w_08915258 (at most 0x1FF), and duel_088BFF78 runs the list handler of an effect key from the sorted table at
   0x08912988 (binary search over 0x16D entries) and returns the count. */
#include "duel.h"

int duel_list_card_level(unsigned int p, int i);
int duel_card_dfac8(unsigned int p, int id, int field);
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
int duel_zone_faceup(unsigned int p, int z, int i);
int duel_zone_f_card_usable(unsigned int p, unsigned int q, int i);
int card_is_1a5c_or_1a53(int id);
int card_is_in_group_1a5c(int id);
int duel_088A6B5C(unsigned int p, int id);
int duel_card_e5b88(int id, int a);
int card_get_kind(int id);
extern unsigned int duel_w_08914920;
int card_is_in_group_1a75(int id);
int duel_088A815C(unsigned int p, int id);
int duel_088A8058(unsigned int p, int z, unsigned int i);
int duel_088A79F0(unsigned int p, int id);
void duel_088BBF88(unsigned int p, int b, int c);
int card_is_in_group_1a90(int id);
extern unsigned int duel_w_089147E4;

typedef struct DuelCandHandler {       /* 8: list handler of an effect key */
    unsigned int key;
    void (*fn)(unsigned int p, unsigned int key, int c);
} DuelCandHandler;

typedef struct DuelCandList {          /* 0x08915258: cards collected by the list handlers */
    unsigned int n;
    DuelCard cards[512];               /* +0x004 (duel_w_0891525C) */
    unsigned short zones[512];         /* +0x804 */
} DuelCandList;

extern DuelCandList duel_w_08915258;
extern DuelCandHandler duel_w_08912988[];

void duel_088BFC64(unsigned int p, int zone, unsigned int i);
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

void duel_088BE638(unsigned int p, int b, unsigned int c)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_in_group_1a6x(id) && card_get_level(id) <= c) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BE70C(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_monster(id) && card_get_type(id) == 0x11 && duel_088DFCC8(CARD_UID(duel.players[p & 1].hand[i]))) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088BE7D4(unsigned int p, int b)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_f; i++) {
        id = duel.players[p & 1].zone_f[i].id;
        if (card_is_monster(id) && card_get_type(id) == card_get_type(b) && duel_zone_faceup(p, 0xF, i)) {
            duel_088BFC64(p, 0xF, i);
        }
    }
}

void duel_088BE8C8(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (duel_card_e5b88(id, card_get_kind(b)) && duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088BE96C(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_monster(id) && card_get_type(id) == 0x11 && duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088BEA18(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_f; i++) {
        id = duel.players[p & 1].zone_f[i].id;
        if (card_is_monster(id) && card_get_type(id) == 0x11 && duel_zone_f_card_usable(p, p, i) && duel_zone_faceup(p, 0xF, i)) {
            duel_088BFC64(p, 0xF, i);
        }
    }
}

void duel_088BEB10(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_18; i++) {
        id = duel.players[p & 1].zone_c[i].id;
        if (card_is_1a5c_or_1a53(id) && duel_088A6B5C(p, id)) {
            duel_088BFC64(p, 0xC, i);
        }
    }
}

void duel_088BEBD8(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_in_group_1a5c(id) && card_get_type(id) == 7 && duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i])) < 5 &&
        duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

#ifdef NON_MATCHING /* o original lê o uid da fonte por um ponteiro em $a1 e troca a3/t0 entre o uid e o tamanho da cadeia */
void duel_088BECC0(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;
    unsigned int j;
    int uid;

    if (card_is_monster(id) && duel_card_dfac8(p, id, 0)) {
        uid = CARD_UID(duel.players[p & 1].hand[i]);
        for (j = 0; j < duel_w_08914920; j++) {
            if (duel_chain.kinds[j] == 0x19 && duel_chain.srcs[j].s_player == p && ((DuelSrc *)&duel_chain.srcs[j])->uid == uid) {
                duel_088BFC64(p, 0xB, i);
                return;
            }
        }
    }
}
#endif

void duel_088BEDEC(unsigned int p, int b)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        if (card_is_monster(id) && card_is_in_group_1a6x(id) && id != b && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BEEDC(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_in_group_1a6x(id) && duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i])) < 5 && duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088BEFB0(unsigned int p)
{
    unsigned int i;
    int id;
    DuelCard *c;
    int id2;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        id = duel.players[p & 1].grave[i].id;
        c = &duel.players[p & 1].grave[i];
        if (card_is_monster(id) && card_is_in_group_1a6x(id) && !c->b20 && c->b19 && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
    for (i = 0; i < duel.players[(1 - p) & 1].n_grave; i++) {
        id2 = duel.players[(1 - p) & 1].grave[i].id;
        c = &duel.players[(1 - p) & 1].grave[i];
        if (card_is_monster(id2) && card_is_in_group_1a6x(id2) && !c->b20 && c->b19 && duel_grave_card_usable(p, 1 - p, i)) {
            duel_088BFC64(1 - p, 0xE, i);
        }
    }
}

void duel_088BF190(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_in_group_1a6f(id) && duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088BF228(unsigned int p, int b, int i)
{
    int id = duel.players[p & 1].hand[i].id;

    if (card_is_in_group_1a75(id) && duel_card_dfac8(p, id, 0)) {
        duel_088BFC64(p, 0xB, i);
    }
}

void duel_088BF2C0(unsigned int p)
{
    unsigned int i;
    int id;

    if (duel_pick_st_zone(p) >= 0) {
        for (i = 0; i < duel.players[p & 1].n_d; i++) {
            id = duel.players[p & 1].zone_d[i].id;
            if (card_get_type(id) == 0x16 && card_get_prop5_hi(id) == 5 && !duel_card_e59b0(id)) {
                duel_088BFC64(p, 0xD, i);
            }
        }
    }
}

void duel_088BF3A8(unsigned int p, int b, int c)
{
    unsigned int i;

    if (c == 0) {
        for (i = 0; i < duel.players[p & 1].n_18; i++) {
            if (duel_088A815C(p, duel.players[p & 1].zone_c[i].id)) {
                duel_088BFC64(p, 0xC, i);
            }
        }
    } else {
        for (i = 0; i < duel.players[p & 1].n_d; i++) {
            if (duel_088A8058(p, 0xD, i)) {
                duel_088BFC64(p, 0xD, i);
            }
        }
    }
}

void duel_088BF4C4(unsigned int p, int b, int c)
{
    unsigned int i;
    int id;

    if (c == 0) {
        for (i = 0; i < duel.players[p & 1].n_18; i++) {
            id = duel.players[p & 1].zone_c[i].id;
            if (card_get_kind(id) == 2 && card_get_type(id) == 7 && duel_088A79F0(p, id)) {
                duel_088BFC64(p, 0xC, i);
            }
        }
    } else {
        duel_088BBF88(p, b, c);
    }
}

void duel_088BF5B8(unsigned int p, int b, unsigned int c)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_get_type(id) == 9 && card_get_level(id) <= c && duel_card_dfac8(p, id, 0)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BF6A8(unsigned int p)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_get_type(id) == 9 && card_get_level(id) < 5 && duel_card_dfac8(p, id, 0)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BF78C(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        if (card_is_in_group_1a90(duel.players[p & 1].zone_d[i].id)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BF834(unsigned int p, int b, int lv)
{
    unsigned int i;
    int id;

    for (i = 0; i < duel.players[p & 1].n_d; i++) {
        id = duel.players[p & 1].zone_d[i].id;
        if (card_is_monster(id) && lv == card_get_level(id) && card_is_in_group_1a90(id) && duel_card_dfac8(p, id, 0)) {
            duel_088BFC64(p, 0xD, i);
        }
    }
}

void duel_088BF92C(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == 9 && duel_grave_card_usable(p, p, i)) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BF9F0(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == 9) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

void duel_088BFA9C(unsigned int p)
{
    unsigned int i;

    for (i = 0; i < duel.players[p & 1].n_grave; i++) {
        if (card_get_type(duel.players[p & 1].grave[i].id) == 9) {
            duel_088BFC64(p, 0xE, i);
        }
    }
}

#ifdef NON_MATCHING /* o original copia z para $s2 só na entrada do laço; o mwcc copia no início e testa o switch em $s1 */
void duel_088BFB48(unsigned int p, int b, int z)
{
    int n;
    int i;

    switch (z) {
    case 0xE:
        n = duel.players[p & 1].n_grave;
        break;
    case 0xC:
        n = duel.players[p & 1].n_18;
        break;
    case 0xF:
        n = duel.players[p & 1].n_f;
        break;
    case 0xD:
        n = duel.players[p & 1].n_d;
        break;
    }
    for (i = 0; i < n; i++) {
        duel_088BFC64(p, z, i);
    }
}
#endif

void duel_088BFC64(unsigned int p, int zone, unsigned int i)
{
    switch (zone) {
    case 0xC:
        duel_w_08915258.cards[duel_w_08915258.n] = duel.players[p & 1].zone_c[i];
        duel_w_08915258.zones[duel_w_08915258.n] = 0xC;
        break;
    case 0xD:
        duel_w_08915258.cards[duel_w_08915258.n] = duel.players[p & 1].zone_d[i];
        duel_w_08915258.zones[duel_w_08915258.n] = 0xD;
        break;
    case 0xE:
        duel_w_08915258.cards[duel_w_08915258.n] = duel.players[p & 1].grave[i];
        duel_w_08915258.zones[duel_w_08915258.n] = 0xE;
        break;
    case 0xF:
        duel_w_08915258.cards[duel_w_08915258.n] = duel.players[p & 1].zone_f[i];
        duel_w_08915258.zones[duel_w_08915258.n] = 0xF;
        break;
    case 0xB:
        duel_w_08915258.cards[duel_w_08915258.n] = duel.players[p & 1].hand[i];
        duel_w_08915258.zones[duel_w_08915258.n] = 0xB;
        break;
    default:
        duel_w_08915258.cards[duel_w_08915258.n] = duel.players[p & 1].field[zone].card;
        duel_w_08915258.zones[duel_w_08915258.n] = zone;
        break;
    }
    duel_w_08915258.n += duel_w_08915258.n < 0x1FF;
}

static int duel_088BFF08(unsigned int key)
{
    int lo = 0;
    int hi = 0x16C;
    int mid;

    do {
        mid = (lo + hi) / 2;
        if (duel_w_08912988[mid].key < key) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    } while (lo < hi);
    if (key == duel_w_08912988[lo].key) {
        return lo;
    }
    return -1;
}

unsigned int duel_088BFF78(unsigned int p, unsigned int key, int c)
{
    int k = duel_088BFF08(key & 0xFFFF);

    duel_w_08915258.n = 0;
    if (k >= 0) {
        duel_w_08912988[k].fn(p, key, c);
    }
    return duel_w_08915258.n;
}

int duel_088BFFD8(unsigned int p, unsigned int key, int c)
{
    int k = duel_088BFF08(key & 0xFFFF);
    unsigned int n = duel_w_08915258.n;
    unsigned int m;

    if (k >= 0) {
        duel_w_08912988[k].fn(p, key, c);
    }
    m = duel_w_08915258.n;
    duel_w_08915258.n = n;
    return m - n;
}
