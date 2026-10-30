// @range 0x088CC8EC-0x088D13CC
// @asm 0x088CC8EC-0x088CEBA8
// @asm 0x088CEEE0-0x088D13CC
/* Cards leaving the field: the triggers of a card that left (duel_088CC8EC), which zones an action can
   reach (duel_act_hits_mask), marking the zones that leave together (duel_leave_mark), sending one
   card to another pile with its host message (duel_send_card), the cards that follow it
   (duel_088CFBA4), and duel_act_apply, which does all of it for a mask of zones. */
#include "duel.h"
#include "card_db.h"

extern unsigned int duel_turn_player;
extern int duel_phase;

int field_get_card(unsigned int p, int z);
unsigned int duel_field_type(unsigned int p, int z);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
int duel_act_can_affect(DuelAct *a, unsigned int p, int z, int arg);
int duel_slot_efacc(unsigned int p, int z, int id);
int duel_find_effect_card(unsigned int p, int id, int skip);
int duel_count_active(int id);
int duel_card_check_9148(unsigned int p, int id);
int duel_slot_immune(unsigned int p, int z, int opp);
int duel_slot_ref_immune(unsigned int p, int z, int opp);
int duel_slot_affected(unsigned int p, int z, int id);
unsigned short duel_slot_src_attr8(unsigned int p, int z, int active);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_effect_with(unsigned int p, int z, unsigned int id, unsigned int arg);
int duel_list_has_effect(int uid, int id);
int duel_slot_e8170(unsigned int p, int z, int id);
unsigned short duel_slot_e1854(unsigned int p, int z);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
int duel_card_can_affect(unsigned int p, int z, unsigned short id, unsigned char zone, int arg);
unsigned short duel_special_e1934(unsigned int p, unsigned int z);
int duel_special_get_id(DuelSlot *s);
unsigned int duel_d_find_name(unsigned int p, int id);
int duel_card_protected(int id, int uid);
int duel_effect_live(DuelEffect *ef, int b, int c);
DuelEffect *duel_card_foreach_effect(DuelSlot *s, int (*fn)(DuelEffect *e, void *ctx), void *ctx);
int duel_088CC820();
unsigned int duel_dfc10(void);
void duel_0886BCDC(void);
void duel_088CC594(unsigned int p, int z, int a, int id);
int duel_0884F30C(unsigned int code, int uid, int arg);
void duel_088AFD5C(int kind, struct DuelActX x);
void duel_088C77D4(unsigned int p, unsigned short id, int a, unsigned short b);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_088C7E68(unsigned int p, unsigned int z, unsigned int bit, unsigned int on);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_088C02A4(int id, unsigned int v);
void duel_088C050C(unsigned int p, int id);
void duel_088C05B0(int id);
void duel_088C4448(unsigned int p, int a, unsigned int uid);
int duel_088C4730(unsigned int p, int n, int f, int id);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_send_card(unsigned int p, int z, int dest, unsigned int f, int cause);
void duel_leave_mark(unsigned int p, int z, unsigned int f, int cause, int opp);
void duel_088CFBA4(DuelSlot *s, unsigned int p, int z, unsigned int f);

#define SLOT_AT(p, z) (&duel.players[(p) & 1].field[z])

/* code word of duel_0884F30C: card id, zone, flag 24, kind (0x16 special card, 0x1B field), player */
#define TRIG(p, kind, id) ((unsigned int)(p) << 31 | (kind) << 25 | (id))

#ifdef NON_MATCHING /* 8.8 KB; escrita a partir do asm, alocação e escalonamento não comparados */
/* A card left the field (x: where it was and where it went, s: its slot, cause: card that moved it,
   f: flags of the move): posts the triggers of the card itself and of the cards that watch it. */
