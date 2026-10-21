// @range 0x0888C1A8-0x0888E5D4
// @data 0x08911868-0x08911894
// @asm 0x0888D74C-0x0888DA10
// @asm 0x0888E1A4-0x0888E3F4
/* Card effect handlers and pick filters, after effects_z.c. */
#include "duel.h"

extern DuelAct *duel_w_089147DC;
extern int duel_step;
extern int duel_w_089148FC;
extern int duel_w_08914900;
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
int duel_088D192C(DuelAct *a, unsigned int p, int z);
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
int duel_08887618(DuelAct *a, int b);
int duel_08890D6C(DuelAct *a, int b);
int duel_088AA224(DuelAct *a);
void duel_088328FC(unsigned int p);
void duel_088C78E0(unsigned int p);
int duel_088D16C4(DuelAct *a, unsigned int p, int z, int f);
int card_is_attribute9(int id);
int duel_field_controller(unsigned int p, int z, int a);
unsigned int duel_card_locate(int uid);
void duel_088CB778(unsigned int p, int z, int uid, int a, int b, int c, int d);
void duel_088D1438(unsigned int p, int z, int a, int b);
void duel_088C7E68(unsigned int p, int z, int a, int b);
int duel_0886CB44(unsigned int p, int n, int uid, int monster, int all);
void duel_08832950(unsigned int p, int a);
void duel_08832670(unsigned int p, int a, int b, int c);
int card_get_type(int id);
void duel_088CBAA4(unsigned int p, int z, int uid);
void duel_088D15B4(DuelAct *a, unsigned int p, int z);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
void duel_088C96EC(unsigned int p, int z, int a);
int card_attribute_to_group(int id);
void duel_088C4508(unsigned int p, int a);
void duel_088C7B90(int a);
int duel_0887DB24(DuelAct *a);
void duel_088CA384(unsigned int p, int i, int f);
void duel_088C5E70(unsigned int p, int a);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
int duel_0884E9E8(DuelAct *a, int b);
void duel_088328E8(unsigned int p, unsigned short id, int a);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_slot_ef098(unsigned int p, int z, int id);
void duel_088CA270(unsigned int p, unsigned short a, unsigned short b);
int duel_088D14B8(DuelAct *a, unsigned int p, int z);
void duel_088CA184(unsigned short loc, unsigned short to);
int duel_pick_monster_zone(unsigned int p);

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

int duel_0888D520(DuelAct *a, int b)
{
    if (a->b4_2) {
        return 0;
    }
    switch (a->w[1]) {
    case 1:
        return duel_08887618(a, b);
    case 2:
        if (duel.mode & 1) {
            return 0;
        }
        if (duel.mode >> 1 & 1) {
            return duel_08890D6C(a, b);
        }
        return 0;
    }
    return 0;
}

int duel_0888D5A8(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_count_free_monster_zones(a->player) == 0) {
            return 0;
        }
        switch (a->id) {
        case 0x140B:
        case 0x1A98:
            if (duel_088AA224(a) == 0) {
                return 0;
            }
        }
        if (duel_088BFF78(a->player, a->id, duel_088AA224(a)) == 0) {
            duel_0884ACD8(a->player, (void *)0xD);
            return 0x6E;
        }
        duel_0884ACD8(a->player, (void *)0x30);
        return 0x7F;
    case 0x7F:
        duel_0884C588(a->player, 6, a->id, duel_088AA224(a));
        return 0x7E;
    case 0x7E:
        duel_088263A8(a->player, duel_0884C7CC(), 1, a->id << 16);
        return 100;
    case 0x6E:
        duel_088328FC(1 - a->player);
        return 100;
    case 100:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}

