// @range 0x088B08B8-0x088B153C
// @asm 0x088B0C88-0x088B1134
// @asm 0x088B1134-0x088B153C
// @data 0x08911AF0-0x08911B78
/* Response window: "activate a card?" question asked to a player after an event (duel_chain.kinds[0])
   or a chain link, with the text of the event; the computer answers by itself (duel_08815288/duel_0881530C). */
#include "duel.h"

extern unsigned int duel_view_player;
extern unsigned int duel_turn_player;
extern int duel_player_type[2];
extern unsigned int duel_w_08914920;   /* a response window is open */
extern int duel_w_089149CC;            /* step of the event window */
extern unsigned int duel_w_089149D0;   /* player asked */
extern int duel_w_089149D4;
extern int duel_w_089149D8;
extern int duel_w_089149E0;            /* step of the local player's window */
extern int duel_w_089149E4;            /* asked for the last chain link instead of an event */
extern int duel_w_08919624;
extern int duel_w_08919628;
extern int duel_w_08919638;
extern int duel_w_0891963C;
extern int duel_w_08919640;
extern int duel_w_0891964C;
extern int duel_w_08919650;
extern int duel_w_08919654;
extern unsigned short duel_wstr_nl2[];  /* 0x08912914 L"\n" */
extern struct {
    unsigned char lo : 5;
    unsigned char b5 : 1;               /* answer received */
    unsigned char hi : 2;
} duel_msg_state;

typedef union DuelEvSrc {               /* duel_chain.srcs[0] */
    unsigned int w;
    struct DuelActX x;
    struct {
        unsigned int lo : 21;
        unsigned int b21 : 1;
        unsigned int hi : 10;
    } y;
} DuelEvSrc;

void duel_wcscpy(unsigned short *dst, int id);
void duel_wcscat(unsigned short *dst, unsigned short *src);
void duel_wfmt_s(unsigned short *dst, int fmt, unsigned short *arg);
unsigned short *card_get_name(int id);
int card_is_monster(int id);
int duel_0884D1E0(void);
int duel_088B0570(unsigned int p, int b);
int duel_088B06F0(DuelAct *a, unsigned int p, int b);
int duel_08815288(unsigned int p);
int duel_0881530C(unsigned int p, DuelAct *a);
void duel_0884AD2C(unsigned short *msg, int kind);
void duel_08835944();
void duel_0884C588(unsigned int p, int a, int id, int b);
void duel_088AFEE0(int a);
void duel_088348BC(int a, unsigned int p);
void duel_08834614(int a);
void duel_08834734(int a, int b, int c);
void duel_0883449C(int a);
void duel_msg_send3(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);

/* text of the event that opened the window, then "activate a card?" */
void duel_088B08B8(unsigned short *buf)
{
    DuelEvSrc ev;
    int n;
    int kind;

    buf[0] = 0;
    if (duel_w_08914920 != 0) {
        kind = duel_chain.kinds[0];
        ev.w = ((unsigned int *)duel_chain.srcs)[0];
        switch (kind) {
        case 1:
            duel_wcscpy(buf, 0x3E);
            break;
        case 2:
            duel_wcscpy(buf, 0x3F);
            break;
        case 3:
        case 4:
            duel_wcscpy(buf, 0x40);
            break;
        case 0x17:
            if (duel_turn_player != duel_view_player) {
                duel_wcscpy(buf, 0x110);
            }
            break;
        case 5:
            if (duel_turn_player != duel_view_player) {
                duel_wcscpy(buf, 0xFB);
            } else {
                duel_wcscpy(buf, 0x43);
            }
            break;
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 0x1F:
        case 0x20:
        case 0x21:
            n = 0;
            switch (kind) {
            case 6:
                n = 0xFC;
                break;
            case 7:
                n = 0xFE;
                break;
            case 8:
                n = 0x100;
                break;
            case 9:
                n = 0xAF;
                break;
            case 10:
                n = 0x9F;
                break;
            case 0x1F:
                n = 0xA9;
                break;
            case 0x20:
                n = 0xAB;
                break;
            case 0x21:
                n = 0xAD;
                break;
            }
            duel_wcscpy(buf, n + ((duel_view_player ^ 1) == (ev.x.s_player ^ ev.x.s_b16)));
            break;
        case 0x12:
            if (ev.w != 0) {
                duel_wcscpy(buf, (duel_special.player[0] == (int)(duel_view_player ^ 1)) + 0xB6);
            } else {
                duel_wcscpy(buf, 0x9A);
            }
            break;
        case 0x13:
            duel_wcscpy(buf, 0xB5);
            break;
        case 0x14:
            duel_wcscpy(buf, 0xB1);
            break;
        case 13:
            duel_wcscpy(buf, 0x10A);
            break;
        case 14:
            duel_wcscpy(buf, 0x105);
            break;
        case 15:
            duel_wcscpy(buf, (ev.y.b21 == (duel_view_player ^ 1)) + 0xB9);
            break;
        case 0x16:
            duel_wcscpy(buf, 0xB8);
            break;
        case 11:
            duel_wcscpy(buf, 0xB2);
            break;
        case 12:
            duel_wcscpy(buf, 0xB3);
            break;
        case 16:
            duel_wcscpy(buf, (ev.x.s_b16 == (duel_view_player ^ 1)) + 0xBD);
            break;
        case 0x18:
            duel_wcscpy(buf, 0xC4);
            break;
        case 0x19:
            duel_wcscpy(buf, (ev.x.s_player == (duel_view_player ^ 1)) + 0xC5);
            break;
        case 0x1B:
            if (ev.x.s_b15) {
                duel_wcscpy(buf, 0x10B);
            } else {
                switch (ev.x.s_zone2) {
                case 0xB:
                    duel_wcscpy(buf, 0xC7);
                    break;
                case 0xE:
                    duel_wcscpy(buf, 0x10C);
                    break;
                }
            }
            break;
        case 0x1C:
            duel_wcscpy(buf, 0xC8);
            break;
        case 0x1D:
            duel_wcscpy(buf, (ev.x.s_player == (duel_view_player ^ 1)) + 0xC9);
            break;
        case 0x1E:
            duel_wcscpy(buf, 0xCB);
            break;
        }
        if (buf[0] != 0) {
            duel_wcscat(buf, duel_wstr_nl2);
        }
    }
    duel_wcscat(buf, (unsigned short *)0x10D);
}

