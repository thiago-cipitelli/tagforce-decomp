// @range 0x08892554-0x08893A54
// @data 0x089118C4-0x089118DC
// @asm 0x08892554-0x088927AC
// @asm 0x08893390-0x088934A0
/* Card effect handlers (DuelAct *a), after effects_zf.c. */
#include "duel.h"

extern int duel_step;
extern int duel_w_089148FC;
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern int duel_turn_player;
extern unsigned short duel_w_0891968C;
extern int duel_w_08919624;
extern int duel_player_type[2];

int duel_phase_ok2();
unsigned int duel_act_target2(DuelAct *a, int i);
unsigned int duel_card_locate(unsigned int uid);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
int card_get_level(int id);
int duel_0884E5E0(DuelAct *a);
void duel_0884ACD8(unsigned int p, void *msg);
int duel_0884E690(DuelAct *a, int (*fn)(int id));
int duel_088537B0(int id);
void duel_088359E8(unsigned int p, int id, int (*fn)(int id));
int duel_08835AC0(void);
int duel_field_level(unsigned int p, int z);
void duel_088CB914(unsigned int p, int z);
void duel_088D184C(DuelAct *a, unsigned int mask, int b);
int duel_0886CE38();
void duel_0887DC94(DuelAct *a);
int duel_slot_has_effect(unsigned int p, int z, int id);
void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);
int duel_088807F0(DuelAct *a, DuelAct *b);
int duel_08892344(DuelAct *a, DuelAct *b);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_088D192C(DuelAct *a, unsigned int p, int z);
int duel_088D1858(DuelAct *a, unsigned int p, int z);
int duel_0888B7A8(DuelAct *a);
void duel_088C9D24(DuelAct *a, unsigned int p, int z);
void duel_088CBAA4(unsigned int p, int z, int uid);
void duel_088D147C(DuelAct *a, unsigned int p, int z);
int duel_0887DDD0(DuelAct *a);
int duel_0887DDB0(DuelAct *a, int b);
int duel_0887F728(DuelAct *a, int b);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
int duel_088D14B8(DuelAct *a, unsigned int p, int z);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_088C5E70(unsigned int p, int a);
void duel_088C78E0(unsigned int p);
int duel_grave_find_uid(unsigned int p, int uid);
void duel_088CA428(unsigned int p, unsigned int uid, int a);
void duel_08832880(unsigned int p, unsigned short id, int a);
void duel_088CAB04();
int duel_088BFF78(unsigned int p, int id, int b);
void duel_088329D0(unsigned int p, int a, int b);
void duel_088C6728(unsigned int p, int uid, int a);
void duel_088328FC(unsigned int p);
void duel_088C7954();
unsigned int duel_0884E4E4(DuelAct *a);
DuelCard *duel_0884C7CC(void);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_0884C588(unsigned int p, int a, int id, int b);
void duel_0884AD18(int msg);
int duel_count_free_monster_zones(unsigned int p);
void duel_08832B18(unsigned int p, int z);
void duel_088CA384(unsigned int p, int i, int f);
void duel_088D19B4(DuelAct *a, unsigned int mask);
void duel_088D2760(DuelAct *a, int i);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_08832974(); /* no prototype here: with one, `a` moves out of $a0 in 0889366C */
int card_is_monster(int id);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
int duel_0888A88C();
int duel_0888B9A8();
int duel_0887F8E4();
int duel_08890A64();
int duel_08897160();
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, unsigned int c2);
unsigned int duel_088D13CC(DuelAct *a, unsigned int mask);

