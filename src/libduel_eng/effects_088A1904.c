// @range 0x088A1904-0x088A3430
// @asm 0x088A2950-0x088A2B64
// @asm 0x088A332C-0x088A3430
/* Card effect handlers (DuelAct *a). */
#include "duel.h"

extern int duel_step;
extern int duel_w_089148FC;
extern unsigned short duel_w_0891968C;
extern unsigned short duel_h_0891968E;
extern int duel_order_ctr;

void duel_088327F8(unsigned int p, int a, int b);
int duel_088DFCC8(unsigned int uid);
int card_is_monster(int id);
int card_get_level(int id);
void duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_088D1438(unsigned int p, int z, int a, int b);
void duel_088CC4B4(unsigned int p, int z, int a);
void duel_088C5E70(unsigned int p, int n);
int duel_count_faceup_name(unsigned int p, int id);
int duel_field_kind(unsigned int p, int z);
void duel_08832988(unsigned int p, unsigned short id, unsigned short n, int a);
void duel_088CC178(unsigned int p, int z, unsigned short a);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_0884EC10();
int duel_0884E440(DuelAct *a, unsigned int p, int z);
void duel_088C87D8(DuelAct *a, unsigned int p, int z, unsigned short id, int b, int c);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
void duel_088D192C(DuelAct *a, unsigned int p, int z);
extern int duel_player_type[2];
extern int duel_w_08919654;
extern int duel_w_08919624;
extern int duel_phase;
int duel_088BFF78(unsigned int p, int id, int b);
int duel_0887FCA8(int id);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)(int id));
void duel_0884ACD8(unsigned int p, int msg);
void duel_08835978(int (*fn)(int id));
int duel_08835AC0(void);
void duel_088C77EC(unsigned int p, int uid);
void duel_088CA384(unsigned int p, int i, int f);
void duel_088C78E0(unsigned int p);
void duel_088C7DD4(unsigned int p, int a, int b);
unsigned int duel_card_locate(unsigned int uid);
void duel_088CBAA4(unsigned int p, int z, int uid);
void duel_088D1F1C(DuelAct *a, int i);
void duel_088D147C(DuelAct *a, unsigned int p, int z);
void duel_088C7478(unsigned int p, int i, DuelCard *c, int a);
void duel_088C7490(unsigned int p, int i, int a);
int duel_0884E690(DuelAct *a, int (*fn)());
unsigned int duel_0884E538(DuelAct *a, int (*fn)());
int duel_088559C0();
int duel_08857FEC();
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_08861480(unsigned short id, int i);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
void duel_08832974();
void duel_088D13CC(DuelAct *a, unsigned int mask);
int duel_0884E398();
unsigned int duel_act_target(DuelAct *a, int i);
int duel_field_atk(unsigned int p, int z);
int duel_088C97F0(DuelAct *a, unsigned int p, int z, int b, int c);
int duel_phase_ok2(unsigned int p);
int duel_count_free_monster_zones(unsigned int p);
void duel_088C7BA4(void);
void duel_0884AD40(int msg, int uid);
int duel_d_find_name(unsigned int p, int id);
int duel_c_find_name(unsigned int p, int id);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_088C7CF0(void);
void duel_088328FC(unsigned int p);
int duel_rand(int n);
void duel_0884AF88(int msg);
void duel_08832AF4(unsigned int p, int z, int a, int b);
void duel_0884C588(unsigned int p, int a, int id, int b);
DuelCard *duel_0884CCBC(int k);
int duel_0887F728(DuelAct *a, int b);
int duel_pick_st_zone(unsigned int p);
void duel_088CA184(unsigned short loc, unsigned short to);
void duel_088C8B70(unsigned int p, unsigned short loc, unsigned short t);
int duel_08855908();
void duel_088359E8(unsigned int p, int id, int (*fn)());
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern unsigned int duel_turn_player;
void duel_088C9D24(DuelAct *a, unsigned int p, int z);
int duel_0887F8E4(DuelAct *a, int b);
void duel_08832894(unsigned int p, unsigned short id);

