// @range 0x08827DA4-0x0882926C
// @asm 0x08827DA4-0x08828740
// @asm 0x08828740-0x0882926C
/* Summon options of a card of the hand (0x10 normal summon, 0x40 set): the checks of the special
   summon conditions per card, then the tributes by level. Sets duel_err for the viewing player. */
#include "duel.h"

extern unsigned int duel_view_player;
extern int duel_err;                   /* 0x0891965C reason of the last refused action */

int card_is_monster(int id);
int duel_card_protected(int id, int uid);
int duel_can_pay(unsigned int p, int id);
int card_is_group_b(int id);
int duel_count_active(int id);
int duel_can_special_summon(unsigned int p);
int duel_can_activate(unsigned int p);
int duel_08827A90(unsigned int p, int id);
int duel_08827CEC(unsigned int p, int b);
int duel_count_tributes(unsigned int p, int id, int flag);
int duel_count_free_monster_zones(unsigned int p);
int duel_count_monsters_fn(unsigned int p, int (*fn)(int id));
int card_is_in_group_1911(int id);
int card_id_remap_164a(int id);
int duel_card_check_9148(unsigned int p, int id);
int duel_count_monsters(unsigned int p);
int duel_08827934(unsigned int p, int id);
int card_is_in_group_1a6f(int id);
int card_is_in_group_19bb(int id);
int card_get_type(int id);
int duel_08827B80(unsigned int p, int i);
int duel_08827BF0(unsigned int p, int i);
int duel_08827C80(unsigned int p, int i);
int duel_slot_effect_with(unsigned int p, int z, int id, unsigned int src);
int duel_list_card_level(unsigned int p, int i);
int duel_phase_ok2(unsigned int p);
int duel_hand_find_uid(unsigned int p, int uid);
int duel_has_st_12be(unsigned int p);
int duel_count_tributable_except(unsigned int p, int src);
int duel_088272E8(unsigned int p, int id);
int duel_08826AE0(unsigned int p, int id);
int duel_088268A8(unsigned int p, int id);
int duel_08826740(unsigned int p, int id);
int duel_0882662C(unsigned int p, int id);
int duel_08826BC4(unsigned int p, int id);
int duel_0882696C(unsigned int p, int id);
int duel_08827118(unsigned int p);
int duel_08826F74(unsigned int p);
int duel_088277BC(unsigned int p, int id);
int duel_08827618(unsigned int p, unsigned short id);
int duel_08826DB8(unsigned int p);
int duel_08826C0C(unsigned int p);
int duel_d_find_uid(unsigned int p, int uid);
int duel_08826548(unsigned int p, int id);
int duel_088271EC(unsigned int p);
int duel_count_faceup_name(unsigned int p, int id);
int duel_08827044(unsigned int p, int id);
int duel_088E9150(int id);

extern int duel_phase;
extern unsigned int duel_turn_player;


