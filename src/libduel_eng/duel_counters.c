// @range 0x088CC178-0x088CC8EC
/* Commands to the presentation for the values and counters on a card (0x3A-0x3C) and the cards
   reacting to them. */
#include "duel.h"
#include "card_db.h"

extern unsigned int duel_turn_player;

void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int field_get_card(unsigned int p, int z);
int duel_slot_value_1909(unsigned int p, int z);
int duel_slot_e68a0(unsigned int p, int z);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_slot_counter(unsigned int p, int z, int id);
int duel_is_cur_card(int id);
void duel_088C87A8(unsigned int p, int z, unsigned short id, unsigned short a, unsigned short b);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
void duel_088D1438(unsigned int p, int z, int a, int b);

void duel_088CC178(unsigned int p, unsigned char z, unsigned int v)
{
    duel_cmd((p ? 0x8000 : 0) | 0x3A, z, v, v >> 16);
}

void duel_088CC1A0(unsigned int p, int z, unsigned int v)
{
    duel_cmd((p ? 0x8000 : 0) | 0x3A, (z & 0xFF) | 0x100, v, v >> 16);
}

void duel_088CC1D0(unsigned int p, int z, int f)
{
    if (card_is_in_group_1909(field_get_card(p, z)) != 0) {
        if (f != 0) {
            duel_cmd((p ? 0x8000 : 0) | 0x3B, z, 1, 0);
        } else if (duel_slot_value_1909(p, z) != 0) {
            duel_cmd((p ? 0x8000 : 0) | 0x3B, z, 0, 0);
        }
    }
}

void duel_088CC284(unsigned int p, int z, int n)
{
    int c = duel_slot_e68a0(p, z);
    int m = card_get_group_value_1983(field_get_card(p, z));

    if (m != 0) {
        if (n > 0) {
            unsigned int q = p & 1;

            if (!(duel.players[q].field[z].flags >> 5 & 1)) {
                if (c < m) {
                    duel_cmd((p ? 0x8000 : 0) | 0x3C, z, (short)n, 0);
                }
                if (duel_slot_e8170(p, z, 0x1817) != 0 && c < 5 && c + n >= 5) {
                    duel_088C87A8(p, z, 0x181A, (unsigned char)((duel_turn_player == p) + 1) << 4 | 2, 0);
                }
                if (duel_slot_e8170(p, z, 0x16DE) != 0 && c < 4 && c + n >= 4) {
                    duel_0884F30C((p << 31 & 0x80000000) | 0x200000 | (z << 16 & 0x1F0000) | duel.players[q].field[z].card.id,
                                  CARD_UID(duel.players[q].field[z].card), 0);
                }
            }
        } else if (c > 0) {
            duel_cmd((p ? 0x8000 : 0) | 0x3C, z, (short)n, 0);
        }
    }
}

void duel_088CC4B4(unsigned int p, int z, int n)
{
    if (card_is_in_group_1a0c(field_get_card(p, z)) != 0) {
        int l = duel.players[p & 1].field[z].link;

        if (n > 0) {
            duel_cmd((p ? 0x8000 : 0) | 0x3C, z, (short)n, 1);
        } else if (l > 0) {
            duel_cmd((p ? 0x8000 : 0) | 0x3C, z, (short)n, 1);
        }
    }
}

void duel_088CC594(unsigned int p, int z, int n, int id)
{
    int c = duel_slot_counter(p, z, id);

    duel_cmd((p ? 0x8000 : 0) | 0x3C, z, (short)n, id);
    switch (id) {
    case 0x1A90:
        if (c + n <= 0) {
            unsigned int e = duel.players[p & 1].field[z].effects;

            while (e != 0) {
                DuelEffect *ef = &duel.effects[e];

                e = ef->next;
                if ((ef->kind & 0xF) == 7) {
                    unsigned int sp = EFF_PLAYER(ef);
                    unsigned int sz = EFF_ZONE(ef);

                    if (duel.players[sp & 1].field[sz].card.id == 0x1A9C &&
                        SLOT_ACTIVE(&duel.players[sp & 1].field[sz])) {
                        duel_088D1438(sp, sz, 0, 0);
                    }
                }
            }
        }
        break;
    }
}

void duel_088CC718(int id)
{
    unsigned int q;
    int z;
    unsigned int i;

    duel_cmd(0x14, id, 1, 0);
    if (duel_is_cur_card(0x10F4) != 0 && card_same_name(id, 0x10F4) == 0) {
        for (i = 0; (int)i < 2; i++) {
            q = duel_turn_player ^ i;
            for (z = 5; z <= 10; z++) {
                if (duel.players[q & 1].field[z].card.id == 0x13F7 && duel.players[q & 1].field[z].faceup != 0 &&
                    !(duel.players[q & 1].field[z].flags >> 5 & 1)) {
                    duel_088D1438(q, z, 0, 0);
                }
            }
        }
    }
}

int duel_088CC820(DuelEffect *ef, struct DuelActX *x)
{
    if ((ef->kind & 0xF) == 10) {
        unsigned int sp = EFF_PLAYER(ef);
        unsigned int sz = EFF_ZONE(ef);

        if (sp != x->s_player && duel.players[sp & 1].field[sz].card.id == 0x18D1) {
            duel_0884F30C((sp << 31 & 0x80000000) | 0x2C4E18D1, CARD_UID(duel.players[sp & 1].field[sz].card),
                          *(unsigned int *)x);
        }
    }
    return 0;
}
