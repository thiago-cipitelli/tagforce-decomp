// @range 0x0889CD38-0x0889E688
// @data 0x08911974-0x08911990
/* Card effect handlers (DuelAct *a), after effects_0889C150.c. */
#include "duel.h"

extern int duel_step;
extern int duel_turn_player;
extern int duel_w_089148FC;
extern unsigned short duel_h_0891968E;
extern unsigned short duel_w_0891968C;
extern unsigned int duel_turn;
extern DuelAct *duel_w_089147DC;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;

void duel_088CC4B4(unsigned int p, int z, int n);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
void duel_088D192C(DuelAct *a, unsigned int p, int z);
void duel_088D1858(DuelAct *a, unsigned int p, int z);
void duel_088D147C(DuelAct *a, unsigned int p, int z);
DuelCard *duel_card_by_uid(int uid);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_088329D0(unsigned int p, int a, int b);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
int duel_phase_ok2(unsigned int p);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
int duel_count_free_monster_zones(unsigned int p);
void duel_088C78E0(unsigned int p);
void duel_088C7880(unsigned int p);
void duel_088CB980(unsigned int p);
void duel_088328FC(unsigned int p);
void duel_08832950(unsigned int p, unsigned short id);
int card_is_monster(int id);
int duel_card_dfac8(unsigned int p, int id, int field);
void duel_08832880(unsigned int p, unsigned short id, int a);
void duel_088CAB04(unsigned int p, int i, int b);
void duel_088C452C(unsigned int p, int n);
unsigned int duel_card_locate(unsigned int uid);
int duel_field_atk(unsigned int p, int z);
int card_get_atk(int id);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_08832B18(unsigned int p, int z);
int duel_08835AC0(void);
void duel_0884ACD8(unsigned int p, int msg);
void duel_088359E8(unsigned int p, int id, int (*fn)());
int duel_088559C0();
void duel_08832624(unsigned int p, int a, int b, int c);
void duel_088C5E70(unsigned int p, int a);
int duel_0884E690(DuelAct *a, int (*fn)());
void duel_088C625C(unsigned int p, int a, int b);
void duel_08832960(unsigned int p, unsigned short id, int a);
int duel_0884AF9C(int a, unsigned short b);
void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);
int duel_0887F728(DuelAct *a, int b);
int duel_0887DDD0();
int field_get_card(unsigned int p, int z);
int card_same_name(int id, int id2);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
void duel_088C4508(unsigned int p, int id);
int duel_zone_faceup(unsigned int p, int z, int i);
int card_is_group_b(int id);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
void duel_088D1C70(DuelAct *a, unsigned int p, int z);
void duel_088C7810(unsigned int p, int a);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)());
void duel_08835978(int (*fn)());
int duel_0884E5E0(DuelAct *a);
int duel_08881160(DuelAct *a, int b);
DuelCard *duel_0884CCBC(int k);
void duel_088CAF24(DuelAct *a, unsigned int p, DuelCard *c);
void duel_0884C588(unsigned int p, int a, int id, int b);
void duel_088327F8(unsigned int p, int a, int b);
int duel_088A2130(DuelAct *a, DuelAct *b);
void duel_088CBAA4(unsigned int p, int z, int uid);
int duel_088D15B4(DuelAct *a, unsigned int p, int z);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);

int duel_0889CD38(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    duel_088CC4B4(p, z, 1);
    return 0;
}

int duel_0889CDF4(DuelAct *a)
{
    unsigned int t = a->idx;
    unsigned int p = t & 0xFF;
    int z = (unsigned char)((t & 0xFFFF) >> 8);

    if (duel_slot_has_effect(p, z, a->id)) {
        switch (a->id) {
        case 0x1700:
            duel_088D1858(a, p, z);
            break;
        case 0x1749:
            duel_088D192C(a, p, z);
            break;
        case 0x1703:
            if (duel.players[p & 1].field[z].pos == 0) {
                duel_088C97F0(a, p, z, 0, 1);
            }
            break;
        }
    }
    return 0;
}

int duel_0889CEF4(DuelAct *a)
{
    unsigned int t = a->idx;
    unsigned int p = t & 0xFF;
    int z = (unsigned char)((t & 0xFFFF) >> 8);

    if (duel_slot_has_effect(p, z, a->id)) {
        duel_088D147C(a, p, z);
    }
    return 0;
}

