// @range 0x088482F8-0x0884ACD0
// @asm 0x088482F8-0x0884ACD0
/* Battle result: compares the ATK/DEF of the attacker (duel_special.player[0]) and of the attacked
   monster, records the damage of each player (duel_special.recs) and which monster is destroyed,
   then applies the cards that change the outcome. show: send the cards' animations to the host;
   check: only a prediction (no change that must happen once). */
#include "duel.h"

typedef struct DuelFighter {           /* duel_special.act[p] seen as a monster in battle */
    int player;
    int zone;                          /* +0x04 */
    int defending;                     /* +0x08 the defense position counts */
    int uid;                           /* +0x0C */
    int id;                            /* +0x10 */
    int w14;
    int w18;
    int atk;                           /* +0x1C */
    int def;                           /* +0x20 */
    int type;                          /* +0x24 card_get_type of the monster */
    int level;                         /* +0x28 */
    int destroyed;                     /* +0x2C destroyed by the battle */
    int w30;                           /* +0x30 set by the cards that win the battle without destroying */
    int w34;                           /* +0x34 2: banished, 1: other effect when destroyed [?] */
} DuelFighter;

#define FIGHTER(p) ((DuelFighter *)&duel_special.act[p])
#define DAMAGE duel_special.recs        /* .w0 damage, .a[0]/.a[1] card that deals it, .b[0] player damaged */

extern unsigned int duel_atk_tp;       /* 0x08913F14 player attacked */
extern int duel_atk_direct;            /* 0x08913F18 */
extern int duel_atk_zone;              /* 0x08913F2C */
extern int duel_atk_tz;                /* 0x08913F30 */
extern unsigned int duel_turn_player;
extern int duel_phase;

