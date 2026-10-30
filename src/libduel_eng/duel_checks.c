// @range 0x088C2F80-0x088C4448
// @asm 0x088C321C-0x088C3348
// @asm 0x088C39F0-0x088C3B48
// @asm 0x088C3F80-0x088C4448
// @data 0x08913518-0x08913528
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
void duel_088C7940(int a);
void duel_088D1438(unsigned int p, int z, int a, int b);
void duel_088CC1A0(unsigned int p, int z, unsigned int v);
void duel_088CC178(unsigned int p, int z, unsigned int v);
void duel_088C7E68(unsigned int p, int z, int a, int b);
int duel_0884F30C(unsigned int a, unsigned short uid, unsigned int b);
int duel_hand_find_name(unsigned int p, int id);
int duel_d_find_name(unsigned int p, int id);
int duel_list_find_id(unsigned int p, unsigned int id);
int duel_088C004C(unsigned int p, unsigned int z, unsigned int uid);
int duel_088C3058(DuelEffect *e, int *ctx);
void duel_088CC284(unsigned int p, int z, int n);
void duel_088CC1D0(unsigned int p, int z, int f);
void duel_088CC4B4(unsigned int p, int z, int n);
void duel_088C4448(unsigned int p, int a, unsigned int uid);

extern int duel_phase;

extern unsigned int duel_turn;

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

int duel_088C3018(void)
{
    unsigned int c = duel_count_active(0x1510) > 0;

    if (c != (duel.mode >> 2 & 1)) {
        duel_088C7940(c);
        return 1;
    }
    return 0;
}

int duel_088C3058(DuelEffect *e, int *ctx)
{
    switch (e->kind & 0xF) {
    case 2:
        return e->src == 0x1598;
    case 10:
    case 11:
        return 1;
    case 6:
    case 7:
        return (((DuelEffectBits *)e)->kind_hi & 1) > 0;
    }
    return 0;
}

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

/* pending entries of kind 0x1B from the cards 0x17C7 and 0x17C8 of the same player: with both, and the card
   0x17C9 in his hand or zone 0xD, post the request 0x17C9 */
int duel_088C3804(void)
{
    int t[2][2] = {{-1, -1}, {-1, -1}};
    unsigned int i;
    int *r;
    int p;

    for (i = 0; i < duel_chain.n_kinds; i++) {
        if (duel_chain.kinds[i] == 0x1B) {
            struct DuelActX *x = &duel_chain.srcs[i];

            if (duel_chain.srcs[i].s_b15) {
                switch (duel.list[x->s_uid].id) {
                case 0x17C7:
                    t[duel_chain.srcs[i].s_player][0] = i;
                    break;
                case 0x17C8:
                    t[duel_chain.srcs[i].s_player][1] = i;
                    break;
                }
            }
        }
    }
    for (p = 0, r = t[0]; p < 2; p++, r += 2) {
        if (r[0] >= 0 && r[1] >= 0
            && (duel_hand_find_name(p, 0x17C9) >= 0 || duel_d_find_name(p, 0x17C9) >= 0)
            && duel_088C004C(p, 0x10, duel_list_find_id(p, 0x17C9)) == 0
            && duel_0884F30C(((unsigned int)p << 31 & 0x80000000) | (duel_chain.kinds[r[0]] << 25 & 0x7E000000) | 0x5017C9, 0,
                             *(unsigned int *)&duel_chain.srcs[r[0]]) != 0) {
            return 1;
        }
    }
    return 0;
}

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

int duel_088C3B48(void)
{
    unsigned int i;
    unsigned int q;
    int z;

    for (i = 0; (int)i < 2; i++) {
        q = duel_turn_player ^ i;
        for (z = 0; z <= 4; z++) {
            if (field_get_card(q, z) == 0x1744) {
                duel_088C7E68(q, z, 4, 0);
            }
        }
    }
    return 0;
}

