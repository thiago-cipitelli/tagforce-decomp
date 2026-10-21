// @range 0x0889A75C-0x0889C150
// @data 0x0891193C-0x08911974
// @asm 0x0889A9C8-0x0889AAC4
// @asm 0x0889B958-0x0889BAA8
// @asm 0x0889BDC0-0x0889C150
/* Card effect handlers (DuelAct *a), after effects_08898B10.c. */
#include "duel.h"

extern int duel_step;
extern int duel_player_type[2];
extern int duel_w_08919624;
extern unsigned short duel_w_0891968C;
extern int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern DuelAct *duel_w_089147DC;
extern int duel_w_08914900;
extern int duel_w_089148FC;

void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);
void duel_08832950(unsigned int p, unsigned short id);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_088C7880(unsigned int p);
void duel_088CB980(unsigned int p);
int card_same_name(int a, int b);
int duel_zone_f_card_usable(unsigned int p, unsigned int q, int i);
int duel_088DFCC8(unsigned int uid);
int duel_088AA224(DuelAct *a);
DuelCard *duel_0884CCBC(int i);
void duel_088CB400(unsigned int p, DuelCard *c, int a);
int duel_088DFE24(unsigned int p, unsigned int q);
int duel_phase_ok2(unsigned int p);
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, int d);
void duel_088CB490(unsigned int p, int uid);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_grave_count_monsters(unsigned int p);
int duel_field_def(unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
int card_is_attribute9(int id);
int card_get_atk(int id);
int duel_grave_count_id(unsigned int p, int id);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, int c);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
int duel_0887F80C(DuelAct *a, DuelAct *b);
void duel_088C7810(unsigned int p, int a);
void duel_088C6508(unsigned int p, int id, int a, int b);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
int duel_grave_find_uid(unsigned int p, int uid);
int card_attribute_icon_is3(int id);
int duel_088D1684(DuelAct *a, unsigned int p, int z, int flags);
unsigned int duel_field_controller(unsigned int p, int z, int arg);
void duel_088C5E70(unsigned int p, int a);
int duel_phase_ok2(unsigned int p);
int duel_count_free_monster_zones(unsigned int p);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)());
void duel_08835978(int (*fn)());
int duel_hand_find_name(unsigned int p, int id);
void duel_0884ACD8(unsigned int p, void *msg);
int duel_list_find_id(unsigned int p, unsigned int id);
void duel_0884AD40(int msg, int uid);
void duel_088C77EC(unsigned int p, int uid);
void duel_088C452C(unsigned int p, int n);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int duel_field_base_def(unsigned int p, int z);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int duel_088A32A4(DuelAct *a);
void duel_088C625C(unsigned int p, int a, int b);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
void duel_0884C588(unsigned int p, int a, int id, int b);
DuelCard *duel_0884C7CC(void);
void duel_088CB554(unsigned int p, DuelCard *c);
void duel_088CAF24(DuelAct *a, unsigned int p, DuelCard *c);
void duel_088328FC(unsigned int p);
void duel_088C78E0(unsigned int p);
int duel_088CA428(unsigned int p, unsigned int uid, int a);
int duel_0886CE38();
unsigned int duel_card_locate(unsigned int uid);
int duel_act_target2(DuelAct *a, int i);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_08832B18(unsigned int p, int z);
unsigned int duel_0884E538(DuelAct *a, int (*fn)());
unsigned int duel_088D13CC(DuelAct *a, unsigned int mask);
int duel_0884E690(DuelAct *a, int (*fn)());
void duel_088359E8(unsigned int p, int id, int (*fn)());
int duel_08835AC0(void);
void duel_088D147C(DuelAct *a, int b, int c);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_088559C0();
int duel_0885CE00();

int duel_0889A75C(DuelAct *a)
{
    duel_088C77D4(1 - a->player, a->id, 1, a->w[2]);
    return 0;
}

int duel_0889A794(DuelAct *a)
{
    int i;
    int n;

    switch (duel_step) {
    case 0x80:
        duel_08832950(1 - a->player, a->id);
        return 0x7F;
    case 0x7F:
        if (duel_w_0891968C != 0) {
            return 0x78;
        }
        duel_cmd((a->player ? 0x8000 : 0) | 0x5E, 0, 0, 0);
        return 0x7E;
    case 0x7E:
        if (duel_hand_find_name(a->player, 0x167B) >= 0) {
            if (duel_player_type[a->player] == 1) {
                duel_w_08919624 = 1;
            } else {
                duel_0884AD40(299, duel_list_find_id(a->player, 0x167B));
            }
            return 0x7D;
        }
        duel_0884ACD8(a->player, (void *)300);
        return 0;
    case 0x7D:
        if (duel_w_08919624 == 0) {
            return 0;
        }
        i = duel_hand_find_name(a->player, 0x167B);
        duel_088C77EC(a->player, CARD_UID(duel.players[a->player & 1].hand[i]));
        return 0x78;
    case 0x78:
        duel_cmd((a->player ? 0x8000 : 0) | 0x5E, 1, 0, 0);
        n = (duel.players[0].lp + duel.players[1].lp + 1) >> 1;
        duel_088C452C(1 - a->player, n);
        duel_088C452C(a->player, n);
        return 0;
    }
    return 0;
}

