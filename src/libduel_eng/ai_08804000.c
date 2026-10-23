// @range 0x08804000-0x088070A8
// @asm 0x088042D4-0x088044C8
// @asm 0x088044C8-0x08804828
// @asm 0x08806730-0x088070A8
/* Computer player: the main-phase steps that summon monsters from the hand. */
#include "duel.h"
#include "card_db.h"

extern unsigned int duel_turn_player;
extern unsigned int duel_turn;
extern unsigned int duel_ai_player;
extern unsigned int duel_w_0891964C;
extern int duel_w_0891962C;
extern int duel_w_08913EC4;
extern int duel_ai_main_step;          /* 0x08913CFC */
extern int duel_w_08913D00;
extern int duel_w_08913EDC[5];
extern unsigned int duel_phase;
extern unsigned int duel_w_0891960C;
extern int duel_w_08919610;
extern int duel_w_08919614;
extern unsigned int duel_w_08913CF4;
extern int duel_ai_phase;              /* 0x08913CF8 index into duel_ai_phases */
extern int (*duel_ai_phases[])(void);  /* 0x08912438 */

int duel_0884E948(DuelAct *a, unsigned int p);
int duel_facedown_can_act(unsigned int p, int z);
int field_get_card(unsigned int p, int z);
int duel_count_field_cards(unsigned int p, int faceup, int facedown, int fzone);
void duel_08826000(unsigned int p, int z);
int duel_088078AC(unsigned int p, int uid);
int duel_double_tribute(unsigned int p, int id, unsigned int q, int z);
int duel_0880750C(unsigned int p, int id, int a, int i, unsigned short *out);
int duel_can_special_summon(unsigned int p);
int duel_can_activate(unsigned int p);
void duel_0882AF5C(int a, int b, int c);
int duel_count_field_all(unsigned int p);
int duel_0880AF74(unsigned int p, int id);
int duel_count_name(unsigned int p, int id);
int duel_08827D98(unsigned int p, int uid);
int duel_count_monsters(unsigned int p);
int duel_count_tributable_except(unsigned int p, int src);
int duel_count_tributes();
int duel_088BFFD8(unsigned int p, int id, int i);
int duel_08809F2C(unsigned int p);
int duel_0880EC5C(unsigned int p, int a);
int duel_088066D0(unsigned int p);
int duel_find_effect_card(unsigned int p, int id, int z);
int duel_08820F60(unsigned int p);
int duel_0880EA18(unsigned int p);
int duel_08821124(unsigned int p);
int duel_08807434(unsigned int p, int a);
int duel_0880F3B4(unsigned int p, int *x);
int duel_08805810(unsigned int p, int z);
int duel_monster_can_act(unsigned int p, int z);
int duel_count_active(int id);
int duel_field_level(unsigned int p, int z);
int duel_088C9878(unsigned int p, int z, int x, int y, int id, int opp);
int duel_08820948(unsigned int p, int a);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
int duel_08836868(unsigned int p);
int duel_0880EA88(unsigned int p, int a);
void duel_088AFD5C(int kind, unsigned int a);
int duel_0883F174(unsigned int p);
int duel_088369D8(unsigned int p);
int duel_08820FD0(unsigned int p, int z);
void duel_field_stats(unsigned int p, int z, DuelStats *st);
int duel_attack_check(unsigned int p, int z, int arg);
int duel_can_attack(unsigned int p, int z, int arg);
int duel_0880A17C(unsigned int p, unsigned int q, int a, int b, int c);
int duel_0880A8AC(unsigned int p, unsigned int q);
int duel_count_facedown_st(unsigned int p);
int duel_hand_count_type(unsigned int p, int type);
int duel_field_base_atk(unsigned int p, int z);
int duel_count_type(unsigned int p, unsigned int type);
int duel_card_check_9148(unsigned int p, int id);
int duel_08805DA4(unsigned int p, int id, int a, int b);
int duel_slot_has_effect(unsigned int p, int z, int id);
int duel_count_faceup_name(unsigned int p, int id);
int duel_count_monsters_state(unsigned int p, int pos, int state);
int duel_count_grave_spells(unsigned int p, unsigned int st);
int duel_hand_count_name(unsigned int p, unsigned short id);
int duel_0880A400(unsigned int p, unsigned int q, int skip, int fa, int fd);
int duel_0880B07C(unsigned int p);
int duel_slot_e68a0(unsigned int p, int z);
int duel_is_cur_card(int id);
int duel_count_monsters_faceup_is(unsigned int p, unsigned int faceup);
int duel_count_free_monster_zones(unsigned int p);
int duel_088071E8(int id);
int duel_0880AEF4(unsigned int p, int id);
int duel_count_monsters_fn(unsigned int p, int (*fn)(int id));
int duel_card_fits_zone(unsigned int p, int z, int id);
int duel_088C1D80(unsigned int p);
int duel_count_grave_type(unsigned int p, unsigned int type);
int duel_count_monsters_pos(unsigned int p, int faceup, int pos0);
int duel_grave_count_id(unsigned int p, int id);

