// @range 0x08839360-0x0883F0DC
// @asm 0x08839360-0x0883A2BC
// @asm 0x0883A2BC-0x0883AEF8
// @asm 0x0883B4A8-0x0883BB00
// @asm 0x0883BB00-0x0883D798
// @asm 0x0883D798-0x0883E63C
// @asm 0x0883E63C-0x0883F0DC
/* Battle: step machines of the damage step (duel_w_08919610) with the cards that act on the attacker and the target. */
#include "duel.h"

typedef struct AiSnap {                /* 0x44, see ai_08836868.c */
    int w[17];
} AiSnap;

extern int duel_w_0891960C;            /* battle step */
extern int duel_w_08919610;            /* sub-step */
extern unsigned int duel_turn_player;
extern unsigned int duel_atk_tp;       /* 0x08913F14 target player */
extern int duel_atk_direct;            /* 0x08913F18 */
extern int duel_atk_w24;               /* 0x08913F24 the battle was stopped [?] */
extern int duel_atk_zone;              /* 0x08913F2C attacker zone */
extern int duel_atk_tz;                /* 0x08913F30 target zone */
extern AiSnap duel_atk_snap;           /* 0x08913FFC */

void duel_088C7D74(int a, int b);
void duel_088C7DD4(unsigned int p, int a, int b);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int field_get_card(unsigned int p, int z);
void duel_08836ED0(AiSnap *s);
int duel_slot_effect_kind(unsigned int p, int z, int id, int kind);
void duel_088C44E4(unsigned int p, int id);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_effect_arg(unsigned int p, int z, int id);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
int duel_slot_faceup_live(unsigned int p, int z);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
int duel_08837250(void);
void duel_088C936C(unsigned int p, int a, int b, int id, int opp);
void duel_088C4508(unsigned int p, int a);
int duel_088480D0(unsigned int p);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
void duel_088C89BC(unsigned int p, int z, int id, int c);
int duel_field_attr_is(unsigned int p, int z, int attr);
int duel_field_level(unsigned int p, int z);
void duel_088AFD5C(int kind, unsigned int a);
void duel_0884ACD0(int a);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
int card_get_special_group(int id);
extern int duel_phase;
void duel_088C7BA4(void);
void duel_088C7CF0(void);
int duel_spec_act_ok(DuelSpecAct *s);
void duel_088C4864(DuelSpecRec *r);
int duel_088C4730(unsigned int p, int n, int f, int id);
int duel_088D1438(unsigned int p, int z, unsigned int q, unsigned short id);
int duel_088D1F1C(DuelAct *a, int i);
int duel_088C168C(void);
int duel_088C30D4(void);
unsigned int duel_field_type(unsigned int p, int z);
int duel_0886B0E4(unsigned int a, unsigned short uid, int c);
int duel_card_protected(int id, int uid);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_slot_value_1909(unsigned int p, int z);
void duel_088CC1D0(unsigned int p, int z, int a);
int card_is_monster(int id);
int card_is_attribute9(int id);
int duel_card_check_9148(unsigned int p, int id);
int duel_088C1D80(unsigned int p);
int duel_088DFCC8(unsigned int uid);
int duel_count_active(int id);
void duel_088D1DF4(DuelCard *c, struct DuelActX x);
int duel_special_has_effect(int p, int z, int id);
void duel_088C78E0(unsigned int p);
extern int duel_w_08919614;
extern int duel_atk_w28;               /* 0x08913F28 */
int duel_08843844(void);
int duel_attack_check(unsigned int p, int z, int arg);
int duel_host_call(int a, int b, unsigned int c, int d);
int duel_088C9878(unsigned int p, int z, int x, int y, int id, int opp);
int duel_slot_counter_1439(unsigned int p, int z);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_slot_e68a0(unsigned int p, int z);
void duel_088CC284(unsigned int p, int z, int n);
int duel_pick_st_zone(unsigned int p);
unsigned int duel_card_locate(unsigned int uid);
void duel_088CB728(unsigned int p, int z, int uid, unsigned int q, int qz, int a, int b);
void duel_088C8B78(unsigned int p, unsigned short loc, unsigned short t, unsigned short k);
unsigned int duel_field_flag9(unsigned int p, int z);
unsigned int duel_088D13D8(unsigned int p, unsigned short id, unsigned int mask);

#define ATK_SLOT(p, z) (&duel.players[(p) & 1].field[(z)])
/* event word of duel_0884F30C: card id, player bit 31, zone bits 16-20 */
#define BATTLE_EV(id, p, z, k) ((unsigned short)(id) | (((p) << 31 & 0x80000000) | (k) | ((z) << 16 & 0x1F0000)))

