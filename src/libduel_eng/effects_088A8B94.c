// @range 0x088A8B94-0x088AF390
// @data 0x089119A8-0x08911A20
// @asm 0x088A9174-0x088A92E0
// @asm 0x088ABE60-0x088AC0E0
// @asm 0x088AC6A0-0x088AC82C
// @asm 0x088ACE70-0x088AD128
// @asm 0x088AD368-0x088AD47C
// @asm 0x088AD8BC-0x088ADC34
// @asm 0x088ADF28-0x088ADFC0
// @asm 0x088AE1F8-0x088AE314
// @asm 0x088AE378-0x088AE548
// @asm 0x088AEE0C-0x088AF0DC
/* Target list of a DuelAct (w[0..4], then duel_chain.uids from w[5]) and effect handlers. */
#include "duel.h"

typedef struct DuelTarget {            /* entry of DuelAct.w[] and duel_chain.uids[] */
    unsigned short player : 1;
    unsigned short zone : 4;
    unsigned short uid : 9;
    unsigned short placed : 2;
} DuelTarget;

typedef struct DuelListCard {          /* view of duel.list[] entries */
    unsigned short id : 13;
    unsigned short placed : 2;
    unsigned short b15 : 1;
    unsigned short hi;
} DuelListCard;

extern unsigned int duel_w_089147E4;
extern int duel_w_08914908;
extern unsigned int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern int duel_player_type[2];
extern int duel_w_08919624;
extern unsigned int duel_w_0891490C;
extern DuelAct *duel_w_089147DC;
extern unsigned short duel_w_0891968C;
extern unsigned int duel_turn_player;

void duel_088CB8EC(unsigned int p, int z, int uid, int n);
int field_get_card(unsigned int p, int z);
void duel_088C87A8(unsigned int p, int z, unsigned short id, int a, unsigned short b);
void duel_wfmt_s(void *dst, void *fmt, void *arg);
unsigned short *card_get_name(int id);
unsigned short *card_db_table_sel(int i);
void duel_0884ACD8(unsigned int p, void *msg);
int duel_08835AC0(void);
int duel_0884E5E0(DuelAct *a);
int duel_0884E3E4(DuelAct *a, unsigned int p, int z);
void duel_088359E8(unsigned int p, int id, int (*fn)(DuelAct *a, unsigned int p, int z));
int duel_08817598(DuelAct *a, int b, int id);
int card_get_level(int id);
unsigned short duel_slot_e1854(unsigned int p, int z);
int duel_field_atk(unsigned int p, int z);
int duel_088BFFD8();
void duel_0884C588(unsigned int p, int a, int id, int b);
DuelCard *duel_0884C7CC(void);
int duel_0884C804(void);
void duel_08832974(unsigned int p, unsigned short id, int a);
void duel_0884ADAC(int a, int b, int c, int d, int e, int f);
void duel_wcscpy(unsigned short *dst, void *src);
void duel_wfmt_d(unsigned short *dst, int fmt, int n);
int duel_0880A2B8(unsigned int p, unsigned int q, int skip, int fa, int fd);
void duel_0884AD18(int msg);
int duel_0880A54C(DuelAct *a, unsigned int p, int skip, int any);
int duel_act_can_target(DuelAct *a, unsigned int p, int z);
int duel_08854D4C(DuelAct *a, unsigned int p, int z);
int duel_08815950(DuelAct *a, unsigned int p, int atk, int def, int z);
DuelCard *duel_0884CCBC(int i);
int duel_0886F89C(DuelAct *b, unsigned int p, int z);
unsigned int duel_field_flag9(unsigned int p, int z);
int duel_slot_can_act(unsigned int p, unsigned int z, int x, int y);
int duel_0884E690(DuelAct *a, int (*fn)(DuelAct *a, unsigned int p, int z));
void duel_08832960(unsigned int p, unsigned short id, int a);
void duel_088CC178(unsigned int p, int z, unsigned int v);
void duel_088329AC(unsigned int p, int a);
int duel_count_free_monster_zones(unsigned int p);
int duel_0884E9E8(DuelAct *a, int b);
void duel_0884CC68(int k);
int duel_phase_ok2(unsigned int p);
int duel_grave_find_uid(unsigned int p, int uid);
void duel_088329BC(unsigned int p, unsigned short id, int b);
int duel_grave_find_name(unsigned int p, int id);
int duel_088711BC(DuelAct *a, DuelAct *b);
int duel_088AD5F4();
void duel_08835A54(unsigned int p, unsigned short id, int (*fn)(DuelAct *a, unsigned int p, int z));

static void duel_088A8B94(DuelAct *a, unsigned short loc)
{
    a->w[a->n7++] = loc;
}

void duel_088A8BB0(DuelAct *a, unsigned int p, int z)
{
    DuelTarget t;

    t.player = p;
    t.zone = z;
    t.uid = CARD_UID(duel.players[p & 1].field[z].card);
    t.placed = ((DuelListCard *)&duel.list[t.uid])->placed;
    a->w[a->n7++] = *(unsigned short *)&t;
}

void duel_088A8CAC(DuelAct *a, unsigned int p, int z, int uid)
{
    DuelTarget t;

    t.player = p;
    t.zone = z;
    t.uid = uid;
    t.placed = 0;
    a->w[a->n7++] = *(unsigned short *)&t;
}

void duel_088A8D40(DuelAct *a, unsigned int p, int z)
{
    if (a->n_targets < 5) {
        duel_088A8BB0(a, p, z);
    } else if (a->n_targets == 5) {
        a->w[a->n_targets] = duel_w_089147E4;
    }
    duel_088CB8EC(p, z, CARD_UID(duel.players[p & 1].field[z].card), a->n_targets);
    a->n_targets++;
    if (duel.players[p & 1].field[z].faceup != 0 && z < 5) {
        switch (field_get_card(p, z)) {
        case 0x1596:
        case 0x1598:
            duel_088C87A8(p, z, 0x1596, 2, a->uid);
            break;
        }
    }
}

void duel_088A8EA0(DuelAct *a, int p, int z, int uid)
{
    if (a->n_targets < 5) {
        duel_088A8CAC(a, p, z, uid);
    } else if (a->n_targets == 5) {
        a->w[a->n_targets] = duel_w_089147E4;
    }
    duel_088CB8EC(p, z, uid, a->n_targets);
    a->n_targets++;
}

unsigned int duel_act_target(DuelAct *a, int i)
{
    DuelTarget *t;

    if (i < 5) {
        t = (DuelTarget *)&a->w[i];
    } else {
        t = (DuelTarget *)&duel_chain.uids[(int)(a->w[5] + i - 5) % 128];
    }
    return (unsigned short)(t->player | t->zone << 8);
}

unsigned int duel_act_target2(DuelAct *a, int i)
{
    DuelTarget *t;

    if (i < 5) {
        t = (DuelTarget *)&a->w[i];
    } else {
        t = (DuelTarget *)&duel_chain.uids[(int)(a->w[5] + i - 5) % 128];
    }
    return t->uid;
}