#ifdef NON_MATCHING /* bits da marca da zona F e alocação dos $s */
int duel_0888D74C(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;
    int uid;
    unsigned int v;
    unsigned int o;
    unsigned int i;
    unsigned int m;
    unsigned char h;

    if (a->b3_4 == 0) {
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            if (duel_0884E440(a, p, z) != 0) {
                uid = CARD_UID(duel.players[p & 1].field[z].card);
                if (duel_088D16C4(a, p, z, 0x40) == 0) {
                    return 0;
                }
                if (card_is_attribute9(duel.players[p & 1].field[z].card.id) == 0) {
                    duel_088C77D4(a->player, a->id, 0x21, duel_field_controller(p, z, 1) + uid * 2);
                }
            }
            return 0;
        }
        return 0;
    }
    v = a->idx;
    uid = v >> 1;
    o = v & 1;
    t = duel_card_locate(uid);
    p = t & 0xFF;
    z = (unsigned short)t >> 8 & 0xFF;
    i = t >> 16 & 0xFFFF;
    switch (duel_step) {
    case 0x80:
        if (z == 0xF) {
            m = duel.players[p & 1].marks[i];
            if ((m & 0xFF) == 0x40) {
                h = (unsigned short)m >> 8 & 0xFF;
                duel_088CB778(p, 0xF, uid, (unsigned char)(h & 0xF) & 1, (unsigned char)((h & 0xF0) >> 4), h >> 1 & 1, h >> 2 & 1);
                return 0x7F;
            }
        }
        return 0;
    case 0x7F:
        if (card_is_monster(duel.list[uid].id) == 0) {
            duel_088D1438(p, z, 0, 0);
            return 0;
        }
        if (p != o) {
            duel_088C7E68(p, z, 0, (duel.players[p & 1].field[z].flags & 1) == 0);
        }
        duel_088C7E68(p, z, 0x15, 0);
        return 0;
    }
    return 0;
}
#endif

int duel_0888DA10(DuelAct *a, DuelAct *b)
{
    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_0886CB44(1 - a->player, 1, 0, 0, 1) == 0) {
            return 0x7E;
        }
        duel_08832950(1 - a->player, a->id);
        return 0x7F;
    case 0x7F:
        if (duel_w_0891968C != 0) {
            duel_08832670(1 - a->player, 1, 7, 1);
            return 0;
        }
        return 0x7E;
    case 0x7E:
        if (b != 0) {
            if (card_get_type(b->id) == 0x16) {
                b->b4_3 = b->b4_2 = b->b4_1 = 1;
                duel_088CBAA4(b->player, b->zone, b->uid);
                duel_088D15B4(a, b->player, b->zone);
            }
        }
        return 0;
    }
    return 0;
}

int duel_0888DBC8(DuelAct *a)
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
        if (duel.players[p & 1].field[z].pos == 0) {
            return 0;
        }
        if (duel.players[p & 1].field[z].faceup != 0) {
            return 0;
        }
        duel_088C97F0(a, p, z, 1, 2);
    }
    return 0;
}

int duel_0888DC9C(DuelAct *a)
{
    unsigned int t = duel_act_target(a, 0);
    unsigned int p = t & 0xFF;
    unsigned int z = (unsigned short)t >> 8 & 0xFF;
    int id = duel.players[p & 1].field[z].card.id;

    switch (duel_step) {
    case 0x80:
        if (duel_088A9008(a, 0, 0) != 0) {
            if (duel.players[p & 1].field[z].faceup == 0 && duel.players[p & 1].field[z].pos != 0) {
                duel_088C96EC(p, z, 1);
                return 0x7F;
            }
        }
        return 0;
    case 0x7F:
        duel_088C4508(p, id);
        duel_088C7B90(0x1E);
        return 0x7E;
    case 0x7E:
        if (card_attribute_to_group(id) != 0 && duel_0887DB24(a) != 0) {
            return 0;
        }
        duel_088C96EC(p, z, 0);
        return 0;
    }
    return 0;
}

int duel_0888DE18(DuelAct *a)
{
    unsigned int loc;
    unsigned int q;
    unsigned int i;

    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    if (duel_count_free_monster_zones(a->player) == 0) {
        return 0;
    }
    if (a->n7 == 1) {
        loc = duel_card_locate(a->w[0]);
        q = loc & 0xFF;
        i = loc >> 16 & 0xFFFF;
        if (((unsigned short)loc >> 8 & 0xFF) == 0xE && duel_grave_card_usable(a->player, q, i) != 0) {
            duel_088263A8(a->player, &duel.players[q & 1].grave[i], 1, 0x20);
        }
    }
    return 0;
}