#ifdef NON_MATCHING /* alocação de registradores: temporários (a2/v1, t0/a3) e o 1 de retorno mantido em $v0 */
/* window after an event, for the player duel_w_089149D0 then for the other one; returns 1 */
int duel_088B0C88(void)
{
    unsigned short buf[256];
    unsigned int p = duel_w_089149D0;
    int n;

    switch (duel_w_089149CC) {
    case 0: /* nothing left to do in the duel [?] */
        if (duel_0884D1E0()) {
            duel_w_089149D4 = 0;
            return 1;
        }
        duel_w_089149D8 = 0;
        duel_w_089149CC++;
    case 1:
    case 5: /* ask the player */
        switch (duel_chain.kinds[0]) {
        case 0x1F:
        case 0x20:
        case 0x21:
            if (duel_088B0570(p, 0) == 0) {
                duel_w_089149CC = 0x32;
                return 1;
            }
        }
        if (duel_player_type[p] == 1) {
            if (duel_088B0570(p, 1) && duel_08815288(p)) {
                duel_w_089149CC = 0xB;
            } else {
                duel_w_089149CC = 0x32;
            }
        } else if (duel_player_type[p] == 2) {
            duel_w_089149CC = 0x14;
        } else {
            if (duel_088B0570(p, 0) == 0) {
                duel_wcscpy(buf, 0xCC);
                duel_wcscat(buf, duel_wstr_nl2);
                duel_wcscat(buf, (unsigned short *)0xCE);
                n = 0;
            } else {
                duel_088B08B8(buf);
                n = duel_088B0570(p, 1) + 1;
            }
            if (duel_w_089149CC == 5) {
                n = 3;
            }
            duel_0884AD2C(buf, n);
            duel_w_089149CC = 2;
        }
        return 1;
    case 2: /* answer */
        if (duel_w_08919624 == 0) {
            duel_w_089149CC = 0x32;
            return 1;
        }
        duel_w_089149CC = 10;
        duel_w_08919638 = 0;
        return 1;
    case 10:
        duel_08835944(4);
        duel_w_089149CC++;
        return 1;
    case 11: /* card picked */
        if (duel_w_08919638 == 0) {
            duel_w_089149CC = 5;
            return 1;
        }
        switch (duel_w_08919640) {
        case 3:
            duel_w_089149CC = 0x1E;
            break;
        case 1:
            duel_0884C588(duel_w_0891964C, duel_w_08919650 - 0xB, -1, duel_w_08919650);
            duel_w_089149CC--;
            break;
        }
        return 1;
    case 0x14: /* remote player: wait for the answer */
        duel_088AFEE0(1);
        duel_w_089149D8 = 0;
        duel_msg_state.b5 = 0;
        duel_w_089149CC++;
        return 1;
    case 0x15:
        if (!duel_msg_state.b5) {
            return 1;
        }
        if (duel_w_089149D8 == 0) {
            duel_w_089149CC = 0x32;
            return 1;
        }
        duel_w_089149CC = 0x1E;
        return 1;
    case 0x1E: /* activate the card picked */
        switch (duel_w_08919650) {
        case 0xB:
            if (card_is_monster(duel_w_08919628)) {
                duel_08834614(0);
            } else {
                duel_08834734(1, 0, 0);
            }
            break;
        case 0xC:
        case 0xD:
        case 0xE:
        case 0xF:
            duel_088348BC(0, duel_w_0891964C);
            break;
        default:
            duel_0883449C(0);
            break;
        }
        if (duel_w_08919638) {
            return 1;
        }
        duel_w_089149D8 = 1;
        duel_w_089149D4 = 0;
        return 1;
    case 0x32: /* next player, until back to the turn player */
        duel_w_089149D0 = 1 - p;
        if (duel_w_089149D0 == duel_turn_player) {
            duel_w_08914920 = 0;
            duel_w_089149D4 = 0;
        } else {
            duel_w_089149CC = 0;
        }
        return 1;
    }
    return 1;
}