#ifdef NON_MATCHING /* forma próxima; diferem a ordem do cálculo de loc e a escolha de registradores */
/* p: attacking player; returns 1 when the damage step is over */
int duel_08839360(unsigned int p)
{
    unsigned int tp = duel_atk_tp;
    unsigned int loc;                  /* attacker | attacker zone << 8 | (target | target zone << 8) << 16 */
    int aid;                           /* attacker card */
    int tid;                           /* target card */
    int id;
    int r;
    int v;
    unsigned int q;

    loc = (unsigned short)((duel_special.player[0] & 0xFF) | (duel_atk_zone & 0xFF) << 8) |
          (unsigned short)((tp & 0xFF) | (unsigned char)duel_atk_tz << 8) << 16;
    aid = field_get_card(duel_special.player[0], duel_atk_zone);
    tid = field_get_card(duel_atk_tp, duel_atk_tz);
    switch (duel_w_08919610) {
    case 0: /* announce the battle */
        duel_088C7D74(0, 1);
        duel_088C7DD4(p, 19, 1);
        duel_cmd((duel_special.player[0] ? 0x8000 : 0) | 0x1B, duel_atk_zone, 1, 0);
        duel_special.unk24 = 0;
        duel_w_08919610++;
        return 0;
    case 1: /* effects at the start of the damage step */
        duel_08836ED0(&duel_atk_snap);
        if (duel_slot_effect_kind(0, 0xB, 0x19B8, 1) != 0 || duel_slot_effect_kind(1, 0xB, 0x19B8, 1) != 0) {
            duel_088C44E4(p, 0x19B8);
        }
        if (duel_slot_has_effect(p, 0xB, 0x15FF) != 0) {
            q = 1 - p;
            if (duel_slot_effect_arg(q, 0xB, 0x15FF) == -1) {
                duel_088C77D4(q, 0x15FF, 1, !duel_atk_direct);
            }
            if (duel_atk_direct == 0) {
                duel_088C87A8(tp, duel_atk_tz, 0x15FF, 1, 0);
            }
        }
        if (SLOT_ACTIVE(ATK_SLOT(duel_special.player[0], duel_atk_zone))) {
            switch (aid) {
            case 0x14D6:
            case 0x1993:
                duel_088C87A8(duel_special.player[0], duel_atk_zone, 0x14D6, 4, 0);
                break;
            case 0x1419:
            case 0x1566:
            case 0x15D2:
            case 0x1912:
            case 0x1915:
            case 0x1A61:
                duel_088C87A8(duel_special.player[0], duel_atk_zone, 0x14D6, 2, 0);
                break;
            case 0x1505:
            case 0x1644:
            case 0x1958:
            case 0x1A4E:
            case 0x1A79:
                if (duel_slot_effect_arg(p, duel_atk_zone, 0x1505) == -1) {
                    duel_088C87A8(p, duel_atk_zone, 0x1505, 1, !duel_atk_direct);
                }
                if (duel_atk_direct == 0) {
                    duel_088C87A8(tp, duel_atk_tz, (p & 0xFF) | (unsigned char)duel_atk_zone << 8, 8, 0);
                }
                break;
            }
        }
        if (duel_atk_w24 == 0 && duel_slot_faceup_live(duel_special.player[0], duel_atk_zone) != 0) {
            switch (aid) {
            case 0x1524:
            case 0x15E0:
            case 0x17D7:
            case 0x17D8:
            case 0x1823:
            case 0x17DF:
            case 0x1829:
            case 0x1963:
            case 0x19C2:
                if (duel_atk_direct == 0) {
                    duel_0884F30C(BATTLE_EV(aid, duel_special.player[0], duel_atk_zone, 0x28200000),
                                  CARD_UID(ATK_SLOT(duel_special.player[0], duel_atk_zone)->card), loc);
                }
                break;
            }
        }
        if (duel_atk_direct == 0 && duel_atk_w24 == 0 && duel_slot_faceup_live(duel_atk_tp, duel_atk_tz) != 0) {
            switch (tid) {
            case 0x1829:
                duel_0884F30C(BATTLE_EV(tid, duel_atk_tp, duel_atk_tz, 0x200000),
                              CARD_UID(ATK_SLOT(duel_atk_tp, duel_atk_tz)->card), loc);
                break;
            }
        }
        duel_w_08919610++;
        return 0;
    case 2: /* flip the face-down target */
        if (duel_08837250() != 0) {
            duel_w_0891960C = 11;
            duel_w_08919610 = 0;
            return 0;
        }
        if (duel_atk_direct == 0) {
            if (ATK_SLOT(duel_atk_tp, duel_atk_tz)->faceup == 0) {
                duel_088C936C(duel_atk_tp, duel_atk_tz, 0, 0, 0);
                duel_088C4508(duel_atk_tp, ATK_SLOT(duel_atk_tp, duel_atk_tz)->card.id);
                duel_special.unk24 = ATK_SLOT(duel_atk_tp, duel_atk_tz)->card.id;
            }
            switch (tid) {
            case 0x129C:
                duel_088C87A8(duel_atk_tp, duel_atk_tz, tid, 4, 0);
                break;
            }
        }
        duel_w_08919610++;
        return 0;
    case 3: /* damage calculation: cards of the attacker, then of the target */
        duel_088480D0(0);
        if (SLOT_ACTIVE(ATK_SLOT(p, duel_atk_zone))) {
            id = field_get_card(p, duel_atk_zone);
            r = -1;
            v = 0;
            switch (id) {
            case 0x1184:
                r = !duel_atk_direct && duel_field_attr_is(tp, duel_atk_tz, 6) != 0;
                break;
            case 0x1397:
                r = !duel_atk_direct && duel_field_attr_is(tp, duel_atk_tz, 2) != 0;
                break;
            case 0x1507:
            case 0x1952:
                r = duel_atk_direct;
                break;
            case 0x1508:
                r = !duel_atk_direct;
                break;
            case 0x17ED:
                r = !duel_atk_direct && duel_field_attr_is(tp, duel_atk_tz, 1) != 0;
                break;
            case 0x18F2:
                r = !duel_atk_direct && p != tp;
                v = 1;
                break;
            case 0x1A7A: /* 200 per level of the target */
                r = !duel_atk_direct && p != tp;
                v = duel_field_level(tp, duel_atk_tz) * 200;
                break;
            }
            if (r == 1) {
                duel_088C4448(p, duel_atk_zone, CARD_UID(ATK_SLOT(p, duel_atk_zone)->card));
                duel_088C87A8(p, duel_atk_zone, id, 4, v);
            } else if (r == 0) {
                duel_088C89BC(p, duel_atk_zone, id, 1);
            }
        }
        if (duel_atk_direct == 0 && SLOT_ACTIVE(ATK_SLOT(tp, duel_atk_tz))) {
            id = field_get_card(tp, duel_atk_tz);
            r = -1;
            v = 0;
            switch (id) {
            case 0x10C6:
            case 0x17ED:
                r = duel_field_attr_is(p, duel_atk_zone, 1);
                break;
            case 0x1397:
                r = duel_field_attr_is(p, duel_atk_zone, 2);
                break;
            case 0x1508:
                r = 1;
                break;
            case 0x1621:
                if (duel_turn_player != tp) {
                    duel_088C4448(tp, duel_atk_tz, CARD_UID(ATK_SLOT(tp, duel_atk_tz)->card));
                    duel_088C87A8(p, duel_atk_zone, id, 4, 0);
                }
                break;
            case 0x18F2:
                r = p != tp;
                break;
            case 0x1A7A:
                r = !duel_atk_direct && p != tp;
                v = duel_field_level(p, duel_atk_zone) * 200;
                break;
            }
            if (r == 1) {
                duel_088C4448(tp, duel_atk_tz, CARD_UID(ATK_SLOT(tp, duel_atk_tz)->card));
                duel_088C87A8(tp, duel_atk_tz, id, 4, v);
            } else if (r == 0) {
                duel_088C89BC(tp, duel_atk_tz, id, 1);
            }
        }
        if (duel_atk_w24 != 0) {
            return 1;
        }
        if (duel_atk_direct == 0 && duel_atk_tp != duel_special.player[0]) {
            if (duel_atk_w24 == 0 && duel_slot_faceup_live(duel_special.player[0], duel_atk_zone) != 0) {
                switch (aid) {
                case 0x1752:
                case 0x18F3:
                    duel_0884F30C(BATTLE_EV(aid, duel_special.player[0], duel_atk_zone, 0x28200000),
                                  CARD_UID(ATK_SLOT(duel_special.player[0], duel_atk_zone)->card), loc);
                    break;
                }
            }
            switch (tid) {
            case 0x1286:
            case 0x1370:
            case 0x1476:
            case 0x19BD:
                if (duel_special.unk24 == 0) { /* only when the target was flipped by the attack */
                    break;
                }
            case 0x148A:
                duel_0884F30C(BATTLE_EV(tid, duel_atk_tp, duel_atk_tz, 0x28200000),
                              CARD_UID(ATK_SLOT(duel_atk_tp, duel_atk_tz)->card), loc);
                break;
            }
        }
        duel_088AFD5C(20, loc);
        duel_w_08919610++;
        return 0;
    }
    return 1;
}
#endif

