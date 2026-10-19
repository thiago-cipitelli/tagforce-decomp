// @range 0x08884F18-0x08885B70
// @asm 0x08884F18-0x0888519C
// @asm 0x088859A8-0x08885B70
/* Card effect handlers (DuelAct *a), after effects_t.c. */
#include "duel.h"

extern int duel_step;
extern int duel_w_089148FC;
extern unsigned short duel_w_0891968C;
extern unsigned int duel_turn_player;
extern DuelCard duel_w_0891525C;
extern int duel_tokens_089117A0[4];    /* tokens summoned by 0x12D2 */
extern int duel_tokens_089117B0[2];    /* tokens summoned by 0x1710 */

unsigned int duel_act_target(DuelAct *a, int i);
int card_is_attribute9(int id);
int card_same_name(int a, int b);
int card_attribute_icon_is3(int id);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);
void duel_088C7478(unsigned int p, int i, DuelCard *c, int a);
void duel_088C7490(unsigned int p, int i, int a);
void duel_088C74EC(unsigned int p, int i, int a);
void duel_088C7B90(int n);
void duel_088C78E0(unsigned int p);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
void duel_08832B18(unsigned int p, int z);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, unsigned short c);
int duel_count_free_monster_zones(unsigned int p);
int duel_phase_ok2(unsigned int p);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, int d);
void duel_088C7DD4(unsigned int p, int a, int b);
void duel_088CAB04();
void duel_088C5E70(unsigned int p, int a);
int duel_0884E5E0(DuelAct *a);
void duel_088328E8(unsigned int p, unsigned short id, int a);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_088D1B78(DuelAct *a, unsigned int p, int z);
void duel_088C4604(DuelAct *a, unsigned int p, int n);
int card_get_atk(int id);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_088328FC(unsigned int p);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
void duel_0884ACD8(unsigned int p, int msg);
void duel_08832ADC(int a);
int duel_0886F65C(DuelAct *a, DuelAct *b);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
int duel_pick_monster_zone(unsigned int p);
void duel_088C87A8();
void duel_088CA184(unsigned short loc, unsigned short to);
int duel_can_attack(unsigned int p, int z, int arg);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);

#ifdef NON_MATCHING /* ordem dos laços e registradores */
int duel_08884F18(DuelAct *a)
{
    unsigned int t = duel_act_target(a, 0);
    unsigned int p = t & 0xFF;
    unsigned short id = a->w[2];
    int i;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (card_is_attribute9(id) != 0) {
            return 0;
        }
    case 0x7F:
        for (i = duel.players[t & 1].n_hand - 1; i >= 0; i--) {
            if (card_same_name(duel.players[t & 1].hand[i].id, id) != 0) {
                duel_088C7478(p, i, duel_field_card_deb2c(p, 0xB, i), p != a->player);
            }
        }
        return 0x7E;
    case 0x7E:
        for (i = duel.players[t & 1].n_d - 1; i >= 0; i--) {
            if (card_same_name(duel.players[t & 1].zone_d[i].id, id) != 0) {
                duel_088C7490(p, i, p != a->player);
            }
        }
        for (i = duel.players[t & 1].n_18 - 1; i >= 0; i--) {
            if (card_same_name(duel.players[t & 1].zone_c[i].id, id) != 0) {
                duel_088C74EC(p, i, p != a->player);
            }
        }
        return 0x7D;
    case 0x7D:
        if (card_attribute_icon_is3(id) == 0) {
            duel_088C7B90(0xF);
            duel_088C78E0(p);
        }
        return 0x78;
    }
    return 0;
}
#endif

int duel_0888519C(DuelAct *a)
{
    unsigned int q = a->w[0] & 1;

    if (a->b4_2) {
        return 0;
    }
    duel_088C77D4(q, a->id, (duel_turn_player != q) << 4 | 1, 0);
    return 0;
}

int duel_08885200(DuelAct *a)
{
    int z;
    int p;

    switch (duel_step) {
    case 0x80:
        duel_08832B18(a->player, a->zone);
        return 0x7F;
    case 0x7F:
        for (p = 0; p < 2; p++) {
            for (z = 0; z < 5; z++) {
                if (duel_0884E440(a, p, z) != 0) {
                    duel_088C87D8(a, p, z, a->id, 2, duel_w_0891968C * 100);
                }
            }
        }
        return 0;
    }
    return 0;
}