int duel_088A1904(DuelAct *a)
{
    unsigned int q;
    int id;

    switch (duel_step) {
    case 0x80:
        q = 1 - a->player;
        if (duel.players[q & 1].n_hand == 0) {
            return 0;
        }
        duel_088327F8(q, 1, 1);
        return 0x7F;
    case 0x7F:
        if (duel_088DFCC8(duel_w_0891968C) != 0) {
            id = duel.list[duel_w_0891968C].id;
            if (card_is_monster(id)) {
                duel_088C4604(a, 1 - a->player, card_get_level(id) * 100);
            }
        }
        return 0;
    }
    return 0;
}

int duel_088A1A18(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    duel_088D1438(p, z, 0, 0);
    return 0;
}

int duel_088A1AB4(DuelAct *a)
{
    unsigned char z = a->zone;
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    duel_088CC4B4(p, z, -1);
    return 0;
}

int duel_088A1B70(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_088C5E70(a->player, 1);
        duel_w_089148FC = duel_count_faceup_name(a->player, 0x181E);
        return 0x7F;
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

int duel_088A1C18(DuelAct *a)
{
    unsigned char z;
    unsigned int p;

    switch (duel_step) {
    case 0x80:
        z = a->zone;
        p = a->player;
        if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
            return 0;
        }
        if (duel.players[p & 1].field[z].faceup == 0) {
            return 0;
        }
        duel_08832988(a->player, a->id, 1, 1 << duel_field_kind(p, z) ^ 0xFF);
        return 0x7F;
    case 0x7F:
        duel_088CC178(a->player, a->zone, duel_h_0891968E);
        duel_088C87A8(a->player, a->zone, a->id, 3, duel_order_ctr);
        return 0;
    }
    return 0;
}

void duel_088A1D64(DuelAct *a, DuelAct *b)
{
    a->id = b->id;
    duel_0884EC10(a, b);
    a->id = 0x183E;
}

int duel_088A1D98(DuelAct *a)
{
    int p;
    int z;

    for (p = 0; p < 2; p++) {
        for (z = 0; z < 5; z++) {
            if (duel_0884E440(a, p, z)) {
                duel_088C87D8(a, p, z, a->id, 2, 0);
            }
        }
    }
    duel_088C77D4(a->player, a->id, 1, 0);
    return 0;
}

int duel_088A1E40(DuelAct *a)
{
    switch (a->w[2]) {
    case 1:
        duel_088C5E70(a->player, 1);
        break;
    case 2:
        duel_088327F8(1 - a->player, 1, 1);
        break;
    }
    return 0;
}

int duel_088A1EA0(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, &t)) {
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z)) {
            duel_088D192C(a, p, z);
        }
    }
    return 0;
}

int duel_088A1F20(DuelAct *a)
{
    unsigned int p = a->player ^ a->opp;
    unsigned int i;
    unsigned int uid;

    switch (duel_step) {
    case 0x80:
        for (i = 0; i < duel.players[p & 1].n_hand; i++) {
            if (!duel_088BFF78(p, a->id, i)) {
                continue;
            }
            return 0x7F;
        }
        return 0;
    case 0x7F:
        if (duel_player_type[p] == 1) {
            duel_0881956C(p, a->id, duel_0887FCA8);
        } else {
            duel_0884ACD8(p, 0x1F);
            duel_08835978(duel_0887FCA8);
        }
        return 0x7E;
    case 0x7E:
        if (duel_08835AC0() != 0) {
            duel_w_089148FC = duel_w_08919654;
            return 0x7D;
        }
        return 0x7F;
    case 0x7D:
        i = duel_w_089148FC;
        uid = CARD_UID(duel.players[p & 1].hand[i]);
        duel_088C77EC(p, uid);
        duel_088CA384(p, i, 1);
        duel_088C78E0(uid & 1);
        return 0x7C;
    case 0x7C:
        duel_088C7DD4(p, 2, 0);
        duel_088C5E70(p, 1);
        return 0;
    }
    return 0;
}

