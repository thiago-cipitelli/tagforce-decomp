// @range 0x08865F2C-0x08867F54
// @asm 0x08866608-0x088667B4
// @asm 0x088674DC-0x08867704
// @data 0x089116FC-0x08911760
/* Card effect handlers (DuelAct *a): step machines on duel_w_08914904 that ask the player (or the
   computer) for a card and apply the result. */
#include "duel.h"

void duel_088328B8(unsigned int p, unsigned short a, int b);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
void duel_088D1AC8(unsigned int p, int z);
void duel_08832974(unsigned int p, unsigned short id, int a);
void duel_0884ACD8(unsigned int p, int msg);
void duel_0881956C(unsigned int p, unsigned short id, int (*fn)());
void duel_08835978(int (*fn)());
int duel_088619C4(DuelAct *a, int b);
int duel_list_card_level(unsigned int p, int i);
void duel_08832B40(unsigned int p, int a, int b);
void duel_088C4564(unsigned int p, int n);
void duel_088CC178(unsigned int p, int z, unsigned int v);
unsigned int duel_card_locate(unsigned int uid);
void duel_088D2468(int same);
int duel_0887FCA8(int id);
void duel_088CABA8(unsigned int p, int i);
int duel_08835AC0(void);
int card_get_level(int id);
int duel_slot_efb84(unsigned int p, int z);
void duel_088D1D64(DuelAct *a, unsigned int mask);
int duel_08877C00();
int duel_088619FC(DuelAct *a, int b);
int duel_grave_find_uid(unsigned int p, int uid);
void duel_088CB400(unsigned int p, DuelCard *c, int a);
int duel_088BFFD8(unsigned int p, int id, int i);
void duel_0884AD40(int msg, int uid);
void duel_088D1CF4(DuelAct *a, unsigned int p, int z);
void duel_08832950(unsigned int p, unsigned short id);
int card_get_atk_q(int id);
int card_is_monster(int id);
int duel_0884E948(DuelAct *a, unsigned int p);
void duel_088CAE34(unsigned int p, int a);
int card_get_atk(int id);
int duel_08866608(DuelAct *a);
int duel_0886CE38();
int duel_slot_value_1909(unsigned int p, int z);
void duel_088359E8(unsigned int p, int id, int (*fn)());
int duel_088537B0(int id);
int duel_host_call(int a, int b, int c, int d);
int duel_0884E5E0(DuelAct *a);
void duel_0884AD18(int msg);
void duel_088D1B20(unsigned int p, unsigned int mask);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_08835A54(unsigned int p, unsigned short id, int (*fn)(DuelAct *a, unsigned int p, int z));
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
int duel_slot_e68a0(unsigned int p, int z);
int duel_0884AFD8(int a, unsigned short b, unsigned int c, unsigned int d);
void duel_088CC284(unsigned int p, int z, int n);
void duel_0884C588(unsigned int p, int a, int id, int b);
DuelCard *duel_0884C7CC(void);
int duel_088A6300(int id);
int duel_088A6360(int id, int i);
int duel_088A62AC(int k, unsigned int p, int z);
int duel_08856860(DuelAct *a, unsigned int p, int z);
unsigned int duel_field_attrs(unsigned int p, int z);
int duel_bit_count(unsigned int m);
unsigned short *card_get_name(int id);
void duel_wfmt_s(unsigned short *dst, int fmt, unsigned short *arg);
int duel_088DFE24(unsigned int p, unsigned int q);
int duel_0886CE90(DuelAct *a, int b);
int duel_08861964(DuelAct *a, int b);
int duel_088674DC(DuelAct *a);
void duel_088C6728(unsigned int p, int uid, int a);
void duel_088328FC(unsigned int p);
void duel_088C78E0(unsigned int p);
DuelCard *duel_0884CCBC(int i);
int duel_08860A88(DuelAct *a);
void duel_088C44E4(unsigned int p, int id);
int duel_08861360(DuelAct *a);
void duel_08832788(unsigned int p, int id, int a, int b);
int duel_slot_effect_arg(unsigned int p, int z, int id);
void duel_088C89BC(unsigned int p, int z, int id, int c);

