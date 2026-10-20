// @range 0x08887618-0x08889BE4
// @data 0x089117DC-0x08911828
// @asm 0x08887618-0x088876B0
// @asm 0x08887B34-0x08887ED8
// @asm 0x088887C0-0x088888F8
// @asm 0x08888ED0-0x08889914
// @asm 0x08889B30-0x08889BE4
/* Card effect handlers (DuelAct *a), after effects_w.c. */
#include "duel.h"

extern int duel_step;
extern unsigned short duel_w_0891968C;
extern unsigned short duel_h_0891968E;
extern unsigned int duel_turn_player;
extern int duel_w_089148FC;
extern int duel_w_08914900;
extern DuelCard duel_w_0891525C[];
extern int duel_dlg_cards;
extern int duel_player_type[2];
extern int duel_w_08919624;

int duel_088A9008(DuelAct *a, int i, unsigned int *loc);
int duel_0884E440(DuelAct *a, unsigned int p, int z);
int duel_slot_can_act(unsigned int p, unsigned int z, int x, int y);
unsigned short duel_slot_e1854(unsigned int p, int z);
void duel_088C8DC4(int loc, unsigned short t);
DuelCard *duel_field_card_deb2c(unsigned int p, int z, int i);
void duel_088CA384(unsigned int p, int i, int f);
void duel_088C78E0(unsigned int p);
void duel_08832894(unsigned int p, int a);
void duel_08832880();
unsigned int duel_0884E4E4(DuelAct *a);
unsigned int duel_088D1920(DuelAct *a, unsigned int mask);
int duel_0886FA90(DuelAct *a);
void duel_0884ACD8(unsigned int p, int msg);
void duel_0884C588(unsigned int p, int a, int id, int b);
void duel_088C6B24(unsigned int p);
void duel_088329E4(unsigned int p, int a, unsigned short *list, int n);
unsigned short *card_get_name(int id);
void duel_wfmt_s(unsigned short *dst, int fmt, unsigned short *arg);
DuelCard *duel_card_by_uid(int uid);
int duel_088CB554(unsigned int p, DuelCard *c);
void duel_088C625C(unsigned int p, int n, int x);
unsigned int duel_088D13CC(DuelAct *a, unsigned int mask);
void duel_088D1F1C(DuelAct *a, int i);
int duel_phase_ok2();
void duel_088C7B90(int a);
void duel_088C7BA4(void);
void duel_088C4508(unsigned int p, int a);
int duel_zone_faceup(unsigned int p, int z, int i);
void duel_088C7880(unsigned int p);
void duel_088CB980(unsigned int p);
int card_is_monster(int id);
int card_get_level(int id);
int duel_card_dfac8(unsigned int p, int id, int field);
int duel_count_free_monster_zones(unsigned int p);
void duel_card_copy(DuelCard *dst, DuelCard *src);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
int duel_pick_monster_zone(unsigned int p);
int duel_088CB55C(unsigned int p, DuelCard *c, int f);
void duel_088CB9E0(unsigned int p, unsigned int mask);
void duel_088C7910(unsigned int p);
void duel_088C7CF0(void);
void duel_088328FC(unsigned int p);
DuelCard *duel_0884C7CC(void);
void duel_0882619C(unsigned int p, DuelCard *c, int a, int b, int d);
int duel_088AA224(DuelAct *a);
int duel_no_1a1b(unsigned int p);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_088C5424(unsigned int p, int n);
void duel_0884AD40(int msg, int uid);
int duel_d_find_name(unsigned int p, int id);
int duel_08889680(DuelAct *a);
void duel_088C77D4(unsigned int p, int id, unsigned short a, unsigned short b);
int duel_grave_count_id(unsigned int p, int id);
void duel_08832624(unsigned int p, unsigned int n, int a, int b);
void duel_088C7E68(unsigned int p, int z, int a, int b);
void duel_088C5E70(unsigned int p, int a);
void duel_08832AF4(unsigned int p, int a, unsigned int x, unsigned int y);
int duel_rand(int n);
int duel_0884AF88(int msg);
int card_is_attribute9(int id);
int card_attribute_icon_is3(int id);
void duel_08832950(unsigned int p, int a);
void duel_08832960(unsigned int p, int a, int b);
void duel_088C7810(unsigned int p, unsigned int on);
int duel_0884E690(DuelAct *a, int (*fn)(int id));
int duel_08857148(int id);
void duel_088C9344(DuelAct *a, unsigned int p, int x, int y);
unsigned int duel_slot_e5a48(unsigned int p, int z);
void duel_088C67F0(unsigned int p, int id, int a, int b);
int duel_act_can_affect();
unsigned short field_get_card(unsigned int p, int z);
int duel_088D1684(DuelAct *a, unsigned int p, int z, int flags);
int card_get_special_group(int id);
int card_get_type(int id);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_act_target2(DuelAct *a, int i);
int duel_grave_find_uid(unsigned int p, int uid);
int duel_grave_card_usable(unsigned int p, unsigned int q, int i);
int duel_slot_counter(unsigned int p, int z, int id);
int card_attribute_icon_in_2_3(int id);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_088CB95C(unsigned int p, int z, int uid);
void duel_088C77EC(unsigned int p, int uid);
void duel_088C1B88(void);