int duel_088C3BD8(void)
{
    unsigned int q;
    int z;
    unsigned int i;

    for (i = 0; (int)i < 2; i++) {
        q = duel_turn_player ^ i;
        for (z = 0; z <= 4; z++) {
            if (field_get_card(q, z) == 0x1817 && duel.players[q & 1].field[z].faceup != 0 &&
                duel_slot_counter(q, z, 0x1817) != 0) {
                if (!(duel.players[q & 1].field[z].flags >> 5 & 1)) {
                    duel_088CC284(q, z, duel_slot_counter(q, z, 0x1817));
                }
                duel_088C89BC(q, z, 0x1817, 1);
                return 1;
            }
        }
    }
    return 0;
}

int duel_088C3D08(void)
{
    unsigned int p;
    int z;

    if (duel_phase == 7) {
        return 0;
    }
    p = duel_turn_player;
    for (z = 0; z <= 4; z++) {
        if (field_get_card(p, z) == 0x1810 && duel.players[p & 1].field[z].faceup != 0 &&
            duel_turn + 1 != (duel.players[p & 1].field[z].link >> 16 & 0xFFFF)) {
            unsigned int n = (unsigned short)duel.players[p & 1].field[z].link;

            duel_088CC1D0(p, z, 1);
            duel_088CC178(p, z, (n + 1 & 0xFFFF) | (duel_turn + 1 & 0xFFFF) << 16);
            return 1;
        }
    }
    return 0;
}

int duel_088C3E34(void)
{
    unsigned int q;
    int z;
    unsigned int i;

    for (i = 0; (int)i < 2; i++) {
        q = duel_turn_player ^ i;
        for (z = 0; z <= 4; z++) {
            if (field_get_card(q, z) == 0x1A8D && (duel.players[q & 1].field[z].flags >> 4 & 1)) {
                duel_088C7E68(q, z, 4, 0);
                if (SLOT_ACTIVE(&duel.players[q & 1].field[z])) {
                    duel_088C4448(q, z, CARD_UID(duel.players[q & 1].field[z].card));
                    duel_088CC4B4(q, z, 2);
                    return 1;
                }
            }
        }
    }
    return 0;
}

#ifdef NON_MATCHING
extern int duel_w_0891961C;
extern unsigned int duel_w_08919620;
extern int duel_w_089195EC;
extern int duel_player_type[2];
extern unsigned int duel_w_08917DB4;
void duel_0884DD94(void);
void duel_088C7C54(void);
int duel_0886BCDC(void);
void duel_088C09C8(unsigned int p, int c);
int duel_088C0120(void);
int duel_088C0CD0(void);
int duel_088C2CA8(void);
int duel_088C2E14(void);
int duel_088C3BD8(void);
int duel_088C205C(void);
int duel_088C14C0(void);
int duel_088C19F0(void);
int duel_088C2A08(void);
int duel_088C21A0(void);
int duel_088C1EC8(void);
int duel_088C134C(void);
int duel_088C2300(void);
int duel_088C2BA0(void);
int duel_088C3630(void);
int duel_088C3B48(void);
int duel_088C168C(void);
int duel_088C373C(void);
int duel_088C1E20(void);
int duel_088C25C0(void);
int duel_088C266C(void);
int duel_088C0364(int id);
int duel_088C0668(int id);
int duel_088C272C(int id);
int duel_088C39F0(void);
int duel_hand_revealed(unsigned int p);
void duel_088C7810(unsigned int p, int a);
void duel_088C7910(unsigned int p);
void duel_088C8F04(unsigned int p, int z);
int duel_088C8094(unsigned int p, int z);
int duel_088C7F18(unsigned int p, int z);
void duel_088C7CF0(void);

/* state checks between the steps of the duel: returns 1 while one of them acted (run again), 0 when the
   field is settled (duel_w_0891961C cleared) */