int duel_088A9008(DuelAct *a, int i, unsigned int *loc)
{
    DuelTarget *t;
    unsigned char z;
    unsigned short uid;
    DuelPlayer *pl;

    if (i < 5) {
        t = (DuelTarget *)&a->w[i];
    } else {
        t = (DuelTarget *)&duel_chain.uids[(int)(a->w[5] + i - 5) % 128];
    }
    if (loc != 0) {
        *loc = (unsigned short)(t->player | t->zone << 8);
    }
    if (i >= (unsigned char)MAX(a->n7, a->n_targets)) {
        return 0;
    }
    z = t->zone;
    if (z > 10) {
        return 0;
    }
    pl = &duel.players[t->player & 1];
    if (pl->field[z].card.id == 0) {
        return 0;
    }
    uid = t->uid;
    if (uid != CARD_UID(pl->field[z].card)) {
        return 0;
    }
    return t->placed == ((DuelListCard *)&duel.list[uid])->placed;
}

#ifdef NON_MATCHING /* falta o andi 0xFFFF do uid (original: 364 bytes) */
void duel_088A9174(DuelAct *a, int i)
{
    DuelTarget *t;
    unsigned char z;
    unsigned short uid;
    int p;
    int j;

    if (i < 5) {
        t = (DuelTarget *)&a->w[i];
    } else {
        t = (DuelTarget *)&duel_chain.uids[(int)(a->w[5] + i - 5) % 128];
    }
    z = t->zone;
    if (z < 11) {
        uid = t->uid;
        if (uid != CARD_UID(duel.players[t->player & 1].field[z].card)) {
            for (p = 0; p < 2; p++) {
                for (j = 0; j < 11; j++) {
                    if (uid == CARD_UID(duel.players[p & 1].field[j].card)) {
                        t->player = p;
                        t->zone = j;
                        return;
                    }
                }
            }
        }
    }
}
#endif

int duel_088A92E0(DuelAct *a, unsigned int p, int z)
{
    int i;

    for (i = 0; i < a->n_targets; i++) {
        if ((unsigned short)((unsigned char)p | (unsigned char)z << 8) == duel_act_target(a, i)) {

            return 1;
        }
    }
    return 0;
}

void duel_088A9360(DuelAct *a, int b)
{
    unsigned short buf[256];
    unsigned short tmp[256];
    void *msg;

    switch (a->id) {
    default:
        msg = (void *)0x112;
        break;
    case 0x13E9:
        duel_wfmt_s(buf, (void *)0x45, card_get_name(0x13EA));
        msg = buf;
        break;
    case 0x1298:
        msg = (void *)(a->w[2] == 2 ? 0x8B : 0x119);
        break;
    case 0x12BB:
        msg = (void *)0x44;
        break;
    case 0x1485:
    case 0x1487:
    case 0x1503:
    case 0x158D:
    case 0x1913:
        msg = (void *)0x57;
        break;
    case 0x1005:
    case 0x16BA:
    case 0x18BA:
    case 0x186E:
        msg = (void *)0x61;
        break;
    case 0xFFA:
    case 0x1086:
    case 0x128E:
    case 0x12E6:
    case 0x16B8:
        msg = (void *)0x5F;
        break;
    case 0x101E:
    case 0x128D:
    case 0x139E:
    case 0x142E:
    case 0x152A:
    case 0x15FA:
    case 0x1613:
    case 0x164B:
    case 0x166F:
    case 0x179A:
    case 0x17DA:
    case 0x1868:
    case 0x186B:
    case 0x18BB:
    case 0x19F4:
    case 0x1A09:
        msg = (void *)0x62;
        break;
    case 0x1281:
    case 0x1284:
        msg = (void *)0x63;
        break;
    case 0x1103:
    case 0x128A:
    case 0x1243:
    case 0x14AC:
    case 0x14B2:
    case 0x1581:
    case 0x15D8:
    case 0x15F5:
    case 0x160E:
    case 0x1685:
    case 0x17B0:
    case 0x1863:
    case 0x192B:
    case 0x1953:
    case 0x19B4:
    case 0x19DB:
    case 0x1A02:
    case 0x1A0D:
    case 0x1A74:
    case 0x1A90:
    case 0x1A96:
        msg = (void *)0x64;
        break;
    case 0x13E3:
    case 0x1842:
        msg = (void *)0x65;
        break;
    case 0x10E6:
        msg = (void *)0x66;
        break;
    case 0x117B:
    case 0x12EB:
        msg = (void *)0x6B;
        break;
    case 0x119A:
        duel_wfmt_s(buf, (void *)0x6C, (void *)0x1F5);
        msg = buf;
        break;
    case 0x119B:
    case 0x12F8:
    case 0x134B:
    case 0x138D:
    case 0x145E:
    case 0x14C5:
    case 0x14CB:
    case 0x14E0:
    case 0x156C:
    case 0x1615:
    case 0x16C0:
    case 0x174D:
    case 0x1773:
    case 0x186A:
    case 0x18BE:
    case 0x18C9:
    case 0x19A9:
        msg = (void *)0x6D;
        break;
    case 0x11C3:
    case 0x14E9:
    case 0x1579:
    case 0x1587:
    case 0x171A:
    case 0x1889:
        msg = (void *)0x6E;
        break;
    case 0x1280:
    case 0x12A9:
    case 0x1388:
    case 0x148D:
    case 0x14B9:
    case 0x14BB:
    case 0x14EB:
    case 0x1511:
    case 0x1537:
    case 0x157B:
    case 0x15E4:
    case 0x15FF:
    case 0x1763:
    case 0x1844:
    case 0x184B:
    case 0x1909:
    case 0x190E:
    case 0x1945:
    case 0x19F1:
    case 0x1A6D:
        msg = (void *)0x6F;
        break;
    case 0x11EA:
    case 0x158E:
    case 0x18F0:
        msg = (void *)0x70;
        break;
    case 0x1227:
        msg = (void *)0x76;
        break;
    case 0x12D1:
    case 0x12FC:
    case 0x13AB:
    case 0x17E0:
    case 0x18BF:
    case 0x18C0:
    case 0x18C1:
    case 0x18C2:
    case 0x18C6:
    case 0x1A9C:
        msg = (void *)0x77;
        break;
    case 0x12C3:
    case 0x1322:
    case 0x1466:
    case 0x169A:
        msg = (void *)0x78;
        break;
    case 0x1253:
        msg = (void *)0x7A;
        break;
    case 0x1718:
    case 0x19DD:
        msg = (void *)0x7B;
        break;
    case 0x1362:
    case 0x157A:
    case 0x1669:
    case 0x167B:
    case 0x169B:
    case 0x16A2:
    case 0x16E3:
    case 0x175D:
    case 0x179F:
    case 0x17B8:
    case 0x17FC:
    case 0x17FF:
    case 0x1819:
    case 0x1853:
    case 0x188A:
    case 0x1893:
    case 0x18CB:
    case 0x18D3:
    case 0x1957:
    case 0x195B:
    case 0x1978:
    case 0x19AB:
    case 0x19DC:
    case 0x1A0F:
    case 0x1A32:
    case 0x1AA6:
        msg = (void *)0x7C;
        break;
    case 0x1420:
        msg = (void *)0x7D;
        break;
    case 0x140C:
    case 0x15B8:
        msg = (void *)0x7E;
        break;
    case 0x12E2:
        msg = (void *)0x80;
        break;
    case 0x12F1:
    case 0x1329:
    case 0x1492:
    case 0x1690:
    case 0x16CE:
    case 0x1890:
    case 0x18CA:
    case 0x192D:
        msg = (void *)0x85;
        break;
    case 0x12F2:
    case 0x132A:
        msg = (void *)0x86;
        break;
    case 0x1330:
        msg = (void *)0x87;
        break;
    case 0x130A:
        msg = (void *)0x88;
        break;
    case 0x1326:
        msg = (void *)0x89;
        break;
    case 0x132D:
    case 0x134C:
    case 0x134D:
    case 0x14E1:
    case 0x152B:
    case 0x15D6:
    case 0x15D7:
    case 0x1617:
    case 0x161E:
    case 0x1753:
    case 0x183F:
    case 0x1962:
    case 0x19F3:
    case 0x1A1C:
    case 0x1A9A:
    case 0x1A9E:
        msg = (void *)0x8D;
        break;
    case 0x137C:
    case 0x15B1:
    case 0x15F9:
    case 0x16B2:
    case 0x19A3:
    case 0x19C8:
        msg = (void *)0x8E;
        break;
    case 0x1363:
        msg = (void *)0x8F;
        break;
    case 0x1364:
        msg = (void *)0x90;
        break;
    case 0x13EB:
        msg = (void *)0x96;
        break;
    case 0x1475:
        duel_wfmt_s(tmp, (void *)0x97, card_get_name(0xFBC));
        duel_wfmt_s(buf, tmp, card_db_table_sel(0x13));
        msg = buf;
        break;
    case 0x13B0:
        duel_wfmt_s(buf, (void *)0x98, card_db_table_sel(7));
        msg = buf;
        break;
    case 0x14CD:
        duel_wfmt_s(buf, (void *)0x98, card_db_table_sel(3));
        msg = buf;
        break;
    case 0x14CE:
        duel_wfmt_s(buf, (void *)0x98, card_db_table_sel(1));
        msg = buf;
        break;
    case 0x1402:
        duel_wfmt_s(buf, (void *)0x99, card_get_name(0x13C7));
        msg = buf;
        break;
    case 0x143B:
        duel_wfmt_s(buf, (void *)0x99, card_get_name(0x1439));
        msg = buf;
        break;
    case 0x14FC:
        duel_wfmt_s(buf, (void *)0x127, card_get_name(0x1414));
        msg = buf;
        break;
    case 0x17F6:
        duel_wfmt_s(buf, (void *)0x127, card_get_name(0xFF8));
        msg = buf;
        break;
    case 0x1412:
        msg = (void *)0x9D;
        break;
    case 0x13F0:
        msg = (void *)0x9E;
        break;
    case 0x146B:
        msg = (void *)0xA3;
        break;
    case 0x147E:
        msg = (void *)0xA6;
        break;
    case 0x149B:
        msg = (void *)0xA7;
        break;
    case 0x13FE:
        msg = (void *)0xA8;
        break;
    case 0x14C3:
    case 0x1514:
    case 0x15A8:
    case 0x15B4:
    case 0x167C:
    case 0x1708:
    case 0x174C:
    case 0x185F:
    case 0x1A1E:
    case 0x1A35:
        msg = (void *)0x119;
        break;
    case 0x14D3:
    case 0x1538:
    case 0x1694:
    case 0x172A:
    case 0x178E:
        msg = (void *)0x11A;
        break;
    case 0x15E9:
    case 0x16C9:
    case 0x16CB:
        msg = (void *)0x125;
        break;
    case 0x1925:
        msg = (void *)0x95;
        break;
    }
    duel_0884ACD8(a->player ^ a->opp, msg);
}