int duel_088A2130(DuelAct *a, DuelAct *b)
{
    unsigned int t;
    unsigned int q;

    if (b == 0) {
        return 0;
    }
    b->b4_3 = b->b4_2 = b->b4_1 = 1;
    t = duel_card_locate(b->uid);
    duel_088CBAA4(b->player, b->zone, b->uid);
    if (((unsigned short)t >> 8 & 0xFF) < 0xB) {
        if ((unsigned short)(t >> 16) != 0) {
            duel_088D1F1C(a, duel_special.player[1] == b->player && duel_special.zone[1] == b->zone);
        } else {
            duel_088D147C(a, b->player, b->zone);
        }
    } else if (((unsigned short)t >> 8 & 0xFF) == 0xB) {
        duel_088C7478(b->player, (unsigned short)(t >> 16), &duel.players[b->player & 1].hand[(unsigned short)(t >> 16)], b->player != a->player);
    } else if (((unsigned short)t >> 8 & 0xFF) == 0xD) {
        q = t & 0xFF;
        duel_088C7490(q, (unsigned short)(t >> 16), q != a->player);
        duel_088C78E0(q);
    }
    return 0;
}

int duel_088A2310(DuelAct *a)
{
    int i;
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    for (i = 0; i < a->n7; i++) {
        if (duel_088A9008(a, i, &t)) {
            p = t & 0xFF;
            z = (unsigned short)t >> 8 & 0xFF;
            if (duel_0884E440(a, p, z)) {
                duel_088C87D8(a, p, z, a->player | a->zone << 8, 0x16, 0);
            }
        }
    }
    return 0;
}

int duel_088A2454(DuelAct *a)
{
    unsigned int m;
    unsigned int q;
    int i;
    int s1;
    int s3;
    int s2;

    switch (duel_step) {
    case 0x80:
        m = 0;
        if (duel_0884E690(a, duel_088559C0)) {
            m |= 1;
        }
        if (duel_0884E690(a, duel_08857FEC)) {
            m |= 2;
        }
        q = 1 - a->player;
        if (duel.players[q & 1].n_hand != 0) {
            m |= 4;
        }
        if ((unsigned int)duel.players[a->player & 1].n_d >= 2 && (duel_phase == 0 || duel_find_effect_card(q, 0x178B, -1) == 0)) {
            m |= 8;
        }
        if (m == 0) {
            return 0;
        }
        if (duel_player_type[a->player] == 1) {
            for (i = 0; i < 4; i++) {
                if (m & (1 << i)) {
                    break;
                }
            }
            duel_w_08919624 = i;
        } else {
            s3 = duel_08861480(a->id, 0);
            s2 = duel_08861480(a->id, 1);
            s1 = duel_08861480(a->id, 2);
            duel_0884ADAC(0x103, s3, s2, s1, duel_08861480(a->id, 3), m);
        }
        return 0x7F;
    case 0x7F:
        duel_08832974(1 - a->player, a->id, duel_w_08919624);
        return 0x7E;
    case 0x7E:
        switch (duel_w_08919624) {
        case 0:
            duel_088D13CC(a, duel_0884E538(a, duel_088559C0));
            break;
        case 1:
            duel_088D13CC(a, duel_0884E538(a, duel_08857FEC));
            break;
        case 2:
            duel_088327F8(1 - a->player, 1, 2);
            break;
        case 3:
            duel_088C5E70(a->player, 2);
            break;
        }
        return 0;
    }
    return 0;
}