#ifdef NON_MATCHING /* switch por tabela (0x08907C3C); não comparado em detalhe */
/* p: attacking player; act[p] is the attacker, act[1 - p] the target */
int duel_0883A2BC(unsigned int p)
{
    unsigned int tp = duel_atk_tp;
    DuelSpecAct *a = &duel_special.act[p];
    DuelSpecAct *o = &duel_special.act[1 - p];
    unsigned short v[2];
    struct DuelActX x;
    int i;
    int j;
    int k;

    switch (duel_w_08919610) {
    case 0: /* attacker cards that act before the damage */
        if (duel_08837250() != 0) {
            duel_w_08919610 = 10;
            return 0;
        }
        duel_088480D0(0);
        switch (a->id) {
        case 0x1643:
        case 0x1983:
            duel_088C87A8(p, duel_atk_zone, a->id, 2, 0);
            break;
        case 0x1837:
        case 0x1913:
        case 0x1962:
        case 0x19BF:
            duel_088C87A8(p, duel_atk_zone, a->id, 4, 0);
            break;
        case 0x182C:
            if (duel_atk_direct == 0 && p != tp && SLOT_ACTIVE(ATK_SLOT(p, duel_atk_zone))) {
                duel_088C4448(p, duel_atk_zone, a->uid);
                duel_088C87A8(tp, duel_atk_tz, a->id, (unsigned char)((tp != duel_turn_player) + 2) << 4 | 2, 0);
            }
            break;
        case 0x19C7:
            duel_088C77D4(p, a->id, 4, a->uid);
            break;
        }
        if (duel_slot_has_effect(p, 0xB, 0x1512) != 0) {
            duel_088C87A8(p, duel_atk_zone, 0x1512, 2, 0);
        }
        duel_w_08919610++;
        return 0;
    case 1: /* target cards */
        if (duel_atk_direct == 0) {
            switch (o->id) {
            case 0x1643:
            case 0x1983:
                duel_088C87A8(tp, duel_atk_tz, o->id, 2, 0);
                break;
            case 0x1749:
                if (ATK_SLOT(tp, duel_atk_tz)->pos != 0) {
                    duel_088C87A8(p, duel_atk_zone, o->id, 1, 0);
                }
                break;
            case 0x1837:
            case 0x1913:
            case 0x1962:
            case 0x19BF:
                duel_088C87A8(tp, duel_atk_tz, o->id, 4, 0);
                break;
            case 0x182C:
                if (p != tp && SLOT_ACTIVE(ATK_SLOT(tp, duel_atk_tz))) {
                    duel_088C4448(tp, duel_atk_tz, o->uid);
                    duel_088C87A8(p, duel_atk_zone, o->id, (unsigned char)((p != duel_turn_player) + 2) << 4 | 2, 0);
                }
                break;
            case 0x19C7:
                duel_088C77D4(tp, o->id, 4, o->uid);
                break;
            }
            if (duel_slot_has_effect(p, 0xB, 0x1512) != 0) {
                duel_088C87A8(tp, duel_atk_tz, 0x1512, 2, 0);
            }
        }
        if (duel_atk_direct == 0 && o->id == 0x129A && o->unk8 == 0 && p != tp) {
            duel_0884F30C(BATTLE_EV(o->id, tp, duel_atk_tz, 0x200000), CARD_UID(ATK_SLOT(tp, duel_atk_tz)->card), 0);
        }
        duel_w_08919610++;
        return 0;
    case 2: /* show the battle, damage events */
        if (duel_08837250() != 0) {
            duel_w_0891960C = 11;
            duel_w_08919610 = 0;
            return 0;
        }
        duel_cmd((duel_special.player[0] ? 0x8000 : 0) | 0x16, duel_special.act[0].id, duel_special.act[1].id, 0);
        duel_088480D0(2);
        if (duel_atk_w24 == 0) {
            v[0] = a->pad14[2];
            v[1] = o->unk8 != 0 ? o->pad14[3] : o->pad14[2];
            if (duel_atk_direct == 0) {
                switch (a->id) {
                case 0x13AA:
                case 0x14CC:
                case 0x152D:
                    duel_0884F30C(BATTLE_EV(a->id, p, duel_atk_zone, 0x1A200000), a->uid, *(unsigned int *)v);
                    break;
                }
                switch (o->id) {
                case 0x13AA:
                case 0x14CC:
                    duel_0884F30C(BATTLE_EV(o->id, tp, duel_atk_tz, 0x1A200000), o->uid, *(unsigned int *)v);
                    break;
                }
            }
            duel_088AFD5C(13, *(unsigned int *)v);
        }
        duel_w_08919610++;
        return 0;
    case 3:
        if (duel_08837250() != 0) {
            duel_w_0891960C = 11;
            duel_w_08919610 = 0;
            return 0;
        }
        duel_088480D0(1);
        duel_w_08919610++;
        return 0;
    case 4:
        duel_0884ACD0(1);
        duel_w_08919610++;
        return 0;
    case 5: /* show the damage of each side */
        for (k = 0; k < 2; k++) {
            v[k] = 0;
            for (j = 0; j < 2; j++) {
                if (duel_special.recs[j].b[0] == duel_special.act[k].unk0 &&
                    duel_special.recs[j].b[1] == duel_special.act[k].unk4 && duel_special.recs[j].w0 != 0) {
                    v[k] = (unsigned int)duel_special.recs[j].w0 >= 0x10000 ? 0xFFFF : duel_special.recs[j].w0;
                }
            }
        }
        duel_cmd((duel_special.player[0] ? 0x8000 : 0) | 0x17, v[0], v[1],
                 (unsigned char)((duel_special.act[0].unk8 & 1) | (duel_special.act[0].pad14[6] & 1) << 1 | (v[0] > 0) << 2) |
                 (unsigned char)((duel_special.act[1].unk8 & 1) | (duel_special.act[1].pad14[6] & 1) << 1 | (v[1] > 0) << 2) << 8);
        duel_w_08919610++;
        return 0;
    case 6: /* destroyed cards */
        if (duel_special.act[duel_special.player[0]].pad14[6] != 0) {
            if (a->id == 0x1A95) {
                for (i = 0; i < 2; i++) {
                    for (j = 0; j <= 10; j++) {
                        if (duel_st_affects(i, j, duel_special.player[0], duel_atk_zone) != 0) {
                            duel_088C87A8(i, j, a->id, 4, a->uid);
                        }
                    }
                }
            }
            duel_cmd((duel_special.player[0] ? 0x8000 : 0) | 0x21, duel_atk_zone, 0, 0);
        }
        if (duel_special.act[1 - duel_special.player[0]].pad14[6] == 0) {
            return 1;
        }
        if (o->id == 0x1A95) {
            for (i = 0; i < 2; i++) {
                for (j = 0; j <= 10; j++) {
                    if (duel_st_affects(i, j, duel_atk_tp, duel_atk_tz) != 0) {
                        duel_088C87A8(i, j, o->id, 4, o->uid);
                    }
                }
            }
        }
        duel_cmd((duel_atk_tp ? 0x8000 : 0) | 0x21, duel_atk_tz, 1, 0);
        return 1;
    case 10: /* the target flipped by the attack */
        if (duel_special.unk24 != 0 && card_get_special_group(duel_special.unk24) != 0) {
            x.s_uid = o->uid;
            x.s_player = tp;
            x.s_zone = o->unk4;
            x.s_b15 = 1;
            x.s_b14 = o->unk8;
            x.s_b16 = 0;
            x.s_b17 = 0;
            duel_0884F30C(BATTLE_EV(o->id, tp, duel_atk_tz, 0x2A200000), o->uid, *(unsigned int *)&x);
        }
        duel_w_0891960C = 11;
        duel_w_08919610 = 0;
        return 0;
    }
    return 1;
}
#endif

typedef struct DuelDmgEv {             /* argument of the event 14 (battle damage) */
    unsigned short dmg;
    unsigned short p : 1;              /* player who takes it */
    unsigned short z : 4;
    unsigned short q : 1;              /* player who deals it */
    unsigned short qz : 4;
} DuelDmgEv;

/* clears a DuelAct byte by byte (inlined memset in the original) */
#define ACT_CLEAR(a)                                        \
    do {                                                    \
        unsigned char *d_ = (unsigned char *)(a);           \
        int n_ = sizeof(DuelAct);                           \
        if (d_ != 0) {                                      \
            do { *d_++ = 0; } while (--n_);                 \
        }                                                   \
    } while (0)