#ifdef NON_MATCHING /* registradores s de z e sum trocados */
int duel_0889A9C8(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;
    int sum;
    int i;

    if (duel_088A9008(a, 0, &loc)) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel_0884E440(a, p, z)) {
            sum = 0;
            for (i = 0; i <= 4; i++) {
                if (duel.players[a->player & 1].field[i].faceup != 0) {
                    sum += duel_field_base_def(a->player, i);
                }
            }
            duel_088C87A8(p, z, a->id, 2, sum);
        }
    }
    return 0;
}
#endif

int duel_0889AAC4(DuelAct *a)
{
    switch (a->w[2]) {
    case 1:
        return duel_088A32A4(a);
    case 2:
        duel_088C625C(1 - a->player, 1, 1);
        break;
    }
    return 0;
}

int duel_0889AB24(DuelAct *a)
{
    int k;

    k = a->w[2];
    switch (duel_step) {
    case 0x80:
        if (duel_088BFF78(a->player, a->id, k) == 0) {
            if (k == 1) {
                duel_0884ACD8(a->player, (void *)0x15);
                return 0x78;
            }
            return 0;
        }
        duel_0884C588(a->player, 6, a->id, k);
        return 0x7F;
    case 0x7F:
        if (k == 1) {
            duel_088CB554(a->player, duel_0884C7CC());
            return 0x64;
        }
        duel_088CAF24(a, a->player, duel_0884C7CC());
        return 0;
    case 0x78:
        duel_088328FC(1 - a->player);
        return 0x64;
    case 0x64:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}

int duel_0889AC58(DuelAct *a)
{
    if (duel_088CA428(a->player, a->uid, 1)) {
        duel_088C78E0(a->player);
    }
    return 0;
}

int duel_0889ACAC(DuelAct *a, int b)
{
    unsigned int loc;
    unsigned int p;
    unsigned int i;

    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    if (a->n7 == 1) {
        loc = duel_card_locate(duel_act_target2(a, 0));
        p = loc & 0xFF;
        i = loc >> 16 & 0xFFFF;
        if (((unsigned short)loc >> 8 & 0xFF) == 0xE && duel_grave_card_usable(a->player, p, i)) {
            duel_088263A8(a->player, &duel.players[p & 1].grave[i], 1, a->id << 16);
        }
    }
    return 0;
}

int duel_0889AD8C(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_08832B18(a->player, a->zone);
        return 0x7F;
    case 0x7F:
        switch (duel_w_0891968C) {
        case 1:
        case 2:
            duel_088D13CC(a, duel_0884E538(a, duel_088559C0));
            break;
        case 3:
        case 4:
        case 5:
            if (duel_0884E690(a, duel_088559C0)) {
                return 0x7E;
            }
            break;
        case 6:
            duel_088D13CC(a, duel_0884E538(a, duel_0885CE00));
            break;
        case 0:
            break;
        }
        return 0;
    case 0x7E:
        duel_0884ACD8(a->player, (void *)0x62);
        duel_088359E8(a->player, a->id, duel_088559C0);
        return 0x7D;
    case 0x7D:
        if (duel_08835AC0() != 0) {
            duel_088D147C(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            return 0;
        }
        return 0x7E;
    }
    return 0;
}

int duel_0889AEF0(DuelAct *a)
{
    if (duel_slot_has_effect(a->player, 0xB, a->id) == 0) {
        duel_088C77D4(a->player, a->id, 5, duel.turn);
    }
    return 0;
}

int duel_0889AF4C(DuelAct *a)
{
    if (duel.players[a->player & 1].n_d != 0) {
        duel_088C7880(a->player);
        duel_088CB980(a->player);
        if (card_same_name(duel.players[a->player & 1].zone_d[0].id, a->w[0])) {
            duel_088CB554(a->player, &duel.players[a->player & 1].zone_d[0]);
        } else {
            duel_088C625C(a->player, 1, 0);
        }
    }
    return 0;
}

static int duel_0889B034(unsigned int p, int z, int i)
{
    if (p == duel_w_089147DC->player && z == 0xB) {
        switch (duel.players[p & 1].hand[i].id) {
        case 0x1531:
            if (!(duel_w_089147DC->w[0] & 1)) {
                return 0x800;
            }
            break;
        case 0x161E:
            if (!(duel_w_089147DC->w[0] & 2)) {
                return 0x800;
            }
            break;
        case 0x1656:
            if (!(duel_w_089147DC->w[0] & 4)) {
                return 0x800;
            }
            break;
        case 0x1685:
            if (!(duel_w_089147DC->w[0] & 8)) {
                return 0x800;
            }
            break;
        case 0x1686:
            if (!(duel_w_089147DC->w[0] & 0x10)) {
                return 0x800;
            }
            break;
        }
    }
    return 0;
}

int duel_0889B138(DuelAct *a)
{
    unsigned int i;

    switch (duel_step) {
    case 0x80:
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_count_free_monster_zones(a->player) != 0) {
            for (i = 0; i < duel.players[a->player & 1].n_hand; i++) {
                if (!duel_0889B034((int)a->player, 0xB, i)) {
                    continue;
                }
                duel_0884ACD8(a->player, (void *)0x11E);
                duel_088C7BA4();
                return 0x7F;
            }
        }
        duel_0884ACD8(a->player, (void *)0x122);
        return 0;
    case 0x7F:
        if (duel_count_free_monster_zones(a->player) != 0) {
            for (i = 0; i < duel.players[a->player & 1].n_hand; i++) {
                if (!duel_0889B034((int)a->player, 0xB, i)) {
                    continue;
                }
                if (duel_player_type[a->player] == 1) {
                    duel_0881956C(a->player, a->id, duel_0889B034);
                } else {
                    duel_08835978(duel_0889B034);
                }
                return 0x7E;
            }
        }
        return 0x7C;
    case 0x7E:
        if (duel_08835AC0() != 0) {
            duel_w_08914900 = duel_w_08919654;
            return 0x7D;
        }
        return 0x7C;
    case 0x7D:
        switch (duel.players[a->player & 1].hand[duel_w_08914900].id) {
        case 0x1531:
            a->w[0] |= 1;
            break;
        case 0x161E:
            a->w[0] |= 2;
            break;
        case 0x1656:
            a->w[0] |= 4;
            break;
        case 0x1685:
            a->w[0] |= 8;
            break;
        case 0x1686:
            a->w[0] |= 0x10;
            break;
        }
        duel_088263A8(a->player, &duel.players[a->player & 1].hand[duel_w_08914900], 1, 0);
        return 0x7F;
    case 0x7C:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_0889B49C(DuelAct *a)
{
    duel_088C77D4(1 - a->player, a->id, 5, 0);
    return 0;
}

int duel_0889B4D4(DuelAct *a, DuelAct *b)
{
    unsigned int q;
    unsigned int i;
    unsigned int n;

    switch (duel_step) {
    case 0x80:
        duel_0887F80C(a, b);
        return 0x7F;
    case 0x7F:
        duel_088C7810(1 - a->player, 1);
        return 0x7E;
    case 0x7E:
        q = 1 - a->player;
        n = duel.players[q & 1].n_hand;
        for (i = 0; i < n; i++) {
            if (b->id == duel.players[q & 1].hand[i].id) {
                duel_088CAE24(q, &duel.players[q & 1].hand[i], 1);
                return 0x7E;
            }
        }
        return 0x7D;
    case 0x7D:
        duel_088C6508(1 - a->player, b->id, 1, 1);
        return 0x7C;
    case 0x7C:
        duel_088328FC(a->player);
        return 0x7B;
    case 0x7B:
        duel_088C7810(1 - a->player, 0);
        return 0x7A;
    case 0x7A:
        duel_088C78E0(1 - a->player);
        return 0;
    }
    return 0;
}

int duel_0889B64C(DuelAct *a)
{
    int u;
    int g;
    unsigned char n;

    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 0;
    case 0x7F:
        if ((n = a->n7) != 0) {
            u = duel_act_target2(a, n - 1);
            g = duel_grave_find_uid(a->player, u);
            if (g >= 0) {
                duel_088CA428(a->player, CARD_UID(duel.players[a->player & 1].grave[g]), 1);
                duel_w_089148FC += !card_attribute_icon_is3(duel.list[u].id);
            }
            a->n7--;
            return 0x7F;
        }
        if (duel_w_089148FC != 0) {
            duel_088C78E0(a->player);
        }
        return 0;
    }
    return 0;
}

int duel_0889B790(DuelAct *a)
{
    int pos;
    int c;

    if (a->b3_4 == 3) {
        if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
            return 0;
        }
        if (duel.players[a->player & 1].field[a->zone].faceup == 0) {
            return 0;
        }
        if (duel_088D1684(a, a->player, a->zone, 0) != 0) {
            pos = duel.players[a->player & 1].field[a->zone].pos;
            c = duel_field_controller(a->player, a->zone, 1);
            duel_088C77D4(a->player, a->id, 1, pos * 2 + (a->uid * 8 + c * 4) + 1);
        }
    }
    return 0;
}

