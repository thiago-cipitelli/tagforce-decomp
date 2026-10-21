// @range 0x08898B10-0x0889A75C
// @data 0x0891190C-0x0891193C
// @asm 0x088995A0-0x088997E8
// @asm 0x088999B8-0x08899BF0
// @asm 0x08899D34-0x0889A100
/* Card effect handlers (DuelAct *a), after effects_088966D8.c. */
#include "duel.h"

void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_08835A54(unsigned int p, unsigned short id, int (*fn)());
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)());
void duel_08835978(int (*fn)());
void duel_0884AD18(int msg);
int duel_0884E5E0(DuelAct *a);
int duel_088D1B78(DuelAct *a, int b, int c);
void duel_088D147C(DuelAct *a, int b, int c);
int duel_08835AC0(void);
int duel_0884E690(DuelAct *a, int (*fn)());
int duel_08855908();
int duel_0887FCA8(int id);
int duel_0885CC08();
int duel_0884E9E8(DuelAct *a, int b);
int duel_088D14B8(DuelAct *a, unsigned int p, int z);
void duel_088C5424(unsigned int p, int n);
int duel_088D16C4(DuelAct *a, unsigned int p, int z, int f);
int card_is_attribute9(int id);
unsigned int duel_field_controller(unsigned int p, int z, int arg);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int duel_pick_monster_zone(unsigned int p);
void duel_088CB490(unsigned int p, int uid);
void duel_088CB778(unsigned int p, int z, int uid, int a, int b, int c, int d);
void duel_088D1438(unsigned int p, int z, int a, int b);
void duel_088C7E68(unsigned int p, int z, int a, int b);
int duel_08882190(DuelAct *a);
void duel_08832960(unsigned int p, unsigned short id, int a);
int duel_count_free_monster_zones(unsigned int p);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_088D1608(unsigned int p, int z, int flags);
int duel_act_target2(DuelAct *a, int i);
int duel_grave_find_uid(unsigned int p, int uid);
int duel_pick_st_zone(unsigned int p);
int duel_act_check_08853CE8(DuelAct *a, int x, int y);
void duel_088CB728(unsigned int p, int z, int uid, unsigned int q, int qz, int a, int b);
void duel_088C8B78(unsigned int p, unsigned short loc, unsigned short t, unsigned short k);
int duel_0886CE38();
unsigned int duel_card_locate(unsigned int uid);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, int c);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
unsigned int duel_act_target(DuelAct *a, int i);
void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);
void duel_088CC284(unsigned int p, int z, int n);
int duel_0887DB24(DuelAct *a);
void duel_088C625C(unsigned int p, int a, int b);
int duel_d_count_monsters(unsigned int p);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_088C4508(unsigned int p, int id);
int duel_zone_faceup(unsigned int p, int z, int i);
void duel_088C7880(unsigned int p);
void duel_088CB980(unsigned int p);
int card_is_monster(int id);
void duel_088CB554(unsigned int p, DuelCard *c);
void duel_0884ACD8(unsigned int p, void *msg);
void duel_088328FC(unsigned int p);
void duel_088C78E0(unsigned int p);
void duel_0884C588(unsigned int p, int a, int id, int b);
int duel_0884C804(void);
DuelCard *duel_0884C7CC(void);
void duel_088CAF24(DuelAct *a, unsigned int p, DuelCard *c);
int duel_slot_e68a0(unsigned int p, int z);
int duel_first_free_zone(unsigned int p);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
int duel_phase_ok2(unsigned int p);
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, int d);

extern unsigned int duel_turn_player;
extern int duel_step;
extern int duel_player_type[2];
extern int duel_w_089148FC;
extern int duel_w_08919624;
extern int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;

int duel_08898B10(DuelAct *a)
{
    int p;
    int z;

    for (p = 0; p < 2; p++) {
        for (z = 0; z <= 4; z++) {
            if (duel_0884E440(a, p, z)) {
                duel_088C87D8(a, p, z, a->id, 3, 0);
            }
        }
    }
    return 0;
}

