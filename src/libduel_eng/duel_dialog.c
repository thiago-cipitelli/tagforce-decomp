// @range 0x0884ACD0-0x0884B094
// @asm 0x0884ADAC-0x0884AF20
/* Dialog requests to the host (duel_host_call 0x31, sub-command, two arguments): messages,
   prompts with a card or a text, yes/no questions; each one only for the local player. */
#include "duel.h"

extern int duel_player_type[2];
int duel_host_call(int a, int b, int c, int d);
int duel_088482F8(int a, int b);
int duel_req_flag(int a);
unsigned short *card_get_name(int id);
void duel_wfmt_s(char *dst, char *fmt, unsigned short *arg);

typedef struct DuelDlgCard {
    unsigned short id : 13;
    unsigned short b13 : 1;
    unsigned short mark : 1;           /* highlighted */
    unsigned short b15 : 1;
    unsigned short pad;
} DuelDlgCard;

typedef struct DuelDlgCards {          /* 0x08915254: cards shown by a dialog */
    int unk0;
    int n;                             /* +0x04 */
    DuelDlgCard c[4];                  /* +0x08 */
} DuelDlgCards;

extern DuelDlgCards duel_dlg_cards;

int duel_0884ACD0(int a)
{
    return duel_088482F8(a, 0);
}

void duel_0884ACD8(unsigned int p, int msg)
{
    if (duel_player_type[p] == 0) {
        duel_host_call(0x31, 0, msg, 0);
    }
}

int duel_0884AD18(int msg)
{
    return duel_host_call(0x31, 1, msg, 0);
}

int duel_0884AD2C(int msg, int arg)
{
    return duel_host_call(0x31, 2, msg, arg);
}

void duel_0884AD40(char *fmt, int uid)
{
    char buf[512];

    duel_wfmt_s(buf, fmt, card_get_name(duel.list[uid].id));
    duel_host_call(0x31, 1, (int)buf, uid);
}

#ifdef NON_MATCHING /* unrolled loop scheduled differently */
void duel_0884ADAC(int msg, int a, int b, int c, int d, unsigned int f)
{
    int *id = &a;
    int n = 0;
    int i;

    for (i = 0; i < 4; i++) {
        duel_dlg_cards.c[i].id = id[i];
        duel_dlg_cards.c[i].mark = f >> i & 1;
        n += id[i] != 0;
    }
    duel_dlg_cards.n = n;
    duel_dlg_cards.unk0 = 0;
    duel_host_call(0x31, 3, msg, 0);
}
#endif

int duel_0884AF20(int a)
{
    return duel_host_call(0x31, 4, 0xE7, a);
}

int duel_0884AF34(int a, int b)
{
    return duel_host_call(0x31, 5, a, b);
}

int duel_0884AF48(int a, unsigned short b, unsigned short c)
{
    return duel_host_call(0x31, 6, a, b | c << 16);
}

int duel_0884AF68(int a, unsigned short b, unsigned short c)
{
    return duel_host_call(0x31, 7, a, b | c << 16);
}

int duel_0884AF88(int msg)
{
    return duel_host_call(0x31, 8, msg, 0);
}

int duel_0884AF9C(int a, unsigned short b)
{
    return duel_host_call(0x31, 9, a, b | 0xFF0000);
}

int duel_0884AFB8(int a, unsigned short b, unsigned short c)
{
    return duel_host_call(0x31, 9, a, b | c << 16);
}

int duel_0884AFD8(int a, unsigned short b, unsigned int c, unsigned int d)
{
    return duel_host_call(0x31, 10, a, b | (unsigned short)((unsigned char)((c & 0xF) | 0x10) | (d & 0xFF) << 8) << 16);
}

int duel_0884B010(int a, unsigned short b, unsigned int c, unsigned int d, unsigned int e)
{
    return duel_host_call(0x31, 10, a, b | (unsigned short)((d & 0xFF) << 8 | (unsigned char)((c & 0xF) | (e & 0xF) << 4)) << 16);
}

int duel_0884B050(void)
{
    return duel_host_call(0x31, 0xB, 0, 0);
}

int duel_0884B064(int a)
{
    return duel_host_call(0x31, 0xC, a, 0);
}

int duel_0884B078(int a, int b)
{
    return duel_host_call(0x31, 0xD, a, b);
}

int duel_0884B08C(void)
{
    return duel_req_flag(0x31);
}
