// @range 0x088A3430-0x088A49F8
// @asm 0x088A3CB4-0x088A3EDC
// @asm 0x088A3EDC-0x088A4180
/* Card effect handlers (DuelAct *a), after effects_088A1904.c. */
#include "duel.h"

extern int duel_step;
extern unsigned int duel_w_089148FC;
extern int duel_w_08919654;
extern int duel_player_type[2];
extern unsigned int duel_turn_player;

int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int duel_088D1858(DuelAct *a, unsigned int p, int z);
int duel_hand_find_uid(unsigned int p, int uid);
int card_get_level(int id);
int duel_0886CF1C(DuelAct *a, int b);
int duel_0887FCA8(int id);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)(int id));
void duel_0884ACD8(unsigned int p, int msg);
void duel_08835978(int (*fn)(int id));
int duel_08835AC0(void);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
int duel_0887DB24(DuelAct *a);
void duel_088C625C(unsigned int p, int a, int b);
void duel_088C77D4(unsigned int p, unsigned short id, unsigned short a, unsigned short b);
int duel_088998D8(DuelAct *a, int b);
void duel_088327C0(unsigned int p, int id, int a, int b);
extern unsigned short duel_w_0891968C;
int duel_0884E9E8(DuelAct *a, int b);
void duel_088328E8(unsigned int p, unsigned short id, int a);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_slot_ef098(unsigned int p, int z, int id);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_088CA270(unsigned int p, unsigned short a, unsigned short b);
void duel_088CB9E0(unsigned int p, unsigned int mask);
int duel_0884E5E0(DuelAct *a);
void duel_088C4604(DuelAct *a, unsigned int p, int n);
int duel_0887DDD0(DuelAct *a, int b);
int duel_0884E398();
void duel_088C77EC(unsigned int p, int uid);
void duel_08832B18(unsigned int p, int z);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
int duel_count_monsters_fn(unsigned int p, int (*fn)(int id));
int card_is_in_group_1a6f(int id);
unsigned int duel_list_effect_arg(int uid, int id);
int duel_count_faceup_name(unsigned int p, int id);
void duel_wfmt_d(unsigned short *dst, int fmt, int n);
int duel_0884E690(DuelAct *a, int (*fn)());
int duel_088537B0(int id);
void duel_088359E8(unsigned int p, int id, int (*fn)(int id));
void duel_088CB914(void);
unsigned int duel_088D13CC(DuelAct *a, unsigned int mask);
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
unsigned int duel_0884E4E4(DuelAct *a);
int duel_088D1D58(DuelAct *a, int b);
int duel_0886CE38(DuelAct *a, int b);
int duel_grave_find_uid(unsigned int p, int uid);
int duel_0887DBB4(DuelAct *a);
void duel_088CC4B4(unsigned int p, int z, int a);
int duel_field_base_atk(unsigned int p, int z);
int duel_0887FD20(DuelAct *a, int b);
int duel_088D1B78(DuelAct *a, unsigned int p, int z);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
void duel_088CC1D0(unsigned int p, int z, int a);
int duel_slot_value_1909(unsigned int p, int z);
void duel_088D1438(unsigned int p, int z, int a, int b);
int duel_hand_find_name(unsigned int p, int id);
int card_get_type(int id);
int duel_0887F728(DuelAct *a, int b);
int duel_0887DDB0(DuelAct *a, int b);
int duel_08883FBC(DuelAct *a, int b);
int duel_0887F8E4(DuelAct *a, int b);
void duel_088C7BA4(void);
void duel_088C7CF0(void);