int duel_08898BA0(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, &loc)) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel_0884E440(a, p, z)) {
            duel_088C87D8(a, p, z, a->id, 2, 1);
            duel_088C77D4(p, a->id, 1, 0);
        }
    }
    return 0;
}

int duel_08898C40(DuelAct *a)
{
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].faceup == 0) {
        return 0;
    }
    duel_088CC284(a->player, a->zone, 1);
    return 0;
}

int duel_08898CFC(DuelAct *a)
{
    unsigned int p;
    int z;
    int i;

    for (i = 0; i < 2; i++) {
        p = duel_turn_player ^ i;
        for (z = 0; z <= 10; z++) {
            if (duel_0884E440(a, p, z)) {
                duel_088CC284(p, z, 1);
            }
        }
    }
    return 0;
}

int duel_08898D94(DuelAct *a)
{
    switch (a->w[2]) {
    case 1:
        duel_0887DB24(a);
        break;
    case 2:
        duel_088C625C(1 - a->player, 2, 1);
        break;
    }
    return 0;
}

int duel_08898DF0(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, 0)) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z)) {
            duel_088CC284(p, z, 1);
        }
    }
    return 0;
}

int duel_08898E70(DuelAct *a)
{
    unsigned int p;
    int n;
    DuelCard *c;
    int id;

    switch (duel_step) {
    case 0x80:
        n = duel.players[a->player & 1].n_d;
        if (n == 0) {
            return 0;
        }
        return n == duel_d_count_monsters(a->player) ? 0x64 : 0x7E;
    case 0x7E:
        duel_088C7BA4();
        return 0x7D;
    case 0x7D:
        p = a->player;
        if (duel.players[p & 1].n_d != 0) {
            c = &duel.players[p & 1].zone_d[0];
            id = c->id;
            duel_088C4508(p, id);
            if (duel_zone_faceup(p, 0xD, 0) == 0) {
                duel_088C7880(p);
            }
            duel_088CB980(p);
            if (card_is_monster(id) == 0) {
                duel_088CB554(p, c);
                return 0x7C;
            }
            duel_088C625C(p, 1, 0);
            return 0x7D;
        }
        return 0x7C;
    case 0x7C:
        duel_088C7CF0();
        return 0;
    case 0x64:
        duel_0884ACD8(a->player, (void *)0xD);
        return 0x63;
    case 0x63:
        duel_088328FC(1 - a->player);
        return 0x62;
    case 0x62:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}

int duel_08899084(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (a->w[2] == 0) {
        return 0;
    }
    if (duel_088A9008(a, 0, 0)) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z)) {
            duel_088CC284(a->player, a->zone, -1);
            duel_088CC284(p, z, 1);
        }
    }
    return 0;
}

int duel_08899134(DuelAct *a)
{
    unsigned int p;

    p = a->player;
    if (duel.players[(1 - p) & 1].n_d != 0 && duel_player_type[p] == 0) {
        duel_0884C588(p, 5, a->id, 0);
    }
    return 0;
}

int duel_088991B4(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_088BFF78(a->player, a->id, 0) == 0) {
            duel_0884ACD8(a->player, (void *)0x15);
            return 0x78;
        }
        duel_0884ACD8(a->player, (void *)0x37);
        return 0x7E;
    case 0x7E:
        duel_0884C588(a->player, 6, a->id, 0);
        return 0x7D;
    case 0x7D:
        switch (duel_0884C804()) {
        case 0xD:
            duel_088CB554(a->player, duel_0884C7CC());
            break;
        case 0xE:
            duel_088CAF24(a, a->player, duel_0884C7CC());
            break;
        }
        return 0x64;
    case 0x78:
        duel_088328FC(1 - a->player);
        return 0x64;
    case 0x64:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}

