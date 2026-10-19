// @range 0x08884118-0x08884F18
// @asm 0x08884118-0x0888439C
// @asm 0x088846A4-0x088847A4
// @asm 0x088847A4-0x08884930
// @asm 0x088849F8-0x08884B18
// @asm 0x08884B18-0x08884C6C
/* Card effect handlers (DuelAct *a), after effects_s.c. */
#include "duel.h"

extern int duel_step;
extern int duel_w_089148FC;
extern int duel_w_08914900;
extern DuelCard duel_w_0891525C;
extern unsigned short duel_w_0891968C;
extern unsigned int duel_turn_player;

void duel_088C87A8(unsigned int p, int z, unsigned short id, unsigned short a, unsigned short b);
void duel_088C4604(DuelAct *a, unsigned int p, int n);
int duel_pick_monster_zone(unsigned int p);
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, int d);
int duel_hand_find_uid(unsigned int p, int uid);
int duel_count_free_monster_zones(unsigned int p);
int duel_phase_ok2(unsigned int p);
void duel_088C7478(unsigned int p, int i, DuelCard *c, int a);
void duel_088C77D4();
void duel_088D1A44(DuelAct *a, unsigned int p, int z);
void duel_088C78E0(unsigned int p);
void duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
void duel_088C7880(unsigned int p);
void duel_088C7B90(int n);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
void duel_0884ACD8(unsigned int p, int msg);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
unsigned int duel_act_target(DuelAct *a, int i);
unsigned int duel_card_locate(unsigned int uid);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
void duel_088CACF0(unsigned int p, DuelCard *c, int a);
void duel_08832870(unsigned int p, int a);
int duel_088E9150(int id);
int duel_0887FD20(DuelAct *a, int b);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, int c);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
void duel_08832624(unsigned int p, int a, int b, int c);
void duel_088CC178(unsigned int p, int z, unsigned int v);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_088C7CF0(void);
void duel_088C7BA4(void);
void duel_088C5E70(unsigned int p, int a);

#ifdef NON_MATCHING /* registradores e desvios */
int duel_08884118(DuelAct *a)
{
    int i;
    unsigned char z;
    unsigned int p;

    if (a->b3_4 == 2) {
        switch (duel_step) {
        case 0x80:
            i = duel_hand_find_uid(a->player, a->uid);
            if (i < 0) {
                return 0;
            }
            if (duel_count_free_monster_zones(a->player) != 0 && duel_phase_ok2(a->player) != 0) {
                duel_w_089148FC = i;
                return 0x7F;
            }
            duel_088C7478(a->player, i, &duel.players[a->player & 1].hand[i], 0);
            return 0;
        case 0x7F:
            duel_w_08914900 = duel_pick_monster_zone(a->player);
            duel_0882619C(a->player, &duel.players[a->player & 1].hand[duel_w_089148FC], 1, 1, 1);
            return 0x7E;
        case 0x7E:
            duel_088C87A8(a->player, duel_w_08914900, a->id, 5, a->player);
            duel_088C4604(a, a->player, 1000);
            return 0;
        }
        return 0;
    }
    z = a->zone;
    p = a->player;
    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    duel_088C77D4(1 - p, a->id, 5);
    duel_088D1A44(a, a->player, a->zone);
    duel_088C78E0(1 - a->player);
    return 0;
}
#endif

int duel_0888439C(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    duel_088C97F0(a, p, z, 0, 1);
    return 0;
}

int duel_08884458(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_088BFF78(a->player, a->id, 0) == 0) {
            duel_0884ACD8(a->player, 0xD);
            return 0;
        }
        duel_088C78E0(a->player);
        duel_cmd((a->player ? 0x8000 : 0) | 0x57, 0xD, CARD_UID(duel_w_0891525C), 0);
        return 0x7F;
    case 0x7F:
        duel_088C7880(a->player);
        duel_088C7B90(0x1E);
        duel_088C7880(a->player);
        return 0;
    }
    return 0;
}

int duel_0888456C(DuelAct *a)
{
    unsigned int t;

    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        duel_088C97F0(a, t & 0xFF, (unsigned short)t >> 8 & 0xFF, 0, 0);
    }
    return 0;
}

int duel_088845CC(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].pos == 0) {
        duel_088C97F0(a, p, z, 0, 1);
    }
    return 0;
}

