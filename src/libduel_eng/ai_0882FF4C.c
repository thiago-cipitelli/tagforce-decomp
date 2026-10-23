// @range 0x0882FF4C-0x08830E94
// @asm 0x08830030-0x088302BC
// @asm 0x0883078C-0x088309F8
// @asm 0x08830CDC-0x08830DC0
/* Questions asked during an event (duel_event.c): the step machines (duel_ev_unk) that let the
   player of the event (duel_w_08919670) pick zones or answer, and their zone filters. The handlers
   take (player, card id, argument) = (duel_w_08919670, duel_ev_data[0], duel_ev_data[1]); the
   answer goes to duel_w_0891968C. */
#include "duel.h"

extern int duel_player_type[2];
extern unsigned int duel_turn_player;
extern unsigned int duel_w_08919670;   /* player of the event */
extern int duel_ev_unk;                /* 0x08919678 step */
extern unsigned short duel_ev_data[8]; /* 0x0891967C */
extern unsigned short duel_w_0891968C; /* answer */
extern unsigned short duel_h_0891968E[2]; /* zones picked by each player (bit per zone) */
extern int duel_w_08919624;            /* answer of the last question */
extern int duel_w_0891964C;            /* picked player */
extern int duel_w_08919650;            /* picked zone */
extern int duel_w_08919654;

int duel_08835AC0(void);
int duel_host_call(int a, int b, unsigned int c, int d);
void duel_0881956C(unsigned int p, int id, int (*fn)());
void duel_08835978(int (*fn)());
int duel_0884ACD8(unsigned int p, int msg);
int duel_0884DF2C(int (*fn)());
void duel_088D1B20(unsigned int p, unsigned int mask);
int duel_tributable(unsigned int p, unsigned int q, int z);
unsigned int duel_field_type(unsigned int p, int z);
int duel_slot_ef098(unsigned int p, int z, int id);
int duel_slot_immune(unsigned int p, int z, int opp);
int duel_slot_ref_immune(unsigned int p, int z, int opp);
int card_is_monster(int id);
int card_is_attribute9(int id);
int card_is_in_group_1a58(int id);
int duel_can_attack_target(unsigned int p, int z, unsigned int q, int i, int arg);
extern int duel_atk_zone;              /* 0x08913F2C */
int duel_zone_card_uid(unsigned int p, int z, int i);
int duel_08809B50(unsigned int p, unsigned short id, unsigned short a);
DuelCard *duel_0884C7CC(void);
void duel_0884C588(unsigned int p, int a, int id, int b);
int duel_0880B448(unsigned int p, int id);
int duel_0880BD00(void);
int duel_0884AF34(int a, int b);
int duel_0884AF48(int a, int b, int c);
int duel_088302BC(unsigned int p, int z, int i);
int duel_08830358(unsigned int p, int z, int i);
int duel_0883042C(unsigned int p, int z, int i);
int duel_088304A0(unsigned int p, int z, int i);
int duel_08830590(unsigned int p, int z, int i);
int duel_08830664(unsigned int p, int z, int i);
int duel_08830728(unsigned int p, int z, int i);

/* Zone filter: own monsters that may be tributed, not picked yet, of the type in the high byte of
   the argument. */