int duel_088A3430(DuelAct *a, int b)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    switch (duel_step) {
    case 0x80:
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            if (duel_0884E440(a, p, z) != 0) {
                duel_w_089148FC = CARD_UID(duel.players[p & 1].field[z].card);
                duel_088D1858(a, p, z);
                return 0x7F;
            }
        }
        return 0;
    case 0x7F:
        if (duel_hand_find_uid(a->player, duel_w_089148FC) < 0) {
            return 0;
        }
        a->w[2] = card_get_level(duel.list[duel_w_089148FC].id);
        return duel_0886CF1C(a, b) ? 0x7D : 0;
    case 0x7D:
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, a->id, duel_0887FCA8);
        } else {
            duel_0884ACD8(a->player, 0x11E);
            duel_08835978(duel_0887FCA8);
        }
        return 0x7C;
    case 0x7C:
        if (duel_08835AC0() != 0) {
            duel_088263A8(a->player, &duel.players[a->player & 1].hand[duel_w_08919654], 1, a->id << 16);
            return 0;
        }
        return 0x7D;
    }
    return 0;
}

int duel_088A3674(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        return duel_0887DB24(a) ? 0x7F : 0;
    case 0x7F:
        duel_088C625C(1 - a->player, 2, 1);
        return 0;
    }
    return 0;
}

int duel_088A36E4(DuelAct *a, int b)
{
    if (a->kind != 2) {
        duel_088C77D4(a->player, a->id, 0x11, a->uid);
        return 0;
    }
    return duel_088998D8(a, b);
}

int duel_088A3740(DuelAct *a)
{
    unsigned int q = duel_turn_player ^ duel_w_089148FC;
    int f = (*(unsigned int *)&a->idx >> (q << 3) & 0xFF) != 0;

    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].faceup == 0) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 0;
    case 0x7F:
        if (f) {
            if (duel.players[q & 1].n_hand != 0) {
                duel_088327C0(q, f, 0, 0);
            }
        }
        return 0x7E;
    case 0x7E:
        if ((int)++duel_w_089148FC < 2) {
            return 0x7F;
        }
        break;
    }
    return 0;
}

int duel_088A38B0(DuelAct *a, int b)
{
    switch (duel_step) {
    case 0x80:
        if (duel_0884E9E8(a, b) == 0) {
            return 0;
        }
        duel_088328E8(1 - a->player, a->id, a->player);
        return 0x7D;
    case 0x7D:
        duel_w_089148FC = duel_w_0891968C >> 8 & 0xFF;
        return 0x7C;
    case 0x7C:
        if (duel_act_immune(a, a->player, a->zone) != 0) {
            return 0;
        }
        if (duel_slot_ef098(a->player, a->zone, a->id) == 0) {
            return 0;
        }
        if (duel_act_immune(a, 1 - a->player, duel_w_089148FC) != 0) {
            return 0;
        }
        if (duel_slot_ef098(1 - a->player, duel_w_089148FC, a->id) == 0) {
            return 0;
        }
        duel_088C87A8(a->player, a->zone, a->id, 5, 1 - a->player);
        duel_088C87A8(1 - a->player, duel_w_089148FC, a->id, 5, a->player);
        duel_088CA270(a->player, a->player | a->zone << 8, (unsigned char)(1 - a->player) | (unsigned char)duel_w_089148FC << 8);
        return 0;
    }
    return 0;
}

int duel_088A3A74(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].faceup == 0) {
        return 0;
    }
    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z)) {
            duel_088C87A8(p, z, a->player | a->zone << 8, 6, a->player);
        }
    }
    return 0;
}

int duel_088A3BAC(DuelAct *a)
{
    duel_088CB9E0(a->player, 0x1F);
    return 0;
}

int duel_088A3BD4(DuelAct *a)
{
    unsigned int p = a->player;
    unsigned short k = (unsigned char)((p == duel_turn_player) + 2) << 4 | 1;

    duel_088C77D4(1 - p, a->id, k, 0);
    return 0;
}

int duel_088A3C2C(DuelAct *a, int b)
{
    switch (duel_step) {
    case 0x80:
        duel_088C4604(a, 1 - a->player, duel_0884E5E0(a) * 1000);
        return 0x7F;
    case 0x7F:
        return duel_0887DDD0(a, b);
    }
    return 0;
}

