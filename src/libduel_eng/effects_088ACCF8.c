// @range 0x088ACCF8-0x088AE378
// @data 0x089119D8-0x089119F0
// @asm 0x088ACE70-0x088AD128
// @asm 0x088AD368-0x088AD47C
// @asm 0x088AD8BC-0x088ADC34
// @asm 0x088ADF28-0x088ADFC0
// @asm 0x088AE1F8-0x088AE314
/* Card effect handlers (DuelAct *a): target selection on duel_w_08914908. */
#include "duel.h"

extern int duel_w_08914908;
extern int duel_w_08919624;
extern int duel_player_type[2];
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern unsigned short duel_w_0891968C;
extern unsigned short duel_h_0891968E;

int duel_slot_effect_arg(unsigned int p, int z, int id);
unsigned int duel_card_locate(unsigned int uid);
void duel_088A8B94(DuelAct *a, unsigned short loc);
void duel_088A8D40(DuelAct *a, unsigned int p, int z);
void duel_088C89BC(unsigned int p, int z, int id, int c);
void duel_0884AD40(int msg, int uid);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
int duel_08835AC0(void);
void duel_088359E8(unsigned int p, int id, int (*fn)(DuelAct *a, unsigned int p, int z));
void duel_08835A54(unsigned int p, unsigned short id, int (*fn)(DuelAct *a, unsigned int p, int z));
void duel_0884ACD8(unsigned int p, void *msg);
int duel_08871868(DuelAct *a);
int duel_08815950(DuelAct *a, unsigned int p, int atk, int def, int z);
int duel_zone_free(unsigned int p, int z);
void duel_088CB938(unsigned int p, int a);
void duel_wfmt_d(char *dst, char *fmt, int n);
int duel_list_counter(int uid, int id);
int duel_088AA1C0(DuelAct *a, int b);
void duel_08832960(unsigned int p, unsigned short id, int a);
unsigned int duel_act_target(DuelAct *a, int i);
int duel_field_kind(unsigned int p, int z);
void duel_08832988(unsigned int p, unsigned short id, unsigned short n, int a);
int duel_088AA5F4(DuelAct *a);
int duel_0884E5E0(DuelAct *a);
void duel_088A9360(DuelAct *a, int b);
int duel_08817598(DuelAct *a, int b, int id);
void duel_0884AD18(int msg);
int duel_088AA844(DuelAct *a, unsigned int p, int z);

int duel_088ACCF8(DuelAct *a)
{
    int t;
    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        t = duel_slot_effect_arg(a->player, 0xB, 0x1415);
        if (t >= 0) {
            duel_card_locate(t);
            duel_088A8B94(a, t);
            duel_088C89BC(a->player, 0xB, a->id, 0);
            if (duel_player_type[a->player] == 1) {
                duel_w_08919624 = 1;
            } else {
                duel_0884AD40(0x35, t);
            }
            duel_w_08914908++;
            return 0;
        }
        return -1;
    case 1:
        if (duel_w_08919624 != 0) {
            return 1;
        }
        return -1;
    }
    return 1;
}


int duel_088ACDF8(DuelAct *a, unsigned int p, int z)
{
    if (duel_0884E3E4(a, p, z) == 0) {
        return 0;
    }
    switch (a->n7) {
    case 0:
        return p != a->player;
    case 1:
        return p == a->player;
    }
    return 1;
}

#ifdef NON_MATCHING /* m[1] em $a1 (original: $a2 com move antes da chamada) */
int duel_088ACE70(DuelAct *a)
{
    int m[2];
    int z;

    switch (a->id) {
    case 0x1430:
        m[0] = 0x94;
        m[1] = 0x95;
        break;
    case 0x14B0:
        m[0] = 100;
        m[1] = 0x7C;
        break;
    case 0x1668:
        m[0] = 100;
        m[1] = 0x7C;
        break;
    }
    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        if (duel_08871868(a) == 0) {
            return 1;
        }
        if (duel_player_type[a->player] == 1) {
            z = duel_08815950(a, 1 - a->player, 1, 1, -1);
            if (z < 0) {
                for (z = 0; z <= 4; z++) {
                    if (duel_0884E3E4(a, 1 - a->player, z)) {
                        break;
                    }
                }
            }
            duel_088A8D40(a, 1 - a->player, z);
            z = duel_08815950(a, a->player, -1, -1, -1);
            if (z < 0) {
                for (z = 0; z <= 4; z++) {
                    if (duel_0884E3E4(a, a->player, z)) {
                        break;
                    }
                }
            }
            duel_088A8D40(a, a->player, z);
            return 1;
        }
        duel_0884ACD8(a->player, (void *)m[0]);
        duel_088359E8(a->player, a->id, duel_088ACDF8);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_0884ACD8(a->player, (void *)m[1]);
        duel_088359E8(a->player, a->id, duel_088ACDF8);
        duel_w_08914908++;
        return 0;
    case 1:
    case 3:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            duel_w_08914908++;
            return 0;
        }
        duel_w_08914908--;
        return 0;
    }
    return 1;
}
#endif

