// @range 0x088F6380-0x088F6790
/* Effective level of monster z of player p: printed level, copies, level-changing effects in its
   list, 0x15E3 and 0x150B. */
#include "duel.h"
#include "card_db.h"

int duel_slot_e1854(unsigned int p, int z);
unsigned int duel_slot_immunity(unsigned int p, int z);
unsigned int duel_slot_ref(unsigned int p, int z);
int duel_field_attr_is(unsigned int p, int z, int attr);
void duel_slot_foreach_effect(unsigned int p, int z, int (*fn)(DuelEffect *e, int *ctx), int *ctx);
int duel_level_cb(DuelEffect *e, int *ctx);
unsigned int duel_field_type_ex(unsigned int p, int z, int skip);

int duel_field_level(unsigned int p, int z)
{
    unsigned int e;
    int id = duel.players[p & 1].field[z].card.id;
    unsigned int imm = duel_slot_immunity(p, z);
    unsigned int ref = duel_slot_ref(p, z);
    int lvl;
    int ctx[2];
    unsigned int q;

    if (id == 0) {
        return 0;
    }
    lvl = card_get_level(id);
    if (z >= 5) {
        return lvl;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return lvl;
    }
    if (id == 0x19ED) {
        unsigned short r = duel_slot_e1854(p, z);

        if (r != 0xFFFF) {
            return duel_field_level(r & 0xFF, (unsigned char)((r & 0xFFFF) >> 8));
        }
    }
    ctx[0] = CARD_UID(duel.players[p & 1].field[z].card);
    ctx[1] = lvl;
    duel_slot_foreach_effect(0, 11, duel_level_cb, ctx);
    lvl = ctx[1];
    for (e = duel.players[p & 1].field[z].effects; e != 0;) {
        DuelEffect *ef = &duel.effects[e];
        unsigned short src = ef->src;
        unsigned short player = src & 0xFF;
        unsigned short zone = src >> 8 & 0xFF;

        e = ef->next;
        switch (ef->kind & 0xF) {
        case 1:
            switch (src) {
            case 0x15C7:
                lvl -= ef->arg * 2;
                break;
            }
            break;
        case 5:
            switch (src) {
            case 0x1602:
            case 0x1608:
                lvl = (unsigned char)((ef->arg & 0xF0) >> 4);
                e = duel.effects[duel.effects[(unsigned short)e].next].next;
                break;
            case 0x1472:
                switch (id) {
                case 0x1472:
                case 0x172F:
                case 0x1809:
                    lvl = 4;
                    break;
                case 0x1636:
                    lvl = 10;
                    break;
                }
                break;
            }
            break;
        case 10: {
            int cid = duel.players[player & 1].field[zone].card.id;

            if (cid != 0 && !(duel.players[player & 1].field[zone].flags >> 5 & 1)) {
                if (card_get_type((unsigned short)cid) == 0x17) {
                    if ((int)(ref & ((p != player) + 1)) > 0) {
                        break;
                    }
                } else if ((int)(imm & ((p != player) + 1)) > 0) {
                    break;
                }
                switch ((unsigned short)cid) {
                case 0x15E3:
                    lvl -= 2;
                    break;
                }
            }
            break;
        }
        }
    }
    for (q = 0; (int)q < 2; q++) {
        if (duel.players[q & 1].field[10].card.id != 0 && SLOT_ACTIVE(&duel.players[q & 1].field[10]) &&
            (int)(imm & ((p != q) + 1)) <= 0) {
            switch (duel.players[q & 1].field[10].card.id) {
            case 0x150B:
                if (duel_field_attr_is(p, z, 3)) {
                    lvl--;
                }
                break;
            }
        }
    }
    return lvl;
}

unsigned int duel_field_type(unsigned int p, int z)
{
    return duel_field_type_ex(p, z, -1);
}
