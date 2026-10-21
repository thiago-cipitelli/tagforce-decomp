// @range 0x08894158-0x08895C8C
// @data 0x089118DC-0x0891190C
// @asm 0x08894500-0x088946B4
// @asm 0x08894DE8-0x08894F78
/* Card effect handlers (DuelAct *a), after effects_zg.c. */
#include "duel.h"

extern int duel_step;
extern int duel_w_089148FC;
extern unsigned short duel_w_0891968C;
extern unsigned short duel_h_0891968E;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern int duel_w_08919650;
extern int duel_w_08919654;
extern int duel_turn_player;

int duel_08886B34(DuelAct *a, DuelAct *b);
unsigned int duel_0884E4E4(DuelAct *a);
void duel_088D184C(DuelAct *a, unsigned int mask, int b);
int duel_0884E398(DuelAct *a, int b);
void duel_08832870(unsigned int p, int a);
int duel_0880B17C(unsigned int p, int i);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
void duel_08832974();
void duel_088C77EC(unsigned int p, int uid);
int card_is_monster(int id);
int card_get_type(int id);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
int duel_0884E9E8(DuelAct *a, int b);
void duel_088328E8(unsigned int p, unsigned short id, unsigned short loc);
int duel_088D1858(DuelAct *a, unsigned int p, int z);
int duel_hand_find_uid(unsigned int p, int uid);
void duel_088CAB04();
int duel_rand(int n);
void duel_0884AF88(int msg);
void duel_08832AF4(unsigned int p, int z, int a, int b);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_088CB400(unsigned int p, DuelCard *c, int a);
void duel_088C625C(unsigned int p, int a, int b);
int duel_0884E5E0(DuelAct *a);
void duel_0884ACD8(unsigned int p, void *msg);
void duel_088359E8(unsigned int p, int id, int (*fn)(DuelAct *a, unsigned int p, int z));
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int duel_08835AC0(void);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int duel_088D1D58(DuelAct *a, int b);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);
int duel_088998D8(DuelAct *a, int b);
int duel_0880B30C(unsigned int p);
void duel_088329D0(unsigned int p, int a, int b);
void duel_088C6728(unsigned int p, int uid, int a);
void duel_088328FC(unsigned int p);
void duel_088C78E0(unsigned int p);
void duel_088D147C(DuelAct *a, unsigned int p, int z);
void duel_088CB9E0(unsigned int p, unsigned int mask);
void duel_088C9D24(DuelAct *a, unsigned int p, int z);
void duel_088D1920(DuelAct *a, unsigned int mask);
void duel_08832B18(unsigned int p, int z);
void duel_088327F8(unsigned int p, int a, int b);
int duel_0886CE38();
unsigned int duel_act_target2(DuelAct *a, int i);
unsigned int duel_card_locate(unsigned int uid);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
int duel_088DFE24(unsigned int p, unsigned int q);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
void duel_088C9344(DuelAct *a, unsigned int p, int z, int b);
int duel_slot_effect_arg(unsigned int p, int z, int id);
int duel_088C5E70(unsigned int p, int n);
int duel_count_monsters(unsigned int p);
int duel_0887FD20(DuelAct *a, int b);
unsigned short *duel_08861480(int id, int n);
void duel_wfmt_s(char *dst, char *fmt, unsigned short *arg);
void duel_08832960();
int duel_count_free_monster_zones(unsigned int p);
int duel_can_set(unsigned int p);
int duel_phase_ok2();
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, int d);
void duel_08832950(unsigned int p, unsigned short id);
int duel_hand_count_monsters(unsigned int p);
void duel_08832880(unsigned int p, unsigned short id, int a);
DuelCard *duel_field_card_deb2c(unsigned int p, int zone, int i);
void duel_088CA384(unsigned int p, int i, int f);
void duel_088C7810(unsigned int p, int a);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, int c);
int duel_d_find_name(unsigned int p, int id);
void duel_088CB554(unsigned int p, DuelCard *c);
int duel_grave_find_uid(unsigned int p, int uid);
int duel_08880D54(DuelAct *a, int b);

int duel_08894158(DuelAct *a, DuelAct *b)
{
    if (a->w[2] != 0) {
        return duel_08886B34(a, b);
    }
    return 0;
}