int duel_0889CF60(DuelAct *a)
{
    int p = duel_w_089148FC;
    int r = 0x78;

    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = duel_turn_player;
        a->w[2] = duel_phase_ok2(0);
        a->w[3] = duel_phase_ok2(1);
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        if (a->w[p + 2] == 0) {
            return r;
        }
        a->w[1] = MIN(duel_count_free_monster_zones(p), duel_088BFF78(p, a->id, 0));
        if (a->w[1] == 0) {
            return 0x78;
        }
        return 0x7E;
    case 0x7E:
        duel_088329D0(p, a->id, a->w[1]);
        a->w[0] = 0;
        return 0x7D;
    case 0x7D:
        if (a->w[0] < a->w[1]) {
            duel_088263A8(p, duel_card_by_uid((&duel_h_0891968E)[a->w[0]]), 1, 0);
            a->w[0]++;
            return 0x7D;
        }
        return r;
    case 0x78:
        duel_w_089148FC = 1 - p;
        if (duel_w_089148FC != duel_turn_player) {
            return 0x7F;
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_0889D150(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_088BFF78(1 - a->player, a->id, 0) == 0) {
            return 0x78;
        }
        duel_088329D0(1 - a->player, a->id, 0);
        return 0x7F;
    case 0x7F:
        duel_088C78E0(1 - a->player);
        duel_cmd(((1 - a->player) ? 0x8000 : 0) | 0x57, 0xD, duel_h_0891968E, 0);
        duel_088C7880(1 - a->player);
        duel_088CB980(1 - a->player);
        duel_088C7880(1 - a->player);
        return 0;
    case 0x78:
        duel_08832950(1 - a->player, a->id);
        return 0x77;
    case 0x77:
        duel_088328FC(a->player);
        return 0x76;
    case 0x76:
        duel_088C78E0(1 - a->player);
        break;
    }
    return 0;
}

int duel_0889D2D0(DuelAct *a)
{
    int i;
    int id;

    switch (duel_step) {
    case 0x80:
        if (duel_count_free_monster_zones(a->player) == 0) {
            return 0;
        }
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel.players[a->player & 1].n_hand == 0) {
            return 0;
        }
        duel_08832880(1 - a->player, a->id, 0);
        return 0x7F;
    case 0x7F:
        i = duel_w_0891968C;
        id = duel.players[a->player & 1].hand[i].id;
        if (card_is_monster(id) && duel_card_dfac8(a->player, id, 0)) {
            duel_088263A8(a->player, &duel.players[a->player & 1].hand[i], 1, 0);
            return 0;
        }
        duel_088CAB04(a->player, i, 0);
        return 0;
    }
    return 0;
}

int duel_0889D468(DuelAct *a)
{
    duel_088C452C(a->player, 0);
    duel_088C452C(1 - a->player, 0);
    return 0;
}

int duel_0889D4B4(DuelAct *a)
{
    int uid;
    unsigned int loc;
    int z;
    int n;

    if (a->kind == 0x16) {
        uid = duel_special.act[duel_special.act[0].uid == a->src.uid].uid;
        loc = duel_card_locate(uid);
        z = (unsigned char)((loc & 0xFFFF) >> 8);
        if (z < 5) {
            n = duel_field_atk(loc & 0xFF, z);
        } else {
            n = card_get_atk(duel.list[uid].id);
        }
        duel_088C4604(a, 1 - a->player, n);
    }
    return 0;
}

int duel_0889D588(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        duel_08832B18(p, z);
        return 0x7F;
    case 0x7F:
        duel_088C87A8(p, z, a->id, 3, duel_w_0891968C * 200);
        break;
    }
    return 0;
}

int duel_0889D69C(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_08832B18(a->player, a->zone);
        return 0x7F;
    case 0x7F:
        switch (duel_w_0891968C) {
        case 1:
            duel_08832624(a->player, 1, 0, 0);
            return 0;
        case 2:
            duel_08832624(1 - a->player, 1, 0, 1);
            return 0;
        case 3:
            duel_088C5E70(a->player, 1);
            return 0;
        case 4:
            duel_088C5E70(1 - a->player, 1);
            return 0;
        case 5:
            return duel_0884E690(a, duel_088559C0) != 0 ? 0x7E : 0;
        case 6:
            duel_088D147C(a, a->player, a->zone);
            return 0;
        }
        return 0;
    case 0x7E:
        duel_0884ACD8(a->player, 0x62);
        duel_088359E8(a->player, a->id, duel_088559C0);
        return 0x7D;
    case 0x7D:
        if (duel_08835AC0() != 0) {
            duel_088D147C(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            break;
        }
        return 0x7E;
    }
    return 0;
}