extern int duel_w_08914904;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern unsigned short duel_w_0891968C;
extern DuelAct *duel_w_089147DC;
extern unsigned int duel_atk_tp;
extern int duel_atk_tz;

/* Picks a monster and lets the opponent choose between its ATK and DEF (w[0], w[1] -> w[2]). */
int duel_08865F2C(DuelAct *a)
{
    DuelStats st;

    switch (duel_w_08914904) {
    case 0:
        duel_088328B8(a->player, 0xFFFF, 0);
        duel_w_08914904++;
        return 0;
    case 1:
        duel_field_stats(duel_w_0891968C & 0xFF, duel_w_0891968C >> 8 & 0xFF, &st);
        a->w[0] = st.atk > 0xFFFF ? 0xFFFF : st.atk;
        a->w[1] = st.def > 0xFFFF ? 0xFFFF : st.def;
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = a->w[0] < a->w[1];
        } else {
            duel_0884ADAC(0x81, 0x198, 0x199, 0, 0, 0xF);
        }
        duel_w_08914904++;
        return 0;
    case 2:
        duel_088D1AC8(duel_w_0891968C & 0xFF, duel_w_0891968C >> 8 & 0xFF);
        duel_w_08914904++;
        return 0;
    case 3:
        switch (duel_w_08919624) {
        case 0:
            a->w[2] = a->w[0];
            break;
        case 1:
            a->w[2] = a->w[1];
            break;
        }
        duel_08832974(1 - a->player, a->id, duel_w_08919624);
        duel_w_08914904++;
        return 0;
    }
    return 1;
}

/* Picks a card of the hand (duel_0887FCA8 filters) and records its level in w[2]. */
int duel_08866104(DuelAct *a, int b)
{
    unsigned int p;
    int i;

    switch (duel_w_08914904) {
    case 0:
        duel_0884ACD8(a->player, 0x29);
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, 2, duel_0887FCA8);
        } else {
            duel_08835978(duel_0887FCA8);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            i = duel_w_08919654;
            duel_088CABA8(p, i);
            switch (a->id) {
            case 0x1727:
                duel_088619C4(a, b);
                break;
            case 0x18CB:
                a->w[2] = duel_list_card_level(p, CARD_UID(duel.players[p & 1].hand[i]));
                break;
            case 0x1895:
                duel_088619C4(a, b);
                a->w[2] = card_get_level(duel.players[p & 1].hand[i].id);
                break;
            }
            return 1;
        }
        duel_w_08914904--;
        return 0;
    }
    return 0;
}

/* Pays duel_w_0891968C * 1000 life points and deals the same damage. */
int duel_088662E8(DuelAct *a)
{
    int n;

    switch (duel_w_08914904) {
    case 0:
        duel_08832B40(a->player, a->id, 0);
        duel_w_08914904++;
        return 0;
    case 1:
        n = duel_w_0891968C * 1000;
        duel_088C4564(a->player, n);
        duel_088CC178(a->player, a->zone, n);
        return 1;
    }
    return 0;
}

int duel_088663A4(DuelAct *a)
{
    int loc = duel_card_locate(a->uid);

    if ((unsigned char)((loc & 0xFFFF) >> 8) < 5) {
        if ((loc >> 16 & 0xFFFF) != 0) {
            duel_088D2468(a->player == duel_atk_tp && a->zone == duel_atk_tz);
        } else {
            duel_088D1AC8(a->player, a->zone);
        }
    }
    return 1;
}

/* Marks the face-up 0x17AD-0x17AF spells/traps of the player; w[2] = all of them pass duel_slot_efb84. */
int duel_08866450(DuelAct *a)
{
    int z;
    int ok;
    unsigned int mask;
    unsigned int p;

    if (a->kind != 5) {
        ok = 1;
        mask = 0;
        for (z = 5; z < 10; z++) {
            p = a->player;
            if (duel.players[p & 1].field[z].faceup != 0) {
                switch (duel.players[p & 1].field[z].card.id) {
                case 0x17AF:
                case 0x17AD:
                case 0x17AE:
                    mask |= 1 << (z + p * 16);
                    ok &= duel_slot_efb84(p, z);
                    break;
                }
            }
        }
        duel_088D1D64(a, mask);
        a->w[2] = ok;
    }
    return 1;
}