int module_entry(void)
{
    return 0;
}

/* Can the computer flip the face-down monster at (p, z)? */
int duel_08804008(unsigned int p, int z)
{
    int id;

    if (duel_facedown_can_act(p, z)) {
        id = field_get_card(p, z);
        switch (id) {
        case 0x16FA:
        case 0x1533:
        case 0x19F1:
            return 1;
        case 0x11F5:
            return duel.players[p & 1].n_hand < 6;
        case 0x119B:
        case 0x11C3:
        case 0x1200:
        case 0x1227:
        case 0x152A:
        case 0x152B:
        case 0xFFA:
        case 0x1086:
        case 0x13AB:
        case 0x179A:
        case 0x13B0:
        case 0x1413:
        case 0x152E:
        case 0x186E:
        case 0x18BA:
        case 0x18BB:
        case 0x16BA:
        case 0x1877:
        case 0x17E0: {
            DuelAct t = {0};

            t.player = p;
            t.zone = z;
            t.id = id;
            t.uid = CARD_UID(duel.players[p & 1].field[z].card);
            t.b3_4 = 1;
            if (duel_0884E948(&t, 1 - p) == 0) {
                return 0;
            }
            break;
        }
        case 0x11F0:
            if (duel_count_field_cards(1 - p, 0, 1, 1) < 2) {
                return 0;
            }
            break;
        }
        return card_get_special_group(id);
    }
    return 0;
}

#ifdef NON_MATCHING /* o original grava duel_ai_player e duel_w_0891964C antes de ler o uid da carta; o mwcc antecipa as leituras */
/* Summon the monster in hand slot h, picking the tributes. */
int duel_088042D4(unsigned int p, int h, int a)
{
    unsigned short buf[4];
    int id = duel.players[p & 1].hand[h].id;
    int n = duel_088078AC(p, CARD_UID(duel.players[p & 1].hand[h]));
    int i;
    unsigned int r;
    int z;
    unsigned int q;

    if (duel_can_special_summon(p) == 0) {
        a = 1;
    }
    for (i = 0; i < n; i++) {
        r = duel_0880750C(p, id, 0, i, buf);
        if (r == -1) {
            return -1;
        }
        q = r & 0xFF;
        if (p == q) {
            z = (unsigned short)r >> 8 & 0xFF;
            if (duel_08804008(p, z) != 0) {
                duel_08826000(p, z);
                return 0;
            }
        }
        if (duel_double_tribute(p, id, q, (unsigned short)r >> 8 & 0xFF)) {
            buf[i] = r;
            i++;
        }
        buf[i] = r;
    }
    a = !a;
    duel_ai_player = p;
    duel_w_0891964C = p;
    duel_w_0891962C = CARD_UID(duel.players[p & 1].hand[h]);
    duel_0882AF5C(a, 0, 1);
    duel_w_08913EC4 = 0;
    return 1;
}
#endif

#ifdef NON_MATCHING /* o original grava duel_ai_player e duel_w_0891964C antes de ler o uid da carta; o mwcc antecipa as leituras */
/* Special summon from the hand a monster whose condition is met. */
int duel_088044C8(unsigned int p)
{
    unsigned int i;
    int id;
    int ok;

    if (duel_can_special_summon(p) == 0) {
        return 0;
    }
    for (i = 0; i < duel.players[p & 1].n_hand; i++) {
        id = duel.players[p & 1].hand[i].id;
        if (!(duel_08827D98(p, CARD_UID(duel.players[p & 1].hand[i])) & 0x10)) {
            continue;
        }
        ok = 0;
        switch (id) {
        case 0x1386:
        case 0x1387:
        case 0x1388:
        case 0x16EC:
        case 0x1B2E:
        case 0x1B2F:
            ok = 1;
            break;
        case 0x147B:
            ok = duel.players[p & 1].n_hand == 1;
            break;
        case 0x154B:
            ok = duel_count_monsters(1 - p) != 0 && duel_count_tributes(p, id, 1) >= 3;
            break;
        case 0x13B5:
            ok = duel_count_field_all(1 - p) != 0 && duel_count_tributes(p, id, 1) >= 3;
            break;
        case 0x1905:
            ok = !duel_count_name(p, id);
            break;
        case 0x19BC:
            ok = duel_count_name(p, 0x1870) != 0 ||
                 (duel_0880AF74(p, 0x18F6) != 0 && (duel_0880AF74(p, 0x12E5) != 0 || duel_0880AF74(p, 0x18FE) != 0));
            break;
        case 0x1870:
            ok = duel_count_name(p, 0x1870) != 0 || duel_count_name(p, 0x18F6) != 0;
            break;
        case 0x185D:
            ok = duel_count_tributable_except(p, -1) != 0 && duel_088BFFD8(p, id, 0) != 0;
            break;
        }
        if (ok) {
            DuelCard *c = &duel.players[p & 1].hand[i];

            duel_ai_player = p;
            duel_w_0891964C = p;
            duel_w_0891962C = CARD_UID(*c);
            duel_0882AF5C(1, 0, 1);
            return 1;
        }
    }
    return 0;
}
#endif