#ifdef NON_MATCHING /* registradores */
int duel_08887618(DuelAct *a, DuelAct *b)
{
    unsigned int loc;
    int p;
    int z;

    if (duel_088A9008(a, 0, &loc) != 0) {
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel.players[p & 1].field[z].card.id != 0) {
            b->w[0] = a->w[0];
        }
    }
    return 0;
}
#endif

int duel_088876B0(DuelAct *a)
{
    unsigned int l1;
    unsigned int l0;
    unsigned int p;
    unsigned int z;
    unsigned int q;
    unsigned int y;

    if (a->b4_2) {
        return 0;
    }
    if (duel_088A9008(a, 0, &l0) != 0 && duel_088A9008(a, 1, &l1) != 0) {
        p = l0 & 0xFF;
        z = (unsigned short)l0 >> 8 & 0xFF;
        q = l1 & 0xFF;
        y = (unsigned short)l1 >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) == 0) {
            return 0;
        }
        if (duel_slot_can_act(p, z, q, y) == 0) {
            return 0;
        }
        if (l1 == duel_slot_e1854(p, z)) {
            return 0;
        }
        if (duel_slot_e1854(p, z) != 0xFFFF) {
            duel_088C8DC4((unsigned short)l0, l1);
        }
    }
    return 0;
}

int duel_088877D4(DuelAct *a)
{
    unsigned int p = a->w[0] & 1;
    int i;
    unsigned int o;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (p == a->player) {
            if (duel.players[a->player & 1].n_hand == 0) {
                return 0;
            }
            duel_08832894(a->player, a->id);
        } else {
            if (duel.players[1 - a->player & 1].n_hand == 0) {
                return 0;
            }
            duel_08832880(a->player, a->id, 1);
        }
        return 0x7F;
    case 0x7F:
        i = duel_w_0891968C;
        o = duel_field_card_deb2c(p, 0xB, i)->owner;
        duel_088CA384(p, i, 1);
        duel_088C78E0(o);
        return 100;
    }
    return 0;
}

int duel_08887910(DuelAct *a)
{
    duel_088D1920(a, duel_0884E4E4(a));
    return 0;
}

int duel_08887944(DuelAct *a)
{
    unsigned short ids[5];
    unsigned short buf[256];

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_0886FA90(a) == 0) {
            return 0;
        }
        duel_0884ACD8(a->player, 0x2B);
        return 0x7F;
    case 0x7F:
        duel_0884C588(a->player, 0xB, a->id, 0);
        return 0x7E;
    case 0x7E:
        duel_088C6B24(a->player);
        return 0x7D;
    case 0x7D:
        ids[0] = duel.players[a->player & 1].zone_d[0].id;
        ids[1] = duel.players[a->player & 1].zone_d[1].id;
        ids[2] = duel.players[a->player & 1].zone_d[2].id;
        ids[3] = duel.players[a->player & 1].zone_d[3].id;
        ids[4] = duel.players[a->player & 1].zone_d[4].id;
        duel_088329E4(1 - a->player, a->id, ids, 5);
        return 0x7C;
    case 0x7C:
        duel_wfmt_s(buf, 0x108, card_get_name(duel_w_0891968C));
        duel_0884ACD8(a->player, (int)buf);
        return 0x7B;
    case 0x7B:
        duel_088CB554(a->player, duel_card_by_uid(duel_h_0891968E));
        return 0x7A;
    case 0x7A:
        duel_088C625C(a->player, 4, 0);
        return 100;
    case 100:
        duel_088C78E0(a->player);
        return 99;
    }
    return 0;
}

