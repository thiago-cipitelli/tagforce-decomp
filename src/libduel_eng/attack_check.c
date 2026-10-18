// @range 0x088EC5D8-0x088ED2E0
/* Monster z of player p may attack now: protections, effects that stop attacks, card-specific
   conditions, and (when `arg`) the conditions that depend on its stats and on life points. */
#include "duel.h"
#include "card_db.h"

extern unsigned int duel_view_player;
extern int duel_err;

int duel_count_active(int id);
int duel_count_field(unsigned int p);
int duel_count_type_skip(unsigned int p, unsigned int type, int skip);
int duel_count_tributable_except(unsigned int p, unsigned int except);
int field_get_card(unsigned int p, int z);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
int duel_field_protected(unsigned int p, int z);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_has_effect2(unsigned int p, int z, int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_slot_effect_arg(unsigned int p, int z, int id);
int duel_slot_effect_kind(unsigned int p, int z, int id, int kind);
int duel_slot_affected(unsigned int p, int z, int id);
int duel_slot_affected_ex(unsigned int p, int z, int id);
unsigned int duel_slot_immunity(unsigned int p, int z);
unsigned int duel_slot_ref(unsigned int p, int z);
int duel_effect_applies(int id, unsigned int p, unsigned int ref);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_count_card_state(unsigned int p, int id, int state);
int duel_has_effect_atk(unsigned int p, int id, int atk);
int duel_has_effect_attr(unsigned int p, int id, int attr);
int duel_has_attack_target(unsigned int p);
int duel_direct_attack_allowed(unsigned int p, int z);

#define BITS_16_18(f) ((f) >> 16 & 1 || (f) >> 17 & 1 || (f) >> 18 & 1)

int duel_attack_check(unsigned int p, int z, int arg)
{
    DuelStats st;
    int id = field_get_card(p, z);
    int free = !(duel.players[p & 1].field[z].flags >> 5 & 1);
    unsigned int imm;
    int atk;
    unsigned int ref;
    unsigned int q;
    int i;
    int bonus;
    int attr;
    int level;

    if (duel.players[p & 1].field[z].flags >> 24 & 1) {
        goto fail;
    }
    if (duel_field_protected(p, z)) {
        if (p == duel_view_player) {
            duel_err = 0x10;
        }
        return 0;
    }
    if (duel_find_effect_card(p, 0x1A79, -1) && (id != 0x1A79 || duel.players[p & 1].field[z].link == 0)) {
        for (i = 0; i <= 4; i++) {
            if (i != z && field_get_card(p, i) == 0x1A79 && duel.players[p & 1].field[i].link != 0) {
                goto fail;
            }
        }
    }
    duel_field_stats(p, z, &st);
    atk = st.atk;
    attr = st.attr;
    level = st.level;
    imm = duel_slot_immunity(p, z);
    ref = duel_slot_ref(p, z);
    if (duel_slot_affected_ex(p, z, 0x149D) || duel_slot_affected_ex(p, z, 0x13F3) ||
        duel_slot_affected_ex(p, z, 0x1842) || duel_slot_affected_ex(p, z, 0x1103) ||
        duel_slot_affected_ex(p, z, 0x1243) || duel_slot_affected_ex(p, z, 0x14B2) ||
        ((int)(ref & 2) <= 0 && duel_has_effect_atk(1 - p, 0x1766, atk)) ||
        ((int)(imm & 1) <= 0 && duel_count_card_state(p, 0x1400, 2)) ||
        ((int)(imm & 2) <= 0 && duel_count_card_state(1 - p, 0x1400, 2)) ||
        (duel_effect_applies(0x14A1, p, imm) && BITS_16_18(duel.players[p & 1].field[z].flags)) ||
        (level >= 4 && duel_effect_applies(0x140E, p, ref))) {
        goto fail;
    }
    if (duel_count_active(0x1469)) {
        int u = duel_slot_effect_arg(p, 11, 0x1469);

        if (u >= 0 && u != CARD_UID(duel.players[p & 1].field[z].card)) {
            goto fail;
        }
    }
    if (duel_slot_affected(p, z, 0x1280) || duel_slot_affected(p, z, 0x128A) || duel_slot_affected(p, z, 0x1743) ||
        duel_count_active(0x1284) - (id == 0x1284) > 0) {
        goto fail;
    }
    q = 1 - p;
    if ((duel_find_effect_card(q, 0x1865, -1) && BITS_16_18(duel.players[p & 1].field[z].flags)) || duel_slot_has_effect(p, z, 0x1944) ||
        duel_slot_has_effect(p, z, 0x1208) || duel_slot_has_effect(p, z, 0x15ED) ||
        duel_slot_has_effect(p, z, 0x156A) || duel_slot_has_effect(p, z, 0x1900) ||
        duel_slot_has_effect(p, z, 0x14F7) || duel_slot_has_effect(p, z, 0x1A7B) ||
        duel_slot_effect_with(p, z, 0x1819, 0) || duel_slot_effect_with(p, z, 0x1890, 1) ||
        (duel_slot_has_effect(p, 11, 0x15FF) && duel_slot_has_effect(p, z, 0x15FF) == 0) ||
        (duel_slot_has_effect(p, 11, 0x195B) &&
         CARD_UID(duel.players[p & 1].field[z].card) != duel_slot_effect_arg(p, 11, 0x195B)) ||
        duel_slot_effect_kind(p, 11, 0x1A02, 2) || ((int)(imm & 1) <= 0 && duel_slot_has_effect(p, 11, 0x1A0D))) {
        goto fail;
    }
    switch (id) {
    case 0xFA6:
    case 0xFA7:
        if (duel_slot_has_effect(p, z, 0x175B) == 0 && duel_slot_has_effect(p, 11, 0x175B)) {
            goto fail;
        }
        if (duel_slot_has_effect(p, 11, 0x15E0)) {
            goto fail;
        }
        break;
    case 0xFF8:
        if (duel_slot_has_effect(p, 11, 0x17F6)) {
            goto fail;
        }
        break;
    case 0x127D:
    case 0x127E:
    case 0x127F:
    case 0x12A5:
    case 0x1563:
    case 0x1564:
    case 0x1565:
    case 0x1566:
        if (free && BITS_16_18(duel.players[p & 1].field[z].flags)) {
            goto fail;
        }
        break;
    case 0x1622:
        if (free) {
            if (duel_count_field(p) >= 2) {
                goto fail;
            }
            if (duel.players[p & 1].n_hand != 0) {
                goto fail;
            }
        }
        break;
    case 0x1636:
    case 0x1639:
        if (id != duel.players[p & 1].field[z].card.id) {
            break;
        }
        goto fail;
    case 0x16C9:
    case 0x16CB:
    case 0x19C8:
        if (duel_slot_has_effect(p, z, 0xFB6)) {
            goto fail;
        }
        break;
    case 0x161B:
        if (free && BITS_16_18(duel.players[p & 1].field[z].flags)) {
            goto fail;
        }
        break;
    case 0x17C7:
    case 0x17C8:
        if (free && (duel.players[p & 1].field[z].flags >> 16 & 1 || duel.players[p & 1].field[z].flags >> 18 & 1)) {
            goto fail;
        }
        break;
    }
    if (duel_has_attack_target(q) == 0 && duel_direct_attack_allowed(p, z) == 0) {
        goto fail;
    }
    if (arg != 0) {
        if (((int)(imm & 2) <= 0 && duel_count_card_state(q, 0x1102, 3)) ||
            (attr != 7 && duel_slot_affected_ex(p, z, 0x130E)) ||
            (atk >= 1500 && duel_effect_applies(0x134A, p, imm)) ||
            (attr == 10 && (int)(imm & 2) <= 0 && duel_find_effect_card(q, 0x12E0, -1)) ||
            duel_slot_has_effect(p, z, 0x12D1) ||
            (duel_effect_applies(0x14D1, p, imm) && BITS_16_18(duel.players[p & 1].field[z].flags) &&
             (attr == duel.players[p & 1].field[10].link || attr == duel.players[q & 1].field[10].link)) ||
            ((int)(ref & 1) <= 0 && duel_has_effect_attr(p, 0x1358, attr)) ||
            ((int)(ref & 2) <= 0 && duel_has_effect_attr(q, 0x1358, attr))) {
            goto fail;
        }
        if ((int)(ref & 2) <= 0 && duel_find_effect_card(q, 0x1A19, -1)) {
            for (i = 1; i <= 6; i++) {
                if ((st.mask & 1 << i) && duel_has_effect_attr(q, 0x1A19, i)) {
                    goto fail;
                }
            }
        }
        if ((atk >= 1900 && duel_count_active(0x1523)) || duel_slot_has_effect(p, z, 0x182C) ||
            duel_slot_has_effect(p, 11, 0x1886) || duel_slot_has_effect2(p, 11, 0x172D) || id == 0x195A ||
            id == 0x1815 || id == 0x1813) {
            goto fail;
        }
        switch (id) {
        case 0x1441:
            if (free) {
                goto fail;
            }
            break;
        case 0x14DB:
            if (free && duel_count_type_skip(p, 1, z) == 0) {
                goto fail;
            }
            break;
        case 0x1AA0:
            if (duel_slot_has_effect(p, z, 0xFB6)) {
                goto fail;
            }
            break;
        }
    }
    if (arg != 0) {
        bonus = 0;
        if (free) {
            switch (id) {
            case 0x11CD:
            case 0x18A4:
                bonus = 1000;
                break;
            case 0x127D:
            case 0x127E:
            case 0x127F:
            case 0x12A5:
                bonus = 500;
                break;
            case 0x128F:
            case 0x12A0:
                if (duel_count_tributable_except(p, (unsigned short)((unsigned char)p | (unsigned char)z << 8)) == 0) {
                    return 0;
                }
                break;
            }
        }
        if (((int)(imm & 2) > 0 || (unsigned int)duel.players[p & 1].n_d >= (unsigned int)duel_find_effect_card(q, 0x131D, -1)) &&
            bonus + duel_effect_applies(0x1320, p, imm) * 500 < duel.players[p & 1].lp) {
            goto ok;
        }
        goto fail;
    }
ok:
    return 1;
fail:
    if (p == duel_view_player) {
        duel_err = 0x13;
    }
    return 0;
}