int duel_088A2700(DuelAct *a)
{
    unsigned int p = duel_special.player[1];
    int z = duel_special.zone[1];
    unsigned int t;
    unsigned int q;
    unsigned int y;

    if (duel_0884E398(a) == 0) {
        return 0;
    }
    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        q = t & 0xFF;
        y = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, q, y)) {
            duel_088C87D8(a, p, z, a->id, 4, (unsigned short)duel_field_atk(q, y));
        }
    }
    return 0;
}

int duel_088A27D4(DuelAct *a)
{
    int p;
    int z;

    for (p = 0; p < 2; p++) {
        for (z = 0; z <= 4; z++) {
            if (duel_0884E440(a, p, z)) {
                duel_088C87D8(a, p, z, a->id, 5, 0);
            }
        }
    }
    return 0;
}

int duel_088A2864(DuelAct *a)
{
    int i = duel_special.act[0].uid == a->uid;
    unsigned int p = duel_special.act[i].unk0;
    int z = duel_special.act[i].unk4;

    if (duel_special.act[i].uid == CARD_UID(duel.players[p & 1].field[z].card)) {
        if (duel.players[p & 1].field[z].pos == 0) {
            duel_088C97F0(a, p, z, 0, 1);
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* o `return 0x7D` do case 0x7E não reaproveita o v0 do teste e duplica os lbu nos delay slots */
int duel_088A2950(DuelAct *a)
{
    unsigned int p;

    switch (duel_step) {
    case 0x80:
        if (duel_count_free_monster_zones(a->player) == 0) {
            return 0;
        }
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_088BFF78(a->player, a->id, 0) == 0) {
            duel_0884ACD8(a->player, 0xD);
            return 0x78;
        }
        duel_088C7BA4();
        duel_w_08919624 = 1;
        return 0x7E;
    case 0x7F:
        if (duel_count_free_monster_zones(a->player) != 0 && duel_088BFF78(a->player, a->id, 0) != 0) {
            if (duel_player_type[a->player] == 1) {
                duel_w_08919624 = 1;
            } else {
                duel_0884AD40(0x35, a->uid);
            }
            return 0x7E;
        }
        return 0x7D;
    case 0x7E:
        if (duel_w_08919624 != 0) {
            p = a->player;
            duel_088263A8(p, &duel.players[p & 1].zone_d[duel_d_find_name(p, a->id)], 1, 0);
            return 0x7F;
        }
        return 0x7D;
    case 0x7D:
        duel_088C78E0(a->player);
        duel_088C7CF0();
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
#endif

int duel_088A2B64(DuelAct *a)
{
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
        if (duel_h_0891968E != 0) {
            duel_088C4604(a, 1 - a->player, 1000);
        } else {
            duel_088C4604(a, a->player, 1000);
        }
        return 0;
    }
    return 0;
}

int duel_088A2C54(DuelAct *a)
{
    int i = duel_c_find_name(a->player, a->w[0]);

    if (duel_phase_ok2(a->player) == 0) {
        return 0;
    }
    if (duel_count_free_monster_zones(a->player) == 0) {
        return 0;
    }
    if (i >= 0) {
        duel_088263A8(a->player, &duel.players[a->player & 1].zone_c[i], 1, 0);
    }
    return 0;
}

int duel_088A2D0C(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_count_free_monster_zones(a->player) < 2) {
            return 0;
        }
        if (duel_088BFF78(a->player, a->id, 0) < 2) {
            duel_0884ACD8(a->player, 0x15);
            return 0x6E;
        }
        duel_0884ACD8(a->player, 0x11D);
        return 0x7F;
    case 0x7F:
        duel_0884C588(a->player, 8, a->id, 0);
        return 0x7D;
    case 0x7D:
        a->w[0] = 0;
        a->w[1] = duel_w_08919624;
        duel_088C7BA4();
        return 0x7C;
    case 0x7C:
        if (a->w[0] < a->w[1]) {
            duel_088263A8(a->player, duel_0884CCBC(a->w[0] + 1), 1, a->id << 16);
            a->w[0]++;
            return 0x7C;
        }
        duel_088C7CF0();
        return 0x64;
    case 0x6E:
        duel_088328FC(1 - a->player);
        return 0x64;
    case 0x64:
        duel_088C78E0(a->player);
        return 0;
    }
    return 0;
}

int duel_088A2EB0(DuelAct *a, int b)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;
    int k;

    if (a->kind == 2) {
        return duel_0887F728(a, b);
    }
    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z)) {
            k = duel_pick_st_zone(a->player);
            if (k >= 0) {
                duel_088CA184(a->player | a->zone << 8, a->player | (unsigned char)k << 8);
                duel_088C8B70(a->player, a->player | (unsigned char)k << 8, (unsigned char)p | (unsigned char)z << 8);
            } else {
                duel_088D1438(a->player, a->zone, 0, a->id);
            }
        }
    }
    return 0;
}