#ifdef NON_MATCHING /* teste da zona 0xE salta direto ao epílogo (original: bne para o `return 0` comum); q/z em $s1/$s0 trocados */
int duel_08892554(DuelAct *a)
{
    unsigned int uid;
    unsigned int loc;
    int z;
    unsigned int q;
    int i;

    switch (duel_step) {
    case 0x80:
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (a->n7 == 1) {
            uid = duel_act_target2(a, 0);
            loc = duel_card_locate(uid);
            i = loc >> 16 & 0xFFFF;
            if (((unsigned short)loc >> 8 & 0xFF) == 0xE) {
                if (duel_grave_card_usable(a->player, a->player, i) != 0) {
                    a->w[2] = card_get_level(duel.list[uid].id);
                    a->w[3] = a->w[0];
                    if (duel_0884E5E0(a) != 0) {
                        return 0x7F;
                    }
                }
            }
            return 0;
        }
        return 0;
    case 0x7F:
        a->w[1] = 0;
        a->w[0] = 0;
        duel_0884ACD8(a->player, (void *)0x46);
        return 0x7E;
    case 0x7E:
        if (duel_0884E690(a, duel_088537B0) == 0) {
            return 0x7C;
        }
        duel_088359E8(a->player, a->id, duel_088537B0);
        return 0x7D;
    case 0x7D:
        if (duel_08835AC0() != 0) {
            q = duel_w_0891964C;
            uid = duel_w_08919650 + duel_w_08919654;
            a->w[q] |= 1 << uid;
            a->w[2] -= duel_field_level(q, uid);
            duel_088CB914(q, uid);
            return --duel_w_089148FC != 0 ? 0x7E : 0x7C;
        }
        return 0x7E;
    case 0x7C:
        duel_088D184C(a, a->w[0] | a->w[1] << 16, 0);
        return 0x7B;
    case 0x7B:
        if (duel_0886CE38() != 0) {
            a->w[0] = a->w[3];
            duel_0887DC94(a);
        }
        break;
    }
    return 0;
}
#endif

int duel_088927AC(DuelAct *a, DuelAct *b)
{
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].faceup == 0) {
        return 0;
    }
    if (duel_step == 0x80) {
        if (duel_slot_has_effect(a->player, 0xB, 0x14C4) != 0) {
            return 0;
        }
        duel_088C77D4(a->player, 0x14C4, 1, 0);
    }
    return duel_088807F0(a, b);
}

int duel_088928BC(DuelAct *a, DuelAct *b)
{
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].faceup != 0) {
        return duel_08892344(a, b);
    }
    return 0;
}

int duel_08892970(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;
    unsigned int t2;
    unsigned int p2;
    unsigned int z2;

    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0) {
            duel_088D192C(a, p, z);
        }
    }
    if (duel_088A9008(a, 1, 0) != 0) {
        t2 = duel_act_target(a, 1);
        p2 = t2 & 0xFF;
        z2 = (unsigned short)t2 >> 8 & 0xFF;
        if (duel_0884E440(a, p2, z2) != 0) {
            duel_088D1858(a, p2, z2);
        }
    }
    return 0;
}

int duel_08892A50(DuelAct *a)
{
    if (a->kind == 0xD) {
        return duel_0888B7A8(a);
    }
    return 0;
}

int duel_08892A84(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (a->b4_2) {
        return 0;
    }
    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0) {
            duel_088C9D24(a, p, z);
        }
    }
    return 0;
}

int duel_08892B24(DuelAct *a)
{
    duel_088C77D4(duel_turn_player, a->id, 1, 0);
    return 0;
}

int duel_08892B54(DuelAct *a, DuelAct *b)
{
    if (a->w[2] == 0) {
        return 0;
    }
    b->b4_2 = 1;
    duel_088CBAA4(b->player, b->zone, b->uid);
    duel_088D147C(a, b->player, b->zone);
    return 0;
}

int duel_08892BE4(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_0884E5E0(a) == 0) {
            return 0;
        }
        duel_0884ACD8(a->player, (void *)0x6E);
        duel_088359E8(a->player, a->id, (int (*)(int))duel_0884E440);
        return 0x7F;
    case 0x7F:
        if (duel_08835AC0() != 0) {
            if (duel_088D1858(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654) != 0) {
                return 0x7E;
            }
            return 0;
        }
        return 0x80;
    case 0x7E:
        a->w[1] = 1;
        duel_0887DDD0(a);
        return 0;
    }
    return 0;
}

