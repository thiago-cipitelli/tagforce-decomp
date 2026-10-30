// @range 0x08850F44-0x08853340
// @asm 0x08850F44-0x08853340
/* Resolution of the last link of the chain (duel_chain.acts[unk480 - 1]), step duel_w_089148F4:
   announce, costs, targets still legal (and the cards that negate the link because of its target),
   the card's own step machine (duel_0884EC10), then the cards that react to the resolution. */
#include "duel.h"

extern int duel_step;
extern unsigned int duel_turn_player;
extern unsigned int duel_view_player;
extern int duel_w_089195EC;
extern unsigned int duel_w_08914920;
extern int duel_w_08914924;
extern int duel_w_089149D4;
extern int duel_w_089147D8;            /* links to clean up after the chain */
extern DuelAct *duel_w_089147DC;       /* link being resolved */
extern int duel_w_089148F0;
extern int duel_w_089148F4;            /* step */
extern int duel_w_089148FC;            /* target being checked */
extern int duel_w_08914900;
extern int duel_w_0891968C;
extern unsigned int duel_w_08917DB4;

int duel_088C3F80(void);
void duel_088C7E68(unsigned int p, int z, int a, int b);
void duel_088D1BEC(unsigned int p, int z, int a, int b);
int field_get_card(unsigned int p, int z);
void duel_088C89BC(unsigned int p, int z, int id, int c);
int duel_0884D1E0(void);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_0884F038(DuelAct *a);
int duel_0884EC10(DuelAct *a, DuelAct *prev);
int card_is_monster(int id);
void duel_0884DBAC(DuelAct *a, int link);
int duel_0886B1F4(DuelAct *a);
void duel_msg_send(unsigned short cmd, int arg, void *data, int len);
void duel_088C7B90(int n);
void duel_msg_send3(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_0886C448(DuelAct *a);
void duel_088CBAA4(unsigned int p, int z, int uid);
void duel_088A9174(DuelAct *a, int i);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int card_get_type(int id);
int duel_slot_affected(unsigned int p, int z, int id);
unsigned short duel_slot_src_of(unsigned int p, int z, int id, int active);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
void duel_088D1438(unsigned int p, int z, int a, int b);
unsigned int duel_field_type(unsigned int p, int z);
int duel_find_effect_card(unsigned int p, int id, int z);
void duel_088C448C(int p, int a);
int duel_count_monster_active_atk(unsigned int p, int id);
unsigned short duel_slot_src_of_pos(unsigned int p, int z, int id, unsigned int pos);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int duel_field_protected(unsigned int p, int z);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_088C9878(unsigned int p, int z, int x, int y, int id, int opp);
void duel_088C936C(unsigned int p, int a, int b, int id, int opp);
int duel_slot_value_1909(unsigned int p, int z);
void duel_088D15B4(DuelAct *a, unsigned int p, int z);
void duel_08832B18(unsigned int p, int z);
int duel_0884FB1C(int id, int a);
int card_is_in_group_1911(int id);
void duel_088C44E4(unsigned int p, int id);
int duel_count_active(int id);
int duel_0884FBB0(DuelAct *a);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
void duel_088C5424(unsigned int p, int n);
int duel_088C4730(unsigned int p, int n, int f, int id);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
int duel_slot_e68a0(unsigned int p, int z);
int card_get_group_value_1983(int id);
void duel_088CC284(unsigned int p, int z, int n);
int card_get_prop5_hi(int id);
int duel_hand_count_name(unsigned int p, unsigned short id);
int duel_hand_find_name(unsigned int p, int id);

#ifdef NON_MATCHING /* 9 KB escritos a partir do pseudocódigo e conferidos no asm; layout não reproduzido */

#define SLOT(p, z) (&duel.players[(p) & 1].field[z])

/* the link is negated: its card leaves the field */
static void duel_link_negate(DuelAct *a)
{
    a->b4_2 = 1;
    duel_088CBAA4(a->player, a->zone, a->uid);
}

static int duel_link_negated_by(DuelAct *a, unsigned int q, int id)
{
    duel_link_negate(a);
    if (a->zone < 11) {
        duel_088D1438(a->player, a->zone, a->player != q, id);
    }
    duel_w_089148F4++;
    return 1;
}

/* step 4: each target must still be legal; cards on the target can negate the link */
static int duel_link_targets(DuelAct *a)
{
    unsigned int t;
    unsigned int loc;
    unsigned int q;
    int z;
    int id;
    unsigned int s;
    int sz;

    if ((a->b4_1 || a->b4_2) &&
        (a->b3_4 == 2 || a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card))) {
        duel_w_089148F4 = 5;
        return 1;
    }
    for (; duel_w_089148FC < a->n_targets; duel_w_089148FC++, duel_w_08914900 = 0) {
        t = duel_act_target(a, duel_w_089148FC);
        q = t & 0xFF;
        z = (unsigned char)((t & 0xFFFF) >> 8);
        loc = t;
        if (z < 5 && duel_088A9008(a, duel_w_089148FC, &loc)) {
            id = field_get_card(q, z);
            if (!duel_0884E440(a, q, z)) {
                continue;
            }
            if (duel_w_08914900 != 0) {
                /* 0x168E: the controller of the target may negate (once per copy) */
                if (duel_w_08914900 <= duel_find_effect_card(q, 0x168E, -1)) {
                    if (duel_w_0891968C == 0) {
                        duel_08832B18(q, z);
                        return 1;
                    }
                    if (!duel_0884FB1C(0x168E, duel_w_0891968C)) {
                        duel_w_08914900++;
                        duel_w_0891968C = 0;
                        return 1;
                    }
                    return duel_link_negated_by(a, q, id);
                }
                continue;
            }
            if (card_get_type(a->id) == 0x16 && duel_slot_affected(q, z, 0x1660)) {
                s = duel_slot_src_of(q, z, 0x1660, 1);
                sz = (s & 0xFFFF) >> 8;
                duel_088C4448(s & 0xFF, sz, CARD_UID(SLOT(s, sz)->card));
                return duel_link_negated_by(a, q, 0x1660);
            }
            if (card_get_type(a->id) == 0x16 && SLOT(t, z)->faceup != 0 &&
                (unsigned short)duel_field_type(q, z) == 0xF && duel_find_effect_card(q, 0x14CA, -1)) {
                duel_088C448C(q, 0x14CA);
                return duel_link_negated_by(a, q, 0x1660);
            }
            if (!card_is_monster(a->id) && a->n_targets == 1 && SLOT(t, z)->faceup != 0 &&
                (unsigned short)duel_field_type(q, z) == 7 && duel_count_monster_active_atk(q, 0x1A07) &&
                !a->b4_2) {
                duel_link_negate(a);
            }
            if (card_get_type(a->id) == 0x16 && (s = duel_slot_src_of_pos(q, z, 0x184B, 0)) != 0xFFFF) {
                sz = (s & 0xFFFF) >> 8;
                duel_088C4448(s & 0xFF, sz, CARD_UID(SLOT(s, sz)->card));
                duel_link_negate(a);
                if (a->zone < 11) {
                    duel_088C87A8(a->player, a->zone, 0x17C2, 5, 0);
                }
                duel_088C7E68(s & 0xFF, sz, 4, 1);
            }
            switch (id) {
            case 0x168C:
            case 0x168D:
            case 0x168F:
            case 0x1690:
            case 0x1691:
            case 0x1692:
                if (SLOT_ACTIVE(SLOT(t, z)) && a->player != q) {
                    if (duel_w_0891968C == 0) {
                        duel_088C4448(q, z, CARD_UID(SLOT(t, z)->card));
                        duel_08832B18(q, z);
                        return 1;
                    }
                    if (duel_0884FB1C(id, duel_w_0891968C)) {
                        return duel_link_negated_by(a, q, id);
                    }
                    duel_w_0891968C = 0;
                }
                break;
            case 0x1A1D:
                if (a->n_targets == 1 && (!card_is_monster(a->id) || (a->zone > 4 && a->zone < 10)) &&
                    SLOT(t, z)->link == 0x1A2F && SLOT_ACTIVE(SLOT(t, z)) && !a->b4_2) {
                    duel_088C4448(q, z, CARD_UID(SLOT(t, z)->card));
                    duel_link_negate(a);
                }
                break;
            case 0x1812:
                if (a->player != q && card_get_type(a->id) == 0x16 && SLOT_ACTIVE(SLOT(t, z)) && !a->b4_2) {
                    duel_088C4448(q, z, CARD_UID(SLOT(t, z)->card));
                    duel_link_negate(a);
                }
                break;
            case 0x172E:
                if (a->player != q && (!card_is_monster(a->id) || (a->zone > 4 && a->zone < 10)) &&
                    SLOT_ACTIVE(SLOT(t, z)) && !a->b4_2) {
                    duel_088C4448(q, z, CARD_UID(SLOT(t, z)->card));
                    duel_link_negate(a);
                }
                break;
            case 0x1614:
                if (card_is_monster(a->id) && a->n_targets == 1 && (a->zone < 5 || a->zone > 10) &&
                    SLOT_ACTIVE(SLOT(t, z)) && !a->b4_2) {
                    duel_088C4448(q, z, CARD_UID(SLOT(t, z)->card));
                    duel_link_negate(a);
                }
                break;
            case 0x1529:
                if (!duel_slot_value_1909(q, z)) {
                    break;
                }
            case 0x1534: /* the spell is negated and the card resolves its own effect */
                if (!card_is_monster(a->id) && a->b3_4 == 0 && SLOT_ACTIVE(SLOT(t, z))) {
                    DuelAct r = { 0 };

                    r.uid = CARD_UID(SLOT(t, z)->card);
                    r.b3_4 = 1;
                    r.player = t;
                    r.zone = z;
                    r.id = id;
                    duel_088C4448(q, z, r.uid);
                    a->b4_1 = 1;
                    a->b4_2 = 1;
                    a->b4_3 = 1;
                    duel_088CBAA4(a->player, a->zone, a->uid);
                    duel_088D15B4(&r, a->player, a->zone);
                    duel_w_089148F4++;
                    return 1;
                }
                break;
            case 0x14D5:
            case 0x14DA:
                if (card_get_type(a->id) == 0x17 && SLOT_ACTIVE(SLOT(t, z))) {
                    duel_088C4448(q, z, CARD_UID(SLOT(t, z)->card));
                    return duel_link_negated_by(a, q, 0x1660);
                }
                break;
            case 0x14C4:
            case 0x14C7:
                if (card_get_type(a->id) == 0x16 && SLOT_ACTIVE(SLOT(t, z))) {
                    duel_088C4448(q, z, CARD_UID(SLOT(t, z)->card));
                    return duel_link_negated_by(a, q, 0x1660);
                }
                break;
            case 0x129C:
            case 0x18BD: /* a face-down monster targeted by a spell is flipped and the spell negated */
                if (card_get_type(a->id) == 0x16 && a->n_targets == 1 && SLOT(t, z)->faceup == 0 &&
                    !duel_field_protected(q, z) && (SLOT(t, z)->pos == 0 || !duel_slot_has_effect(0, 0xB, 0x12BF))) {
                    if (SLOT(t, z)->pos == 0) {
                        duel_088C9878(q, z, 0, 0, id, 0);
                    }
                    duel_088C936C(q, z, 0, 0, 0);
                    a->b4_1 = 1;
                    a->b4_2 = 1;
                    a->b4_3 = 1;
                    duel_088C4448(q, z, CARD_UID(SLOT(t, z)->card));
                    duel_088CBAA4(a->player, a->zone, a->uid);
                    duel_w_089148F4++;
                    return 1;
                }
                break;
            }
            if (a->player != q && card_is_in_group_1911(id) && SLOT(t, z)->faceup != 0 &&
                duel_find_effect_card(q, 0x168E, -1)) {
                duel_088C44E4(q, id);
                duel_w_08914900++;
                return 1;
            }
        } else if (z == 0xE && !a->b4_2 && duel_count_active(0x17B9)) {
            duel_088C448C(-1, 0x17B9);
            duel_link_negate(a);
        }
    }
    duel_w_089148F4++;
    return 1;
}

