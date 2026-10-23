// @range 0x0882AFA4-0x0882CAC0
// @data 0x0890794C-0x08907968
/* Summoning a monster (duel_summon): the step machines that pick the tributes and pay the costs of
   each kind of summon. The player (or the computer) picks a zone with duel_08835978 /
   duel_0881956C and a zone filter (ai_0882A23C.c); duel_08835AC0 tells whether a zone was picked
   (duel_w_0891964C player, duel_w_08919650 + duel_w_08919654 zone). Return: 1 done, 0 continue,
   -1 cancelled. */
#include "duel.h"

extern int duel_player_type[2];
extern int duel_w_08919624;            /* answer of the last question */
extern int duel_w_0891964C;            /* picked player */
extern int duel_w_08919650;            /* picked zone */
extern int duel_w_08919654;

int duel_08835AC0(void);
int duel_host_call(int a, int b, unsigned int c, int d);
int duel_0884AD18(int msg);
void duel_wfmt_d(unsigned short *dst, int fmt, int n);
int duel_list_card_level(unsigned int p, int i);
int duel_double_tribute(unsigned int p, int id, unsigned int q, int z);
void duel_0881956C(unsigned int p, int id, int (*fn)());
void duel_08835978(int (*fn)());
int duel_0882926C(unsigned int p, int z, int i);
int duel_0882AAA0(unsigned int p, int z, int i);
int duel_08827DA4(unsigned int p, int uid, int a, int b);
void duel_0884AD40(int msg, int uid);
int duel_0884ACD8(unsigned int p, int msg);
int duel_list_find_id(unsigned int p, unsigned int id);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
void duel_088C4448(unsigned int p, int z, unsigned int uid);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
void duel_088C448C(int p, int a);
void duel_088D1AC8(unsigned int p, int z);
int card_is_in_group_19bb(int id);
int duel_count_free_monster_zones(unsigned int p);
unsigned short *card_get_name(int id);
void duel_wfmt_s();
int duel_summon_tributes(void);
int duel_0882A680(unsigned int p, int z, int i);
int duel_0882AA10(unsigned int p, int z, int i);
int card_get_type(int id);
void duel_088C4564(unsigned int p, int n);
void duel_088CABA8(int p, unsigned int i);
extern int duel_w_0891962C;            /* picked card (uid) */
int duel_088297A0(unsigned int p, int z, int i);
int duel_08829D74(unsigned int p, int z, int i);
int duel_08829EB8(unsigned int p, int z, int i);
int duel_0882A018(unsigned int p, int z, int i);
int duel_0882A23C(unsigned int p, int z, int i);
int duel_0882ABB4(unsigned int p, int z, int i);
int duel_08826710(int id);
int duel_hand_find_name(unsigned int p, int id);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
void duel_088D1BEC(unsigned int p, int z, int a, int b);
void duel_088CAE24(unsigned int p, DuelCard *c, int a);
int duel_088D1608(int p, int z, int flags);
unsigned int duel_card_locate(unsigned int uid);
void duel_088CABF4(unsigned int p, int i, int a);
void duel_088D1B20(unsigned int p, unsigned int mask);