int duel_0889B904(DuelAct *a)
{
    unsigned int q;

    q = 1 - a->player;
    duel_088C5E70(q, 7 - duel.players[q & 1].n_hand);
    return 0;
}

#ifdef NON_MATCHING /* a->id << 16 duplicado nos dois caminhos do teste de 0x16BE */
int duel_0889B958(DuelAct *a)
{
    unsigned int q;
    unsigned int loc;
    unsigned int p;
    unsigned int i;
    int k;

    q = a->player ^ a->opp;
    loc = duel_card_locate(a->uid);
    p = loc & 0xFF;
    i = loc >> 16 & 0xFFFF;
    if (duel_phase_ok2(q) == 0) {
        return 0;
    }
    if (((unsigned short)loc >> 8 & 0xFF) == 0xF && duel_zone_f_card_usable(q, p, i)) {
        if (duel_count_free_monster_zones(q) != 0) {
            k = 2;
            if (a->id == 0x16BE) {
                k = 0;
            }
            duel_0882619C(q, &duel.players[p & 1].zone_f[i], 1, k, a->id << 16);
        } else if (duel_088DFCC8(a->uid)) {
            duel_088CB490(p, a->uid);
        }
    }
    return 0;
}
#endif

int duel_0889BAA8(DuelAct *a)
{
    unsigned int i;

    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].faceup == 0) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_088DFE24(a->player, a->player) == 0) {
            return 0;
        }
        if (duel_088BFF78(a->player, a->id, duel_088AA224(a)) == 0) {
            return 0;
        }
        duel_0884ACD8(a->player, (void *)0x56);
        return 0x7F;
    case 0x7F:
        duel_0884C588(a->player, 0x29, a->id, 0);
        return 0x7E;
    case 0x7E:
        for (i = 0; i < duel_w_08919624; i++) {
            duel_088CB400(a->player, duel_0884CCBC(i + 1), 0);
        }
        duel_088C87A8(a->player, a->zone, a->id, 0x12, duel_w_08919624 * 300);
        return 0;
    }
    return 0;
}

