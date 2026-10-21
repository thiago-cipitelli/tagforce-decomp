// @range 0x088AE378-0x088AF390
// @data 0x089119F0-0x08911A20
// @asm 0x088AE378-0x088AE548
// @asm 0x088AEE0C-0x088AF0DC
/* Card effect handlers (DuelAct *a): target selection on duel_w_08914908, after effects_088ACCF8.c. */
#include "duel.h"

extern int duel_w_08914908;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern unsigned short duel_h_0891968E;

void duel_088A8B94(DuelAct *a, unsigned short loc);
void duel_088A8D40(DuelAct *a, unsigned int p, int z);
void duel_088A8EA0(DuelAct *a, unsigned int p, int z, int uid);
int duel_088AA1C0(DuelAct *a, int b);
int duel_088AA5F4(DuelAct *a);
int duel_08817598(DuelAct *a, int b, int id);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
int duel_08835AC0(void);
void duel_088359E8(unsigned int p, int id, int (*fn)(DuelAct *a, unsigned int p, int z));
void duel_0884ACD8(unsigned int p, void *msg);
unsigned int duel_act_target(DuelAct *a, int i);
int field_get_card(unsigned int p, int z);
unsigned short *card_get_name(int id);
void duel_wfmt_s(char *dst, char *fmt, unsigned short *arg);
int duel_088BFFD8();
void duel_088329D0();
unsigned int duel_card_locate(unsigned int uid);
void duel_088CC178(unsigned int p, int z, unsigned int v);
void duel_08832960(unsigned int p, unsigned short id, int a);
void duel_08832988(unsigned int p, unsigned short id, unsigned short n, int a);
int card_is_attribute9(int id);
int duel_field_e5ae0(unsigned int p, int z);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
void duel_088D1AC8(unsigned int p, int z);
void duel_088C44E4(unsigned int p, int id);

#ifdef NON_MATCHING /* o retorno 1 padrão do switch externo e o do interno são blocos separados no original */
int duel_088AE378(DuelAct *a, int b)
{
    char buf[512];
    unsigned int t;
    int r;

    switch (a->w[2]) {
    default:
        return 1;
    case 1:
    case 3:
        switch (duel_w_08914908) {
        case 0:
        case 1:
            a->w[2] = 1;
            if (duel_088AA1C0(a, b) != 0) {
                duel_w_08914908 = 2;
            }
            return 0;
        case 2:
            a->w[2] = 3;
            if (duel_player_type[a->player] == 1) {
                return duel_08817598(a, b, a->id);
            }
            t = duel_act_target(a, 0);
            duel_wfmt_s(buf, (char *)0x45, card_get_name(field_get_card(t & 0xFF, (unsigned short)t >> 8 & 0xFF)));
            duel_0884ACD8(a->player, buf);
            duel_088359E8(a->player, a->id, duel_0884E3E4);
            duel_w_08914908++;
            return 0;
        case 3:
            if (duel_08835AC0() != 0) {
                duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
                return 1;
            }
            duel_w_08914908--;
            return 0;
        }
        break;
    case 2:
        return duel_088AA1C0(a, b);
    }
    return 1;
}
#endif

int duel_088AE548(DuelAct *a)
{
    unsigned int p = a->player;

    switch (duel_w_08914908) {
    case 0:
        if (duel_088BFFD8(p, a->id) == 0) {
            return -1;
        }
        duel_w_08914908++;
        return 0;
    case 1:
        duel_088329D0(1 - p, a->id);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_088A8EA0(a, p, 0xE, duel_h_0891968E);
        break;
    }
    return 1;
}

int duel_088AE600(DuelAct *a, unsigned int p, int z)
{
    if (duel_0884E3E4(a, p, z) == 0) {
        return 0;
    }
    switch (a->n7) {
    case 2:
        return a->player == p;
    case 1:
        if ((unsigned short)((unsigned char)p | (unsigned char)z << 8) == duel_act_target(a, 0)) {
            return 0;
        }
    case 0:
        if (a->player != p) {
            return 1;
        }
        break;
    }
    return 0;
}