int duel_088AA0D0(DuelAct *a, int b)
{
    switch (duel_w_08914908) {
    case 0:
        if (duel_0884E5E0(a) == 0) {
            return 1;
        }
        duel_088A9360(a, b);
        duel_088359E8(a->player, a->id, duel_0884E3E4);
        duel_w_08914908++;
        return 0;
    case 1:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            return 1;
        }
        duel_w_08914908--;
        return 0;
    }
    return 1;
}

int duel_088AA1C0(DuelAct *a, int b)
{
    if (duel_player_type[a->player ^ a->opp] == 1) {
        return duel_08817598(a, b, a->id);
    }
    return duel_088AA0D0(a, b);
}

typedef struct DuelAct16 {             /* view of DuelAct byte +0x16 */
    unsigned char pad[0x16];
    unsigned char lo : 2;
    unsigned char n2 : 2;
} DuelAct16;

#define ACT_SRC(a) (*(unsigned short *)&(a)->src)

int duel_088AA224(DuelAct *a)
{
    unsigned int loc;
    unsigned short v;

    switch (a->id) {
    case 0x140B:
    case 0x15A3:
    case 0x1768:
    case 0x1A8D:
        return a->w[0];
    case 0x164A:
    case 0x16BC:
    case 0x1745:
    case 0x1A03:
        return a->zone;
    case 0x169F:
        return card_get_level(duel.list[a->src.uid].id);
    case 0x17F5:
    case 0x198E:
    case 0x19DC:
    case 0x19DD:
        return a->w[1];
    case 0x179A:
    case 0x18C5:
        return a->uid;
    case 0x187F:
        return duel.list[a->src.uid].id;
    case 0x1927:
        return a->w[1];
    case 0x1971:
        return a->src.uid;
    case 0x197C:
        loc = duel_slot_e1854(a->player, a->zone);
        return duel_field_atk(loc & 0xFF, (unsigned short)loc >> 8 & 0xFF) | 0x10000000;
    case 0x19B6:
        if (a->kind == 0xF) {
            return ACT_SRC(a);
        }
        break;
    case 0x19D8:
        if ((v = a->w[0]) == 0) {
            loc = duel_slot_e1854(a->player, a->zone);
            return field_get_card(loc & 0xFF, (unsigned short)loc >> 8 & 0xFF);
        }
        return v;

    case 0x1A16:
        return card_get_level(duel.list[a->src.uid].id);
    case 0x1A98:
        return a->w[2];
    }
    return 0;
}

int duel_088AA488(DuelAct *a, int b)
{
    unsigned int p = a->player ^ a->opp;
    int v = duel_088AA224(a);

    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        if (duel_088BFFD8(p, a->id, v) == 0) {
            if (a->b3_4 == 2) {
                return -1;
            }
            return 1;
        }
        duel_0884ACD8(p, (void *)0x5E);
        duel_w_08914908++;
        return 0;
    case 1:
        duel_0884C588(p, 6, a->id, v);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_088A8EA0(a, duel_0884C7CC()->owner, duel_0884C804(), CARD_UID(*duel_0884C7CC()));
        break;
    }
    return 1;
}