/* Main-phase summon steps of the computer player; 0 while there is more to do. */
int duel_08804828(void)
{
    unsigned int p = duel_turn_player;
    int z;
    int r;
    int x;

    switch (duel_ai_main_step) {
    case 0:
        duel_w_08913EC4 = 0;
        if (duel_turn == 0) {
            return 1;
        }
        if (duel.players[p & 1].flags >> 17 & 1 || duel_088066D0(p) == 0 ||
            (duel_can_special_summon(p) == 0 && duel_can_activate(p) == 0) ||
            duel_find_effect_card(1 - p, 0x1102, -1) > 0) {
            duel_ai_main_step++;
            return 0;
        }
        if (duel_088044C8(p) != 0) {
            return 0;
        }
        if (duel_can_special_summon(p) != 0) {
            z = -1;
            if (duel_08807434(p, 1) >= 0) {
                z = duel_0880EC5C(p, 1);
            } else if (duel_0880EA18(p) != 0) {
                z = duel_0880EC5C(p, 0);
            }
            if (z == -1) {
                z = duel_0880EC5C(p, -1);
            }
            if (z >= 0 && duel_088042D4(p, z, 0) == 0) {
                return 0;
            }
        }
        duel_ai_main_step++;
        return 0;
    case 1:
        if (duel_08809F2C(p) >= 0 && duel_0880EA18(p) != 0) {
            return 1;
        }
        duel_w_08913D00 = 0;
        duel_ai_main_step++;
        return 0;
    case 2:
        if (duel_08821124(p) != 0) {
            duel_ai_main_step = 0;
            return 0;
        }
        duel_ai_main_step++;
        return 0;
    case 3:
        if (duel_08820F60(p) != 0) {
            duel_ai_main_step = 0;
            return 0;
        }
        duel_ai_main_step++;
        return 0;
    case 4:
        if (duel_088066D0(p) == 0 || duel.players[p & 1].flags >> 17 & 1 ||
            (duel_can_special_summon(p) == 0 && duel_can_activate(p) == 0)) {
            duel_ai_main_step++;
            return 0;
        }
        r = duel_0880F3B4(p, &x);
        if (r >= 0 && x == 0) {
            r = duel_088042D4(p, r, 0);
            if (r == 0) {
                return 0;
            }
            if (r > 0) {
                duel_ai_main_step = 1;
                return 0;
            }
        }
        return 1;
    default:
        return 1;
    }
    return 1;
}

int duel_08804BAC(void)
{
    unsigned int p = duel_turn_player;
    int r;
    int x;

    switch (duel_ai_main_step) {
    case 0:
        if (duel_088066D0(p) == 0 || duel.players[p & 1].flags >> 17 & 1 ||
            (duel_can_special_summon(p) == 0 && duel_can_activate(p) == 0)) {
            duel_ai_main_step++;
            return 0;
        }
        r = duel_0880F3B4(p, &x);
        if (r > -1) {
            if (duel_088042D4(p, r, x) != 0) {
                duel_ai_main_step++;
            }
            return 0;
        }
        duel_ai_main_step++;
        return 0;
    }
    return 1;
}