int duel_08899314(DuelAct *a)
{
    unsigned int p;
    int z;
    int i;
    int n;

    for (i = 0; i < 2; i++) {
        p = duel_turn_player ^ i;
        for (z = 0; z <= 10; z++) {
            if (duel.players[p & 1].field[z].faceup != 0 && (n = duel_slot_e68a0(p, z)) != 0) {
                duel_088CC284(p, z, -n);
            }
        }
    }
    return 0;
}

int duel_088993CC(DuelAct *a)
{
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel_first_free_zone(a->player) < 0) {
        return 0;
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    duel_0882619C(a->player, &duel.players[a->player & 1].field[a->zone].card, 1, 1, 1);
    return 0;
}

int duel_088994E4(DuelAct *a)
{
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].faceup == 0) {
        return 0;
    }
    duel_088D1608(a->player, a->zone, 0);
    return 0;
}

#ifdef NON_MATCHING /* registradores s: a, t e g trocados */
int duel_088995A0(DuelAct *a)
{
    int g;
    int z;
    int t;
    DuelAct x;

    char *q;
    int n;

    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.players[a->player & 1].field[a->zone].faceup == 0) {
        return 0;
    }
    t = duel_act_target2(a, 0);
    g = duel_grave_find_uid(a->player, t);
    z = duel_pick_st_zone(a->player);
    q = (char *)&x;
    n = 0x18;
    if (q != 0) {
        do {
            *q++ = 0;
        } while (--n);
    }
    if (z < 0) {
        return 0;
    }
    if (g < 0) {
        return 0;
    }
    x.player = a->player;
    x.zone = 0xB;
    x.b3_4 = 0;
    x.id = duel.players[a->player & 1].grave[g].id;
    x.uid = t;
    if (duel_act_check_08853CE8(&x, a->player, a->zone) == 1) {
        duel_088CB728(a->player, 0xE, t, a->player, z, 1, 0);
        duel_088C8B78(a->player, (unsigned short)(a->player | (unsigned char)z << 8), (unsigned short)(a->player | a->zone << 8), 10);
    }
    return 0;
}
#endif

int duel_088997E8(DuelAct *a, int b)
{
    unsigned int loc;
    unsigned int p;
    unsigned int i;

    if (duel_0886CE38(a, b) == 0) {
        return 0;
    }
    loc = duel_card_locate(a->idx);
    p = loc & 0xFF;
    i = loc >> 16 & 0xFFFF;
    if (((unsigned short)loc >> 8 & 0xFF) == 0xE && card_is_monster(duel.list[a->idx].id) &&
        duel_grave_card_usable(a->player, p, i)) {
        duel_088263A8(a->player, &duel.players[p & 1].grave[i], 1, a->id << 16);
    }
    return 0;
}

int duel_088998D8(DuelAct *a)
{
    int g;

    if (duel_count_free_monster_zones(a->player) == 0) {
        return 0;
    }
    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    g = duel_grave_find_uid(a->player, a->uid);
    if (g >= 0 && duel_grave_card_usable(a->player, a->player, g)) {
        duel_088263A8(a->player, &duel.players[a->player & 1].grave[g], 1, a->id << 16 | 0x20);
    }
    return 0;
}

#ifdef NON_MATCHING /* o original mantém a em a0 e duel_step em v1 no switch interno */
int duel_088999B8(DuelAct *a)
{
    unsigned int q;

    switch (a->w[2]) {
    case 1:
        return duel_08882190(a);
    case 2:
        switch (duel_step) {
        case 0x80:
            if (duel.players[(1 - a->player) & 1].n_d == 0) {
                return 0;
            }
            if (duel_player_type[a->player] == 0) {
                duel_0884C588(a->player, 5, a->id, 0);
            }
            return 0x7F;
        case 0x7F:
            if ((unsigned int)duel.players[(1 - a->player) & 1].n_d < 2) {
                return 0;
            }
            if (duel_player_type[a->player] == 1) {
                duel_w_08919624 = 1;
            } else {
                duel_0884ADAC(0x126, 0x1B6, 0x1B7, 0, 0, 0xF);
            }
            return 0x7E;
        case 0x7E:
            duel_w_089148FC = duel_w_08919624;
            duel_08832960(1 - a->player, a->id, duel_w_08919624 + 3);
            return 0x7D;
        case 0x7D:
            if (duel_w_089148FC != 0) {
                q = 1 - a->player;
                duel_cmd((q ? 0x8000 : 0) | 0x57, 0xD, CARD_UID(duel.players[q & 1].zone_d[0]), 1);
            }
            return 0;
        }
        return 0;
    }
    return 0;
}
#endif