int duel_088AA5F4(a, b)
DuelAct *a;
int b;
{
    if (a->b3_4 == 2) {
        return 1;
    }
    if (duel_player_type[a->player] == 1) {
        return duel_08817598(a, b, 0x10D3);
    }
    switch (duel_w_08914908) {
    case 0:
        if (duel_0884E5E0(a) == 0) {
            return 1;
        }
        duel_0884ACD8(a->player, (void *)0x65);
        duel_088359E8(a->player, 0x10D3, duel_0884E3E4);
        duel_w_08914908++;
        return 0;
    }
    return duel_088AA0D0(a, b);
}

int duel_088AA6DC(DuelAct *a)
{
    switch (duel_w_08914908) {
    case 0:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 0;
        } else {
            duel_0884ADAC(0x67, 0x196, 0x197, 0, 0, 0xF);
        }
        duel_w_08914908++;
        return 0;
    case 1:
        duel_088A8B94(a, a->player ^ duel_w_08919624);
        duel_08832974(1 - a->player, a->id, duel_w_08919624);
        duel_w_08914908++;
        return 0;
    }
    return 1;
}

int duel_088AA7C4(DuelAct *a)
{
    duel_088A8B94(a, a->player);
    return 1;
}

int duel_088AA7E8(DuelAct *a)
{
    duel_088A8B94(a, 1 - a->player);
    return 1;
}

int duel_088AA818(DuelAct *a)
{
    duel_088A8D40(a, duel_special.player[0], duel_special.zone[0]);
    return 1;
}

int duel_088AA844(DuelAct *a, unsigned int p, int z)
{
    if (duel_0884E3E4(a, p, z) == 0) {
        return 0;
    }
    return !duel_088A92E0(a, p, z);
}

int duel_088AA8A4(DuelAct *a)
{
    unsigned short buf[256];
    unsigned short id = a->id;
    int n = 2;

    switch (id) {
    case 0x18F8:
        n = duel.players[a->player & 1].field[a->zone].link;
    }
    switch (duel_w_08914908) {
    case 0:
        if (duel_0884E5E0(a) < n) {
            return 1;
        }
        duel_w_08914908++;
        return 0;
    case 1:
        switch (id) {
        case 0x1147:
            duel_wcscpy(buf, (void *)0x69);
            break;
        case 0x11F0:
        case 0x18F8:
            duel_wfmt_d(buf, 0x71, n);
            break;
        case 0x184A:
            duel_wfmt_d(buf, 0x116, n);
            break;
        case 0x17EA:
            duel_wfmt_d(buf, 0x6D, n);
            break;
        case 0x1A35:
            duel_wcscpy(buf, (void *)0x119);
            break;
        }
        duel_0884ACD8(a->player, buf);
        duel_088359E8(a->player, a->id, duel_088AA844);
        duel_w_08914908++;
        return 0;
    case 3:
        duel_08835A54(a->player, id, duel_088AA844);
        duel_w_08914908++;
        return 0;
    case 2:
    case 4:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            if (a->n_targets >= n) {
                return 1;
            }
            duel_w_08914908 = 3;
            return 0;
        }
        duel_w_08914908--;
        return 0;
    }
    return 1;
}

int duel_088AAB24(DuelAct *a)
{
    unsigned int p;
    unsigned char z;
    int i;
    int t;

    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        p = a->player;
        if (duel_player_type[p] == 1) {
            z = a->zone;
            i = 0;
            t = -1;
            if (a->uid == CARD_UID(duel.players[p & 1].field[z].card)) {
                duel_088A8D40(a, p, z);
                i++;
            }
            for (; i < 2; i++) {
                t = duel_0880A2B8(a->player, 1 - a->player, t, 1, 1);
                if (t > -1 && duel_0884E3E4(a, 1 - a->player, t)) {
                    duel_088A8D40(a, 1 - a->player, t);
                }
            }
            return 1;
        }
        duel_0884ACD8(p, (void *)0x4D);
        duel_w_08914908++;
        return 0;
    case 1:
        duel_088359E8(a->player, a->id, duel_088AA844);
        duel_w_08914908++;
        return 0;
    case 2:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            if (duel_0884E5E0(a) == 1) {
                return 1;
            }
            duel_w_08914908++;
            return 0;
        }
        duel_w_08914908 -= 2;
        return 0;
    case 3:
        duel_0884AD18(0x75);
        duel_w_08914908++;
        return 0;
    case 4:
        if (duel_w_08919624 == 0) {
            return 1;
        }
        duel_088359E8(a->player, a->id, duel_088AA844);
        duel_w_08914908++;
        return 0;
    case 5:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            return 1;
        }
        duel_w_08914908 -= 2;
        return 0;
    }
    return 0;
}

int duel_088AAE5C(DuelAct *a)
{
    int ok;
    int f;
    unsigned int sp;
    int sz;

    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        ok = duel_0884E5E0(a) > 0;
        f = 0;
        if ((unsigned int)(a->kind - 0x12) < 2) {
            sp = duel_special.player[0];
            sz = duel_special.zone[0];
            if (duel.players[sp & 1].field[sz].card.id != 0 && duel.players[sp & 1].field[sz].pos == 0
                && duel_act_can_target(a, sp, sz) && a->player != sp) {
                f = 1;
            }
        }
        if (ok && f) {
            if (duel_player_type[a->player] == 1) {
                duel_w_08919624 = 2;
            } else {
                duel_0884ADAC(0x103, 0x1A0, 0x1A1, 0x1A2, 0, 0xF);
            }
        } else if (f) {
            duel_w_08919624 = 0;
        } else if (ok) {
            duel_w_08919624 = 1;
        }
        duel_w_08914908++;
        return 0;
    case 1:
        a->w[2] = duel_w_08919624 + 1;
        if (duel_w_08919624 != 1) {
            duel_088A8D40(a, duel_special.player[0], duel_special.zone[0]);
        }
        if (duel_w_08919624 == 0) {
            return 1;
        }
        duel_w_08914908++;
        return 0;
    case 2:
        if (duel_player_type[a->player] == 1) {
            duel_088A8D40(a, a->player, duel_0880A54C(a, a->player, -1, 1));
            return 1;
        }
        duel_0884ACD8(a->player, (void *)0x79);
        duel_088359E8(a->player, a->id, duel_0884E3E4);
        duel_w_08914908++;
        return 0;
    case 3:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            return 1;
        }
        duel_w_08914908--;
        return 0;
    }
    return 0;
}

int duel_088AB164(DuelAct *a, int b)
{
    unsigned int p = a->player;

    if (a->uid != CARD_UID(duel.players[p & 1].field[a->zone].card)) {
        return -1;
    }
    if (duel_player_type[p] == 1) {
        return duel_08817598(a, b, a->id);
    }
    return duel_088AA0D0(a, b);
}

