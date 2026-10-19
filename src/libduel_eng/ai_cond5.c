// @range 0x088137BC-0x08814EC8
// @asm 0x08813A6C-0x08813BFC
// @asm 0x088140DC-0x08814248
// @asm 0x08814248-0x08814384
// @asm 0x08814384-0x08814548
// @asm 0x08814548-0x08814618
// @asm 0x08814684-0x08814744
// @asm 0x08814744-0x08814924
/* Conditions of the computer player against a card being activated (b, may be null). */
#include "duel.h"

extern int duel_w_08913D08;
extern int duel_phase;

unsigned int duel_act_target(DuelAct *a, int i);
int duel_act_target2(DuelAct *a, int i);
int card_get_type(unsigned short id);
unsigned int duel_act_e3920(DuelAct *a);
int duel_act_e30c0(DuelAct *a);
int duel_08807B38(int id, int a, int b);
int duel_0886F89C(DuelAct *b, unsigned int p, int z);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int duel_0884E948(DuelAct *a, unsigned int p);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
int duel_08810AE0(DuelAct *a);
int duel_count_monsters(unsigned int p);
int duel_08813A40(DuelAct *a, DuelAct *b);
int duel_08813A6C(DuelAct *a, DuelAct *b);
int card_is_monster(int id);
int card_get_prop5_hi(unsigned short id);
unsigned short duel_slot_src_attr8(unsigned int p, int z, int active);
int card_is_attribute8(unsigned short id);
int duel_pick_st_zone(unsigned int p);
int duel_0886CE38();
int duel_0886CD1C(DuelAct *a, unsigned int p);
int duel_count_field_cards(unsigned int p, int faceup, int facedown, int fzone);
int duel_08814684(DuelAct *a, DuelAct *b);
int card_get_limit(unsigned short id);
int duel_0880AA04(int id, int a);
int duel_0881A038(DuelAct *a, DuelAct *b);
int card_is_attribute9(int id);
unsigned int duel_field_controller(unsigned int p, int z, int arg);
int card_is_in_group_1a90(int id);
int duel_count_field_all(unsigned int p);
int duel_field_level(unsigned int p, int z);
int duel_088BFFD8(unsigned int p, int id, int i);
void duel_act_0887E140(DuelAct *a, int d[2]);
int duel_0880F8C4(DuelAct *a, DuelAct *b);
int duel_08809FC4();
int duel_count_monsters_state(unsigned int p, int pos, int state);