/* p: attacking player; damage events and the cards destroyed by the battle */
int duel_0883AEF8(unsigned int p)
{
    unsigned int tp = duel_atk_tp;
    DuelSpecAct *a = &duel_special.act[p];
    DuelSpecAct *o = &duel_special.act[1 - p];
    DuelDmgEv ev;
    int v;
    DuelAct x1;
    DuelAct x2;
    unsigned int q;
    int i;
    int n;

    switch (duel_w_08919610) {
    case 0: /* battle damage events */
        if (duel_atk_w24 == 0) {
            duel_088C7BA4();
            for (i = 0; i < 2; i++) {
                if ((v = duel_special.recs[i].w0) != 0) {
                    ev.dmg = (unsigned int)v > 0xFFFF ? 0xFFFF : v;
                    ev.p = duel_special.recs[i].a[0];
                    ev.z = duel_special.recs[i].a[1];
                    ev.q = duel_special.recs[i].b[0];
                    ev.qz = duel_special.recs[i].b[1];
                    duel_088AFD5C(14, *(unsigned int *)&ev);
                }
            }
            duel_088C7CF0();
        }
        duel_w_08919610++;
        return 0;
    case 1:
        duel_w_08919610++;
        return 0;
    case 2:
        duel_088C7BA4();
        switch (a->id) {
        case 0x12AC:
            if (duel_spec_act_ok(a) != 0) {
                duel_088C89BC(p, duel_atk_zone, 0x12AC, 1);
            }
            break;
        case 0x13CB:
            if (duel_atk_direct == 0 && duel_turn_player == p && duel_phase == 3 && o->pad14[6] == 0) {
                duel_0884F30C(BATTLE_EV(a->id, p, duel_atk_zone, 0x1E200000), a->uid, 0);
            }
            break;
        }
        /* the target was destroyed */
        if (o->pad14[6] != 0 && p != tp && duel_special.slot[1].card.id != 0) {
            switch (a->id) {
            case 0x13B1:
            case 0x16CB:
            case 0x17E3:
            case 0x1826:
            case 0x1861:
            case 0x19D3:
            case 0x19D4:
            case 0x1A93:
                duel_0884F30C(BATTLE_EV(a->id, p, duel_atk_zone, 0x1E200000), a->uid, 0);
                break;
            }
        }
        /* the attacker was destroyed */
        if (a->pad14[6] != 0 && p != tp && duel_special.slot[0].card.id != 0) {
            switch (o->id) {
            case 0x13B1:
                duel_0884F30C(BATTLE_EV(o->id, tp, duel_atk_tz, 0x1E200000), o->uid, 0);
                break;
            }
        }
        for (i = 0; i < 2; i++) {
            duel_088C4864(&duel_special.recs[i]);
        }
        for (i = 0; i < 2; i++) {
            q = duel_turn_player ^ i;
            n = duel_slot_effect_arg(q, 0xB, 0x1589);
            if (n > 0) {
                duel_088C4730(1 - q, n, 1, 0x1589);
            }
        }
        if (duel_atk_direct == 0 && duel_spec_act_ok(o) != 0) {
            switch (o->id) {
            case 0x16BF:
                if (o->unk8 != 0) {
                    if (o->pad14[6] != 0) {
                        ACT_CLEAR(&x1);
                        x1.player = tp;
                        duel_088D1F1C(&x1, 1);
                    } else {
                        duel_088D1438(tp, duel_atk_tz, 0, 0);
                    }
                }
                break;
            case 0x129A:
                if (o->unk8 == 0 && p != tp) {
                    if (o->pad14[6] != 0) {
                        ACT_CLEAR(&x2);
                        x2.id = o->id;
                        x2.player = tp;
                        duel_088D1F1C(&x2, 1);
                    } else {
                        duel_088D1438(tp, duel_atk_tz, 0, 0);
                    }
                }
                break;
            }
        }
        duel_088C7CF0();
        duel_w_08919610++;
        return 0;
    }
    return 1;
}

#ifdef NON_MATCHING /* não comparado em detalhe */
/* p: attacking player; events of the cards that won or lost the battle */
int duel_0883B4A8(unsigned int p)
{
    unsigned int tp = duel_atk_tp;
    DuelSpecAct *a = &duel_special.act[p];
    DuelSpecAct *o = &duel_special.act[1 - p];
    int r;
    unsigned int ev;
    struct DuelActX x;

    if (duel_atk_direct != 0) {
        return 1;
    }
    switch (duel_w_08919610) {
    case 0:
        if (duel_088C168C() != 0) {
            return 0;
        }
        if (duel_088C30D4() != 0) {
            return 0;
        }
        ev = BATTLE_EV(o->id, tp, duel_atk_tz, 0x2A200000);
        switch (o->id) {
        case 6000:
            if (duel_special.unk24 != 0) {
                duel_0884F30C(ev, o->uid, 0);
            }
            break;
        }
        duel_w_08919610++;
        return 0;
    case 1:
        /* attacker */
        ev = BATTLE_EV(a->id, p, duel_atk_zone, 0x2A200000);
        r = 0;
        switch (a->id) {
        case 0x10A6:
            r = 1;
            break;
        case 0x112F:
        case 0x1135:
            if (duel_spec_act_ok(a) != 0) {
                duel_088C4448(p, duel_atk_zone, a->uid);
                duel_cmd((tp ? 0x8000 : 0) | 0x47, duel_atk_tz, 5, 0);
            }
            break;
        case 0x15D9:
        case 0x18E6:
            r = o->pad14[6];
            break;
        case 0x1657:
            r = p != tp;
            break;
        case 0x172C:
            r = a->pad14[6] != 0 && p != tp;
            break;
        case 0x1866:
            r = !o->pad14[6];
            break;
        }
        if (r) {
            duel_0884F30C(ev, a->uid, 0);
        }
        /* target */
        ev = BATTLE_EV(o->id, tp, duel_atk_tz, 0x2A200000);
        r = 0;
        switch (o->id) {
        case 0x10A6:
        case 0x1310:
        case 0x14F1:
            r = 1;
            break;
        case 0x1130:
            if (duel_spec_act_ok(o) != 0 && (unsigned short)duel_field_type(p, duel_atk_zone) != 7) {
                duel_088C4448(tp, duel_atk_tz, o->uid);
                duel_cmd((p ? 0x8000 : 0) | 0x47, duel_atk_zone, 3, 0);
            }
            break;
        case 0x1208:
            if (duel_spec_act_ok(o) != 0 && (unsigned short)duel_field_type(p, duel_atk_zone) != 2) {
                duel_088C4448(tp, duel_atk_tz, o->uid);
                duel_088C87A8(duel_special.player[0], duel_atk_zone, o->id, 0x21, 0);
            }
            break;
        case 0x13B1:
            r = a->pad14[6] != 0 && p != tp;
            break;
        case 0x15D9:
        case 0x18E6:
            r = a->pad14[6];
            break;
        case 0x1657:
            r = p != tp;
            break;
        case 0x172C:
            r = o->pad14[6] != 0 && p != tp;
            break;
        case 0x1866:
            r = !a->pad14[6];
            break;
        }
        if (r) {
            duel_0884F30C(ev, o->uid, 0);
        }
        /* the target flipped by the attack */
        if (duel_special.unk24 != 0 && card_get_special_group(duel_special.unk24) != 0) {
            x.s_uid = o->uid;
            x.s_player = tp;
            x.s_zone = o->unk4;
            x.s_b15 = 1;
            x.s_b14 = o->unk8;
            x.s_b16 = 0;
            x.s_b17 = 0;
            if (duel_0886B0E4(ev, x.s_uid, *(unsigned int *)&x) != 0) {
                duel_cmd((tp ? 0x8000 : 0) | 0x61, duel_special.unk24, 1, 0);
                duel_0884F30C(ev, o->uid, *(unsigned int *)&x);
            }
        }
        return 1;
    }
    return 1;
}
#endif

#ifdef NON_MATCHING /* 7 KB; não comparado em detalhe. act_x_init e o laço dos efeitos estão inline no original */
/* destruction event word: uid, player, zone, kind 0xE (0xF: not by battle [?], 0xD: with 0x1797) */
static void act_x_init(struct DuelActX *x, DuelSpecAct *s, unsigned int p, unsigned int q)
{
    *(unsigned int *)x = 0;
    x->s_uid = s->uid;
    x->s_b18 = q;
    x->s_player = p;
    x->s_zone = s->unk4;
    x->s_b17 = 1;
    x->s_b14 = s->unk8;
    x->s_b15 = 1;
    x->s_b16 = 1;
    x->s_zone2 = 0xE;
}