/* Flip, change the position of or summon the monster at (p, z) when it is worth it. */
int duel_08804CC4(unsigned int p, int z, int a)
{
    int r = -1;

    if (a == 0 && duel_w_08913EDC[z] != 0) {
        r = 0;
    }
    if (r == -1) {
        r = duel_08805810(p, z);
    }
    if (duel_monster_can_act(p, z) == 0) {
        return 0;
    }
    if (r == 0) {
        if (duel.players[p & 1].field[z].faceup == 0) {
            if (duel_facedown_can_act(p, z) != 0) {
                duel_08826000(p, z);
                return 1;
            }
        } else if (duel.players[p & 1].field[z].pos != 0) {
            if (duel_count_active(0x17A6) != 0 && duel_field_level(p, z) >= 4) {
                return 0;
            }
            duel_088C9878(p, z, 0, 0, 0, 0);
            return 1;
        }
    } else if (r == 1 && duel.players[p & 1].field[z].pos == 0) {
        if (duel_count_active(0x15FB) != 0) {
            return 0;
        }
        if (duel_count_active(0x197B) != 0 && duel_field_level(p, z) < 4) {
            return 0;
        }
        duel_088C9878(p, z, 0, 0, 0, 0);
        return 1;
    }
    return 0;
}

int duel_08804EDC(void)
{
    unsigned int p = duel_turn_player;
    int z;

    switch (duel_ai_main_step) {
    case 0:
        if (duel_phase >= 3) {
            return 1;
        }
        if (duel_08820948(p, 0) != 0) {
            return 0;
        }
        duel_ai_main_step++;
    case 1:
        if (duel_08821124(p) != 0) {
            return 0;
        }
        duel_ai_main_step++;
    case 2:
        duel_0880EA18(p);
        z = 0;
        do {
            if (!duel_08804CC4(p, z, 0)) {
                continue;
            }
            return 0;
        } while (++z <= 4);
        return 1;
    default:
        return 1;
    }
    return 1;
}

int duel_08804FEC(void)
{
    unsigned int p = duel_turn_player;
    int z;

    switch (duel_ai_main_step) {
    case 0:
        if (duel_08820948(p, 1) != 0) {
            return 0;
        }
        duel_ai_main_step++;
    case 1:
        if (duel_08821124(p) != 0) {
            return 0;
        }
        duel_ai_main_step++;
    case 2:
        z = 0;
        do {
            if (!duel_08804CC4(p, z, 1)) {
                continue;
            }
            return 0;
        } while (++z <= 4);
        return 1;
    default:
        return 1;
    }
    return 1;
}

int duel_088050D8(void)
{
    unsigned int p = duel_turn_player;

    switch (duel_ai_main_step) {
    case 0:
        if (duel_08836868(p) == 0) {
            return 1;
        }
        if (duel_088369D8(p) != 0 && duel_0880EA88(p, 0) == 0) {
            return 1;
        }
        if (duel_08820FD0(p, 1) != 0) {
            return 0;
        }
        duel_w_0891960C = 0;
        duel_w_08919610 = 0;
        duel_ai_main_step++;
        break;
    case 1:
        duel_088AFD5C(3, 0);
        duel_ai_main_step++;
        break;
    case 2:
        if (duel_0883F174(p) != 0) {
            if (duel_w_08919614 != 0) {
                return 1;
            }
            duel_ai_main_step++;
        }
        break;
    case 3:
        duel_cmd((p ? 0x8000 : 0) | 0x10, 0, 0, 0);
        return 1;
    }
    return 0;
}

/* One step of the computer's turn: run the handler of the current phase; 1 when the turn is over. */
int duel_08805248(void)
{
    int (*fn)(void);

    duel_w_08913CF4 = duel_turn_player;
    if (duel_ai_phase == 0 && duel_phase == 4) {
        duel_ai_phase = 7;
    }
    fn = duel_ai_phases[duel_ai_phase];
    if (fn != 0) {
        if (fn() != 0) {
            duel_ai_main_step = 0;
            duel_w_08913D00 = 0;
            duel_ai_phase++;
            if (duel_w_08919614 != 0) {
                return 1;
            }
        }
        return 0;
    }
    return 1;
}

/* Stats of the monster at (p, z) as if it were face up. */
void duel_08805308(unsigned int p, int z, DuelStats *st)
{
    if (duel.players[p & 1].field[z].faceup == 0) {
        duel.players[p & 1].field[z].faceup = 1;
        duel_field_stats(p, z, st);
        duel.players[p & 1].field[z].faceup = 0;
    } else {
        duel_field_stats(p, z, st);
    }
}

/* Stats of the monster at (p, z) as the opponent sees them: a guess from the level when face down. */
void duel_08805378(unsigned int p, int z, DuelStats *st)
{
    if (duel.players[p & 1].field[z].faceup == 0) {
        int id = duel.players[p & 1].field[z].card.id;

        if (duel.players[p & 1].field[z].flags >> 7 & 1) {
            duel.players[p & 1].field[z].faceup = 1;
            duel_field_stats(p, z, st);
            duel.players[p & 1].field[z].faceup = 0;
        } else {
            st->level = card_get_level(id);
            if (st->level < 5) {
                st->atk = 1200;
                st->def = 1000;
            } else {
                st->atk = 2100;
                st->def = 1800;
            }
            st->unk8 = 0;
            st->attr = 0;
            st->id = 0;
        }
    } else {
        duel_field_stats(p, z, st);
    }
}