int duel_088852E8(DuelAct *a)
{
    int n = 0;
    int *tbl = 0;

    switch (a->id) {
    case 0x12D2:
        n = 4;
        tbl = duel_tokens_089117A0;
        break;
    case 0x1710:
        n = 2;
        tbl = duel_tokens_089117B0;
        break;
    }
    if (!a->b4_2) {
        switch (duel_step) {
        case 0x80:
            if (duel_count_free_monster_zones(a->player) < n) {
                return 0;
            }
            if (duel_phase_ok2(a->player) == 0) {
                return 0;
            }
            if (a->kind != 0x23 && (duel.players[a->player & 1].flags >> 16 & 1)) {
                return 0;
            }
            duel_088C7BA4();
            duel_w_089148FC = 0;
            return 0x7F;
        case 0x7F:
            if (duel_w_089148FC < n) {
                DuelCard c = {0};

                c.id = tbl[duel_w_089148FC];
                c.owner = a->player;
                duel_0882619C(a->player, &c, 1, 1, 1);
                duel_w_089148FC++;
                return 0x7F;
            }
            return 0x7E;
        case 0x7E:
            duel_088C7CF0();
            break;
        }
    }
    if (a->kind != 0x23) {
        duel_088C7DD4(a->player, 0x15, 1);
        duel_088C7DD4(a->player, 0x16, 1);
    }
    return 0;
}

int duel_0888553C(DuelAct *a)
{
    int i;
    unsigned int q;
    int k;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        for (i = 0; i < 2; i++) {
            q = duel_turn_player ^ i;
            a->w[q] = duel.players[q & 1].n_hand;
            for (k = duel.players[q & 1].n_hand - 1; k >= 0; k--) {
                duel_088CAB04(q, k, a->player != q);
            }
        }
        return 0x7F;
    case 0x7F:
        duel_088C7BA4();
        duel_088C5E70(duel_turn_player, a->w[duel_turn_player]);
        return 0x7E;
    case 0x7E:
        duel_088C5E70(1 - duel_turn_player, a->w[1 - duel_turn_player]);
        duel_088C7CF0();
        return 0x7D;
    }
    return 0;
}

int duel_08885698(DuelAct *a, DuelAct *b)
{
    if (a->b4_2) {
        return 0;
    }
    b->w[0] = !b->w[0];
    return 0;
}

int duel_088856CC(DuelAct *a)
{
    unsigned int p;
    unsigned int z;
    int atk;

    switch (duel_step) {
    case 0x80:
        if (duel_0884E5E0(a) == 0) {
            return 0;
        }
        duel_088328E8(duel_turn_player, a->id, a->player);
        return 0x7F;
    case 0x7F:
        p = duel_w_0891968C & 0xFF;
        z = duel_w_0891968C >> 8 & 0xFF;
        atk = card_get_atk(duel.players[p & 1].field[z].card.id);
        if (duel_act_immune(a, p, z) != 0) {
            return 0;
        }
        duel_088D1B78(a, p, z);
        duel_088C4604(a, 1 - duel_turn_player, (atk + 1) >> 1);
        return 0;
    }
    return 0;
}

int duel_088857F8(DuelAct *a)
{
    if (!a->b4_2) {
        switch (duel_step) {
        case 0x80:
            if (duel_phase_ok2(a->player) == 0) {
                return 0;
            }
            if (duel_count_free_monster_zones(a->player) == 0) {
                return 0;
            }
            if (a->kind != 0x23 && (duel.players[a->player & 1].flags >> 16 & 1)) {
                return 0;
            }
            if (duel_088BFF78(a->player, a->id, 0) == 0) {
                duel_0884ACD8(a->player, 0xD);
                return 0x7E;
            }
            return 0x7F;
        case 0x7F:
            duel_088263A8(a->player, &duel_w_0891525C, 1, 0);
            return 0x7D;
        case 0x7E:
            duel_088328FC(1 - a->player);
            return 0x7D;
        case 0x7D:
            duel_088C78E0(a->player);
            return 0x7C;
        }
    }
    if (a->kind != 0x23) {
        duel_088C7DD4(a->player, 0x15, 1);
        duel_088C7DD4(a->player, 0x16, 1);
    }
    return 0;
}

#ifdef NON_MATCHING /* registradores */
int duel_088859A8(DuelAct *a, DuelAct *b)
{
    unsigned int t = duel_act_target(a, 0);
    unsigned int z = (unsigned short)t >> 8 & 0xFF;
    unsigned int q = t & 0xFF;
    unsigned int p;
    int s;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_0886F65C(a, b) == 0) {
            return 0;
        }
        if (duel.mode & 1) {
            return 0;
        }
        if (!(duel.mode >> 1 & 1)) {
            return 0;
        }
        if (duel_088A9008(a, 0, 0) == 0) {
            return 0;
        }
        if (duel_0884E440(a, q, z) == 0) {
            return 0;
        }
        p = a->player;
        s = duel_pick_monster_zone(p);
        duel_088C87A8(q, z, a->id, 1);
        duel_088CA184(t, p | (s & 0xFF) << 8);
        if (duel_can_attack(duel_special.player[0], duel_special.zone[0], 0) == 0) {
            return 0;
        }
        duel_cmd((a->player == 1 ? 0 : 0x8000) | 0x19, duel_special.zone[0], s & 0xFF, a->player);
        return 0x7F;
    case 0x7F:
        duel_08832ADC(1);
        return 0;
    }
    return 0;
}
#endif