int duel_088AB224(DuelAct *a, int b)
{
    if (duel_player_type[a->player] == 1) {
        duel_08817598(a, b, 0x10D3);
        duel_088A8B94(a, 1);
        return 1;
    }
    switch (duel_w_08914908) {
    case 0:
        duel_0884ACD8(a->player, (void *)0x65);
        a->n_targets = 0;
        a->n7 = 0;
        duel_088359E8(a->player, a->id, duel_08854D4C);
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
        duel_0884ADAC(0x7F, 0x198, 0x199, 0, 0, 0xF);
        duel_w_08914908++;
        return 0;
    case 3:
        duel_088A8B94(a, duel_w_08919624 + 1);
        duel_08832974(1 - a->player, a->id, duel_w_08919624);
        return 1;
    }
    return 0;
}

int duel_088AB3D0(DuelAct *a, int b)
{
    int p;
    int z;
    unsigned int t;

    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        if (duel_0884E5E0(a) > 1) {
            if (duel_player_type[a->player] == 1) {
                duel_08817598(a, b, 0);
                duel_w_08914908 = 2;
            } else {
                duel_0884ACD8(a->player, (void *)0x10F);
                duel_088359E8(a->player, a->id, duel_0884E3E4);
                duel_w_08914908++;
            }
        } else {
            for (p = 0; p < 2; p++) {
                for (z = 0; z <= 4; z++) {
                    if (!duel_0884E3E4(a, p, z) != 0) {
                        continue;
                    }
                    duel_088A8D40(a, p, z);
                    duel_w_08914908 = 2;
                    return 0;
                }
            }
        }
        return 0;
    case 1:
        if (duel_088AA0D0(a, b) != 0) {
            duel_w_08914908++;
        }
        return 0;
    case 2:
        t = duel_act_target(a, 0);
        a->w[2] = field_get_card(t & 0xFF, (unsigned short)t >> 8 & 0xFF);
        return 1;
    }
    return 0;
}

int duel_088AB580(DuelAct *a)
{
    a->n_targets = 0;
    a->n7 = 0;
    duel_088A8D40(a, a->src.player, a->src.zone);
    return 1;
}

int duel_088AB5B8(DuelAct *a, unsigned int p, int z)
{
    if (duel_0884E3E4(a, p, z) == 0) {
        return 0;
    }
    switch (a->n7) {
    case 2:
        return a->player != p;
    case 1:
        if ((unsigned short)((unsigned char)p | (unsigned char)z << 8) == duel_act_target(a, 0)) {
            return 0;
        }
    case 0:
        if (a->player == p) {
            return 1;
        }
    }
    return 0;
}