void duel_088CC8EC(struct DuelActX x, DuelSlot *s, int cause, unsigned int f)
{
    unsigned int owner = s->card.owner;
    int uid = CARD_UID(s->card);
    unsigned int p = x.s_player;      /* where it was */
    unsigned int z = x.s_zone;
    int destroyed = x.s_b15;
    int id = s->card.id;
    int by_effect = (f & 0x80000) != 0;
    int by_opp = x.s_b18 != p;        /* sent by the opponent of its controller */
    int keep = !(x.s_b23 & 1);
    int to_grave = x.s_zone2 == 0xE;
    int f100 = (f & 0x100000) != 0;
    int f200 = (f & 0x200000) != 0;
    int special = x.s_b16;            /* one of the cards held outside the zones */
    int kind = special ? 0x16 : 0x1B;
    unsigned int type;
    int trig = 0;                     /* 1: trigger with the destination, 3: with the zone */
    int prot;
    int guarded;
    unsigned int v;
    unsigned int q;
    unsigned int i;
    unsigned int e;
    DuelEffect *ef;
    DuelCard *c;
    int n;

    if (!special) {
        type = (unsigned short)duel_field_type(p, z);
    } else {
        type = duel_special.act[duel_special.act[1].uid == uid].pad14[4];
    }
    switch (id) {
    case 0x13A1:
        if (s->faceup != 0 && x.s_zone2 == 0xD) {
            duel_0884F30C(TRIG(owner, kind, 0x4D13A1), uid, *(int *)&x);
        }
        break;
    case 0x1668:                       /* the values of its 0x1668 entries */
        if (keep && destroyed && to_grave) {
            v = 0;
            for (e = s->effects; e != 0;) {
                ef = &duel_effects[e];
                e = ef->next;
                if (ef->src == 0x1668) {
                    v = v << 16 | ef->arg;
                }
            }
            if (v != 0) {
                duel_0884F30C(TRIG(owner, kind, 0x400000 | 0x1668) | x.s_zone2 << 16, uid, v);
            }
        }
        break;
    case 0x164F:
        if (keep && destroyed && to_grave) {
            for (e = s->effects; e != 0;) {
                ef = &duel_effects[e];
                e = ef->next;
                if (ef->src == 0x164F) {
                    duel_0884F30C(TRIG(owner, kind, 0x400000 | 0x164F) | x.s_zone2 << 16, uid, ef->arg);
                    break;
                }
            }
        }
        break;
    case 0x1AAD:
    case 0x1B2D:
        trig = destroyed && !special && by_opp && p == owner && to_grave;
        break;
    case 0x1AA9:
        trig = destroyed && cause == 0x1AA9;
        break;
    case 0x1A0C:
        trig = destroyed && to_grave && (unsigned int)SLOT_AT(p, z)->link > 3;
        break;
    case 0x19B0:
        trig = f100 && !f200 && (f & 0x400000) != 0;
        break;
    case 0x1842:
        trig = f100 && to_grave;
        break;
    case 0x1841:
        if (f200 && to_grave && p == owner && p == (duel_special_e1934(p, z) & 0xFF)) {
            trig = 1;
        }
        break;
    case 0x183D:
        trig = s->faceup != 0 && keep;
        break;
    case 0x16FE:                       /* trigger carries its controller and its slot value */
        trig = s->faceup != 0 && !special && destroyed;
        if (trig) {
            *(unsigned int *)&x = (p ^ by_opp) | (unsigned int)(unsigned short)SLOT_AT(p, z)->link << 16;
        }
        break;
    case 0x1294:
        trig = destroyed;
        break;
    case 0x1514:
        if (by_effect && destroyed && (short)duel_slot_e1854(p, z) != -1 && !(duel.mode & 1) &&
            !(SLOT_AT(p, z)->flags & 0x20)) {
            trig = 3;
        }
        break;
    case 0x150D:
        if (by_effect && destroyed && (short)duel_slot_e1854(p, z) != -1 && !(SLOT_AT(p, z)->flags & 0x20)) {
            trig = 3;
        }
        break;
    case 0x18D7:
        if (s->faceup == 0 && destroyed && !(SLOT_AT(p, z)->flags & 0x20)) {
            trig = 3;
        }
        break;
    case 0x1A7D:
        trig = s->faceup == 0 && destroyed && to_grave && by_opp && card_is_monster(cause) == 0;
        break;
    case 0x1435:
    case 0x1543:
    case 0x1544:
    case 0x1A5D:
        trig = s->faceup == 0 && destroyed && to_grave;
        break;
    case 0x19D2:
        trig = s->faceup == 0 && s->pos != 0 && destroyed && !special && to_grave;
        break;
    case 0x16F6:
    case 0x1869:
    case 0x18AA:
    case 0x1951:
        trig = keep && destroyed && to_grave;
        break;
    case 0x1914:
        trig = destroyed && s->faceup != 0 && keep && (!special || duel_special.unk24 != 0x1914);
        break;
    case 0x174E:
    case 0x1A07:
        trig = special && destroyed && to_grave && duel_special.unk24 != id;
        break;
    case 0x13AD:
    case 0x13B0:
    case 0x13B4:
    case 0x1612:
    case 0x172F:
        trig = special && destroyed;
        break;
    case 0x1919:
    case 0x194A:
        if (owner != p) {
            break;
        }
        /* fall through */
    case 0x130B: case 0x1333: case 0x1335: case 0x1339: case 0x133A: case 0x133C: case 0x133E:
    case 0x133F: case 0x1342: case 0x139D: case 0x139E: case 0x13C7: case 0x14AB: case 0x14DD:
    case 0x14E7: case 0x14F6: case 0x152F: case 0x156C: case 0x156D: case 0x1591: case 0x1616:
    case 0x1642: case 0x16C7: case 0x16C8: case 0x17C3: case 0x17D6: case 0x17E5: case 0x17E6:
    case 0x1867: case 0x18B7: case 0x18C5: case 0x18F4: case 0x18F7: case 0x1939: case 0x188F:
    case 0x1950: case 0x196A: case 0x19C5: case 0x1A94:
        trig = special && destroyed && to_grave;
        break;
    case 0x1A8F:
        trig = destroyed && to_grave && !special;
        break;
    case 0x14E9:
    case 0x163F:
        trig = destroyed && to_grave;
        break;
    case 0x165C:
        trig = by_effect && destroyed && to_grave && (short)duel_slot_e1854(p, z) != -1 && !(duel.mode & 1);
        break;
    case 0x1185: case 0x12A2: case 0x1309: case 0x13E3: case 0x1687: case 0x16F5: case 0x17C5:
    case 0x19FE:
        trig = to_grave;
        break;
    case 0x1A90:
        if (!special) {
            break;
        }
        /* fall through */
    case 0x1174:
    case 0x133B:                       /* also when it carries 0x1174 (then flag bit 23 of x) */
        if (keep && to_grave) {
            if (special && duel_special.unk24 == id) {
                trig = 1;
            } else if (s->flags & 0x10) {
                trig = 1;
            } else if (duel_slot_has_effect(p, z, 0x1174)) {
                x.s_b23 |= 1;
                trig = 1;
            }
        }
        break;
    case 0x191A:
        trig = s->faceup != 0 && keep && to_grave && (!special || duel_special.unk24 != 0x191A);
        break;
    case 0x1A8E:
        trig = keep && to_grave && !special;
        break;
    case 0x12A4:
        trig = keep && to_grave && owner == p;
        break;
    case 0xFD6: case 0x10D6: case 0x10DD: case 0x10DF: case 0x10E0: case 0x10E7: case 0x11E4:
    case 0x13E9: case 0x1467: case 0x15B9: case 0x16CC: case 0x1796: case 0x17A5: case 0x1946:
        trig = keep && to_grave;
        break;
    }
    if (duel_slot_has_effect(p, z, 0x19AE)) {
        trig = 0;
    }

    /* monsters destroyed and sent to the graveyard: field spells 0x169F and 0x187F, and 0x1971 */
    if (!special && destroyed && to_grave && z < 5 && card_is_monster(id) && card_is_in_group_1911(id)) {
        for (q = 0; q < 2; q++) {
            if (duel_slot_e8170(q, 10, 0x169F)) {
                c = &SLOT_AT(q, 10)->card;
                duel_0884F30C(q << 31 | kind << 25 | (owner != q) << 24 | 0x2A169F, CARD_UID(*c),
                              *(int *)&x);
            }
        }
    }
    if (destroyed && to_grave && z < 5 && card_attribute_icon_is3(id)) {
        if (!special) {
            for (q = 0; q < 2; q++) {
                if (duel_slot_e8170(q, 10, 0x187F)) {
                    c = &SLOT_AT(q, 10)->card;
                    duel_0884F30C(q << 31 | kind << 25 | (owner != q) << 24 | 0x2A187F, CARD_UID(*c),
                                  *(int *)&x);
                }
            }
        }
        for (n = 0; n < 2; n++) {
            q = duel_turn_player ^ n;
            for (i = 5; i < 10; i++) {
                if (duel_slot_e8170(q, i, 0x1971)) {
                    c = &SLOT_AT(q, i)->card;
                    duel_0884F30C((i & 0x1F) << 16 | q << 31 | kind << 25 | 0x200000 | 0x1971,
                                  CARD_UID(*c), *(int *)&x);
                }
            }
        }
    }
    if (special && to_grave && duel_find_effect_card(1 - p, 0x18D1, -1)) {
        duel_card_foreach_effect(s, duel_088CC820, &x);
    }
    if (special && to_grave && card_is_in_group_1a5c(id)) {
        for (i = 0; i < 5; i++) {
            if (duel_slot_e8170(p, i, 0x1A59)) {
                c = &SLOT_AT(p, i)->card;
                duel_0884F30C((i & 0x1F) << 16 | p << 31 | kind << 25 | 0x200000 | 0x1A59, CARD_UID(*c),
                              *(int *)&x);
            }
        }
    }
    if (special && to_grave) {
        for (i = 0; i < 5; i++) {
            if (duel_slot_e8170(p, i, 0x1A58)) {
                c = &SLOT_AT(p, i)->card;
                duel_0884F30C((i & 0x1F) << 16 | p << 31 | kind << 25 | 0x200000 | 0x1A58, CARD_UID(*c),
                              *(int *)&x);
            }
        }
    }
    if (id == 0x18AA && destroyed && (int)duel_d_find_name(p, 0x1445) >= 0) {
        duel_0884F30C(p << 31 | kind << 25 | 0x501445, 0, *(int *)&x);
    }
    if (card_get_type(id) == 0xB && destroyed && to_grave) {   /* 0x18EC in graveyard and hand */
        for (n = duel.players[p].n_grave - 1; n >= 0; n--) {
            c = &duel.players[p].grave[n];
            if (c->id == 0x18EC && !c->b21) {
                duel_0884F30C(p << 31 | kind << 25 | 0x4E18EC, CARD_UID(*c), *(int *)&x);
            }
        }
        for (n = duel.players[p].n_hand - 1; n >= 0; n--) {
            c = &duel.players[p].hand[n];
            if (c->id == 0x18EC) {
                duel_0884F30C(p << 31 | kind << 25 | 0x5018EC, CARD_UID(*c), *(int *)&x);
            }
        }
    }
    if (card_get_type(id) == 9 && to_grave) {
        for (n = 0; n < 5; n++) {
            if (duel_slot_e8170(owner, n, 0x1A8D)) {
                duel_088C7E68(owner, n, 4, 1);
            }
        }
    }

    /* cards that leave a mark in the owner's list */
    prot = duel_card_protected(id, uid) != 0;
    guarded = duel_slot_effect_with(duel_turn_player, 0xB, 0x15FF, uid) != 0;
    switch (id) {
    case 0x1A9F:
        if (owner == p && z > 4 && s->faceup == 0 && by_opp && destroyed && !special && to_grave) {
            duel_088C77D4(owner, 0x1A9F, 1, uid);
        }
        break;
    case 0x1A64:
        if (!guarded && !prot && special && destroyed && to_grave) {
            duel_088C77D4(owner, 0x1A64, ((owner == duel_turn_player) + 1) * 0x10 | 1, uid);
        }
        break;
    case 0x1A62:
        if (!guarded && !prot && destroyed && to_grave) {
            duel_088C77D4(owner, 0x1A62, ((owner == duel_turn_player) + 1) * 0x10 | 1, uid);
        }
        break;
    case 0x19F7:
    case 0x19F8:
        if (!guarded && !prot && special && destroyed && to_grave) {
            duel_088C77D4(owner, id, 1, uid);
        }
        break;
    case 0x18BC:
        if (!guarded && !prot && keep && x.s_zone2 == 0xF && s->faceup != 0 &&
            (!special || duel_special.unk24 != 0x18BC)) {
            duel_088C77D4(p, 0x18BC, 1, uid);
        }
        break;
    case 0x185C:
        if (!guarded && !prot && z < 5 && !special && destroyed && to_grave) {
            n = owner == duel_turn_player ? (duel_phase != 0) << 1 : 1;
            duel_088C77D4(owner, 0x185C, n << 4 | 1, uid);
        }
        break;
    case 0x17DE:
        if (!guarded && !prot && keep && to_grave && card_get_type(cause) == 0x16 &&
            card_get_prop5_hi(cause) == 4) {
            n = owner == duel_turn_player ? (duel_phase != 0) << 1 : 1;
            duel_088C77D4(owner, 0x17DE, n << 4 | 1, uid);
        }
        break;
    case 0x156B:
        if (s->faceup == 0) {
            break;
        }
        /* fall through */
    case 0x1522:
        if (!guarded && !prot && p == owner && z < 5 && !special && destroyed && to_grave && by_opp) {
            n = owner == duel_turn_player ? (duel_phase != 0) << 1 : 1;
            duel_088C77D4(owner, id, n << 4 | 1, uid);
        }
        break;
    case 0x1466:
        if (!guarded && !prot && p == owner && z < 5 && destroyed && to_grave && by_opp) {
            duel_088C77D4(p, 0x1466, 1, uid);
        }
        break;
    case 0x13A6:
        if (!guarded && !prot && keep && destroyed && to_grave && duel_list_has_effect(uid, s->card.id) == 0) {
            duel_088C77D4(owner, s->card.id, 1, uid);
        }
        break;
    case 0x16F9:
        if (!guarded && !prot && to_grave) {
            duel_088C77D4(owner, 0x16F9, 1, uid);
        }
        break;
    case 0x1670:
    case 0x1671:
    case 0x1672:
        if (destroyed) {
            duel_088C4730(p, 300, 0, 0x166A);
        }
        break;
    case 0x15BD:
        if (special && destroyed) {
            duel_088C4730(s->link, 500, 1, 0x1595);
        }
        break;
    case 0x1571:
        if (guarded || prot) {
            duel_088C77D4(owner, 0x1571, 5, uid);
        }
        break;
    case 0x14A5:
        if (!guarded && !prot && to_grave && !s->card.b14) {
            duel_088C4448(owner, 0xE, uid);
            duel_088C77D4(owner, 0x14A5, 1, uid);
        }
        break;
    }

    if (keep) {
        if (destroyed && type == 0x11 && duel_card_check_9148(p, 0x1782)) {
            for (i = 5; i < 11; i++) {
                if (duel_slot_e8170(p, i, 0x1843)) {
                    duel_088C87A8(p, i, 0x1843, 2, 0);
                    for (n = 0; n < 5; n++) {
                        if (field_get_card(p, n) == 0x1782 && SLOT_AT(p, n)->faceup != 0 &&
                            !(duel.zone_off & 1 << (n + p * 16))) {
                            duel_088C87A8(p, n, p | (i & 0xFF) << 8, 8, 0);
                        }
                    }
                }
            }
        }
        if (destroyed && s->faceup != 0) {
            if (type == 0xB) {
                duel_088C05B0(0x1862);
            }
            if (type == 0x10) {
                duel_088C050C(p, 0x1875);
            }
        }
        if (type == 1 && (f & 0x1000)) {
            duel_088C77D4(p, 0x14E2, 2, 0);
        }
    }
    if (special && to_grave && card_is_in_group_1a6x(id)) {
        for (i = 0; i < 5; i++) {
            if (i != z && duel_slot_e8170(p, i, 0x1A03)) {
                n = owner == duel_turn_player ? 2 : 1;
                duel_088C87A8(p, i, 0x1A03, n << 4 | 2, uid);
            }
        }
    }
    if (x.s_zone2 == 0xB && z < 5 && card_is_attribute9(id) == 0) {   /* back to the hand */
        duel_088C05B0(0x142C);
        if ((x.s_b17) && card_is_attribute7(id)) {
            duel_088C05B0(0x151A);
        }
        duel_088C02A4(0x18B2, 1 << (owner << 3));
    }
    if (trig == 3) {
        duel_0884F30C(id | z << 16 | p << 31 | kind << 25 | 0x200000, uid, *(int *)&x);
    } else if (trig != 2 && trig == 1) {
        duel_0884F30C(id | x.s_zone2 << 16 | owner << 31 | kind << 25 | 0x400000, uid, *(int *)&x);
    }
}
#endif

