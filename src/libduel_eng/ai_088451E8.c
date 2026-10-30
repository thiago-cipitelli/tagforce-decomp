// @range 0x088451E8-0x08846498
// @asm 0x088451E8-0x08846498
/* Computer player: step machine of the end phase (check tables, cards that pay or keep a cost, end-of-turn effects). */
#include "duel.h"

typedef int (*DuelAiCheck)(unsigned int p);

extern unsigned int duel_turn_player;
extern unsigned int duel_w_08919600;   /* step */
extern unsigned int duel_w_08919604;   /* table index / player offset */
extern unsigned int duel_w_08919608;   /* zone index */
extern int duel_w_08919624;            /* answer of the yes/no prompt */
extern int duel_w_0891964C;            /* zone picked by duel_08835978: player */
extern int duel_w_08919650;
extern int duel_w_08919654;
extern int duel_w_089147D8;
extern int duel_player_type[2];
extern DuelAiCheck duel_ai_checks_08907E38[67];
extern DuelAiCheck duel_ai_checks_08907F44[4];

int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_slot_has_effect2(unsigned int p, int z, int id);
int duel_0884ACD8(unsigned int p, int msg);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_088AFD5C(int kind, unsigned int a);
int field_get_card(unsigned int p, int z);
int card_is_monster(int id);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
int duel_count_tributable_except(unsigned int p, unsigned short src);
int duel_08845070(int id);
int duel_sum_e68a0(unsigned int p);
int duel_088D1438(unsigned int p, int z, unsigned int q, unsigned short id);
int duel_count_active(int id);
void duel_088C4564(unsigned int p, int n);
int duel_slot_e12e0(unsigned int p, int z, int id, unsigned int q);
int duel_08809F2C(unsigned int p);
unsigned short *card_get_name(int id);
void duel_wfmt_s();
void duel_wfmt_d();
int duel_0884AD18(int msg);
int duel_08845124(unsigned int p, int z, int i);
int duel_08845190(unsigned int p, int z, int i);
void duel_08835978(int (*fn)());
int duel_08835AC0(void);
void duel_088D1AC8(unsigned int p, int z);
void duel_088CC284(unsigned int p, int z, int n);
void duel_088C07FC(unsigned int p, int z, int c);
void duel_088C28A4(unsigned int p, int z, int c);
void duel_088C09C8(unsigned int p, int c);
void duel_088CC1D0(unsigned int p, int z, int a);
int duel_slot_value_1909(unsigned int p, int z);
int duel_slot_effect_arg(unsigned int p, int z, int id);
void duel_088C7BA4(void);
int duel_088C4730(unsigned int p, int n, int f, int id);
void duel_088C7CF0(void);
unsigned int duel_card_locate(unsigned int uid);
void duel_088C44E4(unsigned int p, int id);

