// @range 0x088175E4-0x0881794C
// @asm 0x088176A0-0x0881793C
// @data 0x08875AD4-0x08875E34
/* The duel-event command dispatcher (export 0x088176A0, called by libduel_mgr and friends): the
   commands 1..73 the duel manager sends to the duel scene (cut-ins, card moves, effects, panels)
   are looked up in a sorted table that also says how to unpack the three packed arguments. */
#include "dd_field_ui.h"

extern DdEvState dd_ev_state;          /* 0x08C5476C */
extern int dd_ev_sorted;               /* 0x08C54798 */

int func_08806434(int a, int b);

/* the handlers (dd_event_cmd.c); the argument shapes are in the table */
int dd_ev_01();
int dd_ev_02();
int dd_ev_03();
int dd_ev_04();
int dd_ev_05();
int dd_ev_06();
int dd_ev_07();
int dd_ev_08();
int dd_ev_09();
int dd_ev_10();
int dd_ev_11();
int dd_ev_12();
int dd_ev_13();
int dd_ev_14();
int dd_ev_15();
int dd_ev_16();
int dd_ev_17();
int dd_ev_18();
int dd_ev_19();
int dd_ev_20();
int dd_ev_21();
int dd_ev_22();
int dd_ev_23();
int dd_ev_24();
int dd_ev_25();
int dd_ev_26();
int dd_ev_27();
int dd_ev_28();
int dd_ev_29();
int dd_ev_30();
int dd_ev_31();
int dd_ev_32();
int dd_ev_33();
int dd_ev_34();
int dd_ev_35();
int dd_ev_36();
int dd_ev_37();
int dd_ev_38();
int dd_ev_39();
int dd_ev_40();
int dd_ev_41();
int dd_ev_42();
int dd_ev_43();
int dd_ev_44();
int dd_ev_45();
int dd_ev_46();
int dd_ev_47();
int dd_ev_48();
int dd_ev_49();
int dd_ev_50();
int dd_ev_51();
int dd_ev_52();
int dd_ev_53();
int dd_ev_54();
int dd_ev_55();
int dd_ev_56();
int dd_ev_57();
int dd_ev_58();
int dd_ev_59();
int dd_ev_60();
int dd_ev_61();
int dd_ev_73();

typedef struct DdEvCmd {
    int id;
    int shape;                         /* how the three arguments are unpacked (see the dispatcher) */
    int (*fn)();
} DdEvCmd;

DdEvCmd dd_ev_table[72] = {
    { 1, 0, dd_ev_01 },
    { 2, 1, dd_ev_02 },
    { 3, 0, dd_ev_03 },
    { 4, 2, dd_ev_04 },
    { 5, 1, dd_ev_05 },
    { 6, 1, dd_ev_06 },
    { 7, 3, dd_ev_07 },
    { 8, 0, dd_ev_08 },
    { 9, 1, dd_ev_09 },
    { 10, 0, dd_ev_10 },
    { 11, 3, dd_ev_11 },
    { 12, 4, dd_ev_12 },
    { 13, 2, dd_ev_13 },
    { 14, 4, dd_ev_14 },
    { 15, 0, dd_ev_15 },
    { 16, 1, dd_ev_16 },
    { 17, 2, dd_ev_17 },
    { 18, 2, dd_ev_18 },
    { 19, 1, dd_ev_19 },
    { 20, 2, dd_ev_20 },
    { 21, 2, dd_ev_21 },
    { 22, 3, dd_ev_22 },
    { 23, 5, dd_ev_23 },
    { 24, 8, dd_ev_24 },
    { 25, 7, dd_ev_25 },
    { 26, 9, dd_ev_26 },
    { 27, 10, dd_ev_27 },
    { 28, 3, dd_ev_28 },
    { 29, 5, dd_ev_29 },
    { 30, 3, dd_ev_30 },
    { 31, 5, dd_ev_31 },
    { 32, 10, dd_ev_32 },
    { 33, 3, dd_ev_33 },
    { 34, 12, dd_ev_34 },
    { 35, 11, dd_ev_35 },
    { 36, 0, dd_ev_36 },
    { 37, 11, dd_ev_37 },
    { 38, 3, dd_ev_38 },
    { 39, 2, dd_ev_39 },
    { 40, 3, dd_ev_40 },
    { 41, 0, dd_ev_41 },
    { 42, 3, dd_ev_42 },
    { 43, 4, dd_ev_43 },
    { 44, 0, dd_ev_44 },
    { 45, 4, dd_ev_45 },
    { 46, 3, dd_ev_46 },
    { 47, 3, dd_ev_47 },
    { 48, 2, dd_ev_48 },
    { 49, 6, dd_ev_49 },
    { 50, 2, dd_ev_50 },
    { 51, 3, dd_ev_51 },
    { 52, 9, dd_ev_52 },
    { 53, 3, dd_ev_53 },
    { 54, 1, dd_ev_54 },
    { 55, 12, dd_ev_55 },
    { 56, 3, dd_ev_56 },
    { 57, 2, dd_ev_57 },
    { 58, 1, dd_ev_58 },
    { 59, 4, dd_ev_59 },
    { 60, 1, dd_ev_60 },
    { 61, 1, dd_ev_61 },
    { 64, 0, 0 },
    { 65, 0, 0 },
    { 66, 0, 0 },
    { 67, 0, 0 },
    { 68, 0, 0 },
    { 69, 0, 0 },
    { 70, 0, 0 },
    { 71, 0, 0 },
    { 72, 0, 0 },
    { 73, 3, dd_ev_73 },
    { 74, 0, 0 },
};

