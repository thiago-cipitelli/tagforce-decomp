// @range 0x0888C1A8-0x0888D520
// @data 0x08911868-0x08911894
/* Card effect handlers and pick filters, after effects_z.c. */
#include "duel.h"

extern DuelAct *duel_w_089147DC;
extern int duel_step;
extern int duel_w_089148FC;
extern int duel_w_08919654;
extern unsigned short duel_w_0891968C;
extern int duel_phase;
extern unsigned short duel_h_0891968E;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern int duel_order_ctr;
extern DuelCard duel_w_0891525C[];
extern struct {
    int pad0[4];
    DuelCard c[4];
} duel_w_0891524C;

int card_is_monster(int id);
int duel_card_dfac8(unsigned int p, int id, int field);
int duel_08870C54(DuelAct *a);
void duel_0884ACD8(unsigned int p, void *msg);
void duel_08835978(int (*fn)(unsigned int p, int z, unsigned int i));
void duel_088359B0(int (*fn)(unsigned int p, int z, unsigned int i));
int duel_08835AC0(void);
void duel_088CB95C(unsigned int p, int z, int uid);
void duel_wfmt_d(unsigned short *dst, int fmt, int n);
void duel_wfmt_s(unsigned short *dst, int fmt, unsigned short *arg);
unsigned short *card_get_name(int id);
int duel_rand(int n);
void duel_088329E4(unsigned int p, int a, unsigned short *list, int n);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, unsigned short c);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
int duel_088BFF78(unsigned int p, int id, int b);
void duel_0884C588(unsigned int p, int a, int id, int b);
DuelCard *duel_0884C7CC(void);
void duel_088CAF24(DuelAct *a, unsigned int p, DuelCard *c);
void duel_08832AF4(unsigned int p, int a, unsigned int x, unsigned int y);
int duel_0884AF88(int msg);
int duel_phase_ok2();
int duel_count_free_monster_zones(unsigned int p);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, int d);
void duel_088D192C(DuelAct *a, unsigned int p, int z);
unsigned int duel_card_b14(DuelCard *c);
int duel_088A6300(int id);
int duel_088A6D2C(int id, int n, void *out);
int duel_count_active(int id);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_grave_find_uid(unsigned int p, int uid);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
void duel_0884AD18(int msg);
void duel_08832ADC(int a);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
unsigned int duel_088D13CC(DuelAct *a, unsigned int mask);
int duel_count_type(unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);

int duel_0888C1A8(unsigned int p, int z, unsigned int i)
{
    int id;
    int ok;

    if (p == duel_w_089147DC->player) {
        if (z == 0xB) {
        id = duel.players[p & 1].hand[i].id;
        ok = 0;
        switch (duel_w_089147DC->n7) {
        case 0:
            ok = card_is_monster(id) != 0 && duel_card_dfac8(p, id, 1) != 0;
            break;
        case 1:
            ok = !(card_is_monster(id) != 0);
            break;
        case 2:
            ok = !(card_is_monster(id) != 0) && i != duel_w_089147DC->w[1];
            break;
        }
        if (ok) {
            return 0x800;
        }
        }
    }
    return 0;
}

