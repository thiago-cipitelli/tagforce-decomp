// @range 0x08832B54-0x08834734
// @asm 0x0883449C-0x08834614
// @data 0x08907A18-0x08907A30
/* Picking a card from the hand for an effect: the filter of the hand cards (flags of the pending
   event) and the step machines that let the player or the computer choose. */
#include "duel.h"

extern unsigned int duel_w_08919670;   /* player of the event */
extern int duel_ev_unk;                /* 0x08919678 step */
extern unsigned short duel_ev_data[8]; /* 0x0891967C */
extern unsigned short duel_w_0891968C;
extern unsigned short duel_h_0891968E; /* cards still to pick */
extern unsigned short duel_h_08919690; /* index picked */
extern int duel_w_08919654;
extern int duel_player_type[2];
extern unsigned int duel_turn_player;
extern unsigned int duel_turn;
extern unsigned int duel_view_player;
extern int duel_w_08919600;            /* step of duel_08833DC0 */
extern int duel_w_089195E8;
extern int duel_w_089195D8;
extern int duel_w_089195E4;
extern int duel_w_08917DC4;
extern int duel_step;
extern int duel_w_089148FC;
extern int duel_w_08914900;
extern int duel_w_08914904;
extern int duel_w_08914908;
extern int duel_w_08914924;
extern int duel_w_0891490C;
extern DuelAct *duel_w_089147DC;
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern unsigned int duel_ai_player;
extern int duel_w_08919628;
extern int duel_w_0891962C;
extern int duel_w_08919638;
extern unsigned char duel_b_0891434D;  /* steps of the chain messages */
extern unsigned char duel_b_0891434E;
extern unsigned char duel_b_0891434F;

typedef struct DuelFlags8 {
    unsigned char b0 : 1;
    unsigned char b1 : 1;
    unsigned char b2 : 1;
    unsigned char b3 : 1;
    unsigned char b4 : 1;
    unsigned char b5 : 1;
    unsigned char b6 : 1;
    unsigned char b7 : 1;
} DuelFlags8;

extern DuelFlags8 duel_ev_flags;       /* 0x08914341 */
extern DuelFlags8 duel_msg_state;      /* 0x08914340 low byte */

int card_is_monster(int id);
int card_get_type(int id);
int card_is_attribute7(int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_088DFCC8(unsigned int uid);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_0884B064(int msg);
void duel_0881956C(unsigned int p, int id, int (*fn)());
int duel_host_call(int a, int b, int c, int d);
void duel_088359B0(int (*fn)());
int duel_08835AC0(void);
void duel_088CAE34(unsigned int p, int a);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
void duel_088CAB5C(unsigned int p, int i);
void duel_088CABA8(unsigned int p, int i);
void duel_088CAB04(unsigned int p, int i, int b);
void duel_088CACF0(unsigned int p, DuelCard *c, int a);
void duel_088CB95C(unsigned int p, int z, int uid);
int duel_rand(int n);
int card_same_name(int id, int id2);
void duel_088C7810(unsigned int p, unsigned int on);
int duel_0880ADF4(unsigned int p, int (*fn)(unsigned int p, int z, int i));
void duel_0884ACD8();
void duel_08835978(int (*fn)(unsigned int p, int z, int i));
int duel_088BFFD8(unsigned int p, int id, int i);
unsigned short *card_get_name(int id);
void duel_wfmt_s(unsigned short *dst, int fmt, unsigned short *arg);
int field_get_card(unsigned int p, int z);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_0884CEBC(unsigned int p);
int duel_0884EB10(DuelAct *a, DuelAct *b);
int duel_0884EB90(DuelAct *a, DuelAct *b);
int duel_0884EC10(DuelAct *a, DuelAct *b);
int duel_088B1134(void);
void duel_msg_send(unsigned short cmd, int arg, void *data, int len);
void duel_088C9760(unsigned int p, int z);
void duel_0884F398(unsigned int code, int b);
void duel_0884F654(unsigned int code, int b, int c);
void duel_088C1618(unsigned int p);
void duel_08826000(unsigned int p, int z);
void duel_08834734(int a, int b, int c);
void duel_cmd(unsigned short cmd, int p, unsigned short id, int arg);

int duel_08832B54(unsigned int p, int z, int i)
{
    unsigned short f;
    int f4, f8, f10, f20;
    int id;

    if (p == duel_w_08919670 && z == 0xB) {
    f = duel_ev_data[1];
    f4 = f & 4;
    f8 = f & 8;
    f10 = f & 0x10;
    f20 = f & 0x20;
    id = duel.players[p & 1].hand[i].id;
    if ((f & 1) && !card_is_monster(id)) {
        return 0;
    }
    if (f8 && card_get_type(id) != 0x16) {
        return 0;
    }
    if (f4 && card_get_type(id) != 0x17) {
        return 0;
    }
    if (duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].hand[i]))) {
        return 0;
    }
    if (f10 && f20 && !duel_088DFCC8(CARD_UID(duel.players[p & 1].hand[i]))) {
        return 0;
    }
    return 0x800;
    }
    return 0;
}