#ifdef NON_MATCHING /* operandos do addu do 4º argumento do case 0x7D invertidos */
int duel_088A3CB4(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_0884E398(a) == 0) {
            return 0;
        }
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, a->id, duel_0887FCA8);
        } else {
            duel_0884ACD8(a->player, 0x49);
            duel_08835978(duel_0887FCA8);
        }
        return 0x7F;
    case 0x7F:
        if (duel_08835AC0() != 0) {
            duel_w_089148FC = duel_w_08919654;
            duel_088C77EC(a->player, CARD_UID(duel.players[a->player & 1].hand[duel_w_089148FC]));
            return 0x7E;
        }
        return 0x80;
    case 0x7E:
        duel_08832B18(a->player, a->zone);
        return 0x7D;
    case 0x7D:
        if (duel_w_0891968C == 1) {
            duel_088CAE24(a->player, &duel.players[a->player & 1].hand[duel_w_089148FC], 0);
        } else {
            duel_088C77D4(0, a->id, 1, (duel_w_0891968C << 12) + CARD_UID(duel.players[a->player & 1].hand[duel_w_089148FC]));
        }
        return 0;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* endereço de a->w[i] calculado antes da máscara e o fim do case 0x7D com outro bloco de retorno */
int duel_088A3EDC(DuelAct *a)
{
    unsigned short buf[256];
    int n = 0;
    int f = 1;
    int id = a->id;
    int r;
    unsigned short *w;

    switch (id) {
    case 0x18E0:
        n = a->w[2];
        break;
    case 0x1A11:
        n = duel_count_monsters_fn(a->player, card_is_in_group_1a6f);
        break;
    case 0x1A28:
        n = duel_list_effect_arg(a->uid, id);
        f = 0;
        break;
    case 0x1A9F:
        n = duel_count_faceup_name(0, id) + duel_count_faceup_name(1, id);
        break;
    }
    switch (duel_step) {
    case 0x80:
        a->w[1] = 0;
        a->w[0] = 0;
        if (n == 0) {
            return 0;
        }
        duel_w_089148FC = n;
        r = duel_0884E5E0(a);
        if (r == 0) {
            return 0;
        }
        if (f && r < (int)duel_w_089148FC) {
            return 0;
        }
        if (f) {
            duel_wfmt_d(buf, 0x71, duel_w_089148FC);
            duel_0884ACD8(a->player, (int)buf);
        } else {
            duel_0884ACD8(a->player, 0x119);
        }
        return 0x7F;
    case 0x7F:
        if (duel_0884E690(a, duel_088537B0) == 0) {
            return 0x7C;
        }
        duel_088359E8(a->player, a->id, duel_088537B0);
        return 0x7D;
    case 0x7D:
        if (duel_08835AC0() != 0) {
            w = &a->w[duel_w_0891964C];
            *w |= 1 << (duel_w_08919650 + duel_w_08919654);
            duel_088CB914();
            return --duel_w_089148FC != 0 ? 0x7F : 0x7C;
        }
        if (f) {
            return 0x7F;
        }
        if (n == duel_w_089148FC) {
            return 0x7F;
        }
        return 0x7C;
    case 0x7C:
        duel_088D13CC(a, a->w[0] | a->w[1] << 16);
        return 0;
    }
    return 0;
}
#endif

int duel_088A4180(DuelAct *a)
{
    unsigned int p;
    unsigned int q;

    switch (duel_step) {
    case 0x80:
        duel_088D1D58(a, duel_0884E4E4(a));
        return 0x7F;
    case 0x7F:
        p = a->player;
        if (duel.players[p & 1].n_hand != 0) {
            duel_088CAE24(p, duel.players[p & 1].hand, 0);
            return 0x7F;
        }
        q = 1 - p;
        if (duel.players[q & 1].n_hand != 0) {
            duel_088CAE24(q, duel.players[q & 1].hand, 1);
            return 0x7F;
        }
        return 0;
    }
    return 0;
}

