// @range 0x08850164-0x08850F44
// @asm 0x08850164-0x08850F44
/* Computer player: resolving the chain built in duel_chain.acts (step duel_w_089148EC). */
#include "duel.h"

typedef struct DuelEvFlags {           /* 0x08914341, see duel_event.c */
    unsigned char b0 : 1;
    unsigned char b1 : 1;
    unsigned char b2 : 1;
    unsigned char b3 : 1;
    unsigned char b4 : 1;
    unsigned char b5 : 1;
    unsigned char b6 : 2;
} DuelEvFlags;

extern DuelEvFlags duel_ev_flags;
extern unsigned int duel_turn_player;
extern int duel_w_089195EC;
extern int duel_w_089147D8;            /* next chain link to announce */
extern int duel_w_089148E8;
extern int duel_w_089148EC;            /* step */
extern int duel_w_089148F0;
extern int duel_w_089148F4;
extern int duel_w_08914904;
extern int duel_w_08914908;
extern int duel_w_0891490C;
extern unsigned int duel_w_08914920;
extern int duel_w_08914924;
extern int duel_w_089149D4;
extern int duel_w_089149D8;
extern int duel_ai_step;
extern DuelAct *duel_w_089147DC;

int duel_0884D1E0(void);
int duel_088C0CD0(void);
void duel_088AFEE0(int a);
int duel_088C3F80(void);
int duel_0884F930(DuelAct *a);
int duel_0884ED08();
int duel_0884EB10(DuelAct *a, DuelAct *b);
int duel_0884EB5C(DuelAct *a);
int duel_0884EB90(DuelAct *a, DuelAct *b);
int duel_0884EBDC(DuelAct *a);
int duel_0884F038(DuelAct *a);
int duel_0884FA28(DuelAct *a);
int duel_0884FD04(DuelAct *a, unsigned int p);
int duel_0886B0E4(unsigned int code, unsigned short uid, int arg);
void duel_0884F07C(int which, unsigned int code, unsigned short uid, unsigned int arg);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_msg_send(unsigned short cmd, int arg, void *data, int len);
void duel_memcpy(void *dst, void *src, int n);
int field_get_card(unsigned int p, int z);
int card_is_monster(int id);
int card_get_type(int id);
int card_get_prop5_hi(int id);
int card_is_in_group_1983(int id);
int duel_special_get_id(DuelSlot *s);