/* Whether action a reaches the card in zone z of player p (kind 0xF: banishing; check: targeting). */
int duel_act_hits_slot(DuelAct *a, unsigned int p, int z, int kind, int check)
{
    int id = SLOT_AT(p, z)->card.id;

    if (id == 0) {
        return 0;
    }
    if (duel.zone_off & 1 << (z + p * 16)) {
        return 0;
    }
    if (duel_act_immune(a, p, z)) {
        return 0;
    }
    if (check && duel_act_can_affect(a, p, z, 0) == 0) {
        return 0;
    }
    if (kind == 0xF && duel_slot_efacc(p, z, a->id) == 0) {
        return 0;
    }
    if (check && id == 0x169F && SLOT_AT(p, z)->faceup != 0 && p == 1 - (a->player ^ a->opp) &&
        duel_find_effect_card(p, 0x1683, -1)) {
        return 0;
    }
    if (a->b4_4 && check && p == 1 - (a->player ^ a->opp) && z >= 5 && card_get_type(id) == 0x17) {
        if (id != 0x1258 || !(SLOT_AT(p, z)->flags >> 2 & 1)) {
            return 0;
        }
    }
    return 1;
}

/* The zones of mask (bit player * 16 + zone) that action a reaches. */
unsigned int duel_act_hits_mask(DuelAct *a, unsigned int mask, int kind, int check)
{
    unsigned int bit;
    int p;
    int z;
    unsigned int hit = 0;

    for (p = 0; p < 2; p++) {
        for (z = 0; z <= 10; z++) {
            bit = 1 << (z + p * 16);
            if ((mask & bit) && duel_act_hits_slot(a, p, z, kind, check)) {
                hit |= bit;
            }
        }
    }
    return hit;
}

