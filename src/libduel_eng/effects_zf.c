// @range 0x088917D4-0x08892554
// @asm 0x088918EC-0x08891A58
// @asm 0x08891F38-0x08892344
/* Card effect handlers (DuelAct *a), after effects_ze.c. */
#include "duel.h"

extern int duel_step;
extern int duel_w_089148FC;
extern unsigned short duel_w_0891968C;
extern int duel_player_type[2];
extern int duel_w_08919654;

void duel_088C9344(DuelAct *a, unsigned int p, int z, int b);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, int c);
void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);
void duel_088C7CF0(void);
void duel_088C7BA4(void);
int duel_0886CE38();
int duel_count_free_monster_zones(unsigned int p);
int duel_grave_find_uid(unsigned int p, int uid);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_088C5424(unsigned int p, int n);
int duel_bit_count(unsigned int m);
unsigned int duel_0884E4E4(DuelAct *a);
unsigned int duel_088D13CC(DuelAct *a, unsigned int mask);
int duel_0884E9E8(DuelAct *a, int b);
int duel_hand_count_monsters(unsigned int p);
void duel_088C7810(unsigned int p, int a);
void duel_08832880(unsigned int p, unsigned short id, int a);
void duel_0884ACD8(unsigned int p, void *msg);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
int duel_field_base_atk(unsigned int p, int z);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
unsigned int duel_088D1920(DuelAct *a, unsigned int mask);
int field_get_card(unsigned int p, int z);
int card_is_in_group_160f(int id);
int duel_088BFFD8();
int duel_0887FCA8(int id);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)(int id));
void duel_08835978(int (*fn)(int id));
int duel_08835AC0(void);
void duel_08832950(unsigned int p, unsigned short id);
void duel_088CBAA4(unsigned int p, int z, int uid);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_slot_foreach_effect(unsigned int p, int z, int (*fn)(DuelEffect *e, int *ctx), int *ctx);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);

int duel_088917D4(DuelAct *a)
{
    int z;
    unsigned int q;

    for (z = 0; z < 5; z++) {
        q = 1 - a->player;
        if (duel.players[q & 1].field[z].card.id != 0) {
            if (duel.players[q & 1].field[z].pos != 0) {
                duel_088C97F0(a, q, z, !duel.players[q & 1].field[z].faceup, 2);
            } else if (duel.players[q & 1].field[z].faceup == 0) {
                duel_088C9344(a, q, z, 2);
            }
            duel_088C87D8(a, 1 - a->player, z, a->id, 3, 500);
        }
    }
    duel_088C77D4(1 - a->player, a->id, 1, 0);
    return 0;
}

#ifdef NON_MATCHING /* retorno 0x7E com `li; b; nop` após o teste de zonas livres; despacho do switch em outros registradores */
int duel_088918EC(DuelAct *a)
{
    int i;

    switch (duel_step) {
    case 0x80:
        if (duel_0886CE38() == 0) {
            return 0;
        }
        duel_088C7BA4();
        duel_w_089148FC = 0;
        return 0x7F;
    case 0x7F:
        if (duel_w_089148FC < a->n7) {
            if (duel_count_free_monster_zones(a->player) == 0) {
                return 0x7E;
            }
            i = duel_grave_find_uid(a->player, a->w[duel_w_089148FC]);
            if (i >= 0 && duel_grave_card_usable(a->player, a->player, i) != 0) {
                duel_088263A8(a->player, &duel.players[a->player & 1].grave[i], 1, a->id << 16);
            }
            duel_w_089148FC++;
            return 0x7F;
        }
        return 0x7E;
    case 0x7E:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}
#endif

int duel_08891A58(DuelAct *a)
{
    unsigned int m;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        m = duel_0884E4E4(a);
        duel_w_089148FC = duel_bit_count(m & duel_088D13CC(a, m));
        return 0x7F;
    case 0x7F:
        switch (a->id) {
        case 0x14AA:
            duel_088C4604(a, 1 - a->player, duel_w_089148FC * 500);
            break;
        case 0x16D7:
            duel_088C4604(a, 1 - a->player, duel_w_089148FC * 300);
            break;
        case 0x1665:
            duel_088C5424(a->player, duel_w_089148FC * 800);
            break;
        case 0x170F:
            duel_088C5424(a->player, duel_w_089148FC * 500);
            break;
        case 0x18DC:
            duel_088C4604(a, 1 - a->player, duel_w_089148FC * 300);
            break;
        case 0x19B0:
            duel_088C4604(a, 1 - (a->player ^ a->opp), duel_w_089148FC * 100);
            break;
        }
        return 0;
    }
    return 0;
}

int duel_08891C6C(DuelAct *a, int b)
{
    switch (duel_step) {
    case 0x80:
        if (duel_0884E9E8(a, b) == 0) {
            return 0;
        }
        if (duel_hand_count_monsters(1 - a->player) == 0) {
            duel_088C7810(1 - a->player, 1);
            return 0x6E;
        }
        duel_08832880(a->player, a->id, 1);
        return 0x7F;
    case 0x7F:
        duel_cmd(0x3F,
                 (1 - a->player & 1 | 0x16) |
                     (CARD_UID(duel.players[1 - a->player & 1].hand[duel_w_0891968C]) & 0x3FF) << 6,
                 (a->player & 1 | 0x16) | (duel.players[a->player & 1].n_hand & 0x3FF) << 6, 0);
        return 0;
    case 0x6E:
        duel_0884ACD8(a->player, (void *)0x11);
        return 100;
    case 100:
        duel_088C7810(1 - a->player, 0);
        return 0;
    }
    return 0;
}