/* p: attacking player; destruction of the monsters that lost the battle */
int duel_0883BB00(unsigned int p)
{
    unsigned int tp = duel_atk_tp;
    DuelSpecAct *a = &duel_special.act[p];
    DuelSpecAct *o = &duel_special.act[1 - p];
    DuelCard *acard;                   /* the attacker (its copy when it left the field) */
    DuelCard *ocard;
    int adead;                         /* destroyed by the battle */
    int odead;
    int aon;                           /* still the same card face up on the field */
    int oon;
    unsigned int both;                 /* players whose 0x1797 applies */
    struct DuelActX ax;
    struct DuelActX ox;
    DuelAct x1;
    DuelAct x2;
    DuelSlot *slot;
    DuelEffect *ef;
    unsigned int e;
    unsigned int sp;
    int sz;
    int k;
    int f;
    int i;
    int z;
    unsigned int q;

    acard = a->pad14[6] != 0 ? &duel_special.slot[0].card : &duel.players[p & 1].field[a->unk4].card;
    ocard = o->pad14[6] != 0 ? &duel_special.slot[1].card : &duel.players[tp & 1].field[o->unk4].card;
    adead = 0;
    if (!a->pad14[7] && a->pad14[6] != 0 && duel_special.slot[0].card.id != 0) {
        adead = 1;
    }
    odead = 0;
    if (!o->pad14[7] && o->pad14[6] != 0 && duel_special.slot[1].card.id != 0) {
        odead = 1;
    }
    aon = 0;
    if (a->uid == CARD_UID(duel.players[p & 1].field[a->unk4].card) && duel.players[p & 1].field[a->unk4].faceup != 0) {
        aon = 1;
    }
    oon = 0;
    if (o->uid == CARD_UID(duel.players[tp & 1].field[o->unk4].card) && duel.players[tp & 1].field[o->unk4].faceup != 0) {
        oon = 1;
    }
    if (duel_atk_direct != 0) {
        return 1;
    }
    switch (duel_w_08919610) {
    case 0: /* cards that survive the battle */
        if (duel_spec_act_ok(a) != 0) {
            switch (a->id) {
            case 0x13A4:
                if (duel_088C1D80(p) != 0) {
                    a->pad14[7] = 1;
                }
                break;
            case 0x147A:
                if (duel_card_check_9148(p, 0x146F) == 0) {
                    a->pad14[7] = 1;
                }
                break;
            }
        }
        if (duel_spec_act_ok(o) != 0) {
            switch (o->id) {
            case 0x13A4:
                if (duel_088C1D80(tp) != 0) {
                    o->pad14[7] = 1;
                }
                break;
            case 0x147A:
                if (duel_card_check_9148(tp, 0x146F) == 0) {
                    o->pad14[7] = 1;
                }
                break;
            }
        }
        duel_w_08919610++;
        return 0;
    case 1: /* effects of the destroyed cards on the other one */
        if (adead && duel_card_protected(a->id, a->uid) == 0 &&
            duel_slot_effect_with(duel_turn_player, 0xB, 0x15FF, a->uid) == 0) {
            switch (a->id) {
            case 0x13AD:
            case 0x13B0:
            case 0x13B4:
            case 0x1A94:
                if (oon) {
                    duel_088C87A8(tp, duel_atk_tz, a->id, 4, 0);
                }
                break;
            case 0x1836:
                duel_088C77D4(p, a->id, 1, a->uid);
                break;
            }
        }
        if (adead) {
            switch (o->id) {
            case 0x12A0:
                if (oon) {
                    duel_088C87A8(tp, duel_atk_tz, o->id, 1, 0);
                }
                break;
            case 0x12A6:
                if (oon && card_is_monster(a->id) != 0) {
                    duel_088C87A8(tp, duel_atk_tz, 0x12A6, 3, tp + a->uid * 2);
                }
                break;
            case 0x1415:
                if (duel_spec_act_ok(o) != 0) {
                    duel_088C77D4(tp, 0x1415, 1, CARD_UID(*acard));
                }
                break;
            case 0x147D:
                if (oon && duel_spec_act_ok(o) != 0) {
                    duel_088C4448(tp, duel_atk_tz, o->uid);
                    duel_088C87A8(tp, duel_atk_tz, o->id, 3, 0);
                }
                break;
            case 0x1529:
                if (duel_slot_value_1909(tp, duel_atk_tz) < 2) {
                    duel_088C4448(tp, duel_atk_tz, o->uid);
                    duel_088CC1D0(tp, duel_atk_tz, 1);
                }
                break;
            case 0x17D2:
            case 0x17D3:
            case 0x17D7:
            case 0x17D8:
            case 0x17DA:
                if (oon) {
                    duel_088C87A8(tp, duel_atk_tz, 0x17D2, 2, 0);
                }
                break;
            }
        }
        if (odead && duel_card_protected(o->id, o->uid) == 0 &&
            duel_slot_effect_with(duel_turn_player, 0xB, 0x15FF, o->uid) == 0) {
            switch (o->id) {
            case 0x13AD:
            case 0x13B0:
            case 0x13B4:
            case 0x1A94:
                if (aon) {
                    duel_088C87A8(p, duel_atk_zone, o->id, 4, 0);
                }
                break;
            case 0x1836:
                if (duel_special.unk24 == 0) {
                    duel_088C77D4(tp, o->id, 1, o->uid);
                }
                break;
            }
        }
        if (odead) {
            switch (a->id) {
            case 0x12A0:
                if (aon) {
                    duel_088C87A8(p, duel_atk_zone, a->id, 1, 0);
                }
                break;
            case 0x12A6:
                if (aon && card_is_monster(o->id) != 0) {
                    duel_088C87A8(p, duel_atk_zone, 0x12A6, 3, p + o->uid * 2);
                }
                break;
            case 0x1415:
                if (duel_spec_act_ok(a) != 0) {
                    duel_088C77D4(p, 0x1415, 1, CARD_UID(*ocard));
                }
                break;
            case 0x147D:
                if (aon && duel_spec_act_ok(a) != 0) {
                    duel_088C4448(p, duel_atk_zone, a->uid);
                    duel_088C87A8(p, duel_atk_zone, a->id, 3, 0);
                }
                break;
            case 0x1529:
                if (duel_slot_value_1909(p, duel_atk_zone) < 2) {
                    duel_088C4448(p, duel_atk_zone, a->uid);
                    duel_088CC1D0(p, duel_atk_zone, 1);
                }
                break;
            case 0x17D2:
            case 0x17D3:
            case 0x17D7:
            case 0x17D8:
            case 0x17DA:
                if (aon) {
                    duel_088C87A8(p, duel_atk_zone, 0x17D2, 2, 0);
                }
                break;
            }
        }
        duel_w_08919610++;
        return 0;
    case 2: /* send the destroyed cards to the graveyard */
        duel_088C7BA4();
        both = 0;
        if (a->pad14[7] != 0) {
            if (a->pad14[6] != 0) {
                ACT_CLEAR(&x1);
                x1.player = p;
                duel_088D1F1C(&x1, 0);
            } else {
                duel_088D1438(p, a->unk4, 0, 0);
            }
        } else if (adead) {
            act_x_init(&ax, a, p, tp);
            if (card_is_attribute9(a->id) != 0 || (a->id == 0x16F8 && duel_spec_act_ok(a) != 0) ||
                duel_088DFCC8(a->uid) == 0) {
                ax.s_zone2 = 0xF;
            }
            if (duel_spec_act_ok(o) != 0) {
                switch (o->id) {
                case 0x16F8:
                    ax.s_zone2 = 0xF;
                    break;
                case 0x147A:
                case 0x14B7:
                    if (oon) {
                        ax.s_zone2 = 0xF;
                    }
                    break;
                case 0x15D9:
                case 0x18E6:
                    if (duel_slot_effect_with(tp, 0xB, o->id, o->uid) != 0) {
                        ax.s_zone2 = 0xF;
                    }
                    break;
                }
            }
            if (card_is_attribute9(a->id) == 0 && duel_count_active(0x1797) != 0) {
                ax.s_zone2 = 0xD;
                both |= 1 << (ax.s_uid & 1);
            }
            duel_088D1DF4(acard, ax);
            f = 0;
            switch (o->id) {
            case 0x147A: case 0x1520: case 0x158F: case 0x1592: case 0x15DC: case 0x164D: case 0x16C5:
            case 0x1704: case 0x170B: case 0x172A: case 0x172B: case 0x174A: case 0x174B: case 0x1791:
            case 0x1792: case 0x17C7: case 0x17C8: case 0x18AE: case 0x18C8: case 0x193B: case 0x1943:
            case 0x194F: case 0x1987: case 0x19A4: case 0x19D3: case 0x1A1C: case 0x1A23: case 0x1AAD:
            case 0x1B2D: /* the target's effect when it destroys a monster */
                f = oon;
                break;
            case 0x16C6:
                f = 1;
                break;
            case 0x1A95:
                if (oon && ax.s_zone2 == 14 && duel_spec_act_ok(o) != 0 && duel_special_has_effect(p, a->unk4, 0x1A90) != 0) {
                    duel_088C87A8(tp, o->unk4, o->id, 2, a->uid);
                }
                break;
            }
            if (f) {
                duel_0884F30C(BATTLE_EV(o->id, o->unk0, o->unk4, 0x2C200000), o->uid, *(unsigned int *)&ax);
            }
            if (oon) {
                /* cards equipped to / affecting the target */
                for (e = duel.players[tp & 1].field[duel_atk_tz].effects; e != 0;) {
                    ef = &duel.effects[e];
                    e = ef->next;
                    k = ef->kind & 0xF;
                    if (k != 11 && k != 10) {
                        continue;
                    }
                    sp = EFF_PLAYER(ef);
                    sz = EFF_ZONE(ef);
                    slot = &duel.players[sp & 1].field[sz];
                    if (!SLOT_ACTIVE(slot)) {
                        continue;
                    }
                    f = 0;
                    switch (slot->card.id) {
                    case 0x15D1:
                        f = p != sp && !duel_slot_has_effect(sp, sz, 0x15B3);
                        break;
                    case 0x15D5:
                        f = k == 11 && !duel_slot_has_effect(sp, sz, 0x15B3);
                        break;
                    case 0x18D0:
                    case 0x1A69:
                        f = p != sp && ax.s_zone2 == 14;
                        break;
                    case 0x197C:
                        f = 1;
                        break;
                    }
                    if (f) {
                        duel_0884F30C(BATTLE_EV(slot->card.id, sp, sz, 0x2C200000), CARD_UID(slot->card), *(unsigned int *)&ax);
                    }
                }
            }
        }
        if (o->pad14[7] != 0) {
            if (o->pad14[6] != 0) {
                ACT_CLEAR(&x2);
                x2.player = tp;
                duel_088D1F1C(&x2, 1);
            } else {
                duel_088D1438(tp, o->unk4, 0, 0);
            }
        } else if (odead) {
            act_x_init(&ox, o, tp, p);
            if (card_is_attribute9(o->id) != 0 || (o->id == 0x16F8 && duel_spec_act_ok(o) != 0) ||
                duel_088DFCC8(o->uid) == 0) {
                ox.s_zone2 = 0xF;
            }
            if (duel_spec_act_ok(a) != 0) {
                switch (a->id) {
                case 0x16F8:
                    ox.s_zone2 = 0xF;
                    break;
                case 0x147A:
                case 0x14B7:
                    if (aon) {
                        ox.s_zone2 = 0xF;
                    }
                    break;
                case 0x15D9:
                case 0x18E6:
                    if (duel_slot_effect_with(p, 0xB, a->id, a->uid) != 0) {
                        ox.s_zone2 = 0xF;
                    }
                    break;
                }
            }
            if (card_is_attribute9(o->id) == 0 && duel_count_active(0x1797) != 0) {
                ox.s_zone2 = 0xD;
                both |= 1 << (ox.s_uid & 1);
            }
            f = 0;
            switch (a->id) {
            case 0x147A: case 0x1520: case 0x158F: case 0x1592: case 0x15DC: case 0x164D: case 0x1641:
            case 0x16C5: case 0x1704: case 0x170B: case 0x172A: case 0x172B: case 0x174A: case 0x174B:
            case 0x178D: case 0x1791: case 0x1792: case 0x17C7: case 0x17C8: case 0x18AE: case 0x18C8:
            case 0x193B: case 0x1943: case 0x194F: case 0x1987: case 0x19A4: case 0x19D3: case 0x1A1C:
            case 0x1A23: case 0x1AAD: case 0x1B2D:
                f = aon;
                break;
            case 0x16C6:
                f = 1;
                break;
            case 0x1A95:
                if (aon && ox.s_zone2 == 14 && duel_spec_act_ok(a) != 0 && duel_special_has_effect(tp, o->unk4, 0x1A90) != 0) {
                    duel_088C87A8(p, a->unk4, a->id, 2, o->uid);
                }
                break;
            }
            if (f) {
                duel_0884F30C(BATTLE_EV(a->id, a->unk0, a->unk4, 0x2C200000), a->uid, *(unsigned int *)&ox);
            }
            if (aon) {
                for (e = duel.players[p & 1].field[duel_atk_zone].effects; e != 0;) {
                    ef = &duel.effects[e];
                    e = ef->next;
                    k = ef->kind & 0xF;
                    if (k != 11 && k != 10) {
                        continue;
                    }
                    sp = EFF_PLAYER(ef);
                    sz = EFF_ZONE(ef);
                    slot = &duel.players[sp & 1].field[sz];
                    if (!SLOT_ACTIVE(slot)) {
                        continue;
                    }
                    f = 0;
                    switch (slot->card.id) {
                    case 0x15D1:
                        f = tp != sp && !duel_slot_has_effect(sp, sz, 0x15B3);
                        break;
                    case 0x15D5:
                        f = k == 11 && !duel_slot_has_effect(sp, sz, 0x15B3);
                        break;
                    case 0x18D0:
                    case 0x1A69:
                        f = tp != sp && ox.s_zone2 == 14;
                        break;
                    case 0x197C:
                        f = 1;
                        break;
                    case 0x19D8:
                        if (o->pad14[5] >= 5) {
                            duel_088C87A8(sp, sz, slot->card.id, 1, 0);
                        }
                        break;
                    }
                    if (f) {
                        duel_0884F30C(BATTLE_EV(slot->card.id, sp, sz, 0x2C200000), CARD_UID(slot->card), *(unsigned int *)&ox);
                    }
                }
            }
            duel_088D1DF4(ocard, ox);
        }
        /* spells/traps that leave with a destroyed monster */
        for (i = 0; i < 2; i++) {
            q = duel_turn_player ^ i;
            for (z = 5; z < 10; z++) {
                if (duel.players[q & 1].field[z].card.id != 0 && duel_slot_has_effect(q, z, 0x15B3) != 0) {
                    duel_088D1438(q, z, 0, duel.players[q & 1].field[z].card.id);
                }
            }
        }
        /* the attacker hit a stronger face-down defender */
        if (aon && a->pad14[2] < o->pad14[3] && o->unk8 != 0 && duel_spec_act_ok(o) != 0) {
            f = 0;
            switch (o->id) {
            case 0x152C:
                f = duel_special.unk24 > 0;
                break;
            case 0x1658:
            case 0x1751:
                f = p != tp;
                break;
            case 0x16B7:
                f = 1;
                break;
            }
            if (f) {
                duel_088C4448(tp, duel_atk_tz, o->uid);
                duel_088D1438(p, duel_atk_zone, 1, o->id);
            }
        }
        if (aon && duel_slot_has_effect(p, duel_atk_zone, 0x1493) != 0) {
            duel_088C44E4(1 - p, 0x1493);
            duel_088D1438(p, duel_atk_zone, 1, 0x1493);
        }
        if (aon && duel_slot_has_effect(p, duel_atk_zone, 0x162E) != 0) {
            duel_088C44E4(1 - p, 0x162E);
            duel_088D1438(p, duel_atk_zone, 1, 0x162E);
        }
        if (aon && duel_slot_has_effect(p, duel_atk_zone, 0x1883) != 0) {
            duel_088C44E4(1 - p, 0x1883);
            duel_088D1438(p, duel_atk_zone, 1, 0x1883);
        }
        if (duel_slot_has_effect(tp, duel_atk_tz, 0x1140) != 0 && oon) {
            duel_088C4448(p, duel_atk_zone, a->uid);
            duel_088D1438(tp, duel_atk_tz, 1, a->id);
        }
        if (both != 0) {
            if (both & 1 << p) {
                duel_088C78E0(p);
            }
            if (both & 1 << tp) {
                duel_088C78E0(tp);
            }
        }
        duel_088C7CF0();
        duel_w_08919610++;
        return 0;
    }
    return 1;
}
#endif