int dd_ev_cmp(const void *a, const void *b);

void dd_ev_reset(int player)
{
    memset(&dd_ev_state, 0, sizeof(DdEvState));
    dd_ev_state.player = player;
    dd_ev_state.rival = 1 - player;
}

void dd_ev_slot_set(int i, int a, int b, int c)
{
    dd_ev_state.slot[i].a = a;
    dd_ev_state.slot[i].b = b;
    dd_ev_state.slot[i].c = c;
}

void dd_ev_8817670(int v)
{
    dd_ev_state.unk28 = v;
    func_08806434(0x10, 1);
    req_flag_set(0xC);
}

/* export: runs duel event `id` with its packed arguments; 1 when there is no handler */
#ifdef NON_MATCHING /* o flag de ordenação era static do arquivo (.bss própria): o lui sai antes do último sw do prólogo */
int func_088176A0(int id, DdEvArg a, DdEvArg b, DdEvArg c)
{
    DdEvCmd *found = 0;
    DdEvCmd *e;
    int (*fn)();
    int lo, hi, mid;
    int ret;
    DdEvCmd *tab;

    if (dd_ev_sorted == 0) {
        qsort(dd_ev_table, 72, sizeof(DdEvCmd), dd_ev_cmp);
        dd_ev_sorted = 1;
    }
    tab = dd_ev_table;
    lo = -1;
    hi = 72;
    mid = 35;
    while (lo < mid && mid < hi) {
        e = &tab[mid];
        if (id == e->id) {
            found = e;
            break;
        }
        if (id < e->id)
            hi = mid;
        else
            lo = mid;
        mid = (lo + hi) / 2;
    }
    if (found == 0 || (fn = found->fn) == 0)
        return 1;
    switch (found->shape) {
    case 0:
        ret = fn();
        break;
    case 1:
        ret = fn(a.i);
        break;
    case 2:
        ret = fn(a.i, b.i);
        break;
    case 3:
        ret = fn(a.i, b.i, c.i);
        break;
    case 4:
        ret = fn(a.i, b.i, (unsigned char)c.i, (unsigned char)((unsigned short)c.i >> 8));
        break;
    case 5:
        ret = fn(a.i, (unsigned char)b.i, (unsigned char)((unsigned short)b.i >> 8), c.i);
        break;
    case 6:
        ret = fn(a.i, b.i, c.i);
        break;
    case 7:
        ret = fn(a.i, b.s, c.s);
        break;
    case 8:
        ret = fn((unsigned short)a.i, (unsigned short)(a.i >> 16), b.s, c.s);
        break;
    case 9:
        ret = fn(a.s, b.i);
        break;
    case 10:
        ret = fn(a.s, b.i, c.i);
        break;
    case 11:
        ret = fn((unsigned char)a.i, (unsigned char)((unsigned short)a.i >> 8), b.i, c.i);
        break;
    case 12:
        ret = fn((unsigned char)a.i, (unsigned char)((unsigned short)a.i >> 8), (unsigned char)b.i, (unsigned char)((unsigned short)b.i >> 8), c.i);
        break;
    }
    return ret;
}
#endif

int dd_ev_cmp(const void *a, const void *b)
{
    return ((DdEvCmd *)a)->id - ((DdEvCmd *)b)->id;
}