int duel_08887ED8(DuelAct *a)
{
    unsigned int p = a->player ^ a->opp;
    int id;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_count_free_monster_zones(p) == 0) {
            return 0;
        }
        if (duel_phase_ok2(p) == 0) {
            return 0;
        }
        switch (a->id) {
        case 0x136A:
        case 0x1612:
            if (duel_no_1a1b(p) == 0) {
                return 0;
            }
        }
        if (duel_088BFF78(p, a->id, duel_088AA224(a)) == 0) {
            duel_0884ACD8(p, 0xD);
            return 0x7B;
        }
        return 0x7F;
    case 0x7F:
        switch (a->id) {
        case 0x1232:
        case 0x14BA:
        case 0x14DD:
        case 0x136A:
        case 0x15B6:
        case 0x194F:
            duel_dlg_cards = 0;
            return 0x7C;
        }
        duel_0884ACD8(p, 0x30);
        return 0x7E;
    case 0x7E:
        duel_0884C588(p, 6, a->id, duel_088AA224(a));
        return 0x7C;
    case 0x7C:
        id = a->id;
        switch (id) {
        case 0x1232:
            duel_088263A8(p, duel_0884C7CC(), 1, (unsigned short)id << 16 | 1);
            break;
        case 0x14BA:
        case 0x14DD:
        case 0x152F:
        case 0x1585:
        case 0x15B6:
        case 0x17E5:
        case 0x17E6:
        case 0x1900:
        case 0x194F:
        case 0x1A13:
        case 0x1A8F:
            duel_088263A8(p, duel_0884C7CC(), 1, (unsigned short)id << 16);
            break;
        case 0x136A:
        case 0x1612:
            duel_0882619C(p, duel_0884C7CC(), 0, 1, 0);
            break;
        default:
            duel_0882619C(p, duel_0884C7CC(), 1, 0, 0);
            break;
        }
        return 0x78;
    case 0x7B:
        duel_088328FC(1 - p);
        return 0x78;
    case 0x78:
        duel_088C78E0(p);
        return 100;
    }
    return 0;
}