int duel_0888DF08(DuelAct *a)
{
    unsigned int t;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_088A9008(a, 0, 0) != 0) {
            t = duel_act_target(a, 0);
            if (duel_088D192C(a, t & 0xFF, (unsigned short)t >> 8 & 0xFF) == 0) {
                return 0;
            }
            duel_w_089148FC = duel.players[a->player & 1].n_hand;
            duel_w_08914900 = 0;
            return 0x7F;
        }
        return 0;
    case 0x7F:
        if (duel.players[a->player & 1].n_hand != 0) {
            if (a->player != duel.players[a->player & 1].hand[0].owner) {
                duel_w_08914900 = 1;
            }
            duel_088CA384(a->player, 0, 1);
            return 0x7F;
        }
        return 0x7E;
    case 0x7E:
        duel_088C78E0(a->player);
        if (duel_w_08914900 != 0) {
            duel_088C78E0(1 - a->player);
        }
        return 0x7D;
    case 0x7D:
        duel_088C5E70(a->player, duel_w_089148FC);
        return 0x7C;
    }
    return 0;
}

int duel_0888E0E4(DuelAct *a)
{
    unsigned int p;

    if (a->b4_2) {
        return 0;
    }
    p = a->player;
    if (a->uid != CARD_UID(duel.players[p & 1].field[a->zone].card)) {
        return 0;
    }
    if (a->w[0] != 0) {
        duel_088C4604(a, 1 - p, 500);
    }
    return 0;
}

#ifdef NON_MATCHING /* queda de 0x80 em 0x7F (beqzl com v0=0) e registradores do caso 0x7C */
int duel_0888E1A4(DuelAct *a, int b)
{
    switch (duel_step) {
    case 0x80:
        if (duel_0884E9E8(a, b) != 0) {
            return 0x7F;
        }
    case 0x7F:
        duel_088328E8(a->player, a->id, a->player);
        return 0x7E;
    case 0x7E:
        duel_w_089148FC = duel_w_0891968C >> 8 & 0xFF;
        duel_088328E8(1 - a->player, a->id, a->player);
        return 0x7D;
    case 0x7D:
        duel_w_08914900 = duel_w_0891968C >> 8 & 0xFF;
        return 0x7C;
    case 0x7C:
        if (duel_act_immune(a, a->player, duel_w_089148FC) != 0) {
            return 0;
        }
        if (duel_slot_ef098(a->player, duel_w_089148FC, a->id) == 0) {
            return 0;
        }
        if (duel_act_immune(a, 1 - a->player, duel_w_08914900) != 0) {
            return 0;
        }
        if (duel_slot_ef098(1 - a->player, duel_w_08914900, a->id) == 0) {
            return 0;
        }
        duel_088C87A8(a->player, duel_w_089148FC, a->id, 5, 1 - a->player);
        duel_088C87A8(1 - a->player, duel_w_08914900, a->id, 5, a->player);
        duel_088CA270(a->player, a->player | (duel_w_089148FC & 0xFF) << 8, (1 - a->player & 0xFF) | (duel_w_08914900 & 0xFF) << 8);
        return 0x7B;
    case 0x7B:
        duel_088C7E68(a->player, duel_w_089148FC, 0x15, 1);
        duel_088C7E68(1 - a->player, duel_w_08914900, 0x15, 1);
        return 0;
    }
    return 0;
}
#endif

int duel_0888E3F4(DuelAct *a)
{
    unsigned int t0;
    unsigned int t1;
    unsigned int p0;
    int z0;
    unsigned int z1;
    unsigned int p1;

    if (a->b4_2) {
        return 0;
    }
    if (a->n7 == 2) {
        t0 = duel_act_target(a, 0);
        t1 = duel_act_target(a, 1);
        z0 = (unsigned short)t0 >> 8 & 0xFF;
        p0 = t0 & 0xFF;
        z1 = (unsigned short)t1 >> 8 & 0xFF;
        p1 = t1 & 0xFF;
        switch (duel_step) {
        case 0x80:
            if (duel_088A9008(a, 0, 0) != 0) {
                if (duel_088A9008(a, 1, 0) != 0 && p0 != a->player && p1 == a->player) {
                    duel_088D14B8(a, p0, z0);
                    return 0x7F;
                }
            }
            return 0;
        case 0x7F:
            duel_088C87D8(a, p1, z1, a->id, 5, p0);
            if (duel_act_immune(a, p1, z1) == 0) {
                if (duel.players[p0 & 1].field[z0].card.id != 0) {
                    z0 = duel_pick_monster_zone(p0);
                }
                if (z0 >= 0) {
                    duel_088CA184(t1, (unsigned char)p0 | (unsigned char)z0 << 8);
                }
            }
            return 0;
        }
    }
    return 0;
}
