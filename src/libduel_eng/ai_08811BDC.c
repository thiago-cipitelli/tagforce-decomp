// @range 0x08811BDC-0x088137BC
// @asm 0x08811BDC-0x08812DBC
// @asm 0x08812DBC-0x088137BC
/* Conditions of the computer player: battle cards (quick effects activated during an attack). */
#include "duel.h"

int duel_field_atk(unsigned int p, int z);
int duel_field_def(unsigned int p, int z);
int duel_field_base_def(unsigned int p, int z);
int duel_field_level(unsigned int p, int z);
int duel_08809F2C(unsigned int p);
int field_get_card(unsigned int p, int z);
int duel_088BFFD8(unsigned int p, int id, int i);
int duel_count_monsters_pos(unsigned int p, int faceup, int pos0);
int duel_count_monsters_pos_is(unsigned int p, unsigned int pos);
int duel_0880A17C(unsigned int p, unsigned int q, int a, int b, int c);
int duel_0880A0B4(unsigned int p);
int duel_hand_count_level(unsigned int p, int lvl);
int duel_count_monsters(unsigned int p);
int duel_count_name(unsigned int p, int id);
int card_get_special_group(int id);
int card_get_level(int id);
int card_get_atk(int id);
int card_attribute_icon_is3(int id);
int card_attribute_icon_in_2_3(int id);
int card_is_in_group_1a6f(int id);
int card_is_in_group_1a6x(int id);
int duel_count_monsters_fn(unsigned int p, int (*fn)(int id));
void duel_08809B50(unsigned int p, unsigned short id, unsigned short a);
DuelCard *duel_0884C7CC(void);
int duel_088054A8(unsigned int p, int skip, int mode);
int duel_08807AB4(unsigned int p);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_0881A038(DuelAct *a, DuelAct *b);
int duel_count_field_all(unsigned int p);
int duel_088154D8(unsigned int p);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
int duel_count_type(unsigned int p, unsigned int type);
int duel_088070A8(int id);
int duel_grave_count_monsters(unsigned int p);
int duel_can_attack(unsigned int p, int z, int arg);