int duel_08891E0C(DuelAct *a)
{
    unsigned int t0;
    unsigned int t1;
    unsigned int p0;
    unsigned int z0;
    unsigned int p1;
    unsigned int z1;
    int atk0;
    int atk1;

    if (a->n7 == 2) {
        t0 = duel_act_target(a, 0);
        t1 = duel_act_target(a, 1);
        p0 = t0 & 0xFF;
        z0 = (unsigned short)t0 >> 8 & 0xFF;
        p1 = t1 & 0xFF;
        z1 = (unsigned short)t1 >> 8 & 0xFF;
        if (duel_088A9008(a, 0, 0) != 0 && duel_088A9008(a, 1, 0) != 0) {
            atk0 = duel_field_base_atk(p0, z0);
            atk1 = duel_field_base_atk(p1, z1);
            duel_088C87A8(p0, z0, a->id, 2, atk1 > 0xFFFF ? 0xFFFF : atk1);
            duel_088C87A8(p1, z1, a->id, 2, atk0 > 0xFFFF ? 0xFFFF : atk0);
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* q e z do laço que enche duel_chain.uids em a3/t2 trocados */
int duel_08891F38(DuelAct *a, DuelAct *b)
{
    int i;
    unsigned int mask;
    unsigned int t;
    int z;
    unsigned int p;
    int q;
    unsigned int uid;

    switch (duel_step) {
    case 0x80:
        mask = 0;
        for (i = 0; i < b->n_targets; i++) {
            t = duel_act_target(b, i);
            z = (unsigned short)t >> 8 & 0xFF;
            if (z < 5) {
                p = t & 0xFF;
                if (duel.players[p & 1].field[z].faceup != 0 && card_is_in_group_160f(field_get_card(p, z))) {
                    mask |= 1 << (z + p * 16);
                }
            }
        }
        mask = duel_088D1920(a, mask);
        if (mask == 0) {
            return 0;
        }
        duel_chain.uids[duel_chain.uids_head & 0x7F] = 0;
        for (q = 0; q < 2; q++) {
            for (z = 0; z <= 4; z++) {
                if (mask & 1 << (z + q * 16)) {
                    uid = CARD_UID(duel.players[q & 1].field[z].card);
                    duel_chain.uids[duel_chain.uids_head & 0x7F]++;
                    duel_chain.uids[duel_chain.uids_head + duel_chain.uids[duel_chain.uids_head & 0x7F] & 0x7F] = uid;
                }
            }
        }
        return 0x7F;
    case 0x7F:
        if (duel_0886CE38() == 0) {
            return 0;
        }
        for (i = 0; i < duel.players[a->player & 1].n_hand; i++) {
            if (!duel_088BFFD8(a->player, a->id, i)) {
                continue;
            }
            duel_0884ACD8(a->player, (void *)0x11E);
            return 0x7E;
        }
        duel_0884ACD8((int)a->player, (void *)0x122);
        return 0x78;
    case 0x7E:
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, a->id, duel_0887FCA8);
        } else {
            duel_08835978(duel_0887FCA8);
        }
        return 0x7D;
    case 0x7D:
        if (duel_08835AC0() != 0) {
            duel_088263A8(a->player, &duel.players[a->player & 1].hand[duel_w_08919654], 1, a->id << 16);
            return 0;
        }
        return 0x7F;
    case 0x78:
        duel_088C7810(a->player, 1);
        duel_08832950(1 - a->player, a->id);
        return 0x77;
    case 0x77:
        duel_088C7810(a->player, 0);
        return 0;
    }
    return 0;
}
#endif

int duel_08892344(DuelAct *a, DuelAct *b)
{
    unsigned char z;

    b->b4_2 = 1;
    duel_088CBAA4(b->player, b->zone, b->uid);
    z = b->zone;
    if (z >= 5 && z < 11 && b->uid == CARD_UID(duel.players[b->player & 1].field[z].card)) {
        duel_088C87A8(b->player, z, 0x17C2, 3, 0);
    }
    return 0;
}

int duel_0889243C(DuelAct *a)
{
    int p;
    int z;

    for (p = 0; p < 2; p++) {
        for (z = 0; z <= 4; z++) {
            if (duel_0884E440(a, p, z) != 0) {
                duel_088C87D8(a, p, z, a->id, 5, a->player);
            }
        }
    }
    return 0;
}

int duel_088924D0(DuelEffect *e, int *ctx)
{
    if (e->src == 0x14C0 && ((DuelEffectBits *)e)->kind_hi == 0) {
        duel_088C5424(((DuelAct *)ctx)->player, (unsigned int)e->arg >> 1);
    }
    return 0;
}

int duel_08892520(DuelAct *a)
{
    duel_slot_foreach_effect(a->player, 0xB, duel_088924D0, (int *)a);
    return 0;
}