int duel_08832D08(void)
{
    return duel_h_0891968E;
}

int duel_08832D14(unsigned int p, int n, unsigned int f)
{
    int f1 = f & 1;
    int f8 = f & 8;
    int f2 = f & 2;
    int f10 = f & 0x10;
    int f20 = f & 0x20;
    int f80 = f & 0x80;
    unsigned int i;
    int found = 0;
    int k;

    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        if (duel_08832B54(p, 0xB, i)) {
            found = 1;
            break;
        }
    }
    switch (duel_ev_unk) {
    case 0:
        if (!found) {
            return 1;
        }
        duel_088C7BA4();
        duel_h_0891968E = n;
        duel_ev_unk++;
    case 1:
        if (duel_player_type[p] == 0) {
            if (f20) {
                duel_0884B064(0x3D);
            } else if (f8) {
                duel_0884B064(0x14);
            } else if (f1) {
                duel_0884B064(0x29);
            } else {
                duel_0884B064(0xEB);
            }
        }
        duel_ev_unk++;
        return 0;
    case 2:
        if (duel_h_0891968E == 0 || !found) {
            duel_088C7CF0();
            return 1;
        }
        if (duel_player_type[p] == 1) {
            duel_0881956C(p, (!f10 && !f80) + 2, duel_08832B54);
        } else {
            duel_host_call(7, p, 0xB, MIN(duel.players[p & 1].n_hand - 1, duel_w_08919654 - 1));
            duel_088359B0(duel_08832B54);
        }
        duel_ev_unk++;
        return 0;
    case 3:
        if (duel_08835AC0()) {
            duel_h_08919690 = duel_w_08919654;
            duel_ev_unk++;
            return 0;
        }
        duel_ev_unk -= 2;
        return 0;
    case 4:
        if (f20) {
            k = duel_h_08919690;
            if (f10) {
                duel_088CAE34(p, k);
            } else {
                duel_088CAE24(p, &duel.players[p & 1].hand[k], f2 != 0);
            }
        } else {
            k = duel_h_08919690;
            if (f80) {
                duel_088CAB5C(p, k);
            } else if (f10) {
                duel_088CABA8(p, k);
            } else {
                duel_088CAB04(p, k, f2);
            }
        }
        duel_h_0891968E--;
        duel_ev_unk -= 2;
        return 0;
    }
    return 0;
}

int duel_088330E8(unsigned int p, int b, int n)
{
    int i;

    switch (duel_ev_unk) {
    case 0:
        duel_h_0891968E = n;
        if (n == 0 || duel.players[p & 1].n_hand == 0) {
            return 1;
        }
        duel_088C7BA4();
        duel_ev_unk++;
        return 0;
    case 1:
        if (duel_h_0891968E != 0 && duel.players[p & 1].n_hand != 0) {
            i = duel_rand(duel.players[p & 1].n_hand);
            duel_cmd((p ? 0x8000 : 0) | 0x53, 0, 0, 0);
            duel_088CB95C(p, 0xB, CARD_UID(duel.players[p & 1].hand[i]));
            duel_088CAB04(p, i, b);
            duel_w_0891968C = CARD_UID(duel.players[p & 1].hand[i]);
            duel_h_0891968E--;
            return 0;
        }
        duel_ev_unk++;
        return 0;
    case 2:
        duel_088C7CF0();
        return 1;
    }
    return 1;
}

