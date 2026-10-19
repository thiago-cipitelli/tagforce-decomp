// @range 0x0888146C-0x08883114
// @asm 0x088823E0-0x088824E4
// @asm 0x0888293C-0x08882B04
// @asm 0x08882B04-0x08882D9C
// @asm 0x08882D9C-0x08883114
/* Card effect handlers (DuelAct *a), after effects_p.c. */
#include "duel.h"

extern int duel_step;
extern int duel_player_type[2];
extern int duel_w_089148FC;
extern int duel_w_08919624;
extern unsigned int duel_turn_player;
extern int duel_phase;
extern unsigned int duel_turn;
extern int duel_w_08919654;
extern unsigned short duel_h_0891968E;
extern DuelAct *duel_w_089147DC;

int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
void duel_088C97F0();
unsigned int duel_field_type(unsigned int p, int z);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_088C9344(DuelAct *a, unsigned int p, int z, int b);
void duel_088C87A8(unsigned int p, int z, unsigned short id, unsigned short a, unsigned short b);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_grave_find_name(unsigned int p, int id);
void duel_088CAF24(DuelAct *a, unsigned int p, DuelCard *c);
void duel_088D13CC(DuelAct *a, unsigned int mask);
DuelCard *duel_0884C7CC(void);
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, int d);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
int duel_088AA224(DuelAct *a);
void duel_0884C588(unsigned int p, int a, int id, int b);
int duel_phase_ok2(unsigned int p);
int duel_count_free_monster_zones(unsigned int p);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
void duel_0884ACD8(unsigned int p, int msg);
void duel_088C78E0(unsigned int p);
void duel_088328FC(unsigned int p);
int duel_d_find_name(unsigned int p, int id);
int duel_088CB554(unsigned int p, DuelCard *c);
void duel_0884AD40(int msg, int uid);
int duel_rand(int n);
void duel_08832AF4(unsigned int p, int z, int a, int b);
void duel_0884AF88(int msg);
void duel_088C5424(unsigned int p, int n);
void duel_088C45B4(unsigned int p, int n);
int duel_08835AC0(void);
void duel_088CA384(unsigned int p, int id, int a);
void duel_08835978(int (*fn)(unsigned int p, int z));
void duel_0884AD18(int msg);
void duel_088C6508(unsigned int p, int id, int a, int b);
void duel_088C5E70(unsigned int p, int a);
void duel_088D1858(DuelAct *a, unsigned int p, int z);
int duel_0887F728(DuelAct *a, int b);
void duel_088C89BC(unsigned int p, int z, int id, int c);
int duel_grave_count_id(unsigned int p, int id);
void duel_088C625C(unsigned int p, int n, int b);
void duel_088C96EC(unsigned int p, int z, int a);
void duel_088C4508(unsigned int p, int id);
void duel_088C7B90(int n);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
void duel_0887DBB4();
void duel_088C7CF0(void);
void duel_088C7BA4(void);
void duel_088CAB04();
void duel_088D1920(DuelAct *a, unsigned int mask);
int duel_slot_effect_arg(unsigned int p, int z, int id);
int duel_hand_find_uid(unsigned int p, int uid);
void duel_088C8AD0(unsigned int p, int z, int id, int v);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_088C5510(unsigned int p, int n, int f);
void duel_088CC178(unsigned int p, int z, unsigned int v);
int duel_slot_ef098(unsigned int p, int z, int id);
void duel_088CA270(unsigned int p, unsigned short a, unsigned short b);
void duel_088C8B70();
void duel_088C77EC(unsigned int p, int uid);
int card_is_monster(int id);
int card_get_atk(int id);
int card_get_atk_q(int id);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);
void duel_088C7478(unsigned int p, int i, DuelCard *c, int a);
void duel_088D147C(DuelAct *a, unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);