#ifdef NON_MATCHING /* forma próxima (~400 palavras): ordem dos operandos dos | e blocos de retorno compartilhados */
unsigned int duel_08827DA4(unsigned int p, int uid, int mode, int tribute)
{
    int id = duel.list[uid].id;
    unsigned int r = 0;
    unsigned int sum = 0x10;
    unsigned int set = 0x40;

    if (id == 0) {
        return 0;
    }
    if (card_is_monster(id) == 0) {
        return 0;
    }
    if (duel_card_protected(id, -1) != 0) {
        if (p == duel_view_player) {
            duel_err = 0x10;
        }
        return 0;
    }
    if (duel_can_pay(p, id) == 0) {
        if (p == duel_view_player) {
            duel_err = 0x17;
        }
        return 0;
    }
    if (card_is_group_b(id) == 0) {
        return 0;
    }
    if (duel_count_active(0x135D) != 0) {
        sum = 0x50;
        set = 0;
    }
    if (duel_can_special_summon(p) == 0) {
        sum = 0;
        if (p == duel_view_player) {
            duel_err = 0x11;
        }
    }
    if (duel_can_activate(p) == 0) {
        set = 0;
    }
    switch (id) {
    case 0x1386:
    case 0x1387:
    case 0x1388:
    case 0x1905:
        if (tribute != 0) {
            return 0;
        }
        if (duel_08827CEC(p, 0) == 0) {
            if (p == duel_view_player) {
                duel_err = 0xB;
            }
            return 0;
        }
        if (duel_count_tributes(p, id, 1) >= 3) {
            r = r | sum | set;
        }
        if (r == 0) {
            if (p == duel_view_player) {
                duel_err = 0;
            }
            return 0;
        }
        return r;
    case 0x16EC:
    case 0x1725:
    case 0x1729:
    case 0x1985:
    case 0x1986:
    case 0x1B2E:
    case 0x1B2F:
        if (duel_08827934(p, id) != 0) {
            r |= sum | set;
            return r;
        }
        return r;
    case 0x147B:
        if (tribute == 0 && duel.players[p & 1].n_hand == 1 && duel_count_free_monster_zones(p) != 0 && mode != 0) {
            r |= sum;
        }
        break;
    case 0x14DB:
        if (duel_count_monsters(p) != 0) {
            sum = 0;
        }
        break;
    case 0x158B:
    case 0x160F:
        if (duel_card_check_9148(p, id) != 0) {
            sum = 0;
        }
        break;
    case 0x164A:
    case 0x164B:
    case 0x164C:
    case 0x164D:
    case 0x164E:
    case 0x164F:
        if (duel_card_check_9148(p, card_id_remap_164a(id)) == 0) {
            sum = 0;
        }
        break;
    case 0x1691:
        if (duel_count_monsters_fn(p, card_is_in_group_1911) == 0) {
            sum = 0;
        }
        break;
    case 0x17D8:
    case 0x1823:
        sum = 0;
        break;
    case 0x1838:
        if (tribute == 0 && duel_count_free_monster_zones(p) != 0 && mode != 0) {
            r |= sum | set;
        }
        break;
    case 0x1864:
        if (mode != 0 && tribute == 0) {
            if (duel_08827CEC(p, 0) == 0) {
                if (p == duel_view_player) {
                    duel_err = 0xB;
                }
            } else {
                if (duel_count_tributes(p, id, 1) > 0) {
                    r |= sum | set;
                }
                if (r == 0 && p == duel_view_player) {
                    duel_err = 0;
                }
            }
        }
        break;
    case 0x194C:
        if (duel.players[p & 1].n_hand < 2) {
            return 0;
        }
        if (duel_08827CEC(p, 0) == 0) {
            if (p == duel_view_player) {
                duel_err = 0xB;
            }
            return 0;
        }
        if (duel_count_tributes(p, 0x194C, 1) <= 0) {
            return 0;
        }
        return sum | set;
    case 0x1A20:
        if (tribute == 0 && duel_count_free_monster_zones(p) != 0 && mode != 0) {
            r = sum;
        }
        break;
    case 0x1A21:
        if (mode != 0 && tribute == 0) {
            if (duel_08827CEC(p, 0) == 0) {
                if (p == duel_view_player) {
                    duel_err = 0xB;
                }
            } else {
                if (duel_count_tributes(p, 0x1A21, 1) > 0) {
                    r = sum;
                }
                if (r == 0 && p == duel_view_player) {
                    duel_err = 0;
                }
            }
        }
        break;
    case 0x1A96:
    case 0x1B2D:
        if (tribute == 0 && mode != 0 && duel_08827A90(p, id) != 0) {
            r = sum;
        }
        break;
    }
    if (tribute == 0 && mode != 0) {
        if (card_is_in_group_1a6f(id) != 0 && duel_08827B80(p, uid) != 0) {
            r |= sum;
        }
        if (card_is_in_group_19bb(id) != 0 && duel_08827BF0(p, uid) != 0) {
            r |= sum | set;
        }
        if (card_get_type(id) == 9 && duel_08827C80(p, uid) != 0) {
            r |= sum;
        }
        if (mode != 2 && card_is_in_group_19bb(id) != 0 && duel_slot_effect_with(p, 0xB, 0x19AD, uid) != 0 &&
            duel_count_free_monster_zones(p) != 0) {
            r |= sum;
        }
    }
    if (mode != 2) {
        switch (duel_list_card_level(p, uid)) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            if (tribute == 0) {
                if (duel_count_free_monster_zones(p) == 0) {
                    if (p == duel_view_player) {
                        duel_err = 0xB;
                    }
                } else {
                    r |= sum | set;
                }
            }
            break;
        case 5:
        case 6:
            if (tribute == 1 && duel_count_free_monster_zones(p) != 0) {
                r |= sum | set;
            } else if (tribute != 0) {
                return r;
            }
            if (duel_08827CEC(p, 0) == 0) {
                if (p == duel_view_player) {
                    duel_err = 0xB;
                }
            } else {
                if (duel_count_tributes(p, id, 1) > 0) {
                    r |= sum;
                }
                if (duel_count_tributes(p, id, 0) > 0) {
                    r |= set;
                }
                if (r == 0 && p == duel_view_player) {
                    duel_err = 0;
                }
            }
            break;
        default:
            if (tribute == 2 && duel_count_free_monster_zones(p) != 0) {
                r |= sum | set;
            } else if (tribute != 0) {
                return r;
            }
            if (duel_08827CEC(p, 0) == 0) {
                if (p == duel_view_player) {
                    duel_err = 0xB;
                }
            } else {
                if (duel_count_tributes(p, id, 1) > 1) {
                    r |= sum;
                }
                if (duel_count_tributes(p, id, 0) > 1) {
                    r |= set;
                }
                if (r == 0 && p == duel_view_player) {
                    duel_err = 0;
                }
            }
            break;
        }
    }
    return r;
}
#endif