int duel_088AE6CC(DuelAct *a)
{
    switch (duel_w_08914908) {
    case 0:
        duel_0884ACD8(a->player, (void *)100);
        duel_088359E8(a->player, a->id, duel_088AE600);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_088359E8(a->player, a->id, duel_088AE600);
        duel_w_08914908++;
        return 0;
    case 4:
        duel_0884ACD8(a->player, (void *)0x7C);
        duel_088359E8(a->player, a->id, duel_088AE600);
        duel_w_08914908++;
        return 0;
    case 1:
    case 3:
    case 5:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            duel_w_08914908++;
            return 0;
        }
        duel_w_08914908--;
        return 0;
    }
    return 1;
}

int duel_088AE840(DuelAct *a, int b)
{
    if (a->w[2] != 0) {
        return duel_088AA1C0(a, b);
    }
    return 1;
}

int duel_088AE868(DuelAct *a)
{
    DuelSrc *s = &a->src;
    unsigned int t;

    t = duel_card_locate(s->uid);
    if (((unsigned short)t >> 8 & 0xFF) == 0xE) {
        duel_088A8EA0(a, t & 0xFF, 0xE, s->uid);
    }
    return 1;
}

int duel_088AE8CC(DuelAct *a)
{
    unsigned int n;

    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        duel_08832988(a->player, a->id, 1, 0xFF);
        duel_w_08914908++;
        return 0;
    case 1:
        n = duel_h_0891968E;
        duel_088CC178(a->player, a->zone, n);
        duel_088A8B94(a, n);
        duel_08832960(1 - a->player, 0x171F, n);
        duel_w_08914908++;
        return 0;
    }
    return 1;
}

int duel_088AE9A8(DuelAct *a)
{
    unsigned int t;
    unsigned int tz;
    int z;
    int n;
    int id;

    if (duel_w_08914908 != 10) {
        if (duel_088AA5F4(a) != 0) {
            duel_w_08914908 = 10;
        }
        return 0;
    }
    t = duel_act_target(a, 0);
    tz = (unsigned short)t >> 8 & 0xFF;
    n = 0;
    for (z = 0; z <= 4; z++) {
        id = duel.players[a->player & 1].field[z].card.id;
        if (z != tz && id != 0 && card_is_attribute9(id) == 0 && duel_field_e5ae0(a->player, z) != 0 &&
            duel_act_immune(a, a->player, z) == 0) {
            duel_088D1AC8(a->player, z);
            n++;
        }
    }
    duel_088CC178(a->player, a->zone, n);
    return 1;
}

int duel_088AEAFC(DuelAct *a)
{
    duel_088C44E4(a->player, a->id);
    return 1;
}

int duel_0884E5E0(DuelAct *a);
int duel_088AA844(DuelAct *a, unsigned int p, int z);
int duel_0884E690(DuelAct *a, int (*fn)(DuelAct *a, unsigned int p, int z));
int duel_0884E948(DuelAct *a, unsigned int p);
void duel_0884AD18(int msg);
int duel_zone_free(unsigned int p, int z);
void duel_08835A54(unsigned int p, unsigned short id, int (*fn)(DuelAct *a, unsigned int p, int z));
void duel_088CB938(unsigned int p, int a);
void duel_088329AC(unsigned int p, int a);
extern unsigned short duel_w_0891968C;
int duel_0884EB90(DuelAct *a, int b);
int duel_pick_st_zone(unsigned int p);
int duel_0885FB0C(DuelAct *a, unsigned int p, int z);
int duel_088601AC(DuelAct *a, unsigned int p, int z);
unsigned int card_get_type(int id);
int duel_0886CE90(DuelAct *a, int b);
int duel_088AA488(DuelAct *a, int b);

int duel_088AEB24(DuelAct *a)
{
    int n;
    int m1;
    int m2;

    switch (a->id) {
    case 0x17E2:
        n = 2;
        m1 = 0x8D;
        m2 = 0x21;
        break;
    case 0x17EC:
        n = 3;
        m1 = 0x4D;
        m2 = 0x75;
        break;
    }
    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        if (duel_0884E5E0(a) == 0) {
            return -1;
        }
        duel_0884ACD8(a->player, (void *)m1);
        duel_w_08919624 = 1;
        duel_w_08914908++;
        return 0;
    case 1:
    case 4:
        if (duel_w_08919624 == 0) {
            return 1;
        }
        duel_088359E8(a->player, a->id, duel_088AA844);
        duel_w_08914908++;
        return 0;
    case 2:
    case 5:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            if (duel_0884E690(a, duel_088AA844) == 0) {
                return 1;
            }
            if (a->n7 >= n) {
                return 1;
            }
            duel_w_08914908 = 3;
            return 0;
        }
        duel_w_08914908 -= 2;
        return 0;
    case 3:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = a->n7 < duel_0884E948(a, 1 - a->player);
        } else {
            duel_0884AD18(m2);
        }
        duel_w_08914908++;
        return 0;
    }
    return 0;
}