int duel_08892CD0(DuelAct *a, int b)
{
    switch (a->w[2]) {
    case 1:
        return duel_0887F728(a, b);
    case 2:
        return duel_0887DDB0(a, b);
    }
    return 0;
}

int duel_08892D20(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, &loc) != 0) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0) {
            duel_088C7BA4();
            if (duel_088D14B8(a, p, z) != 0) {
                duel_088C4604(a, p, 500);
            }
            duel_088C7CF0();
        }
    }
    return 0;
}

int duel_08892DC8(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = a->idx;
    case 0x7F:
        if (duel_w_089148FC != 0) {
            duel_088C5E70(a->player, 1);
            duel_w_089148FC--;
            return 0x7F;
        }
        return 0;
    }
    return 0;
}

int duel_08892E44(DuelAct *a)
{
    unsigned char n;
    int i;

    if ((n = a->n7) != 0) {
        i = duel_grave_find_uid(1 - a->player, duel_act_target2(a, n - 1));
        if (i >= 0) {
            duel_088CA428(1 - a->player, CARD_UID(duel.players[1 - a->player & 1].grave[i]), 1);
        }
        a->n7--;
        return 0x80;
    }
    duel_088C78E0(1 - a->player);
    return 0;
}

int duel_08892F1C(DuelAct *a)
{
    unsigned int q = duel_turn_player ^ duel_w_089148FC;
    unsigned int r;

    switch (duel_step) {
    case 0x80:
        if (duel.players[0].n_hand == 0 || duel.players[1].n_hand == 0) {
            return 0;
        }
        duel_w_089148FC = 0;
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        duel_08832880(q, a->id, 1);
        return 0x7E;
    case 0x7E:
        r = 1 - q;
        duel_088CAB04(r, duel_w_0891968C, r != a->player);
        return 0x7D;
    case 0x7D:
        if (++duel_w_089148FC < 2) {
            return 0x7F;
        }
        duel_088C7CF0();
        return 0x7C;
    case 0x7C:
        duel_088C7BA4();
        duel_088C5E70(a->player, 1);
        duel_088C5E70(1 - a->player, 1);
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_08893068(DuelAct *a)
{
    int n = a->w[2];
    int i;

    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = duel_088BFF78(1 - a->player, a->id, 0) < n;
        duel_088329D0(1 - a->player, a->id, n);
        return 0x7F;
    case 0x7F:
        for (i = 0; i < duel_w_0891968C; i++) {
            duel_088C6728(1 - a->player, duel.unk3832[i], 1);
        }
        return 0x7E;
    case 0x7E:
        if (duel_w_089148FC) {
            duel_088328FC(a->player);
        }
        return 0x7D;
    case 0x7D:
        duel_088C78E0(1 - a->player);
        return 0;
    }
    return 0;
}

int duel_088931A8(void)
{
    duel_088C7954();
    return 0;
}

int duel_088931C8(DuelAct *a)
{
    duel_088C7BA4();
    if (duel_088D13CC(a, duel_0884E4E4(a)) != 0) {
        duel_088C4604(a, 1 - a->player, 1000);
    }
    duel_088C7CF0();
    return 0;
}

int duel_08893230(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_count_free_monster_zones(a->player) == 0) {
            return 0;
        }
        duel_08832B18(a->player, a->zone);
        return 0x7F;
    case 0x7F:
        if (duel_088BFF78(a->player, a->id, duel_w_0891968C) == 0) {
            return 0;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 1;
        } else {
            duel_0884AD18(0x2F);
        }
        return 0x7E;
    case 0x7E:
        if (duel_w_08919624 == 0) {
            return 0;
        }
        duel_0884C588(a->player, 6, a->id, duel_w_0891968C);
        return 0x7D;
    case 0x7D:
        duel_088263A8(a->player, duel_0884C7CC(), 1, 0x20);
        return 0;
    }
    return 0;
}