#ifdef NON_MATCHING /* registradores no caso 0x7D, beqz no nível */
int duel_08887B34(DuelAct *a)
{
    int i;
    unsigned int p;
    unsigned int o;
    int id;
    DuelCard *c;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        duel_088D13CC(a, 0x1F001F);
        if (a->kind == 0x15) {
            duel_088D1F1C(a, 0);
            duel_088D1F1C(a, 1);
        }
        return 0x7F;
    case 0x7F:
        duel_w_089148FC = duel_turn_player;
        duel_w_08914900 = 5;
        a->w[1] = 0;
        a->w[0] = 0;
        a->w[2] = duel_phase_ok2(0);
        a->w[3] = duel_phase_ok2(1);
        duel_088C7B90(0x1E);
        duel_088C7BA4();
        return 0x7E;
    case 0x7E:
        p = duel_w_089148FC;
        if (duel.players[p & 1].n_d != 0) {
        c = &duel.players[p & 1].zone_d[0];
        o = c->owner;
        id = c->id;
        duel_088C4508(p, id);
        if (duel_zone_faceup(p, 0xD, 0) == 0) {
            duel_088C7880(p);
        }
        duel_088CB980(p);
        if (card_is_monster(id) != 0) {
        if (card_get_level(id) < 5 && duel_card_dfac8(p, id, 0) != 0) {
            if (duel_count_free_monster_zones(o) > 0 && a->w[2 + o] != 0) {
                duel_card_copy(duel_w_0891525C, c);
                return 0x7D;
            }
            duel_088C625C(p, 1, 0);
            return 0x7A;
        } else {
            return 0x7C;
        }
        }
        return 0x7C;
        }
        return 0x7A;
    case 0x7D:
        o = duel_w_0891525C->owner;
        duel_088263A8(o, duel_w_0891525C, 0, 0);
        a->w[o] |= 1 << duel_pick_monster_zone(o);
        return 0x7A;
    case 0x7C:
        duel_088CB55C(duel_w_089148FC, &duel.players[duel_w_089148FC & 1].zone_d[0], 0);
        return 0x7A;
    case 0x7A:
        if (--duel_w_08914900 > 0) {
            return 0x7E;
        }
        duel_w_089148FC = 1 - duel_w_089148FC;
        duel_w_08914900 = 5;
        if (duel_w_089148FC != duel_turn_player) {
            return 0x7E;
        }
        return 0x78;
    case 0x78:
        for (i = 0; i < 2; i++) {
            duel_088CB9E0(duel_turn_player ^ i, a->w[duel_turn_player ^ i]);
        }
        for (i = 0; i < 2; i++) {
            duel_088C7910(duel_turn_player ^ i);
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}
#endif

int duel_08888224(DuelAct *a)
{
    unsigned int p;

    switch (duel_step) {
    case 0x80:
        switch (a->id) {
        case 0x1339:
            duel_088C4604(a, 1 - a->player, 500);
            break;
        case 0x133A:
            duel_088C5424(a->player, 1000);
            break;
        }
        return 0x7F;
    case 0x7F:
        duel_w_089148FC = 0;
        if (duel_count_free_monster_zones(a->player) == 0) {
            return 0;
        }
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        switch (a->id) {
        case 0x133A:
            if (duel_no_1a1b(a->player) == 0) {
                return 0;
            }
            break;
        }
        if (duel_088BFF78(a->player, a->id, 0) == 0) {
            return 0;
        }
    case 0x7E:
        if (duel_count_free_monster_zones(a->player) == 0 || duel_088BFF78(a->player, a->id, 0) == 0) {
            return 100;
        }
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 1;
        } else {
            switch (a->id) {
            case 0x1339:
                duel_0884AD40(0x35, a->uid);
                break;
            case 0x133A:
                duel_0884AD40(0x34, a->uid);
                break;
            }
        }
        return 0x7D;
    case 0x7D:
        if (duel_w_08919624 == 0) {
            return 100;
        }
        if (duel_w_089148FC++ == 0) {
            duel_088C7BA4();
        }
        switch (a->id) {
        case 0x1339:
            p = a->player;
            duel_0882619C(p, &duel.players[p & 1].zone_d[duel_d_find_name(p, a->id)], 1, 0, 0);
            break;
        case 0x133A:
            p = a->player;
            duel_0882619C(p, &duel.players[p & 1].zone_d[duel_d_find_name(p, a->id)], 0, 1, 0);
            break;
        }
        return 0x7E;
    case 100:
        duel_088C78E0(a->player);
        if (duel_w_089148FC != 0) {
            duel_088C7CF0();
        }
        return 0;
    }
    return 0;
}

int duel_08888548(DuelAct *a)
{
    if (a->b3_4 != 2) {
        return 0;
    }
    return duel_08889680(a);
}

int duel_08888584(DuelAct *a)
{
    duel_088C77D4(0, a->id, 1, 0);
    return 0;
}

int duel_088885B0(DuelAct *a)
{
    int i;

    if (a->b4_2) {
        return 0;
    }
    switch (a->id) {
    case 0x1352:
        duel_088C5424(a->player, 1000);
        for (i = 0; i < duel_grave_count_id(a->player, a->id); i++) {
            duel_088C5424(a->player, 500);
        }
        break;
    case 0x135A:
        duel_088C4604(a, 1 - a->player, 700);
        for (i = 0; i < duel_grave_count_id(a->player, a->id); i++) {
            duel_088C4604(a, 1 - a->player, 300);
        }
        break;
    }
    return 0;
}

int duel_088886E4(DuelAct *a)
{
    unsigned int p;
    unsigned int n;

    switch (duel_step) {
    case 0x80:
        duel_088C7E68(a->player, a->zone, 4, 0);
        return 0x7F;
    case 0x7F:
        p = a->player;
        n = duel.players[p & 1].field[a->zone].link + 1;
        duel_08832624(1 - p, MIN(n, duel.players[1 - p & 1].n_hand), 0, 1);
        return 0;
    }
    return 0;
}

#ifdef NON_MATCHING /* b; nop para o return 10 comum */
int duel_088887C0(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = duel_rand(2);
        } else {
            duel_0884AF88(0x38);
        }
        return 0x7F;
    case 0x7F:
        duel_08832AF4(a->player, a->zone, 1, duel_w_08919624);
        return 0x7E;
    case 0x7E:
        if (duel_h_0891968E != 0) {
            if (duel.players[a->player & 1].n_hand < 5) {
                duel_088C5E70(a->player, 5 - duel.players[a->player & 1].n_hand);
            } else {
                return 10;
            }
        } else {
            duel_088C77D4(a->player, a->id, 0x21, 0);
        }
        return 10;
    }
    return 0;
}
#endif

