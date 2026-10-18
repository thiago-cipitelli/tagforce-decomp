// @range 0x088FAEFC-0x088FB5E4
// @asm 0x088FAEFC-0x088FB168
/* Card effect handlers (continued). */
#include "duel.h"
#include "card_db.h"

extern int duel_step;
extern int duel_w_08919624;

int duel_count_active(int id);
int duel_count_free_monster_zones(unsigned int p);
int duel_phase_ok2(unsigned int p);
int duel_find_effect_card(unsigned int p, int id, int z);
unsigned int duel_act_target(DuelAct *a, int i);
unsigned int duel_field_controller(unsigned int p, int z, int arg);
int duel_field_base_atk(unsigned int p, int z);
int duel_088A9008(DuelAct *a, int b, int c);
int duel_0884E440();
int duel_088D16C4(DuelAct *a, unsigned int p, int z, int b);
void duel_088C77D4(unsigned int p, int id, int a, unsigned short b);
int duel_088C4604(DuelAct *a, unsigned int p, int n);
void duel_088C7BA4(void);
void duel_088C7CF0(void);
unsigned int duel_0884E538(DuelAct *a, int (*fn)());
int duel_08855908();
void duel_088D13CC(DuelAct *a, unsigned int mask);

#ifdef NON_MATCHING /* close; the operand order of uid * 8 + controller * 4 and the last return differ */
int duel_eff_faefc(DuelAct *a)
{
    if (a->b3_4 == 3) {
        unsigned char z = a->zone;
        unsigned int p = a->player;

        if (a->uid != CARD_UID(duel.players[p & 1].field[z].card)) {
            return 0;
        }
        if (duel.players[p & 1].field[z].faceup == 0) {
            return 0;
        }
        if (duel.players[p & 1].field[z].pos != 0) {
            return 0;
        }
        if (duel_088A9008(a, 0, 0) != 0) {
            unsigned int t = duel_act_target(a, 0);
            unsigned int q = t & 0xFF;
            unsigned int qz = (unsigned char)((t & 0xFFFF) >> 8);
            int uid;

            if (duel_0884E440(a, q, qz) != 0) {
                uid = CARD_UID(duel.players[q & 1].field[qz].card);
                if (duel_088D16C4(a, q, qz, 0x2000) == 0) {
                    return 0;
                }
                if (card_is_attribute9(duel.players[q & 1].field[qz].card.id) == 0) {
                    int pos = duel.players[q & 1].field[qz].pos;
                    int up = duel.players[q & 1].field[qz].faceup;

                    duel_088C77D4(a->player, a->id, 0x41, up + (pos * 2 + (uid * 8 + duel_field_controller(q, qz, 1) * 4)));
                    duel_088C77D4(a->player, a->id, 2, 0);
                    return 0;
                }
            }
        }
    }
    return 0;
}
#endif

int duel_df180(unsigned int p, int z, int a);
void duel_088C7880(unsigned int p);
void duel_088CB980(unsigned int p);
void duel_088C625C(unsigned int p, int a, int b);
int duel_088DFCC8(unsigned int uid);
void duel_cmd(unsigned short cmd, int p, unsigned short id, int arg);
DuelCard *duel_0884CCBC(int i);
void duel_088263A8(unsigned int p, DuelCard *c, int a, unsigned int b);
void duel_0884C588(unsigned int p, int a, unsigned short id, unsigned int b);
int duel_088BFF78(unsigned int p, unsigned short id, int b);
void duel_0884AD18(int msg);
int duel_0885B2F8();

int duel_eff_fb168(DuelAct *a)
{
    unsigned int p = a->player;

    if (duel.players[p & 1].n_d != 0) {
        int id = duel.players[p & 1].zone_d[0].id;
        int uid = CARD_UID(duel.players[p & 1].zone_d[0]);

        if (duel_df180(p, 0xD, 0) == 0) {
            duel_088C7880(a->player);
        }
        duel_088CB980(a->player);
        if (card_get_type(id) == 0x16 && card_get_prop5_hi(id) == 0) {
            duel_088C625C(a->player, 1, 1);
            if (duel_088DFCC8(uid)) {
                duel_088C77D4(a->player, a->id, 0x21, uid);
            }
        } else {
            duel_cmd((a->player ? 0x8000 : 0) | 0x57, 0xD, uid, 1);
        }
    }
    return 0;
}

int duel_eff_fb2c0(DuelAct *a)
{
    int n;

    switch (duel_step) {
    case 0x80:
        duel_088D13CC(a, duel_0884E538(a, duel_0885B2F8));
        return 0x7F;
    case 0x7F:
        if (duel_count_free_monster_zones(a->player) == 0) {
            return 0;
        }
        if (duel_phase_ok2(a->player) == 0) {
            return 0;
        }
        if (duel_088BFF78(a->player, a->id, 0) == 0) {
            return 0;
        }
        if (duel_count_active(0x159D) == 0 || duel_find_effect_card(a->player, 0x158B, -1)) {
            duel_0884AD18(0x2F);
            return 0x7E;
        }
        return 0;
    case 0x7E:
        if (duel_w_08919624 == 0) {
            return 0;
        }
        n = duel_count_free_monster_zones(a->player);
        n = MIN(n, 2);
        n = MIN(n, 32);
        duel_0884C588(a->player, n + 0x27, a->id, 0);
        return 0x7D;
    case 0x7D:
        a->w[0] = 0;
        a->w[1] = duel_w_08919624;
        duel_088C7BA4();
        return 0x7C;
    case 0x7C:
        if (a->w[0] < a->w[1]) {
            duel_088263A8(a->player, duel_0884CCBC(a->w[0] + 1), 1, a->id << 16);
            a->w[0]++;
            return 0x7C;
        }
        duel_088C7CF0();
        return 0;
    }
    return 0;
}

int duel_eff_fb4a0(DuelAct *a)
{
    if (duel_088A9008(a, 0, 0)) {
        unsigned int t = duel_act_target(a, 0);
        unsigned int q = t & 0xFF;
        unsigned int z = (unsigned char)((t & 0xFFFF) >> 8);

        if (duel_0884E440(a, q, z)) {
            duel_088C4604(a, 1 - a->player, duel_field_base_atk(q, z) / 2);
        }
    }
    if (a->kind != 0x23) {
        duel_088C77D4(a->player, a->id, 1, 0);
    }
    return 0;
}

int duel_eff_fb57c(DuelAct *a)
{
    duel_088C7BA4();
    duel_088C4604(a, 1 - a->player, 300);
    duel_088D13CC(a, duel_0884E538(a, duel_08855908));
    duel_088C7CF0();
    return 0;
}