int duel_08894180(DuelAct *a)
{
    duel_088D184C(a, duel_0884E4E4(a), 2);
    return 0;
}

int duel_088941B8(DuelAct *a, int b)
{
    int id;
    int ok;

    switch (duel_step) {
    case 0x80:
        if (duel_0884E398(a, b) == 0) {
            return 0;
        }
        duel_08832870(1 - a->player, 0);
        return 0x7F;
    case 0x7F:
        duel_w_089148FC = duel_w_0891968C;
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = duel_0880B17C(1 - a->player, duel_w_0891968C);
        } else {
            duel_0884ADAC(0x104, 0x1B8, 0x1B9, 0x1BA, 0, 0xF);
        }
        return 0x7E;
    case 0x7E:
        a->w[0] = duel_w_08919624;
        duel_08832974(1 - a->player, a->id, duel_w_08919624);
        return 0x7D;
    case 0x7D:
        duel_088C77EC(1 - a->player, CARD_UID(duel.players[1 - a->player & 1].hand[duel_w_089148FC]));
        ok = 0;
        id = duel.players[1 - a->player & 1].hand[duel_w_089148FC].id;
        switch (a->w[0]) {
        case 0:
            ok = card_is_monster(id);
            break;
        case 1:
            ok = card_get_type(id) == 0x16;
            break;
        case 2:
            ok = card_get_type(id) == 0x17;
            break;
        }
        if (ok) {
            duel_088C4604(a, 1 - a->player, 700);
        }
        return 0;
    }
    return 0;
}

int duel_08894408(DuelAct *a, int b)
{
    switch (duel_step) {
    case 0x80:
        return duel_0884E9E8(a, b) != 0 ? 0x7F : 0;
    case 0x7F:
        duel_088328E8(1 - a->player, a->id, 0);
        return 0x7E;
    case 0x7E:
        duel_088D1858(a, duel_w_0891968C & 0xFF, duel_w_0891968C >> 8 & 0xFF);
        return 0;
    }
    return 0;
}

int duel_088944A0(DuelAct *a)
{
    int i = duel_hand_find_uid(1 - a->player, a->src.uid);

    if (i >= 0) {
        duel_088CAB04(1 - a->player, i, 1);
    }
    return 0;
}

#ifdef NON_MATCHING /* duel_step despachado em $a2 (original: $v1) por causa da alocação do caso 0x7D */
int duel_08894500(DuelAct *a, int b)
{
    int n;

    switch (duel_step) {
    case 0x80:
        if (duel_0884E9E8(a, b) == 0) {
            return 0;
        }
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
        duel_088C7BA4();
        return 0x7D;
    case 0x7D:
        if (duel_h_0891968E != 0) {
            n = duel.players[1 - a->player & 1].n_grave;
            if (n == 0) {
                return 0x7C;
            }
            duel_088CB400(1 - a->player, &duel.players[1 - a->player & 1].grave[n - 1], 1);
            return 0x7D;
        }
        duel_088C625C(a->player, duel.players[1 - a->player & 1].n_grave, 0);
        return 0x7C;
    case 0x7C:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}
#endif

int duel_088946B4(DuelAct *a)
{
    unsigned int m;
    int i;
    unsigned int q;

    if (a->b3_4 == 0) {
        switch (duel_step) {
        case 0x80:
            if (duel.mode & 1) {
                return 0;
            }
            duel_w_089148FC = 0x10;
            if (duel_0884E5E0(a) != 0) {
                duel_0884ACD8(a->player, (void *)0x7C);
                duel_088359E8(a->player, a->id, duel_0884E440);
                return 0x7F;
            }
            return 0x7D;
        case 0x7F:
            if (duel_08835AC0() != 0) {
                duel_w_089148FC = duel_w_08919650 + duel_w_08919654;
                return 0x7E;
            }
            return 0x80;
        case 0x7E:
            if (duel_w_089148FC < 5) {
                duel_088C87A8(a->player, duel_w_089148FC, a->id, 1, 0);
            }
            return 0x7D;
        case 0x7D:
            m = -1;
            if (duel_w_089148FC != 0x10) {
                m &= ~(1 << (duel_w_089148FC + a->player * 16));
            }
            duel_088D1D58(a, m);
            return 0x7C;
        case 0x7C:
            for (i = 0; i < 2; i++) {
                q = duel_turn_player ^ i;
                if (!duel.players[q & 1].n_hand) {
                    continue;
                }
                duel_088CAE24(q, &duel.players[q & 1].hand[0], 0);
                return 0x7C;
            }
            return 0x7B;
        case 0x7B:
            duel_088C77D4(1 - a->player, a->id, 1, 0);
            return 0;
        }
    }
    return 0;
}

int duel_088948B8(DuelAct *a, int b)
{
    if (a->kind == 2) {
        return duel_088998D8(a, b);
    }
    switch (duel_step) {
    case 0x80:
        if (duel.players[1 - a->player & 1].n_d == 0) {
            return 0;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = duel_0880B30C(1 - a->player);
        } else {
            duel_0884ADAC(0x104, 0x1B8, 0x1B9, 0x1BA, 0, 0xF);
        }
        return 0x7F;
    case 0x7F:
        duel_w_089148FC = duel_w_08919624;
        duel_08832974(1 - a->player, a->id, duel_w_089148FC);
        return 0x7E;
    case 0x7E:
        duel_088329D0(1 - a->player, a->id, duel_w_089148FC);
        return 0x7D;
    case 0x7D:
        if (duel_w_0891968C != 0) {
            duel_088C6728(1 - a->player, duel_h_0891968E, 1);
            return 0x7B;
        }
        duel_0884ACD8(a->player, (void *)0xD);
        return 0x7C;
    case 0x7C:
        duel_088328FC(a->player);
        return 0x7B;
    case 0x7B:
        duel_088C78E0(1 - a->player);
        return 0;
    }
    return 0;
}

int duel_08894A84(DuelAct *a)
{
    if (duel_special.padc[2] != 0) {
        return 0;
    }
    duel_088D147C(a, duel_special.player[1], duel_special.zone[1]);
    return 0;
}

int duel_08894AC4(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_088C9D24(a, a->player, a->zone);
        return 0x7F;
    case 0x7F:
        duel_088CB9E0(a->player, 0x1F);
        return 0;
    }
    return 0;
}