int duel_0888146C(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;

    if (a->b4_2) {
        return 0;
    }
    if (duel_088A9008(a, 0, &loc) != 0) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel.players[p & 1].field[z].pos != 0) {
            if (duel.players[p & 1].field[z].faceup != 0) {
                duel_088C97F0(a, p, z, 0, 1);
            } else {
                duel_088C97F0(a, p, z, 1, 1);
            }
        }
    }
    return 0;
}

int duel_08881544(DuelAct *a)
{
    int i;
    int z;

    if (a->b4_2) {
        return 0;
    }
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    for (i = 0; i < 2; i++) {
        for (z = 0; z < 5; z++) {
            if (duel.players[i & 1].field[z].faceup != 0 && duel.players[i & 1].field[z].pos == 0
                && duel.players[i & 1].field[z].card.id != 0 && (unsigned short)duel_field_type(i, z) == 1) {
                duel_088C97F0(a, i, z, 0, 0);
            }
        }
    }
    return 0;
}

int duel_088816AC(DuelAct *a)
{
    int i;
    unsigned int q;
    int z;

    for (i = 0; i < 2; i++) {
        q = duel_turn_player ^ i;
        for (z = 0; z <= 10; z++) {
            if (duel_0884E440(a, q, z) != 0) {
                duel_088C9344(a, q, z, 1);
            }
        }
    }
    return 0;
}

int duel_08881748(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;
    int k;

    if (!(a->uid == CARD_UID(duel.players[a->player & 1].field[a->zone].card))) {
        return 0;
    }
    if (duel_088A9008(a, 0, &loc) != 0) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        k = 6;
        switch (a->id) {
        case 0x1243:
        case 0x16A2:
        case 0x17FF:
        case 0x180F:
        case 0x184B:
        case 0x1A9C:
            k = 7;
            break;
        }
        duel_088C87A8(p, z, a->player | a->zone << 8, (k & 0xF) | 0x10, 0);
    }
    return 0;
}

int duel_0888188C(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel.players[p & 1].field[z].faceup != 0) {
            duel_088C87A8(p, z, 0x1119, 4, 0);
        }
    }
    return 0;
}

int duel_08881924(DuelAct *a)
{
    int i = duel_grave_find_name(a->player, 0x12E5);

    if (i >= 0) {
        unsigned int p = a->player;

        duel_088CAF24(a, p, &duel.players[p & 1].grave[i]);
    }
    return 0;
}

int duel_08881998(DuelAct *a)
{
    unsigned int loc;
    int i;
    unsigned int mask = 0;
    unsigned int p;
    unsigned int z;

    for (i = 0; i < 2; i++) {
        if (duel_088A9008(a, i, &loc) != 0) {
            p = loc & 0xFF;
            z = (unsigned short)loc >> 8 & 0xFF;
            if (duel_0884E440(a, p, z) != 0) {
                mask |= 1 << (z + p * 16);
            }
        }
    }
    duel_088D13CC(a, mask);
    return 0;
}

int duel_08881A54(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_count_free_monster_zones(a->player) == 0) {
            return 0;
        }
        if (duel_088BFF78(a->player, a->id, duel_088AA224(a)) == 0) {
            return 0;
        }
        duel_0884ACD8(a->player, 0x1A);
        return 0x7F;
    case 0x7F:
        duel_0884C588(a->player, 6, a->id, duel_088AA224(a));
        return 0x7E;
    case 0x7E:
        switch (a->id) {
        case 0x114A:
            duel_0882619C(a->player, duel_0884C7CC(), 1, 0, 0);
            break;
        default:
            duel_088263A8(a->player, duel_0884C7CC(), 1, 0);
            break;
        }
        return 0;
    }
    return 0;
}