int duel_088AB684(DuelAct *a)
{
    unsigned int p = a->player;
    int z;

    if (duel_player_type[p] == 1) {
        a->n_targets = 0;
        a->n7 = 0;
        z = duel_08815950(a, a->player, -1, -1, -1);
        if (z < 0) {
            return 1;
        }
        duel_088A8D40(a, a->player, z);
        z = duel_08815950(a, a->player, -1, -1, z);
        if (z < 0) {
            return 1;
        }
        duel_088A8D40(a, a->player, z);
        z = duel_08815950(a, 1 - a->player, 1, 0, -1);
        if (z < 0) {
            return 1;
        }
        duel_088A8D40(a, 1 - a->player, z);
        return 1;
    }
    switch (duel_w_08914908) {
    case 0:
        duel_0884ACD8(p, (void *)0x82);
        duel_088359E8(a->player, a->id, duel_088AB5B8);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_0884ACD8(p, (void *)0x83);
        duel_088359E8(a->player, a->id, duel_088AB5B8);
        duel_w_08914908++;
        return 0;
    case 4:
        duel_0884ACD8(p, (void *)0x84);
        duel_088359E8(a->player, a->id, duel_088AB5B8);
        duel_w_08914908++;
        return 0;
    case 1:
    case 3:
    case 5:
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

int duel_088AB8FC(DuelAct *a)
{
    int n = 0;
    unsigned int i;
    DuelCard *c;

    switch (a->id) {
    case 0x12ED:
    case 0x1480:
        n = 2;
        break;
    case 0x183C:
        n = 3;
        break;
    case 0x12F9:
    case 0x1996:
        n = 5;
        break;
    case 0x1515:
        n = 1;
        break;
    }
    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        if (duel_088BFFD8(a->player, a->id, 0) == 0) {
            return 1;
        }
        duel_w_08919624 = 1;
        duel_w_08914908++;
        return 0;
    case 1:
        if (duel_w_08919624 == 0) {
            return 1;
        }
        duel_w_0891490C = 0;
        duel_0884ACD8(a->player, (void *)0x56);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_0884C588(a->player, MIN(n, 0x20) + 0x27, a->id, 0);
        duel_w_08914908++;
        return 0;
    case 3:
        for (i = 0; i < duel_w_08919624; i++) {
            c = duel_0884CCBC(i + 1);
            duel_088A8EA0(a, c->owner, 0xE, CARD_UID(*c));
        }
        return 1;
    }
    return 1;
}

int duel_088ABAFC(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    return duel_0886F89C(a, p, z);
}

int duel_088ABB28(DuelAct *a, unsigned int p, int z)
{
    if (z >= 5) {
        return 0;
    }
    if (p != (duel_act_target(a, 0) & 0xFF)) {
        return 0;
    }
    return duel_0886F89C(a, p, z);
}

int duel_088ABB98(DuelAct *a, unsigned int p, int z)
{
    if (z < 5 && duel_field_flag9(p, z) == 0) {
        return 0;
    }
    return duel_0886F89C(a, p, z);
}

int duel_088ABC04(DuelAct *a, DuelAct *b)
{
    unsigned short buf[256];
    int (*fn)(DuelAct *a, unsigned int p, int z);

    switch (a->id) {
    case 0x1327:
        fn = duel_088ABAFC;
        break;
    case 0x140A:
        fn = duel_088ABB28;
        break;
    case 0x1719:
        fn = duel_088ABB98;
        break;
    }
    switch (duel_w_08914908) {
    case 0:
        duel_wfmt_s(buf, (void *)0x8A, card_get_name(b->id));
        duel_0884ACD8(a->player, buf);
        duel_w_089147DC = b;
        duel_088359E8(a->player, a->id, fn);
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
        duel_w_089147DC = a;
        return 1;
    }
    return 0;
}

int duel_088ABD7C(DuelAct *a, unsigned int p, int z)
{
    unsigned int tp = duel_act_target(a, 0) & 0xFF;
    unsigned int tz = (unsigned short)duel_act_target(a, 0) >> 8 & 0xFF;
    unsigned int loc = duel_slot_e1854(tp, tz);

    if (duel_act_can_target(a, p, z) == 0) {
        return 0;
    }
    if (loc != 0xFFFF && loc != (unsigned short)((unsigned char)p | (unsigned char)z << 8)
        && duel_slot_can_act(tp, tz, p, z) != 0) {
        return 1;
    }
    return 0;
}

#ifdef NON_MATCHING /* passo 2: o original calcula z * 0x14 antes do índice do jogador */
int duel_088ABE60(DuelAct *a)
{
    unsigned short buf[256];
    unsigned int t;
    DuelSlot *s;
    int p;
    int z;
    int i;

    switch (duel_w_08914908) {
    case 0:
        if (duel_0884E5E0(a) == 0) {
            return 1;
        }
        duel_0884ACD8(a->player, (void *)0x8B);
        duel_088359E8(a->player, a->id, duel_0884E3E4);
        duel_w_08914908++;
        return 0;
    case 2:
        if (duel_0884E690(a, duel_088ABD7C) == 0) {
            return 1;
        }
        if (duel_player_type[a->player] == 1) {
            p = (unsigned char)duel_act_target(a, 0) ^ 1;
            for (i = 0; i < 2; i++, p ^= 1) {
                for (z = 0; z <= 4; z++) {
                    if (!duel_088ABD7C(a, p, z) != 0) {
                        continue;
                    }
                    duel_088A8D40(a, p, z);
                    return 1;
                }
            }
            return 1;
        }
        t = duel_act_target(a, 0);
        z = (unsigned short)t >> 8 & 0xFF;
        p = t & 0xFF;
        duel_wfmt_s(buf, (void *)0x8C, card_get_name(duel.players[p & 1].field[z].card.id));
        duel_0884ACD8(a->player, buf);
        duel_088359E8(a->player, a->id, duel_088ABD7C);
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

int duel_088AC0E0(DuelAct *a, int b)
{
    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        duel_088329AC(a->player, a->id);
        duel_w_08914908++;
        return 0;
    case 1:
        duel_088CC178(a->player, a->zone, duel_w_0891968C);
        duel_088A8B94(a, duel_w_0891968C);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_08832960(1 - a->player, 0x1357, duel_w_0891968C);
        duel_w_08914908++;
        return 0;
    }
    return 1;
}

int duel_088AC1D8(DuelAct *a, int b)
{
    unsigned short buf[256];
    int n;
    int fmt = 0x37;
    int v;
    DuelCard *c;

    switch (a->id) {
    case 0x1359:
        n = 0x2A;
        break;
    case 0x156A:
        fmt = 0x11D;
        n = 8;
        break;
    case 0x149E:
        fmt = 0x5A;
        n = 9;
        break;
    case 0x16D6:
        fmt = 0x116;
        n = 8;
        break;
    case 0x148B:
    case 0x1714:
    case 0x17F1:
    case 0x1974:
    case 0x1A26:
        n = 8;
        break;
    case 0x14E7:
    case 0x1630:
        fmt = 0x4E;
        n = 8;
        break;
    case 0x16A8:
    case 0x17F7:
        fmt = 0x4C;
        n = 8;
        break;
    case 0x1788:
        v = duel_count_free_monster_zones(a->player) > 4 ? 4 : duel_count_free_monster_zones(a->player);
        fmt = 0x11D;
        n = MIN(v, 0x20) + 0x27;
        break;
    case 0x196F:
        fmt = 0x4C;
        n = 0xB;
        break;
    case 0x1864:
        n = MIN(((DuelAct16 *)a)->n2, 0x20) + 6;
        break;
    }
    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        if (duel_0884E9E8(a, b) == 0) {
            return 1;
        }
        duel_wfmt_d(buf, fmt, n - 6);
        duel_0884ACD8(a->player, buf);
        duel_w_08914908++;
        return 0;
    case 1:
        duel_0884C588(a->player, n, a->id, 0);
        duel_w_08914908++;
        duel_w_0891490C = 0;
        return 0;
    case 2:
        if (duel_w_0891490C++ < duel_w_08919624) {
            c = duel_0884CCBC(duel_w_0891490C);
            duel_0884CC68(duel_w_0891490C);
            duel_088A8EA0(a, c->owner, duel_0884C804(), CARD_UID(*c));
            return 0;
        }
        duel_w_08914908++;
        return 0;
    }
    return 1;
}

int duel_088AC4CC(DuelAct *a)
{
    unsigned int p = duel_turn_player ^ duel_w_0891490C;
    DuelCard *c;

    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        duel_w_0891490C = 0;
        duel_w_08914908++;
        return 0;
    case 1:
        if (duel_count_free_monster_zones(p) && duel_phase_ok2(p) && duel_088BFFD8(p, a->id, a->uid)) {
            duel_088329BC(p, a->id, a->uid);
            duel_w_08914908++;
        } else {
            duel_w_08914908 += 2;
        }
        return 0;
    case 2:
        c = &duel.players[p & 1].grave[duel_grave_find_uid(p, duel_w_0891968C)];
        duel_088A8EA0(a, c->owner, 0xE, CARD_UID(*c));
        duel_w_08914908++;
        return 0;
    case 3:
        if ((int)++duel_w_0891490C >= 2) {
            duel_w_08914908++;
        } else {
            duel_w_08914908 = 1;
        }
        return 0;
    }
    return 1;
}

#ifdef NON_MATCHING /* a fica em $a3 através da chamada a duel_088A8B94 (original: $a0) */
int duel_088AC6A0(DuelAct *a)
{
    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        duel_0884ACD8(a->player, (void *)0x91);
        duel_w_08914908++;
        return 0;
    case 1:
        switch (a->id) {
        case 0x1368:
            duel_0884C588(a->player, 0x4B, a->id, 0);
            break;
        case 0x1568:
            duel_0884C588(a->player, 0x4D, a->id, 0);
            break;
        case 0x15DE:
        case 0x169D:
        case 0x16D3:
        case 0x1803:
            duel_0884C588(a->player, 0x4C, a->id, 0);
            break;
        }
        duel_w_08914908++;
        return 0;
    case 2:
        duel_088A8B94(a, duel_w_08919624);
        duel_088CC178(a->player, a->zone, a->w[0]);
        duel_w_08914908++;
        return 0;
    case 3:
        duel_08832974(1 - a->player, a->id, a->w[0]);
        return 1;
    }
    return 1;
}
#endif

int duel_088AC82C(DuelAct *a, int b)
{
    switch (a->w[2]) {
    case 1:
        return duel_088AA1C0(a, b);
    case 2:
        return 1;
    }
    return 1;
}

int duel_088AC86C(DuelAct *a)
{
    int ids[3];
    int i;
    int k;

    switch (a->id) {
    case 0x138A:
        ids[0] = 0x1288;
        ids[1] = 0x129B;
        ids[2] = 0x12B8;
        break;
    case 0x18A4:
        ids[0] = 0x189E;
        ids[1] = 0x18A0;
        ids[2] = 0x189F;
        break;
    }
    for (i = 0; i < 3; i++) {
        k = duel_grave_find_name(a->player, ids[i]);
        if (k >= 0) {
            duel_088A8EA0(a, a->player, 0xE, CARD_UID(duel.players[a->player & 1].grave[k]));
        } else {
            return -1;
        }
    }
    return 1;
}