int duel_08894B28(DuelAct *a)
{
    duel_088C9D24(a, a->player, a->zone);
    return 0;
}

int duel_08894B54(DuelAct *a)
{
    duel_088D1920(a, 0x1F << (1 - a->player) * 16);
    return 0;
}

int duel_08894B8C(DuelAct *a)
{
    unsigned int q = duel_turn_player ^ duel_w_089148FC;
    int l;
    int n;

    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 0;
        return 0x7F;
    case 0x7F:
        duel_08832B18(q, a->player == q ? a->zone : -1);
        return 0x7E;
    case 0x7E:
        a->w[q] = duel_w_0891968C;
        if (++duel_w_089148FC < 2) {
            return 0x7F;
        }
        if (a->w[0] == a->w[1]) {
            return 0x80;
        }
        l = a->w[1] < a->w[0];
        n = a->w[1 - l];
        duel_088C4604(a, l, n == 6 ? 6000 : n * 500);
        return 0;
    }
    return 0;
}

int duel_08894CA4(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_088329D0(1 - a->player, a->id, 0);
        return 0x7F;
    case 0x7F:
        if (duel_w_0891968C != 0) {
            duel_088C6728(1 - a->player, duel_h_0891968E, 1);
            return 0x7D;
        }
        return 0x7E;
    case 0x7E:
        duel_088328FC(a->player);
        return 0x7D;
    case 0x7D:
        duel_088C78E0(1 - a->player);
        return 0;
    }
    return 0;
}

int duel_08894D84(DuelAct *a)
{
    switch (a->w[2]) {
    case 1:
        duel_088327F8(1 - a->player, 1, 1);
        break;
    case 2:
        duel_088C625C(1 - a->player, 2, 1);
        break;
    }
    return 0;
}