int duel_0886655C(DuelAct *a, int b)
{
    int r;
    int i;
    unsigned int p;

    if (duel_08877C00() == 0) {
        return -1;
    }
    r = duel_088619FC(a, b);
    if (r == 1) {
        i = duel_grave_find_uid(a->player, a->uid);
        p = a->player;
        duel_088CB400(p, &duel.players[p & 1].grave[i], 0);
    }
    return r;
}

#ifdef NON_MATCHING /* o retorno 1 do passo 1 passa por um bloco de salto (b; lw ra) antes do caso 2 que nenhuma ordem de case/default reproduziu */
int duel_08866608(DuelAct *a)
{
    unsigned int p;

    switch (duel_w_08914904) {
    case 0:
        p = a->player;
        if (a->uid != CARD_UID(duel.players[p & 1].field[a->zone].card)) {
            return -1;
        }
        if (duel_player_type[p] == 1) {
            duel_w_08919624 = duel_088BFFD8(p, a->id, 0) > 0;
        } else if (duel_088BFFD8(p, a->id, 0) == 0) {
            duel_0884AD40(0x142, a->uid);
        } else {
            duel_0884AD40(0xEC, a->uid);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        if (duel_w_08919624 != 0) {
            duel_088D1CF4(a, a->player, a->zone);
            break;
        }
        duel_08832950(1 - a->player, 0x10D6);
        duel_w_08914904++;
        return 0;
    default:
        return 1;
    case 2:
        return -1;
    }
    return 1;
}
#endif

int duel_088667B4(DuelAct *a)
{
    switch (a->kind) {
    case 2:
    case 5:
        return duel_08866608(a);
    }
    return 1;
}

/* Selection filter: monsters of the hand whose ATK the opponent can be targeted with (0x800 = valid). */
int duel_088667FC(unsigned int p, int z, int i)
{
    int id;

    if (p != duel_w_089147DC->player) {
        return 0;
    }
    if (z == 0xB) {
        id = duel.players[p & 1].hand[i].id;
        duel_w_089147DC->w[2] = card_get_atk_q(id);
        if (card_is_monster(id) && duel_w_089147DC->w[2] != 0xFFFF && duel_0884E948(duel_w_089147DC, 1 - p)) {
            return 0x800;
        }
    }
    return 0;
}

/* Picks a card of the hand (duel_088667FC) and records its ATK in w[2]. */
int duel_088668D4(DuelAct *a)
{
    unsigned int p;
    int i;

    switch (duel_w_08914904) {
    case 0:
        duel_0884ACD8(a->player, 0x3A);
        if (duel_player_type[a->player] == 1) {
            duel_0881956C(a->player, 2, duel_088667FC);
        } else {
            duel_08835978(duel_088667FC);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            i = duel_w_08919654;
            duel_088CAE34(p, i);
            a->w[2] = card_get_atk(duel.players[p & 1].hand[i].id);
            return 1;
        }
        duel_w_08914904--;
        return 0;
    }
    return 0;
}

int duel_08866A10(DuelAct *a)
{
    duel_088D1CF4(a, a->player, a->zone);
    return 1;
}

int duel_08866A3C(DuelAct *a, int b)
{
    unsigned int p = a->player ^ a->opp;
    unsigned int i;

    if (duel_0886CE38(a, b) == 0) {
        return -1;
    }
    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        if (!duel_088BFFD8(p, a->id, i)) {
            continue;
        }
        return duel_088619FC(a, b);
    }
    switch (duel_w_08914904) {
    case 0:
        duel_0884ACD8(p, 0x122);
        duel_w_08914904++;
        return 0;
    case 1:
        duel_08832950(1 - p, 0x10D6);
        break;
    }
    return -1;
}