#ifdef NON_MATCHING /* forma diferente na primeira cadeia de ids (o original repete os testes antes do pagamento); ~640 palavras */
/* Whether the card uid can be special summoned from the hand (or another zone) by its own rule:
   4 when it can. Sets duel_err for the viewing player. */
int duel_08828740(unsigned int p, int uid)
{
    int id = duel.list[uid].id;
    int r = 0;
    int n;

    if (id == 0) {
        return 0;
    }
    if (card_is_monster(id) == 0) {
        return 0;
    }
    if (duel_card_protected(id, -1) != 0) {
        if (p == duel_view_player) {
            duel_err = 0x10;
        }
        return 0;
    }
    if (duel_phase_ok2(p) == 0) {
        if (p == duel_view_player) {
            duel_err = 0x11;
        }
        return 0;
    }
    switch (id) {
    case 0x196B:
    case 0x196C:
    case 0x196D:
    case 0x196E:
        if (duel_hand_find_uid(p, uid) >= 0) {
            break;
        }
        /* fallthrough: from another zone, only in the main phases of its own turn */
    case 0x112E:
    case 0x128C:
    case 0x15B1:
    case 0x15B4:
    case 0x15F9:
    case 0x15FA:
    case 0x1953:
    case 0x1954:
    case 0x1A6D:
    case 0x1A6E:
    case 0x1A6F:
        if (duel_phase != 2 && duel_phase != 4) {
            return 0;
        }
        if (duel_turn_player != p) {
            return 0;
        }
        goto check;
    }
    if (duel_can_pay(p, id) == 0) {
        if (p == duel_view_player) {
            duel_err = 0x17;
        }
        return 0;
    }
check:
    switch (id) {
    case 0x1488:
        if (duel_count_free_monster_zones(p) != 0) {
            r = 4;
        }
        break;
    case 0x1299:
        if (duel_count_free_monster_zones(p) != 0) {
            n = duel_count_monsters(p);
            if (n + 1 < duel_count_monsters(1 - p)) {
                r = 4;
            }
        }
        break;
    case 0x164C:
        if (duel_count_free_monster_zones(p) != 0 && duel.players[p & 1].n_hand < 2 &&
            duel_card_check_9148(p, 0x165E) != 0) {
            r = 4;
        }
        break;
    case 0x17C7:
    case 0x17C8:
        if (duel_count_free_monster_zones(p) != 0 && duel_088E9150(0x17BF) != 0 &&
            duel_count_active(0x132C) * 500 + 500 < duel.players[p & 1].lp) {
            r = 4;
        }
        break;
    case 0x1806:
        if (duel_count_free_monster_zones(p) != 0 && duel.players[p & 1].n_hand > 1) {
            r = 4;
        }
        break;
    case 0x18F6:
        if (duel_count_free_monster_zones(p) != 0 && duel_count_monsters(p) == 0 && duel_count_monsters(1 - p) != 0) {
            r = 4;
        }
        break;
    case 0x18F9:
        if (duel_count_free_monster_zones(p) != 0 && duel.players[p & 1].n_hand == 1) {
            r = 4;
        }
        break;
    case 0x196B:
    case 0x196C:
    case 0x196D:
    case 0x196E:
        if (duel_08827044(p, id) != 0) {
            r = 4;
        }
        break;
    case 0x19AA:
        if (duel_count_free_monster_zones(p) != 0 && duel_count_faceup_name(p, 0x19AA) != 0) {
            r = 4;
        }
        break;
    case 0x1A92:
        if (duel_088271EC(p) != 0) {
            r = 4;
        }
        break;
    case 0xFDE:
    case 0xFDF:
    case 0xFE9:
        if (duel_08826548(p, id) != 0) {
            r = 4;
        }
        break;
    case 0x112E:
    case 0x128C:
        if (duel_d_find_uid(p, uid) >= 0 && duel_0882662C(p, id) != 0) {
            r = 4;
        }
        break;
    case 0x111C:
        if (duel_08826C0C(p) != 0) {
            r = 4;
        }
        break;
    case 0x138A:
        if (duel_08826DB8(p) != 0) {
            r = 4;
        }
        break;
    case 0x1466: case 0x1483: case 0x1484: case 0x1485: case 0x1486: case 0x1487: case 0x16C5:
    case 0x16C6: case 0x16C7: case 0x16C8: case 0x18B4: case 0x19CA:
        if (duel_08827618(p, id) != 0) {
            r = 4;
        }
        break;
    case 0x16C9:
    case 0x16CB:
    case 0x16E4:
        if (duel_088277BC(p, id) != 0) {
            r = 4;
        }
        break;
    case 0x13E9:
        if (duel_08826F74(p) != 0) {
            r = 4;
        }
        break;
    case 0x14FC:
        if (duel_count_free_monster_zones(p) != 0 && duel_card_check_9148(p, 0x1414) != 0) {
            r = 4;
        }
        break;
    case 0x1578:
        if (duel_08827118(p) != 0) {
            r = 4;
        }
        break;
    case 0x19A3:
    case 0x19A4:
    case 0x19C8:
        if (duel_0882696C(p, id) != 0) {
            r = 4;
        }
        break;
    case 0x19A6:
        if (duel_08826BC4(p, 0x19A6) != 0) {
            r = 4;
        }
        break;
    case 0x1758: case 0x176C: case 0x17C2: case 0x1894: case 0x18B9: case 0x18E8: case 0x1982:
    case 0x198C: case 0x19F0: case 0x19F7: case 0x19F8:
        if (duel_0882662C(p, id) != 0) {
            r = 4;
        }
        break;
    case 0x17C6:
    case 0x19A5:
        if (duel_08826740(p, id) != 0) {
            r = 4;
        }
        break;
    case 0x1A65:
        if (duel_088268A8(p, 0x1A65) != 0) {
            r = 4;
        }
        break;
    case 0x1895:
        if (duel_08826AE0(p, 0x1895) != 0) {
            r = 4;
        }
        break;
    case 0x15B1: case 0x15B4: case 0x15F9: case 0x15FA: case 0x1953: case 0x1954: case 0x1A6D:
    case 0x1A6E: case 0x1A6F:
        if (duel_088272E8(p, id) != 0) {
            r = 4;
        }
        break;
    case 0x127D: case 0x127E: case 0x127F: case 0x12A5: case 0x154A:
        if (duel_has_st_12be(p) == 0) {
            return 0;
        }
        switch (duel_list_card_level(p, uid)) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            if (duel_count_free_monster_zones(p) == 0) {
                if (p == duel_view_player) {
                    duel_err = 0xB;
                }
            } else {
                r = 4;
            }
            break;
        case 5:
        case 6:
            if (duel_count_tributable_except(p, -1) < 1) {
                if (p == duel_view_player) {
                    duel_err = 0;
                }
            } else if (duel_08827CEC(p, 1) == 0) {
                if (p == duel_view_player) {
                    duel_err = 0xB;
                }
            } else {
                r = 4;
            }
            break;
        default:
            if (duel_count_tributable_except(p, -1) < 2) {
                if (p == duel_view_player) {
                    duel_err = 0;
                }
            } else if (duel_08827CEC(p, 1) == 0) {
                if (p == duel_view_player) {
                    duel_err = 0xB;
                }
            } else {
                r = 4;
            }
            break;
        }
        break;
    }
    return r;
}
#endif