int duel_08881BA4(DuelAct *a)
{
    int i;

    switch (duel_step) {
    case 0x80:
        if (a->id == 0x1919) {
            duel_w_089148FC = 9;
        } else {
            duel_w_089148FC = 2;
        }
        return 0x7F;
    case 0x7F:
        i = duel_d_find_name(a->player, a->id);
        if (i >= 0 && duel_088CB554(a->player, &duel.players[a->player & 1].zone_d[i]) != 0) {
            if (--duel_w_089148FC > 0) {
                return 0x7E;
            }
            return 100;
        }
        duel_0884ACD8(a->player, 0x15);
        return 0x6F;
    case 0x7E:
        if (duel_088BFF78(a->player, a->id, 0) <= 0) {
            return 100;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 1;
            return 0x7D;
        }
        duel_0884AD40(0x1C, a->uid);
        return 0x7D;
    case 0x7D:
        if (duel_w_08919624 != 0) {
            return 0x7F;
        }
        return 100;
    case 0x6F:
        duel_088328FC(1 - a->player);
        return 100;
    case 100:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}

int duel_08881D84(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = duel_rand(2);
        } else {
            duel_0884AF88(0x38);
        }
        return 0x7F;
    case 0x7F:
        duel_08832AF4(a->player, a->zone, 1, duel_w_08919624);
        return 0x7E;
    case 0x7E:
        duel_088C87A8(a->player, a->zone, a->id, 2, duel_h_0891968E);
        return 10;
    }
    return 0;
}

int duel_08881E74(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    if (a->b3_4 != 2) {
        duel_088C5424(a->player, 3000);
    } else {
        if (a->y.y16_7) {
            duel_088C5424(a->player, 3000);
        }
        duel_088C45B4(a->player, 5000);
    }
    return 0;
}

static int duel_08881F10(unsigned int p, int z)
{
    if (p == duel_w_089147DC->player && z == 0xB) {
        return 0x800;
    }
    return 0;
}

int duel_08881F48(DuelAct *a)
{
    unsigned int p;

    if (a->b4_2) {
        return 0;
    }
    p = a->player;
    if (duel_player_type[p] == 1) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel.players[p & 1].n_hand == 0) {
            return 0;
        }
        duel_0884AD18(0x1E);
        return 0x7F;
    case 0x7F:
        if (duel_w_08919624 == 0) {
            return 0;
        }
        duel_08835978(duel_08881F10);
        return 0x7E;
    case 0x7E:
        if (duel_08835AC0() != 0) {
            duel_088CA384(a->player, duel_w_08919654, 0);
        }
        return 0x80;
    }
    return 0;
}

int duel_0888206C(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel.players[a->player & 1].n_18 == 0) {
            return 0;
        }
        duel_0884ACD8(a->player, 0x1B);
        return 0x7F;
    case 0x7F:
        duel_0884C588(a->player, 6, a->id, 0);
        return 0x7E;
    case 0x7E:
        duel_088C6508(a->player, duel_0884C7CC()->id, 0, 0);
        return 0;
    }
    return 0;
}

int duel_08882160(DuelAct *a)
{
    duel_088C5E70(1 - a->player, 2);
    return 0;
}

int duel_08882190(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, &loc) != 0) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0) {
            duel_088D1858(a, p, z);
        }
    }
    return 0;
}

int duel_08882210(DuelAct *a, int b)
{
    unsigned int p;

    if (a->kind == 2) {
        return duel_0887F728(a, b);
    }
    duel_088C89BC(a->player, a->zone, a->id, 1);
    p = a->player;
    duel_088C87A8(p, a->zone, a->id, 5, 1 - p);
    return 0;
}

int duel_0888229C(DuelAct *a)
{
    int n = 1;

    switch (a->id) {
    case 0x11D8:
        n = 5;
        break;
    case 0x1744:
        n = 2;
        break;
    case 0x19D0:
        n = duel_grave_count_id(a->player, a->id);
        break;
    }
    duel_088C625C(1 - a->player, n, 1);
    return 0;
}