/* Picks the one, two or three tributes of a normal summon by the level of the card. */
int duel_summon_tributes(void)
{
    unsigned short buf3[256];
    unsigned short buf2[256];
    unsigned int p;
    int z;

    switch (duel_summon.step) {
    case 0:
        switch (duel_list_card_level(duel_summon.player, duel_summon.uid)) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            duel_summon.n = 0;
            return 1;
        case 5:
        case 6:
            if (duel_summon.f4 == 1) {
                duel_summon.n = 0;
                return 1;
            }
            duel_summon.step = 0xA;
            return 0;
        default:
            if (duel_summon.f4 == 2) {
                duel_summon.n = 0;
                return 1;
            }
            duel_summon.step = 0x14;
            return 0;
        }
    case 0x1E:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_w_08919624 = 1;
        } else {
            duel_wfmt_d(buf3, 2, 3);
            duel_0884AD18((int)buf3);
        }
        duel_summon.n = 0;
        duel_summon.step++;
        return 0;
    case 0x1F:
        if (duel_w_08919624 == 0) {
            return -1;
        }
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, 1, duel_0882926C);
        } else {
            duel_08835978(duel_0882926C);
        }
        duel_summon.step++;
        return 0;
    case 0x20:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
            duel_summon.locs[duel_summon.n] = (unsigned char)p | (unsigned char)z << 8;
            if (++duel_summon.n < 3) {
                if (duel_double_tribute(duel_summon.player, duel_summon.id, p, z) != 0) {
                    if (duel_player_type[duel_summon.player] == 1) {
                        duel_w_08919624 = 1;
                    } else {
                        duel_0884AD18(0x124);
                    }
                    duel_summon.step++;
                } else {
                    duel_summon.step--;
                }
                return 0;
            }
            return 1;
        }
        duel_host_call(0x29, 0, 0, 0);
        duel_summon.step = 0x1E;
        return 0;
    case 0x21:
        if (duel_w_08919624 != 0) {
            duel_summon.locs[duel_summon.n] = duel_summon.locs[duel_summon.n - 1];
            if (++duel_summon.n >= 3) {
                return 1;
            }
        }
        duel_w_08919624 = 1;
        duel_summon.step -= 2;
        return 0;
    case 0x14:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_w_08919624 = 1;
        } else {
            duel_wfmt_d(buf2, 2, 2);
            duel_0884AD18((int)buf2);
        }
        duel_summon.n = 0;
        duel_summon.step++;
        return 0;
    case 0x15:
        if (duel_w_08919624 == 0) {
            return -1;
        }
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, 1, duel_0882926C);
        } else {
            duel_08835978(duel_0882926C);
        }
        duel_summon.step++;
        return 0;
    case 0x16:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
            duel_summon.locs[duel_summon.n] = (unsigned char)p | (unsigned char)z << 8;
            if (++duel_summon.n < 2) {
                if (duel_double_tribute(duel_summon.player, duel_summon.id, p, z) != 0) {
                    if (duel_player_type[duel_summon.player] == 1) {
                        duel_w_08919624 = 1;
                    } else {
                        duel_0884AD18(0x124);
                    }
                    duel_summon.step++;
                } else {
                    duel_summon.step--;
                }
                return 0;
            }
            return 1;
        }
        duel_host_call(0x29, 0, 0, 0);
        duel_summon.step = 0x14;
        return 0;
    case 0x17:
        if (duel_w_08919624 != 0) {
            duel_summon.locs[duel_summon.n] = duel_summon.locs[duel_summon.n - 1];
            if (++duel_summon.n >= 2) {
                return 1;
            }
        }
        duel_w_08919624 = 1;
        duel_summon.step -= 2;
        return 0;
    case 0xA:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_w_08919624 = 1;
        } else {
            duel_0884AD18(1);
        }
        duel_summon.step++;
        return 0;
    case 0xB:
        if (duel_w_08919624 == 0) {
            return -1;
        }
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, 1, duel_0882926C);
        } else {
            duel_08835978(duel_0882926C);
        }
        duel_summon.step++;
        return 0;
    case 0xC:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
            duel_summon.n = 1;
            duel_summon.locs[0] = (unsigned char)p | (unsigned char)z << 8;
            return 1;
        }
        duel_host_call(0x29, 0, 0, 0);
        duel_summon.step = 0xA;
        return 0;
    }
    return -1;
}

/* Normal summon that may use 0x197F from the graveyard instead of the tributes. */
int duel_0882B8D8(void)
{
    int mask;
    int i;
    int uid;

    switch (duel_summon.step) {
    case 0:
        mask = duel_summon.kind ? 0x40 : 0x10;
        if ((int)(mask & duel_08827DA4(duel_summon.player, duel_summon.uid, 2, duel_summon.f4)) > 0) {
            if ((int)(mask & duel_08827DA4(duel_summon.player, duel_summon.uid, 0, duel_summon.f4)) > 0) {
                if (duel_player_type[duel_summon.player] == 1) {
                    duel_w_08919624 = 1;
                } else {
                    duel_0884AD40(0x72, duel_summon.uid);
                }
            } else {
                duel_w_08919624 = 1;
            }
        } else {
            duel_w_08919624 = 0;
        }
        duel_summon.step++;
        return 0;
    case 1:
        if (duel_w_08919624 != 0) {
            duel_summon.n = 0;
            duel_summon.res_id = 0x197F;
            uid = 0;
            for (i = duel.players[duel_summon.player & 1].n_grave - 1; i >= 0; i--) {
                if (duel.players[duel_summon.player & 1].grave[i].id == 0x197F
                    && duel_slot_effect_with(duel_summon.player, 0xB, 0x197F, CARD_UID(duel.players[duel_summon.player & 1].grave[i])) == 0) {
                    uid = CARD_UID(duel.players[duel_summon.player & 1].grave[i]);
                    break;
                }
            }
            duel_088C4448(duel_summon.player, 0xE, uid);
            duel_088C77D4(duel_summon.player, 0x197F, 5, uid);
            return 1;
        }
        duel_summon.step = 0;
        break;
    }
    return duel_summon_tributes();
}