#ifdef NON_MATCHING /* 3 KB; escrita a partir do asm */
/* Marks zone z of player p as leaving (duel.zone_off) together with the cards that go with it:
   equips, cards linked by effect, the cards named by 0x1A95, 0x1A7C, 0x1A2B-0x1A2E, 0x150E... */
void duel_leave_mark(unsigned int p, int z, unsigned int f, int cause, int opp)
{
    int destroy = (f & 2) != 0;
    int id = field_get_card(p, z);
    unsigned int bit;
    unsigned int q;
    unsigned int v;
    unsigned int e;
    DuelEffect *ef;
    DuelSlot *s;
    int i;

    if (card_get_type(cause) == 0x16) {
        if (duel_slot_immune(p, z, opp)) {
            return;
        }
    } else if (card_get_type(cause) == 0x17) {
        if (duel_slot_ref_immune(p, z, opp)) {
            return;
        }
    }
    if (z < 5 && destroy && duel_slot_affected(p, z, 0x1825)) {   /* its substitute goes instead */
        v = duel_slot_src_attr8(p, z, 1);
        duel_leave_mark(v & 0xFF, v >> 8, 2, 0, 0);
        return;
    }
    if (z > 4 && destroy && cause != 0 && duel_slot_has_effect(p, z, 0x1513)) {
        duel_088CC594(p, z, -1, 0x1513);
        return;
    }
    if (id == 0) {
        return;
    }
    bit = 1 << (z + p * 16);
    if (duel.zone_off & bit) {
        return;
    }
    duel.zone_off |= bit;
    switch (id) {
    case 0x1A95:
        for (q = 0; q < 2; q++) {
            for (i = 0; i < 11; i++) {
                if (duel_st_affects(q, i, p, z) && SLOT_AT(q, i)->card.id != 0 &&
                    duel_card_can_affect(q, i, 0x1A95, z, 0)) {
                    duel_leave_mark(q, i, 2, 0x1A95, q != p);
                }
            }
        }
        break;
    case 0x1A7C:
        if (z > 4 && SLOT_ACTIVE(SLOT_AT(p, z))) {
            for (q = 0; q < 2; q++) {
                for (i = 0; i < 5; i++) {
                    if (duel_slot_effect_with(q, i, 0x1A7C, CARD_UID(SLOT_AT(p, z)->card))) {
                        duel_leave_mark(q, i, 0, 0x1A7C, q != p);
                    }
                }
            }
        }
        break;
    case 0x1A2B:
    case 0x1A2C:
    case 0x1A2D:
    case 0x1A2E:
        if (SLOT_AT(p, z)->faceup != 0) {
            for (q = 0; q < 2; q++) {
                for (i = 0; i < 5; i++) {
                    if (SLOT_AT(q, i)->card.id != 0 && duel_slot_has_effect(q, i, id) &&
                        duel_card_can_affect(q, i, id, z, 0)) {
                        duel_leave_mark(q, i, 2, id, q != p);
                    }
                }
            }
        }
        break;
    case 0x150E:
        if (z > 4 && SLOT_ACTIVE(SLOT_AT(p, z))) {
            for (q = 0; q < 2; q++) {
                for (i = 0; i < 5; i++) {
                    s = SLOT_AT(q, i);
                    if (s->faceup != 0 && card_is_attribute7(s->card.id)) {
                        duel_leave_mark(q, i, 0, 0x150E, q != p);
                    }
                }
            }
        }
        break;
    case 0x17AD:
    case 0x17AE:
    case 0x17AF:                       /* the other pieces of the set */
        if (z > 4 && SLOT_AT(p, z)->faceup != 0 && duel_find_effect_card(p, 0x17AF, -1)) {
            for (i = 5; i < 10; i++) {
                s = SLOT_AT(p, i);
                if (i != z && s->card.id != 0 && s->faceup != 0 &&
                    (s->card.id == 0x17AE || s->card.id == 0x17AD || s->card.id == 0x17AF)) {
                    duel_leave_mark(p, i, 2, 0x17AF, 0);
                }
            }
        }
        break;
    case 0x1468:
    case 0x1497:
    case 0x1498:
    case 0x1499:
    case 0x149A:
        if (z > 4 && SLOT_AT(p, z)->faceup != 0 && duel_find_effect_card(p, 0x1468, -1)) {
            for (i = 5; i < 10; i++) {
                s = SLOT_AT(p, i);
                if (i != z && s->card.id != 0 && s->faceup != 0 &&
                    (s->card.id == 0x149A || s->card.id == 0x1499 || s->card.id == 0x1498 ||
                     s->card.id == 0x1497 || s->card.id == 0x1468)) {
                    duel_leave_mark(p, i, 2, 0x1468, 0);
                }
            }
        }
        break;
    case 0x1366:
    case 0x1880:
    case 0x1881:
    case 0x19D7:
        if (!destroy) {
            break;
        }
        /* fall through */
    case 0x12D3:
    case 0x15E6:
    case 0x1625:                       /* the monster it is attached to */
        if (SLOT_ACTIVE(SLOT_AT(p, z))) {
            v = duel_slot_e1854(p, z);
            if (v != 0xFFFF) {
                q = v & 0xFF;
                i = v >> 8;
                if (SLOT_AT(v, i)->card.id != 0 && SLOT_AT(v, i)->faceup != 0) {
                    if (id == 0x1881 || id == 0x1880 || id == 0x1625) {
                        duel_leave_mark(q, i, 0, id, q != p);
                    } else if (duel_card_can_affect(q, i, id, z, 1)) {
                        duel_leave_mark(q, i, 2, id, q != p);
                    }
                }
            }
        }
        break;
    case 0x137D:
    case 0x16A2:
    case 0x1768:
    case 0x17B7:
    case 0x1A36:
    case 0x1A42:
    case 0x1A7E:
    case 0x1A9B:
        if (SLOT_ACTIVE(SLOT_AT(p, z))) {
            for (q = 0; q < 2; q++) {
                for (i = 0; i < 11; i++) {
                    if (duel_st_affects(q, i, p, z) && SLOT_AT(q, i)->card.id != 0 &&
                        duel_card_can_affect(q, i, id, z, 1)) {
                        duel_leave_mark(q, i, 2, id, q != p);
                    }
                }
            }
        }
        break;
    case 0x12BE:
        if (!(f & 8) && destroy) {
            for (q = 0; q < 2; q++) {
                for (i = 0; i < 5; i++) {
                    if (SLOT_ACTIVE(SLOT_AT(q, i)) && card_is_attribute6(field_get_card(q, i))) {
                        duel_leave_mark(q, i, 2, 0, q != p);
                    }
                }
            }
        }
        break;
    }
    for (e = SLOT_AT(p, z)->effects; e != 0;) {   /* cards bound to it by an effect entry */
        ef = &duel_effects[e];
        e = ef->next;
        if (duel_effect_live(ef, destroy, 0)) {
            duel_leave_mark(ef->src & 0xFF, ef->src >> 8, 2, 0, (ef->src & 0xFF) != p);
        }
    }
}
#endif

