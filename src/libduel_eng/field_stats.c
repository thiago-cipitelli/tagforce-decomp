// @range 0x088F2D70-0x088F5D54
// @asm 0x088F2D70-0x088F5D54
/* duel_field_stats: the effective stats of monster z of player p (type, kind, level, ATK/DEF with
   every modifier): the monster's own effect, the bonuses of the other cards on the field and of the
   field spells, the effect entries attached to its slot (equips, counters, changes by card), and
   the halving and doubling cards. Exported (libduel_draw, libduel_mgr and the AI use it). */
#include "duel.h"
#include "card_db.h"

extern unsigned int duel_turn_player;
extern int duel_phase;

int field_get_card(unsigned int p, int z);
unsigned int duel_field_type(unsigned int p, int z);
unsigned int duel_field_type_ex(unsigned int p, int z, int skip);
int duel_field_kind_ex(unsigned int p, int z, unsigned int *stamp);
int duel_field_level(unsigned int p, int z);
unsigned int duel_field_attrs(unsigned int p, int z);
void duel_field_base_stats(unsigned int p, int z, DuelStats *st);
int duel_field_base_atk(unsigned int p, int z);
unsigned int duel_field_bonus(DuelStats *st, unsigned int p, int z, unsigned int q, int qz);
unsigned int duel_card_bonus(DuelStats *st, unsigned int p, int z, unsigned int q, int qz);
unsigned int duel_slot_immunity(unsigned int p, int z);
unsigned int duel_slot_ref(unsigned int p, int z);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_affected_ex(unsigned int p, int z, int id);
int duel_slot_count_14c0(unsigned int p, int z);
int duel_st_affects(unsigned int p, int z, unsigned int q, int sz);
unsigned short duel_slot_e1854(unsigned int p, int z);
unsigned int duel_field_flag9(unsigned int p, int z);
unsigned short duel_slot_value_1909(unsigned int p, int z);
unsigned int duel_slot_e68a0(unsigned int p, int z);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_find_effect_card(unsigned int p, int id, int skip);
int duel_count_active(int id);
int duel_card_check_9148(unsigned int p, int id);
int duel_088E9150(int id);
int duel_count_type_skip(unsigned int p, unsigned int type, int skip);
int duel_count_type(unsigned int p, unsigned int type);
int duel_count_attr_skip(unsigned int p, int attr, int skip);
int duel_count_attr(unsigned int p, int attr);
int duel_count_monsters(unsigned int p);
int duel_count_monsters_special(int p);
int duel_count_monsters_fn(unsigned int p, int (*fn)(int id));
int duel_count_attr9(unsigned int p);
int duel_count_monsters_pos(unsigned int p, int faceup, int pos0);
int duel_count_field_all(unsigned int p);
int duel_count_field_cards(unsigned int p, int faceup, int facedown, int fzone);
int duel_count_faceup_traps(unsigned int p);
int duel_count_grave_type(unsigned int p, unsigned int type);
int duel_count_grave_traps(unsigned int p, unsigned int st);
int duel_count_same_name_other(unsigned int p, int z);
int duel_grave_count_id(unsigned int p, unsigned int id);
int duel_grave_count_id_b21(unsigned int p, unsigned int id);
int duel_grave_count_monsters(unsigned int p);
int duel_grave_count_1a6f(unsigned int p);
int duel_zone_f_count_monsters(unsigned int p);
void duel_field_stats(unsigned int p, int z, DuelStats *st);

/* An effect of the side of player q applies to a monster of player p unless its immunity bits say no
   (bit 0: effects of its controller, bit 1: of the opponent). */
#define IMMUNE(bits, p, q) ((bits) & (((p) != (q)) + 1))

/* bit 5 of a slot's flags: the card there is negated */
/* the monster itself */
#define SLOT (duel.players[p & 1].field[z])

#define SLOT_NEGATED(s) ((s)->flags >> 5 & 1)