int duel_088C3F80(void)
{
    int z;
    unsigned int q;
    int i;

    if (duel_w_0891961C == 0) {
        return 0;
    }
    duel_0884DD94();
    if (duel_w_089195EC != 0 && duel_player_type[duel_turn_player] == 2) {
        return 0;
    }
    duel_088C7C54();
    if (duel_0886BCDC() != 0) goto busy;
    if (duel_088C0120() != 0) goto busy;
    if (duel_088C0CD0() != 0) goto busy;
    duel_088C09C8(-1, 0);
    if (duel_088C2CA8() != 0) goto busy;
    if (duel_088C2E14() != 0) goto busy;
    if (duel_088C3BD8() != 0) goto busy;
    if (duel_088C3E34() != 0) goto busy;
    if (duel_088C3018() != 0) goto busy;
    if (duel_088C205C() != 0) goto busy;
    if (duel_088C14C0() != 0) goto busy;
    if (duel_088C19F0() != 0) goto busy;
    if (duel_088C2A08() != 0) goto busy;
    if (duel_088C21A0() != 0) goto busy;
    if (duel_088C1EC8() != 0) goto busy;
    if (duel_088C134C() != 0) goto busy;
    if (duel_088C2300() != 0) goto busy;
    if (duel_088C2BA0() != 0) goto busy;
    if (duel_088C3348() != 0) goto busy;
    if (duel_088C33F4() != 0) goto busy;
    if (duel_088C34F0() != 0) goto busy;
    if (duel_088C3630() != 0) goto busy;
    if (duel_088C0364(0x16DA) != 0) goto busy;
    if (duel_088C0668(0x142C) != 0) goto busy;
    if (duel_088C0668(0x17A4) != 0) goto busy;
    if (duel_088C0668(0x151A) != 0) goto busy;
    if (duel_088C0668(0x1574) != 0) goto busy;
    if (duel_088C0668(0x1A51) != 0) goto busy;
    if (duel_088C39F0() != 0) goto busy;
    if (duel_088C3B48() != 0) goto busy;
    if (duel_088C2F80() != 0) goto busy;
    if (duel_088C3D08() != 0) goto busy;
    if (duel_088C3804() != 0) goto busy;
    if (duel_088C30D4() != 0) goto busy;
    if (duel_088C168C() != 0) goto busy;
    if (!(duel_w_08917DB4 & 1)) {
        if (duel_088C373C() != 0) goto busy;
        if (duel_088C1E20() != 0) goto busy;
        if (duel_088C25C0() != 0) goto busy;
        if (duel_088C272C(0x158B) != 0) goto busy;
        if (duel_088C272C(0x160F) != 0) goto busy;
        if (duel_088C266C() != 0) goto busy;
    }
    for (i = 0; i < 2; i++) {
        q = duel_turn_player ^ i;
        if (duel_hand_revealed(q)) {
            duel_088C7810(q, 1);
        } else {
            duel_088C7810(q, 0);
            if (duel.players[q & 1].flags >> 2 & 1) {
                duel_088C7910(q);
            }
        }
    }
    for (i = 0; i < 2; i++) {
        q = duel_turn_player ^ i;
        for (z = 0; z < 5; z++) {
            if (duel.players[q & 1].field[z].card.id != 0) {
                if (duel.players[q & 1].field[z].faceup == 0) {
                    duel_088C8F04(q, z);
                }
                if (duel_088C8094(q, z) != 0) goto busy;
                if (duel_088C7F18(q, z) != 0) goto busy;
            }
        }
    }
    if (duel_088C0668(0x1762) != 0) goto busy;
    if (duel_088C0668(0x186B) != 0) goto busy;
    if (duel_088C0668(0x1862) != 0) goto busy;
    if (duel_088C0668(0x1875) != 0) goto busy;
    duel_088C7CF0();
    duel_w_0891961C = 0;
    duel_w_08919620 = 0;
    return 0;
busy:
    duel_088C7CF0();
    return 1;
}
#endif