int duel_0889D860(DuelAct *a)
{
    int i;

    switch (duel_step) {
    case 0x80:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 0x60;
        } else {
            duel_0884AF9C(0x128, 2);
        }
        return 0x7F;
    case 0x7F:
        duel_w_089148FC = duel_w_08919624;
        duel_08832960(1 - a->player, a->id, duel_w_089148FC);
        return 0x7E;
    case 0x7E:
        duel_08832B18(1 - a->player, -1);
        return 0x7D;
    case 0x7D:
        i = duel_w_0891968C;
        if (duel_w_089148FC & 1 << i) {
            duel_088C5E70(a->player, i);
            return 0;
        }
        duel_088C625C(a->player, i, 0);
        break;
    }
    return 0;
}

int duel_0889D984(DuelAct *a, int b)
{
    duel_088C77D4(a->player, a->id, 1, 0);
    if (a->b4_2 == 0) {
        if (duel.players[a->player & 1].field[10].faceup == 0) {
            return 0;
        }
        if (duel.players[a->player & 1].field[10].card.id != 0x175E) {
            return 0;
        }
        return duel_0887F728(a, b);
    }
    return 0;
}

int duel_0889DA40(DuelAct *a)
{
    unsigned short u;

    if (duel_special.padc[2] != 0) {
        return 0;
    }
    if (duel_special.unk24 != 0) {
        u = duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].faceup;
        duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].faceup = 0;
        duel_088D147C(a, duel_special.player[1], duel_special.zone[1]);
        duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].faceup = u;
    } else {
        duel_088D147C(a, duel_special.player[1], duel_special.zone[1]);
    }
    return 0;
}

int duel_0889DB20(DuelAct *a, int b)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].field[10].faceup == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[10].card.id != 0x175E) {
        return 0;
    }
    return duel_0887DDD0(a, b);
}

int duel_0889DB98(DuelAct *a, int b)
{
    int z;

    if (a->b4_2) {
        return 0;
    }
    if (a->kind != 0x23) {
        for (z = 0; z <= 4; z++) {
            if (card_same_name(field_get_card(a->player, z), 0xFA7) && duel_act_immune(a, a->player, z)) {
                duel_088C87A8(a->player, z, a->id, 1, 0);
            }
        }
        duel_088C77D4(a->player, a->id, 1, 0);
    }
    return duel_0887DDD0(a, b);
}

int duel_0889DC90(DuelAct *a)
{
    DuelCard *c;
    int owner;
    unsigned int p;
    int id;

    switch (duel_step) {
    case 0x80:
        if (duel.players[a->player & 1].n_d == 0) {
            return 0;
        }
        return duel_088BFF78(a->player, a->id, 0) != 0 ? 0x7E : 0x64;
    case 0x7E:
        duel_088C7BA4();
        return 0x7D;
    case 0x7D:
        p = a->player;
        if (duel.players[p & 1].n_d != 0) {
            c = &duel.players[p & 1].zone_d[0];
            owner = c->owner;
            id = c->id;
            duel_088C4508(p, id);
            if (!duel_zone_faceup(p, 0xD, 0)) {
                duel_088C7880(p);
            }
            duel_088CB980(p);
            if (card_is_monster(id) && card_is_group_b(id)) {
                if (duel_count_free_monster_zones(a->player) && duel_phase_ok2(a->player)
                    && duel_card_dfac8(a->player, id, 0)) {
                    duel_088263A8(owner, c, 1, 0);
                } else {
                    duel_088C625C(p, 1, 0);
                }
            } else {
                duel_088C625C(p, 1, 0);
                return 0x7D;
            }
        }
        return 0x7C;
    case 0x7C:
        duel_088C7CF0();
        return 0;
    case 0x64:
        duel_0884ACD8(a->player, 0xD);
        return 0x63;
    case 0x63:
        duel_088328FC(1 - a->player);
        return 0x62;
    case 0x62:
        duel_088C78E0(a->player);
        break;
    }
    return 0;
}

