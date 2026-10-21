// @range 0x0889C150-0x0889CD38
/* Card effect handlers (DuelAct *a), after effects_0889A75C.c. */
#include "duel.h"

extern int duel_step;
extern unsigned short duel_w_0891968C;
extern int duel_turn_player;
extern int duel_w_089148FC;

unsigned int duel_0884E4E4(DuelAct *a);
void duel_088D184C(DuelAct *a, unsigned int mask, int b);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);
void duel_088C67F0(unsigned int p, int id, int a, int b);
int card_is_attribute9(int id);
int card_attribute_icon_is3(int id);
void duel_088C78E0(unsigned int p);
int duel_slot_efb84(unsigned int p, int z);
int duel_088D1D58(DuelAct *a, int b);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
int duel_count_field(unsigned int p);
void duel_088D1C70(DuelAct *a, unsigned int p, int z);
void duel_08832950(unsigned int p, unsigned short id);
void duel_088328E8(unsigned int p, unsigned short id, int a);
int duel_0884E690(DuelAct *a, int (*fn)());
int duel_08860788();
int duel_0884E398(DuelAct *a, int b);
void duel_088C7810(unsigned int p, int a);
void duel_088C7B90(int n);
int duel_hand_count_name(unsigned int p, unsigned short id);
int card_same_name(int id, int id2);
void duel_088328A4(unsigned int p, int a, int b);
void duel_088CACF0(unsigned int p, DuelCard *c, int a);
void duel_0884ACD8(unsigned int p, void *msg);
void duel_08832870(unsigned int p, int a);
int duel_act_target2(DuelAct *a, int i);
int duel_f_find_uid(unsigned int p, int uid);
void duel_088CB14C(DuelAct *a, unsigned int p, DuelCard *c);
int duel_count_free_monster_zones(unsigned int p);
int duel_phase_ok2(unsigned int p);
unsigned int duel_card_locate(unsigned int uid);
int duel_zone_f_card_usable(unsigned int p, unsigned int q, int i);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
int card_is_monster(int id);
void duel_088CB400(unsigned int p, DuelCard *c, int a);
void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);
int duel_0887F80C(DuelAct *a, DuelAct *b);
int duel_slot_effect_arg(unsigned int p, int z, int id);
void duel_088D1438(unsigned int p, int z, int a, int b);
int duel_088C4604(DuelAct *a, unsigned int p, int n);

int duel_0889C150(DuelAct *a, int b)
{
    int n;
    unsigned int i;
    int k;

    k = a->w[0];
    switch (duel_step) {
    case 0x80:
        if (duel_0884E398(a, b) == 0) {
            return 0;
        }
        duel_088C7810(1 - a->player, 1);
        duel_088C7B90(30);
        n = duel_hand_count_name(1 - a->player, k);
        if (n == 0) {
            return 0x78;
        }
        if (n == 1) {
            for (i = 0; i < duel.players[(1 - a->player) & 1].n_hand; i++) {
                if (card_same_name(duel.players[(1 - a->player) & 1].hand[i].id, k)) {
                    duel_w_0891968C = i;
                }
            }
            return 0x7E;
        }
        return 0x7F;
    case 0x7F:
        duel_088328A4(1 - a->player, a->id, k);
        return 0x7E;
    case 0x7E:
        duel_088CACF0(1 - a->player, &duel.players[(1 - a->player) & 1].hand[duel_w_0891968C], 1);
        return 0x64;
    case 0x78:
        duel_0884ACD8(a->player, (void *)0x11);
        return 0x77;
    case 0x77:
        duel_08832870(a->player, 1);
        return 0x64;
    case 0x64:
        duel_088C7810(1 - a->player, 0);
        return 0;
    }
    return 0;
}

int duel_0889C39C(DuelAct *a)
{
    int g;
    unsigned char n;

    if ((n = a->n7) != 0) {
        g = duel_f_find_uid(a->player, duel_act_target2(a, n - 1));
        if (g >= 0) {
            duel_088CB14C(a, a->player, &duel.players[a->player & 1].zone_f[g]);
        }
        a->n7--;
        return 0x80;
    }
    return 0;
}

int duel_0889C438(DuelAct *a)
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
    if (a->n7 == 1) {
        loc = duel_card_locate(duel_act_target2(a, 0));
        p = loc & 0xFF;
        i = loc >> 16 & 0xFFFF;
        if (((unsigned short)loc >> 8 & 0xFF) == 0xF && duel_zone_f_card_usable(a->player, p, i)) {
            duel_088263A8(a->player, &duel.players[p & 1].zone_f[(unsigned short)i], 1, 0);
        }
    }
    return 0;
}

