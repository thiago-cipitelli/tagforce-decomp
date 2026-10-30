// @range 0x088420F4-0x08842C24
// @asm 0x088420F4-0x08842C24
/* Computer player: step machine of the standby phase (cards that act at the start of the turn, then the check tables). */
#include "duel.h"

typedef int (*DuelAiCheck)(unsigned int p);

extern unsigned int duel_w_08919600;   /* step */
extern unsigned int duel_w_08919604;   /* sub-step / table index */
extern unsigned int duel_w_08919608;   /* zone index */
extern unsigned int duel_turn_player;
extern int duel_w_08919624;
extern int duel_w_089147D8;
extern int duel_player_type[2];
extern DuelAiCheck duel_ai_checks_08907CE4[40];
extern DuelAiCheck duel_ai_checks_08907D84[32];
extern DuelAiCheck duel_ai_checks_08907E04[5];

int duel_08841B54(unsigned int p);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_088AFD5C(int kind, unsigned int a);
int field_get_card(unsigned int p, int z);
void duel_088CC1D0(unsigned int p, int z, int a);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
unsigned short *card_get_name(int id);
void duel_wfmt_s();
int duel_0884AD18(int msg);
void duel_08832624(unsigned int p, unsigned int id, int a, int b);
int duel_088D1438(unsigned int p, int z, unsigned int q, unsigned short id);
int duel_088D189C(unsigned int p, int z, unsigned int q, unsigned short id);
unsigned short duel_slot_e1854(unsigned int p, int z);
int duel_slot_counter_1a90(unsigned int p, int z);
int duel_slot_ref_immune(unsigned int p, int z, int opp);
void duel_088CC594(unsigned int p, int z, int n, int id);
int duel_slot_value_1909(unsigned int p, int z);
int duel_card_can_affect(unsigned int p, int z, unsigned short id, int zone, int arg);
int duel_slot_min_1130(unsigned int p, int z);