int duel_08866B68(DuelAct *a)
{
    duel_088D1AC8(a->player, a->zone);
    a->w[0] = duel_slot_value_1909(a->player, a->zone);
    return 1;
}

int duel_08866BB8(DuelAct *a)
{
    switch (a->kind) {
    case 2:
        return duel_08866608(a);
    }
    return 1;
}

/* Lets the player pick cards of the field one by one (up to duel_0884E5E0), marking them in w[0]/w[1]. */
int duel_08866BF4(DuelAct *a, int b)
{
    unsigned int p;
    int z;

    switch (duel_w_08914904) {
    case 0:
        a->w[1] = 0;
        a->w[0] = 0;
        a->w[2] = 0;
        duel_w_08914904++;
        return 0;
    case 1:
        duel_0884ACD8(a->player, 0x12);
        duel_w_08919624 = 1;
        duel_w_08914904++;
        return 0;
    case 2:
    case 5:
        if (duel_w_08919624 == 0) {
            duel_w_08914904 = 7;
            return 0;
        }
        duel_088359E8(a->player, 1, duel_088537B0);
        duel_w_08914904++;
        return 0;
    case 3:
    case 6:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            a->w[p] |= 1 << z;
            duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
            a->w[2]++;
            duel_w_08914904++;
            return 0;
        }
        duel_w_08914904 -= 2;
        return 0;
    case 4:
        if (a->w[2] >= duel_0884E5E0(a)) {
            duel_w_08914904 = 7;
            return 0;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 0;
        } else {
            duel_0884AD18(0x138);
        }
        duel_w_08914904++;
        return 0;
    case 7:
        duel_088D1B20(a->player, a->w[0] | a->w[1] << 16);
        duel_088619C4(a, b);
        return 1;
    }
    return 0;
}

int duel_08866E90(DuelAct *a, DuelAct *b)
{
    unsigned int p;

    switch (duel_w_08914904) {
    case 0:
        duel_088C7BA4();
        duel_w_08914904++;
        return 0;
    case 1:
        p = a->player;
        if (duel.players[p & 1].n_hand != 0) {
            duel_088CABA8(p, 0);
            return 0;
        }
        duel_088C7CF0();
        switch (b->id) {
        case 0x14E0:
            a->w[2] = b->w[2];
            break;
        }
        duel_w_08914904++;
        return 0;
    }
    return 1;
}

int duel_08866F7C(DuelAct *a)
{
    unsigned int mask;

    switch (duel_w_08914904) {
    case 0:
        duel_0884ACD8(a->player, 0x12);
        duel_w_08914904++;
        return 0;
    case 1:
        duel_08835A54(a->player, 1, duel_0884E3E4);
        duel_w_08914904++;
        return 0;
    case 2:
        if (duel_08835AC0() != 0) {
            mask = 1 << (duel_w_08919650 + duel_w_08919654 + duel_w_0891964C * 16);
            mask |= 1 << (a->zone + a->player * 16);
            duel_088D1B20(a->player, mask);
            duel_w_08914904++;
            return 0;
        }
        duel_w_08914904--;
        return 0;
    }
    return 1;
}

int duel_088670AC(DuelAct *a)
{
    int i;
    int n;

    switch (duel_w_08914904) {
    case 0:
        n = duel_slot_e68a0(a->player, a->zone);
        for (i = 1; i < n; i++) {
            a->w[2] = i;
            if (duel_0884E5E0(a)) {
                break;
            }
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = n;
        } else {
            duel_0884AFD8(0x13D, 0x13E, i, n);
        }
        duel_w_08914904++;
        return 0;
    case 1:
        a->w[2] = duel_w_08919624;
        duel_088CC284(a->player, a->zone, -duel_w_08919624);
        return 1;
    }
    return 1;
}

