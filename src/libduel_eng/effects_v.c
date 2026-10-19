// @range 0x08885B70-0x08886984
// @asm 0x08885B70-0x08885E40
// @asm 0x08885F68-0x08886084
// @asm 0x088860BC-0x08886250
// @asm 0x088862DC-0x08886604
// @asm 0x088867D0-0x08886984
/* Card effect handlers (DuelAct *a), after effects_u.c. */
#include "duel.h"

extern int duel_step;
extern int duel_w_089148FC;
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern DuelAct *duel_w_089147DC;
extern int duel_player_type[2];
extern int duel_w_08919624;
extern int duel_w_08919638;
extern int duel_w_08919640;
extern struct {
    unsigned char b0 : 3;
    unsigned char b3 : 1;
    unsigned char b4 : 2;
    unsigned char b6 : 2;
} duel_w_08914A2A;

void duel_088D13CC(DuelAct *a, unsigned int mask);
int duel_08835AC0(void);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
int duel_field_def(unsigned int p, int z);
int duel_0884E5E0(DuelAct *a);
int duel_0884E4E4(DuelAct *a);
void duel_0884ACD8(unsigned int p, int msg);
void duel_088359E8(unsigned int p, int id, int (*fn)(DuelAct *a, unsigned int p, int z));
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
int duel_bit_count(unsigned int m);
int duel_count_active(int id);
void duel_0887DC94(DuelAct *a);
int duel_act_target2(DuelAct *a, int i);
unsigned int duel_card_locate(unsigned int uid);
int duel_088DFE24(unsigned int p, unsigned int q);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);
void duel_088CB400(unsigned int p, DuelCard *c, int a);
void duel_08835978(int (*fn)(unsigned int p, int z));
void duel_088CB95C(unsigned int p, int z, int uid);
void duel_088C4508(unsigned int p, int id);
void duel_088C77EC(unsigned int p, int uid);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, unsigned short c);
void duel_088C7810(unsigned int p, int a);
void duel_08832950(unsigned int p, unsigned short id);
void duel_0882AF5C(int a, int b, int c);
void duel_08819B94(unsigned int p);
void duel_08835944(int a);
int duel_08827D98(unsigned int p, int uid);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
void duel_0884C588(unsigned int p, int a, int id, int b);
void duel_088CBAA4(unsigned int p, int z, int uid);
void duel_088C5E70(unsigned int p, int a);
int duel_0886F2C0();
int duel_08807B38(int id, int a, int b);
void duel_0884AD18(int msg);
void duel_08832624(unsigned int p, int a, int b, int c);

#ifdef NON_MATCHING /* registradores e desvios */
int duel_08885B70(DuelAct *a)
{
    int m;
    int p;
    int z;
    int v;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        a->n7 = 0;
        m = -1;
        for (p = 0; p < 2; p++) {
            for (z = 0; z < 5; z++) {
                if (duel_0884E440(a, p, z) != 0) {
                    switch (a->id) {
                    case 0x1711:
                        v = duel_field_def(p, z);
                        if (m < 0 || m < v) {
                            m = v;
                        }
                        break;
                    case 0x12E3:
                    case 0x195E:
                        v = duel_field_atk(p, z);
                        if (m < 0 || v < m) {
                            m = v;
                        }
                        break;
                    case 0x1254:
                    case 0x1540:
                    case 0x17F2:
                        v = duel_field_atk(p, z);
                        if (m < 0 || m < v) {
                            m = v;
                        }
                        break;
                    }
                }
            }
        }
        if (m >= 0) {
            a->n7 = 1;
            a->w[0] = m;
            return 0x7F;
        }
        return 0;
    case 0x7F:
        if (duel_0884E5E0(a) < 2) {
            duel_w_089148FC = duel_0884E4E4(a);
            return 0x78;
        }
        switch (a->id) {
        case 0x17F2:
        case 0x195E:
            duel_0884ACD8(a->player, 0x6D);
            break;
        case 0x1540:
            duel_w_089148FC = duel_0884E4E4(a);
            return 0x78;
        default:
            duel_0884ACD8(a->player, 0x62);
            break;
        }
        duel_088359E8(a->player, a->id, duel_0884E440);
        return 0x7E;
    case 0x7E:
        if (duel_08835AC0() == 0) {
            return 0x7F;
        }
        duel_w_089148FC = 1 << (duel_w_08919650 + duel_w_08919654 + duel_w_0891964C * 16);
        return 0x78;
    case 0x78:
        duel_088D13CC(a, duel_w_089148FC);
        return 0;
    }
    return 0;
}
#endif

int duel_08885E40(DuelAct *a)
{
    unsigned int loc;
    int i;
    unsigned int mask;
    unsigned int p;
    unsigned int z;

    if (a->n7 == 3) {
        mask = 0;
        for (i = 0; i < a->n7; i++) {
            if (duel_088A9008(a, i, &loc) != 0) {
                p = loc & 0xFF;
                z = (unsigned short)loc >> 8 & 0xFF;
                if (duel_0884E440(a, p, z) != 0) {
                    mask |= 1 << (z + p * 16);
                }
            }
        }
        if (duel_bit_count(mask) == 3) {
            duel_088D13CC(a, mask);
        }
    }
    return 0;
}

int duel_08885F24(DuelAct *a)
{
    if (duel_count_active(0x12FB) > 0) {
        return 0;
    }
    duel_0887DC94(a);
    return 0;
}