unsigned int duel_slot_ref(unsigned int p, int z);
void duel_088480D8(int *a, int *b, int c);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_slot_e12e0(unsigned int p, int z, int id, unsigned int q);
int duel_count_active(int id);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int duel_slot_e8170(unsigned int p, int z, int id);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
int duel_slot_effect_kind(unsigned int p, int z, int id, int kind);
int duel_is_cur_card(int id);
int duel_field_e5ae0(unsigned int p, int z);
int duel_count_monsters(unsigned int p);
int duel_slot_affected(unsigned int p, int z, int id);
int duel_count_id_flag18(unsigned int p, int id);
int card_is_in_group_1a6x(int id);
int duel_field_base_atk(unsigned int p, int z);
unsigned short duel_slot_e1854(unsigned int p, int z);
int card_get_special_group(unsigned short id);
unsigned int duel_field_attrs(unsigned int p, int z);
int duel_field_attr_is(unsigned int p, int z, int attr);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
int duel_count_type_skip(unsigned int p, unsigned int type, int skip);
int duel_slot_e5a48(unsigned int p, int z);
int duel_count_attr(unsigned int p, int attr);
int duel_slot_counter_1a90(unsigned int p, int z);
int card_is_in_group_158d(int id);
int duel_slot_affected_ex(unsigned int p, int z, int id);
int card_is_attribute8(int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
void duel_088CC594(unsigned int p, int z, int a, int id);
void duel_0884DAF0(int a, int b);

#ifdef NON_MATCHING /* 10,7 KB escritos a partir do pseudocódigo e conferidos no asm; ordem dos blocos não reproduzida */

/* effects of the monster `me` (not negated, player mp, zone mz) on the battle against `foe` */
static void duel_battle_card(DuelFighter *me, DuelFighter *foe, unsigned int mp, int mz, unsigned int fp, int fz,
                             int show, int check, int doubled, int attacker)
{
    int i;
    DuelSpecRec *r;

    switch (me->id) {
    case 0x1955:
        if (duel_count_monsters(1 - mp) == 1) {
            me->destroyed = 0;
        }
        break;
    case 0x1949:
        if (me->defending == 0 && duel.players[mp & 1].n_hand == 0) {
            me->destroyed = 0;
        }
        break;
    case 0x17E4:
    case 0x1827:
    case 0x1861:
        if (duel_count_attr(0, 5) || duel_count_attr(1, 5)) {
            foe->w34 = duel_slot_e5a48(fp, fz);
        }
        break;
    case 0x18AC:
        if (mp == fp) {
            foe->w34 = 0;
        } else {
            foe->w34 = duel_slot_e5a48(fp, fz) != 0;
        }
        break;
    case 0x182B:
        foe->w34 = card_get_special_group(foe->id) == 1;
        break;
    case 0x14B6:
    case 0x1622:
    case 0x164D:
    case 0x1691:
        foe->w34 = duel_slot_e5a48(fp, fz);
        break;
    case 0x14BC:
        if (attacker && duel_count_type_skip(mp, 3, mz)) {
            foe->w34 = card_get_special_group(foe->id) == 1;
        }
        break;
    case 0x1439:
        if (duel.players[mp & 1].field[mz].pos == 0) {
            me->destroyed = 0;
        }
        if (!attacker && show) {
            duel_088C4448(mp, me->zone, me->uid);
            duel_088CC594(fp, fz, 1, 0x1439);
        }
        break;
    case 0x1743:
        if (duel.players[mp & 1].field[mz].pos == 0) {
            me->destroyed = 0;
            if (show) {
                duel_088C4448(mp, me->zone, me->uid);
                duel_088C87A8(fp, fz, (mp & 0xFF) | (mz & 0xFF) << 8, 6, 0);
            }
        }
        break;
    case 0x1500:
    case 0x168D: /* halves the damage it deals */
        for (i = 0; i < 2; i++) {
            r = &DAMAGE[i];
            if (r->a[0] == mp && r->a[1] == me->zone && r->b[0] == 1 - mp) {
                r->w0 = (r->w0 + 1) >> 1;
            }
        }
        break;
    case 0x1292: /* doubles the damage it takes */
        if (!attacker && !doubled) {
            for (i = 0; i < 2; i++) {
                if (DAMAGE[i].b[0] == 1 - mp) {
                    DAMAGE[i].w0 <<= 1;
                }
            }
        }
        break;
    case 0x14E9:
        if (me->atk == foe->atk) {
            me->destroyed = 0;
        }
        break;
    case 0x14AF:
    case 0x1642:
    case 0x18B8: /* no damage for its controller */
        for (i = 0; i < 2; i++) {
            if (DAMAGE[i].b[0] == mp) {
                DAMAGE[i].w0 = 0;
            }
        }
        break;
    case 0x1441: /* not destroyed and no damage for its controller */
        me->destroyed = 0;
        for (i = 0; i < 2; i++) {
            if (DAMAGE[i].b[0] == mp) {
                DAMAGE[i].w0 = 0;
            }
        }
        break;
    case 0x1140:
        if (attacker && duel_field_attr_is(fp, fz, 2) && show) {
            duel_088C87A8(fp, fz, me->id, 1, 0);
        }
        break;
    case 0x1AAD:
    case 0x1437:
    case 0x1596:
    case 0x1598:
    case 0x1645:
    case 0x1770:
    case 0x1855:
    case 0x19FF:
    case 0x1A00: /* not destroyed by battle */
        me->destroyed = 0;
        break;
    case 0x1837:
    case 0x1913:
    case 0x1962:
    case 0x19BF:
        if (!check) {
            me->destroyed = 0;
        }
        break;
    case 0x157E:
        if (duel_field_attrs(fp, fz) & 0x7C) {
            me->destroyed = 0;
        }
        break;
    case 0x1570:
        if (duel_count_monsters(me->player) == 1) {
            foe->w34 = card_get_special_group(foe->id) == 1;
        }
        break;
    case 0x14AC:
        if ((short)duel_slot_e1854(mp, mz) != -1) {
            me->destroyed = 0;
        }
        break;
    case 0x147C:
        if (foe->atk > 0x76B) {
            me->destroyed = 0;
        }
        break;
    case 0x13CB:
        if (duel_turn_player == mp && duel_phase == 3) {
            me->destroyed = 0;
            for (i = 0; i < 2; i++) {
                if (DAMAGE[i].b[0] == mp && DAMAGE[i].b[1] == me->zone) {
                    DAMAGE[i].w0 = 0;
                }
            }
        }
        break;
    case 0x12AC:
        if (foe->level < 8) {
            me->destroyed = 0;
        }
        break;
    }
}

/* cards after the battle on monster `me` (player mp, zone mz) against `foe` */
static void duel_battle_after(DuelFighter *me, DuelFighter *foe, unsigned int mp, int mz, unsigned int fp, int fz,
                              int show, int check, int attacker)
{
    switch (me->id) {
    case 0x1A93:
        if (attacker && show && foe->destroyed && duel_slot_counter_1a90(fp, fz)) {
            duel_088C87A8(mp, mz, me->id, 4, 0);
        }
        break;
    case 0x1950:
        if (show && me->destroyed && foe->type == 4) {
            duel_088C77D4(me->uid & 1, 0x1950, 4, me->uid);
        }
        break;
    case 0x17D6:
        if (show && me->destroyed && duel_slot_has_effect(mp, mz, 0x17D5)) {
            duel_088C77D4(me->uid & 1, me->id, 4, me->uid);
        }
        break;
    case 0x1441:
    case 0x1700:
    case 0x19A6:
        if (mp == fp) {
            break;
        }
    case 0x1703: /* the other monster gets w30 */
        if (check) {
            foe->w30 = 1;
        }
        if (show) {
            duel_088C87A8(fp, fz, me->id, 4, 0);
        }
        break;
    case 0x170E:
        if (foe->type == 0x12 || foe->type == 0xF) {
            if (check) {
                foe->w30 = 1;
            }
            if (show) {
                duel_088C87A8(fp, fz, me->id, 4, 0);
            }
        }
        break;
    case 0x170D:
        if (foe->type == 2 || foe->type == 3) {
            if (check) {
                foe->w30 = 1;
            }
            if (show) {
                duel_088C87A8(fp, fz, me->id, 4, 0);
            }
        }
        break;
    }
}

/* destruction of monster `me` cancelled by the field */
static void duel_battle_keep(DuelFighter *me, unsigned int mp, int mz, unsigned int ap, int show, int check,
                             unsigned int neg, int defender)
{
    int i;
    DuelSpecRec *r;
    unsigned int e;
    DuelEffectBits *ef;
    unsigned int src;
    unsigned int q;
    int sz;
    int id;
    int hit;

    for (i = 0; i < 2; i++) {
        r = &DAMAGE[i];
        if (r->a[0] == mp && r->a[1] == me->zone && r->w0 != 0) {
            if (duel_slot_e12e0(mp, me->zone, 0x1663, 1 - r->b[0])) {
                r->w0 = 1000;
            }
            if (r->b[0] != mp && duel_slot_has_effect(mp, me->zone, 0x1890)) {
                r->w0 = 0;
            }
        }
    }
    if (mp != ap) {
        if (duel.players[mp & 1].flags >> 23 & 1) {
            me->destroyed = 0;
        }
        if (duel_slot_has_effect(mp, 0xB, 0x1805)) {
            me->destroyed = 0;
        }
        if (duel_slot_effect_kind(mp, 0xB, 0x19B8, 1)) {
            me->destroyed = 0;
        }
    }
    if (me->destroyed && card_is_in_group_158d(me->id) && duel_slot_has_effect(mp, 0xB, 0x1594)) {
        me->destroyed = 0;
    }
    if (duel_slot_affected_ex(mp, mz, 0x150A)) {
        me->destroyed = 0;
    }
    if (duel_slot_affected_ex(mp, mz, 0x17FF)) {
        me->destroyed = 0;
    }
    if (duel_slot_affected_ex(mp, mz, 0x1992)) {
        me->destroyed = 0;
    }
    if (duel_slot_affected_ex(mp, mz, 0x1A18)) {
        me->destroyed = 0;
    }
    if (duel_slot_affected(mp, mz, 0x1957)) {
        me->destroyed = 0;
    }
    if (card_is_in_group_1a6x(me->id) && duel_count_id_flag18(mp, 0x1A05)) {
        me->destroyed = 0;
    }
    if (defender) {
        if (duel_slot_has_effect(mp, mz, 0x1AAC)) {
            me->destroyed = 0;
        }
        if (duel_slot_has_effect(mp, mz, 0x1B2C)) {
            me->destroyed = 0;
        }
    }
    if (!check && me->destroyed) {
        /* equip cards and others attached to the monster that keep it on the field (0x15B3) */
        for (e = duel.players[mp & 1].field[mz].effects; e != 0;) {
            ef = (DuelEffectBits *)&duel.effects[e];
            e = ef->next;
            if (ef->kind < 6) {
                continue;
            }
            hit = 0;
            src = ef->src;
            sz = src >> 8;
            q = src & 1;
            id = duel.players[q].field[sz].card.id;
            if (ef->kind == 0xD && !neg) {
                hit = 1;
            } else if (card_is_attribute8(id) && (unsigned int)(ef->kind - 10) < 2 &&
                       SLOT_ACTIVE(&duel.players[q].field[sz])) {
                hit = 1;
            } else if (id == 0x1989 && SLOT_ACTIVE(&duel.players[q].field[sz])) {
                hit = 1;
            }
            if (hit) {
                me->destroyed = 0;
                if (show) {
                    duel_088C87A8(src & 0xFF, sz, 0x15B3, 1, 0);
                }
            }
        }
    }
}

/* the monster `me` was destroyed by 0x18F1 unless 0xFB6 protects it */
static void duel_battle_18f1(DuelFighter *me, unsigned int mp, int mz, int show)
{
    if (show) {
        if (!duel_slot_has_effect(mp, mz, 0xFB6)) {
            me->destroyed = 0;
            duel_088C4448(mp, me->zone, me->uid);
            duel_088C87A8(mp, mz, 0xFB6, 2, 0);
            duel_088C87A8(mp, mz, 0x18F1, 4, 0);
        }
    } else if (duel_slot_has_effect(mp, mz, 0xFB6)) {
        me->destroyed = 0;
    }
}

void duel_088482F8(int show, int check)
{
    unsigned int tp = duel_atk_tp;
    unsigned int p = duel_special.player[0];
    DuelFighter *A = FIGHTER(p);
    int q = 1 - p;
    DuelFighter *D = FIGHTER(q);
    unsigned int a_neg = duel.players[p & 1].field[duel_atk_zone].flags >> 5 & 1;
    unsigned int d_neg;
    unsigned int ref;
    int doubled;
    int av;
    int dv;
    int diff;
    int bypass;
    int i;
    DuelSpecRec *r;

    if (duel_atk_direct == 0) {
        d_neg = duel.players[tp & 1].field[duel_atk_tz].flags >> 5 & 1;
    } else {
        d_neg = 0;
    }
    ref = duel_slot_ref(p, duel_atk_zone);
    A->destroyed = 0;
    A->w30 = 0;
    A->w34 = 0;
    DAMAGE[0].w0 = 0;
    D->destroyed = 0;
    D->w30 = 0;
    D->w34 = 0;
    doubled = 0;
    DAMAGE[1].w0 = 0;

    /* 1. compare ATK with the ATK or DEF of the target */
    if (duel_atk_direct == 0) {
        av = A->atk;
        dv = D->defending ? D->def : D->atk;
        if (av == dv) {
            if (D->defending == 0 && av > 0) {
                A->destroyed = 1;
                D->destroyed = 1;
            }
        } else if (av > dv) {
            diff = av - dv;
            if (D->defending == 0) {
                duel_088480D8((int *)A, (int *)D, diff);
            } else {
                /* piercing damage against a defending monster */
                bypass = 0;
                if (a_neg == 0) {
                    switch (A->id) {
                    case 0x1A1D:
                        bypass = duel.players[p & 1].field[duel_atk_zone].link == 0x191C;
                        break;
                    case 0x154C:
                    case 0x196B:
                    case 0x196C:
                    case 0x196D:
                    case 0x196E:
                        bypass = duel_slot_has_effect(p, duel_atk_zone, A->id);
                        break;
                    case 0x1B2B:
                    case 0x139F:
                    case 0x1416:
                    case 0x14D6:
                    case 0x1588:
                    case 0x1651:
                    case 0x168B:
                    case 0x16FF:
                    case 0x1728:
                    case 0x18AB:
                    case 0x1835:
                    case 0x18FD:
                    case 0x194D:
                    case 0x1991:
                    case 0x19C9:
                    case 0x1A21:
                    case 0x1A53:
                    case 0x1AAB:
                        bypass = 1;
                        break;
                    }
                }
                if (!(ref & 1) && duel_slot_has_effect(p, 0xB, 0x15F2)) {
                    bypass = 1;
                }
                if (A->type == 1) {
                    if (!(ref & 1) && duel_find_effect_card(p, 0x14E3, -1)) {
                        bypass = 1;
                    }
                } else if (A->type == 0x10 || A->type == 0xC || A->type == 0xB) {
                    if (duel_find_effect_card(p, 0x16FC, -1)) {
                        bypass = 1;
                    }
                }
                if ((A->id == 0x147B || A->id == 0xFCC || A->id == 0xFCB) && duel_find_effect_card(p, 0x187D, -1)) {
                    bypass = 1;
                }
                if (bypass || duel_slot_e12e0(p, duel_atk_zone, 0x1408, p) ||
                    duel_slot_e12e0(p, duel_atk_zone, 0x1625, p) || duel_slot_e12e0(p, duel_atk_zone, 0x1496, p) ||
                    duel_slot_e12e0(p, duel_atk_zone, 0x15CE, p) || duel_slot_e12e0(p, duel_atk_zone, 0x1620, p) ||
                    duel_slot_has_effect(p, duel_atk_zone, 0x1A0F)) {
                    if (p == tp) {
                        duel_088480D8((int *)A, 0, diff);
                    } else {
                        duel_088480D8((int *)A, (int *)D, diff);
                    }
                }
                if (duel_slot_e12e0(p, duel_atk_zone, 0x1408, 1 - p) ||
                    duel_slot_e12e0(p, duel_atk_zone, 0x1625, 1 - p) ||
                    duel_slot_e12e0(p, duel_atk_zone, 0x1496, 1 - p) ||
                    duel_slot_e12e0(p, duel_atk_zone, 0x15CE, 1 - p) ||
                    duel_slot_e12e0(p, duel_atk_zone, 0x1620, 1 - p)) {
                    duel_088480D8((int *)A, (int *)A, diff);
                }
            }
            D->destroyed = 1;
        } else {
            /* the attacker loses: damage to its controller (doubled by 0x1883, 0x19F2) */
            diff = dv - av;
            if (D->defending != 0 && duel_slot_has_effect(1 - p, 0xB, 0x1883)) {
                diff *= 2;
                doubled = 1;
            } else if (D->defending != 0 && D->type == 6 && duel_count_active(0x19F2)) {
                diff *= 2;
                doubled = 1;
            }
            duel_088480D8((int *)D, (int *)A, diff);
            if (D->defending == 0) {
                A->destroyed = 1;
            } else if (show) {
                if (duel_slot_has_effect(1 - p, 0xB, 0x1493)) {
                    duel_088C87A8(p, duel_atk_zone, 0x1493, 4, 0);
                }
                if (duel_slot_has_effect(1 - p, 0xB, 0x1883)) {
                    duel_088C87A8(p, duel_atk_zone, 0x1883, 4, 0);
                }
                if (duel_find_effect_card(1 - p, 0x162E, -1)) {
                    duel_088C87A8(p, duel_atk_zone, 0x162E, 4, 0);
                }
            }
        }
    } else {
        duel_088480D8((int *)A, 0, A->atk);
    }

    /* 2. damage cancelled or kept by the cards of each player */
    for (i = 0; i < 2; i++) {
        r = &DAMAGE[i];
        if (r->w0 == 0) {
            continue;
        }
        if (r->a[0] != r->b[0]) {
            if (duel_slot_e8170(r->a[0], r->a[1], 0x1589)) { /* the damage is redirected */
                if (show) {
                    duel_088C77D4(r->a[0], 0x1589, 4, MIN((unsigned int)r->w0, 0xFFFF));
                }
                if (check == 0) {
                    r->w0 = 0;
                }
                continue;
            }
            if (duel.players[r->b[0] & 1].flags >> 23 & 1) {
                r->w0 = 0;
                continue;
            }
            if (duel_slot_effect_kind(r->b[0], 0xB, 0x19B8, 1)) {
                r->w0 = 0;
                continue;
            }
        }
        if (r->a[0] == p && duel_is_cur_card(0x10F4) && duel_find_effect_card(r->b[0], 0x13F7, -1)) {
            r->w0 = 0;
            continue;
        }
        if (duel_count_active(0x175E)) {
            if (r->b[0] == p && A->type == 0x11) {
                r->w0 = 0;
                continue;
            }
            if (duel_atk_direct == 0 && r->b[0] == tp && D->type == 0x11) {
                r->w0 = 0;
                continue;
            }
        }
        if (duel_find_effect_card(r->b[0], 0x179D, -1)) {
            if (duel_field_e5ae0(p, A->zone) && A->level <= 3) {
                r->w0 = 0;
                continue;
            }
            if (duel_atk_direct == 0 && duel_field_e5ae0(tp, D->zone) && D->level <= 3) {
                r->w0 = 0;
                continue;
            }
        }
        if (duel_slot_has_effect(r->b[0], 0xB, 0x18AA)) {
            r->w0 = 0;
            continue;
        }
        if (duel_count_monsters(r->b[0]) && duel_find_effect_card(r->b[0], 0x17FE, -1)) {
            r->w0 = 0;
            continue;
        }
        if (duel_atk_direct == 0) {
            if (r->b[0] == p && duel_slot_affected(p, duel_atk_zone, 0x1989)) {
                r->w0 = 0;
                continue;
            }
            if (r->b[0] == tp && duel_slot_affected(tp, duel_atk_tz, 0x1989)) {
                r->w0 = 0;
                continue;
            }
        }
        if (duel_count_id_flag18(r->b[0], 0x1A05)) {
            if (r->b[0] == p && card_is_in_group_1a6x(A->id)) {
                r->w0 = 0;
                continue;
            }
            if (r->b[0] == tp && card_is_in_group_1a6x(D->id)) {
                r->w0 = 0;
                continue;
            }
        }
        if (duel_slot_e8170(r->b[0], 10, 0x1A0C) && duel.players[r->b[0] & 1].unk4 > 3) {
            r->w0 = 0;
            continue;
        }
        if (r->a[0] != r->b[0] && duel_slot_has_effect(r->b[0], 0xB, 0x1805)) {
            r->w0 = 0;
        }
    }
    if (duel_slot_effect_kind(p, 0xB, 0x151E, 4)) {
        DAMAGE[1].w0 = 0;
        DAMAGE[0].w0 = 0;
    }
    if (A->atk == D->atk) {
        if (duel_find_effect_card(p, 0x15EC, -1)) {
            A->destroyed = 0;
        }
        if (duel_find_effect_card(tp, 0x15EC, -1)) {
            D->destroyed = 0;
        }
    }

    /* 3. direct attack: damage changed by the attacker */
    if (duel_atk_direct != 0) {
        if (a_neg == 0) {
            switch (A->id) {
            case 0x1750:
                DAMAGE[0].w0 <<= 1;
                break;
            case 0x1500:
            case 0x168D:
                DAMAGE[0].w0 = (DAMAGE[0].w0 + 1) >> 1;
                break;
            }
        }
        if (DAMAGE[0].w0 != 0 && duel_slot_has_effect(p, duel_atk_zone, 0x16A3)) {
            DAMAGE[0].w0 = 400;
        }
        if (DAMAGE[0].w0 != 0 && duel_slot_e12e0(p, duel_atk_zone, 0x1663, p)) {
            DAMAGE[0].w0 = 1000;
        }
        if (DAMAGE[0].w0 != 0 && duel_slot_has_effect(p, 0xB, 0x1A41)) {
            DAMAGE[0].w0 = duel_field_base_atk(p, A->zone);
        }
        if (DAMAGE[0].w0 != 0 && duel_slot_has_effect(p, duel_atk_zone, 0x1890)) {
            DAMAGE[0].w0 = 0;
        }
    } else {
        /* 4. effects of the attacker, then of the attacked monster */
        if (a_neg == 0) {
            duel_battle_card(A, D, p, duel_atk_zone, tp, duel_atk_tz, show, check, doubled, 1);
        }
        duel_battle_after(A, D, p, duel_atk_zone, tp, duel_atk_tz, show, check, 1);
        duel_battle_keep(A, p, duel_atk_zone, tp, show, check, a_neg, 0);
        if (duel_slot_e5a48(tp, duel_atk_tz)) {
            if (duel_slot_effect_with(p, duel_atk_zone, 0x15FF, 1)) {
                D->w34 = 1;
                if (show) {
                    duel_088C77D4(p, 0x15FF, 4, D->uid);
                }
            }
            if (duel_slot_e12e0(p, duel_atk_zone, 0x165F, tp ^ 1)) {
                D->w34 = 1;
            }
            if (A->type == 3 && duel_find_effect_card(p, 0x14B5, -1)) {
                D->w34 = (duel_find_effect_card(p, 0x14B5, duel_atk_zone) > 0) + 1;
            }
        }
        if (show && duel_slot_has_effect(p, duel_atk_zone, 0x18CD)) {
            duel_088C87A8(tp, duel_atk_tz, 0x18CD, 4, 0);
        }
        if (d_neg == 0) {
            duel_battle_card(D, A, tp, duel_atk_tz, p, duel_atk_zone, show, check, doubled, 0);
        }
        duel_battle_after(D, A, tp, duel_atk_tz, p, duel_atk_zone, show, check, 0);
        duel_battle_keep(D, tp, duel_atk_tz, p, show, check, d_neg, 1);
        if (duel_slot_e5a48(p, duel_atk_zone)) {
            if (duel_slot_e12e0(tp, duel_atk_tz, 0x165F, p ^ 1)) {
                A->w34 = 1;
            }
            if (D->type == 3 && duel_find_effect_card(tp, 0x14B5, -1)) {
                A->w34 = (duel_find_effect_card(tp, 0x14B5, duel_atk_tz) > 0) + 1;
            }
        }
        if (show && duel_slot_has_effect(tp, duel_atk_tz, 0x18CD)) {
            duel_088C87A8(p, duel_atk_zone, 0x18CD, 4, 0);
        }
        if (show) {
            if (duel_slot_affected(p, duel_atk_zone, 0x1392) && D->type == 1) {
                duel_088C87A8(tp, duel_atk_tz, 0x1392, 1, 0);
            }
            if (duel_slot_affected(tp, duel_atk_tz, 0x1392) && A->type == 1) {
                duel_088C87A8(p, duel_atk_zone, 0x1392, 1, 0);
            }
        }
    }

    /* 5. 0x18F1 survives its destruction once, unless 0xFB6 */
    if (duel_atk_direct == 0 && check == 0) {
        if (A->destroyed && a_neg == 0 && A->id == 0x18F1) {
            duel_battle_18f1(A, p, duel_atk_zone, show);
        }
        if (D->destroyed && d_neg == 0 && D->id == 0x18F1) {
            duel_battle_18f1(D, tp, duel_atk_tz, show);
        }
    }
    if (show && p != tp && A->destroyed && D->destroyed) {
        duel_0884DAF0(0x11, 1);
    }
    if (A->destroyed == 0) {
        A->w34 = 0;
    }
    if (D->destroyed == 0) {
        D->w34 = 0;
    }
    if (A->destroyed && D->destroyed) {
        if (A->w34 < 2) {
            A->w34 = 0;
        }
        if (D->w34 < 2) {
            D->w34 = 0;
        }
    }
    if ((unsigned int)DAMAGE[0].w0 > 0xFFFF) {
        DAMAGE[0].w0 = 0xFFFF;
    }
    if ((unsigned int)DAMAGE[1].w0 > 0xFFFF) {
        DAMAGE[1].w0 = 0xFFFF;
    }
}
#endif