int duel_088A2FE0(DuelAct *a)
{
    switch (duel_step) {
    case 0x80:
        duel_08832AF4(a->player, a->zone, 3, 1);
        return 0x7F;
    case 0x7F:
        if (duel_h_0891968E == 0) {
            return 0;
        }
        if (duel_0884E690(a, duel_08855908) == 0) {
            return 0;
        }
        duel_088C7BA4();
        duel_w_089148FC = 0;
        return 0x7E;
    case 0x7E:
        if (duel_0884E690(a, duel_08855908) == 0) {
            return 0x7C;
        }
        if (duel_w_089148FC == 0) {
            duel_0884ACD8(a->player, 0x6D);
        }
        duel_088359E8(a->player, a->id, duel_08855908);
        return 0x7D;
    case 0x7D:
        if (duel_08835AC0() != 0) {
            duel_088D147C(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            return ++duel_w_089148FC < duel_h_0891968E ? 0x7E : 0x7C;
        }
        return 0x7E;
    case 0x7C:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_088A3178(DuelAct *a)
{
    int i;
    unsigned int q;
    int z;

    for (i = 0; i < 2; i++) {
        q = duel_turn_player ^ i;
        for (z = 0; z <= 10; z++) {
            if (duel_0884E440(a, q, z)) {
                duel_088C9D24(a, q, z);
            }
        }
    }
    return 0;
}

int duel_088A3210(DuelAct *a)
{
    switch (a->w[2]) {
    case 1:
        duel_088C77D4(1 - a->player, a->id, (unsigned char)((a->player != duel_turn_player) + 1) << 4 | 1, 0);
        break;
    case 2:
        duel_088C77D4(1 - a->player, a->id, 1, 1);
        break;
    }
    return 0;
}

int duel_088A32A4(DuelAct *a)
{
    unsigned int t;
    unsigned int p;
    unsigned int z;

    if (duel_088A9008(a, 0, 0) != 0) {
        t = duel_act_target(a, 0);
        p = t & 0xFF;
        z = (unsigned short)t >> 8 & 0xFF;
        if (duel_0884E440(a, p, z)) {
            duel_088D192C(a, p, z);
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* `a` vai para $a2 e `p` para $a0 no case 0x7F (o original mantém `a` em $a0) */
int duel_088A332C(DuelAct *a, int b)
{
    unsigned int p;
    unsigned int n;

    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = duel.players[a->player & 1].n_hand;
        duel_0887F8E4(a, b);
        return 0x7F;
    case 0x7F:
        p = a->player;
        n = duel.players[p & 1].n_hand;
        if (n == 0) {
            return 0;
        }
        if (duel_w_089148FC < n) {
            duel_08832894(p, a->id);
            return 0x7E;
        }
        return 0;
    case 0x7E:
        if (duel_h_0891968E != 0) {
            duel_088CA384(a->player, duel_w_0891968C, 0);
        }
        return 0;
    }
    return 0;
}
#endif