/* Stats of the monster at (p, z) as the computer player knows them. */
void duel_0880546C(unsigned int p, int z, DuelStats *st)
{
    if (duel_w_08913CF4 == p) {
        duel_08805308(p, z, st);
    } else {
        duel_08805378(p, z, st);
    }
}

/* Highest ATK among the monsters of p that can attack, except the one at skip. */
int duel_088054A8(unsigned int p, int skip, int mode)
{
    int z;
    int best = -1;
    int r;
    DuelStats st;

    for (z = 0; z <= 4; z++) {
        if (z != skip && duel.players[p & 1].field[z].card.id != 0) {
            if (mode != 0) {
                r = duel_can_attack(p, z, 1);
            } else {
                r = duel_attack_check(p, z, 1);
            }
            if (r != 0) {
                duel_0880546C(p, z, &st);
                if (best < st.atk) {
                    best = st.atk;
                }
            }
        }
    }
    return best;
}

/* Should the computer attack (1), defend (0) or not care (-1) with a monster of these ATK/DEF? */
int duel_088055A0(unsigned int p, int atk, int def)
{
    int best;
    int margin;
    unsigned int q;
    int d;
    int x;
    int y;

    q = 1 - p;
    best = duel_088054A8(q, -1, 0);
    x = duel_0880A17C(p, p, -1, 1, 0);
    y = duel_0880A17C(p, p, -1, 0, 1);
    margin = 100;

    if (duel_count_monsters(q) == 0 && duel.players[q & 1].lp <= duel_0880A8AC(p, p) &&
        duel_08836868(p) != 0 && atk > 0) {
        return 0;
    }
    if (atk == 0) {
        return 1;
    }
    if (duel_find_effect_card(p, 0x13F9, -1) != 0) {
        return 0;
    }
    best = MAX(best, 1500);
    d = best - atk;
    if (d >= 1000) {
        return 1;
    }
    if (duel_count_facedown_st(p) != 0 && duel_hand_count_type(p, 0x16) != 0) {
        margin = 300;
    }
    if (duel_find_effect_card(q, 0x130C, -1) != 0) {
        margin = 0;
    }
    if (atk >= def) {
        if (d <= margin && margin < duel.players[p & 1].lp) {
            return 0;
        }
    } else {
        if (atk >= best && atk >= 1700) {
            return 0;
        }
        x = MAX(x, y);
        if (def >= x && def > 1500) {
            return 1;
        }
    }
    if (atk >= best) {
        return 0;
    }
    if (margin < d) {
        return 1;
    }
    return -1;
}

/* Position the computer wants for its monster at (p, z): 1 attack, 0 defense, -1 no preference. */
int duel_08805810(unsigned int p, int z)
{
    int id;
    int r;
    int i;
    DuelStats st;

    id = field_get_card(p, z);
    if (!(duel.players[p & 1].field[z].flags >> 5 & 1)) {
        switch (id) {
        case 0x1476:
        case 0x1370:
        case 0x1286:
        case 0x19BD:
        case 0x19D2:
            if (duel.players[p & 1].field[z].faceup == 0) {
                return 1;
            }
            break;
        case 0x12A3:
        case 0x129A:
        case 0x1439:
        case 0x1621:
        case 0x1639:
        case 0x1743:
        case 0x14AF:
        case 0x18C3:
        case 0x1A07:
        case 0x1A78:
            return 0;
        case 0x1813:
        case 0x1815:
        case 0x195A:
            return 1;
        case 0x13AD:
            if (duel.players[p & 1].field[z].faceup == 0) {
                return 0;
            }
            break;
        case 0x13A7:
            if (duel.players[p & 1].lp > 2000) {
                return 0;
            }
            break;
        case 0x1451:
            if (duel.players[p & 1].field[z].faceup != 0 && duel_count_monsters(1 - p) == 0 &&
                duel_count_monsters(p) > 1) {
                return 1;
            }
            break;
        case 0x147F:
            if (duel.players[p & 1].field[z].faceup == 0) {
                /* stay in defense only when the opponent has a monster with flag 3 and we have none */
                r = 0;
                for (i = 0; i <= 4; i++) {
                    if (duel.players[(1 - p) & 1].field[i].card.id != 0 &&
                        duel.players[(1 - p) & 1].field[i].flags >> 3 & 1) {
                        r = 1;
                    }
                }
                if (r != 0) {
                    for (i = 0; i <= 4; i++) {
                        if (duel.players[p & 1].field[i].card.id != 0 &&
                            duel.players[p & 1].field[i].flags >> 3 & 1) {
                            r = 0;
                        }
                    }
                    if (r != 0) {
                        return 0;
                    }
                }
            }
            break;
        case 0x147A:
            if (duel_card_check_9148(p, 0x146F) == 0) {
                return 1;
            }
            break;
        case 0x19A8:
            if (duel_count_monsters(1 - p) == 0) {
                return 0;
            }
            break;
        case 0x191C:
            if (duel_count_type(p, 7) != 0) {
                return 0;
            }
            break;
        case 0x1855:
            r = duel_08807434(p, 0);
            if (r >= 0 && duel_field_base_atk(r & 0xFF, (unsigned short)r >> 8 & 0xFF) >= 1000) {
                return 0;
            }
            break;
        }
        if (card_get_special_group(id) != 0 && duel.players[p & 1].field[z].faceup == 0) {
            return 1;
        }
    }
    if (duel.players[p & 1].field[z].faceup != 0) {
        duel_field_stats(p, z, &st);
    } else {
        duel.players[p & 1].field[z].faceup = 1;
        duel_field_stats(p, z, &st);
        duel.players[p & 1].field[z].faceup = 0;
    }
    r = duel_088055A0(p, st.atk, st.def);
    if (r != -1) {
        return r;
    }
    return -1;
}