/* step 7: cards that react to the resolution of the link */
static void duel_link_resolved(DuelAct *a, DuelAct *prev)
{
    unsigned int i;
    unsigned int q;
    int z;
    int id;
    int k;

    if (a->zone < 11 && a->uid == CARD_UID(duel.players[a->player & 1].field[a->zone].card) && duel_0884FBB0(a)) {
        duel_088C7E68(a->player, a->zone, 2, 1);
    }
    if (!a->b4_1 && a->b3_4 == 0) {
        /* 0x161A: the cards that counted the activation */
        for (i = 0; (int)i < 2; i++) {
            q = duel_turn_player ^ i;
            for (z = 0; z < 11; z++) {
                id = field_get_card(q, z);
                if (id == 0 || !SLOT_ACTIVE(SLOT(q, z)) || !duel_slot_effect_with(q, z, 0x161A, a->uid)) {
                    continue;
                }
                if (card_is_monster(id) ? z >= 5 : z < 5) {
                    continue;
                }
                switch (id) {
                case 0x1A22:
                case 0x1A26:
                case 0x1A37:
                    duel_0884F30C((id & 0xFFFF) | q << 31 | 0x200000 | (z & 0x1F) << 16, CARD_UID(SLOT(q, z)->card), 0);
                    break;
                case 0x16BB:
                    duel_0884F30C((z & 0x1F) << 16 | q << 31 | (q != a->player) << 24 | 0x200000 | 0x16BB,
                                  CARD_UID(SLOT(q, z)->card), 0);
                    break;
                case 0x1982:
                    duel_088C4448(q, z, CARD_UID(SLOT(q, z)->card));
                    duel_088C4730(1 - q, 1000, 1, 0x1982);
                    break;
                case 0x1964:
                    if (q == a->player) {
                        duel_088C4448(q, z, CARD_UID(SLOT(q, z)->card));
                        duel_088C4730(1 - q, 400, 1, 0x1964);
                    }
                    break;
                case 0x171D:
                case 0x184F:
                    duel_088C4448(q, z, CARD_UID(SLOT(q, z)->card));
                    duel_088C4730(a->player, 1000, a->player != q, id);
                    break;
                case 0x1520:
                    duel_088C4448(q, z, CARD_UID(SLOT(q, z)->card));
                    duel_088C87A8(q, z, 0x1520, 2, 200);
                    break;
                case 0x15FE:
                    duel_088C4448(q, z, CARD_UID(SLOT(q, z)->card));
                    duel_088C5424(q, 500);
                    break;
                default: /* spell counters */
                    if (duel_slot_e68a0(q, z) < card_get_group_value_1983(id)) {
                        duel_088CC284(q, z, 1);
                        if (field_get_card(q, z) == 0x16DE && duel_slot_e68a0(q, z) == 3) {
                            duel_088C87A8(q, z, 0x16DE, 3, a->player);
                        }
                    }
                    break;
                }
            }
        }
    }
    if (!a->b4_1 && !(a->b4_5 && a[1].id == 0x1975) && a->n_targets != 0) {
        for (i = 0; (int)i < 2; i++) {
            q = duel_turn_player ^ i;
            for (z = 0; z < 5; z++) {
                if (SLOT(q, z)->card.id != 0 && SLOT_ACTIVE(SLOT(q, z)) &&
                    duel_slot_effect_with(q, z, 0x1596, a->uid)) {
                    duel_088C87A8(q, z, 0x1598, 2, 0);
                }
            }
        }
    }
    if (!a->b4_1 && a->zone < 11 && a->b3_4 == 0 && duel_slot_effect_with(a->player, 0xB, 0x1379, a->uid)) {
        duel_088C44E4(a->player, 0x1379);
        duel_088C4730(a->player, 2000, 0, 0x1379);
    }
    if (card_get_prop5_hi(a->id) == 1 && prev != 0 && prev->player != a->player && prev->b4_1 &&
        !(duel_w_08917DB4 & 1) && duel_hand_count_name(a->player, 0x190A)) {
        q = a->player;
        k = duel_hand_find_name(q, 0x190A);
        duel_0884F30C(a->player << 31 | 0x50190A, CARD_UID(duel.players[q].hand[k]), prev->id);
    }
    if (card_get_prop5_hi(a->id) == 1 && !a->b4_1 && !(duel_w_08917DB4 & 1) &&
        duel_hand_count_name(a->player, 0x1A28)) {
        q = a->player;
        k = duel_hand_find_name(q, 0x190A); /* sic: the uid of 0x190A */
        duel_0884F30C(a->player << 31 | 0x501A28, CARD_UID(duel.players[q].hand[k]), 0);
    }
}