int duel_088AD128(DuelAct *a, int p, int z)
{
    if (z > 4) {
        return 0;
    }
    if (duel_zone_free(p, z) == 0) {
        return 0;
    }
    if (a->n7 != 0) {
        if (a->w[(a->n7 & 0xFF) - 1] == (unsigned short)((p & 0xFF) | (z & 0xFF) << 8)) {
            return 0;
        }
    }
    return 1;
}

int duel_088AD1CC(DuelAct *a)
{
    char buf[512];
    unsigned int p;
    int z;

    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        duel_wfmt_d(buf, (char *)0x9B, 2);
        duel_0884ACD8(a->player, buf);
    case 2:
        duel_08835A54(a->player, a->id, (void *)duel_088AD128);
        duel_w_08914908++;
        return 0;
    case 1:
    case 3:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_088CB938(p, z);
            duel_088A8B94(a, (unsigned char)p | (unsigned char)z << 8);
            duel_w_08914908++;
            return 0;
        }
        duel_w_08914908--;
        return 0;
    }
    return 1;
}

int duel_088AD30C(DuelAct *a, int b)
{
    if (duel_list_counter(a->uid, a->id) < 3) {
        return 1;
    }
    return duel_088AA1C0(a, b);
}

#ifdef NON_MATCHING /* o original assume que duel_088A8B94 preserva $a1 (mesma unidade de compilação?) */
int duel_088AD368(DuelAct *a)
{
    unsigned int t;

    switch (duel_w_08914908) {
    case 0:
    case 1:
        if (duel_088AA5F4(a) != 0) {
            duel_w_08914908 = 2;
        }
        return 0;
    case 2:
        t = duel_act_target(a, 0);
        t = duel_field_kind(t & 0xFF, (unsigned short)t >> 8 & 0xFF);
        duel_08832988(a->player, a->id, 1, 1 << t ^ 0xFF);
        duel_w_08914908++;
        return 0;
    case 3:
        t = duel_h_0891968E;
        duel_088A8B94(a, t);
        duel_08832960(1 - a->player, 0x171F, t);
        duel_w_08914908++;
        return 0;
    }
    return 1;
}
#endif

int duel_088AD47C(DuelAct *a, int b)
{
    switch (duel_w_08914908) {
    case 0:
        duel_088A8D40(a, duel_special.player[0], duel_special.zone[0]);
        if (duel_player_type[a->player] == 1) {
            return duel_08817598(a, b, a->id);
        }
        duel_w_08914908++;
        return 0;
    case 1:
        if (duel_0884E5E0(a) == 0) {
            return 1;
        }
        duel_088A9360(a, b);
        duel_088359E8(a->player, a->id, duel_0884E3E4);
        duel_w_08914908++;
        return 0;
    case 2:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            duel_w_08914908++;
            return 0;
        }
        duel_w_08914908--;
        return 0;
    }
    return 1;
}

int duel_088AD5F4(DuelAct *a)
{
    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        if (duel_0884E5E0(a) == 0) {
            return -1;
        }
        switch (a->id) {
        case 0x140A:
        case 0x148A:
            duel_0884ACD8(a->player, (void *)0xF);
            duel_w_08919624 = 1;
            break;
        case 0x1476:
            if (duel_player_type[a->player] == 1) {
                duel_w_08919624 = 1;
            } else {
                duel_0884AD18(0xE2);
            }
            break;
        }
        duel_w_08914908++;
        return 0;
    case 1:
        if (duel_w_08919624 == 0) {
            return -1;
        }
        duel_088359E8(a->player, a->id, duel_0884E3E4);
        duel_w_08914908++;
        return 0;
    case 2:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            duel_w_08914908++;
            return 0;
        }
        duel_w_08914908 = 0;
        return 0;
    }
    return 1;
}