#ifdef NON_MATCHING /* não comparado em detalhe */
/* p: attacking player; end of the damage step: cards that act after the battle */
int duel_0883D798(unsigned int p)
{
    unsigned int tp = duel_atk_tp;
    DuelSpecAct *a = &duel_special.act[p];
    DuelSpecAct *o = &duel_special.act[1 - p];
    int aon;
    int oon;
    unsigned int e;
    DuelEffect *ef;
    DuelSlot *slot;
    unsigned int sp;
    int sz;
    unsigned int q;
    int z;
    int i;

    aon = 0;
    if (a->uid == CARD_UID(duel.players[p & 1].field[a->unk4].card) && duel.players[p & 1].field[a->unk4].faceup != 0) {
        aon = 1;
    }
    oon = 0;
    if (o->uid == CARD_UID(duel.players[tp & 1].field[o->unk4].card) && duel.players[tp & 1].field[o->unk4].faceup != 0) {
        oon = 1;
    }
    switch (duel_w_08919610) {
    case 0:
        duel_088C7BA4();
        /* 0x14D6 on a face-down attacker: flip it */
        if (aon && SLOT_ACTIVE(ATK_SLOT(p, duel_atk_zone))) {
            switch (a->id) {
            case 0x14D6:
            case 0x1993:
                if (duel_slot_has_effect(p, duel_atk_zone, 0x14D6) != 0 && ATK_SLOT(p, duel_atk_zone)->pos == 0) {
                    duel_088C4448(p, duel_atk_zone, a->uid);
                    duel_088C9878(p, duel_atk_zone, 0, 0, a->id, 0);
                }
                break;
            }
        }
        if (duel_atk_direct == 0 && oon && SLOT_ACTIVE(ATK_SLOT(tp, duel_atk_tz))) {
            switch (o->id) {
            case 0x129C:
                if (ATK_SLOT(tp, duel_atk_tz)->faceup != 0 && ATK_SLOT(tp, duel_atk_tz)->pos != 0 &&
                    duel_slot_has_effect(tp, duel_atk_tz, o->id) != 0) {
                    duel_088C4448(tp, duel_atk_tz, o->uid);
                    duel_088C9878(tp, duel_atk_tz, 0, 1, o->id, 0);
                }
                break;
            }
        }
        if (duel_slot_effect_kind(tp, duel_atk_tz, 0x18CD, 4) != 0) {
            duel_088D1438(tp, duel_atk_tz, 1, 0x18CD);
        }
        if (duel_slot_effect_kind(p, duel_atk_zone, 0x18CD, 4) != 0) {
            duel_088D1438(p, duel_atk_zone, 1, 0x18CD);
        }
        /* attacker */
        switch (a->id) {
        case 0x1441:
        case 0x1700:
        case 0x1703:
        case 0x170D:
        case 0x170E:
        case 0x19A6:
            if (duel_atk_direct == 0 && oon && duel_slot_has_effect(tp, duel_atk_tz, a->id) != 0) {
                duel_0884F30C(BATTLE_EV(a->id, p, duel_atk_zone, 0x200000), a->uid,
                              (unsigned short)((tp & 0xFF) | (unsigned char)duel_atk_tz << 8));
            }
            break;
        case 0x1837:
        case 0x1913:
        case 0x1962:
        case 0x19BF:
            if (aon && SLOT_ACTIVE(ATK_SLOT(p, duel_atk_zone)) && duel_slot_has_effect(p, duel_atk_zone, a->id) != 0) {
                if (ATK_SLOT(p, duel_atk_zone)->link != 0) {
                    duel_0884F30C(BATTLE_EV(a->id, p, duel_atk_zone, 0x200000), a->uid, 0);
                } else {
                    duel_088D1438(p, duel_atk_zone, 0, 0);
                }
            }
            break;
        case 0x19C7:
            if (duel_slot_effect_with(p, 0xB, a->id, a->uid) != 0) {
                duel_0884F30C(BATTLE_EV(a->id, aon ? p : (unsigned int)a->uid & 1, aon ? duel_atk_zone : 16, 0x200000),
                              a->uid, 0);
            }
            break;
        case 0x1A52:
            if (aon && SLOT_ACTIVE(ATK_SLOT(p, duel_atk_zone)) && duel_slot_has_effect(p, 0xB, 0x1A41) != 0) {
                duel_0884F30C(BATTLE_EV(a->id, p, duel_atk_zone, 0x200000), a->uid, 0);
            }
            break;
        case 0x1A7A:
            if (aon) {
                duel_0884F30C(BATTLE_EV(a->id, p, duel_atk_zone, 0x200000), a->uid, 0);
            }
            break;
        }
        /* cards attached to the attacker */
        if (aon) {
            for (e = ATK_SLOT(p, duel_atk_zone)->effects; e != 0;) {
                ef = &duel.effects[e];
                e = ef->next;
                if ((ef->kind & 0xF) != 10) {
                    continue;
                }
                sp = EFF_PLAYER(ef);
                sz = EFF_ZONE(ef);
                slot = &duel.players[sp & 1].field[sz];
                if (!SLOT_ACTIVE(slot)) {
                    continue;
                }
                switch (slot->card.id) {
                case 0x1A9E:
                    duel_0884F30C(BATTLE_EV(slot->card.id, sp, sz, 0x200000), CARD_UID(slot->card), 0);
                    break;
                case 0x143A: /* the attacker dealt battle damage */
                    if (p == sp && duel_atk_direct == 0 && p != tp && oon && duel_slot_counter_1439(tp, duel_atk_tz) != 0 &&
                        ((duel_special.recs[0].b[0] == p && duel_special.recs[0].w0 != 0) ||
                         (duel_special.recs[1].b[0] == p && duel_special.recs[1].w0 != 0))) {
                        duel_088C4448(sp, sz, CARD_UID(slot->card));
                        duel_088C87A8(tp, duel_atk_tz, 0x143A, 5, p);
                    }
                    break;
                }
            }
        }
        /* 0x143E in a field spell zone */
        for (i = 0; i < 2; i++) {
            if (duel_slot_e8170(i, 10, 0x143E) != 0) {
                duel_0884F30C(((unsigned int)i << 31 & 0x80000000) | ((i != duel_turn_player) << 24 & 0x1000000) | 0x2A143E,
                              CARD_UID(duel.players[i & 1].field[10].card), 0);
            }
        }
        /* target */
        if (duel_atk_direct == 0) {
            switch (o->id) {
            case 0x1441:
            case 0x1700:
            case 0x1703:
            case 0x170D:
            case 0x170E:
            case 0x1749:
            case 0x19A6:
                if (aon && duel_slot_has_effect(p, duel_atk_zone, o->id) != 0) {
                    duel_0884F30C(BATTLE_EV(o->id, tp, duel_atk_tz, 0x200000), o->uid,
                                  (unsigned short)((p & 0xFF) | (unsigned char)duel_atk_zone << 8));
                }
                break;
            case 0x1837:
            case 0x1913:
            case 0x1962:
            case 0x19BF:
                if (oon && SLOT_ACTIVE(ATK_SLOT(tp, duel_atk_tz))) {
                    if (duel_slot_has_effect(tp, duel_atk_tz, o->id) != 0) {
                        if (ATK_SLOT(tp, duel_atk_tz)->link != 0) {
                            duel_0884F30C(BATTLE_EV(o->id, tp, duel_atk_tz, 0x200000), o->uid, 0);
                        } else {
                            duel_088D1438(tp, duel_atk_tz, 0, 0);
                        }
                    }
                    break;
                }
            case 0x19C7:
                if (duel_slot_effect_with(tp, 0xB, o->id, o->uid) != 0) {
                    duel_0884F30C(BATTLE_EV(o->id, oon ? tp : (unsigned int)o->uid & 1, oon ? duel_atk_tz : 16, 0x200000),
                                  o->uid, 0);
                }
                break;
            }
        }
        duel_088C7CF0();
        duel_w_08919610++;
        return 0;
    case 1:
        duel_cmd(0x1E, 0, 0, 0);
        if (duel_08843844() != 0) {
            duel_w_0891960C = 12;
            duel_w_08919610 = 0;
            duel_w_08919614 = 0;
            return 0;
        }
        /* 0x16CB: attack again */
        if (duel_slot_effect_kind(p, duel_atk_zone, 0x16CB, 4) != 0 && !(ATK_SLOT(p, duel_atk_zone)->flags >> 23 & 1) &&
            duel_attack_check(p, duel_atk_zone, 1) != 0) {
            duel_088C87A8(p, duel_atk_zone, 0x16CB, 4, 0);
            duel_w_0891960C = 2;
            duel_w_08919610 = 0;
            duel_atk_w28 = 1;
            duel_host_call(11, duel_atk_zone, 1 - duel_special.player[0], -1);
            return 0;
        }
        duel_w_08919610++;
        return 0;
    }
    duel_w_0891960C = 1;
    duel_w_08919610 = 0;
    return 0;
}
#endif