int duel_088332E0(unsigned int p, int b, int n)
{
    int z = (n != 0) ^ 1;
    int i;

    switch (duel_ev_unk) {
    case 0:
        if (duel.players[p & 1].n_hand == 0) {
            return 1;
        }
        duel_h_0891968E = n + z;
        if (!z) {
            duel_088C7BA4();
        }
        duel_ev_unk++;
        return 0;
    case 1:
        if (duel_h_0891968E != 0 && duel.players[p & 1].n_hand != 0) {
            i = duel_rand(duel.players[p & 1].n_hand);
            duel_cmd((p ? 0x8000 : 0) | 0x53, 0, 0, 0);
            duel_088CB95C(p, 0xB, CARD_UID(duel.players[p & 1].hand[i]));
            if (z) {
                duel_w_0891968C = i;
                duel_ev_unk++;
                return 0;
            }
            duel_088CACF0(p, &duel.players[p & 1].hand[i], b != 0);
            duel_w_0891968C = CARD_UID(duel.players[p & 1].hand[i]);
            duel_h_0891968E--;
            return 0;
        }
        duel_ev_unk++;
        return 0;
    case 2:
        if (!z) {
            duel_088C7CF0();
        }
        return 1;
    }
    return 1;
}

int duel_08833520(unsigned int p, int z, int i)
{
    if (p != duel_w_08919670 && z == 0xB) {
        return 0x800;
    }
    return 0;
}

int duel_08833550(unsigned int p, int z, int i)
{
    if (p != duel_w_08919670 && z == 0xB && card_is_attribute7(duel.players[p & 1].hand[i].id)) {
        return 0x800;
    }
    return 0;
}

int duel_088335C4(unsigned int p, int z, int i)
{
    if (p != duel_w_08919670 && z == 0xB && card_is_monster(duel.players[p & 1].hand[i].id)) {
        return 0x800;
    }
    return 0;
}

int duel_08833638(unsigned int p, int id, int a)
{
    int (*fn)(unsigned int p, int z, int i) = duel_08833520;

    switch (id) {
    case 0x1509:
        fn = duel_08833550;
        break;
    case 0x14AB:
    case 0x1546:
    case 0x1A70:
        fn = duel_088335C4;
        break;
    }
    switch (duel_ev_unk) {
    case 0:
        if (duel.players[(1 - p) & 1].n_hand == 0) {
            duel_w_0891968C = 0xFFFF;
        }
        duel_0884ACD8(p, 0xF4);
        duel_ev_unk++;
        return 0;
    case 1:
        if (a) {
            duel_088C7810(1 - p, 1);
        }
        duel_ev_unk++;
        return 0;
    case 2:
        if (duel_player_type[p] == 1) {
            duel_cmd((1 - p ? 0x8000 : 0) | 0x53, 0, 0, 0);
            if (a) {
                duel_w_0891968C = duel_0880ADF4(p, fn);
            } else {
                duel_w_0891968C = duel_rand(duel.players[(1 - p) & 1].n_hand);
            }
            duel_ev_unk = 4;
            return 0;
        }
        duel_08835978(fn);
        duel_ev_unk++;
        return 0;
    case 3:
        if (duel_08835AC0()) {
            duel_w_0891968C = duel_w_08919654;
            duel_ev_unk++;
            return 0;
        }
        duel_ev_unk = 0;
        return 0;
    case 4:
        duel_088CB95C(1 - p, 0xB, CARD_UID(duel.players[(1 - p) & 1].hand[duel_w_0891968C]));
        duel_ev_unk++;
        return 0;
    case 5:
        if (a) {
            duel_088C7810(1 - p, 0);
        }
        duel_ev_unk++;
        return 0;
    }
    return 1;
}

int duel_08833944(unsigned int p, int z, int i)
{
    int r;
    int id;
    int k;

    if (p == duel_w_08919670 && z == 0xB) {
        k = duel_h_08919690;
        id = duel.players[p & 1].hand[i].id;
        r = 1;
        switch (k) {
        case 0x137C:
        case 0x135B:
        case 0x1579:
        case 0x17C3:
            r = duel_088BFFD8(p, k, i);
            break;
        case 0x16D3:
            r = card_same_name(id, duel_ev_data[1]);
            break;
        case 0x15DC:
        case 0x1888:
            r = !duel_slot_effect_with(p, 0xB, 0x12A1, CARD_UID(duel.players[p & 1].hand[i]));
            break;
        case 0x1A19:
            r = card_is_monster(id);
            break;
        }
        if (r) {
            return 0x800;
        }
    }
    return 0;
}