#ifdef NON_MATCHING /* switch de 60 casos com a forma de cada caso próxima; diferem desvios (beql/bnel) e registradores */
/* a: card the computer could activate during the battle (kinds 18/19), b: the card being resolved (may be null) */
int duel_08811BDC(DuelAct *a, DuelAct *b)
{
    unsigned int ap;                   /* attacker */
    int az;
    unsigned int pl;                   /* player of a */
    unsigned int tp;                   /* attacked player */
    int tz;
    int direct;
    int atk;                           /* attacker ATK */
    int tv;                            /* target ATK or DEF (0 for a direct attack) */
    unsigned int q;
    int id;
    int i;
    unsigned int n;

    if ((unsigned int)(a->kind - 18) < 2) {
        ap = duel_special.player[0];
        pl = a->player;
        az = duel_special.zone[0];
        tp = duel_special.player[1];
        tz = duel_special.zone[1];
        direct = tz >= 5;
        atk = duel_field_atk(ap, az);
        if (direct) {
            tv = 0;
        } else if (duel.players[tp & 1].field[tz].pos != 0) {
            tv = duel_field_def(tp, tz);
        } else {
            tv = duel_field_atk(tp, tz);
        }
        switch (a->id) {
        case 0x1254:
        case 0x12E2:
        case 0x14F5:
        case 0x153F:
        case 0x1402:
        case 0x143B:
        case 0x16E2:
        case 0x17FE:
        case 0x15F5: /* the opponent attacks */
            return ap == 1 - a->player;
        case 0x17B5:
            return direct && ap == 1 - a->player;
        case 0x1575:
            return duel_count_field_all(1 - a->player) >= duel_count_field_all(a->player);
        case 0x171A:
            if (ap != 1 - a->player) {
                return 0;
            }
            id = duel.players[ap & 1].field[az].card.id;
            if (card_get_level(id) >= 5) {
                return 1;
            }
            if (card_attribute_icon_in_2_3(id) != 0) {
                return 1;
            }
            if (direct && atk >= duel.players[pl & 1].lp) {
                return 1;
            }
            return 0;
        case 0x140A: /* another face-up ATK monster stronger than the attacker */
            if (ap != 1 - pl) {
                return 0;
            }
            if (!direct && atk >= tv) {
                for (i = 0; i < 5; i++) {
                    if (i != tz && duel.players[pl & 1].field[i].card.id != 0 && duel.players[pl & 1].field[i].pos == 0 &&
                        atk < duel_field_atk(pl, i)) {
                        return 1;
                    }
                }
            }
            return 0;
        case 0x1420:
            if (ap != 1 - pl) {
                return 0;
            }
            if (direct || atk < tv) {
                return 0;
            }
            if (atk >= duel.players[pl & 1].lp) {
                return 0;
            }
            if (duel.players[tp & 1].field[tz].faceup == 0) {
                return 0;
            }
            return card_get_special_group(duel.players[tp & 1].field[tz].card.id) != 0;
        case 0x146B: /* another monster of the attacker weaker than the target */
            q = 1 - pl;
            if (ap != q) {
                return 0;
            }
            if (!direct && atk >= tv) {
                for (i = 0; i < 5; i++) {
                    if (i != az && duel.players[q & 1].field[i].card.id != 0 && duel.players[q & 1].field[i].faceup != 0 &&
                        duel_field_atk(q, i) < tv) {
                        return 1;
                    }
                }
            }
            return 0;
        case 0x1316:
        case 0x1317:
        case 0x13F9:
        case 0x1381:
        case 0x1805:
        case 0x1AAC:
        case 0x1B2C:
            q = 1 - pl;
            if (ap != q) {
                return 0;
            }
            if (direct) {
                if (atk >= duel.players[pl & 1].lp) {
                    return 1;
                }
                if (atk >= 1500) {
                    return 1;
                }
                duel_count_monsters_pos(q, 1, 1);
                return 1;
            }
            if (atk >= tv) {
                return 1;
            }
            if (duel_088154D8(pl) != 0) {
                return 1;
            }
            return 0;
        case 0x19EC:
            if (duel_0881A038(a, b) == 0) {
                return 0;
            }
            if (ap == 1 - pl && direct) {
                if (atk >= duel.players[pl & 1].lp) {
                    return 1;
                }
                if (atk >= 2000) {
                    return 1;
                }
            }
        case 0x1907:
            q = 1 - pl;
            if (ap != q) {
                return 0;
            }
            return duel_count_monsters_pos(q, 0, 1) >= 2;
        case 0x13FE:
            if (ap != 1 - pl) {
                return 0;
            }
            if (card_attribute_icon_is3(duel.players[ap & 1].field[az].card.id) != 0) {
                return 1;
            }
            return 0;
        case 0x195F:
            if (duel_count_monsters_fn(pl, card_is_in_group_1a6f) == 0) {
                return 0;
            }
        case 0x12C4:
            q = 1 - pl;
            if (ap != q) {
                return 0;
            }
            if (direct) {
                return 1;
            }
            if (atk >= tv) {
                return 1;
            }
            if (b != 0 && b->player == q) {
                return 1;
            }
            return 0;
        case 0x1404:
            q = 1 - pl;
            if (ap != q) {
                return 0;
            }
            if (atk >= duel.players[q & 1].lp) {
                return 1;
            }
            if (direct && atk >= duel.players[pl & 1].lp) {
                return 1;
            }
            if (atk < duel_088054A8(q, az, 1)) {
                return 0;
            }
            if (atk >= 1500) {
                return 1;
            }
            if (!direct && atk >= tv) {
                return 1;
            }
            return 0;
        case 0x171E:
            if (ap != 1 - pl) {
                return 0;
            }
            if (direct) {
                return 1;
            }
            if (tv < atk && duel.players[tp & 1].field[tz].pos == 0) {
                return 1;
            }
            return 0;
        case 0x1930: /* the battle damage would be lethal or large */
            if (ap != 1 - pl) {
                return 0;
            }
            if (!direct) {
                if (atk - tv >= duel.players[(1 - pl) & 1].lp) {
                    return 1;
                }
                if (atk - tv >= 1500) {
                    return 1;
                }
            }
            return 0;
        case 0x1380:
        case 0x176A:
            if (duel_08807AB4(pl) == 0) {
                return 0;
            }
        case 0x16A7:
        case 0x14B2:
        case 0x1243:
        case 0x1103:
            q = 1 - pl;
            if (ap != q) {
                return 0;
            }
            if (direct && atk >= duel.players[pl & 1].lp) {
                return 1;
            }
            if (atk >= duel_088054A8(q, az, 1)) {
                return 1;
            }
            return 0;
        case 0x140E:
            if (ap != 1 - pl) {
                return 0;
            }
            return duel_field_level(ap, az) >= 4;
        case 0x1766:
            if (ap != 1 - pl) {
                return 0;
            }
            return duel.players[pl & 1].lp > 3000 && duel_088054A8(1 - pl, -1, 0) <= 3000;
        case 0x1538:
            if (ap != 1 - pl) {
                return 0;
            }
            if (direct && atk >= duel.players[pl & 1].lp) {
                return 1;
            }
            return 0;
        case 0x137D:
            if (ap != 1 - pl) {
                return 0;
            }
            if (direct && duel_count_monsters(a->player) == 0) {
                if (atk >= duel.players[a->player & 1].lp) {
                    return 1;
                }
                duel_08809B50(pl, a->id, 0);
                if (atk <= card_get_atk(duel_0884C7CC()->id)) {
                    return 1;
                }
            }
            return 0;
        case 0x12D2:
            if (duel_find_effect_card(1 - pl, 0x1398, -1) != 0) {
                return 0;
            }
        case 0x17B7:
        case 0x16A4:
        case 0x1636:
            if (!(ap == 1 - pl && direct)) {
                return 0;
            }
            return !duel_count_monsters(a->player);
        case 0x1362:
            if (ap != 1 - pl) {
                return 0;
            }
            if (!direct) {
                id = duel.players[tp & 1].field[tz].card.id;
                if (duel_088BFFD8(pl, a->id, 0) < 2) {
                    return 0;
                }
                if (card_get_special_group(id) != 0) {
                    return duel.players[tp & 1].field[tz].faceup;
                }
                if (atk >= tv) {
                    if (tv >= 1800) {
                        return 1;
                    }
                    if (duel_count_name(pl, 0x12F3) != 0) {
                        n = duel.players[a->player & 1].n_hand;
                        for (i = 0; i < n; i++) {
                            switch (duel.players[a->player & 1].hand[i].id) {
                            case 0x1386:
                            case 0x1387:
                            case 0x1388:
                            case 0x1905:
                            case 0x13B5:
                            case 0x154B:
                                return 1;
                            }
                        }
                    }
                }
            }
            return 0;
        case 0x1231:
            if (ap != 1 - pl) {
                return 0;
            }
            if (!direct && atk >= tv && duel_count_monsters_pos(pl, 1, 0) != 0) {
                return 1;
            }
            return 0;
        case 0x1561:
        case 0x1852:
            if (ap != 1 - pl) {
                return 0;
            }
            if (!direct) {
                if (atk >= duel.players[a->player & 1].lp) {
                    return 0;
                }
                if (tv >= atk) {
                    return 0;
                }
                return duel_hand_count_level(a->player, 5) != 0;
            }
            return 0;
        case 0x1669:
            q = 1 - pl;
            if (duel_count_monsters_pos(q, 1, 1) < 2) {
                return 0;
            }
            return duel_0880A17C(pl, q, -1, 1, 0) < duel_0880A0B4(pl);
        case 0x1980:
            if (!direct && ap == 1 - pl) {
                if (duel_0884E3E4(a, tp, tv) != 0) {
                    return atk >= 1900;
                }
                return 0;
            }
            return 0;
        case 0x17B3:
            q = 1 - pl;
            if (ap != q) {
                return 0;
            }
            if (atk < tv) {
                return 0;
            }
            if (duel_count_monsters_pos(q, 1, 1) >= 2) {
                return 1;
            }
            return 0;
        case 0x19B5:
            if (!direct && ap == 1 - pl) {
                if (field_get_card(tp, tz) != 0x18F6) {
                    return 0;
                }
                if (duel_088BFFD8(pl, a->id, 0) == 0) {
                    return 0;
                }
                return atk <= 2800 && atk >= tv;
            }
            return 0;
        case 0x1A0E:
            if (ap == 1 - pl) {
                if (!(duel.players[(1 - pl) & 1].flags >> 17 & 1)) {
                    return 0;
                }
                if (duel_08809F2C(pl) < 0) {
                    return 1;
                }
            }
            return 0;
        case 0x14A6:
            if (ap != 1 - pl) {
                return 0;
            }
            if (tv < atk - 500) {
                return 0;
            }
            return 1;
        }
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* forma próxima; diferem os limites a 0 (bgez sem likely no original) e um bloco morto no caso 0x14BE */
/* a: card that changes the ATK/DEF in the battle (kind 20): would it change the outcome? */
int duel_08812DBC(DuelAct *a)
{
    unsigned int ap;                   /* attacker */
    int az;
    unsigned int pl;                   /* player of a */
    unsigned int tp;                   /* attacked player */
    int tz;
    int direct;
    int up;                            /* ATK change */
    int dup;                           /* DEF change */
    int i;
    int n;
    int v;
    int pos;
    unsigned int q;
    int av;                            /* ATK of the opponent's monster */
    int dv;                            /* ATK or DEF of the player's monster */
    int t;
    int u;

    if (a->kind == 20) {
        ap = duel_special.player[0];
        up = 0;
        pl = a->player;
        az = duel_special.zone[0];
        tp = duel_special.player[1];
        tz = duel_special.zone[1];
        direct = tz >= 5;
        dup = 0;
        switch (a->id) {
        case 0x12CF:
            dup = up = 400;
            break;
        case 0x12D0:
            dup = up = -400;
            break;
        case 0x12F1:
            up = 500;
            break;
        case 0x1329:
            up = 700;
            break;
        case 0x13EE:
            up = -700;
            break;
        case 0x12F2:
            dup = 500;
            break;
        case 0x132A:
            dup = 700;
            break;
        case 0x1330:
            dup = -500;
            break;
        case 0x1492:
            up = duel_grave_count_monsters(pl) * 100;
            break;
        case 0x14EB: /* minus half the ATK of the target */
            up = -card_get_atk(duel.players[tp & 1].field[tz].card.id) / 2;
            break;
        case 0x14BE:
            up = -duel.players[pl & 1].lp;
            if (duel_088070A8(duel.players[tp & 1].field[tz].card.id) != 0) {
                return 0;
            }
            if (duel.players[tp & 1].field[tz].pos != 0 && up < -799) {
                up = -800;
            }
            break;
        case 0x1882:
            up = duel_count_type(pl, 0xB) * 200;
            break;
        case 0x1514:
            up = 500;
            break;
        case 0x192D:
            up = 1500;
            break;
        case 0x16E3: /* 200 per card in the opponent's hand */
            up = dup = duel.players[(1 - pl) & 1].n_hand * 200;
            break;
        case 0x153D:
            up = 200;
            dup = 500;
            break;
        case 0x1763:
            dup = -4000;
            break;
        case 0x1409:
            if (ap == tp) {
                break;
            }
            if (ap == pl) {
                n = 0;
                if (duel.players[ap & 1].field[az].pos != 0) {
                    break;
                }
                if (duel_0884E3E4(a, ap, az) == 0) {
                    break;
                }
                for (i = 0; i <= 4; i++) {
                    if (duel_0884E3E4(a, pl, i) != 0 && duel.players[pl & 1].field[i].pos == 0) {
                        n += (duel.players[pl & 1].field[i].card.id == 0x18FC) + 1;
                    }
                }
                if (duel_count_monsters_pos_is(1 - pl, 1) + 1 < n) {
                    return 1;
                }
            } else {
                if (duel.players[tp & 1].field[tz].pos != 0) {
                    break;
                }
                if (duel_0884E3E4(a, ap, az) == 0) {
                    break;
                }
                if (duel_field_atk(tp, tz) * 2 - duel_field_atk(ap, az) >= duel.players[(1 - pl) & 1].lp) {
                    return 1;
                }
            }
            break;
        case 0x167B: /* DEF of the other monsters */
            for (i = 0; i <= 4; i++) {
                if (i != tz && duel.players[pl & 1].field[i].faceup != 0) {
                    dup += duel_field_base_def(pl, i);
                }
            }
            break;
        case 0x1853: /* ATK of the strongest monster it can target */
            for (i = 0; i <= 4; i++) {
                if (duel_0884E3E4(a, pl, i) != 0) {
                    v = duel_field_atk(pl, i);
                    up = MAX(v, up);
                }
            }
            break;
        case 0x1A60:
            if (pl == tp && card_is_in_group_1a6x(field_get_card(tp, tz)) != 0) {
                up = 800;
            }
            if (pl != tp && card_is_in_group_1a6x(field_get_card(ap, az)) != 0) {
                up = 800;
            }
            break;
        }
        if (direct) {
            n = 0;
            for (i = 0; i <= 4; i++) {
                n += duel_can_attack(ap, i, 1);
            }
            if (n > 1) {
                switch (a->id) {
                case 0x12CF:
                    return a->player == ap;
                case 0x12D0:
                    return a->player == tp;
                }
            }
            if (pl != ap) {
                return 0;
            }
            if (up != 0) {
                if (up + duel_field_atk(ap, az) >= duel.players[(1 - pl) & 1].lp) {
                    return 1;
                }
            }
        } else if (ap != tp && (up | dup) != 0) {
            if (pl == tp) {
                /* the player defends: does the change win the battle? */
                pos = duel.players[pl & 1].field[tz].pos;
                q = 1 - pl;
                av = duel_field_atk(q, az);
                dv = pos ? duel_field_def(pl, tz) : duel_field_atk(pl, tz);
                if (pos) {
                    if (dup < 0) {
                        dup = 0;
                    }
                    t = -up;
                    if (t < 0) {
                        t = 0;
                    }
                    t += dup;
                } else {
                    u = up >= 0 ? up : 0;
                    t = -up;
                    if (t < 0) {
                        t = 0;
                    }
                    t += u;
                }
                t += dv;
                if (t < av) {
                    return 0;
                }
                if (dv < av) {
                    return 1;
                }
                if (field_get_card(pl, tz) == 0x1292) {
                    return 1;
                }
                if (t - av >= duel.players[q & 1].lp) {
                    return 1;
                }
            } else {
                /* the player attacks */
                q = 1 - pl;
                pos = duel.players[q & 1].field[tz].pos;
                av = duel_field_atk(pl, az);
                dv = pos ? duel_field_def(q, tz) : duel_field_atk(q, tz);
                if (pos) {
                    t = -dup;
                } else {
                    t = -up;
                }
                if (t < 0) {
                    t = 0;
                }
                if (up < 0) {
                    up = 0;
                }
                t += up;
                if (dv >= av && dv < av + t) {
                    return 1;
                }
                if (pos == 0 && av - dv < duel.players[q & 1].lp && av + t - dv >= duel.players[q & 1].lp) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
#endif