#ifdef NON_MATCHING /* 3 KB; escrita a partir do asm */
/* After the card of slot s (zone z of player p) left: sends the cards that go with it (the same
   cases as duel_leave_mark), then the cards bound to it by effect entries. */
void duel_088CFBA4(DuelSlot *s, unsigned int p, int z, unsigned int f)
{
    int id = duel_special_get_id(s);
    int destroy = (f & 2) != 0;
    unsigned int n;
    unsigned int q;
    unsigned int v;
    unsigned int e;
    DuelEffect *ef;
    DuelSlot *t;
    int i;

    switch (id) {
    case 0x1A95:
        for (n = 0; n < 2; n++) {
            q = duel_turn_player ^ n;
            for (i = 0; i < 11; i++) {
                if ((duel_st_affects(q, i, p, z) || duel_slot_effect_with(q, i, 0x1A95, CARD_UID(s->card))) &&
                    SLOT_AT(q, i)->card.id != 0 && duel_card_can_affect(q, i, 0x1A95, z, 0)) {
                    duel_send_card(q, i, 0xE, (q != p) + 2, 0x1A95);
                }
            }
        }
        break;
    case 0x1A7C:
        if (z > 4 && SLOT_ACTIVE(SLOT_AT(p, z))) {
            for (n = 0; n < 2; n++) {
                q = duel_turn_player ^ n;
                for (i = 0; i < 5; i++) {
                    if (duel_slot_effect_with(q, i, 0x1A7C, CARD_UID(SLOT_AT(p, z)->card))) {
                        duel_send_card(q, i, 0xB, q != p, 0x1A7C);
                    }
                }
            }
        }
        break;
    case 0x1A2B:
    case 0x1A2C:
    case 0x1A2D:
    case 0x1A2E:
        if (s->faceup != 0) {
            for (n = 0; n < 2; n++) {
                q = duel_turn_player ^ n;
                for (i = 0; i < 5; i++) {
                    if (duel_slot_has_effect(q, i, id)) {
                        duel_send_card(q, i, 0xE, (q != p) + 2, id);
                    }
                }
            }
        }
        break;
    case 0x150E:
        if (z > 4 && SLOT_ACTIVE(SLOT_AT(p, z))) {
            for (n = 0; n < 2; n++) {
                q = duel_turn_player ^ n;
                for (i = 0; i < 5; i++) {
                    t = SLOT_AT(q, i);
                    if (t->faceup != 0 && card_is_attribute7(t->card.id)) {
                        duel_send_card(q, i, 0xB, q != p, 0x150E);
                    }
                }
            }
        }
        break;
    case 0x17AD:
    case 0x17AE:
    case 0x17AF:
        if (z > 4 && SLOT_AT(p, z)->faceup != 0 && duel_find_effect_card(p, 0x17AF, -1)) {
            for (i = 5; i < 10; i++) {
                t = SLOT_AT(p, i);
                if (i != z && t->card.id != 0 && t->faceup != 0 &&
                    (t->card.id == 0x17AE || t->card.id == 0x17AD || t->card.id == 0x17AF)) {
                    duel_send_card(p, i, 0xE, 0, 0x17AF);
                }
            }
        }
        break;
    case 0x1468:
    case 0x1497:
    case 0x1498:
    case 0x1499:
    case 0x149A:
        if (z > 4 && SLOT_AT(p, z)->faceup != 0 && duel_find_effect_card(p, 0x1468, -1)) {
            for (i = 5; i < 10; i++) {
                t = SLOT_AT(p, i);
                if (i != z && t->card.id != 0 && t->faceup != 0 &&
                    (t->card.id == 0x149A || t->card.id == 0x1499 || t->card.id == 0x1498 ||
                     t->card.id == 0x1497 || t->card.id == 0x1468)) {
                    duel_send_card(p, i, 0xE, 2, 0x1468);
                }
            }
        }
        break;
    case 0x1366:
    case 0x1880:
    case 0x1881:
    case 0x19D7:
        if (!destroy) {
            break;
        }
        /* fall through */
    case 0x12D3:
    case 0x15E6:
    case 0x1625:                       /* the monster it is attached to: banished or destroyed */
        if (SLOT_AT(p, z)->card.id != 0 && SLOT_ACTIVE(SLOT_AT(p, z))) {
            v = duel_slot_e1854(p, z);
            if (v != 0xFFFF) {
                q = v & 0xFF;
                i = v >> 8;
                if (SLOT_AT(v, i)->card.id != 0 && SLOT_AT(v, i)->faceup != 0) {
                    if (id == 0x1881 || id == 0x1880 || id == 0x1625) {
                        duel_send_card(q, i, 0xF, q != p, id);
                    } else if (duel_card_can_affect(q, i, id, z, 1)) {
                        duel_send_card(q, i, 0xE, (q != p) + 2, id);
                    }
                }
            }
        }
        break;
    case 0x17BF:                       /* banishes its 0x17C7/0x17C8 monsters */
        if (SLOT_AT(p, z)->card.id != 0 && SLOT_ACTIVE(SLOT_AT(p, z))) {
            for (i = 0; i < 5; i++) {
                n = field_get_card(p, i);
                if ((n == 0x17C8 || n == 0x17C7) && SLOT_AT(p, i)->faceup != 0) {
                    duel_send_card(p, i, 0xF, 2, 0x17BF);
                }
            }
        }
        break;
    case 0x137D:
    case 0x16A2:
    case 0x1768:
    case 0x17B7:
    case 0x1A36:
    case 0x1A42:
    case 0x1A7E:
    case 0x1A9B:
        if (SLOT_AT(p, z)->card.id != 0 && SLOT_ACTIVE(SLOT_AT(p, z))) {
            for (q = 0; q < 2; q++) {
                for (i = 0; i < 11; i++) {
                    if (duel_st_affects(q, i, p, z) && SLOT_AT(q, i)->card.id != 0 &&
                        duel_card_can_affect(q, i, id, z, 1)) {
                        duel_send_card(q, i, 0xE, (q != p) + 2, id);
                    }
                }
            }
        }
        break;
    case 0x12BE:
        if (!(f & 8) && destroy) {
            for (n = 0; n < 2; n++) {
                q = duel_turn_player ^ n;
                for (i = 0; i < 5; i++) {
                    if (SLOT_ACTIVE(SLOT_AT(q, i)) && card_is_attribute6(field_get_card(q, i))) {
                        duel_send_card(q, i, 0xE, (q != p) + 2, 0x12BE);
                    }
                }
            }
        }
        break;
    }
    for (e = s->effects; e != 0;) {
        ef = &duel_effects[e];
        e = ef->next;
        if (duel_effect_live(ef, destroy, (f & 0x1000000) != 0)) {
            duel_send_card(ef->src & 0xFF, ef->src >> 8, 0xE,
                           (f & 0x400000) + destroy * 0x100000 + 2 + ((f & 4) != 0) * 0x200000, 0);
        }
    }
}
#endif