int duel_08882314(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, &loc) != 0) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0) {
            duel_088C96EC(p, z, 1);
            duel_088C4508(p, duel.players[p & 1].field[z].card.id);
            duel_088C7B90(0x1E);
            duel_088C96EC(p, z, 0);
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* registradores do laço */
int duel_088823E0(DuelAct *a)
{
    unsigned int q = 1 - a->player;
    unsigned int mask = 0;
    int z;

    for (z = 5; z < 10; z++) {
        unsigned int id = duel.players[q & 1].field[z].card.id;

        if (duel.players[q & 1].field[z].faceup != 0 && id == 0x1102) {
            mask |= 1 << (z + q * 16);
        }
    }
    duel_088D13CC(a, mask);
    if (mask != 0) {
        unsigned int p = a->player;
        unsigned int k = duel_turn_player != p;

        if (duel_turn_player == p && duel_phase == 4) {
            k = 2;
        }
        duel_088C77D4(p, a->id, (k & 0xF) << 4 | 1, 0);
    }
    return 0;
}
#endif

int duel_088824E4(DuelAct *a)
{
    duel_0887DBB4(a);
    return 0;
}

int duel_08882504(DuelAct *a)
{
    unsigned int p;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        p = a->player;
        if (duel.players[p & 1].n_hand != 0) {
            duel_088CAB04(p, 0, 0);
            return 0x80;
        }
        return 0x7F;
    case 0x7F:
        p = 1 - a->player;
        if (duel.players[p & 1].n_hand != 0) {
            duel_088CAB04(p, 0, 1);
            return 0x7F;
        }
        return 0x7E;
    case 0x7E:
        duel_088C7BA4();
        duel_088C5E70(a->player, 5);
        return 0x7D;
    case 0x7D:
        duel_088C5E70(1 - a->player, 5);
        duel_088C7CF0();
        return 0x7C;
    }
    return 0;
}

int duel_08882660(DuelAct *a)
{
    unsigned int loc;
    int i;
    unsigned int mask = 0;
    unsigned int p;
    unsigned int z;

    for (i = 0; i < a->n_targets; i++) {
        if (duel_088A9008(a, i, &loc) != 0) {
            p = loc & 0xFF;
            z = (unsigned short)loc >> 8 & 0xFF;
            if (duel_0884E440(a, p, z) != 0) {
                mask |= 1 << (z + p * 16);
            }
        }
    }
    duel_088D1920(a, mask);
    return 0;
}