int duel_08899BF0(DuelAct *a)
{
    int g;

    g = duel_grave_find_uid(a->player, a->uid);
    if (g >= 0) {
        duel_088CAF24(a, a->player, &duel.players[a->player & 1].grave[g]);
    }
    return 0;
}

int duel_08899C6C(DuelAct *a)
{
    int g;

    switch (duel_step) {
    case 0x80:
        g = duel_grave_find_uid(a->player, duel_act_target2(a, 0));
        if (g >= 0) {
            duel_088CAF24(a, a->player, &duel.players[a->player & 1].grave[g]);
            return 0x7F;
        }
        return 0;
    case 0x7F:
        duel_088C77D4(a->player, a->id, 1, duel_act_target2(a, 0));
        return 0;
    }
    return 0;
}

#ifdef NON_MATCHING /* registradores s trocados nos laços de 0x80/0x7F */
int duel_08899D34(DuelAct *a)
{
    unsigned int q;
    int u;
    unsigned int c1;
    unsigned int c0;

    int v[2];
    char *cp;
    int n;
    int i;
    unsigned int t;
    unsigned int p;
    unsigned int z;
    int f;
    int ctrl;
    unsigned short w;
    int uid;
    unsigned int b2;
    unsigned int loc;
    unsigned int lp;

    if (a->b3_4 == 0) {
        cp = (char *)v;
        n = 8;
        if (cp != 0) {
            do {
                *cp++ = 0;
            } while (--n);
        }
        duel_088C7BA4();
        for (i = 0; i < a->n7; i++) {
            if (duel_088A9008(a, i, 0) != 0) {
                t = duel_act_target(a, i);
                p = t & 0xFF;
                z = (unsigned short)t >> 8 & 0xFF;
                if (duel_0884E440(a, p, z) != 0 && duel_088D16C4(a, p, z, 0x2000) != 0) {
                    if (!card_is_attribute9(duel.players[p & 1].field[z].card.id)) {
                        f = duel.players[p & 1].field[z].faceup;
                        t = duel.players[p & 1].field[z].pos;
                        ctrl = duel_field_controller(p, z, 1);
                        uid = CARD_UID(duel.players[p & 1].field[z].card);
                        v[p] = f + (t * 2 + (uid * 8 + ctrl * 4));
                    }
                }
            }
        }
        for (i = 0; i < 2; i++) {
            duel_088C87A8(a->player, a->zone, a->id, 3, v[i]);
        }
        duel_088C7CF0();
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        a->w[1] = 0xFFFF;
        a->w[0] = 0xFFFF;
        for (i = 0; i < 2; i++) {
            q = a->player ^ i;
            w = *(unsigned int *)&a->idx >> ((1 - q) << 4);
            u = w >> 3;
            c1 = w >> 1 & 1;
            c0 = w & 1;
            loc = duel_card_locate(u);
            lp = loc & 0xFF;
            a->w[q] = duel_pick_monster_zone(q);
            if (((unsigned short)loc >> 8 & 0xFF) == 0xF) {
                if (a->w[q] < 0) {
                    duel_088CB490(lp, u);
                } else {
                    duel_088CB778(lp, 0xF, u, q, a->w[q], c0, c1);
                }
            }
        }
        return 0x7F;
    case 0x7F:
        for (i = 0; i < 2; i++) {
            q = a->player ^ i;
            w = *(unsigned int *)&a->idx >> ((1 - q) << 4);
            b2 = w >> 2 & 1;
            if (card_is_monster(duel.list[w >> 3].id) == 0) {
                duel_088D1438(q, a->w[q], 0, a->id);
            } else if (q != b2) {
                duel_088C7E68(q, a->w[q], 0, !(duel.players[q & 1].field[a->w[q]].flags & 1));
            }
        }
        return 0;
    }
    return 0;
}
#endif