#ifdef NON_MATCHING /* 2 KB; escrita a partir do asm */
/* Sends the card in zone z of player p to pile dest (0xB hand, 0xC extra deck, 0xD deck, 0xE graveyard,
   0xF banished; < 11 another zone) because of card cause. f: bit 0 by the opponent, 1 destroyed,
   3 not by battle, 12, 18 and 23 host message variants, 16 skip the immunity checks. Returns 1 if
   the card moved. */
int duel_send_card(unsigned int p, int z, int dest, unsigned int f, int cause)
{
    DuelSlot *s = SLOT_AT(p, z);
    int id = s->card.id;
    unsigned int bit;
    int opp;
    int uid;
    int keep;
    int to_grave;
    int destroyed;
    unsigned int q;
    unsigned int n;
    unsigned int v;
    struct DuelActX x;

    if (!(f & 0x10000)) {
        if (card_get_type(cause) == 0x16) {
            if (duel_slot_immune(p, z, (f & 1) != 0)) {
                return 0;
            }
        } else if (card_get_type(cause) == 0x17 && duel_slot_ref_immune(p, z, (f & 1) != 0)) {
            return 0;
        }
    }
    if (z < 5 && (f & 2) && duel_slot_affected(p, z, 0x1825)) {   /* its substitute goes instead */
        v = duel_slot_src_attr8(p, z, 1);
        duel_send_card(v & 0xFF, v >> 8, 0xE, 2, 0);
        return 0;
    }
    if (z > 4 && (f & 2) && cause != 0 && duel_slot_has_effect(p, z, 0x1513)) {
        return 0;
    }
    if (dest != 0xE && dest != 0xF && (s->flags & 4) && cause != 0x19E1 && cause != 0x14FB) {
        return 0;
    }
    if (id == 0) {
        return 0;
    }
    bit = 1 << (z + p * 16);
    if (duel.zone_off & bit) {
        return 0;
    }
    opp = (f & 1) != 0;
    uid = CARD_UID(s->card);
    keep = (f & 8) == 0;
    to_grave = dest == 0xE ? card_is_attribute9(id) == 0 : 0;
    destroyed = (f & 2) != 0;
    *(unsigned int *)&x = 0;
    if (dest == 0xE) {                 /* rules that banish instead (duel.unk1f60 from duel_dfc10) */
        if (duel.unk1f60 & 1) {
            dest = 0xF;
        }
        if ((duel.unk1f60 & 2) && card_is_monster(id)) {
            dest = 0xF;
        }
        if ((duel.unk1f60 & 4) && (uid & 1) == 0) {
            dest = 0xF;
        }
        if ((duel.unk1f60 & 8) && (uid & 1) == 1) {
            dest = 0xF;
        }
        if (dest == 0xF) {
            to_grave = 0;
        }
    }
    if (keep && dest > 10 && duel_slot_e8170(p, z, 0x16F8)) {
        dest = 0xF;
        to_grave = 0;
    }
    if (card_get_prop5_hi(cause) == 6 && duel_count_active(0x19DA)) {
        dest = 0xD;
        to_grave = 0;
    }
    x.s_uid = uid;
    x.s_player = p;
    x.s_zone = z;
    x.s_b14 = s->pos;
    x.s_b15 = destroyed;
    x.s_b17 = s->faceup;
    x.s_b18 = p ^ opp;
    x.s_zone2 = dest;
    x.s_b23 = !keep;
    duel.zone_off |= bit;
    if (f & 0x1000) {
        duel_cmd((p == 0 ? 0 : 0x8000) | 0x46, z, dest, x.s_uid);
    } else if (destroyed) {
        duel_cmd((p == 0 ? 0 : 0x8000) | 0x31, z, (dest & 0xFF) | (opp | !keep << 4) << 8, x.s_uid);
    } else if (dest < 11) {
        duel_cmd((p == 0 ? 0 : 0x8000) | 0x3D, (p & 0xFF) | (z & 0xFF) << 8,
                 (dest & 0xFF) << 8 | ((p ^ ((f & 0x40000) != 0)) & 0xFF), 0);
    } else if (card_is_attribute9(id)) {
        duel_cmd((p == 0 ? 0 : 0x8000) | 0x31, z, 0x10, x.s_uid);
    } else if (dest == 0xF) {
        duel_cmd((p == 0 ? 0 : 0x8000) | 0x33, z & 0xFF, f, x.s_uid);
    } else {                           /* to its owner's hand, extra deck, deck or graveyard */
        q = ((f & 0x40000) != 0) ^ s->card.owner;
        switch (dest) {
        case 0xE:
            n = 1;
            break;
        case 0xD:
            n = (f & 0x800000) != 0;
            break;
        case 0xC:
            n = 1;
            break;
        case 0xB:
            n = duel.players[q].n_hand;
            break;
        default:
            n = 0;
            break;
        }
        duel_cmd(0x40, (p & 1) | (z & 0x1F) << 1, (n & 0x3FF) << 6 | q | (dest & 0x1F) << 1, f);
    }
    if (dest > 10) {
        duel_088CC8EC(x, s, cause, f | (cause != 0 ? 0x80000 : 0));
        if (card_is_attribute9(id) == 0) {
            duel_088AFD5C(0x1B, x);
        }
    }
    duel_088CFBA4(s, p, z, f | (to_grave ? 0x1400000 : 0));
    return 1;
}
#endif