int duel_0889BC80(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, &loc)) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel_0884E440(a, p, z)) {
            duel_088D1684(a, p, z, 0);
        }
    }
    return 0;
}

int duel_0889BD04(DuelAct *a)
{
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].faceup != 0) {
        duel_088C87A8(a->player, a->zone, 0x16CB, 4, 0);
    }
    return 0;
}

#ifdef NON_MATCHING /* registradores s de z, i e n trocados */
int duel_0889BDC0(DuelAct *a)
{
    int t;
    unsigned int p;
    unsigned int z;
    int i;
    int v;

    if (duel_088A9008(a, 0, 0)) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z)) {
            t = 0;
            switch (a->id) {
            case 0x1492:
                t = duel_grave_count_monsters(a->player) * 100;
                break;
            case 0x14BE:
                t = a->w[2];
                break;
            case 0x1511:
                t = 1 - a->player;
                break;
            case 0x16CE:
                v = duel_field_def(p, z);
                t = v < 0xFFFF ? v : 0xFFFF;
                break;
            case 0x16E3:
                t = duel.players[(1 - a->player) & 1].n_hand * 200;
                break;
            case 0x1890:
                for (i = 0; i < 5; i++) {
                    if (i == z) {
                        continue;
                    }
                    if (duel.players[a->player & 1].field[i].card.id != 0 && duel.players[a->player & 1].field[i].faceup != 0 &&
                        duel.players[a->player & 1].field[i].pos == 0) {
                        t += duel_field_atk(a->player, i);
                        duel_088C87D8(a, a->player, i, a->id, 2, 1);
                    }
                }
                if (t >= 0xFFFF) {
                    t = 0xFFFF;
                }
                break;
            case 0x18CA:
                if (!card_is_attribute9(a->w[1])) {
                    t = card_get_atk(a->w[1]);
                }
                break;
            case 0x19AB:
                t = (duel_field_atk(p, z) + 1) >> 1;
                break;
            case 0x19B4:
                t = duel_grave_count_id(a->player, 0x19A7) * 400;
                break;
            }
            duel_088C87D8(a, p, z, a->id, 2, t);
        }
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* extração do bit player de src (sll/srl/ext) diferente */
int duel_0889C080(DuelAct *a)
{
    if (a->src.uid == CARD_UID(duel.players[a->src.player & 1].field[a->src.zone].card) &&
        duel.players[a->src.player & 1].field[a->src.zone].faceup != 0 &&
        duel.players[a->src.player & 1].field[a->src.zone].pos == 0) {
        duel_088C97F0(a, a->src.player, a->src.zone, 0, 0);
    }
    return 0;
}
#endif