/* window of the local player (duel_view_player) after an event or a chain link; 1: closed */
int duel_088B1134(void)
{
    unsigned short buf[256];
    DuelAct *a = 0;
    unsigned int p = duel_view_player;
    int n;

    if (duel_w_089149E4) {
        a = &duel_chain.acts[duel_chain.len - 1];
    }
    switch (duel_w_089149E0) {
    case 0:
        if (duel_player_type[p] == 1) {
            if (duel_w_089149E4) {
                if (duel_088B06F0(a, p, 1) && duel_0881530C(p, a)) {
                    duel_w_089149E0 = 4;
                    return 0;
                }
            } else {
                if (duel_088B0570(p, 1) && duel_08815288(p)) {
                    duel_w_089149E0 = 4;
                    return 0;
                }
            }
            duel_msg_send3(0x10, 0, 0, 0);
            return 1;
        }
        duel_w_089149E0++;
    case 1:
    case 5: /* ask */
        if (duel_w_089149E4) {
            if (duel_088B06F0(a, p, 0) == 0) {
                duel_wcscpy(buf, 0xCD);
                duel_wcscat(buf, duel_wstr_nl2);
                duel_wcscat(buf, (unsigned short *)0xCE);
                n = 0;
            } else if (card_is_monster(a->id)) {
                duel_wfmt_s(buf, 7, card_get_name(a->id));
                n = duel_088B06F0(a, p, 1) + 1;
            } else {
                duel_wfmt_s(buf, 8, card_get_name(a->id));
                n = duel_088B06F0(a, p, 1) + 1;
            }
        } else {
            if (duel_088B0570(p, 0) == 0) {
                duel_wcscpy(buf, 0xCC);
                duel_wcscat(buf, duel_wstr_nl2);
                duel_wcscat(buf, (unsigned short *)0xCE);
                n = 0;
            } else {
                duel_088B08B8(buf);
                n = duel_088B0570(p, 1) + 1;
            }
        }
        if (duel_w_089149E0 == 5) {
            n = 3;
        }
        duel_0884AD2C(buf, n);
        duel_w_089149E0 = 2;
        return 0;
    case 2: /* answer: no */
        if (duel_w_08919624 == 0) {
            duel_msg_send3(0x10, 0, 0, 0);
            duel_w_08914920 = 0;
            return 1;
        }
        duel_w_089149E0++;
        duel_w_08919638 = 0;
        return 0;
    case 3: /* pick the card */
        if (duel_w_089149E4) {
            duel_08835944(5);
        } else {
            duel_08835944();
        }
        duel_w_089149E0++;
        return 0;
    case 4:
        if (duel_w_08919638 == 0) {
            duel_w_089149E0 = 5;
            return 0;
        }
        if (duel_w_08919640 != 3) {
            duel_w_0891963C = 1;
            duel_w_089149E0--;
            return 0;
        } else {
            duel_w_08919638 = 0;
            duel_w_089149E0 = 10;
            return 0;
        }
    case 10: /* send the card picked */
        if (duel_w_089149E4) {
            duel_msg_send3(0x11, duel_w_0891964C, duel_w_08919650, duel_w_08919654);
        } else {
            duel_msg_send3(0xE, duel_w_0891964C, duel_w_08919650, duel_w_08919654);
        }
        return 1;
    }
    return 1;
}
#endif