/* Summon paid with the counters of 0x19B2 (or by 0x19AD for the 0x19BB group). */
int duel_0882BBB4(void)
{
    unsigned short buf[256];
    int mask;
    unsigned int p;
    int z;

    switch (duel_summon.step) {
    case 0:
        mask = duel_summon.kind ? 0x40 : 0x10;
        if ((int)(mask & duel_08827DA4(duel_summon.player, duel_summon.uid, 2, duel_summon.f4)) > 0) {
            if ((int)(mask & duel_08827DA4(duel_summon.player, duel_summon.uid, 0, duel_summon.f4)) > 0) {
                if (duel_player_type[duel_summon.player] == 1) {
                    duel_w_08919624 = 1;
                } else {
                    duel_0884AD40(0xFA, duel_list_find_id(duel_summon.player, 0x19B2));
                }
            } else {
                duel_w_08919624 = 1;
            }
        } else {
            duel_w_08919624 = 0;
        }
        duel_summon.step++;
        return 0;
    case 1:
        if (duel_w_08919624 == 0) {
            if (duel_summon.kind == 0 && card_is_in_group_19bb(duel_summon.id)
                && duel_slot_effect_with(duel_summon.player, 0xB, 0x19AD, duel_summon.uid)
                && duel_count_free_monster_zones(duel_summon.player)) {
                duel_summon.n = 0;
                return 1;
            }
            duel_summon.step = 0;
            break;
        } else {
            duel_wfmt_s(buf, 0x13, card_get_name(0x19B2));
            duel_0884ACD8(duel_summon.player, (int)buf);
            if (duel_player_type[duel_summon.player] == 1) {
                duel_0881956C(duel_summon.player, 1, duel_0882AAA0);
            } else {
                duel_08835978(duel_0882AAA0);
            }
            duel_summon.step++;
            return 0;
        }
    case 2:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_088C4448(p, z, CARD_UID(duel.players[p & 1].field[z].card));
            duel_088D1AC8(p, z);
            duel_summon.step++;
            return 0;
        }
        return -1;
    case 3:
        duel_summon.n = 0;
        duel_summon.res_id = 0x19B2;
        return 1;
    }
    return duel_summon_tributes();
}

/* Summon that may be paid by 0x1AA3 instead of the tributes. */
int duel_0882BF2C(void)
{
    int mask;

    switch (duel_summon.step) {
    case 0:
        mask = duel_summon.kind ? 0x40 : 0x10;
        if ((int)(mask & duel_08827DA4(duel_summon.player, duel_summon.uid, 2, duel_summon.f4)) > 0) {
            if ((int)(mask & duel_08827DA4(duel_summon.player, duel_summon.uid, 0, duel_summon.f4)) > 0) {
                if (duel_player_type[duel_summon.player] == 1) {
                    duel_w_08919624 = 1;
                } else {
                    duel_0884AD40(0x72, duel_summon.uid);
                }
            } else {
                duel_w_08919624 = 1;
            }
            duel_summon.step++;
            return 0;
        }
        break;
    case 1:
        if (duel_w_08919624 != 0) {
            duel_summon.n = 0;
            duel_summon.res_id = 0x1AA3;
            duel_088C448C(duel_summon.player, 0x1AA3);
            return 1;
        }
        duel_summon.step = 0;
        break;
    }
    return duel_summon_tributes();
}

/* Summon of a fusion-like card: the player picks one monster (0x10BC) to send. */
int duel_0882C0C0(void)
{
    unsigned short msg[256];
    unsigned short tmp[256];
    unsigned int p;
    int z;

    switch (duel_summon.step) {
    case 0:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, duel_summon.id, duel_088297A0);
        } else {
            duel_wfmt_s(tmp, 0xD1, card_get_name(duel_summon.id));
            duel_wfmt_s(msg, tmp, card_get_name(0x10BC));
            duel_0884ACD8(duel_summon.player, (int)msg);
            duel_08835978(duel_088297A0);
        }
        duel_summon.step++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
            duel_summon.n = 1;
            duel_summon.locs[0] = (unsigned char)p | (unsigned char)z << 8;
            return 1;
        }
        return -1;
    }
    return 0;
}