int duel_08882728(DuelAct *a)
{
    int v;
    int i;

    switch (duel_step) {
    case 0x80:
        duel_088C5510(1 - a->player, 3, a->id);
        return 0x7F;
    case 0x7F:
        if (duel_slot_has_effect(1 - a->player, 0xB, a->id) == 0) {
            return 0;
        }
        duel_088C7BA4();
        return 0x7E;
    case 0x7E:
        v = duel_slot_effect_arg(1 - a->player, 0xB, a->id);
        i = duel_hand_find_uid(1 - a->player, v);
        if (v > 0 && i >= 0) {
            duel_088CAB04(1 - a->player, i, 1);
            duel_088C8AD0(1 - a->player, 0xB, a->id, v);
            return 0x7E;
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_0888286C(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        duel_088CC178(p, z, duel_turn + 1 + (duel_turn_player == p));
    }
    return 0;
}

#ifdef NON_MATCHING /* registradores e ordem dos blocos */
int duel_0888293C(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int q;
    unsigned int z;
    unsigned int qz;

    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].faceup == 0) {
        return 0;
    }
    if (duel_088A9008(a, 0, &loc) != 0) {
        p = a->player;
        q = loc & 0xFF;
        z = a->zone;
        qz = (unsigned short)loc >> 8 & 0xFF;
        if (p != q && duel_slot_ef098(p, z, a->id) != 0 && duel_slot_ef098(q, qz, a->id) != 0) {
            duel_088C89BC(p, z, a->id, 1);
            duel_088C89BC(q, qz, a->id, 1);
            duel_088C87A8(p, z, a->id, 5, q);
            duel_088C87A8(q, qz, a->id, 5, p);
            duel_088CA270(p, p | z << 8, q | qz << 8);
        }
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* registradores e ordem dos blocos */
int duel_08882B04(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;
    unsigned int y;

    if (a->b4_2) {
        a->b4_3 = 1;
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 0;
        duel_088C7BA4();
        if (a->w[2] == 2) {
            return 0x7F;
        }
        if (duel_088A9008(a, duel_w_089148FC, &loc) != 0) {
            z = (unsigned short)loc >> 8 & 0xFF;
            if (duel.players[loc & 1].field[z].card.id != 0 && duel.players[loc & 1].field[z].pos == 0
                && a->player != (loc & 0xFF)) {
                duel_088C97F0(a, loc & 0xFF, z, 0, 0);
            }
        }
        duel_w_089148FC++;
        return 0x7F;
    case 0x7F:
        if (a->w[2] != 1 && duel_088A9008(a, duel_w_089148FC, &loc) != 0) {
            y = a->zone;
            p = a->player;
            z = (unsigned short)loc >> 8 & 0xFF;
            if (a->uid == CARD_UID(duel.players[p].field[y].card) && duel.players[loc & 1].field[z].card.id != 0
                && duel.players[loc & 1].field[z].faceup != 0 && p == (loc & 0xFF)) {
                duel_088C8B70(p, p | y << 8);
                return 0x7E;
            }
        }
        a->b4_3 = 1;
        return 0x7E;
    case 0x7E:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* registradores e ordem dos blocos */
int duel_08882D9C(DuelAct *a)
{
    unsigned int q = 1 - a->player;
    int z;
    unsigned int id;
    int ok;

    switch (duel_step) {
    case 0x80:
        for (z = 0; z < 5; z++) {
            id = duel.players[q & 1].field[z].card.id;
            if (id != 0) {
                if (duel.players[q & 1].field[z].faceup == 0) {
                    ok = 0;
                    duel_088C96EC(q, z, 1);
                    duel_088C4508(q, id);
                    duel_088C7B90(0x1E);
                    if (a->id == 0x188C) {
                        ok = card_get_atk_q(id) <= 1500;
                    } else if (a->id == 0x123B) {
                        ok = card_get_atk(id) >= 1500;
                    }
                    if (ok) {
                        duel_088D147C(a, q, z);
                    } else {
                        duel_088C96EC(q, z, 0);
                    }
                } else {
                    ok = 0;
                    if (a->id == 0x188C) {
                        ok = duel_field_atk(q, z) <= 1500;
                    } else if (a->id == 0x123B) {
                        ok = duel_field_atk(q, z) >= 1500;
                    }
                    if (ok) {
                        duel_088D147C(a, q, z);
                    }
                }
            }
        }
        duel_w_089148FC = 0;
        return 0x7F;
    case 0x7F:
        if ((unsigned int)duel_w_089148FC < duel.players[q & 1].n_hand) {
            id = duel.players[q & 1].hand[duel_w_089148FC].id;
            duel_088C4508(q, id);
            duel_088C77EC(q, CARD_UID(duel.players[q & 1].hand[duel_w_089148FC]));
            if (card_is_monster(id) != 0) {
                ok = 0;
                if (a->id == 0x188C) {
                    ok = card_get_atk_q(id) <= 1500;
                } else if (a->id == 0x123B) {
                    ok = card_get_atk(id) >= 1500;
                }
                if (ok) {
                    duel_088C7478(q, duel_w_089148FC, duel_field_card_deb2c(q, 0xB, duel_w_089148FC), 1);
                    return 0x7F;
                }
            }
            duel_w_089148FC++;
            return 0x7F;
        }
        return 0x7E;
    case 0x7E:
        duel_088C77D4(q, a->id, (((duel_turn_player == a->player) + 4) & 0xF) << 4 | 1, 0);
        return 0;
    }
    return 0;
}
#endif