int duel_088137BC(DuelAct *a, DuelAct *b)
{
    int i;

    if (b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    for (i = 0; i < b->n_targets; i++) {
        unsigned int t = duel_act_target(b, i);
        unsigned int z = (unsigned short)t >> 8 & 0xFF;

        if ((t & 0xFF) != a->player && z == 0xE) {
            int u = duel_act_target2(b, i);

            if (card_get_type(duel.list[u].id) == 0x16) {
                duel_w_08913D08 = u;
                return 1;
            }
        }
    }
    return 0;
}

int duel_088138C0(DuelAct *a, DuelAct *b)
{
    int i;

    if (b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    for (i = 0; i < b->n_targets; i++) {
        unsigned int t = duel_act_target(b, i);
        unsigned int z = (unsigned short)t >> 8 & 0xFF;

        if ((t & 0xFF) != a->player && z == 0xE) {
            duel_w_08913D08 = duel_act_target2(b, i);
            return 1;
        }
    }
    return 0;
}

int duel_08813990(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    return b->player != a->player;
}

int duel_088139BC(DuelAct *a, DuelAct *b)
{
    switch (b->id) {
    case 0x12E5:
    case 0x1303:
    case 0x18FE:
    case 0x149C:
    case 0x1A5C:
    case 0x1920:
    case 0x1921:
    case 0x1A7F:
    case 0x1A7E:
        return 1;
    }
    return 0;
}

int duel_08813A40(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    return b->player != a->player;
}

#ifdef NON_MATCHING /* desvios do switch */
int duel_08813A6C(DuelAct *a, DuelAct *b)
{
    unsigned int t;
    unsigned int z;
    unsigned int p;
    int n;

    if (b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    switch (duel_act_e3920(b)) {
    case 1:
    case 2:
        if (b->n_targets != 1) {
            return 1;
        }
        t = duel_act_target(b, 0);
        z = (unsigned short)t >> 8 & 0xFF;
        p = t & 0xFF;
        switch (duel.players[p & 1].field[z].card.id) {
        case 0x10DD:
        case 0x12FE:
        case 0x12FF:
        case 0x1435:
        case 0x15F1:
        case 0x14DE:
            return 0;
        default:
            return 1;
        }
    case 4:
    case 6:
        n = duel_0884E440(b, a->player, a->zone);
        return duel_0884E948(b, a->player) - n > 0;
    default:
        return 1;
    }
}
#endif

int duel_08813BFC(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    if (card_get_type(b->id) != 0x16) {
        return 0;
    }
    return duel_08813A40(a, b);
}

int duel_08813C58(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    if (card_get_type(b->id) != 0x17) {
        return 0;
    }
    return duel_08813A40(a, b);
}

int duel_08813CB4(DuelAct *a, DuelAct *b)
{
    if (duel.players[a->player & 1].lp > 2500) {
        return 0;
    }
    if (b != 0) {
        return duel_08813A6C(a, b);
    }
    return duel_08810AE0(a);
}

int duel_08813D24(DuelAct *a)
{
    return duel_count_monsters(a->player) > 2;
}

int duel_08813D4C(DuelAct *a, DuelAct *b)
{
    if (duel_chain.n_kinds == 0) {
        return 0;
    }
    if (duel_chain.kinds[0] < 6) {
        return 0;
    }
    switch (duel_chain.kinds[0]) {
    case 0x12:
    case 0x13:
        return 0;
    default:
        return duel_08813A6C(a, b);
    }
}

int duel_08813DC0(DuelAct *a, DuelAct *b)
{
    unsigned int t;

    if (b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    t = duel_act_target(b, 0);
    return duel_0884E3E4(a, t & 0xFF, ((unsigned short)t >> 8) & 0xFF);
}

int duel_08813E28(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    return duel_act_e30c0(b) != 0;
}

int duel_08813E74(DuelAct *a)
{
    unsigned int i;
    int r;

    for (i = 0; i < duel.players[a->player & 1].n_hand; i++) {
        r = !duel_08807B38(duel.players[a->player & 1].hand[i].id, 0, 1);
        if (!r) {
            return 1;
        }
    }
    return 0;
}

int duel_08813F4C(DuelAct *a, DuelAct *b)
{
    int z;
    int r;

    if (b == 0) {
        return 0;
    }
    if (a->player != (duel_act_target(b, 0) & 0xFF)) {
        return 0;
    }
    for (z = 0; z <= 10; z++) {
        r = !duel_0886F89C(b, 1 - a->player, z);
        if (!r) {
            return 1;
        }
    }
    return 0;
}

int duel_08813FF8(DuelAct *a, DuelAct *b)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;


    if (b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    if (card_get_type(b->id) != 0x16) {
        return 0;
    }
    if (b->n_targets == 1 && (unsigned char)((unsigned short)duel_act_target(b, 0) >> 8) < 11) {
        t = duel_act_target(b, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (p == a->player && duel_0884E3E4(a, p, z) != 0) {
            return 1;
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* ordem dos desvios e registradores */
int duel_088140DC(DuelAct *a, DuelAct *b)
{
    unsigned int p;
    unsigned int z;

    if (b == 0) {
        return 0;
    }
    p = b->player;
    if (p != a->player) {
        return 0;
    }
    z = b->zone;
    if (z < 5 || z > 9 || b->uid != CARD_UID(duel.players[p].field[z].card)
        || card_is_monster(b->id) != 0 || card_get_prop5_hi(b->id) == 3 || card_get_prop5_hi(b->id) == 4) {
        return 0;
    }
    switch (b->id) {
    case 0x1102:
    case 0x1231:
    case 0x1238:
    case 0x1400:
    case 0x1514:
    case 0x159C:
        return 0;
    }
    return 1;
}
#endif

#ifdef NON_MATCHING /* ordem dos desvios e registradores */
int duel_08814248(DuelAct *a, DuelAct *b)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;
    unsigned short *fu;
    unsigned short *ps;
    unsigned short f;
    unsigned short pos;
    int r;

    if (b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    if (b->n_targets == 1 && (unsigned char)((unsigned short)duel_act_target(b, 0) >> 8) < 11) {
        t = duel_act_target(b, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E3E4(a, p, z) != 0) {
            fu = &duel.players[p & 1].field[z].faceup;
            ps = &duel.players[p & 1].field[z].pos;
            f = *fu;
            pos = *ps;
            *fu = 0;
            *ps = 1;
            r = duel_0884E3E4(b, p, z);
            *fu = f;
            *ps = pos;
            if (r == 0) {
                return 1;
            }
        }
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* ordem dos desvios e registradores */
int duel_08814384(DuelAct *a, DuelAct *b)
{
    unsigned int t;
    unsigned int z;
    unsigned int p;
    unsigned short s;

    if (b != 0 && b->player != a->player && b->n_targets == 1 && (duel_act_target(b, 0) & 0xFFFF) >> 8 < 0xB) {
        t = duel_act_target(b, 0);
        z = (t & 0xFFFF) >> 8;
        p = a->player;
        if ((t & 0xFF) != p) {
            return 0;
        }
        s = duel_slot_src_attr8(p, z, 0);
        if (s != 0xFFFF && (s & 0xFF) == a->player) {
            z = s >> 8;
        }
        if (card_is_attribute8(duel.players[a->player & 1].field[z].card.id) != 0) {
            if (z < 5) {
                a->w[2] = 1;
                if (duel_pick_st_zone(a->player) >= 0 && duel_0884E3E4(a, a->player, z) != 0) {
                    return 1;
                }
            } else {
                a->w[2] = 2;
                if (duel_0886CE38(a, b) != 0 && duel_0884E3E4(a, a->player, z) != 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* ordem dos desvios e registradores */
int duel_08814548(DuelAct *a, DuelAct *b)
{
    if (duel_0886CD1C(a, 1 - a->player) == 3) {
        if (duel_count_monsters(1 - a->player) == 0) {
            return 1;
        }
        if (b == 0) {
            return 0;
        }
        if (b->player == a->player) {
            return 0;
        }
        if (b->n_targets != 0) {
            if ((unsigned char)((unsigned short)duel_act_target(b, 0) >> 8) > 10) {
                return 1;
            }
        }
        return 0;
    }
    return 0;
}
#endif

int duel_08814618(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    switch (b->id) {
    case 0x12D5:
        return 1;
    case 0x1325:
        return duel.players[(1 - a->player) & 1].n_hand >= 4;
    }
    return 0;
}

#ifdef NON_MATCHING /* ordem dos desvios e registradores */
int duel_08814684(DuelAct *a, DuelAct *b)
{
    unsigned int t;

    if (b == 0) {
        return 0;
    }
    if (b->n_targets == 0) {
        return (int)duel_act_e3920(b) >= 4;
    }
    t = duel_act_target(b, 0);
    if ((t & 0xFF) == a->player) {
        return (t & 0xFFFF) >> 8 == a->zone;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* desvios das condições */
int duel_08814744(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    if (duel_count_field_cards(1 - a->player, 0, 1, 1) != 0) {
        if (b->id == 0x131B) {
            return 0;
        }
        if (duel_08814684(a, b) != 0) {
            return 1;
        }
    }
    if (b->zone > 10) {
        return 0;
    }
    switch (card_get_prop5_hi(b->id)) {
    case 2:
    case 3:
        if (b->id != 0x1322) {
            return 1;
        }
        if (duel_phase == 1) {
            return 0;
        }
        return 1;
    case 4:
        if (card_get_limit(duel.players[b->player].field[b->zone].card.id) < 3) {
            return 1;
        }
        if (duel_0880AA04(duel.players[b->player].field[b->zone].card.id, 1) != 0) {
            return 1;
        }
        return 0;
    }
    return 0;
}
#endif

int duel_08814924(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    if (duel_0884E948(a, a->player) != 0) {
        return duel_08814684(a, b);
    }
    return 0;
}

int duel_08814984(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    if (duel_0884E948(a, 1 - a->player) != 0) {
        return duel_08814684(a, b);
    }
    return 0;
}

int duel_088149EC(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    if (duel_0881A038(a, b) != 0) {
        return duel_08814684(a, b);
    }
    return 0;
}

int duel_08814A48(DuelAct *a, DuelAct *b)
{
    int i;
    unsigned int t;
    unsigned int p;
    int z;
    unsigned int id;

    if (b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    for (i = 0; i < b->n_targets; i++) {
        t = duel_act_target(b, i);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (p == a->player && z < 5) {
            id = duel.players[p & 1].field[z].card.id;
            if (card_is_monster(id) != 0 && card_is_attribute9(id) == 0 && a->player == duel_field_controller(p, z, 1)) {
                return 1;
            }
        }
    }
    return 0;
}

int duel_08814B90(DuelAct *a, DuelAct *b)
{
    int i;
    unsigned int t;
    unsigned int p;
    int z;
    unsigned int id;

    if (b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    for (i = 0; i < b->n_targets; i++) {
        t = duel_act_target(b, i);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (p == a->player && z < 5) {
            id = duel.players[p & 1].field[z].card.id;
            if (card_is_monster(id) != 0 && card_is_in_group_1a90(id) != 0) {
                if (a->id == 0x1A98 && duel_088BFFD8(a->player, a->id, duel_field_level(p, z)) == 0) {
                    return 0;
                }
                if (a->id == 0x1A9A && duel_count_field_all(1 - a->player) == 0) {
                    return 0;
                }
                return 1;
            }
        }
    }
    return 0;
}

int duel_08814D30(DuelAct *a, DuelAct *b)
{
    if (b == 0) {
        return 0;
    }
    if (b->player == a->player) {
        return 0;
    }
    if (b->zone > 4) {
        return 0;
    }
    return duel.mode & 1;
}

int duel_08814D84(DuelAct *a, DuelAct *b)
{
    int d[2];

    if (b == 0) {
        return 0;
    }
    duel_act_0887E140(b, d);
    return d[a->player] < 0;
}

int duel_08814DD4(DuelAct *a, DuelAct *b)
{
    int x;
    int y;
    int d;

    if (duel_0880F8C4(a, b) != 0 && duel_count_monsters(1 - a->player) != 0) {
        duel_08809FC4(1 - a->player, &x, &d, 1);
        duel_08809FC4(a->player, &y, &d, 1);
        if (!(x < y)) {
            return 1;
        }
    }
    return 0;
}

int duel_08814E70(DuelAct *a, DuelAct *b)
{
    if (duel_0880F8C4(a, b) != 0) {
        if (duel_count_monsters_state(1 - a->player, 1, 1) != 0) {
            return 1;
        }
    }
    return 0;
}
