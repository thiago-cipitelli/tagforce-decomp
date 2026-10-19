// @range 0x088DEB2C-0x088DFFC0
// @asm 0x088DFAC8-0x088DFC10
// @asm 0x088DFE9C-0x088DFFC0
/* Rule checks for a player: cards that forbid drawing, summoning or activating, cards that can be
   used from the hand, locks on both players, who must answer now. */
#include "duel.h"

int duel_count_active(int id);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_has_effect2(unsigned int p, int z, int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_card_check_9148(unsigned int p, int id);
int duel_has_st_12be(unsigned int p);
int duel_req_flag(int i);
int duel_msg_busy(void);
int card_is_in_group_1a58(int id);
int card_attribute_icon_in_2_3(int id);
int card_id_remap_164a(int id);
int card_is_attribute6(int id);
int card_is_monster(int id);
int duel_can_special_summon(unsigned int p);
int duel_card_protected(int id, int uid);
unsigned int duel_hand_locked(unsigned int p);

typedef struct DuelListCard {
    unsigned short id : 13;
    unsigned short placed : 2;
    unsigned short b15 : 1;
    unsigned short effects;
} DuelListCard;

extern DuelListCard duel_cards[512];
extern int duel_player_type[2];
extern unsigned int duel_view_player;
extern unsigned int duel_turn_player;
extern unsigned char duel_msg_state;
extern int duel_busy_0891523C;
extern int duel_busy_08915238;

/* Card i places after zone z of player p, as a pointer, an id and an instance id. */
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i)
{
    switch (z) {
    case 0xC:
        return &duel.players[p & 1].zone_c[i];
    case 0xD:
        return &duel.players[p & 1].zone_d[i];
    case 0xE:
        return &duel.players[p & 1].grave[i];
    case 0xF:
        return &duel.players[p & 1].zone_f[i];
    case 0xB:
        return &duel.players[p & 1].hand[i];
    }
    if (z + i > 10) {
        return &(p == duel_special.player[0] && z == duel_special.zone[0] ? &duel_special.slot[0] : &duel_special.slot[1])->card;
    }
    return &duel.players[p & 1].field[z + i].card;
}

unsigned short duel_zone_card_id(unsigned int p, int z, int i)
{
    switch (z) {
    case 0xC:
        return duel.players[p & 1].zone_c[i].id;
    case 0xD:
        return duel.players[p & 1].zone_d[i].id;
    case 0xE:
        return duel.players[p & 1].grave[i].id;
    case 0xF:
        return duel.players[p & 1].zone_f[i].id;
    case 0xB:
        return duel.players[p & 1].hand[i].id;
    }
    if (z + i > 10) {
        return (p == duel_special.player[0] && z == duel_special.zone[0] ? &duel_special.slot[0] : &duel_special.slot[1])->card.id;
    }
    return duel.players[p & 1].field[z + i].card.id;
}

int duel_zone_card_uid(unsigned int p, int z, int i)
{
    switch (z) {
    case 0xC:
        return CARD_UID(duel.players[p & 1].zone_c[i]);
    case 0xD:
        return CARD_UID(duel.players[p & 1].zone_d[i]);
    case 0xE:
        return CARD_UID(duel.players[p & 1].grave[i]);
    case 0xF:
        return CARD_UID(duel.players[p & 1].zone_f[i]);
    case 0xB:
        return CARD_UID(duel.players[p & 1].hand[i]);
    }
    if (z + i > 10) {
        DuelSlot *s = p == duel_special.player[0] && z == duel_special.zone[0] ? &duel_special.slot[0] : &duel_special.slot[1];

        return CARD_UID(s->card);
    }
    return CARD_UID(duel.players[p & 1].field[z + i].card);
}

/* Position of the card i places after zone z of player p (0 for the piles). */
unsigned short duel_zone_pos(unsigned int p, int z, int i)
{
    switch (z) {
    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF:
    case 0xB:
        return 0;
    }
    if (z + i > 10) {
        return (p == duel_special.player[0] && z == duel_special.zone[0] ? &duel_special.slot[0] : &duel_special.slot[1])->pos;
    }
    return duel.players[p & 1].field[z + i].pos;
}