int duel_0888C2E0(DuelAct *a)
{
    unsigned short buf2[256];
    unsigned short buf[256];
    int i;
    int j;
    int k;
    unsigned short t;
    DuelCard *c;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_08870C54(a) == 0) {
            return 0;
        }
        a->n7 = 0;
        duel_0884ACD8(a->player, (void *)0x49);
        duel_08835978(duel_0888C1A8);
        return 0x7F;
    case 0x7F:
        if (duel_08835AC0() != 0) {
            k = duel_w_08919654;
            duel_088CB95C(a->player, 0xB, CARD_UID(duel.players[a->player & 1].hand[k]));
            a->w[a->n7] = k;
            a->n7++;
            return 0x7E;
        }
        return 0x80;
    case 0x7E:
        duel_wfmt_d(buf2, 0x4B, 2);
        duel_0884ACD8(a->player, buf2);
        duel_08835978(duel_0888C1A8);
        return 0x7D;
    case 0x7D:
        if (duel_08835AC0() != 0) {
            k = duel_w_08919654;
            duel_088CB95C(a->player, 0xB, CARD_UID(duel.players[a->player & 1].hand[k]));
            a->w[a->n7] = k;
            a->n7++;
            return 0x7C;
        }
        return 0x7E;
    case 0x7C:
        duel_088359B0(duel_0888C1A8);
        return 0x7B;
    case 0x7B:
        if (duel_08835AC0() != 0) {
            k = duel_w_08919654;
            duel_088CB95C(a->player, 0xB, CARD_UID(duel.players[a->player & 1].hand[k]));
            a->w[a->n7] = k;
            a->n7++;
            return 0x7A;
        }
        return 0x7C;
    case 0x7A:
        for (i = 2; i > 0; i--) {
            j = duel_rand(i + 1);
            t = a->w[j];
            a->w[j] = a->w[i];
            a->w[i] = t;
        }
        duel_088329E4(1 - a->player, a->id, a->w, 3);
        return 0x79;
    case 0x79:
        for (i = 0; i < 3; i++) {
            if (duel_w_0891968C == duel.players[a->player & 1].hand[a->w[i]].id) {
                duel_w_089148FC = i;
                break;
            }
        }
        duel_088CB95C(a->player, 0xB, CARD_UID(duel.players[a->player & 1].hand[a->w[duel_w_089148FC]]));
        return 0x78;
    case 0x78:
        duel_wfmt_s(buf, 0x108, card_get_name(duel.players[a->player & 1].hand[a->w[duel_w_089148FC]].id));
        duel_0884ACD8(a->player, buf);
        return 0x77;
    case 0x77:
        for (i = 0; i < 3; i++) {
            if (i != duel_w_089148FC) {
                duel_088CAE24(a->player, duel_field_card_deb2c(a->player, 0xB, a->w[i]), 0);
                a->w[duel_w_089148FC] -= a->w[i] < a->w[duel_w_089148FC];
            }
        }
        return 0x76;
    case 0x76:
        c = duel_field_card_deb2c(a->player, 0xB, a->w[duel_w_089148FC]);
        if (card_is_monster(c->id) != 0) {
            duel_088263A8(a->player, c, 1, 0);
        } else {
            duel_088CAE24(a->player, c, 0);
        }
        return 0;
    }
    return 0;
}

int duel_0888C830(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel.players[p & 1].field[z].faceup != 0) {
            duel_088C87D8(a, p, z, a->id, 2, 700);
        }
    }
    return 0;
}

int duel_0888C8CC(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) != 0) {
            duel_088C87A8(p, z, a->player | a->zone << 8, 6, 0);
            return 0;
        }
    }
    return 0;
}

int duel_0888C9F0(DuelAct *a)
{
    if (duel_phase != 5) {
        if (a->b4_2) {
            return 0;
        }
        duel_088C77D4(a->player, a->id, 1, 0);
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_088BFF78(a->player, a->id, 0) == 0) {
            return 0;
        }
        duel_0884ACD8(a->player, (void *)0x4D);
        return 0x7F;
    case 0x7F:
        duel_0884C588(a->player, 6, a->id, 0);
        return 0x7E;
    case 0x7E:
        duel_088CAF24(a, a->player, duel_0884C7CC());
        return 0x78;
    }
    return 0;
}

int duel_0888CAFC(DuelAct *a)
{
    unsigned char z;
    unsigned int p;

    if (a->b4_2) {
        return 0;
    }
    z = a->zone;
    p = a->player;
    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (a->kind != 0x12) {
            return 0;
        }
        if (a->idx == 0) {
            return 0;
        }
        if (duel_special.player[0] == p) {
            return 0;
        }
        if (duel_player_type[p] == 1) {
            duel_w_08919624 = duel_rand(2);
        } else {
            duel_0884AF88(0x38);
        }
        return 0x7F;
    case 0x7F:
        duel_08832AF4(p, z, 1, duel_w_08919624);
        return 0x7E;
    case 0x7E:
        if (duel_h_0891968E != 0) {
            duel_088C87D8(a, *(unsigned int *)&duel_special, duel_special.zone[0], p | z << 8, 7, 0);
        }
        return 10;
    }
    return 0;
}