#ifdef NON_MATCHING /* registradores e desvios */
int duel_08885F68(DuelAct *a)
{
    int i;
    unsigned int loc;
    DuelCard *c;
    unsigned int q;

    if (a->n7 == 0) {
        return 0;
    }
    for (i = a->n7 - 1; i >= 0; i--) {
        loc = duel_card_locate(duel_act_target2(a, i));
        if ((loc & 0xFFFF) >> 8 == 0xE && duel_088DFE24(a->player, loc & 0xFF) == 0) {
            return 0;
        }
    }
    duel_088C7BA4();
    for (i = a->n7 - 1; i >= 0; i--) {
        loc = duel_card_locate(duel_act_target2(a, i));
        if ((loc & 0xFFFF) >> 8 == 0xE) {
            c = duel_field_card_deb2c(loc & 0xFF, 0xE, loc >> 16);
            q = c->owner;
            duel_088CB400(q, c, a->player != q);
        }
    }
    duel_088C7CF0();
    return 0;
}
#endif

static int duel_08886084(unsigned int p, int z)
{
    if (p != duel_w_089147DC->player && z == 0xB) {
        return 0x800;
    }
    return 0;
}

#ifdef NON_MATCHING /* registradores e desvios */
int duel_088860BC(DuelAct *a)
{
    unsigned int w = a->w[0] & 1;
    unsigned int p;
    unsigned int q;
    DuelCard *c;

    if (a->b4_2) {
        return 0;
    }
    p = a->player;
    if (w == p) {
        return 0;
    }
    if (duel.players[(1 - p) & 1].n_hand == 0) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        duel_0884ACD8(p, 0xF4);
        duel_08835978(duel_08886084);
        return 0x7F;
    case 0x7F:
        if (duel_08835AC0() == 0) {
            return 0x80;
        }
        q = duel_w_0891964C;
        c = &duel.players[q & 1].hand[duel_w_08919654];
        duel_088CB95C(q, 0xB, CARD_UID(*c));
        duel_088C4508(q, c->id);
        duel_088C77EC(q, CARD_UID(*c));
        return 0;
    }
    return 0;
}
#endif

int duel_08886250(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, &loc) != 0) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0) {
            duel_088C87D8(a, p, z, a->id, 2, 0);
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* registradores e desvios */
int duel_088862DC(DuelAct *a)
{
    unsigned int p;
    unsigned int i;
    int n;

    if (a->b4_2) {
        return 0;
    }
    p = a->player;
    if (a->uid != CARD_UID(duel.players[p & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.mode & 1) {
        return 0;
    }
    if (a->w[2] == 0) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        n = 0;
        for (i = 0; i < duel.players[p & 1].n_hand; i++) {
            if (duel_08827D98(p, CARD_UID(duel.players[p & 1].hand[i])) != 0) {
                n++;
            }
            p = a->player;
        }
        if (n == 0) {
            return 0x78;
        }
        duel_0884ACD8(p, 0x27);
        return 0x7F;
    case 0x7F:
        duel_w_08914A2A.b4 = 0;
        if (duel_player_type[p] == 1) {
            duel_08819B94(p);
        } else {
            duel_08835944(6);
        }
        return 0x7E;
    case 0x7E:
        if (duel_w_08919638 == 0) {
            return 0x80;
        }
        if (duel_w_08919640 == 4 || duel_w_08919640 == 6) {
            duel_0882AF5C(duel_w_08919640 == 4, 0, 0);
            return 0x7D;
        }
        return 0x7F;
    case 0x7D:
        if (duel_w_08914A2A.b3) {
            return 0x7F;
        }
        duel_088C77D4(p, a->id, 1, 0);
        return 0;
    case 0x78:
        duel_088C7810(p, 1);
        duel_08832950(1 - a->player, a->id);
        return 0x77;
    case 0x77:
        duel_088C7810(p, 0);
        return 0;
    }
    return 0;
}
#endif

int duel_08886604(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[(1 - p) & 1].n_d != 0 && duel_player_type[p] == 0) {
        duel_0884C588(p, 5, a->id, 0);
    }
    return 0;
}

int duel_08886684(DuelAct *a, DuelAct *b)
{
    unsigned int p;

    b->b4_2 = 1;
    switch (a->id) {
    case 0x12F5:
        duel_088D13CC(a, 0x1F << ((1 - a->player) * 16));
        return 0;
    case 0x12F6:
        duel_088CBAA4(b->player, b->zone, b->uid);
        return 0;
    case 0x12F7:
        p = a->player;
        duel_088D13CC(a, 0x1F << ((1 - p) * 16) | 1 << (a->zone + p * 16));
        return 0;
    case 0x131C:
        duel_088D13CC(a, duel_0884E4E4(a));
        return 0;
    case 0x162A:
        duel_088CBAA4(b->player, b->zone, b->uid);
        duel_088C5E70(a->player, 1);
        return 0;
    }
    return 0;
}

#ifdef NON_MATCHING /* registradores e desvios */
int duel_088867D0(DuelAct *a)
{
    unsigned int p = a->player;
    unsigned int i;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 3;
        return 0x7D;
    case 0x7F:
        if (duel_w_089148FC == 0) {
            return 0;
        }
        if (duel_0886F2C0(a) == 0) {
            return 0;
        }
        if (duel_player_type[p] == 1) {
            duel_w_08919624 = 0;
            for (i = 0; i < duel.players[p].n_hand; i++) {
                if (duel_08807B38(duel.players[p].hand[i].id, 0, 0) != 0) {
                    duel_w_08919624 = 1;
                }
            }
        } else {
            duel_0884AD18(0x28);
        }
        return 0x7E;
    case 0x7E:
        if (duel_w_08919624 == 0) {
            return 0;
        }
        return 0x7D;
    case 0x7D:
        duel_08832624(p, 1, 1, 0);
        duel_w_089148FC--;
        return 0x7F;
    }
    return 0;
}
#endif