/* Cards the computer prefers to set face down. */
int duel_08805C88(int id)
{
    if (card_get_special_group(id) != 0) {
        return 1;
    }
    switch (id) {
    case 0x1286:
    case 0x133D:
    case 0x1370:
    case 0x13E8:
    case 0x1476:
    case 0x152E:
    case 0x19BD:
        return 1;
    }
    return 0;
}

int duel_08805D18(unsigned int p, int id, int a, int dflt)
{
    int r = duel_08805DA4(p, id, a, 0);

    if (r != -1) {
        return r;
    }
    r = duel_088055A0(p, card_get_atk(id), card_get_def(id));
    if (r != -1) {
        return r;
    }
    return dflt;
}

/* Position for summoning card id from the hand: 1 attack, 0 set, -1 decide from the stats.
   set: the card is being set anyway; tribute: it is a tribute summon. */
int duel_08805DA4(unsigned int p, int id, int set, int tribute)
{
    int n;
    int z;

    if (duel_phase == 3 && duel_slot_has_effect(p, 0xB, 0x1669) != 0) {
        return 1;
    }
    if (duel_count_active(0x135D) > 0) {
        set = 1;
    }
    switch (id) {
    case 0xFE0:
        if (set != 0 && duel_count_free_monster_zones(p) >= 2 && duel_count_name(p, 0x12C5) != 0) {
            return 0;
        }
        break;
    case 0x119A:
    case 0x1334:
    case 0x1341:
    case 0x1293:
    case 0x128A:
    case 0x170C:
    case 0x14C6:
    case 0x1617:
    case 0x128E:
    case 0x1918:
    case 0x19C6:
        if (tribute == 0 && set == 0) {
            return 0;
        }
        break;
    case 0xFB6:
    case 0x101E:
    case 0x11A6:
    case 0x139C:
    case 0x13AD:
    case 0x144D:
    case 0x148C:
    case 0x14CB:
    case 0x156B:
    case 0x158B:
    case 0x15DA:
    case 0x172E:
    case 0x1807:
    case 0x180B:
    case 0x180C:
    case 0x183A:
    case 0x1868:
    case 0x19BC:
    case 0x1A1E:
        if (set == 0) {
            return 0;
        }
        break;
    case 0x12BB:
        if (set == 0 && duel_count_monsters_faceup_is(1 - p, 1) != 0) {
            return 0;
        }
        break;
    case 0x1694:
        if (set == 0) {
            return duel_count_monsters(0) + duel_count_monsters(1) > 0;
        }
        break;
    case 0x1281:
    case 0x1284:
        if (duel_count_monsters(1 - p) != 0) {
            return 0;
        }
        break;
    case 0x13CD:
    case 0x12A8:
    case 0x1706:
    case 0x1709:
        if (set == 0 && duel_is_cur_card(0x10F4) != 0) {
            return 0;
        }
        break;
    case 0x13A7:
        if (duel.players[p & 1].lp > 2000) {
            return 0;
        }
        break;
    case 0x14D7:
        if (duel_hand_count_type(p, 1) > 1) {
            return 0;
        }
        break;
    case 0x147A:
        if (duel_card_check_9148(p, 0x146F) == 0) {
            return 1;
        }
        break;
    case 0x118A:
        if (duel_0880AF74(p, 0x142A) != 0) {
            return 0;
        }
        break;
    case 0x14AC:
        if (tribute == 0 && duel_count_monsters(1 - p) != 0) {
            return 0;
        }
        break;
    case 0x1612:
        for (z = 0; z < 10; z++) {
            n = card_get_group_value_1983(field_get_card(p, z));
            if (n != 0 && duel_slot_e68a0(p, z) < n) {
                return 0;
            }
        }
        break;
    case 0x161C:
        if (set == 0 && duel_0880B07C(p) != 0) {
            return 0;
        }
        break;
    case 0x1399:
        if (set == 0 && duel_count_monsters(p) >= duel_count_monsters(1 - p)) {
            return 0;
        }
        break;
    case 0x1744:
    case 0x1755:
    case 0x1756:
    case 0x1863:
        if (set == 0 && duel_count_type(p, card_get_type(id)) >= duel_count_monsters(1 - p)) {
            return 0;
        }
        break;
    case 0x17DD:
        if (set == 0 && duel_hand_count_name(p, 0xFF8) != 0) {
            return 0;
        }
        break;
    case 0x1836:
        if (set == 0 && duel.players[p & 1].lp >= 4000) {
            return 0;
        }
        break;
    case 0x1523:
        if (set == 0) {
            n = duel_0880A400(p, 1 - p, -1, 1, 0);
            if (n <= 1100 || n >= 1900) {
                return 0;
            }
        }
        break;
    case 0x189A:
        if (set == 0 && duel_hand_count_name(p, 0xFA7) != 0) {
            return 0;
        }
        break;
    case 0x1821:
        if (set == 0 && duel_hand_count_name(p, 0x1820) != 0) {
            return 0;
        }
        break;
    case 0x185D:
        if (set == 0 && duel_count_monsters(p) != 0) {
            return 0;
        }
        break;
    case 0x18B6:
        if (set == 0 && duel_count_monsters_state(p, 0, 1) != 0) {
            return 0;
        }
        break;
    case 0x16BC:
        if (set == 0 && duel_count_grave_spells(p, 3) != 0) {
            return 1;
        }
        break;
    case 0x17DE:
        if (set == 0 && duel_0880AF74(p, 0x12DC) != 0) {
            return 0;
        }
        break;
    case 0x1813:
    case 0x1815:
    case 0x195A:
        return 1;
    case 0x129A:
    case 0x1439:
    case 0x1621:
    case 0x1639:
    case 0x1743:
    case 0x1829:
    case 0x18C3:
    case 0x1A07:
        return 0;
    case 0x191C:
        if (duel_count_type(p, 7) != 0) {
            return 0;
        }
        if (duel_find_effect_card(p, 0x1357, -1) != 0 && duel_0880AF74(p, 0x198E) != 0) {
            return 0;
        }
        break;
    case 0x182D:
        if (duel_count_active(0x140E) != 0 || duel_count_active(0x17A6) != 0) {
            return 0;
        }
        break;
    case 0x1914:
        if (set == 0) {
            return !duel_count_faceup_name(0, 0x1784) && !duel_count_faceup_name(1, 0x1784);
        }
        break;
    }
    if (duel_08836868(p) != 0 && duel_0880AF74(p, 0x142A) != 0 && duel_088071E8(id) != 0) {
        return 0;
    }
    if (set == 0 && duel_08805C88(id) != 0) {
        return 1;
    }
    return -1;
}