#ifdef NON_MATCHING /* só a alocação de registradores do caso 0xB difere (q, z e o ponteiro do campo) */
int duel_08850164(void)
{
    DuelAct *cur = &duel_chain.acts[duel_chain.len];
    DuelAct *prev = duel_chain.len > 0 ? cur - 1 : 0;
    DuelAct *last = &duel_chain.acts[duel_chain.unk480 - 1];
    DuelAct *t;
    unsigned char k;
    int q;
    int i;
    int z;
    int card;
    int r;
    int n;
    unsigned int code;
    int uid;

    switch (duel_w_089148EC) {
    case 0:
        if (duel_0884D1E0() != 0) {
            return 1;
        }
        if (duel_088C0CD0() != 0) {
            return 1;
        }
        duel_w_08914924 = 1;
        duel_chain.len = 0;
        duel_w_089147D8 = 0;
        duel_088AFEE0(0);
        duel_w_089148EC++;
        return 1;
    case 1:
        if (duel_0884F930(cur) != 0) {
            if (duel_0884ED08() != 0) {
                k = cur->zone;
                if (k != 0x10) {
                    duel_cmd((cur->player != 0 ? 0x8000 : 0) | 0x28, k, cur->uid, 0);
                }
            } else {
                duel_088C4448(cur->player, cur->zone, cur->uid);
            }
        }
        if (cur->b3_4 == 0) {
            for (q = 0; q < 2; q++) {
                for (z = 0; z <= 10; z++) {
                    if (duel.players[q & 1].field[z].faceup == 0) {
                        continue;
                    }
                    card = field_get_card(q, z);
                    r = 0;
                    if (card_is_monster(card) != 0) {
                        if (z >= 5) {
                            continue;
                        }
                    } else if (z <= 4) {
                        continue;
                    }
                    if (card_get_type(cur->id) == 0x16) {
                        if (card_is_in_group_1983(card) != 0) {
                            r = 1;
                        }
                        switch (card) {
                        case 0x1520:
                            switch (card_get_prop5_hi(cur->id)) {
                            case 0:
                            case 5:
                                r = 1;
                            }
                            break;
                        case 0x15FE:
                        case 0x171D:
                            r = 1;
                            break;
                        case 0x16BB:
                            r = cur->id == 0x12EC;
                            break;
                        case 0x1964:
                        case 0x1982:
                            r = card_get_prop5_hi(cur->id) == 0;
                            break;
                        }
                    } else if (card_get_type(cur->id) == 0x17) {
                        switch (card) {
                        case 0x184F:
                            r = 1;
                            break;
                        case 0x1A22:
                        case 0x1A26:
                        case 0x1A37:
                            r = card_get_prop5_hi(cur->id) == 1;
                            break;
                        }
                    }
                    if (r) {
                        duel_088C87A8(q, z, 0x161A, 2, cur->uid);
                    }
                }
            }
        }
        duel_ev_flags.b3 = 0;
        duel_ev_flags.b5 = 0;
        duel_w_08914904 = 0;
        duel_w_08914908 = 0;
        duel_w_089148EC++;
        return 1;
    case 2:
        duel_w_089147DC = cur;
        if (cur->b15 || duel_0884EB5C(cur) == 0) {
            cur->b15 = 1;
            duel_w_089148EC += 2;
            return 1;
        }
        if (duel_0884F038(cur) != 0) {
            duel_msg_send(0x1C, duel_chain.len, cur, 0x18);
        }
        duel_w_089148EC++;
        duel_w_0891490C = 0;
    case 3:
        if (duel_0884F038(cur) == 0) {
            r = duel_0884EB10(cur, prev);
            if (r != 0) {
                cur->b15 = r > 0;
                duel_ev_flags.b5 = 1;
            }
        }
        if (duel_ev_flags.b5) {
            if (cur->b15) {
                if (duel_0884F930(cur) != 0 && duel_0884ED08() != 0) {
                    duel_088C4448(cur->player, cur->zone, cur->uid);
                }
                duel_w_089148EC++;
            } else {
                duel_w_089148EC = 6;
            }
        }
        return 1;
    case 4:
        duel_w_089147DC = cur;
        if (cur->b4_0 || duel_0884EBDC(cur) == 0) {
            cur->b4_0 = 1;
            duel_w_089148EC += 2;
            return 1;
        }
        if (duel_0884F038(cur) != 0) {
            duel_msg_send(0x1A, duel_chain.len, cur, 0x18);
        }
        duel_w_089148EC++;
        duel_w_0891490C = 0;
    case 5:
        if (duel_0884F038(cur) == 0) {
            r = duel_0884EB90(cur, prev);
            if (r != 0) {
                cur->b4_0 = r > 0;
                duel_ev_flags.b3 = 1;
            }
        }
        if (duel_ev_flags.b3) {
            if (cur->b4_0) {
                duel_w_089148EC++;
            } else {
                duel_w_089148EC = 6;
            }
        }
        return 1;
    case 6:
        if (cur->b15 && cur->b4_0) {
            if (duel_w_089195EC != 0) {
                duel_msg_send(0x12, duel_chain.len, cur, 0x18);
            }
            duel_chain.len++;
        } else {
            k = cur->zone;
            if (k < 11 && duel.players[cur->player & 1].field[k].flags >> 1 & 1) {
                duel_cmd((cur->player != 0 ? 0x8000 : 0) | 0x39, k, 0, 0);
            }
            if (--duel_chain.unk480 == 0) {
                if (duel_w_08914920 != 0) {
                    duel_w_089149D4 = 1;
                    duel_w_089149D8 = 0;
                }
                duel_w_089148E8 = 0;
                duel_w_089148F0 = 0;
                duel_w_08914924 = 0;
                return 1;
            }
            for (i = duel_chain.len; i < duel_chain.unk480; i++) {
                duel_memcpy(&duel_chain.acts[i], &duel_chain.acts[i + 1], 0x18);
            }
        }
        if (duel_chain.len < duel_chain.unk480) {
            duel_w_089148EC = 1;
            return 1;
        }
        duel_w_089148EC++;
    case 7:
        if (duel_088C3F80() != 0) {
            return 1;
        }
        if (duel_0884D1E0() != 0) {
            return 1;
        }
        if (duel_chain.unk480 < 2) {
            duel_w_089148EC = 0xB;
            return 1;
        }
        duel_w_089148EC++;
        return 1;
    case 8:
        for (; duel_w_089147D8 < duel_chain.unk480; duel_w_089147D8++) {
            t = &duel_chain.acts[duel_w_089147D8];
            z = t->zone;
            duel_cmd((t->player != 0 ? 0x8000 : 0) | 0x12, z, duel_w_089147D8 + 1, 0);
        }
        duel_w_089148EC += 3;
        return 1;
    case 0xB:
        if (duel_chain.unk480 >= 16 || duel_0884FA28(last) == 0) {
            goto dflt;
        }
        if (card_is_monster(last->id) != 0) {
            n = 0;
            for (i = 0; i < 2; i++) {
                q = duel_turn_player ^ i;
                for (z = 0; z < 5; z++) {
                    if (field_get_card(q, z) != 0x1771) {
                        continue;
                    }
                    if (duel.players[q & 1].field[z].faceup == 0) {
                        continue;
                    }
                    if (duel.players[q & 1].field[z].flags >> 1 & 1) {
                        continue;
                    }
                    code = (q << 31 & 0x80000000) | 0x200000 | ((unsigned int)z << 16 & 0x1F0000) | 0x1771;
                    uid = CARD_UID(duel.players[q & 1].field[z].card);
                    if (duel_0886B0E4(code, uid, n) != 0) {
                        duel_0884F07C(1, code, uid, n);
                        n++;
                    }
                }
                if (q == duel_special.player[0] && duel_special_get_id(&duel_special.slot[0]) == 0x1771) {
                    code = (q << 31 & 0x80000000) | 0x200000 | (duel_special.zone[0] << 16 & 0x1F0000) | 0x1771;
                    uid = CARD_UID(duel_special.slot[0].card);
                    if (duel_0886B0E4(code, uid, n) != 0) {
                        duel_0884F07C(1, code, uid, n);
                        n++;
                    }
                }
                if (q == duel_special.player[1] && duel_special_get_id(&duel_special.slot[1]) == 0x1771) {
                    code = (q << 31 & 0x80000000) | 0x200000 | (duel_special.zone[1] << 16 & 0x1F0000) | 0x1771;
                    uid = CARD_UID(duel_special.slot[1].card);
                    if (duel_0886B0E4(code, uid, n) != 0) {
                        duel_0884F07C(1, code, uid, n);
                        n++;
                    }
                }
            }
            if (n != 0) {
                duel_w_089148EC = 1;
                return 1;
            }
        }
        duel_ai_step = 0;
        duel_w_089149D8 = 0;
        duel_w_089148EC++;
    case 0xC:
        if (duel_0884FD04(last, 1 - last->player) != 0) {
            if (duel_w_089149D8 != 0) {
                duel_w_089148EC = 1;
            } else {
                duel_w_089148EC++;
            }
        }
        return 1;
    case 0xD:
        duel_ai_step = 0;
        duel_w_089149D8 = 0;
        duel_w_089148EC++;
    case 0xE:
        if (duel_0884FD04(last, last->player) != 0) {
            if (duel_w_089149D8 != 0) {
                duel_w_089148EC = 1;
            } else {
                duel_w_089148EC++;
            }
        }
        return 1;
    default:
    dflt:
        duel_w_089148E8 = 0;
        duel_w_089148F0 = 1;
        duel_w_089148F4 = 0;
        duel_w_089147D8 = duel_chain.unk480;
        duel_w_08914924 = 0;
        duel_w_08914920 = 0;
        duel_w_089149D4 = 0;
        return 1;
    }
}
#endif