#ifdef NON_MATCHING /* testes da zona 0xE viram bnel direto ao epílogo (original: bne para o bloco de retorno, como 08892554) */
int duel_08894DE8(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int i;
    unsigned int o;

    switch (duel_step) {
    case 0x80:
        if (duel_0886CE38() != 0) {
            loc = duel_card_locate(duel_act_target2(a, 0));
            p = loc & 0xFF;
            i = loc >> 16 & 0xFFFF;
            if (((unsigned short)loc >> 8 & 0xFF) == 0xE && duel_grave_card_usable(a->player, p, i) != 0) {
                duel_088263A8(a->player, &duel.players[p & 1].grave[i], 1, 0x20);
            }
            return 0x7F;
        }
        return 0x7F;
    case 0x7F:
        if (duel_088DFE24(a->player, 1 - a->player) != 0) {
            loc = duel_card_locate(duel_act_target2(a, 1));
            if (((unsigned short)loc >> 8 & 0xFF) == 0xE) {
                o = duel.players[(loc & 0xFF) & 1].grave[loc >> 16 & 0xFFFF].owner;
                duel_088CB400(o, &duel.players[(loc & 0xFF) & 1].grave[loc >> 16 & 0xFFFF], a->player != o);
            }
            return 0x7E;
        }
        return 0x7E;
    }
    return 0;
}
#endif

int duel_08894F78(DuelAct *a)
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
        if (duel.players[p & 1].field[z].faceup == 0) {
            if (duel.players[p & 1].field[z].pos != 0) {
                duel_088C97F0(a, p, z, 1, 1);
            } else {
                duel_088C9344(a, p, z, 1);
            }
        }
    }
    return 0;
}

int duel_08895054(DuelAct *a)
{
    unsigned int p;
    unsigned char z;
    int n;

    if (a->b4_2) {
        return 0;
    }
    z = a->zone;
    p = a->player;
    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel_turn_player != p) {
        n = 4 - duel.players[p & 1].n_hand;
        if (n != 0 && duel_088C5E70(p, n) != 0) {
            duel_088C87A8(a->player, a->zone, a->id, 0x11, n);
        }
    } else {
        n = duel_slot_effect_arg(p, z, a->id);
        if (n > 0) {
            duel_088327F8(a->player, 0, n);
        }
    }
    return 0;
}

int duel_0889519C(DuelAct *a, int b)
{
    if (duel_step == 0x80 && duel_count_monsters(a->player) != 0) {
        return 0;
    }
    return duel_0887FD20(a, b);
}

int duel_08895200(DuelAct *a)
{
    duel_088C77D4(0, a->id, 0x11, 0);
    return 0;
}

int duel_0889522C(DuelAct *a)
{
    char buf[512];
    int id;
    int ok;

    switch (duel_step) {
    case 0x80:
        if (duel.players[a->player & 1].n_hand == 0) {
            return 0;
        }
        duel_08832870(a->player, 0);
        return 0x7F;
    case 0x7F:
        duel_w_089148FC = duel_w_0891968C;
        duel_08832960(1 - a->player, a->id, duel_w_089148FC);
        return 0x7E;
    case 0x7E:
        duel_wfmt_s(buf, (char *)0x108, duel_08861480(a->id, duel_w_0891968C));
        duel_0884ACD8(a->player, buf);
        return 0x7D;
    case 0x7D:
        duel_088C77EC(a->player, CARD_UID(duel.players[a->player & 1].hand[duel_w_089148FC]));
        ok = 0;
        id = duel.players[a->player & 1].hand[duel_w_089148FC].id;
        switch (duel_w_0891968C) {
        case 0:
            ok = card_is_monster(id);
            break;
        case 1:
            ok = card_get_type(id) == 0x16;
            break;
        case 2:
            ok = card_get_type(id) == 0x17;
            break;
        }
        if (!ok) {
            duel_088D1858(a, duel_special.player[0], duel_special.zone[0]);
        }
        return 0;
    }
    return 0;
}

int duel_08895448(DuelAct *a)
{
    DuelCard c;
    char *q;
    int n;
    int mode;

    if (duel_count_free_monster_zones(a->player) == 0) {
        return 0;
    }
    if (a->id == 0x13FF) {
        if (duel_can_set(a->player) == 0) {
            return 0;
        }
    } else if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    q = (char *)&c;
    n = 4;
    if (q != 0) {
        do {
            *q++ = 0;
        } while (--n);
    }
    mode = 2;
    switch (a->id) {
    case 0x12A0:
        c.id = 0x13FB;
        mode = 0;
        break;
    case 0x13FF:
        c.id = 0x14FA;
        mode = 0;
        break;
    case 0x1543:
        c.id = 0x154E;
        break;
    case 0x1595:
        c.id = 0x15BD;
        break;
    case 0x15BC:
        c.id = 0x15BE;
        mode = 0;
        break;
    case 0x15D5:
        c.id = 0x1603;
        break;
    case 0x1600:
        c.id = 0x1639;
        break;
    case 0x19A5:
        c.id = 0x195A;
        break;
    case 0x1A37:
        c.id = 0x189B;
        break;
    }
    c.owner = a->player;
    duel_0882619C(a->player, &c, 1, mode, a->id << 16 | 1);
    return 0;
}