#ifdef NON_MATCHING /* forma quase igual; difere a alocação de registradores (z, q e id trocados, `me` não vai para a pilha) */
/* returns 0 while the step machine has work left */
int duel_088451E8(void)
{
    unsigned int p = duel_turn_player;
    unsigned int me = p & 1;
    int b24 = duel.players[me].flags >> 24 & 1;
    int skip;                          /* no automatic resolutions (flag bit 24 or 0x137E in the hand) */
    unsigned int k;
    unsigned int q;
    int z;
    int id;
    int r;
    int n;
    int lp;
    unsigned int e;
    DuelEffectBits *ef;
    unsigned int uid;
    unsigned int loc;
    unsigned short txt2[256];
    unsigned short txt1[256];
    unsigned short txt120[256];
    unsigned short txt130[256];

    skip = b24 != 0;
    if (!skip) {
        skip = duel_slot_has_effect2(p, 0xB, 0x137E) != 0;
    }
    switch (duel_w_08919600) {
    case 0:
        if (skip) {
            if (!b24) {
                duel_0884ACD8(p, 0x134);
            }
            duel_w_08919600 = 200;
            return 0;
        }
        duel_cmd((p ? 0x8000 : 0) | 0xD, 0, 0, 0);
        duel_w_08919600++;
        duel_w_08919604 = 0;
        duel_w_08919608 = 0;
        return 0;
    case 1: /* the end-phase check tables */
        for (k = 0; k < 4; k++) {
            if (duel_ai_checks_08907F44[k](p) == 0) {
                return 0;
            }
        }
        for (; duel_w_08919604 < 67; duel_w_08919604++) {
            if (duel_ai_checks_08907E38[duel_w_08919604](p) == 0) {
                return 0;
            }
            duel_w_08919608 = 0;
        }
        duel_w_08919600++;
        return 0;
    case 2:
        duel_088AFD5C(2, 0);
        duel_w_089147D8 = 0;
        duel_w_08919600++;
        return 0;
    case 3: /* wait, then run the first table again */
        if (duel_w_089147D8 != 0) {
            duel_w_08919600--;
            return 0;
        }
        for (k = 0; k < 4; k++) {
            if (duel_ai_checks_08907F44[k](p) == 0) {
                return 0;
            }
        }
        duel_w_08919600 = 100;
        return 0;
    case 100:
        duel_w_08919604 = 0;
        duel_w_08919608 = 0;
        duel_w_08919600++;
    case 101: /* cards of both players that ask for a cost to stay on the field */
        for (; duel_w_08919604 < 2; duel_w_08919604++) {
            for (; duel_w_08919608 < 10; duel_w_08919608++) {
                z = duel_w_08919608;
                q = p ^ duel_w_08919604;
                if (duel.players[q & 1].field[z].card.id == 0 || duel.players[q & 1].field[z].faceup == 0) {
                    continue;
                }
                id = field_get_card(q, z);
                r = 0;
                if (card_is_monster(id)) {
                    if (z >= 5) {
                        continue;
                    }
                } else if (z < 5) {
                    continue;
                }
                switch (id) {
                case 0x1358:
                case 0x144A: /* tribute a monster */
                    if (q == p) {
                        duel_088C4448(p, z, CARD_UID(duel.players[me].field[z].card));
                        if (duel_count_tributable_except(p, (unsigned char)p | (unsigned char)z << 8) > 0) {
                            duel_w_08919600 = 120;
                            return 0;
                        }
                        r = 1;
                    }
                    break;
                case 0x1381:
                case 0x134A:
                case 0x1360:
                case 0x13F4:
                case 0x13F9:
                case 0x151A:
                case 0x1639: /* pay life points */
                    if (q == p) {
                        duel_088C4448(p, z, CARD_UID(duel.players[me].field[z].card));
                        lp = duel.players[me].lp;
                        if (duel_08845070(id) >= lp) {
                            r = 1;
                        } else {
                            duel_w_08919600 = 110;
                            return 0;
                        }
                    }
                    break;
                case 0x161B:
                    duel_088C4448(q, z, CARD_UID(duel.players[q & 1].field[z].card));
                    if (duel_sum_e68a0(q) == 0) {
                        r = 1;
                    } else {
                        duel_w_08919600 = 130;
                        return 0;
                    }
                    break;
                }
                if (r) {
                    duel_088D1438(q, z, 0, 0);
                }
            }
            duel_w_08919608 = 0;
        }
        duel_w_08919600++;
        return 0;
    case 102: /* 0x168C-0x1692: pay life points or be destroyed */
        q = 1 - p;
        for (; duel_w_08919608 < 10; duel_w_08919608++) {
            z = duel_w_08919608;
            if (duel.players[me].field[z].card.id == 0 || duel.players[me].field[z].faceup == 0) {
                continue;
            }
            id = field_get_card(p, z);
            r = 0;
            if (card_is_monster(id)) {
                if (z >= 5) {
                    continue;
                }
            } else if (z < 5) {
                continue;
            }
            switch (id) {
            case 0x168C:
            case 0x168D:
            case 0x168E:
            case 0x168F:
            case 0x1690:
            case 0x1691:
            case 0x1692:
                if (duel_count_active(0x169F) == 0) {
                    duel_088C4448(p, z, CARD_UID(duel.players[me].field[z].card));
                    lp = duel.players[me].lp;
                    if (duel_08845070(id) < lp) {
                        n = duel_08845070(id);
                        duel_088C4564(p, n);
                        if (duel_slot_e12e0(p, duel_w_08919608, 0x16A2, p) != 0 && n < duel.players[q & 1].lp) {
                            duel_088C4564(q, n);
                        }
                        if (duel_slot_e12e0(p, duel_w_08919608, 0x16A2, q) != 0 && n * 2 < duel.players[me].lp) {
                            duel_088C4564(p, n);
                        }
                    } else {
                        r = 1;
                    }
                }
                break;
            }
            if (r) {
                duel_088D1438(p, z, 0, 0);
                duel_w_08919608++;
                return 0;
            }
        }
        duel_w_08919600 = 150;
        return 0;
    case 110: /* life point cost: ask the player (or decide for the computer) */
        id = field_get_card(p, duel_w_08919608);
        if (duel_player_type[p] == 1) {
            duel_w_08919624 = 1;
            switch (id) {
            case 0x1381:
                duel_w_08919624 = !(duel.players[me].lp < 6001);
                break;
            case 0x1360:
                duel_w_08919624 = !(duel_08809F2C(p) > 0);
                break;
            case 0x1639:
                duel_w_08919624 = duel_08809F2C(p) < 0;
                break;
            }
            if (SLOT_ACTIVE(&duel.players[(int)me].field[duel_w_08919608]) == 0) {
                duel_w_08919624 = 0;
            }
        } else {
            duel_wfmt_s(txt1, 0xE8, card_get_name(id));
            duel_wfmt_d(txt2, txt1, duel_08845070(id));
            duel_0884AD18((int)txt2);
        }
        duel_w_08919600++;
        return 0;
    case 111: /* pay or destroy */
        if (duel_w_08919624 != 0) {
            duel_088C4564(p, duel_08845070(field_get_card(p, duel_w_08919608)));
        } else {
            duel_088D1438(p, duel_w_08919608, 0, 0);
        }
        duel_w_08919608++;
        duel_w_08919600 = 101;
        return 0;
    case 120: /* tribute cost: ask the player */
        if (duel_player_type[p] == 1) {
            duel_w_08919624 = 0;
            duel_w_08919600++;
            return 0;
        }
        duel_wfmt_s(txt120, 0xE9, card_get_name(field_get_card(p, duel_w_08919608)));
        duel_0884AD18((int)txt120);
        duel_w_08919600++;
        return 0;
    case 121:
        if (duel_w_08919624 != 0) {
            duel_w_08919600++;
            duel_08835978(duel_08845124);
            return 0;
        }
        duel_088D1438(p, duel_w_08919608, 0, 0);
        duel_w_08919608++;
        duel_w_08919600 = 101;
        return 0;
    case 122: /* tribute the picked monster */
        if (duel_08835AC0() != 0) {
            duel_088D1AC8(duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            duel_w_08919608++;
            duel_w_08919600 = 101;
        } else {
            duel_w_08919600 = 120;
        }
        return 0;
    case 130: /* 0x161B: ask the controller */
        q = p ^ duel_w_08919604;
        if (duel_player_type[q] == 1) {
            duel_w_08919624 = 0;
            duel_w_08919600++;
            return 0;
        }
        duel_wfmt_s(txt130, 0xDB, card_get_name(field_get_card(q, duel_w_08919608)));
        duel_0884AD18((int)txt130);
        duel_w_08919600++;
        return 0;
    case 131:
        if (duel_w_08919624 != 0) {
            duel_w_08919600++;
            duel_08835978(duel_08845190);
            return 0;
        }
        duel_088D1438(p ^ duel_w_08919604, duel_w_08919608, 0, 0);
        duel_w_08919608++;
        duel_w_08919600 = 101;
        return 0;
    case 132:
        if (duel_08835AC0() != 0) {
            duel_088CC284(duel_w_0891964C, duel_w_08919650 + duel_w_08919654, -1);
            duel_w_08919608++;
            duel_w_08919600 = 101;
        } else {
            duel_w_08919600 = 130;
        }
        return 0;
    case 150:
        duel_w_08919600 = 200;
        return 0;
    case 200: /* end-of-turn effects of the turn player's cards */
        for (z = 0; z < 5; z++) {
            if (duel.players[me].field[z].card.id == 0 || duel.players[me].field[z].faceup == 0) {
                continue;
            }
            switch ((unsigned short)duel.players[me].field[z].card.id) {
            case 0xFF9:
                duel_088C07FC(p, z, skip);
                break;
            case 0x14AC:
                duel_088C28A4(p, z, skip);
                break;
            }
        }
        duel_088C09C8(p, skip);
        q = 1 - p;
        for (z = 5; z < 10; z++) {
            if (duel.players[me].field[z].card.id == 0 || duel.players[me].field[z].faceup == 0) {
                continue;
            }
            switch ((unsigned short)duel.players[me].field[z].card.id) {
            case 0x130D:
            case 0x131A:
            case 0x180F:
                duel_088CC1D0(p, z, 1);
                break;
            case 0x159C: /* return the card it holds to the hand */
                duel_088CC1D0(p, z, 1);
                if (!skip && !(duel.players[me].field[z].flags >> 5 & 1) && duel_slot_value_1909(p, z) + 1 == 2) {
                    uid = duel_slot_effect_arg(p, z, 0x159C);
                    duel_088C4448(p, z, CARD_UID(duel.players[me].field[z].card));
                    duel_088D1438(p, z, 0, 0);
                    duel_cmd(0x3F, (uid & 1) | 0x1E | (uid & 0x3FF) << 6,
                             me | 0x16 | (duel.players[(int)me].n_hand & 0x3FF) << 6, 0);
                }
                break;
            case 0x15A0: /* both players lose 200 << counter */
                duel_088CC1D0(p, z, 1);
                if (!skip && !(duel.players[me].field[z].flags >> 5 & 1)) {
                    n = duel_slot_value_1909(p, z);
                    duel_088C4448(p, z, CARD_UID(duel.players[me].field[z].card));
                    duel_088C7BA4();
                    n = 200 << n;
                    duel_088C4730(p, n, 0, 0x15A0);
                    duel_088C4730(q, n, 1, 0x15A0);
                    duel_088C7CF0();
                }
                break;
            case 0x15EE:
                duel_088CC1D0(p, z, 1);
                break;
            case 0x17A1:
            case 0x187C:
                duel_088CC1D0(p, z, 1);
                if (!skip && duel_slot_value_1909(p, z) + 1 == 2) {
                    duel_088C4448(p, z, CARD_UID(duel.players[me].field[z].card));
                    duel_088D1438(p, z, 0, 0);
                }
                break;
            case 0x1AA9:
                duel_088CC1D0(p, z, 1);
                if (!skip && duel_slot_value_1909(p, z) + 1 == 3) {
                    duel_088C4448(p, z, CARD_UID(duel.players[me].field[z].card));
                    duel_088D1438(p, z, 0, duel.players[me].field[z].card.id);
                }
                break;
            case 0x1A7E:
                if (skip) {
                    duel_088CC1D0(p, z, 1);
                }
                break;
            }
        }
        /* 0x12C8 in the hand: count down its effects, return the banished card at the end */
        if (duel_slot_has_effect(p, 0xB, 0x12C8) != 0 && (e = duel.players[me].effects) != 0) {
            do {
                ef = (DuelEffectBits *)&duel.effects[e];
                e = ef->next;
                if (ef->kind == 1 && ef->src == 0x12C8) {
                    duel_cmd((p ? 0x8000 : 0) | 0x3B, 0x12C8, 1, 4 - (ef->kind_hi >> 1));
                    if (!skip && ef->kind_hi == 0) {
                        uid = ef->arg;
                        loc = duel_card_locate(uid);
                        if ((loc & 0xFFFF) >> 8 == 0xF) {
                            duel_088C44E4(p, 0x12C8);
                            duel_cmd(0x3F, (loc & 0xFF & 1) | 0x1E | (uid & 0x3FF) << 6,
                                     me | 0x16 | (duel.players[(int)me].n_hand & 0x3FF) << 6, 0);
                        }
                    }
                }
            } while (e != 0);
        }
        duel_w_08919600++;
        return 0;
    }
    return 1;
}
#endif