#ifdef NON_MATCHING /* switch por tabela (0x08907C68); não comparado em detalhe */
/* p: attacking player; after the battle: flips, moved cards, then the end of the battle step */
int duel_0883E63C(unsigned int p)
{
    unsigned int q;
    unsigned int mask;
    unsigned int bit;
    unsigned int loc;
    int arg;
    int uid;
    unsigned int ap;
    int zz;
    int id;
    int i;
    int z;

    switch (duel_w_08919610) {
    case 0:
        duel_088AFD5C(23, 0);
        duel_w_08919610++;
        return 0;
    case 1: /* face-down monsters flipped by 0x14D6 and similar */
        for (i = 0; i < 2; i++) {
            q = duel_turn_player ^ i;
            for (z = 0; z < 5; z++) {
                if (!SLOT_ACTIVE(ATK_SLOT(q, z))) {
                    continue;
                }
                id = field_get_card(q, z);
                switch (id) {
                case 0x1419:
                case 0x1566:
                case 0x15D2:
                case 0x1912:
                case 0x1915:
                    if (duel_slot_has_effect(q, z, 0x14D6) != 0 && ATK_SLOT(q, z)->pos == 0) {
                        duel_088C89BC(q, z, 0x14D6, 1);
                        duel_088C4448(q, z, CARD_UID(ATK_SLOT(q, z)->card));
                        duel_088C9878(q, z, 0, 0, id, 0);
                        duel_088C87A8(q, z, 0x1419, 34, 0);
                    }
                    break;
                case 0x14D7:
                    if (duel_slot_has_effect(q, z, id) != 0) {
                        duel_088C89BC(q, z, id, 1);
                    }
                    break;
                case 0x1983:
                    if (duel_slot_has_effect(q, z, id) != 0 && duel_slot_e68a0(q, z) != 0) {
                        duel_088C4448(q, z, CARD_UID(ATK_SLOT(q, z)->card));
                        duel_088CC284(q, z, -duel_slot_e68a0(q, z));
                    }
                    break;
                case 0x1A61:
                    if (duel_slot_has_effect(q, z, 0x14D6) != 0 && ATK_SLOT(q, z)->pos == 0) {
                        duel_088C4448(q, z, CARD_UID(ATK_SLOT(q, z)->card));
                        duel_088C9878(q, z, 0, 0, 0x1A61, 0);
                    }
                    break;
                }
            }
        }
        duel_w_08919610++;
        return 0;
    case 2:
        for (i = 0; i < 2; i++) {
            q = duel_turn_player ^ i;
            for (z = 0; z < 5; z++) {
                if (duel_slot_has_effect(q, z, 0x1392) != 0) {
                    duel_088C44E4(q, 0x1392);
                    duel_088D1438(q, z, 1, 0x1392);
                }
            }
        }
        duel_w_08919610++;
        return 0;
    case 3: /* 0x12A6: set the card it holds from the graveyard */
        for (i = 0; i < 2; i++) {
            q = duel_turn_player ^ i;
            for (z = 0; z < 5; z++) {
                if (duel_slot_e8170(q, z, 0x12A6) == 0) {
                    continue;
                }
                arg = duel_slot_effect_arg(q, z, 0x12A6);
                if (arg < 0) {
                    continue;
                }
                uid = arg / 2;
                ap = arg & 1;
                zz = duel_pick_st_zone(ap);
                loc = duel_card_locate(uid);
                if (zz >= 0 && (loc & 0xFFFF) >> 8 == 0xE) {
                    duel_088C4448(q, z, CARD_UID(ATK_SLOT(q, z)->card));
                    duel_088CB728(loc & 0xFF, 0xE, uid, ap, zz, 1, 0);
                    duel_088C8B78(ap, (ap & 0xFF) | (zz & 0xFF) << 8, (q & 0xFF) | (z & 0xFF) << 8, 12);
                    duel_088C89BC(q, z, 0x12A6, 0);
                    return 0;
                }
            }
        }
        duel_w_08919610++;
    case 4:
        for (i = 0; i < 2; i++) {
            if (duel_slot_has_effect(i, 0xB, 0x1415) != 0) {
                if (duel_0884F30C(((unsigned int)i << 31 & 0x80000000) | 0x501415, 0, 0) != 0) {
                    return 0;
                }
                duel_088C89BC(i, 0xB, 0x1415, 0);
                return 0;
            }
        }
        duel_w_08919610++;
    case 5:
        for (i = 0; i < 2; i++) {
            q = duel_turn_player ^ i;
            for (z = 0; z < 5; z++) {
                if (duel_slot_e8170(q, z, 0x1A95) != 0 && (arg = duel_slot_effect_arg(q, z, 0x1A95)) >= 0) {
                    duel_0884F30C(BATTLE_EV(0x1A95, q, z, 0x200000), CARD_UID(ATK_SLOT(q, z)->card), arg);
                    duel_088C89BC(q, z, 0x1A95, 0);
                    return 0;
                }
            }
        }
        duel_w_08919610++;
    case 6:
        for (i = 0; i < 2; i++) {
            q = duel_turn_player ^ i;
            for (z = 0; z < 5; z++) {
                if (ATK_SLOT(q, z)->card.id != 0) {
                    duel_088C89BC(q, z, 0x12E2, 1);
                }
            }
        }
        duel_w_08919610++;
        return 0;
    case 7: /* 0x1362 on a non-monster card */
        mask = 0;
        for (i = 0; i < 2; i++) {
            q = duel_turn_player ^ i;
            bit = q << 4;
            for (z = 0; z < 5; z++, bit++) {
                if (ATK_SLOT(q, z)->card.id != 0 && card_is_monster(ATK_SLOT(q, z)->card.id) == 0 &&
                    duel_field_flag9(q, z) == 0 && duel_slot_has_effect(q, z, 0x1362) != 0) {
                    mask |= 1 << bit;
                }
            }
        }
        if (mask != 0) {
            duel_088D13D8(1 - p, 0x1362, mask);
            return 0;
        }
        duel_w_08919610++;
    case 8:
        if (duel_slot_has_effect(p, 0xB, 0x1512) != 0) {
            mask = 0;
            for (i = 0; i < 2; i++) {
                q = duel_turn_player ^ i;
                bit = q << 4;
                for (z = 0; z < 5; z++, bit++) {
                    if (ATK_SLOT(q, z)->card.id != 0 && duel_slot_has_effect(q, z, 0x1512) != 0) {
                        mask |= 1 << bit;
                    }
                }
            }
            if (mask != 0) {
                duel_088C44E4(p, 0x1512);
                duel_088D13D8(p, 0x1512, mask);
            }
        }
        duel_w_08919610++;
        return 0;
    case 9: /* 0x1571 in the opponent's graveyard */
        q = 1 - p;
        for (i = 0; i < duel.players[q & 1].n_grave; i++) {
            if (duel.players[q & 1].grave[i].id == 0x1571) {
                if (duel_0884F30C((q << 31 & 0x80000000) | 0x4E1571, CARD_UID(duel.players[q & 1].grave[i]), 0) != 0) {
                    return 0;
                }
            }
        }
        duel_w_08919610++;
        return 0;
    }
    duel_088C89BC(p, 0xB, 0x1469, 1);
    if (duel_slot_has_effect(p, 0xB, 0x11ED) != 0) {
        duel_088C89BC(p, 0xB, 0x11ED, 1);
        duel_w_0891960C = 0;
        duel_w_08919610 = 0;
        return 0;
    }
    return 1;
}
#endif