int duel_08895678(DuelAct *a)
{
    char buf[512];

    switch (duel_step) {
    case 0x80:
        if (duel.players[1 - a->player & 1].n_hand != 0 || duel_count_monsters(1 - a->player) != 0) {
            duel_08832950(1 - a->player, a->id);
            return 0x7F;
        }
        return 0;
    case 0x7F:
        duel_wfmt_s(buf, (char *)0x108, duel_08861480(a->id, duel_w_0891968C));
        duel_0884ACD8(a->player, buf);
        return 0x7E;
    case 0x7E:
        switch (duel_w_0891968C) {
        case 0:
            duel_088327F8(1 - a->player, 1, 1);
            return 0;
        case 1:
            duel_088328E8(1 - a->player, a->id, 0);
            break;
        }
        return 0x7D;
    case 0x7D:
        duel_088D147C(a, duel_w_0891968C & 0xFF, duel_w_0891968C >> 8 & 0xFF);
        return 0;
    }
    return 0;
}

int duel_088957DC(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_08832B18(a->player, a->zone);
        return 0x7F;
    case 0x7F:
        if (duel_w_0891968C == 6) {
            return 0x80;
        }
        duel_088D147C(a, 1 - a->player, duel_w_0891968C - 1);
        return 0;
    }
    return 0;
}

int duel_08895860(DuelAct *a)
{
    int w;
    unsigned int o;

    switch (duel_step) {
    case 0x80:
        if (duel.players[1 - a->player & 1].n_hand == 0) {
            return 0;
        }
        if (duel_hand_count_monsters(1 - a->player) == 0) {
            return 100;
        }
        duel_08832880(a->player, a->id, 1);
        return 0x7F;
    case 0x7F:
        w = duel_w_0891968C;
        o = duel_field_card_deb2c(1 - a->player, 0xB, w)->owner;
        duel_088CA384(1 - a->player, w, 1);
        duel_088C78E0(o);
        return 0;
    case 100:
        duel_088C7810(1 - a->player, 1);
        return 99;
    case 99:
        duel_0884ACD8(a->player, (void *)0x11);
        return 0x62;
    case 0x62:
        duel_088C7810(1 - a->player, 0);
        return 0;
    }
    return 0;
}

int duel_088959D8(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    if (duel_088C5E70(a->player, 2) != 0) {
        duel_088C77D4(a->player, a->id, (unsigned short)((unsigned char)((duel_turn_player == a->player) + 3) << 4 | 1), 0);
    }
    return 0;
}

int duel_08895A64(DuelAct *a, int b)
{
    if (duel.mode & 1) {
        return 0;
    }
    if (!(duel.mode >> 1 & 1)) {
        return 0;
    }
    if (duel_0884E398(a, b) != 2) {
        return 0;
    }
    if (duel.players[duel_special.player[0] & 1].field[duel_special.zone[0]].card.id != 0) {
        duel_088C87D8(a, duel_special.player[0], duel_special.zone[0], a->player | a->zone << 8, 8, 0);
    }
    return 0;
}

int duel_08895B44(DuelAct *a)
{
    int i;
    int n = a->w[0];

    switch (duel_step) {
    case 0x80:
        i = duel_d_find_name(1 - a->player, n);
        if (i < 0) {
            duel_0884ACD8(a->player, (void *)0xD);
            return 0x78;
        }
        duel_088CB554(1 - a->player, &duel.players[1 - a->player & 1].zone_d[i]);
        return 100;
    case 0x78:
        duel_088328FC(a->player);
        return 100;
    case 100:
        duel_088C78E0(1 - a->player);
        return 0;
    }
    return 0;
}

int duel_08895C30(DuelAct *a, int b)
{
    if (duel_grave_find_uid(a->player, a->uid) < 0) {
        return 0;
    }
    return duel_08880D54(a, b);
}