int duel_088AC978(DuelAct *a, int b)
{
    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        if (duel_0884E5E0(a) < 2) {
            return 1;
        }
        duel_0884ACD8(a->player, (void *)0x92);
        if (duel_player_type[a->player] == 1) {
            duel_08817598(a, b, a->id);
            duel_w_08914908 += 2;
            return 0;
        }
        duel_088359E8(a->player, a->id, duel_088AA844);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_0884ACD8(a->player, (void *)0x93);
        duel_088359E8(a->player, a->id, duel_088AA844);
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

int duel_088ACB24(DuelAct *a, DuelAct *b)
{
    int ok;
    int f;
    int t1;

    switch (duel_w_08914908) {
    case 0:
        ok = duel_088711BC(a, b) == 2;
        f = 0;
        t1 = a->kind == 0x12;
        if (t1) {
            t1 = duel_special.player[0] != a->player;
        }
        if (t1) {
            t1 = duel_special.player[1] == a->player;
        }
        if (t1 && duel_0884E5E0(a)) {
            f = 1;
        }
        if (ok && f) {
            duel_w_08914908 = 10;
            return 0;
        }
        if (ok) {
            a->w[1] = 1;
        } else {
            a->w[1] = 2;
        }
        break;
    case 10:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = 2;
        } else {
            duel_0884AD18(0x109);
        }
        duel_w_08914908++;
        return 0;
    case 11:
        a->w[1] = duel_w_08919624 + 1;
        duel_w_08914908 = 0;
        break;
    }
    switch (a->w[1]) {
    case 1:
        return duel_088ABC04(a, b);
    case 2:
        return duel_088AD5F4(a, b);
    }
    return 1;
}

/* Card effect handlers (DuelAct *a): target selection on duel_w_08914908. */

extern unsigned short duel_h_0891968E;

int duel_slot_effect_arg(unsigned int p, int z, int id);
unsigned int duel_card_locate(unsigned int uid);
void duel_088C89BC(unsigned int p, int z, int id, int c);
void duel_0884AD40(int msg, int uid);
int duel_08871868(DuelAct *a);
int duel_zone_free(unsigned int p, int z);
void duel_088CB938(unsigned int p, int a);
int duel_list_counter(int uid, int id);
int duel_field_kind(unsigned int p, int z);
void duel_08832988(unsigned int p, unsigned short id, unsigned short n, int a);

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
        duel_wfmt_d((unsigned short *)buf, 0x9B, 2);
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

int duel_088AD5F4(a)
DuelAct *a;
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
        duel_wfmt_d((unsigned short *)buf, 0xA1, 2);
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


int duel_088DFE24(unsigned int p, unsigned int q);
void duel_088329D0();
int duel_count_type(unsigned int p, unsigned int type);

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

/* Card effect handlers (DuelAct *a): target selection on duel_w_08914908, after effects_088ACCF8.c. */


int card_is_attribute9(int id);
int duel_field_e5ae0(unsigned int p, int z);
int duel_act_immune(DuelAct *a, unsigned int p, int z);
void duel_088D1AC8(unsigned int p, int z);
void duel_088C44E4(unsigned int p, int id);

#ifdef NON_MATCHING /* o retorno 1 padrão do switch externo e o do interno são blocos separados no original */
int duel_088AE378(DuelAct *a, int b)
{
    char buf[512];
    unsigned int t;
    int r;

    switch (a->w[2]) {
    default:
        return 1;
    case 1:
    case 3:
        switch (duel_w_08914908) {
        case 0:
        case 1:
            a->w[2] = 1;
            if (duel_088AA1C0(a, b) != 0) {
                duel_w_08914908 = 2;
            }
            return 0;
        case 2:
            a->w[2] = 3;
            if (duel_player_type[a->player] == 1) {
                return duel_08817598(a, b, a->id);
            }
            t = duel_act_target(a, 0);
            duel_wfmt_s(buf, (char *)0x45, card_get_name(field_get_card(t & 0xFF, (unsigned short)t >> 8 & 0xFF)));
            duel_0884ACD8(a->player, buf);
            duel_088359E8(a->player, a->id, duel_0884E3E4);
            duel_w_08914908++;
            return 0;
        case 3:
            if (duel_08835AC0() != 0) {
                duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
                return 1;
            }
            duel_w_08914908--;
            return 0;
        }
        break;
    case 2:
        return duel_088AA1C0(a, b);
    }
    return 1;
}
#endif

int duel_088AE548(DuelAct *a)
{
    unsigned int p = a->player;

    switch (duel_w_08914908) {
    case 0:
        if (duel_088BFFD8(p, a->id) == 0) {
            return -1;
        }
        duel_w_08914908++;
        return 0;
    case 1:
        duel_088329D0(1 - p, a->id);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_088A8EA0(a, p, 0xE, duel_h_0891968E);
        break;
    }
    return 1;
}

int duel_088AE600(DuelAct *a, unsigned int p, int z)
{
    if (duel_0884E3E4(a, p, z) == 0) {
        return 0;
    }
    switch (a->n7) {
    case 2:
        return a->player == p;
    case 1:
        if ((unsigned short)((unsigned char)p | (unsigned char)z << 8) == duel_act_target(a, 0)) {
            return 0;
        }
    case 0:
        if (a->player != p) {
            return 1;
        }
        break;
    }
    return 0;
}