int duel_088888F8(DuelAct *a)
{
    unsigned int p;
    unsigned int mask;
    int i;
    int z;
    int id;

    p = duel_turn_player;
    p ^= duel_w_089148FC;
    switch (duel_step) {
    case 0x80:
        if (a->b4_2) {
            return 0;
        }
        a->w[1] = 0;
        a->w[0] = 0;
        a->w[3] = 0;
        a->w[2] = 0;
        mask = duel_088D1920(a, 0x1F001F);
        for (i = 0; i < 2; i++) {
            for (z = 0; z < 5; z++) {
                if (mask & 1 << (z + i * 16)) {
                    id = duel.players[i & 1].field[z].card.id;
                    if (card_is_monster(id) != 0 && card_is_attribute9(id) == 0 && card_attribute_icon_is3(id) == 0) {
                        a->w[duel.players[i & 1].field[z].card.owner]++;
                    }
                }
            }
        }
        return 0x7F;
    case 0x7F:
        duel_w_089148FC = 0;
        duel_w_08914900 = duel_phase_ok2(0) | duel_no_1a1b(0);
        duel_w_08914900 |= (duel_phase_ok2(1) | duel_no_1a1b(1)) << 1;
        duel_088C7BA4();
        return 0x7E;
    case 0x7E:
        if (a->w[p] > 0 && duel_count_free_monster_zones(p) != 0 && (duel_w_08914900 >> p & 1)) {
            for (i = 0; i < duel.players[p & 1].n_hand; i++) {
                if (!(!duel_088BFF78(p, a->id, i))) {
                    return 0x7D;
                }
            }
            duel_088C7810(p, 1);
            return 0x79;
        }
        return 0x78;
    case 0x7D:
        duel_08832950(p, a->id);
        return 0x7C;
    case 0x7C:
        for (i = 0; i < duel.players[p & 1].n_hand; i++) {
            if (!(!duel_088BFF78(p, a->id, i))) {
                duel_08832894(p, a->id);
                return 0x7B;
            }
        }
        duel_088C7810(p, 1);
        return 0x79;
    case 0x7B:
        duel_0882619C(p, &duel.players[p & 1].hand[duel_w_0891968C], 0, 1, 0);
        a->w[2 + p] |= 1 << duel_pick_monster_zone(p);
        if (--a->w[p] != 0) {
            return 0x7C;
        }
        return 0x78;
    case 0x79:
        duel_08832960(1 - p, a->id, 1);
        return 0x78;
    case 0x78:
        duel_088C7810(p, 0);
        if (++duel_w_089148FC < 2) {
            return 0x7E;
        }
        return 0x77;
    case 0x77:
        for (i = 0; i < 2; i++) {
            duel_088CB9E0(duel_turn_player ^ i, a->w[2 + (duel_turn_player ^ i)]);
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_08888D60(DuelAct *a)
{
    unsigned int q;
    int z;
    int i;
    int n = duel_0884E690(a, duel_08857148);

    for (i = 0; i < 2; i++) {
        q = duel_turn_player ^ i;
        for (z = 0; z < 5; z++) {
            if (duel.players[q & 1].field[z].card.id != 0 && duel.players[q & 1].field[z].faceup == 0 && duel.players[q & 1].field[z].pos != 0) {
                duel_088C9344(a, q, z, 2);
                if (duel_slot_e5a48(q, z) != 0) {
                    n++;
                }
            }
        }
    }
    switch (a->id) {
    case 0x135C:
        duel_088C4604(a, 1 - a->player, n * 500);
        break;
    case 0x1635:
        duel_088C5424(a->player, n * 400);
        break;
    }
    return 0;
}

#ifdef NON_MATCHING /* ordem dos argumentos de duel_088C67F0 */
int duel_08889434(DuelAct *a)
{
    unsigned int loc;
    unsigned int p;
    unsigned int z;
    int id;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        if (duel_088A9008(a, 0, &loc) == 0) {
            return 0;
        }
        p = loc & 0xFF;
        z = (unsigned short)loc >> 8 & 0xFF;
        if (duel_0884E440(a, p, z) == 0) {
            return 0;
        }
        if (duel_act_can_affect(a, p, z, 1) == 0) {
            return 0;
        }
        id = a->w[1] = field_get_card(p, z);
        if (duel_088D1684(a, p, z, 2) == 0) {
            return 0;
        }
        duel_088C7B90(0x1E);
        switch (a->id) {
        case 0x1363:
            if (card_is_monster(id) != 0 && card_get_special_group(id) == 1) {
                return 0x7F;
            }
            return 0;
        case 0x1364:
            if (card_get_type(id) == 0x17) {
                return 0x7F;
            }
            return 0;
        }
        return 0;
    case 0x7F:
        duel_088C67F0(a->player, a->w[1], 1, 0);
        return 0x7E;
    case 0x7E:
        duel_088C67F0(1 - a->player, a->w[1], 1, 1);
        return 0x7D;
    case 0x7D:
        duel_088328FC(a->player);
        return 0x7C;
    case 0x7C:
        duel_088328FC(1 - a->player);
        return 0x7B;
    case 0x7B:
        duel_088C78E0(a->player);
        return 0x7A;
    case 0x7A:
        duel_088C78E0(1 - a->player);
        return 1;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* constante 100 em v0 reaproveitada */
int duel_08889680(DuelAct *a)
{
    unsigned int t = duel_act_target(a, duel_w_089148FC);
    unsigned int p = t & 0xFF;
    int i;
    DuelCard *c;

    if (a->b4_2) {
        return 0;
    }
    switch (duel_step) {
    case 0x80:
        duel_w_089148FC = 0;
        duel_w_08914900 = duel_phase_ok2(0) | duel_phase_ok2(1) << 1;
        duel_088C7BA4();
        return 0x7F;
    case 0x7F:
        if (a->n7 <= duel_w_089148FC) {
            return 0x7E;
        }
        if (duel_count_free_monster_zones(p) != 0 && (duel_w_08914900 >> p & 1)) {
            i = duel_grave_find_uid(p, duel_act_target2(a, duel_w_089148FC));
            if (i >= 0 && duel_grave_card_usable(p, p, i) != 0) {
                c = &duel.players[p & 1].grave[i];
                switch (a->id) {
                case 0x133B:
                    if (c->id == a->id) {
                        if (duel_slot_counter(p, 0xB, a->id) >= 10) {
                            break;
                        }
                        duel_088C77D4(p, a->id, 1, 0);
                    }
                    duel_088263A8(p, c, card_attribute_icon_in_2_3(c->id) != 0, a->id << 16 | 0x20);
                    break;
                case 0x1365:
                    if (duel_no_1a1b(p) != 0) {
                        duel_0882619C(p, c, 0, 1, a->id << 16 | 0x20);
                    }
                    break;
                case 0x1990:
                    duel_0882619C(p, c, 1, 1, a->id << 16 | 0x20);
                    break;
                }
            }
        }
        return 0x7E;
    case 0x7E:
        if (++duel_w_089148FC < 2) {
            return 0x7F;
        }
        return 100;
    case 100:
        duel_088C7CF0();
        return 0;
    }
    return 0;
}
#endif

int duel_08889914(DuelAct *a)
{
    int i;
    int id;

    if (a->b4_2) {
        return 0;
    }
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    if (duel.players[1 - a->player & 1].n_hand != 0) {
        i = duel_rand(duel.players[1 - a->player & 1].n_hand);
        id = duel.players[1 - a->player & 1].hand[i].id;
        duel_cmd((1 - a->player ? 0x8000 : 0) | 0x53, 0, 0, 0);
        duel_088CB95C(1 - a->player, 0xB, CARD_UID(duel.players[1 - a->player & 1].hand[i]));
        duel_088C4508(1 - a->player, id);
        duel_088C77EC(1 - a->player, CARD_UID(duel.players[1 - a->player & 1].hand[i]));
    }
    return 0;
}

#ifdef NON_MATCHING /* zona com andi 0xFF */
int duel_08889B30(DuelAct *a)
{
    if (a->b4_2) {
        return 0;
    }
    if (a->uid != CARD_UID(duel.players[a->player & 1].field[a->zone].card)) {
        return 0;
    }
    duel_088C1B88();
    return 0;
}
#endif