int duel_088AD794(DuelAct *a)
{
    char buf[512];

    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        if (duel_0884E5E0(a) < 2) {
            return 1;
        }
        duel_wfmt_d(buf, (char *)0xA1, 2);
        duel_0884ACD8(a->player, buf);
    case 2:
        duel_08835A54(a->player, a->id, duel_088AA844);
        duel_w_08914908++;
        return 0;
    case 1:
    case 3:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            duel_w_08914908++;
            return 0;
        }
        duel_w_08914908--;
        return 0;
    }
    return 1;
}

void duel_field_stats(unsigned int p, int z, DuelStats *st);
void duel_088C4564(unsigned int p, int n);
void duel_08832B40(unsigned int p, int a, int b);

#ifdef NON_MATCHING /* ordem do andi 0xff do alvo antes do ext */
int duel_088AD8BC(DuelAct *a)
{
    DuelStats st;
    unsigned int t;
    int p;
    int z;
    int v;
    unsigned int q;

    switch (duel_w_08914908) {
    case 0:
        for (p = 0; p < 2; p++) {
            for (z = 0; z <= 4; z++) {
                if (!duel_0884E3E4(a, p, z)) {
                    continue;
                }
                duel_088A8D40(a, p, z);
                duel_w_08914908 = 10;
                return 0;
            }
        }
        return 0;
    case 10:
        t = duel_act_target(a, 0);
        z = (unsigned short)t >> 8 & 0xFF;
        duel_field_stats(t & 0xFF, z, &st);
        v = st.atk > st.def ? st.atk : st.def;
        q = a->player;
        v = v < duel.players[q & 1].lp - 1 ? v : duel.players[q & 1].lp - 1;
        v = v < 0xFFFF ? v : 0xFFFF;
        if (v / 100 == 0) {
            a->w[2] = 0;
            return 1;
        }
        duel_08832B40(q, a->id, v / 100);
        duel_w_08914908++;
        return 0;
    case 11:
        t = duel_w_0891968C;
        duel_088C4564(a->player, t * 100);
        a->w[2] = t * 100;
        return 1;
    }
    return 0;
}
#endif


void duel_088A8EA0(DuelAct *a, unsigned int p, int z, int uid);
int duel_088DFE24(unsigned int p, unsigned int q);
int duel_088BFFD8(unsigned int p, int id, int i);
void duel_088329D0(unsigned int p, int a, int b);
int duel_count_type(unsigned int p, unsigned int type);
int duel_088AC0E0(DuelAct *a, int b);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_0884C588(unsigned int p, int a, int id, int b);
DuelCard *duel_0884C7CC(void);
void duel_08832974(unsigned int p, unsigned short id, int a);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
void duel_088A8BB0(DuelAct *a, unsigned int p, int z);

#ifdef NON_MATCHING /* despacho do switch: constante 1 em \$v1 e retorno 0 padrão no despacho */
int duel_088ADA84(DuelAct *a)
{
    if (a->w[2] == 0) {
        return 1;
    }
    switch (duel_w_08914908) {
    case 0:
        if (duel_088DFE24(a->player, 1 - a->player) && duel_088BFFD8(a->player, a->id, 0)) {
            duel_088329D0(a->player, a->id, 0);
            duel_w_08914908++;
            return 0;
        }
        duel_w_08914908 += 2;
    case 2:
        if (duel_088DFE24(a->player, a->player) && duel_088BFFD8(1 - a->player, a->id, 0)) {
            duel_088329D0(1 - a->player, a->id, 0);
            duel_w_08914908++;
            return 0;
        }
        duel_w_08914908 += 2;
        return 1;
    case 4:
        return 1;
    case 1:
    case 3:
        duel_088A8EA0(a, duel_h_0891968E & 1, 0xE, duel_h_0891968E);
        duel_w_08914908++;
        return 0;
    }
    return 0;
}
#endif