int duel_0889A100(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, 0)) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z)) {
            duel_088C87A8(p, z, a->id, 2, 0);
            duel_088C77D4(1 - a->player, a->id, 1, 0);
        }
    }
    return 0;
}

int duel_0889A1AC(DuelAct *a, int b)
{
    static int ids[3] = { 0x1670, 0x1671, 0x1672 };
    DuelCard c;
    char *q;
    int n;

    switch (duel_step) {
    case 0x80:
        if (duel_0884E9E8(a, b) == 0) {
            return 0;
        }
        duel_088C7BA4();
        duel_w_089148FC = 0;
        return 0x7F;
    case 0x7F:
        if ((unsigned int)duel_w_089148FC < 3) {
            q = (char *)&c;
            n = 4;
            if (q != 0) {
                do {
                    *q++ = 0;
                } while (--n);
            }
            c.id = ids[duel_w_089148FC];
            duel_0882619C(1 - a->player, &c, 1, 1, a->id << 16 | 1);
            duel_w_089148FC++;
            return 0x7F;
        }
        return 0x7E;
    case 0x7E:
        duel_088C7CF0();
        break;
    }
    return 0;
}

int duel_0889A2D8(DuelAct *a)
{
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    duel_088C7E68(a->player, a->zone, 5, a->b4_2);
    return 0;
}

int duel_0889A380(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, &loc)) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel_0884E440(a, p, z)) {
            duel_088C7BA4();
            if (duel_088D14B8(a, p, z)) {
                duel_088C5424(1 - a->player, 1000);
            }
            duel_088C7CF0();
        }
    }
    return 0;
}

int duel_0889A430(DuelAct *a)
{
    unsigned int i;

    switch (duel_step) {
    case 0x80:
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_0884E5E0(a) == 0) {
            return 0;
        }
        for (i = 0; i < duel.players[a->player & 1].n_hand; i++) {
            if (!duel_088BFF78((int)a->player, a->id, i)) {
                continue;
            }
            return 0x7F;
        }
        return 0;
    case 0x7F:
        duel_0884ACD8(a->player, (void *)0x12);
        duel_08835A54(a->player, 1, duel_0885CC08);
        return 0x7E;
    case 0x7E:
        if (duel_08835AC0() != 0) {
            duel_088D1B78(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            return 0x7D;
        }
        return 0x7F;
    case 0x7D:
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, a->id, duel_0887FCA8);
        } else {
            duel_0884ACD8(a->player, (void *)0x11E);
            duel_08835978(duel_0887FCA8);
        }
        return 0x7C;
    case 0x7C:
        if (duel_08835AC0() != 0) {
            duel_w_089148FC = duel_w_08919654;
            return 0x7B;
        }
        return 0x7D;
    case 0x7B:
        duel_088263A8(a->player, &duel.players[a->player & 1].hand[duel_w_089148FC], 1, a->id << 16);
        return 0x7A;
    case 0x7A:
        if (duel_0884E690(a, duel_08855908) == 0) {
            return 0;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 1;
        } else {
            duel_0884AD18(0x68);
        }
        return 0x79;
    case 0x79:
        if (duel_w_08919624 == 0) {
            return 0;
        }
        duel_08835A54(a->player, a->id, duel_08855908);
        return 0x78;
    case 0x78:
        if (duel_08835AC0() != 0) {
            duel_088D147C(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            return 0;
        }
        return 0x7A;
    }
    return 0;
}
