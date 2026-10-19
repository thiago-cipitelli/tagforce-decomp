// @range 0x088C2F80-0x088C3B48
// @asm 0x088C3018-0x088C3058
// @asm 0x088C3058-0x088C30D4
// @asm 0x088C321C-0x088C3348
// @asm 0x088C3804-0x088C39F0
// @asm 0x088C39F0-0x088C3B48
/* State checks run between the steps of the duel: continuous cards whose condition ended are
   destroyed or lose their effect. */
#include "duel.h"
#include "card_db.h"

extern unsigned int duel_turn_player;
extern unsigned int duel_w_0891960C;

int field_get_card(unsigned int p, int z);
int duel_slot_counter(unsigned int p, int z, int id);
int duel_slot_e8170(unsigned int p, int z, int id);
int duel_slot_e68a0(unsigned int p, int z);
int duel_count_active(int id);
int duel_count_monster_active(unsigned int p, int id);
int duel_grave_count_id(unsigned int p, int id);
int duel_slot_foreach_effect(unsigned int p, int z, int (*fn)(DuelEffect *e, int *ctx), int *ctx);
void duel_088C44E4(unsigned int p, int a);
void duel_088C5E70(unsigned int p, int a);
void duel_088C89BC(unsigned int p, int z, int id, int c);
void duel_088C7940(void);
void duel_088D1438(unsigned int p, int z, int a, int b);
void duel_088CC1A0(unsigned int p, int z, unsigned int v);
void duel_088CC178(unsigned int p, unsigned char z, unsigned int v);
void duel_088C7E68(unsigned int p, int z, int a, int b);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
int duel_088C3058(DuelEffect *e, int *ctx);