int duel_088ADC34(DuelAct *a)
{
    switch (duel_w_08914908) {
    case 0:
        if (duel_0884E5E0(a) == 0) {
            return 1;
        }
        duel_0884ACD8(a->player, (void *)0x6F);
        duel_088359E8(a->player, a->id, duel_088AA844);
        duel_w_08914908++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            duel_w_08914908++;
            return 0;
        }
        duel_w_08914908--;
        return 0;
    case 2:
        if (duel_0884E5E0(a) < 2) {
            return 1;
        }
        if (duel_count_type(a->player, 0xF) == 0) {
            return 1;
        }
        duel_0884AD18(0x75);
        duel_w_08914908++;
        return 0;
    case 3:
        if (duel_w_08919624 == 0) {
            return 1;
        }
        duel_088359E8(a->player, a->id, duel_088AA844);
        duel_w_08914908++;
        return 0;
    case 4:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            return 1;
        }
        duel_w_08914908 -= 2;
        return 0;
    }
    return 0;
}

int duel_088ADE48(DuelAct *a, int b)
{
    if (a->kind != 0xD) {
        return duel_088AC0E0(a, b);
    }
    return 1;
}

int duel_088ADE7C(DuelAct *a, int b)
{
    if (a->w[2] != 2) {
        return 1;
    }
    return duel_088AA1C0(a, b);
}

int duel_088ADEB0(DuelAct *a, int b)
{
    if (a->b3_4 == 1 && a->kind == 5) {
        return 1;
    }
    switch (a->id) {
    case 0x1503:
    case 0x1694:
        return duel_088AA1C0(a, b);
    }
    return 1;
}

#ifdef NON_MATCHING /* alocação de registradores (a/p/z) */
void duel_088ADF28(DuelAct *a)
{
    unsigned int p;
    int z;

    for (p = 0; (int)p < 2; p++) {
        for (z = 0; z <= 4; z++) {
            if (duel_0884E3E4(a, p, z) != 0) {
                duel_088C87A8(p, z, 0x1518, 2, a->uid);
            }
        }
    }
    a->w[0] = 1;
}
#endif

int duel_088ADFC0(DuelAct *a)
{
    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        if (duel_088BFFD8(a->player, a->id, 0) == 0) {
            return -1;
        }
        if (duel_088BFFD8(a->player, a->id, 1) == 0) {
            return -1;
        }
        duel_0884ACD8(a->player, (void *)0x5E);
        duel_w_08914908++;
        return 0;
    case 1:
        duel_0884C588(a->player, 6, a->id, 0);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_088A8EA0(a, duel_0884C7CC()->owner, 0xE, CARD_UID(*duel_0884C7CC()));
        duel_w_08914908++;
        return 0;
    case 3:
        duel_0884ACD8(a->player, (void *)0x56);
        duel_w_08914908++;
        return 0;
    case 4:
        duel_0884C588(a->player, 6, a->id, 1);
        duel_w_08914908++;
        return 0;
    case 5:
        duel_088A8EA0(a, duel_0884C7CC()->owner, 0xE, CARD_UID(*duel_0884C7CC()));
        duel_w_08914908++;
        return 0;
    }
    return 1;
}

#ifdef NON_MATCHING /* o original assume que duel_088A8B94 preserva \$a0 */
int duel_088AE1F8(DuelAct *a)
{
    switch (duel_w_08914908) {
    case 0:
        if (duel_player_type[a->player] == 1) {
            if (duel_special.player[1] == a->player) {
                duel_w_08919624 = 0;
            } else {
                duel_w_08919624 = duel.players[duel_special.player[1] & 1].field[duel_special.zone[1]].pos;
            }
        } else {
            duel_0884ADAC(0x7F, 0x198, 0x199, 0, 0, 0xF);
        }
        duel_w_08914908++;
        return 0;
    case 1:
        duel_088A8B94(a, duel_w_08919624 + 1);
        duel_08832974(1 - a->player, a->id, duel_w_08919624);
        return 1;
    }
    return 0;
}
#endif

int duel_088AE314(DuelAct *a)
{
    if (a->zone < 5) {
        return duel_088AA5F4(a);
    }
    return 1;
}

int duel_088AE348(DuelAct *a)
{
    duel_088A8BB0(a, a->src.player, a->src.zone);
    return 1;
}