/* Card i of zone z of player p is visible to the viewer. */
unsigned int duel_zone_faceup(unsigned int p, int z, int i)
{
    unsigned int m;

    switch (z) {
    case 0xC:
        return 0;
    case 0xD:
        return (duel.mode >> 2 & 1) ^ duel_slot_effect_with(p, 11, 0x12A1, CARD_UID(duel.players[p & 1].zone_d[i]));
    case 0xE:
        return 1;
    case 0xF:
        m = duel.players[p & 1].marks[i];
        switch (m & 0xFF) {
        case 0x80:
            return 0;
        case 0x40:
            return (unsigned char)((unsigned short)m >> 8) >> 1 & 1;
        default:
            return 1;
        }
    case 0xB:
        return p == duel_view_player || duel_hand_locked(p) != 0;
    }
    if (z + i > 10) {
        return (p == duel_special.player[0] && z == duel_special.zone[0] ? &duel_special.slot[0] : &duel_special.slot[1])->faceup;
    }
    return duel.players[p & 1].field[z + i].faceup;
}

/* Number of cards in zone z of player p (0 or 1 on the field). */
unsigned int duel_zone_count(unsigned int p, int z)
{
    switch (z) {
    case 0xC:
        return duel.players[p & 1].n_18;
    case 0xD:
        return duel.players[p & 1].n_d;
    case 0xE:
        return duel.players[p & 1].n_grave;
    case 0xF:
        return duel.players[p & 1].n_f;
    case 0xB:
        return duel.players[p & 1].n_hand;
    }
    return duel.players[p & 1].field[z].card.id != 0;
}

void duel_mode_set(unsigned int bit, int on)
{
    if (on != 0) {
        duel.mode |= 1 << bit;
    } else {
        duel.mode &= ~(1 << bit);
    }
}

void duel_player_flag_set(unsigned int p, unsigned int bit, int on)
{
    if (on != 0) {
        duel.players[p & 1].flags |= 1 << bit;
    } else {
        duel.players[p & 1].flags &= ~(1 << bit);
    }
}

void duel_088DF554(unsigned int p, int z, unsigned int bit, int on)
{
    if (on != 0) {
        duel.players[p & 1].field[z].flags |= 1 << bit;
    } else {
        duel.players[p & 1].field[z].flags &= ~(1 << bit);
    }
}

/* Player p can pay for card id (0x132C on the field costs 500 per copy). */
int duel_can_pay(unsigned int p, int id)
{
    if (duel_card_protected(id, -1)) {
        return 0;
    }
    return duel_count_active(0x132C) * 500 < duel.players[p & 1].lp;
}

int duel_df664(unsigned int p)
{
    if (duel_slot_effect_with(p, 11, 0x1679, 2)) {
        return 0;
    }
    return !duel_slot_has_effect(p, 11, 0x1A1B);
}

/* Player p can activate cards normally (0x135D lets only the summon checks below through). */
int duel_can_activate(unsigned int p)
{
    if (duel_count_active(0x135D) != 0 && duel_can_special_summon(p) == 0) {
        return 0;
    }
    if (duel_slot_has_effect2(p, 11, 0x15AD)) {
        return 0;
    }
    if (duel_slot_effect_with(p, 11, 0x1679, 1)) {
        return 0;
    }
    if (duel_slot_has_effect(p, 11, 0x1578)) {
        return 0;
    }
    if (duel_slot_has_effect(p, 11, 0x1A92)) {
        return 0;
    }
    if (duel_slot_has_effect(p, 11, 0x1A1B)) {
        return 0;
    }
    if (duel_slot_has_effect(p, 11, 0x1436)) {
        return 0;
    }
    return !duel_find_effect_card(p, 0x1972, -1);
}

int duel_no_1a1b(unsigned int p)
{
    return !duel_slot_has_effect(p, 11, 0x1A1B);
}

int duel_can_special_summon(unsigned int p)
{
    if (duel.players[p & 1].flags >> 21 & 1) {
        return 0;
    }
    if (duel_find_effect_card(p, 0x13FF, -1)) {
        return 0;
    }
    if (duel_count_active(0x12B1)) {
        return 0;
    }
    if (duel_slot_has_effect2(p, 11, 0x15AD)) {
        return 0;
    }
    if (duel_slot_effect_with(p, 11, 0x1679, 1)) {
        return 0;
    }
    if (duel_slot_has_effect(p, 11, 0x1578)) {
        return 0;
    }
    if (duel_slot_has_effect(p, 11, 0x1A92)) {
        return 0;
    }
    if (duel_slot_has_effect(p, 11, 0x1436)) {
        return 0;
    }
    return !duel_find_effect_card(p, 0x1972, -1);
}

int duel_phase_ok(unsigned int p)
{
    if (duel.players[p & 1].flags >> 21 & 1) {
        return 0;
    }
    if (duel_find_effect_card(p, 0x13FF, -1)) {
        return 0;
    }
    if (duel_count_active(0x12B1)) {
        return 0;
    }
    return !duel_slot_effect_with(p, 11, 0x1679, 1);
}