int duel_0888CCA4(DuelAct *a)
{
    int cid = 0;
    int n = 0;
    int k = 2;

    switch (a->id) {
    case 0x13FC:
        cid = 0x1422;
        n = 2;
        break;
    case 0x1810:
        n = a->w[0];
        cid = 0x1813;
        k = 1;
        break;
    case 0x19AF:
        cid = 0x19BA;
        n = 3;
        k = 0;
        break;
    case 0x19FE:
        n = 2;
        cid = 0x19EE;
        k = 1;
        break;
    case 0x1A62:
        cid = 0x189C;
        n = 2;
        break;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_count_free_monster_zones(a->player) < n) {
            return 0;
        }
        duel_w_089148FC = 0;
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        if (duel_w_089148FC < n) {
            DuelCard t = {0};

            t.id = cid;
            t.owner = a->player;
            duel_0882619C(a->player, &t, 1, k, 1);
            duel_w_089148FC++;
            return 0x7F;
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_0888CE58(DuelAct *a)
{
    unsigned int t = duel_act_target(a, 0);
    unsigned int p = t & 0xFF;
    unsigned int z = (unsigned short)t >> 8 & 0xFF;
    int n;
    int i;
    int j;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_088A9008(a, 0, 0) == 0) {
            return 0;
        }
        if (duel_0884E440(a, p, z) == 0) {
            return 0;
        }
        a->w[1] = duel.players[p & 1].field[z].card.id;
        a->w[2] = CARD_UID(duel.players[p & 1].field[z].card);
        duel_088D192C(a, p, z);
        if (duel_card_b14(&duel.players[p & 1].field[z].card) == 0) {
            return 0;
        }
        n = duel_088A6300(a->w[1]);
        if (n != duel_088BFF78(a->player, a->id, a->w[2])) {
            return 0;
        }
        if (duel_088A6D2C(a->w[1], n, duel_w_0891525C) == 0) {
            return 0;
        }
        if (duel_count_active(0x159D) != 0 && duel_find_effect_card(a->player, 0x158B, -1) == 0) {
            return 0;
        }
        return 0x7F;
    case 0x7F:
        n = duel_088A6300(a->w[1]);
        for (i = 0; i < n; i++) {
            j = duel_grave_find_uid(a->player, CARD_UID(duel_w_0891524C.c[i]));
            if (duel_grave_card_usable(a->player, a->player, j) == 0) {
                return 0;
            }
        }
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_count_free_monster_zones(a->player) < n) {
            return 0;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 1;
        } else {
            duel_0884AD18(0x59);
        }
        return 0x7D;
    case 0x7D:
        if (duel_w_08919624 != 0) {
            duel_w_089148FC = 0;
            duel_088C7BA4();
            return 0x7C;
        }
        return 0;
    case 0x7C:
        n = duel_088A6300(a->w[1]);
        duel_088263A8(a->player, &duel_w_0891524C.c[duel_w_089148FC], 1, 0x20);
        return ++duel_w_089148FC < n ? 0x7C : 0x7B;
    case 0x7B:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_0888D1DC(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    switch (duel_step) {
    case 0x80:
        if (duel.mode & 1) {
            return 0;
        }
        if (!(duel.mode >> 1 & 1)) {
            return 0;
        }
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            if (duel_0884E440(a, p, z) != 0 &&
                duel.players[duel_special.player[0] & 1].field[duel_special.zone[0]].card.id != 0) {
                duel_cmd((duel_special.player[0] ? 0x8000 : 0) | 0x19, duel_special.zone[0], z, p);
                return 0x7F;
            }
        }
        return 0;
    case 0x7F:
        duel_08832ADC(1);
        return 0;
    }
    return 0;
}

int duel_0888D320(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    duel_088D13CC(a, 0x4000400);
    return 0;
}

int duel_0888D3D0(DuelAct *a)
{
    duel_088C77D4(0, a->id, 0x11, 0);
    return 0;
}

int duel_0888D3FC(DuelAct *a)
{
    int z;
    int v;

    for (z = 0; z < 5; z++) {
        if (duel_0884E440(a, a->player, z) != 0) {
            v = 0;
            switch (a->id) {
            case 0x1409:
                v = duel_field_atk(a->player, z) / 5;
                v = MIN(v, 0xFFFF);
                break;
            case 0x153D:
                v = a->w[0];
                break;
            case 0x15AE:
                v = duel_order_ctr;
                break;
            case 0x1882:
                v = duel_count_type(a->player, 0xB) * 200;
                break;
            }
            duel_088C87D8(a, a->player, z, a->id, 2, v);
        }
    }
    return 0;
}