int duel_08833A88(unsigned int p, int id)
{
    int name = duel_ev_data[1];
    unsigned int i;
    int found;
    unsigned short buf[256];

    switch (duel_ev_unk) {
    case 0:
        duel_w_0891968C = 0;
        duel_h_0891968E = 0;
        duel_h_08919690 = id;
        found = 0;
        for (i = 0; i < duel.players[p & 1].n_hand; i++) {
            if (duel_08833944(p, 0xB, i)) {
                found = 1;
                break;
            }
        }
        if (!found) {
            return 1;
        }
        switch (id) {
        case 0x132B:
            duel_0884ACD8(p, 0x1F);
            break;
        case 0x135B:
            break;
        case 0x137C:
            duel_0884ACD8(p, 0x2E);
            break;
        case 0x1579:
        case 0x17C3:
            duel_0884ACD8(p, 0x11E);
            break;
        case 0x16D3:
            duel_wfmt_s(buf, 0x60, card_get_name(name));
            duel_0884ACD8(p, buf);
            break;
        case 0x15DC:
        case 0x1888:
            duel_0884ACD8(p, 0x1F);
            break;
        case 0x15E8:
            duel_0884ACD8(p, 0xF3);
            break;
        case 0x1A19:
            duel_0884ACD8(p, 0x46);
            break;
        }
        if (duel_player_type[p] == 1) {
            duel_0881956C(p, id, duel_08833944);
        } else {
            duel_08835978(duel_08833944);
        }
        duel_ev_unk++;
        return 0;
    case 1:
        if (duel_08835AC0()) {
            duel_w_0891968C = duel_w_08919654;
            duel_h_0891968E = CARD_UID(duel.players[p & 1].hand[duel_w_08919654]);
            duel_ev_unk++;
            return 0;
        }
        duel_ev_unk--;
        return 0;
    case 2:
        duel_088CB95C(p, 0xB, duel_h_0891968E);
        duel_ev_unk++;
        return 0;
    }
    return 1;
}

int duel_08833DC0(void)
{
    unsigned int tp = duel_turn_player;
    int z;
    unsigned int q;
    unsigned int w;
    int r;

    switch (duel_w_08919600) {
    case 0:
        duel_cmd((tp ? 0x8000 : 0) | 3, 0, 0, 0);
        duel_w_08919600++;
        return 0;
    case 1:
        q = 1 - tp;
        if (duel.players[q & 1].n_hand == 0) {
            for (z = 0; z <= 4; z++) {
                if (duel.players[q & 1].field[z].faceup) {
                    switch (field_get_card(q, z)) {
                    case 0x1740:
                        duel_088C87A8(q, z, 0x1740, 1, 0);
                        break;
                    }
                }
            }
        }
        duel_w_08919600++;
    case 2:
        if (duel_w_089195E8 == 3) {
            duel_0884CEBC(tp);
        } else if (duel_turn == duel_w_089195D8 + 3 && duel.players[tp & 1].lp != duel.players[(1 - tp) & 1].lp) {
            w = duel.players[1].lp > duel.players[0].lp;
            r = w == duel_view_player ? 1 : 2;
            duel_w_089195E4 = r;
            duel.players[w & 1].unk20[3] = 9;
            duel_w_08917DC4 = 1;
        } else {
            duel_cmd((tp ? 0x8000 : 0) | 4, 0, 0, 0);
        }
        duel_w_08919600++;
        return 0;
    }
    return 0;
}

int duel_08834020(void)
{
    DuelAct *a = &duel_chain.acts[duel_chain.len];
    DuelAct *prev;
    int r;

    if (duel_chain.len > 0) {
        prev = a - 1;
    } else {
        prev = 0;
    }
    switch (duel_b_0891434D) {
    case 0:
        duel_w_08914904 = 0;
        duel_w_0891490C = 0;
        duel_b_0891434D++;
    case 1:
        duel_w_089147DC = a;
        r = duel_0884EB10(a, prev);
        if (r != 0) {
            a->b15 = r > 0;
            duel_b_0891434D++;
        }
        return 0;
    case 2:
        duel_msg_send(0x1D, -1, a, 0x18);
        return 1;
    }
    return 1;
}