int duel_can_set(unsigned int p)
{
    if (duel.players[p & 1].flags >> 22 & 1) {
        return 0;
    }
    if (duel_count_active(0x147F)) {
        return 0;
    }
    if (duel_count_active(0x12B1)) {
        return 0;
    }
    if (duel_slot_has_effect2(p, 11, 0x15AD)) {
        return 0;
    }
    if (duel_slot_effect_with(p, 11, 0x1679, 1)) {
        return 0;
    }
    return !duel_slot_has_effect(p, 11, 0x1A29);
}

int duel_phase_ok2(unsigned int p)
{
    if (duel_find_effect_card(p, 0x13FF, -1)) {
        return 0;
    }
    return duel_can_set(p);
}

#ifdef NON_MATCHING /* the field branch returns 0 straight to the epilogue in the original */
/* Card id of player p can be used (field: from the field, not only the hand). */
int duel_card_dfac8(unsigned int p, int id, int field)
{
    int r;

    if (card_is_in_group_1a58(id) == 0) {
        return 0;
    }
    if (card_attribute_icon_in_2_3(id) != 0) {
        if (field == 0) {
            return 0;
        }
        if (card_is_attribute6(id) == 0 || duel_has_st_12be(p) == 0) {
            return 0;
        }
        return 1;
    }
    switch (id) {
    case 0x158B:
    case 0x160F:
        if (duel_card_check_9148(p, id) != 0) {
            return 0;
        }
        break;
    case 0x164A:
    case 0x164B:
    case 0x164C:
    case 0x164D:
    case 0x164E:
    case 0x164F:
        if (duel_card_check_9148(p, card_id_remap_164a(id)) == 0) {
            return 0;
        }
        break;
    }
    return 1;
}
#endif

/* Locks on both players: 1 nobody may, 2 no monsters, 4 not player 0, 8 not player 1. */
unsigned int duel_dfc10(void)
{
    unsigned int r = 0;

    if (duel_count_active(0x1332)) {
        return 1;
    }
    if (duel_count_active(0x1A27)) {
        return 1;
    }
    if (duel_count_active(0x1A1A)) {
        return 1;
    }
    if (duel_count_active(0x1A33)) {
        r |= 2;
    }
    if (duel_find_effect_card(0, 0x1444, -1)) {
        r |= 4;
    }
    if (duel_find_effect_card(1, 0x1444, -1)) {
        r |= 8;
    }
    return r;
}

/* The card with uid is not locked. */
int duel_088DFCC8(unsigned int uid)
{
    unsigned int f = duel_dfc10();
    int r;

    if (f & 1) {
        r = 0;
    } else if ((f & 2) && card_is_monster(duel.list[uid].id)) {
        r = 0;
    } else if ((f & 4) && (uid & 1) == 0) {
        r = 0;
    } else if ((f & 8) && (uid & 1) == 1) {
        r = 0;
    } else {
        r = 1;
    }
    return r;
}

int duel_locks_ok(int p, int monster)
{
    unsigned int f = duel_dfc10();
    int r;

    if (f & 1) {
        r = 0;
    } else if ((f & 2) && monster != 0) {
        r = 0;
    } else if ((f & 4) && p == 0) {
        r = 0;
    } else if ((f & 8) && p == 1) {
        r = 0;
    } else {
        r = 1;
    }
    return r;
}

int duel_088DFE24(int p, unsigned int q)
{
    int r;

    if (duel_find_effect_card(1 - p, 0x1480, -1) != 0) {
        r = 0;
    } else if (duel_count_active(0x159D) != 0 && duel_find_effect_card(q, 0x158B, -1) == 0) {
        r = 0;
    } else {
        r = 1;
    }
    return r;
}

#ifdef NON_MATCHING /* the first flag test branches to the epilogue and its value is reused in the last test */
/* Player p has to answer now (local player waiting for input, or the CPU's turn). */
int duel_must_answer(int p)
{
    if (duel_busy_0891523C != 0) {
        return 0;
    }
    if (duel_busy_08915238 != 0) {
        return 0;
    }
    if (duel_player_type[p] == 1) {
        return duel_turn_player == p && duel.unk3798 != 0 && duel.unk37a0 == 3;
    }
    if (p == duel_view_player) {
        return duel.unk37d4 != 0 || duel_req_flag(0x31) != 0 || duel_req_flag(0x32) != 0;
    }
    return !(duel_msg_state & 1) && !duel_busy_0891523C && duel_msg_busy() != 0;
}
#endif
