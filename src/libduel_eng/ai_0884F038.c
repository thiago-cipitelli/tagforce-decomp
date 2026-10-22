// @range 0x0884F038-0x08850164
// @data 0x089116A4-0x089116C0
/* Computer player: tests on the action being considered, queueing actions. */
#include "duel.h"

extern int duel_w_089195EC;
extern unsigned int duel_view_player;
extern unsigned int duel_turn_player;
extern int duel_player_type[2];
extern unsigned int duel_w_089147E0;
extern unsigned int duel_w_08914920;
extern int duel_ai_step;               /* 0x089149C8 */
extern int duel_w_089149D4;
extern int duel_w_089149D8;
extern int duel_w_08919624;
extern int duel_w_08919628;
extern int duel_w_08919638;
extern int duel_w_0891963C;
extern int duel_w_08919640;
extern int duel_w_08919650;
extern unsigned short duel_wstr_nl[];  /* 0x0891261C L"\n" */
extern struct {
    unsigned char lo : 5;
    unsigned char b5 : 1;
    unsigned char hi : 2;
} duel_msg_state;
extern int duel_w_08914948[16];   /* duel_chain.srcs */

int duel_list_find_id(unsigned int p, unsigned int id);
void duel_msg_send(unsigned short cmd, int arg, void *data, int len);
void duel_bzero(void *p, int n);
void duel_088C7E68(unsigned int p, int z, int a, int b);
int duel_0886B0E4(unsigned int code, int uid, int arg);
int duel_0884E9E8(DuelAct *a, int b);
int card_is_monster(int id);
int card_get_prop5_hi(int id);
int duel_088B06F0();
int duel_0881530C(unsigned int p, DuelAct *a);
void duel_wcscpy(unsigned short *dst, int id);
void duel_wcscat(unsigned short *dst, unsigned short *src);
unsigned short *card_get_name(int id);
void duel_wfmt_s(unsigned short *dst, int fmt, unsigned short *arg);
void duel_0884AD2C(unsigned short *msg, int kind);
void duel_08835944(int a);
void duel_msg_send3(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_08834614(DuelAct *a);
void duel_08834734(int a, int b, DuelAct *c);
void duel_0883449C(DuelAct *a);

int duel_0884F038(DuelAct *a)
{
    if (duel_w_089195EC == 0) {
        return 0;
    }
    return duel_view_player != (a->player ^ a->opp);
}

void duel_0884F07C(which, code, uid, arg)
    int which;
    unsigned int code;
    unsigned short uid;
    unsigned int arg;
{
    DuelAct *a;
    unsigned int id;
    unsigned short m[6];

    if (uid >> 1 == 0) {
        uid = duel_list_find_id((code & 0x80000000) >> 31, code & 0xFFFF);
    }
    id = code & 0xFFFF;
    if (id == 0) {
        return;
    }
    if (duel_w_089195EC != 0 && duel_player_type[duel_turn_player] == 2) {
        m[0] = which;
        m[1] = id;
        m[2] = code >> 16;
        m[3] = arg;
        m[4] = arg >> 16;
        m[5] = uid;
        duel_msg_send(0x14, -1, m, 0xC);
        return;
    }
    if (which != 0) {
        a = &duel_chain.acts[duel_chain.unk480];
    } else {
        a = &((DuelAct *)duel_chain.pad0)[duel_w_089147E0];
    }
    duel_bzero(a, 0x18);
    a->id = id;
    a->player = (code & 0x80000000) >> 31;
    a->opp = (code & 0x1000000) >> 24;
    a->b3_4 = (code & 0xE00000) >> 21;
    a->zone = (code & 0x1F0000) >> 16;
    a->kind = (code & 0x7E000000) >> 25;
    a->uid = uid;
    a->idx = arg;
    if (which != 0) {
        duel_chain.unk480++;
    } else {
        duel_w_089147E0++;
    }
    if (a->zone < 0xB && a->b3_4 == 0) {
        duel_088C7E68(a->player, a->zone, 1, 1);
    }
}

int duel_0884F30C(unsigned int code, int uid, int arg)
{
    if ((unsigned short)uid >> 1 == 0) {
        uid = (unsigned short)duel_list_find_id((code & 0x80000000) >> 31, code & 0xFFFF);
    }
    if (duel_0886B0E4(code, uid, arg) != 0) {
        duel_0884F07C(0, code, uid, arg);
        return 1;
    }
    return 0;
}

void duel_0884F398(unsigned int code, int uid)
{
    DuelAct t = {0};
    unsigned int i;

    t.id = code;
    t.player = (code & 0x80000000) >> 31;
    t.opp = (code & 0x1000000) >> 24;
    t.b3_4 = (code & 0xE00000) >> 21;
    t.zone = (code & 0x1F0000) >> 16;
    t.uid = uid;
    if (duel_w_08914920 != 0) {
        for (i = 0; i < duel_w_08914920; i++) {
            t.kind = duel_chain.kinds[i];
            t.idx = *(int *)&duel_chain.srcs[i];
            if (!duel_0884E9E8(&t, 0)) {
                continue;
            }
            duel_0884F07C(0, t.id | ((t.zone << 16 & 0x1F0000) | ((t.b3_4 << 21 & 0xE00000) | ((t.opp << 24 & 0x1000000) | ((t.player << 31 & 0x80000000) | (t.kind << 25 & 0x7E000000))))), uid, duel_w_08914948[i]);
            return;
        }
    } else if (duel_0884E9E8(&t, 0) != 0) {
        duel_0884F07C(0, t.id | ((t.zone << 16 & 0x1F0000) | ((t.b3_4 << 21 & 0xE00000) | (((t.player << 31 & 0x80000000) | (t.opp << 24 & 0x1000000))))), uid, 0);
    }
}

void duel_0884F654(unsigned int code, int uid, int b)
{
    DuelAct t = {0};
    unsigned int i;

    t.id = code;
    t.player = (code & 0x80000000) >> 31;
    t.opp = (code & 0x1000000) >> 24;
    t.b3_4 = (code & 0xE00000) >> 21;
    t.zone = (code & 0x1F0000) >> 16;
    t.uid = uid;
    if (duel_w_08914920 != 0) {
        for (i = 0; i < duel_w_08914920; i++) {
            t.kind = duel_chain.kinds[i];
            t.idx = *(int *)&duel_chain.srcs[i];
            if (!duel_0884E9E8(&t, b)) {
                continue;
            }
            duel_0884F07C(1, t.id | ((t.zone << 16 & 0x1F0000) | ((t.b3_4 << 21 & 0xE00000) | ((t.opp << 24 & 0x1000000) | ((t.player << 31 & 0x80000000) | (t.kind << 25 & 0x7E000000))))), uid, duel_w_08914948[i]);
            return;
        }
    } else if (duel_0884E9E8(&t, b) != 0) {
        duel_0884F07C(1, t.id | ((t.zone << 16 & 0x1F0000) | ((t.b3_4 << 21 & 0xE00000) | (((t.player << 31 & 0x80000000) | (t.opp << 24 & 0x1000000))))), uid, 0);
    }
}

void duel_0884F91C(unsigned int code, int uid)
{
    duel_0884F07C(0, code, uid, 0);
}

int duel_0884F930(DuelAct *a)
{
    switch (a->id) {
    case 0x1174:
    case 0x133B:
    case 0x17CC:
        return 0;
    case 0x1445:
    case 0x17C9:
    case 0x190A:
    case 0x1A28:
        return a->b3_4 != 2;
    case 0x18EC:
        return a->b3_4 != 2 || a->zone == 0xE;
    case 0x1A90:
        return a->b3_4 == 2;
    default:
        return 1;
    }
    return 1;
}

int duel_0884FA28(DuelAct *a)
{
    unsigned int k = a->kind;

    if (k == 0x22) {
        return 0;
    }
    switch (a->id) {
    case 0x1315:
    case 0x1449:
    case 0x144C:
    case 0x1452:
    case 0x1457:
    case 0x140C:
    case 0x14E2:
    case 0x13F5:
    case 0x15B8:
    case 0x1A02:
    case 0x1A7A:
        if (a->b3_4 == 2) {
            return 0;
        }
        break;
    case 0x1282:
        if (k == 2) {
            return 0;
        }
        break;
    }
    return 1;
}

int duel_0884FB1C(int id, int n)
{
    switch (id) {
    case 0x168C:
    case 0x168D:
    case 0x168F:
        return n == 3;
    case 0x1690:
    case 0x1691:
        return n == 2 || n == 5;
    case 0x168E:
    case 0x1692:
        return n == 1 || n == 3 || n == 6;
    }
    return 0;
}

int duel_0884FBB0(DuelAct *a)
{
    int t;

    if (a->b3_4 == 2) {
        return 0;
    }
    if (a->b4_3) {
        return 1;
    }
    if (card_is_monster(a->id) != 0) {
        return 0;
    }
    t = card_get_prop5_hi(a->id);
    switch (t) {
    case 2:
    case 3:
    case 4:
        return 0;
    }
    switch (a->id) {
    case 0xFEE:
    case 0x1102:
    case 0x1400:
    case 0x1AA3:
        return 0;
    case 0x1231:
    case 0x1238:
    case 0x1514:
    case 0x159C:
    case 0x1980:
    case 0x1A18:
    case 0x1A69:
        return a->b4_2;
    }
    return 1;
}

int duel_0884FD04(DuelAct *a, unsigned int p)
{
    unsigned short buf[256];
    int n;

    switch (duel_ai_step) {
    case 0:
        duel_ai_step++;
    case 1:
    case 5:
        if (duel_player_type[p] == 1) {
            if (duel_088B06F0(a, p, 1) != 0 && duel_0881530C(p, a) != 0) {
                duel_ai_step = 0xB;
                return 0;
            }
            return 1;
        }
        if (duel_player_type[p] == 2) {
            duel_ai_step = 0x14;
        } else {
            if (duel_088B06F0(a, p, 0) == 0) {
                duel_wcscpy(buf, 0xCD);
                duel_wcscat(buf, duel_wstr_nl);
                duel_wcscat(buf, (unsigned short *)0xCE);
                n = 0;
            } else if (card_is_monster(a->id) != 0) {
                duel_wfmt_s(buf, 7, card_get_name(a->id));
                n = duel_088B06F0(a, p, 1) + 1;
            } else {
                duel_wfmt_s(buf, 8, card_get_name(a->id));
                n = duel_088B06F0(a, p, 1) + 1;
            }
            if (duel_ai_step == 5) {
                n = 3;
            }
            duel_0884AD2C(buf, n);
            duel_ai_step = 2;
        }
        return 0;
    case 2:
        if (duel_w_08919624 == 0) {
            return 1;
        }
        duel_ai_step = 0xA;
        duel_w_08919638 = 0;
        return 0;
    case 0xA:
        duel_08835944(5);
        duel_ai_step++;
        return 0;
    case 0xB:
        if (duel_w_08919638 == 0) {
            duel_ai_step = 5;
            return 0;
        }
        if (duel_w_08919640 != 3) {
            duel_w_0891963C = 1;
            duel_ai_step--;
            return 0;
        }
        duel_ai_step = 0x1E;
        return 0;
    case 0x14:
        duel_msg_send3(0xF, duel_chain.unk480, 0, 0);
        duel_w_089149D8 = 0;
        duel_msg_state.b5 = 0;
        duel_ai_step++;
        return 0;
    case 0x15:
        if (!duel_msg_state.b5) {
            return 0;
        }
        if (duel_w_089149D8 == 0) {
            return 1;
        }
        duel_ai_step = 0x1E;
        return 0;
    case 0x1E:
        duel_cmd((p != 0 ? 0x8000 : 0) | 0x62, p != (a->player ^ a->opp), 0, 0);
        duel_ai_step++;
    case 0x1F:
        switch (duel_w_08919650) {
        case 0xB:
            if (card_is_monster(duel_w_08919628) != 0) {
                duel_08834614(a);
            } else {
                duel_08834734(1, 1, a);
            }
            break;
        case 0xC:
        case 0xD:
        case 0xE:
        case 0xF:
            break;
        default:
            duel_0883449C(a);
        }
        if (duel_w_08919638 != 0) {
            return 0;
        }
        duel_w_089149D4 = 0;
        duel_w_089149D8 = 1;
        return 1;
    default:
        return 1;
    }
}