/* Is the life of p above what the cards like 0x132C would cost (500 each)? */
int duel_088066D0(unsigned int p)
{
    return duel_count_active(0x132C) * 500 < duel.players[p & 1].lp;
}

#ifdef NON_MATCHING /* nos casos 0x11ED, 0xFCE e 0x185F o original materializa !f() (sltu/xori) antes do beqzl e guarda o resultado num registrador s; nenhuma forma testada reproduz */
/* Is it worth summoning card id (n: tributes available) for the computer? */
int duel_08806730(unsigned int p, int n, unsigned short id)
{
    int z;
    int r;

    switch (id) {
    case 0xFB7:
    case 0xFB8:
    case 0xFB9:
    case 0xFBA:
    case 0xFBB:
        return !duel_0880AEF4(p, 0x1645);
    case 0x1386:
    case 0x1387:
    case 0x1388:
    case 0x16EC:
    case 0x1725:
    case 0x1729:
    case 0x1985:
    case 0x1986:
    case 0x1B2E:
    case 0x1B2F:
        return 1;
    }
    if (n > 0) {
        switch (id) {
        case 0x10F7:
        case 0x10F6:
        case 0x12FC:
        case 0x12EC:
        case 0x1246:
        case 0x1317:
        case 0x12FE:
        case 0x12FF:
        case 0x12EA:
        case 0x1322:
        case 0x1102:
        case 0x1281:
        case 0x12BE:
        case 0x131B:
        case 0x1468:
        case 0x14F0:
            return 1;
        case 0x1331:
            if (duel_08809F2C(p) > 0) {
                return 1;
            }
            break;
        case 0xFD6:
        case 0x11E4:
            if (n == 1) {
                return 1;
            }
            break;
        case 0x179A:
            if (duel_phase == 5) {
                return 1;
            }
            break;
        }
    }
    if (n > 1) {
        switch (id) {
        case 0xFE0:
            return !duel_0880AF74(p, 0x12C5);
        case 0x12BB:
        case 0x157B:
            return !duel_count_monsters_pos(1 - p, 1, 0);
        case 0x11ED:
            p = 1 - p;
            r = 0;
            if (!duel_find_effect_card(p, 0x1102, -1) &&
                duel_hand_count_name(p, 0x1102) + duel_grave_count_id(p, 0x1102) > 0) {
                r = 1;
            }
            return r;
        case 0xFCE:
            p = 1 - p;
            r = 0;
            if (!duel_find_effect_card(p, 0x10EF, -1) &&
                duel_hand_count_name(p, 0x10EF) + duel_grave_count_id(p, 0x10EF) > 0) {
                r = 1;
            }
            return r;
        case 0x1152:
            return !duel_count_grave_type(p, 0x16);
        case 0x100C:
            return !duel_count_grave_type(p, 0x17);
        case 0x11F0:
        case 0x1086:
        case 0x10B0:
            return !duel_count_facedown_st(1 - p);
        case 0x13CD:
        case 0x12A8:
        case 0x1706:
        case 0x1709:
            return !duel_is_cur_card(0x10F4) &&
                   duel_hand_count_name(p, 0x10F4) + duel_grave_count_id(p, 0x10F4) > 0;
        case 0x1129:
        case 0x1133:
        case 0x1134:
        case 0x114C:
        case 0x118E:
        case 0x1221:
            return !duel_count_active(0x140E);
        case 0xFB6:
            return duel_08809F2C(p) > 0;
        case 0x147A:
            return !duel_card_check_9148(p, 0x146F);
        case 0x14CB:
        case 0x174D:
            return !duel_count_monsters(1 - p);
        case 0x1505:
            return duel_count_monsters(1 - p) == 1;
        case 0x158B:
            return duel_count_name(p, id) > 0;
        case 0x185F:
            p = 1 - p;
            r = 0;
            if (!duel_count_monsters_state(p, 0, 1) && !duel_count_field_cards(p, 0, 1, 1)) {
                r = 1;
            }
            return r;
        case 0x179A:
            return duel_grave_count_id(p, 0x179A) > 0;
        case 0x185D:
            return !duel_count_monsters(p);
        case 0x118A:
            return !duel_0880AF74(p, 0x142A);
        case 0x1298:
            for (z = 5; z < 10; z++) {
                if (!duel_card_fits_zone(1 - p, z, duel.players[(1 - p) & 1].field[z].card.id)) {
                    continue;
                }
                return 0;
            }
            for (z = 5; z < 10; z++) {
                if (!duel_card_fits_zone(p, z, duel.players[p & 1].field[z].card.id)) {
                    continue;
                }
                return 1;
            }
            break;
        case 0x13A4:
            if (duel_088C1D80(p) != 0) {
                return 1;
            }
            break;
        case 0x1807:
            if (duel_count_name(p, 0x1807) != 0 &&
                (duel_hand_count_name(p, 0x180B) != 0 || duel_hand_count_name(p, 0x180C) != 0)) {
                return 1;
            }
            break;
        case 0x1918:
            return !duel_grave_count_id(p, 0x1919);
        case 0x1526:
            return duel_count_monsters_pos(1 - p, 1, 0) < 2 && duel_08809F2C(p) > 0;
        case 0x18F7:
            return !duel_grave_count_id(p, 0x12E5);
        case 0x1914:
            return !duel_count_faceup_name(0, 0x1784) && !duel_count_faceup_name(1, 0x1784);
        case 0x18F6:
            return duel_count_monsters(p) > 0;
        case 0x19BC:
            return !(duel_hand_count_name(p, 0x18F6) || duel_count_name(p, 0x18F6) || duel_0880AF74(p, 0x19B5)) &&
                   !duel_count_name(p, 0x1870);
        case 0x1A1E:
            return !duel_count_monsters_fn(p, card_is_in_group_183f);
        }
    }
    return 0;
}
#endif