#ifdef NON_MATCHING /* forma igual; o original guarda o contador externo na pilha (alocação diferente) */
/* Applies action a (kind: pile the cards go to, flags) to the zones of mask: marks them and the cards
   that leave with them, then sends each one, turn player first. Returns the zones affected. */
unsigned int duel_act_apply(DuelAct *a, unsigned int mask, int kind, unsigned int flags)
{
    unsigned int old = duel.zone_off;
    unsigned int hit = duel_act_hits_mask(a, mask, kind, (flags & 2) != 0);
    unsigned int kept = 0;
    unsigned int bit;
    unsigned int p;
    int n;
    int dest;
    int z;

    if (flags & 2) {                   /* destroyed: substitutes and 0x1513 keep their card */
        for (n = 0; n < 2; n++) {
            p = duel_turn_player ^ n;
            for (z = 0; z < 5; z++) {
                bit = 1 << (z + p * 16);
                if ((hit & bit) && duel_slot_affected(p, z, 0x1825)) {
                    kept |= bit;
                }
            }
            if (a->id != 0) {
                for (z = 5; z < 11; z++) {
                    bit = 1 << (z + p * 16);
                    if ((hit & bit) && duel_slot_has_effect(p, z, 0x1513)) {
                        kept |= bit;
                    }
                }
            }
        }
    }
    for (n = 0; n < 2; n++) {
        p = duel_turn_player ^ n;
        for (z = 0; z < 11; z++) {
            if (hit & 1 << (z + p * 16)) {
                duel_leave_mark(p, z, flags, a->id, p != a->player);
            }
        }
    }
    if (duel.zone_off != 0) {
        duel_0886BCDC();
        duel.unk1f60 = duel_dfc10();
        duel_088C7BA4();
        duel.zone_off = old | (hit ^ kept);
        for (n = 0; n < 2; n++) {
            p = duel_turn_player ^ n;
            for (z = 0; z < 11; z++) {
                bit = 1 << (z + p * 16);
                if (hit & bit) {
                    dest = kind;
                    if (card_attribute_icon_is3(SLOT_AT(p, z)->card.id)) {   /* extra deck monsters */
                        if (kind == 0xB) {
                            dest = 0xC;
                        } else if (kind == 0xD) {
                            dest = 0xC;
                        }
                    }
                    duel.zone_off &= ~bit;
                    duel_send_card(p, z, dest, flags + (p != (a->player ^ a->opp)) + 0x10000, a->id);
                }
            }
        }
        duel_088C7CF0();
    }
    return duel.zone_off ^ old;
}
#endif