int duel_088A4268(DuelAct *a, int b)
{
    int i;

    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    if (a->zone == 0xE) {
        i = duel_grave_find_uid(a->player, a->uid);
        if (i >= 0) {
            duel_088263A8(a->player, &duel.players[a->player & 1].grave[i], 1, a->id << 16);
        }
    } else {
        i = duel_hand_find_uid(a->player, a->uid);
        if (i >= 0) {
            duel_088263A8(a->player, &duel.players[a->player & 1].hand[i], 1, a->id << 16);
        }
    }
    return 0;
}

int duel_088A4370(DuelAct *a)
{
    unsigned char z;
    unsigned int p;

    if (duel_0887DBB4(a) != 0) {
        z = a->zone;
        p = a->player;
        if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
            return 0;
        }
        if (duel.players[p & 1].field[z].faceup == 0) {
            return 0;
        }
        duel_088CC4B4(p, z, -duel.players[p & 1].field[z].link);
    }
    return 0;
}

int duel_088A4454(DuelAct *a)
{
    unsigned int m;
    int p;
    int z;
    int sum = 0;

    duel_088C7BA4();
    m = duel_088D13CC(a, duel_0884E4E4(a));
    for (p = 0; p < 2; p++) {
        for (z = 0; z <= 4; z++) {
            if (m & (1 << (z + p * 16))) {
                sum += duel_field_base_atk(p, z);
            }
        }
    }
    duel_088C4604(a, 1 - a->player, sum);
    duel_088C7CF0();
    return 0;
}

int duel_088A4538(DuelAct *a, int b)
{
    int i;
    unsigned int t;
    unsigned int p;
    unsigned int z;
    int r;

    switch (duel_step) {
    case 0x80:
        for (i = 0; i < a->n7; i++) {
            if (duel_088A9008(a, i, 0) != 0) {
                t = duel_act_target(a, i);
                if (duel_0884E440(a, t & 0xFF, (unsigned short)t >> 8 & 0xFF) == 0) {
                    return 0;
                }
            } else {
                return 0;
            }
        }
        for (i = 0; i < a->n7; i++) {
            t = duel_act_target(a, i);
            duel_088C97F0(a, t & 0xFF, (unsigned short)t >> 8 & 0xFF, 0, 0);
        }
        return 0x7F;
    case 0x7F:
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_act_immune(a, p, z) != 0) {
            return 0;
        }
        duel_088D1B78(a, p, z);
        return 0x7E;
    default:
        duel_step += 2;
        r = duel_0887FD20(a, b);
        return r ? r - 2 : 0;
    }
}

int duel_088A469C(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;
    unsigned char y;

    if (a->b4_2 == 0) {
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            if (duel_0884E440(a, p, z)) {
                duel_088C97F0(a, p, z, 0, 1);
            }
        }
    }
    y = a->zone;
    p = a->player;
    if (a->uid != CARD_UID(duel.players[p & 1].field[y].card)) {
        return 0;
    }
    duel_088CC1D0(p, y, 1);
    if (duel_slot_value_1909(a->player, a->zone) + 1 >= 3) {
        duel_088D1438(a->player, a->zone, 0, 0);
    }
    return 0;
}

int duel_088A4800(DuelAct *a, int b)
{
    int i;

    if (a->b3_4 == 2) {
        i = duel_hand_find_name(a->player, a->id);
        if (i < 0) {
            return 0;
        }
        if (duel_0886CE38(a, b) == 0) {
            return 0;
        }
        duel_088263A8(a->player, &duel.players[a->player & 1].hand[i], 1, a->id << 16);
        duel_088C77D4(a->player, a->id, 1, a->idx);
        return 0;
    }
    if (card_get_type(a->idx) == 0x16) {
        return duel_0887F728(a, b);
    }
    if (card_get_type(a->idx) == 0x17) {
        return duel_0887DDB0(a, b);
    }
    return duel_08883FBC(a, b);
}

int duel_088A4938(DuelAct *a, int b)
{
    unsigned char z;
    unsigned int p;

    if (a->kind == 5) {
        z = a->zone;
        p = a->player;
        if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
            return 0;
        }
        duel_088D1438(p, z, 0, 0);
        return 0;
    }
    return duel_0887F8E4(a, b);
}