int duel_088671D0(DuelAct *a)
{
    int n;
    int k;
    unsigned int p;
    int z;
    int id;
    int bit;

    switch (duel_w_08914904) {
    case 0:
        duel_0884ACD8(a->player, 0x1A);
        duel_w_08914904++;
        return 0;
    case 1:
        duel_0884C588(a->player, 6, a->id, 0);
        duel_088C7BA4();
        duel_w_08914904++;
        return 0;
    case 2:
        a->w[0] = duel_0884C7CC()->id;
        a->w[1] = 0;
        a->w[2] = 0;
        duel_0884ACD8(a->player, 0x12);
        duel_w_08914904++;
        return 0;
    case 3:
        duel_08835A54(a->player, 1, duel_0884E3E4);
        duel_w_08914904++;
        return 0;
    case 4:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            id = a->w[0];
            z = duel_w_08919650 + duel_w_08919654;
            n = duel_088A6300(id);
            for (k = 0; k < n; k++) {
                bit = 1 << k;
                if (a->w[1] & bit) {
                    continue;
                }
                if (duel_088A62AC(duel_088A6360(id, k), p, z)) {
                    a->w[1] |= bit;
                    break;
                }
            }
            duel_088D1AC8(p, z);
            if (++a->w[2] >= n - 1) {
                duel_w_08914904++;
                return 0;
            }
        }
        duel_w_08914904--;
        return 0;
    case 5:
        duel_088D1AC8(a->player, a->zone);
        duel_088C7CF0();
        duel_w_08914904++;
        return 0;
    }
    return 1;
}

/* Selection filter: cards accepted by duel_08856860 and not yet marked in w[2 + p]. */
int duel_08867474(DuelAct *a, unsigned int p, int z)
{
    if (duel_08856860(a, p, z) == 0) {
        return 0;
    }
    return (a->w[p + 2] & 1 << z) == 0;
}

#ifdef NON_MATCHING /* forma igual; p e attrs trocam de registrador $s (s2/s0) com qualquer ordem e tipo das declarações */
/* Picks field cards until their attributes cover 4 (w[0] attribute bits, w[1] multi-attribute cards). */
int duel_088674DC(DuelAct *a)
{
    unsigned int p;
    unsigned int attrs;
    int z;

    switch (duel_w_08914904) {
    case 0:
        a->w[0] = 0;
        a->w[1] = 0;
        a->w[3] = 0;
        a->w[2] = 0;
        duel_0884ACD8(a->player, 0x12);
        duel_w_08914904++;
        return 0;
    case 1:
        duel_088359E8(a->player, a->id, duel_08867474);
        duel_w_08914904++;
        return 0;
    case 2:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            attrs = duel_field_attrs(p, z);
            a->w[p + 2] |= 1 << z;
            duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
            if (duel_bit_count(attrs) > 1) {
                a->w[1]++;
            } else {
                a->w[0] |= attrs;
            }
            if (a->w[1] + duel_bit_count(a->w[0]) >= 4) {
                duel_088D1B20(a->player, a->w[2] | a->w[3] << 16);
                return 1;
            }
            duel_w_08914904--;
            return 0;
        }
        duel_host_call(0x29, 0, 0, 0);
        duel_w_08914904 = 0;
        return 0;
    }
    return 0;
}
#endif

static const int ids_08867704[3] = { 0x189E, 0x18A0, 0x189F };

/* Picks one card for each of the three cards 0x189E, 0x18A0, 0x189F (w[0] zone bits). */
int duel_08867704(DuelAct *a)
{
    unsigned short buf[0x100];

    switch (duel_w_08914904) {
    case 0:
        a->n7 = 0;
        a->w[0] = 0;
        duel_w_08914904++;
    case 1:
        duel_wfmt_s(buf, 0x143, card_get_name(ids_08867704[a->n7]));
        duel_0884ACD8(a->player, (int)buf);
        duel_088359E8(a->player, a->id, duel_0884E3E4);
        duel_w_08914904++;
        return 0;
    case 2:
        if (duel_08835AC0() != 0) {
            a->w[0] |= 1 << (duel_w_08919650 + duel_w_08919654);
            if (++a->n7 < 3) {
                duel_w_08914904--;
                return 0;
            }
            duel_w_08914904++;
            return 0;
        }
        duel_w_08914904 = 0;
        return 0;
    case 3:
        duel_088D1D64(a, a->w[0] << (a->player * 16));
        duel_w_08914904++;
        return 0;
    }
    return 1;
}