int duel_0882FF4C(unsigned int p, int z, int i)
{
    int zz = z + i;
    unsigned int type;
    unsigned short w;

    if (zz < 5) {
        if (duel_tributable(duel_w_08919670, p, zz) != 0) {
            if (duel_ev_data[0] != (unsigned short)((unsigned char)p | (unsigned char)zz << 8)) {
                type = (unsigned char)(duel_ev_data[1] >> 8);
                if (type != 0 && type != (unsigned short)duel_field_type(p, zz)) {
                    return 0;
                }
                w = duel_h_0891968E[p];
                if (w & 1 << zz) {
                    return 0;
                }
                return 0x800;
            }
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* o original relê duel_w_0891964C/50/54 depois de marcar a zona; o mwcc reaproveita os registradores */
/* Pick n (low byte of the argument) monsters to tribute, one per question. */
int duel_08830030(unsigned int p, int id, int arg)
{
    int n = arg & 0xFF;
    int q;
    int z;

    switch (duel_ev_unk) {
    case 0:
        duel_ev_data[3] = n;
        duel_h_0891968E[1] = 0;
        duel_h_0891968E[0] = 0;
        if (duel_0884DF2C(duel_0882FF4C) == 0) {
            return 1;
        }
        duel_0884ACD8(p, 0x12);
        duel_ev_unk++;
        return 0;
    case 1:
        if (duel_player_type[p] == 1) {
            duel_0881956C(p, 1, duel_0882FF4C);
        } else {
            duel_08835978(duel_0882FF4C);
        }
        duel_ev_unk++;
        return 0;
    case 2:
        if (duel_08835AC0() != 0) {
            q = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_h_0891968E[q] |= 1 << z;
            if (n != 0) {
                duel_host_call(0x28, q, z, CARD_UID(duel.players[q & 1].field[z].card));
            } else {
                duel_w_0891968C = (unsigned char)duel_w_0891964C | (unsigned char)(duel_w_08919650 + duel_w_08919654) << 8;
                return 1;
            }
            duel_ev_unk++;
            return 0;
        }
        duel_host_call(0x29, 0, 0, 0);
        duel_ev_unk = 0;
        return 0;
    case 3:
        if (duel_ev_data[3] > 1) {
            duel_ev_data[3]--;
            duel_ev_unk = 1;
            return 0;
        }
        duel_088D1B20(p, duel_h_0891968E[0] | duel_h_0891968E[1] << 16);
        duel_ev_unk++;
        return 0;
    }
    return 1;
}

#endif

/* Zone filter: face-up monsters that the player of the turn may tribute. */
int duel_088302BC(unsigned int p, int z, int i)
{
    int zz = z + i;

    if (zz > 4) {
        return 0;
    }
    if (duel_tributable(duel_turn_player, p, zz) == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[zz].faceup != 0) {
        return 0x800;
    }
    return 0;
}

/* Zone filter: own monsters with the effect of 0x142A that the card may affect. */
int duel_08830358(unsigned int p, int z, int i)
{
    int zz = z + i;

    if (p != duel_w_08919670) {
        return 0;
    }
    if (zz > 4) {
        return 0;
    }
    if (duel.players[p & 1].field[zz].card.id == 0) {
        return 0;
    }
    if (duel_slot_ef098(p, zz, 0x142A) == 0) {
        return 0;
    }
    if (duel_slot_immune(p, zz, p != duel_ev_data[1]) != 0) {
        return 0;
    }
    return 0x800;
}

/* Zone filter: own monsters. */
int duel_0883042C(unsigned int p, int z, int i)
{
    z += i;
    if (p != duel_w_08919670) {
        return 0;
    }
    if (z > 4) {
        return 0;
    }
    if (duel.players[p & 1].field[z].card.id != 0) {
        return 0x800;
    }
    return 0;
}

/* Zone filter: own face-up monsters that are not of attribute 9 and are in group 0x1A58. */
int duel_088304A0(unsigned int p, int z, int i)
{
    int id;

    z += i;
    if (p != duel_w_08919670) {
        return 0;
    }
    if (z > 4) {
        return 0;
    }
    id = duel.players[p & 1].field[z].card.id;
    if (id == 0) {
        return 0;
    }
    if (duel.players[p & 1].field[z].faceup == 0) {
        return 0;
    }
    if (card_is_monster(id) == 0) {
        return 0;
    }
    if (card_is_attribute9(id) != 0) {
        return 0;
    }
    if (card_is_in_group_1a58(id) != 0) {
        return 0x800;
    }
    return 0;
}

/* Zone filter: the hand, or own cards but the one of the argument that the card may affect. */
int duel_08830590(unsigned int p, int z, int i)
{
    if (p == duel_w_08919670) {
        if (z == 0xB) {
            return 0x800;
        }
        z += i;
        if (z < 0xB) {
            if (duel.players[p & 1].field[z].card.id == 0) {
                return 0;
            }
            if (duel_ev_data[1] == (unsigned short)((unsigned char)p | (unsigned char)z << 8)) {
                return 0;
            }
            if (duel_slot_ref_immune(p, z, p != (duel_ev_data[1] & 0xFF)) != 0) {
                return 0;
            }
            return 0x800;
        }
    }
    return 0;
}

/* Zone filter: own cards but the one of the argument that the card may affect. */
int duel_08830664(unsigned int p, int z, int i)
{
    if (p == duel_w_08919670) {
        z += i;
        if (z < 0xB) {
            if (duel.players[p & 1].field[z].card.id == 0) {
                return 0;
            }
            if (duel_ev_data[1] == (unsigned short)((unsigned char)p | (unsigned char)z << 8)) {
                return 0;
            }
            if (duel_slot_ref_immune(p, z, p != (duel_ev_data[1] & 0xFF)) != 0) {
                return 0;
            }
            return 0x800;
        }
    }
    return 0;
}

/* Zone filter: own monsters that the attacker of the argument may attack. */
int duel_08830728(unsigned int p, int z, int i)
{
    int zz;

    if (p == duel_w_08919670) {
        zz = z + i;
        if (zz < 5 && duel_can_attack_target(1 - p, duel_ev_data[1], p, zz, 1) != 0) {
            return 0x800;
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* os registradores a1/a2 das cargas de duel_w_08919650/54 saem trocados */
/* Pick a zone with the filter of the card (the filter is put in the third argument). */
int duel_0883078C(unsigned int p, int id, int (*fn)())
{
    switch (duel_ev_unk) {
    case 0:
        switch (id) {
        case 0x12DC:
            duel_0884ACD8(p, 0x12);
            fn = duel_088302BC;
            break;
        case 0x139C:
            duel_special.player[0] = 1 - p;
            duel_atk_zone = duel_ev_data[1];
            duel_host_call(0xB, duel_atk_zone, p, -1);
            duel_host_call(0xA, 0, 0, 0);
            duel_0884ACD8(p, 0xDE);
            fn = duel_08830728;
            break;
        case 0x142A:
        case 0x18B5:
            duel_0884ACD8(p, 0x95);
            fn = duel_08830358;
            break;
        case 0x1516:
            fn = duel_08830590;
            break;
        case 0x151A:
            duel_0884ACD8(p, 0x4D);
            fn = duel_0883042C;
            break;
        case 0x1544:
        case 0x15F3:
            duel_0884ACD8(p, 0x7B);
            fn = duel_0883042C;
            break;
        case 0x16DF:
            fn = duel_08830664;
            break;
        case 0x198E:
            duel_0884ACD8(p, 0x7C);
            fn = duel_088304A0;
            break;
        }
        if (duel_player_type[p] == 1) {
            duel_0881956C(p, id, fn);
        } else {
            duel_08835978(fn);
        }
        duel_ev_unk++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            duel_w_0891968C = (unsigned char)duel_w_0891964C | (unsigned char)(duel_w_08919650 + duel_w_08919654) << 8;
            duel_h_0891968E[0] = duel_zone_card_uid(duel_w_0891964C, duel_w_08919650, duel_w_08919654);
            duel_ev_unk++;
            return 0;
        }
        duel_ev_unk--;
        return 0;
    }
    return 1;
}

#endif

/* Pick a card of the hand (by the computer: duel_08809B50); 0x1365, 0x1990, 0x1488, 0x133B show a
   message first. */
int duel_088309F8(unsigned int p, int id, int arg)
{
    switch (duel_ev_unk) {
    case 0:
        if (duel_player_type[p] == 1) {
            duel_08809B50(p, (unsigned short)id, (unsigned short)arg);
            duel_w_0891968C = duel_0884C7CC()->owner + (duel_0884C7CC()->uid_hi << 1);
            return 1;
        }
        switch ((unsigned short)id) {
        case 0x133B:
        case 0x1488:
        case 0x1990:
            duel_0884ACD8(p, 0xF0);
            break;
        case 0x1365:
            duel_0884ACD8(p, 0xEF);
            break;
        default:
            return 1;
        }
        duel_ev_unk++;
        return 0;
    case 1:
        duel_0884C588(p, 6, (unsigned short)id, (unsigned short)arg);
        duel_ev_unk++;
        return 0;
    }
    duel_w_0891968C = duel_0884C7CC()->owner + (duel_0884C7CC()->uid_hi << 1);
    return 1;
}

/* Pick a monster type (0x161C offers only the types of the face-up monsters). */
int duel_08830B7C(unsigned int p, int id)
{
    unsigned int mask;
    int q;
    int k;

    switch (duel_ev_unk) {
    case 0:
        mask = -1;
        switch (id) {
        case 0x161C:
            mask = 0;
            for (q = 0; q < 2; q++) {
                for (k = 0; k <= 4; k++) {
                    if (duel.players[q & 1].field[k].card.id != 0 && duel.players[q & 1].field[k].faceup != 0) {
                        mask |= 1 << (unsigned short)duel_field_type(q, k);
                    }
                }
            }
            break;
        }
        if (duel_player_type[p] == 1) {
            duel_w_08919624 = duel_0880B448(p, id);
        } else {
            duel_0884AF34(0xF1, mask);
        }
        duel_ev_unk++;
        return 0;
    }
    duel_w_0891968C = duel_w_08919624;
    return 1;
}

#ifdef NON_MATCHING /* o original guarda os bytes do argumento em t0/t1 e o laço da resposta tem outra forma */
/* Pick a number between the bytes of the argument; the answer is also given as its lowest bit. */
int duel_08830CDC(unsigned int p, int id, int arg)
{
    int hi = (unsigned char)((arg & 0xFFFF) >> 8);
    int lo = arg & 0xFF;
    unsigned int r;
    int k;

    switch (duel_ev_unk) {
    case 0:
        if (duel_player_type[p] == 1) {
            duel_w_08919624 = duel_0880BD00();
        } else {
            duel_0884AF48(0xF2, lo, hi);
        }
        duel_ev_unk++;
        return 0;
    case 1:
        r = duel_w_08919624;
        k = 1;
        if (!(r & 2) != 0) {
            k++;
            for (;;) {
                if (!(r & 1 << k) == 0) {
                    break;
                }
                k++;
            }
        }
        duel_w_0891968C = r;
        duel_h_0891968E[0] = k;
        return 1;
    }
    return 1;
}

#endif

/* Shows the deck of the opponent (0xF9, then duel_0884C588 kind 2). */
int duel_08830DC0(unsigned int p)
{
    unsigned int q;

    if (duel_player_type[p] == 1) {
        return 1;
    }
    q = 1 - p;
    if (duel.players[q & 1].n_d == 0) {
        return 1;
    }
    switch (duel_ev_unk) {
    case 0:
        duel_0884ACD8(p, 0xF9);
        duel_ev_unk++;
        return 0;
    case 1:
        duel_0884C588(q, 2, -1, 0xD);
        duel_ev_unk++;
        return 0;
    }
    return 1;
}