#ifdef NON_MATCHING /* forma igual; difere a alocação de registradores dos casos 4 e 5 e o original mantém o duel_w_08919600++ do fim do caso 6, que o mwcc elimina por ser sobrescrito no caso 7 */
/* returns 0 while the step machine has work left */
int duel_088420F4(void)
{
    unsigned int p = duel_turn_player;
    unsigned int me = p & 1;
    int quiet = duel.players[me].flags >> 24 & 1;   /* player flag bit 24: skips the automatic resolutions [?] */
    int id;
    unsigned short loc;
    unsigned int lp;
    unsigned int lz;
    unsigned int q;
    int i;
    DuelAiCheck *f;
    unsigned int k;
    int z;
    int j;
    int r;
    unsigned int n;
    unsigned short buf[256];

    if (duel_w_08919600 != 0 && duel_08841B54(p) == 0) {
        return 0;
    }
    switch (duel_w_08919600) {
    case 0: /* announce the phase */
        if (!quiet) {
            duel_cmd((p ? 0x8000 : 0) | 0x11, 0, 0, 0);
        }
        duel_w_08919600++;
        duel_w_08919604 = 0;
        duel_w_08919608 = 0;
        return 0;
    case 1:
        if (!quiet) {
            duel_088AFD5C(5, 0);
            duel_w_089147D8 = 0;
        }
        duel_w_08919600++;
        return 0;
    case 2: /* wait for the previous step to finish */
        if (duel_w_089147D8 != 0) {
            duel_w_08919600--;
            return 0;
        }
        duel_w_08919600++;
        duel_w_08919604 = 0;
        duel_w_08919608 = 0;
    case 3: /* own cards with a standby effect */
        for (; duel_w_08919608 < 10; duel_w_08919608++) {
            z = duel_w_08919608;
            id = field_get_card(p, z);
            if (id == 0 || duel.players[me].field[z].faceup == 0) {
                continue;
            }
            switch (id) {
            case 0xFEE:
                if (duel_w_08919608 >= 5 && !(duel.players[me].field[z].flags >> 5 & 1)) {
                    duel_088CC1D0(p, z, 1);
                    duel_w_08919608++;
                    return 0;
                }
                break;
            case 0x150E: /* ask whether to use the effect, then resolve it */
                if (duel_w_08919608 >= 5) {
                    switch (duel_w_08919604) {
                    case 0:
                        duel_088C4448(p, duel_w_08919608, CARD_UID(duel.players[(int)me].field[duel_w_08919608].card));
                        duel_w_08919604++;
                        break;
                    case 1:
                        if (duel.players[me].n_hand != 0) {
                            if (duel_player_type[p] == 1) {
                                duel_w_08919624 = 1;
                            } else {
                                duel_wfmt_s(buf, 0xEE, card_get_name(id));
                                duel_0884AD18((int)buf);
                            }
                        } else {
                            duel_w_08919624 = 0;
                        }
                        duel_w_08919604++;
                        break;
                    case 2:
                        if (duel_w_08919624 != 0) {
                            duel_08832624(p, 1, 0, 0);
                        } else {
                            duel_088D1438(p, z, 0, 0);
                        }
                        duel_w_08919604 = 0;
                        duel_w_08919608++;
                        break;
                    }
                    return 0;
                }
                break;
            case 0x1A9C:
                if (duel_w_08919608 >= 5 && !(duel.players[me].field[z].flags >> 5 & 1)) {
                    loc = duel_slot_e1854(p, z);
                    if (loc != 0xFFFF) {
                        lz = loc >> 8 & 0xFF;
                        lp = loc & 0xFF;
                        if (duel_slot_counter_1a90(lp, lz) != 0 && duel_slot_ref_immune(lp, lz, lp != p) == 0) {
                            duel_088CC594(lp, lz, -1, 0x1A90);
                            duel_w_08919608++;
                            return 0;
                        }
                    }
                }
                break;
            }
        }
        duel_w_08919600++;
        duel_w_08919608 = 5;
    case 4: /* opponent's spells/traps that act on this turn */
        p = 1 - p;
        for (; duel_w_08919608 < 10; duel_w_08919608++) {
            z = duel_w_08919608;
            id = duel.players[p & 1].field[z].card.id;
            if (id == 0 || duel.players[p & 1].field[z].faceup == 0) {
                continue;
            }
            switch ((unsigned short)id) {
            case 0x1102:
            case 0x1AA3:
                duel_088CC1D0(p, z, 1);
                if (!quiet && duel_slot_value_1909(p, z) + 1 == 3) {
                    duel_088D1438(p, z, 0, 0);
                }
                duel_w_08919608++;
                return 0;
            case 0x1400:
                duel_088CC1D0(p, z, 1);
                if (!quiet && duel_slot_value_1909(p, z) + 1 == 2) {
                    duel_088D1438(p, z, 0, 0);
                }
                duel_w_08919608++;
                return 0;
            }
        }
        duel_w_08919600++;
        return 0;
    case 5: /* 0x149D of both players pointing at a card of the turn player */
        for (i = 0; i < 2; i++) {
            q = p ^ i;
            for (j = 5; j < 10; j++) {
                if (duel.players[q & 1].field[j].card.id != 0x149D || duel.players[q & 1].field[j].faceup == 0) {
                    continue;
                }
                loc = duel_slot_e1854(q, j);
                lp = loc & 0xFF;
                lz = loc >> 8 & 0xFF;
                if (lp != p) {
                    continue;
                }
                duel_088CC1D0(q, j, 1);
                if (!quiet && !(duel.players[q & 1].field[j].flags >> 5 & 1) && duel_slot_value_1909(q, j) == 1) {
                    r = 0;
                    duel_088C4448(q, j, CARD_UID(duel.players[q & 1].field[j].card));
                    duel.players[q & 1].field[j].card.id = 0;
                    if (duel_card_can_affect(lp, lz, 0x149D, j, 1) != 0) {
                        r = duel_088D1438(lp, lz, q != lp, 0x149D);
                    }
                    duel.players[q & 1].field[j].card.id = 0x149D;
                    if (r != 0) {
                        duel_088D189C(q, j, 0, 0);
                    }
                }
            }
        }
        duel_w_08919600++;
        duel_w_08919608 = 0;
    case 6: /* monster counters that run out */
        for (; duel_w_08919608 < 5; duel_w_08919608++) {
            z = duel_w_08919608;
            if (duel.players[me].field[z].card.id != 0 && (r = duel_slot_min_1130(p, z)) > 0) {
                duel_cmd((p ? 0x8000 : 0) | 0x47, z, n = r - 1, 0);
                if (!quiet && n == 0) {
                    duel_088D1438(p, z, 1, 0);
                    duel_w_08919608++;
                    return 0;
                }
            }
        }
        duel_w_08919600++;
        duel_w_08919608 = 0;
    case 7:
        if (quiet) {
            duel_w_08919600 = 20;
            return 0;
        }
        duel_w_08919600 = 10;
    case 10:
        duel_w_08919600++;
        duel_w_08919604 = 0;
        duel_w_08919608 = 0;
    case 11:
        for (; duel_w_08919604 < 40; duel_w_08919604++) {
            if (duel_ai_checks_08907CE4[duel_w_08919604](p) == 0) {
                return 0;
            }
            duel_w_08919608 = 0;
        }
        duel_w_08919600++;
        duel_w_08919604 = 0;
        duel_w_08919608 = 0;
        return 0;
    case 12:
        duel_w_08919608 = 0;
        for (k = 0, f = duel_ai_checks_08907D84; k < 32; k++, f++) {
            if ((*f)(p) == 0) {
                return 0;
            }
            duel_w_08919608 = 0;
        }
        duel_w_08919600 = 20;
        duel_w_08919604 = 0;
        duel_w_08919608 = 0;
        return 0;
    case 20:
        for (; duel_w_08919604 < 5; duel_w_08919604++) {
            if (duel_ai_checks_08907E04[duel_w_08919604](p) == 0) {
                return 0;
            }
            duel_w_08919608 = 0;
        }
        duel_w_08919600++;
        duel_w_08919604 = 0;
        duel_w_08919608 = 0;
        return 0;
    }
    return 1;
}
#endif