int duel_088AED54(DuelAct *a, int p, int z)
{
    int i;

    if (p == a->player) {
        return 0;
    }
    if (z > 4) {
        return 0;
    }
    if (duel_zone_free(p, z) == 0) {
        return 0;
    }
    for (i = 0; i < a->n7; i++) {
        if (a->w[i] == (unsigned short)((unsigned char)p | (unsigned char)z << 8)) {
            return 0;
        }
    }
    return 1;
}

#ifdef NON_MATCHING /* o original assume que duel_088A8B94 (static na mesma unidade) preserva $a0; bate com ela static no arquivo */
int duel_088AEE0C(DuelAct *a)
{
    unsigned int p;
    int z;

    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        if (duel_0884E690(a, (void *)duel_088AED54) == 0) {
            return -1;
        }
        duel_0884ACD8(a->player, (void *)0x9B);
        duel_08835A54(a->player, a->id, (void *)duel_088AED54);
        duel_w_08914908++;
        return 0;
    case 2:
    case 4:
        duel_088359E8(a->player, a->id, (void *)duel_088AED54);
        duel_w_08914908++;
        return 0;
    case 1:
    case 3:
    case 5:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_088CB938(p, z);
            duel_088A8B94(a, (unsigned char)p | (unsigned char)z << 8);
            if (duel_0884E690(a, (void *)duel_088AED54) == 0) {
                return 1;
            }
            duel_w_08914908++;
            return 0;
        }
        if (a->n7 != 0) {
            return 1;
        }
        duel_w_08914908--;
        return 0;
    }
    return 1;
}
#endif

#ifdef NON_MATCHING /* o original assume que duel_088A8B94 (static na mesma unidade) preserva $a0; bate com ela static no arquivo */
int duel_088AEFB4(DuelAct *a)
{
    switch (duel_w_08914908) {
    case 0:
        duel_08832988(a->player, a->id, 1, 0xFF);
        duel_w_08914908++;
        return 0;
    case 1:
        duel_088A8B94(a, duel_h_0891968E);
        duel_08832960(1 - a->player, 0x171F, duel_h_0891968E);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_088329AC(a->player, a->id);
        duel_w_08914908++;
        return 0;
    case 3:
        duel_088A8B94(a, duel_w_0891968C);
        duel_08832960(1 - a->player, 0x1357, duel_w_0891968C);
        duel_w_08914908++;
        return 0;
    }
    return 1;
}
#endif

void duel_088AF0DC(DuelAct *a, DuelAct *b)
{
    a->id = b->id;
    duel_0884EB90(a, (int)b);
    a->id = 0x183E;
}

int duel_088AF110(DuelAct *a, int b)
{
    if (duel_pick_st_zone(a->player) == -1) {
        return -1;
    }
    if (duel_0884E5E0(a) != 0) {
        return duel_088AA1C0(a, b);
    }
    return -1;
}

int duel_088AF180(DuelAct *a)
{
    if (a->kind == 2) {
        return 1;
    }
    return duel_088AA5F4(a);
}

int duel_088AF1BC(DuelAct *a)
{
    switch (duel_w_08914908) {
    case 0:
        duel_0884ACD8(a->player, (void *)0x7C);
        duel_088359E8(a->player, a->id, duel_088601AC);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_0884ACD8(a->player, (void *)100);
        duel_088359E8(a->player, a->id, duel_0885FB0C);
        duel_w_08914908++;
        return 0;
    case 1:
    case 3:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            duel_w_08914908++;
            return 0;
        }
        duel_w_08914908--;
        return 0;
    }
    return 1;
}

int duel_088AF2FC(DuelAct *a, int b)
{
    if (card_get_type(a->idx) == 0x16) {
        return 1;
    }
    if (card_get_type(a->idx) == 0x17) {
        return duel_088AA1C0(a, b);
    }
    if (duel_0886CE90(a, b) != 0) {
        return duel_088AA488(a, b);
    }
    return 1;
}