#ifdef NON_MATCHING /* registradores e desvios */
int duel_088846A4(DuelAct *a)
{
    unsigned int loc;

    if (!a->b4_2 && duel_count_free_monster_zones(a->player) != 0 && duel_phase_ok2(a->player) != 0) {
        loc = duel_card_locate(a->src.uid);
        if ((loc & 0xFFFF) >> 8 == 0xE && duel_grave_card_usable(a->player, loc & 0xFF, loc >> 16) != 0) {
            duel_0882619C(a->x.s_player, duel_field_card_deb2c(loc & 0xFF, 0xE, loc >> 16), 1, a->x.s_b14, 0x20);
        }
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* registradores e ordem dos argumentos */
int duel_088847A4(DuelAct *a)
{
    unsigned int loc;
    unsigned int z;
    DuelStats st;

    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].faceup == 0) {
        return 0;
    }
    if (duel_088A9008(a, 0, &loc) != 0) {
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel.players[loc & 1].field[z].card.id != 0 && duel.players[loc & 1].field[z].faceup != 0) {
            duel_field_stats(loc & 0xFF, z, &st);
            if (st.bdef > 0xFFFF) {
                st.bdef = 0xFFFF;
            }
            if (st.batk > 0xFFFF) {
                st.batk = 0xFFFF;
            }
            duel_088CC178(a->player, a->zone, (st.batk & 0xFFFF) | st.bdef << 16);
        }
    }
    return 0;
}
#endif

int duel_08884930(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_act_immune(a, p, z) == 0) {
            duel_cmd((p ? 0x8000 : 0) | 0x1B, z, 1, 0);
            duel_cmd((a->player ? 0x8000 : 0) | 0x1C, 0, 0, 0);
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* registradores e ordem dos argumentos */
int duel_088849F8(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        if (duel_count_free_monster_zones(a->player) == 0) {
            return 0x7E;
        } else {
            DuelCard c = {0};

            c.id = 0x14F8;
            c.owner = a->player;
            duel_0882619C(a->player, &c, 1, 1, 1);
            return 0x7F;
        }
    case 0x7E:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* registradores e ordem dos argumentos */
int duel_08884B18(DuelAct *a)
{
    unsigned int q;
    DuelCard *c;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        q = 1 - a->player;
        if (duel.players[q & 1].n_hand == 0) {
            return 0;
        }
        duel_08832870(q, 0);
        return 0x7F;
    case 0x7F:
        c = duel_field_card_deb2c(1 - a->player, 0xB, duel_w_0891968C);
        duel_088C77D4(1 - a->player, a->id, (unsigned char)((duel_turn_player == a->player) + 6) << 4 | 1, CARD_UID(*c));
        duel_088CACF0(1 - a->player, c, 0x81);
        return 0;
    }
    return 0;
}
#endif

int duel_08884C6C(DuelAct *a, int b)
{
    if (a->kind != 0x23 && duel_088E9150(0x128B) == 0) {
        return 0;
    }
    return duel_0887FD20(a, b);
}

int duel_08884CD0(DuelAct *a)
{
    int p;
    int z;

    for (p = 0; p < 2; p++) {
        for (z = 0; z < 5; z++) {
            if (duel_0884E440(a, p, z) != 0) {
                duel_088C87D8(a, p, z, a->id, 2, 0);
            }
        }
    }
    return 0;
}

int duel_08884D60(DuelAct *a)
{
    int n = 0;
    unsigned int q;
    unsigned int i;
    int k;
    int m;

    switch (a->id) {
    case 0x12CC:
        n = 3;
        q = a->w[0] & 1;
        break;
    case 0x187D:
        n = 2;
        q = a->player;
        break;
    case 0x19B7:
        n = 2;
        q = a->player;
        break;
    }
    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = duel.players[q & 1].n_hand;
        duel_088C5E70(q, n);
        return 0x7F;
    case 0x7F:
        k = 0;
        m = duel.players[q & 1].n_hand - duel_w_089148FC;
        for (i = 0; i < duel.players[q & 1].n_hand; i++) {
            if (duel_slot_effect_with(q, 0xB, 0x12A1, CARD_UID(duel.players[q & 1].hand[i])) != 0) {
                k++;
            }
        }
        m--;
        if (k > 1) {
            m -= k - 1;
        }
        if (m > 0) {
            duel_08832624(q, m, 0, 0);
        }
        return 0x7E;
    }
    return 0;
}
