// @range 0x088C7F18-0x088C87A8
/* Checks run on a monster zone after a change: control taken by the other player (the monster
   moves), and the effects attached to it (equips and others) that destroy it or are destroyed. */
#include "duel.h"

extern unsigned int duel_w_08919620;   /* moves in a row, reset after 10 */

int duel_slot_eef84(unsigned int p, int z);
unsigned int duel_field_controller(unsigned int p, int z, int arg);
int duel_pick_monster_zone(unsigned int p);
unsigned int duel_field_flag9(unsigned int p, int z);
int duel_pick_st_zone(unsigned int p);
void duel_088D1438(unsigned int p, int z, int a, int b);
void duel_088CA184(unsigned short loc, unsigned short to);
void duel_088C914C(unsigned int p, int z);
unsigned short duel_slot_src_of(unsigned int p, int z, int id, int active);
int duel_find_effect_card(unsigned int p, int id, int z);
unsigned short duel_slot_src_of_pos(unsigned int p, int z, int id, unsigned int pos);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
int card_get_type(int id);
void duel_088CBAA4(unsigned int p, int z, int uid);
void duel_088C7E68(unsigned int p, int z, int a, int b);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_088C88B8(unsigned int p, int z, int v, int c);
int field_get_card(unsigned int p, int z);
int duel_act_check_08853CE8(DuelAct *a, unsigned int p, int z);

/* the monster (p, z) is controlled by the other player: move it to his side; 1 if handled */
int duel_088C7F18(unsigned int p, int z)
{
    unsigned int q;
    int to;

    if (duel_slot_eef84(p, z)) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id != 0 && (q = duel_field_controller(p, z, 0), q != p)) {
        to = duel_pick_monster_zone(q);
        if (duel_field_flag9(p, z) && duel_pick_st_zone(1 - p) < 0) {
            to = -1;
        }
        if (to == -1) {
            duel_088D1438(p, z, 1, 0);
        } else if (++duel_w_08919620 >= 10) {
            duel_w_08919620 = 0;
            duel_088D1438(p, z, 0, 0);
        } else {
            duel_088CA184((unsigned char)p | (unsigned char)z << 8, (unsigned char)q | (unsigned char)to << 8);
        }
        return 1;
    }
    return 0;
}

/* effects attached to the monster (p, z): an equip spell (kind >= 10) sent away with its monster,
   0x14C4/0x14C7/0x14D5/0x14DA on the zone, 0x1660, 0x14CA, 0x184B; 1 if one of them acted */
int duel_088C8094(unsigned int p, int z)
{
    DuelStats st;
    unsigned int e;
    DuelEffectBits *ef;
    unsigned int src;
    unsigned int q;
    int sz;
    int id;
    unsigned int by1660;
    int by14ca;
    unsigned int by184b;

    if (duel.players[p & 1].field[z].faceup == 0) {
        duel_088C914C(p, z);
    } else {
        by1660 = duel_slot_src_of(p, z, 0x1660, 1);
        by14ca = duel_find_effect_card(p, 0x14CA, -1);
        by184b = duel_slot_src_of_pos(p, z, 0x184B, 0);
        duel_field_stats(p, z, &st);
        for (e = duel.players[p & 1].field[z].effects; e != 0;) {
            ef = (DuelEffectBits *)&duel.effects[e];
            src = ef->src;
            e = ef->next;
            q = src & 0xFF;
            sz = src >> 8 & 0xFF;
            if (ef->kind < 6) {
                continue;
            }
            id = duel.players[q & 1].field[sz].card.id;
            if ((ef->kind >= 10 || ef->kind_hi) && sz >= 5 && card_get_type(id) != 0x17) {
                if (by1660 != 0xFFFF && by1660 != src) {
                    duel_088CBAA4(q, sz, CARD_UID(duel.players[q & 1].field[sz].card));
                    duel_088C7E68(q, sz, 6, 1);
                    duel_088D1438(q, sz, (by1660 & 0xFF) != q, 0x1660);
                    return 1;
                }
                if (by14ca && st.attr == 0xF) {
                    duel_088CBAA4(q, sz, CARD_UID(duel.players[q & 1].field[sz].card));
                    duel_088C7E68(q, sz, 6, 1);
                    duel_088D1438(q, sz, p != q, 0x14CA);
                    return 1;
                }
                if (by184b != 0xFFFF && st.attr == 7) {
                    duel_088CBAA4(q, sz, CARD_UID(duel.players[q & 1].field[sz].card));
                    duel_088C87A8(q, sz, 0x17C2, 5, 0);
                    duel_088C7E68(by184b & 0xFF, (unsigned char)((by184b & 0xFFFF) >> 8), 4, 1);
                    return 1;
                }
            }
            if (SLOT_ACTIVE(&duel.players[p & 1].field[z])) {
                switch (field_get_card(p, z)) {
                case 0x14C4:
                case 0x14C7:
                    if ((ef->kind >= 10 || ef->kind_hi) && sz >= 5 && card_get_type(id) != 0x17) {
                        duel_088CBAA4(q, sz, CARD_UID(duel.players[q & 1].field[sz].card));
                        duel_088C7E68(q, sz, 6, 1);
                        duel_088D1438(q, sz, p != q, field_get_card(p, z));
                        return 1;
                    }
                    break;
                case 0x14D5:
                case 0x14DA:
                    if ((ef->kind >= 10 || ef->kind_hi) && sz >= 5 && card_get_type(id) == 0x17) {
                        duel_088CBAA4(q, sz, CARD_UID(duel.players[q & 1].field[sz].card));
                        duel_088C7E68(q, sz, 6, 1);
                        duel_088D1438(q, sz, p != q, 0x14DA);
                        return 1;
                    }
                    break;
                }
            }
            switch (ef->kind) {
            case 10:
            case 11: { /* the effect source must still be able to target the monster */
                DuelAct t = { 0 };

                t.player = q;
                t.zone = sz;
                t.b3_4 = 1;
                t.uid = CARD_UID(duel.players[q & 1].field[sz].card);
                t.id = id;
                if (!duel_act_check_08853CE8(&t, p, z)) {
                    duel_088D1438(q, sz, 0, 0);
                    return 1;
                }
                break;
            }
            case 6:
                switch (duel.players[q & 1].field[sz].card.id) {
                case 0x14FC:
                case 0x19ED:
                    if (field_get_card(p, z) == 0x1A74) {
                        duel_088D1438(q, sz, 0, 0);
                        return 1;
                    }
                }
                break;
            case 13:
                if (field_get_card(p, z) == 0x1A74) {
                    duel_088D1438(q, sz, 0, 0);
                    return 1;
                }
                break;
            case 7:
                if (id == 0x184B && st.attr != 7) {
                    duel_088C88B8(p, z, src, 1);
                    return 1;
                }
                break;
            }
        }
    }
    return 0;
}