#ifdef NON_MATCHING /* teste da zona 0xE vira bnel direto ao epílogo (original: bne para o `return 0` comum, como 08892554) */
int duel_08893390(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int i;

    if (duel_count_free_monster_zones(a->player) == 0) {
        return 0;
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    if (a->n7 != 1) {
        return 0;
    }
    loc = duel_card_locate(duel_act_target2(a, 0));
    p = loc & 0xFF;
    i = loc >> 16 & 0xFFFF;
    if (((unsigned short)loc >> 8 & 0xFF) == 0xE && duel_grave_card_usable(a->player, p, i) != 0) {
        duel_0882619C(a->player, &duel.players[p & 1].grave[i], 1, 1, a->id << 16 | 0x20);
        return 0x7F;
    }
    return 0;
}
#endif

int duel_088934A0(DuelAct *a)
{
    int i;

    switch (duel_step) {
    case 0x80:
        duel_088D19B4(a, -1);
        if (a->kind == 0x15) {
            duel_088D2760(a, 0);
            duel_088D2760(a, 1);
        }
        return 0x7F;
    case 0x7F:
        for (i = 0; i < 2; i++) {
            duel_cmd((duel_turn_player ^ i ? 0x8000 : 0) | 0x5A, 1, 0, 0);
        }
        return 0x7E;
    case 0x7E:
        if (duel.players[duel_turn_player & 1].n_hand != 0) {
            duel_088CA384(duel_turn_player, 0, 1);
            return 0x7E;
        }
        return 0x7D;
    case 0x7D:
        if (duel.players[1 - duel_turn_player & 1].n_hand != 0) {
            duel_088CA384(1 - duel_turn_player, 0, 1);
            return 0x7D;
        }
        return 0x7C;
    case 0x7C:
        for (i = 0; i < 2; i++) {
            duel_088C78E0(duel_turn_player ^ i);
        }
        return 0x7B;
    case 0x7B:
        for (i = 0; i < 2; i++) {
            duel_088C5E70(duel_turn_player ^ i, 5);
        }
        return 0;
    }
    return 0;
}

int duel_0889366C(DuelAct *a)
{
    unsigned int p;

    switch (duel_step) {
    case 0x80:
        p = a->player;
        if (duel.players[p & 1].n_d == 0) {
            return 0;
        }
        if (duel_player_type[p] == 0) {
            duel_0884C588(p, 5, a->id, 0);
        }
        return 0x7F;
    case 0x7F:
        p = a->player;
        if ((unsigned int)duel.players[p & 1].n_d < 2) {
            return 0;
        }
        if (duel_player_type[p] == 1) {
            duel_w_08919624 = !card_is_monster(duel.players[a->id & 1].zone_d[0].id);
        } else {
            duel_0884ADAC(0x126, 0x1B6, 0x1B7, 0, 0, 0xF);
        }
        return 0x7E;
    case 0x7E:
        duel_w_089148FC = duel_w_08919624;
        duel_08832974(1 - a->player, a->id, duel_w_089148FC);
        return 0x7D;
    case 0x7D:
        if (duel_w_089148FC != 0) {
            p = a->player;
            duel_cmd((p ? 0x8000 : 0) | 0x57, 0xD, CARD_UID(duel.players[(unsigned char)p & 1].zone_d[0]), 1);
        }
        return 0;
    }
    return 0;
}

int duel_08893884(DuelAct *a, int b)
{
    if (a->b3_4 == 1 && a->kind == 5) {
        if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
            return 0;
        }
        duel_088D1858(a, a->player, a->zone);
        return 0;
    }
    switch (a->id) {
    case 0x14FD:
        return duel_0888A88C(a, b);
    case 0x14FF:
        return duel_0888B9A8(a, b);
    case 0x1501:
        return duel_0887F8E4(a, b);
    case 0x1502:
        return duel_0888B9A8(a, b);
    case 0x1503:
        return duel_08890A64(a, b);
    case 0x1504:
        return duel_08897160(a, b);
    case 0x1506:
        return duel_0887F728(a, b);
    case 0x1526:
        return duel_0887DDD0(a);
    case 0x1694:
        return duel_08892A84(a);
    }
    return 0;
}