int duel_0889DF20(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    int z;
    DuelStats st;

    if (duel_088A9008(a, 0, &t)) {
        p = t & 0xFF;
        z = (unsigned char)((t & 0xFFFF) >> 8);
        if (duel_0884E440(a, p, z)) {
            duel_field_stats(p, z, &st);
            duel_088C87A8(p, z, a->player | a->zone << 8, 7, duel_turn);
            duel_088C87A8(p, z, a->id, 0x12, st.def > 0xFFFF ? 0xFFFF : st.def);
            duel_088C87A8(p, z, a->id, 0x12, st.atk > 0xFFFF ? 0xFFFF : st.atk);
        }
    }
    return 0;
}

int duel_0889E034(DuelAct *a)
{
    unsigned int p;
    int z;
    int i;

    for (i = 0; i < 2; i++) {
        p = duel_turn_player;
        p ^= i;
        for (z = 0; z <= 4; z++) {
            if (duel_0884E440(a, p, z)) {
                duel_088C97F0(a, p, z, 0, 1);
            }
        }
    }
    return 0;
}

int duel_0889E0D4(unsigned int p, int z, int n)
{
    if (p != duel_w_089147DC->player) {
        if (z == 0xB) {
            return 0x800;
        }
        if (z < 0xB && duel_0884E440(duel_w_089147DC, p, z + n)) {
            return 0x800;
        }
    }
    return 0;
}

int duel_0889E140(DuelAct *a)
{
    unsigned int p;

    switch (duel_step) {
    case 0x80:
        if (duel.players[(1 - a->player) & 1].n_hand == 0 && duel_0884E5E0(a) == 0) {
            return 0;
        }
        duel_088C7810(1 - a->player, 1);
        return 0x7F;
    case 0x7F:
        p = a->player;
        if (duel_player_type[p] == 1) {
            duel_0881956C(p, a->id, duel_0889E0D4);
        } else {
            duel_0884ACD8(p, 0x3A);
            duel_08835978(duel_0889E0D4);
        }
        return 0x7E;
    case 0x7E:
        if (duel_08835AC0() != 0) {
            if (duel_w_08919650 == 0xB) {
                p = 1 - a->player;
                duel_088CAE24(p, &duel.players[p & 1].hand[duel_w_08919654], 1);
            } else {
                duel_088D1C70(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            }
            duel_088C7810(1 - a->player, 0);
            break;
        }
        return 0x7F;
    }
    return 0;
}

int duel_0889E2F8(DuelAct *a, int b)
{
    unsigned int p;

    switch (duel_step) {
    default:
        return duel_08881160(a, b);
    case 1:
        p = a->player;
        duel_088C87A8(p, duel_w_089148FC, p | a->zone << 8, 0x17, 0);
        return 0;
    }
}

int duel_0889E360(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = duel.players[a->player & 1].n_hand;
        return 0x7F;
    case 0x7F:
        if (duel.players[a->player & 1].n_hand != 0) {
            duel_088CAB04(a->player, 0, 0);
            return 0x7F;
        }
        return 0x7E;
    case 0x7E:
        duel_0884C588(a->player, MIN(duel_w_089148FC, 0x20) + 6, a->id, 0);
        duel_w_089148FC = 0;
        duel_088C7BA4();
        return 0x7D;
    case 0x7D:
        if ((unsigned int)duel_w_089148FC < duel_w_08919624) {
            duel_088CAF24(a, a->player, duel_0884CCBC(duel_w_089148FC + 1));
            duel_w_089148FC++;
            return 0x7D;
        }
        duel_088C7CF0();
        break;
    }
    return 0;
}

int duel_0889E4D0(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].field[10].faceup == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[10].card.id != 0x169F) {
        return 0;
    }
    duel_088327F8(1 - p, 1, 3);
    return 0;
}

int duel_0889E550(DuelAct *a, DuelAct *b)
{
    while (a->idx-- != 0) {
        b--;
    }
    return duel_088A2130(a, b);
}

int duel_0889E57C(DuelAct *a, DuelAct *b)
{
    switch (duel_step) {
    case 0x80:
        b->b4_3 = b->b4_2 = b->b4_1 = 1;
        duel_088CBAA4(b->player, b->zone, b->uid);
        if (b->zone < 0xB && duel_088D15B4(a, b->player, b->zone)) {
            return 0x7F;
        }
        break;
    case 0x7F:
        duel_088C5E70(a->player, 1);
        break;
    }
    return 0;
}