#ifdef NON_MATCHING /* 12 KB; o original guarda endereços de duel.players[p].field[z] na pilha e avalia SLOT_ACTIVE antes do teste de stamp: alocação e escalonamento diferentes em quase todo o corpo */
void duel_field_stats(unsigned int p, int z, DuelStats *st)
{
    int atk_eq = 0;                    /* equips and cards attached to the slot */
    int def_eq = 0;
    int atk_eff = 0;                   /* the monster's own effect and the changes by card */
    int def_eff = 0;
    int atk_bon = 0;                   /* bonuses of the cards on the field */
    int def_bon = 0;
    int fixed = 0;                     /* the stats were set outright: no modifier applies */
    int atk_fld;                       /* field spell bonus not yet counted */
    int def_fld;
    int neg;                           /* 0x1257 on the field: modifiers count backwards */
    unsigned int stamp;
    DuelSlot *s;
    DuelPlayer *pl;
    DuelEffect *ef;
    DuelEffect *e1;
    DuelEffect *e2;
    unsigned int b;
    unsigned int q;
    unsigned int sp;
    int sz;
    int kind;
    int src;
    int arg;
    int id;
    int i;
    int n;
    int all;
    int cnt[2];
    DuelStats other;
    unsigned int e;
    unsigned int v;
    unsigned char *c;

    st->attr = 0;
    st->unk8 = 0;
    st->level = 0;
    st->batk = 0;
    st->atk = 0;
    st->bdef = 0;
    st->def = 0;
    st->imm = 0;
    st->imm2 = 0;
    st->id = field_get_card(p, z);
    if (st->id == 0) {
        return;
    }
    if (z > 4) {
        return;
    }
    st->attr = (unsigned short)duel_field_type(p, z);
    st->unk8 = duel_field_kind_ex(p, z, &stamp);
    st->level = duel_field_level(p, z);
    st->mask = 1 << st->unk8;
    if (stamp == 0 && (st->id == 0x19EF || st->id == 0x18C7) && SLOT_ACTIVE(&SLOT)) {
        st->mask |= 0x78;
    }
    duel_field_base_stats(p, z, st);
    st->atk = st->batk;
    st->def = st->bdef;
    if (SLOT.faceup == 0) {
        return;
    }
    st->imm = duel_slot_immunity(p, z);
    st->imm2 = duel_slot_ref(p, z);
    neg = duel_slot_has_effect(0, 0xB, 0x1257);

    /* the monster's own effect */
    if (SLOT_ACTIVE(&SLOT)) {
        switch (st->id) {
        case 0xFB2:
            atk_eff += duel_card_check_9148(p, 0x11D0) * 500;
            break;
        case 0x1009:
            if (SLOT.flags & 0x10) {
                atk_eff += 100;
                def_eff += 100;
            }
            break;
        case 0x1116:
            atk_eff += duel_grave_count_monsters(p) * 100;
            break;
        case 0x1091:
            atk_eff += duel_count_type(0, 0xD) * 100 + duel_count_type(1, 0xD) * 100;
            break;
        case 0x1128:
            n = duel_088E9150(0xFE4) * 300;
            atk_eff += n;
            def_eff += n;
            break;
        case 0x113D:
            atk_eff += duel_count_type(0, 7) * 100 + duel_count_type(1, 7) * 100;
            break;
        case 0x1193:
            atk_eff += duel_slot_count_14c0(p, z) * 500;
            break;
        case 0x11AA:
            atk_eff += duel.players[p & 1].n_hand * 300;
            def_eff += duel.players[p & 1].n_hand * 300;
            break;
        case 0x11D0:
            atk_eff += duel_card_check_9148(p, 0xFB2) * 500;
            break;
        case 0x129E:
        case 0x154A:                   /* 300 for each 0x1278, 0x142D and 0xFC9 in both graveyards */
            for (q = 0; q < 2; q++) {
                pl = &duel.players[q & 1];
                for (i = 0; i < pl->n_grave; i++) {
                    switch (pl->grave[i].id) {
                    case 0xFC9:
                    case 0x142D:
                    case 0x1278:
                        atk_eff += 300;
                        break;
                    }
                }
            }
            break;
        case 0x12A0:
            atk_eff += duel_count_type(0, 10) * 200 + duel_count_type(1, 10) * 200;
            break;
        case 0x12BB:
            st->atk = SLOT.link & 0xFFFF;
            st->def = SLOT.link >> 16;
            break;
        case 0x1377:
            atk_eff += duel_count_grave_type(1 - p, 1) * 500 + duel_count_type(1 - p, 1) * 500;
            break;
        case 0x15FC:
            for (q = 0; q < 2; q++) {
                atk_eff += duel_count_grave_type(q, 1) * 500 + duel_count_type(q, 1) * 500;
            }
            break;
        case 0x1336:
            atk_eff += duel.players[p & 1].n_hand * -400;
            def_eff += duel.players[p & 1].n_hand * -400;
            break;
        case 0x133D:
            if (duel_count_monsters_special(1 - p) > 0) {
                atk_eff += -1000;
            }
            break;
        case 4999:
            st->atk = duel.players[p & 1].n_hand * 1000;
            st->def = duel.players[p & 1].n_hand * 1000;
            break;
        case 5000:                     /* the slot value holds ATK | DEF << 16, or a bonus with flag bit 3 */
            if (SLOT.flags & 8) {
                atk_eff += SLOT.link;
                def_eff += SLOT.link;
            } else {
                st->atk = SLOT.link & 0xFFFF;
                st->def = SLOT.link >> 16;
            }
            break;
        case 0x13A9:
            if (duel_count_type_skip(p, 10, z) == 0) {
                st->atk = 1000;
                st->def = 1000;
            }
            break;
        case 0x13AD:
            if (SLOT.flags & 0x10) {
                atk_eff += 500;
                def_eff += 500;
            }
            break;
        case 0x13E8:
            atk_eff += duel_count_monsters_special(1 - p) * -200;
            break;
        case 0x1437:
            if (duel_count_faceup_traps(0) != 0 || duel_count_faceup_traps(1) != 0) {
                st->atk = 3000;
                st->def = 3000;
            }
            break;
        case 0x1484:
            if (p == duel_turn_player && duel_phase == 3) {
                atk_eff += 300;
            }
            break;
        case 0x1486:
            if (p != duel_turn_player && duel_phase == 3) {
                atk_eff += 300;
            }
            break;
        case 0x14EC:
            atk_eff += duel_count_grave_type(p, 0x11) * 200;
            break;
        case 0x14FC:                   /* copies the stats of the monster in slot v */
            v = duel_slot_e1854(p, z);
            if (v != 0xFFFF) {
                duel_field_stats(v & 0xFF, v >> 8, &other);
                fixed = 1;
                st->atk = other.atk;
                st->def = other.def;
            }
            break;
        case 0x1570:
            if (duel.players[p & 1].n_hand < 2) {
                atk_eff += 400;
            }
            break;
        case 0x157D:
            atk_eff += duel_count_type_skip(p, 1, z) * 500;
            break;
        case 0x160D:
            atk_eff += duel_count_monsters_fn(p, card_is_in_group_160f) * 100;
            break;
        case 0x160F:
            atk_eff += duel_count_monsters_fn(p, card_is_in_group_160f) * 400;
            break;
        case 0x1615:
            atk_eff += duel_slot_e68a0(p, z) * 200;
            break;
        case 0x1617:
            atk_eff += duel_slot_e68a0(p, z) * 300;
            break;
        case 0x1651:
            atk_eff += duel_count_monsters_special(1 - p) * 500;
            break;
        case 0x1654:
        case 0x1688:
            st->atk = SLOT.link;
            break;
        case 0x16AC:
            atk_eff += duel_count_type_skip(p, 7, z) * 500 + duel_count_type(1 - p, 7) * 500;
            break;
        case 0x16C3:
            st->atk = duel_grave_count_monsters(p) * 300;
            break;
        case 0x16CA:
            st->atk = duel.players[p & 1].n_f * 400;
            st->def = duel.players[p & 1].n_f * 400;
            break;
        case 0x1742:                   /* with field spell 0x175E: the LP lead */
            if (duel.players[p & 1].field[10].card.id == 0x175E) {
                n = duel.players[p & 1].lp - duel.players[(1 - p) & 1].lp;
                if (n < 0) {
                    n = 0;
                }
                atk_eff += n;
                def_eff += n;
            }
            break;
        case 0x1755:
            st->atk = duel_count_type(1 - p, 3) * 1000 + duel_count_type_skip(p, 3, z) * 1000;
            st->def = st->atk;
            break;
        case 0x1789:
            if (duel.players[p & 1].n_hand == 0 && duel_count_field_cards(p, 0, 0, 0) == 0) {
                atk_eff += 1500;
            }
            break;
        case 0x17E1:
            atk_eff += duel_count_attr(1 - p, 4) * 1000 + duel_count_attr_skip(p, 4, z) * 1000;
            break;
        case 0x17E3:
        case 0x1827:
        case 0x1860:
            if (duel_count_attr(p, 4) != 0 || duel_count_attr(1 - p, 4) != 0) {
                atk_eff += 500;
            }
            break;
        case 0x17EB:
            atk_eff += duel.players[p & 1].n_hand * 400;
            def_eff += duel.players[p & 1].n_hand * 400;
            break;
        case 0x1809:
            if (duel_field_flag9(p, z) != 0 && duel_card_check_9148(p, 0x1807) != 0 &&
                duel_card_check_9148(p, 0x180B) != 0 && duel_card_check_9148(p, 0x180C) != 0) {
                st->atk = 3000;
            }
            break;
        case 0x1817:
            atk_eff += duel_slot_e68a0(p, z) * 500;
            break;
        case 0x17C2:
        case 0x1894:
            atk_eff += duel_count_grave_type(p, 1) * 300;
            break;
        case 0x18C3:                   /* 1000 for each 0x18C3 in attack position, or in defense */
            c = (unsigned char *)cnt;
            if (c != 0) {
                for (n = 8; n != 0; n--) {
                    *c++ = 0;
                }
            }
            s = &duel.players[p & 1].field[0];
            for (i = 0; i < 5; i++, s++) {
                if (field_get_card(p, i) == 0x18C3 && s->faceup != 0) {
                    cnt[s->pos]++;
                }
            }
            if (cnt[1] == 0) {
                atk_eff += cnt[0] * 1000;
            }
            if (cnt[0] == 0) {
                def_eff += cnt[1] * 1000;
            }
            break;
        case 0x190F:
            if (duel.players[p & 1].n_hand == 0) {
                atk_eff += 1000;
            }
            break;
        case 0x1943:
        case 0x1A00:
            atk_eff += duel_grave_count_1a6f(p) * 300;
            break;
        case 0x194C:
            atk_eff += duel_count_monsters(1 - p) * 200 + (duel_count_monsters(p) - 1) * -500;
            break;
        case 0x1983:
            atk_eff += duel_slot_e68a0(p, z) * 500;
            break;
        case 0x19A3:
            atk_eff += duel_count_grave_traps(p, 4) * 1000;
            break;
        case 0x19BE:
            n = duel_count_type_skip(p, 7, z) * 100 + duel_count_type(1 - p, 7) * 100;
            atk_eff += n;
            def_eff += n;
            break;
        case 0x19C4:
            st->atk = (duel_count_attr9(0) + duel_count_attr9(1)) * 500;
            st->def = st->atk;
            break;
        case 0x19CC:
            atk_eff += duel_grave_count_id(p, 0x1919) * 300;
            break;
        case 0x19D1:
            atk_eff += duel_grave_count_id(p, 0x19D1) * 500;
            break;
        case 0x19D3:
            if (SLOT.link & 1) {
                atk_eff += 1000;
            }
            break;
        case 0x19D6:
            atk_eff += duel_grave_count_id(p, 0x19CB) * 500;
            break;
        case 0x19EF:                   /* 300 for each opposing monster sharing a kind */
            n = 0;
            s = &duel.players[(1 - p) & 1].field[0];
            for (i = 0; i < 5; i++, s++) {
                if (s->card.id != 0 && s->faceup != 0 && (st->mask & duel_field_attrs(1 - p, i))) {
                    n++;
                }
            }
            atk_eff += n * 300;
            break;
        case 0x19F5:
            atk_eff += duel.players[p & 1].n_f * 300;
            def_eff += duel.players[p & 1].n_f * 300;
            break;
        case 0x19F6:
            st->atk = (duel_zone_f_count_monsters(0) + duel_zone_f_count_monsters(1)) * 100;
            st->def = st->atk;
            break;
        case 0x19F7:
            st->atk = (duel_zone_f_count_monsters(0) + duel_zone_f_count_monsters(1)) * 200;
            st->def = st->atk;
            break;
        case 0x19F8:
            st->atk = (duel_zone_f_count_monsters(0) + duel_zone_f_count_monsters(1)) * 300;
            st->def = st->atk;
            break;
        case 0x1A05:                   /* the base ATK of the other 0x1A6x monsters it controls */
            st->atk = 0;
            s = &duel.players[p & 1].field[0];
            for (i = 0; i < 5; i++, s++) {
                if (i != z && s->card.id != 0 && s->faceup != 0 &&
                    card_is_in_group_1a6x(field_get_card(p, i))) {
                    st->atk += duel_field_base_atk(p, i);
                }
            }
            st->def = st->atk;
            break;
        case 0x1A6F:
            atk_eff += (duel_count_field_all(0) + duel_count_field_all(1)) * 400;
            break;
        case 0x1A75:
            atk_eff += duel_count_field_all(1 - p) * 400;
            break;
        }
    }
    if (st->id == 0x19ED) {
        fixed = 1;
    } else if (st->id == 0x1782 && !(st->imm & 1)) {
        for (i = 5; i < 11; i++) {
            if (duel_slot_e8170(p, i, 0x1843) && duel_slot_has_effect(p, i, 0x1843)) {
                st->atk = 3000;
            }
        }
    }

    /* bonuses given by the cards on both fields */
    for (q = 0; q < 2; q++) {
        s = &duel.players[q & 1].field[0];
        for (i = 0; i < 10; i++, s++) {
            if (s->card.id != 0 && SLOT_ACTIVE(s)) {
                b = duel_card_bonus(st, q, i, p, z);
                atk_bon += (short)b;
                def_bon += (short)(b >> 16);
            }
        }
    }
    if (card_is_in_group_1aad(st->id)) {
        atk_bon += duel_grave_count_id_b21(p, 0x1AAA) * 300;
    }
    if (card_is_in_group_1b2d(st->id)) {
        atk_bon += duel_grave_count_id_b21(0, 0x1B2A) * 300 + duel_grave_count_id_b21(1, 0x1B2A) * 300;
    }

    /* field spells; a bonus the monster ignores stays pending */
    atk_fld = 0;
    def_fld = 0;
    for (q = 0; q < 2; q++) {
        s = &duel.players[q & 1].field[10];
        if (s->card.id != 0 && SLOT_ACTIVE(s) && IMMUNE(st->imm, p, q) == 0) {
            b = duel_field_bonus(st, q, 10, p, z);
            atk_fld += (short)b;
            def_fld += (short)(b >> 16);
            if (duel_st_affects(p, z, q, 10) == 0) {
                atk_bon += atk_fld;
                atk_fld = 0;
                def_bon += def_fld;
                def_fld = 0;
            }
        }
    }

    /* effect entries attached to the slot */
    for (e = SLOT.effects; e != 0;) {
        ef = &duel_effects[e];
        src = ef->src;
        arg = ef->arg;
        e = ef->next;
        kind = ef->kind & 0xF;
        sp = src & 0xFF;
        sz = src >> 8;
        switch (kind) {
        case 1:
        case 2:
        case 3:
        case 4:                        /* changes by card: src is the card id */
            switch (src) {
            case 0x1482:
            case 0x1492:
            case 0x1520:
            case 0x16C4:
            case 0x16CE:
            case 0x1737:
            case 0x17BC:
            case 0x1835:
            case 0x1882:
            case 0x18CA:
            case 0x1945:
            case 0x1996:
            case 0x19A5:
                atk_eff += arg;
                break;
            case 0x1A7A:
                atk_eff += arg;
                break;
            case 0x1A1C:
                atk_eff += 400;
                break;
            case 0x1A32:
                atk_eff += 400;
                break;
            case 0x12F1:
            case 0x1397:
            case 0x147A:
            case 0x1645:
            case 0x1704:
            case 0x1862:
            case 0x188F:
            case 0x1875:
            case 0x18AE:
            case 0x1941:
            case 0x1952:
                atk_eff += 500;
                break;
            case 0x1A0F:
                atk_eff += 500;
                break;
            case 0x10AC:
            case 0x1329:
            case 0x154D:
                atk_eff += 700;
                break;
            case 0x172B:
                atk_eff += 700;
                break;
            case 0x14A7:
                atk_eff += 800;
                break;
            case 0x1A60:
                atk_eff += 800;
                break;
            case 0x1184:
            case 0x12AC:
            case 0x1507:
            case 0x1508:
            case 0x1690:
            case 0x17ED:
            case 0x1A1D:
            case 0x164D:
            case 0x182D:
                atk_eff += 1000;
                break;
            case 0x1A61:
                atk_eff += 1000;
                break;
            case 0x192D:
                atk_eff += 1500;
                break;
            case 0x17C9:
                atk_eff += 3000;
                break;
            case 0x13EE:
            case 0x1446:
            case 0x14A6:
                atk_eff -= arg;
                break;
            case 0x19B4:
                atk_eff -= arg;
                break;
            case 0x147D:
                atk_eff += -200;
                break;
            case 0x154C:
                atk_eff += -400;
                break;
            case 0x13CB:
            case 0x160E:
                atk_eff += -500;
                break;
            case 0x1644:
                atk_eff += -500;
                break;
            case 0x157B:
                atk_eff += -800;
                break;
            case 0x12F2:
                def_eff += 500;
                break;
            case 0x132A:
                def_eff += 700;
                break;
            case 0x1330:
                def_eff += -500;
                break;
            case 0x12CF:
            case 0x16E3:
                atk_eff += arg;
                def_eff += arg;
                break;
            case 0x1009:
                atk_eff += 100;
                def_eff += 100;
                break;
            case 0x1527:
            case 0x19F7:
                atk_eff += 300;
                def_eff += 300;
                break;
            case 0x19F8:
                atk_eff += 500;
                def_eff += 500;
                break;
            case 0x14B9:
                atk_eff += 700;
                def_eff += 700;
                break;
            case 0x14D7:
                atk_eff += 1000;
                def_eff += 1000;
                break;
            case 0x12D0:
            case 0x14BE:
                atk_eff += -arg;
                def_eff += -arg;
                break;
            case 0x15A6:
                atk_eff += -500;
                def_eff += -500;
                break;
            case 0x167B:
            case 0x1763:               /* DEF set outright */
                st->def = arg;
                def_bon = 0;
                def_eq = 0;
                def_eff = 0;
                break;
            case 0x1863:
                st->def = arg;
                def_bon = 0;
                def_eq = 0;
                def_eff = 0;
                break;
            case 0x17B3:
                st->def = 0;
                break;
            case 0x1819:
                if (arg != 0) {
                    st->atk = 3000;
                    atk_eff = 0;
                }
                break;
            case 0x1840:
                st->atk = 2700;
                atk_eff = 0;
                break;
            case 0x190E:
                st->atk = 2000;
                atk_bon = 0;
                atk_eq = 0;
                atk_eff = 0;
                break;
            case 0x10C6:               /* DEF halved */
                if (!fixed) {
                    n = def_eq + st->def + def_eff;
                    def_eq = 0;
                    def_eff = 0;
                    st->def = (def_bon + n + 1) >> 1;
                    def_bon = 0;
                }
                break;
            case 0x1153:               /* ATK halved (arg 0) or doubled */
                if (!fixed) {
                    if (arg == 0) {
                        st->atk = (atk_bon + atk_eq + st->atk + atk_eff + 1) >> 1;
                    } else {
                        st->atk = (atk_bon + atk_eq + st->atk + atk_eff) * 2;
                    }
                    atk_bon = 0;
                    atk_eq = 0;
                    atk_eff = 0;
                }
                break;
            case 0x1337:
                if (!fixed) {
                    st->atk = st->batk << 1;
                }
                break;
            case 0x1391:               /* arg: value / 10 << 1 | 1 to add instead of set */
                n = (arg >> 1) * 10;
                if (arg & 1) {
                    atk_eff += n;
                } else if (!fixed) {
                    st->atk = n;
                    atk_bon = 0;
                    atk_eq = 0;
                    atk_eff = 0;
                }
                break;
            case 0x1409:
            case 0x1445:
                if (!fixed) {
                    st->atk = arg * 10;
                    atk_bon = 0;
                    atk_eq = 0;
                    atk_eff = 0;
                }
                break;
            case 0x14EB:
                if (!fixed) {
                    st->atk = (st->batk + 1) >> 1;
                }
                break;
            case 0x15F8:
                if (!fixed) {
                    st->atk = (st->batk + 1) >> 1;
                }
                break;
            case 0x19AB:
            case 0x1A31:               /* ATK set outright */
                if (!fixed) {
                    st->atk = arg;
                    atk_bon = 0;
                    atk_eq = 0;
                    atk_eff = 0;
                }
                break;
            case 0x13AD:
            case 0x13B0:
            case 0x13B4:
                if (kind == 3) {
                    atk_eff += -500;
                    def_eff += -500;
                }
                break;
            case 0x13B2:
                if (kind == 3) {
                    atk_eff += 1000;
                }
                break;
            case 0x153D:
                if (arg == 2) {
                    def_eff += 500;
                } else if (arg == 1) {
                    atk_eff += 200;
                }
                break;
            case 0x17AB:
                st->atk += neg == 0 ? 1000 : -1000;
                st->def += neg == 0 ? 1000 : -1000;
                break;
            case 0x17AC:
                st->atk += neg == 0 ? 2000 : -2000;
                st->def += neg == 0 ? 2000 : -2000;
                break;
            case 0x1890:
                if (arg != 1 && duel_phase == 3) {
                    atk_eff += arg;
                }
                break;
            case 0x18F2:
                if (arg == 0) {
                    atk_eff += -500;
                } else {
                    atk_eff += 500;
                }
                break;
            case 0x18FE:
                if (!(st->imm & 1)) {
                    atk_eff += st->batk;
                }
                break;
            }
            break;
        case 5:
            if (src == 0x13A6) {
                st->atk = 1000;
                st->def = 1000;
            }
            break;
        case 6:
        case 7:
        case 8:
        case 9:                        /* cards on the field acting on this monster: src is the card slot */
            pl = &duel.players[src & 1];
            s = &pl->field[sz];
            if (s->card.id != 0 && !SLOT_NEGATED(s)) {
                switch (field_get_card(sp, sz)) {
                case 0xFF9:
                    if (st->attr == 2) {
                        atk_bon += 200;
                        def_bon += 200;
                    }
                    break;
                case 0x1243:
                    if (!(st->imm2 & 2)) {
                        atk_bon += -700;
                    }
                    break;
                case 0x13F9:
                    if (!(st->imm2 & 2)) {
                        st->atk = 0;
                        atk_bon = 0;
                        atk_eq = 0;
                        atk_eff = 0;
                    }
                    break;
                case 0x175D:           /* the next two entries hold the new ATK and DEF */
                    if (!fixed && !(st->imm & 1)) {
                        e1 = &duel_effects[ef->next];
                        e2 = &duel_effects[e1->next];
                        if (e1->src == 0x175D && e2->src == 0x175D) {
                            atk_bon = 0;
                            atk_eq = 0;
                            st->atk = e1->arg;
                            atk_eff = 0;
                            def_bon = 0;
                            st->def = e2->arg;
                            def_eq = 0;
                            def_eff = 0;
                        }
                    }
                    break;
                case 0x180F:
                    if (IMMUNE(st->imm2, p, sp) == 0) {
                        atk_bon += duel_slot_value_1909(sp, sz) * -500;
                    }
                    break;
                case 0x1843:
                    if (p == sp && IMMUNE(st->imm, p, sp) == 0) {
                        st->atk = 3000;
                        atk_eff = 0;
                    }
                    break;
                case 0x184B:
                    if (IMMUNE(st->imm2, p, sp) == 0) {
                        atk_bon += 500;
                    }
                    break;
                case 0x18D3:           /* 700 DEF when its controller has 2+ monsters, all in defense */
                    if (!(st->imm & 1)) {
                        n = 0;
                        all = 1;
                        for (i = 0; i < 5; i++) {
                            if (pl->field[i].card.id != 0) {
                                n++;
                                if (pl->field[i].pos == 0) {
                                    all = 0;
                                }
                            }
                        }
                        if (n > 1 && all) {
                            def_bon += 700;
                        }
                    }
                    break;
                case 0x1A7C:
                    if (IMMUNE(st->imm, p, sp) == 0) {
                        atk_bon += arg;
                    }
                    break;
                }
                if (card_get_type(s->card.id) == 2) {
                    atk_bon += atk_fld;
                    atk_fld = 0;
                    def_bon += def_fld;
                    def_fld = 0;
                }
            }
            break;
        case 10:
        case 11:                       /* equips (11: equip spell) */
            pl = &duel.players[src & 1];
            s = &pl->field[sz];
            id = s->card.id;
            if (id != 0) {
                if (s->faceup != 0 && !SLOT_NEGATED(s)) {
                    if (card_get_prop5_hi(id) == 0x17) {
                        v = IMMUNE(st->imm2, p, sp);
                    } else {
                        v = IMMUNE(st->imm, p, sp);
                    }
                    if (v != 0) {
                        break;
                    }
                    switch (id) {
                    case 0x1496:
                    case 0x165C:
                        atk_eq += 300;
                        break;
                    case 0x193A:
                        atk_eq += 300;
                        break;
                    case 0x1625:
                        atk_eq += 400;
                        break;
                    case 0x10DD:
                    case 0x10E3:
                    case 0x1231:
                    case 0x1514:
                    case 0x14ED:
                    case 0x165E:
                    case 0x165F:
                    case 0x19B0:
                        atk_eq += 500;
                        break;
                    case 0x1A69:
                        atk_eq += 500;
                        break;
                    case 0x10D8:
                    case 0x10E7:
                    case 0x1234:
                    case 0x1242:
                    case 0x1392:
                    case 0x1467:
                        atk_eq += 700;
                        break;
                    case 0x16CC:
                        atk_eq += 700;
                        break;
                    case 0x13F6:
                    case 0x150C:
                    case 0x1989:
                        atk_eq += 800;
                        break;
                    case 0x19D8:
                        atk_eq += 800;
                        break;
                    case 0x10D6:
                        atk_eq += 1000;
                        break;
                    case 0x143C:
                        atk_eq += s->link;
                        break;
                    case 0x1660:
                        def_eq += 500;
                        break;
                    case 0x10DF:
                        def_eq += 800;
                        break;
                    case 0x10D3:
                    case 0x10D5:
                    case 0x10D7:
                    case 0x10DA:
                    case 0x10DC:
                    case 0x10DE:
                    case 0x10E1:
                    case 0x10E2:
                    case 0x10E5:
                    case 0x10E8:
                    case 0x10E9:
                    case 0x10EB:
                    case 0x10EC:
                    case 0x10ED:
                    case 0x10EE:
                    case 0x1238:
                        atk_eq += 300;
                        def_eq += 300;
                        break;
                    case 0x1309:
                        atk_eq += 500;
                        def_eq += 500;
                        break;
                    case 0x10E0:
                        atk_eq += 700;
                        def_eq += 700;
                        break;
                    case 0x17FA:
                    case 0x17FB:
                        atk_eq += 1500;
                        def_eq += 1500;
                        break;
                    case 0x1842:
                        atk_eq += -300;
                        break;
                    case 0x13E3:
                        atk_eq += -500;
                        break;
                    case 0x1661:
                        atk_eq += -500;
                        break;
                    case 0x165D:
                        atk_eq += -1000;
                        break;
                    case 0x1318:
                        atk_eq += -500;
                        def_eq += -500;
                        break;
                    case 0x10D4:
                    case 0x10D9:
                    case 0x10DB:
                    case 0x10EA:
                    case 0x12EE:
                    case 0x12EF:
                        atk_eq += 400;
                        def_eq += -200;
                        break;
                    case 0x13F4:
                        atk_eq += 1000;
                        def_eq += -1000;
                        break;
                    case 0x1237:       /* ATK doubled while behind on LP, halved while ahead */
                        if (!fixed) {
                            if (pl->lp < duel.players[(1 - sp) & 1].lp) {
                                st->atk = st->batk << 1;
                            }
                            if (duel.players[(1 - sp) & 1].lp < pl->lp) {
                                st->atk = (st->batk + 1) >> 1;
                            }
                        }
                        break;
                    case 0x12C6:
                        if (st->attr == 7) {
                            if (s->link == 2) {
                                def_eq += 700;
                            } else if (s->link == 1) {
                                atk_eq += 700;
                            }
                        }
                        break;
                    case 0x130D:
                        if (st->attr != 7) {
                            atk_eq += duel_slot_value_1909(sp, sz) * -300;
                        }
                        break;
                    case 0x131A:
                        atk_eq += duel_slot_value_1909(sp, sz) * -200 + 700;
                        break;
                    case 0x145C:
                        atk_eq += duel_count_monsters_pos(sp, 1, 0) * 800;
                        def_eq += duel_count_monsters_pos(sp, 1, 0) * 800;
                        break;
                    case 0x145D:
                        atk_eq += duel_count_field_all(sp) * 500;
                        def_eq += duel_count_field_all(sp) * 500;
                        break;
                    case 0x149F:
                        if (duel_field_type_ex(p, z, (short)s->unk4) == 0xF) {
                            atk_eq += 500;
                            def_eq += 500;
                        }
                        break;
                    case 0x17A0:
                        atk_eq += s->link * 1000;
                        break;
                    case 0x1959:
                        atk_eq += duel_count_same_name_other(p, z) * 800;
                        break;
                    case 0x15B0:
                    case 0x15CE:
                    case 0x194B:
                        if (kind == 0xB) {
                            atk_eq += 400;
                            def_eq += 400;
                        }
                        break;
                    case 0x15B3:
                        if (kind == 0xB) {
                            atk_eq += 600;
                            def_eq += 600;
                        }
                        break;
                    case 0x15CF:
                        if (kind == 0xB) {
                            atk_eq += 900;
                            def_eq += 900;
                        }
                        break;
                    case 0x15D1:
                    case 0x1825:
                        if (kind == 0xB) {
                            atk_eq += 500;
                            def_eq += 500;
                        }
                        break;
                    case 0x1620:
                        atk_eq += 3000;
                        break;
                    case 0x1917:
                        if (kind == 0xB) {
                            atk_eq += 2500;
                        }
                        break;
                    }
                }
            }
            if (st->id == 0x10BC && s->card.id == 0xFEE) {
                st->atk = 0;
                st->def = 2000;
            }
            break;
        case 0xD:                      /* 0x1284/0x1281 take the stats of the card in slot src */
            if (st->id == 0x1284 || st->id == 0x1281) {
                s = &duel.players[src & 1].field[sz];
                if (s->faceup == 0) {
                    st->atk = 0;
                    st->def = 0;
                } else {
                    id = s->card.id;
                    switch (id) {
                    case 0x1472:
                        st->atk = 1600;
                        st->def = 1800;
                        break;
                    case 0x1636:
                        st->atk = 0;
                        st->def = 3000;
                        break;
                    case 0x172F:
                        st->atk = 1800;
                        st->def = 500;
                        break;
                    case 0x1809:
                        st->atk = 0;
                        st->def = 2000;
                        break;
                    default:
                        st->atk = card_get_atk(id);
                        st->def = card_get_def(id);
                        break;
                    }
                }
                def_eff = 0;
                atk_eff = 0;
            }
            break;
        case 0xC:
            s = &duel.players[src & 1].field[sz];
            if (s->card.id != 0 && !SLOT_NEGATED(s) && IMMUNE(st->imm, p, sp) == 0) {
                atk_eq += 200;
            }
            break;
        case 0:
            break;
        }
    }

    if (!fixed) {
        if (neg == 0) {
            st->atk += atk_bon + atk_eff + atk_eq;
            st->def += def_bon + def_eff + def_eq;
        } else {
            st->atk -= atk_bon + atk_eff + atk_eq;
            st->def -= def_bon + def_eff + def_eq;
        }
        if (duel_count_active(0x1951)) {
            if (st->attr == 4) {
                st->atk = 0;
            } else if (st->mask & 0x10) {
                st->atk = 0;
            }
        }
        if (duel_slot_e8170(p, z, 0x1955) && duel_count_monsters_special(1 - p) == 2) {
            st->atk <<= 1;
        }
        n = duel_slot_affected_ex(p, z, 0x1381);
        for (i = 0; i < n; i++) {
            st->atk = (st->atk + 1) >> 1;
        }
        n = duel_find_effect_card(1 - p, 0x1905, -1) + duel_find_effect_card(p, 0x1905, z);
        for (i = 0; i < n; i++) {
            st->atk = (st->atk + 1) >> 1;
            st->def = (st->def + 1) >> 1;
        }
    }
    if (st->atk < 0) {
        st->atk = 0;
    }
    if (st->def < 0) {
        st->def = 0;
    }
}
#endif