int duel_0889C538(DuelAct *a)
{
    int i;
    unsigned int q;

    q = duel_turn_player ^ duel_w_089148FC;
    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 0;
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        for (i = duel.players[q & 1].n_grave - 1; i >= 0; i--) {
            if (!card_is_monster(duel.players[q & 1].grave[i].id)) {
                continue;
            }
            duel_088CB400(q, &duel.players[q & 1].grave[i], q != a->player);
            return 0x7F;
        }
        return 0x78;
    case 0x78:
        if (++duel_w_089148FC < 2) {
            return 0x7F;
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_0889C68C(DuelAct *a, DuelAct *b)
{
    switch (duel_step) {
    case 0x80:
        duel_088C77D4(1 - a->player, a->id, 5, b->id);
        return 0x7F;
    case 0x7F:
        duel_0887F80C(a, b);
        return 0;
    }
    return 0;
}

int duel_0889C6F8(DuelAct *a)
{
    int r;

    r = duel_slot_effect_arg(a->player, a->zone, a->id);
    duel_088D1438(a->player, a->zone, 0, 0);
    if (r >= 0) {
        duel_088C4604(a, r, 3000);
    }
    return 0;
}

int duel_0889C774(DuelAct *a)
{
    unsigned int q;

    q = duel_w_089148FC;
    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = duel_turn_player;
        return 0x7F;
    case 0x7F:
        if (duel_count_field(q) < 6) {
            return 0x78;
        }
        if (duel_0884E690(a, duel_08860788) == 0) {
            return 0x78;
        }
        duel_08832950(q, a->id);
        return 0x7E;
    case 0x7E:
        if (duel_count_field(q) < 6) {
            return 0x78;
        }
        if (duel_0884E690(a, duel_08860788) == 0) {
            return 0x78;
        }
        duel_088328E8(q, a->id, (unsigned short)(a->player | a->zone << 8));
        return 0x7D;
    case 0x7D:
        duel_088D1C70(a, q, duel_w_0891968C >> 8 & 0xFF);
        return 0x7E;
    case 0x78:
        if (q == duel_turn_player) {
            duel_w_089148FC = q ^ 1;
            return 0x7F;
        }
        return 0;
    }
    return 0;
}

int duel_0889C8E0(DuelAct *a)
{
    int i;
    int z;
    int bit;
    int k;
    unsigned int q;

    int flag;
    unsigned int mask;
    unsigned int p;
    int id;

    q = 1 - a->player;
    flag = 0;
    mask = duel_0884E4E4(a);
    duel_088C7BA4();
    duel_088D184C(a, mask, 0);
    for (i = 0; i < 2; i++) {
        p = duel_turn_player ^ i;
        bit = p << 4;
        for (z = 0; z <= 10; bit++, z++) {
            if (mask & 1 << bit) {
                id = duel.players[p & 1].field[z].card.id;
                for (k = duel.players[q & 1].n_hand - 1; k >= 0; k--) {
                    if (card_same_name(duel.players[q & 1].hand[k].id, id)) {
                        duel_088CACF0(q, duel_field_card_deb2c(q, 0xB, k), q != a->player);
                    }
                }
                duel_088C67F0(q, id, 1, q != a->player);
                if (!card_is_attribute9(id) && !card_attribute_icon_is3(id)) {
                    flag = 1;
                }
            }
        }
    }
    duel_088C7CF0();
    if (flag) {
        duel_088C7B90(15);
        duel_088C78E0(q);
    }
    return 0;
}

int duel_0889CAF4(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    duel_088D184C(a, duel_0884E4E4(a), 0);
    return 0;
}

int duel_0889CB48(DuelAct *a)
{
    int p;
    int z;
    unsigned int q;

    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 0;
        for (p = 0; p < 2; p++) {
            for (z = 0; z <= 10; z++) {
                if (duel_slot_efb84(p, z)) {
                    duel_w_089148FC++;
                }
            }
        }
        duel_088D1D58(a, -1);
        return 0x7F;
    case 0x7F:
        if (duel.players[a->player & 1].n_hand != 0) {
            duel_088CAE24(a->player, &duel.players[a->player & 1].hand[0], 0);
            duel_w_089148FC++;
            return 0x7F;
        }
        return 0x7E;
    case 0x7E:
        q = 1 - a->player;
        if (duel.players[q & 1].n_hand != 0) {
            duel_088CAE24(q, &duel.players[q & 1].hand[0], 0);
            duel_w_089148FC++;
            return 0x7E;
        }
        return 0x7D;
    case 0x7D:
        duel_088C4604(a, 1 - a->player, duel_w_089148FC * 300);
        return 0;
    }
    return 0;
}