int duel_088AE6CC(DuelAct *a)
{
    switch (duel_w_08914908) {
    case 0:
        duel_0884ACD8(a->player, (void *)100);
        duel_088359E8(a->player, a->id, duel_088AE600);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_088359E8(a->player, a->id, duel_088AE600);
        duel_w_08914908++;
        return 0;
    case 4:
        duel_0884ACD8(a->player, (void *)0x7C);
        duel_088359E8(a->player, a->id, duel_088AE600);
        duel_w_08914908++;
        return 0;
    case 1:
    case 3:
    case 5:
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

int duel_088AE840(DuelAct *a, int b)
{
    if (a->w[2] != 0) {
        return duel_088AA1C0(a, b);
    }
    return 1;
}

int duel_088AE868(DuelAct *a)
{
    DuelSrc *s = &a->src;
    unsigned int t;

    t = duel_card_locate(s->uid);
    if (((unsigned short)t >> 8 & 0xFF) == 0xE) {
        duel_088A8EA0(a, t & 0xFF, 0xE, s->uid);
    }
    return 1;
}

int duel_088AE8CC(DuelAct *a)
{
    unsigned int n;

    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        duel_08832988(a->player, a->id, 1, 0xFF);
        duel_w_08914908++;
        return 0;
    case 1:
        n = duel_h_0891968E;
        duel_088CC178(a->player, a->zone, n);
        duel_088A8B94(a, n);
        duel_08832960(1 - a->player, 0x171F, n);
        duel_w_08914908++;
        return 0;
    }
    return 1;
}

int duel_088AE9A8(DuelAct *a)
{
    unsigned int t;
    unsigned int tz;
    int z;
    int n;
    int id;

    if (duel_w_08914908 != 10) {
        if (duel_088AA5F4(a) != 0) {
            duel_w_08914908 = 10;
        }
        return 0;
    }
    t = duel_act_target(a, 0);
    tz = (unsigned short)t >> 8 & 0xFF;
    n = 0;
    for (z = 0; z <= 4; z++) {
        id = duel.players[a->player & 1].field[z].card.id;
        if (z != tz && id != 0 && card_is_attribute9(id) == 0 && duel_field_e5ae0(a->player, z) != 0 &&
            duel_act_immune(a, a->player, z) == 0) {
            duel_088D1AC8(a->player, z);
            n++;
        }
    }
    duel_088CC178(a->player, a->zone, n);
    return 1;
}

int duel_088AEAFC(DuelAct *a)
{
    duel_088C44E4(a->player, a->id);
    return 1;
}

int duel_0884E948(DuelAct *a, unsigned int p);
int duel_0884EB90(DuelAct *a, int b);
int duel_pick_st_zone(unsigned int p);
int duel_0885FB0C(DuelAct *a, unsigned int p, int z);
int duel_088601AC(DuelAct *a, unsigned int p, int z);
unsigned int card_get_type(int id);
int duel_0886CE90(DuelAct *a, int b);

int duel_088AEB24(DuelAct *a)
{
    int n;
    int m1;
    int m2;

    switch (a->id) {
    case 0x17E2:
        n = 2;
        m1 = 0x8D;
        m2 = 0x21;
        break;
    case 0x17EC:
        n = 3;
        m1 = 0x4D;
        m2 = 0x75;
        break;
    }
    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        if (duel_0884E5E0(a) == 0) {
            return -1;
        }
        duel_0884ACD8(a->player, (void *)m1);
        duel_w_08919624 = 1;
        duel_w_08914908++;
        return 0;
    case 1:
    case 4:
        if (duel_w_08919624 == 0) {
            return 1;
        }
        duel_088359E8(a->player, a->id, duel_088AA844);
        duel_w_08914908++;
        return 0;
    case 2:
    case 5:
        if (duel_08835AC0() != 0) {
            duel_088A8D40(a, duel_w_0891964C, duel_w_08919650 + duel_w_08919654);
            if (duel_0884E690(a, duel_088AA844) == 0) {
                return 1;
            }
            if (a->n7 >= n) {
                return 1;
            }
            duel_w_08914908 = 3;
            return 0;
        }
        duel_w_08914908 -= 2;
        return 0;
    case 3:
        if (duel_player_type[a->player] == 1) {
            duel_w_08919624 = a->n7 < duel_0884E948(a, 1 - a->player);
        } else {
            duel_0884AD18(m2);
        }
        duel_w_08914908++;
        return 0;
    }
    return 0;
}

int duel_088AED54(DuelAct *a, int p, int z)
{
    int i;

    if (p == a->player) {
        return 0;
    }
    if (z > 4) {
        return 0;
    }
    if (duel_zone_free(p, z) == 0) {
        return 0;
    }
    for (i = 0; i < a->n7; i++) {
        if (a->w[i] == (unsigned short)((unsigned char)p | (unsigned char)z << 8)) {
            return 0;
        }
    }
    return 1;
}

#ifdef NON_MATCHING /* o original assume que duel_088A8B94 (static na mesma unidade) preserva $a0; bate com ela static no arquivo */
int duel_088AEE0C(DuelAct *a)
{
    unsigned int p;
    int z;

    switch (duel_w_08914908) {
    case 0:
        a->n_targets = 0;
        a->n7 = 0;
        if (duel_0884E690(a, (void *)duel_088AED54) == 0) {
            return -1;
        }
        duel_0884ACD8(a->player, (void *)0x9B);
        duel_08835A54(a->player, a->id, (void *)duel_088AED54);
        duel_w_08914908++;
        return 0;
    case 2:
    case 4:
        duel_088359E8(a->player, a->id, (void *)duel_088AED54);
        duel_w_08914908++;
        return 0;
    case 1:
    case 3:
    case 5:
        if (duel_08835AC0() != 0) {
            p = duel_w_0891964C;
            z = duel_w_08919650 + duel_w_08919654;
            duel_088CB938(p, z);
            duel_088A8B94(a, (unsigned char)p | (unsigned char)z << 8);
            if (duel_0884E690(a, (void *)duel_088AED54) == 0) {
                return 1;
            }
            duel_w_08914908++;
            return 0;
        }
        if (a->n7 != 0) {
            return 1;
        }
        duel_w_08914908--;
        return 0;
    }
    return 1;
}
#endif

#ifdef NON_MATCHING /* o original assume que duel_088A8B94 (static na mesma unidade) preserva $a0; bate com ela static no arquivo */
int duel_088AEFB4(DuelAct *a)
{
    switch (duel_w_08914908) {
    case 0:
        duel_08832988(a->player, a->id, 1, 0xFF);
        duel_w_08914908++;
        return 0;
    case 1:
        duel_088A8B94(a, duel_h_0891968E);
        duel_08832960(1 - a->player, 0x171F, duel_h_0891968E);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_088329AC(a->player, a->id);
        duel_w_08914908++;
        return 0;
    case 3:
        duel_088A8B94(a, duel_w_0891968C);
        duel_08832960(1 - a->player, 0x1357, duel_w_0891968C);
        duel_w_08914908++;
        return 0;
    }
    return 1;
}
#endif

void duel_088AF0DC(DuelAct *a, DuelAct *b)
{
    a->id = b->id;
    duel_0884EB90(a, (int)b);
    a->id = 0x183E;
}

int duel_088AF110(DuelAct *a, int b)
{
    if (duel_pick_st_zone(a->player) == -1) {
        return -1;
    }
    if (duel_0884E5E0(a) != 0) {
        return duel_088AA1C0(a, b);
    }
    return -1;
}

int duel_088AF180(DuelAct *a)
{
    if (a->kind == 2) {
        return 1;
    }
    return duel_088AA5F4(a);
}

int duel_088AF1BC(DuelAct *a)
{
    switch (duel_w_08914908) {
    case 0:
        duel_0884ACD8(a->player, (void *)0x7C);
        duel_088359E8(a->player, a->id, duel_088601AC);
        duel_w_08914908++;
        return 0;
    case 2:
        duel_0884ACD8(a->player, (void *)100);
        duel_088359E8(a->player, a->id, duel_0885FB0C);
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

int duel_088AF2FC(DuelAct *a, int b)
{
    if (card_get_type(a->idx) == 0x16) {
        return 1;
    }
    if (card_get_type(a->idx) == 0x17) {
        return duel_088AA1C0(a, b);
    }
    if (duel_0886CE90(a, b) != 0) {
        return duel_088AA488(a, b);
    }
    return 1;
}