int duel_08850F44(void)
{
    DuelAct *a = &duel_chain.acts[duel_chain.unk480 - 1];
    DuelAct *prev;
    DuelAct *x;
    int i;
    unsigned int q;
    int z;
    int id;

    if (duel_chain.unk480 < 2) {
        prev = 0;
    } else {
        prev = a - 1;
    }
    switch (duel_w_089148F4) {
    case 0: /* chain of 2 links or more: show it */
        duel_w_08914920 = 0;
        duel_w_089149D4 = 0;
        if (duel_chain.unk480 >= 2) {
            duel_088C7B90(0x1E);
            duel_w_089148F4++;
            return 1;
        }
        duel_w_089148F4 += 2;
        return 1;
    case 1: /* announce the link */
        duel_cmd((a->player ? 0x8000 : 0) | 0x13, a->zone, duel_chain.unk480, 0);
        duel_w_089148F4++;
        return 1;
    case 2:
        if (duel_w_089195EC != 0) {
            duel_msg_send3(0x13, duel_chain.unk480 - 1, 0, 0);
        }
        duel_chain.len = duel_chain.unk480 - 1;
        duel_w_089148F4++;
    case 3: /* the card is negated before resolving */
        if (!a->b4_2 && duel_0886C448(a)) {
            a->b4_2 = 1;
            duel_088CBAA4(a->player, a->zone, a->uid);
        }
        duel_w_089148FC = 0;
        duel_w_089148F4++;
        duel_w_08914900 = 0;
        duel_w_0891968C = 0;
        for (i = 0; i < a->n_targets; i++) {
            duel_088A9174(a, i);
        }
        return 1;
    case 4:
        return duel_link_targets(a);
    case 5: /* start of the card's step machine */
        duel_step = 0x80;
        duel_w_089148FC = 0;
        duel_w_08914900 = 0;
        duel_w_089147DC = a;
        if (duel_0884F038(a) == 0) {
            if (!a->b4_1) {
                if (!card_is_monster(a->id) && duel_view_player == (a->player ^ a->opp)) {
                    duel.list[a->uid].b15 = 1;
                }
                duel_0884DBAC(a, duel_chain.unk480 - 1);
            }
            if (duel_0886B1F4(a) == 0) {
                duel_w_089148F4 = 7;
                duel_step = 0;
                return 1;
            }
        } else {
            duel_msg_send(0x18, duel_chain.unk480 - 1, a, 0x18);
        }
        duel_w_089148F4++;
        return 1;
    case 6: /* the card's step machine */
        if (duel_0884F038(a) == 0) {
            duel_step = duel_0884EC10(a, prev);
        }
        if (duel_step != 0) {
            return 1;
        }
        duel_w_089148F4++;
        return 1;
    case 7:
        duel_link_resolved(a, prev);
        duel_w_089148F4 = 100;
        return 1;
    case 100:
        if (a->zone < 11 && duel.players[a->player & 1].field[a->zone].flags >> 1 & 1) {
            duel_cmd((a->player ? 0x8000 : 0) | 0x39, a->zone, 0, 0);
            return 1;
        }
        if (duel_088C3F80()) {
            return 1;
        }
        duel_w_089148F4++;
        return 1;
    case 101: /* next link */
        duel_0884D1E0();
        if (duel_chain.unk480 - 1 != 0) {
            duel_chain.unk480--;
            duel_w_089148F4 = 1;
            return 1;
        }
        duel_w_089148F4++;
        duel_chain.unk480 = 0;
    case 102: /* end of the chain */
        duel_w_08914924 = 1;
        for (i = duel_w_089147D8 - 1; i >= 0; i--) {
            x = &duel_chain.acts[i];
            if (x->zone < 11 && SLOT(x->player, x->zone)->card.id != 0 && SLOT(x->player, x->zone)->flags >> 2 & 1) {
                if (SLOT(x->player, x->zone)->faceup == 0) {
                    duel_088C7E68(x->player, x->zone, 2, 0);
                } else {
                    duel_088D1BEC(x->player, x->zone, 0, 0);
                }
            }
        }
        for (q = 0; (int)q < 2; q++) {
            for (z = 0; z < 11; z++) {
                if (SLOT(q, z)->card.id == 0) {
                    continue;
                }
                id = field_get_card(q, z);
                if (id == 0x1598 || id == 0x1596) {
                    duel_088C89BC(q, z, 0x1596, 1);
                }
                duel_088C89BC(q, z, 0x161A, 1);
                duel_088C89BC(q, z, 0x1518, 1);
            }
        }
        duel_w_089148F4++;
        return 1;
    case 103:
        if (duel_088C3F80()) {
            return 1;
        }
        duel_w_089148F4++;
        duel_w_08914924 = 0;
        return 1;
    }
    duel_w_089148F0 = 0;
    return 1;
}
#endif