int duel_088678AC(DuelAct *a, int b)
{
    unsigned int p;

    switch (duel_w_08914904) {
    case 0:
        p = a->player;
        if (duel_088DFE24(p, p) == 0) {
            return -1;
        }
        if (duel_0886CE90(a, b) == 0) {
            return -1;
        }
        duel_0884ACD8(a->player, 0x56);
        duel_w_08914904++;
        return 0;
    case 1:
        duel_0884C588(a->player, 6, a->id, a->uid);
        duel_w_08914904++;
        return 0;
    case 2:
        duel_088CB400(a->player, duel_0884C7CC(), 0);
        duel_w_08914904++;
        return 0;
    }
    return 1;
}

int duel_088679CC(DuelAct *a, int b)
{
    if (duel_088619C4(a, b) == 0) {
        return 0;
    }
    return duel_08861964(a, b) != 0;
}

int duel_08867A18(DuelAct *a)
{
    switch (duel_w_08914904) {
    case 0:
        if (duel_088BFFD8(a->player, a->id, 0) == 0) {
            duel_0884ACD8(a->player, 0xD);
            duel_w_08914904 = 5;
            return 0;
        }
        duel_0884C588(a->player, 6, a->id, 0);
        duel_w_08914904++;
        return 0;
    case 1:
        duel_088C6728(a->player, duel_0884C7CC()->owner + (duel_0884C7CC()->uid_hi << 1), 0);
        a->w[0] = 500;
        duel_w_08914904 = 10;
        return 0;
    case 5:
        duel_088328FC(1 - a->player);
        a->w[0] = 0;
        duel_w_08914904 = 10;
        return 0;
    case 10:
        duel_088C78E0(a->player);
        break;
    }
    return 1;
}

int duel_08867B74(DuelAct *a, int b)
{
    int r;
    unsigned int i;

    switch (duel_w_08914904) {
    case 0:
    case 1:
    case 2:
        r = duel_088619FC(a, b);
        if (r != 1) {
            return r;
        }
        duel_w_08914904 = 5;
        return 0;
    case 5:
        duel_0884ACD8(a->player, 0x56);
        duel_w_08914904++;
        return 0;
    case 6:
        r = duel_0884E5E0(a);
        duel_0884C588(a->player, MIN(MIN(r, 5), 0x20) + 0x27, a->id, 0);
        duel_w_08914904++;
        return 0;
    case 7:
        duel_088C7BA4();
        for (i = 0; i < duel_w_08919624; i++) {
            duel_088CB400(a->player, duel_0884CCBC(i + 1), 0);
        }
        duel_088C7CF0();
        a->w[2] = duel_w_08919624;
        return 1;
    }
    return 1;
}

int duel_08867CF4(DuelAct *a, int b)
{
    unsigned int p = a->player ^ a->opp;
    int n = duel_08860A88(a);

    if (duel_0886CE38(a, b) == 0) {
        return -1;
    }
    if (duel.players[p & 1].lp <= n) {
        return -1;
    }
    b = duel_088619FC(a, b);
    if (b == 1) {
        if (a->zone != 0xE) {
            duel_088C44E4(a->player, a->id);
        }
        duel_088C4564(p, n);
    }
    return b;
}

int duel_08867DE8(DuelAct *a)
{
    switch (duel_w_08914904) {
    case 0:
        duel_0884ACD8(a->player, 0x3A);
        duel_088359E8(a->player, a->id, duel_0884E3E4);
        duel_w_08914904++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            duel_088D1CF4(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            duel_w_08914904++;
            return 0;
        }
        duel_w_08914904--;
        return 0;
    case 2:
        duel_08832788(a->player, duel_08861360(a), 0, 0);
        return 1;
    }
    return 1;
}

int duel_08867F00(DuelAct *a)
{
    a->idx = duel_slot_effect_arg(a->player, 0xB, a->id);
    duel_088C89BC(a->player, 0xB, a->id, 0);
    return 1;
}