int duel_08834120(void)
{
    DuelAct *a = &duel_chain.acts[duel_chain.len];
    DuelAct *prev;
    int r;

    if (duel_chain.len > 0) {
        prev = a - 1;
    } else {
        prev = 0;
    }
    if (prev != 0 && prev->id == 0) {
        prev = 0;
    }
    switch (duel_b_0891434E) {
    case 0:
        duel_w_08914908 = 0;
        duel_w_0891490C = 0;
        duel_b_0891434E++;
    case 1:
        duel_w_089147DC = a;
        r = duel_0884EB90(a, prev);
        if (r != 0) {
            a->b4_0 = r > 0;
            duel_b_0891434E++;
        }
        return 0;
    case 2:
        duel_msg_send(0x1B, -1, a, 0x18);
        return 1;
    }
    return 1;
}

int duel_08834234(void)
{
    DuelAct *a = &duel_chain.acts[duel_chain.len];
    DuelAct *prev;

    if (duel_chain.len > 0) {
        prev = a - 1;
    } else {
        prev = 0;
    }
    switch (duel_b_0891434F) {
    case 0:
        duel_step = 0x80;
        duel_w_089148FC = 0;
        duel_w_08914900 = 0;
        duel_b_0891434F++;
    case 1:
        duel_w_089147DC = a;
        if ((duel_step = duel_0884EC10(a, prev)) == 0) {
            duel_b_0891434F++;
        }
        return 0;
    case 2:
        duel_msg_send(0x19, -1, prev ? prev : a, prev ? 0x30 : 0x18);
        return 1;
    }
    return 1;
}

int duel_0883432C(void)
{
    if (duel_ev_flags.b4) {
        if (duel_08834020()) {
            duel_ev_flags.b4 = 0;
        }
        return 1;
    }
    if (duel_ev_flags.b2) {
        if (duel_08834120()) {
            duel_ev_flags.b2 = 0;
        }
        return 1;
    }
    if (duel_msg_state.b4) {
        if (duel_088B1134()) {
            duel_msg_state.b4 = 0;
        }
        return 1;
    }
    if (duel_ev_flags.b6) {
        if (duel_08834234()) {
            duel_ev_flags.b6 = 0;
        }
        return 1;
    }
    return 0;
}

int duel_0883447C(void)
{
    duel_0883432C();
    return 0;
}

#ifdef NON_MATCHING /* agendamento do valor de bits do duel_0884F398 (p != duel_ai_player) e da soma z não reproduzido */
void duel_0883449C(int a)
{
    unsigned int p = duel_w_0891964C;
    int z = duel_w_08919650 + duel_w_08919654;
    unsigned short *f = (unsigned short *)((char *)&duel.players[0].field[0].faceup + (p & 1) * sizeof(DuelPlayer) + z * sizeof(DuelSlot));
    unsigned short v = *f;
    int k;

    k = 3;
    if (v == 0) {
        k = 0;
        duel_088C9760(p, z);
        *f = 1;
    }
    if (a != 0) {
        duel_0884F654((unsigned short)duel_w_08919628 | ((p << 31 & 0x80000000) | (k << 21 & 0xE00000) | (z << 16 & 0x1F0000)),
                      (unsigned short)duel_w_0891962C, a);
        *f = v;
    } else {
        duel_0884F398((unsigned short)duel_w_08919628 | ((p << 31 & 0x80000000) | ((p != duel_ai_player) << 24 & 0x1000000) | (k << 21 & 0xE00000) | (z << 16 & 0x1F0000)),
                      (unsigned short)duel_w_0891962C);
        *f = v;
    }
    duel_w_08919638 = 0;
}
#endif

void duel_08834614(int a)
{
    switch (duel_w_08919628) {
    case 0xFEE:
        duel_08834734(1, 0, 0);
        break;
    default:
        duel_w_08914924 = 1;
        duel_088C1618(duel_w_0891964C);
        if (a != 0) {
            duel_0884F654((duel_w_0891964C << 31 & 0x80000000) | 0x4E0000 | (unsigned short)duel_w_08919628, (unsigned short)duel_w_0891962C, a);
        } else {
            duel_0884F398((duel_w_0891964C << 31 & 0x80000000) | 0x4E0000 | (unsigned short)duel_w_08919628, (unsigned short)duel_w_0891962C);
        }
        duel_w_08919638 = 0;
        break;
    }
}

void duel_088346F8(void)
{
    duel_08826000(duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
    duel_w_08919638 = 0;
}