/* Summon by an effect (f1): one tribute picked with duel_08829D74. */
int duel_0882C270(void)
{
    unsigned int p;
    int z;

    if (!duel_summon.f1) {
        return duel_summon_tributes();
    }
    {
        switch (duel_summon.step) {
        case 0:
            if (duel_player_type[duel_summon.player] == 1) {
                duel_0881956C(duel_summon.player, duel_summon.id, duel_08829D74);
            } else {
                duel_0884ACD8(duel_summon.player, 0x12);
                duel_08835978(duel_08829D74);
            }
            duel_summon.step++;
            return 0;
        case 1:
            if (duel_08835AC0() != 0) {
                p = duel_w_0891964C;
                z = duel_w_08919650 + duel_w_08919654;
                duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
                duel_summon.n = 1;
                duel_summon.locs[0] = (unsigned char)p | (unsigned char)z << 8;
                return 1;
            }
            return -1;
        }
        return 0;
    }
}

/* Summon with as many tributes as duel_08826710 asks (0x1A65 picks them with duel_0882ABB4). */
int duel_0882C400(void)
{
    unsigned int p;
    int z;
    unsigned int id = duel_summon.id;
    int (*fn)() = duel_08829D74;

    switch (id) {
    case 0x1A65:
        fn = duel_0882ABB4;
        break;
    }
    switch (duel_summon.step) {
    case 0:
        duel_summon.n = 0;
        duel_0884ACD8(duel_summon.player, 0x12);
        duel_summon.step++;
        return 0;
    case 1:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, id, fn);
        } else {
            duel_08835978(fn);
        }
        duel_summon.step++;
        return 0;
    case 2:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
            duel_summon.locs[duel_summon.n] = (unsigned char)p | (unsigned char)z << 8;
            if (++duel_summon.n < duel_08826710(duel_summon.id)) {
                duel_summon.step--;
                return 0;
            }
            return 1;
        }
        duel_host_call(0x29, 0, 0, 0);
        return -1;
    }
    return 0;
}

/* Summon paid by discarding 0x15A3 from the hand. */
int duel_0882C62C(void)
{
    unsigned int p;
    int z;
    int i;

    switch (duel_summon.step) {
    case 0:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, duel_summon.id, duel_08829EB8);
        } else {
            duel_0884ACD8(duel_summon.player, 0x3A);
            duel_08835978(duel_08829EB8);
        }
        duel_summon.step++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            i = duel_hand_find_name(duel_summon.player, 0x15A3);
            duel_088C7BA4();
            duel_088D1BEC(p, z, 0, 0);
            duel_088CAE24(duel_summon.player, &duel.players[duel_summon.player & 1].hand[i], 0);
            duel_088C7CF0();
            duel_summon.step++;
            return 0;
        }
        return -1;
    case 2:
        return 1;
    }
    return 0;
}

/* Summon that destroys a picked monster (duel_08829EB8) first. */
int duel_0882C7B8(void)
{
    switch (duel_summon.step) {
    case 0:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, duel_summon.id, duel_08829EB8);
        } else {
            duel_0884ACD8(duel_summon.player, 0x7E);
            duel_08835978(duel_08829EB8);
        }
        duel_summon.step++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            duel_088D1608(duel_w_0891964C, duel_w_08919650 + duel_w_08919654, 0);
            duel_summon.step++;
            return 0;
        }
        return -1;
    case 2:
        return 1;
    }
    return 0;
}

/* Summon with three tributes picked with duel_0882A018 (one message per tribute). */
int duel_0882C8C0(void)
{
    unsigned short msg[256];
    unsigned int p;
    int z;

    switch (duel_summon.step) {
    case 0:
        if (duel_player_type[duel_summon.player] == 1) {
            duel_0881956C(duel_summon.player, duel_summon.id, duel_0882A018);
        } else {
            duel_wfmt_s(msg, 0xD2, card_get_name(duel_summon.n + 0x1119));
            duel_0884ACD8(duel_summon.player, (int)msg);
            duel_08835978(duel_0882A018);
        }
        duel_summon.step++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_host_call(0x28, p, z, CARD_UID(duel.players[p & 1].field[z].card));
            duel_summon.locs[duel_summon.n] = (unsigned char)p | (unsigned char)z << 8;
            if (++duel_summon.n < 3) {
                duel_summon.step--;
                return 0;
            }
            return 1;
        }
        return -1;
    }
    return 0;
}

/* Summon with three tributes (starts at step 0x1E of duel_summon_tributes). */
int duel_0882CA98(void)
{
    switch (duel_summon.step) {
    case 0:
        duel_summon.step = 0x1E;
        break;
    }
    return duel_summon_tributes();
}