int duel_088C2F80(void)
{
    unsigned int i;

    for (i = 0; (int)i < 2; i++) {
        unsigned int q = duel_turn_player ^ i;

        int r = !duel_slot_counter(q, 11, 0x1A0B);

        if (!r) {
            duel_088C44E4(q, 0x1A0B);
            duel_088C5E70(q, 1);
            duel_088C89BC(q, 11, 0x1A0B, 0);
            return 1;
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* register allocation and bitfield extraction not reproduced */
int duel_088C3018(void)
{
    unsigned int c = duel_count_active(0x1510) > 0;

    if (c != (duel.mode >> 2 & 1)) {
        duel_088C7940();
        return 1;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* register allocation and bitfield extraction not reproduced */
int duel_088C3058(DuelEffect *e, int *ctx)
{
    switch (e->kind & 0xF) {
    case 2:
        return e->src == 0x1598;
    case 10:
        return 1;
    case 11:
        return 1;
    case 6:
    case 7:
        return (e->kind >> 4 & 1) > 0;
    }
    return 0;
}
#endif

int duel_088C30D4(void)
{
    unsigned int q;
    int z;
    unsigned int i;

    if ((duel.mode & 1) && duel_w_0891960C < 9) {
        return 0;
    }
    for (i = 0; (int)i < 2; i++) {
        q = duel_turn_player ^ i;
        for (z = 0; z <= 4; z++) {
            switch (field_get_card(q, z)) {
            case 0x1596:
            case 0x1598:
                if (SLOT_ACTIVE(&duel.players[q & 1].field[z]) &&
                    duel_slot_foreach_effect(q, z, duel_088C3058, 0) != 0) {
                    duel_088D1438(q, z, 0, 0);
                    return 1;
                }
                break;
            }
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* register allocation and bitfield extraction not reproduced */
void duel_088C321C(unsigned int p)
{
    unsigned int q = 1 - p;
    int z;

    for (z = 5; z < 10; z++) {
        if (duel_slot_e8170(q, z, 0x159B) != 0) {
            if (duel.players[q & 1].field[z].flags >> 4 & 1) {
                duel_088CC1A0(q, z, 1);
            } else if (duel_0884F30C(q << 31 | 0x200000 | (z & 0x1F) << 16 | 0x159B,
                                     CARD_UID(duel.players[q & 1].field[z].card), 0) != 0) {
                duel_088C7E68(q, z, 4, 1);
                duel_088CC1A0(q, z, 1);
            }
        }
    }
}
#endif

int duel_088C3348(void)
{
    unsigned int i;
    unsigned int q;
    int z;

    for (i = 0; (int)i < 2; i++) {
        q = duel_turn_player ^ i;
        for (z = 5; z < 10; z++) {
            if (duel_slot_e8170(q, z, 0x1624) != 0 && duel_slot_e68a0(q, z) == 0) {
                duel_088D1438(q, z, 0, 0);
                return 1;
            }
        }
    }
    return 0;
}

int duel_088C33F4(void)
{
    unsigned int q;
    int z;
    unsigned int i;

    for (i = 0; (int)i < 2; i++) {
        q = duel_turn_player ^ i;
        for (z = 0; z <= 4; z++) {
            if (duel_slot_e8170(q, z, 0x1645) != 0 &&
                (duel_grave_count_id(q, 0xFB7) == 0 || duel_grave_count_id(q, 0xFB8) == 0 ||
                 duel_grave_count_id(q, 0xFB9) == 0 || duel_grave_count_id(q, 0xFBA) == 0 ||
                 duel_grave_count_id(q, 0xFBB) == 0)) {
                duel_088D1438(q, z, 0, 0);
                return 1;
            }
        }
    }
    return 0;
}

int duel_088C34F0(void)
{
    unsigned int i;
    int z;

    for (i = 0; (int)i < 2; i++) {
        unsigned int q = duel_turn_player ^ i;

        for (z = 5; z < 10; z++) {
            if (duel.players[q & 1].field[z].card.id == 0x169A && duel.players[q & 1].field[z].faceup != 0 &&
                !(duel.players[q & 1].field[z].flags >> 5 & 1)) {
                int y;
                int n = 0;

                for (y = 0; y <= 10; y++) {
                    if (duel.players[q & 1].field[y].faceup != 0 &&
                        card_is_in_group_1911(duel.players[q & 1].field[y].card.id) != 0) {
                        n++;
                    }
                }
                if (n == 0) {
                    duel_088D1438(q, z, 0, 0);
                    return 1;
                }
            }
        }
    }
    return 0;
}

int duel_088C3630(void)
{
    unsigned int q;
    int z;
    unsigned int i;

    for (i = 0; (int)i < 2; i++) {
        q = duel_turn_player ^ i;
        for (z = 5; z < 10; z++) {
            switch (duel.players[q & 1].field[z].card.id) {
            case 0x1760:
            case 0x1765:
                if (SLOT_ACTIVE(&duel.players[q & 1].field[z]) && duel_count_monster_active(q, 0x173C) == 0) {
                    duel_088D1438(q, z, 0, 0);
                    return 1;
                }
                break;
            }
        }
    }
    return 0;
}

int duel_088C373C(void)
{
    unsigned int q;
    int z;
    unsigned int i;

    for (i = 0; (int)i < 2; i++) {
        q = duel_turn_player ^ i;
        for (z = 0; z <= 4; z++) {
            if (duel_slot_e8170(q, z, 0x16BF) != 0 && duel.players[q & 1].field[z].pos != 0) {
                duel_088D1438(q, z, 0, 0);
                return 1;
            }
        }
    }
    return 0;
}

/* 0x088C3804: kept in asm. */

#ifdef NON_MATCHING /* register allocation and bitfield extraction not reproduced */
int duel_088C39F0(void)
{
    unsigned int i;
    int z;

    for (i = 0; (int)i < 2; i++) {
        unsigned int q = duel_turn_player ^ i;

        for (z = 0; z < 5; z++) {
            if (field_get_card(q, z) == 0x18B2 && duel.players[q & 1].field[z].faceup != 0 &&
                duel.players[q & 1].field[z].link != 0) {
                duel_0884F30C(q << 31 | 0x200000 | (z & 0x1F) << 16 | 0x18B2,
                              CARD_UID(duel.players[q & 1].field[z].card), duel.players[q & 1].field[z].link);
                duel_088CC178(q, z, 0);
                return 1;
            }
        }
    }
    return 0;
}
#endif
