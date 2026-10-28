// @range 0x088075F4-0x08808E68
// @asm 0x0880798C-0x08807B88
// @data 0x0881C02C-0x0881C0E0
/* Script instructions 0xC351..0xC37E: messages, the choice window, the random branch and the
   conditional blocks (each tests a story value and, when false, skips to its end marker op + 0x23). */
#include "libstory.h"

#define CMD (story_vm.script[story_vm.pc])

typedef struct StoryChoiceArg {        /* argument of the field command 0x14 (choice window) */
    unsigned short **lines;
    int count;
} StoryChoiceArg;

int st_remap_812334(int id);
void stalk_say(int a, int b, int c, int op, int text);
void stalk_bu_enter(int a, int b, int c);
void stalk_bu_enter2(int a, int b, int c, int d);
void stalk_bu_hide_all(int a);
void stalk_bu_close_all(void);
void stalk_bu_wait(void);
void stalk_show(int text, int a);
int stalk_msg_close(void);
void stalk_msg_wait(void);
void stext_split(int text, unsigned short **lines, int *count);
int st_pl_item_has(int a);
int st_pl_card_has(int a);
int st_pl_x1c_is(int a);
int st_pl_x20_is(int a);
int st_pl_cards_full(void);
int st_pl_card_last_is(int a);
int st_id_to_chr(int id);
int st_id_to_rec(int id);
int st_chr_affinity(int chr);
int st_chr_is_best(int chr);
int st_chr_look(int chr);
int st_mail_has(int id);
int scal_get_month(void);
int scal_get_day(void);
int st_pl_points_get(void);
int st_pl_x14_is(int chr);
int st_pl_x154_get(void);
int st_pl_x156_get(void);
int st_pl_x158_get(void);
int st_chr_named(int chr);
int sysstate_get_u32_54(int i);
unsigned short sysstate2_rec_get_u16(int which, int i);
DeckName *deck_name_get(int i);
int deck_equals_recipe(int n);
int deck_slot_get_level(int i);
int cfg_is_mode1(void);

int vm_text(int id);
int vm_skip(unsigned int op, unsigned int a, unsigned int b, unsigned int c);
int vm_cmp(unsigned int v, unsigned int ref, int mode);
int vm_test(int v, int mode);
int vm_test2(int v, int mode);

void op_msg(void)
{
    int a = CMD.a;
    int b = st_remap_812334(CMD.b);
    int cc = CMD.c;
    int op = CMD.op;

    stalk_say(a, b, cc, op, vm_text(CMD.d));
}

void op_msg_c353(void)
{
    int a = CMD.a;
    int b = st_remap_812334(CMD.b);

    stalk_bu_enter(a, b, CMD.c);
}

void op_msg_pair(void)
{
    StoryVm *vm = &story_vm;
    int first, second;

    switch (CMD.op) {
    case 0xC354:
        first = vm->pc;
        second = first + 1;
        break;
    case 0xC355:
        second = vm->pc;
        first = second + 1;
        break;
    }
    switch (vm->script[vm->pc + 1].op) {
    case 0xC354: case 0xC355: {
        int a = vm->script[first].a;
        int b = st_remap_812334(vm->script[first].b);
        int e = vm->script[second].a;

        stalk_bu_enter2(a, b, e, st_remap_812334(vm->script[second].b));
        vm->pc++;
        break;
    }
    }
}

void op_c356(void)
{
    stalk_bu_hide_all(CMD.a);
    stalk_bu_wait();
}

void op_c357(void)
{
    stalk_bu_close_all();
    stalk_bu_wait();
}

void op_c358(void)
{
    stalk_show(vm_text(CMD.d), 0);
}

void op_c359(void)
{
    int text = vm_text(CMD.d);

    stalk_show(text, CMD.a);
}

void op_c35a(void)
{
    if (stalk_msg_close())
        stalk_msg_wait();
}

void op_nop(void)
{
}

/* choice window: the text of the previous instruction split in lines; branch n of the block */
#ifdef NON_MATCHING /* a tradução resposta->ramo reaproveita as constantes das comparações de outro jeito */
void op_choice(void)
{
    StoryVm *vm = &story_vm;
    unsigned short *lines[5];
    int count;
    StoryChoiceArg arg;
    int res;
    int text;
    int n;

    switch (CMD.a) {
    case 1:
        text = vm_text(vm->script[vm->pc - 1].d);
        memset(vm->text, 0, sizeof(vm->text));
        lines[0] = vm->text[0];
        lines[1] = vm->text[1];
        lines[2] = vm->text[2];
        lines[3] = vm->text[3];
        lines[4] = vm->text[4];
        stext_split(text, lines, &count);
        if (count == 0)
            break;
        arg.lines = lines;
        arg.count = count;
        if (fld_cmd_start(0x14, &arg) == 0)
            break;
        while (!fld_cmd_poll(0x14, &res))
            sys_frame_wait();
        switch (res) {
        case 1: n = 2; break;
        case 2: n = 3; break;
        case 3: n = 4; break;
        case 4: n = 5; break;
        default: n = 1; break;
        }
        if (n != 1)
            vm_skip(0xC35C, n, 0, 0);
        break;
    case 2: case 3: case 4: case 5:
    default:
        vm_skip(0xC35C, 11, 0, 0);
        break;
    case 11:
        break;
    }
}
#endif

/* random branch among the next n */
void op_random(void)
{
    StoryVm *const vm = &story_vm;
    unsigned int n, r;

    switch (CMD.a) {
    case 1:
        switch (vm->script[vm->pc].b) {
        case 0: case 1: default: n = 1; break;
        case 2: n = 2; break;
        case 3: n = 3; break;
        case 4: n = 4; break;
        case 5: n = 5; break;
        case 6: n = 6; break;
        case 7: n = 7; break;
        case 8: n = 8; break;
        case 9: n = 9; break;
        case 10: n = 10; break;
        }
        switch (rand_next() % n) {
        case 0: default: r = 1; break;
        case 1: r = 2; break;
        case 2: r = 3; break;
        case 3: r = 4; break;
        case 4: r = 5; break;
        case 5: r = 6; break;
        case 6: r = 7; break;
        case 7: r = 8; break;
        case 8: r = 9; break;
        case 9: r = 10; break;
        }
        if (r != 1)
            vm_skip(0xC35D, r, 0, 0);
        break;
    case 0: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10:
    default:
        vm_skip(0xC35D, 11, 0, 0);
        break;
    case 11:
        break;
    }
}

void op_if_c35e(void)
{
    int v = st_pl_item_has(CMD.a);

    if (!vm_test2(v, CMD.b))
        vm_skip(0xC37F, 0, 0, 0);
}

void op_if_c35f(void)
{
    int v = st_pl_card_has(CMD.a);

    if (!vm_test2(v, CMD.b))
        vm_skip(0xC380, 0, 0, 0);
}

void op_if_c360(void)
{
    int v = st_pl_x1c_is(CMD.a);

    if (!vm_test(v, CMD.b))
        vm_skip(0xC381, 0, 0, 0);
}

void op_if_c361(void)
{
    int v = st_pl_x20_is(CMD.a);

    if (!vm_test(v, CMD.b))
        vm_skip(0xC382, 0, 0, 0);
}

void op_if_c362(void)
{
    int v = st_pl_cards_full();

    if (!vm_test(v, CMD.a))
        vm_skip(0xC383, 0, 0, 0);
}

void op_if_c363(void)
{
    int v = st_pl_card_last_is(CMD.a);

    if (!vm_test(v, CMD.b))
        vm_skip(0xC384, 0, 0, 0);
}

void op_if_c364(void)
{
    int v = st_chr_affinity(st_id_to_chr(CMD.a));

    if (!vm_cmp(v, CMD.b, CMD.c))
        vm_skip(0xC385, 0, 0, 0);
}

void op_if_c365(void)
{
    int v = st_chr_is_best(st_id_to_chr(CMD.a));

    if (!vm_test(v, CMD.b))
        vm_skip(0xC386, 0, 0, 0);
}

void op_if_c366(void)
{
    int v = sysstate_get_u32_54(0);

    if (!vm_cmp(v, CMD.a, CMD.b))
        vm_skip(0xC387, 0, 0, 0);
}

void op_if_c367(void)
{
    int v = story_flag_get(CMD.a);

    if (!vm_test(v, CMD.b))
        vm_skip(0xC388, 0, 0, 0);
}

void op_if_c368(void)
{
    op_if_c367();
}

void op_if_c369(void)
{
    int chr = st_id_to_chr(CMD.a);
    int v = st_chr_flag_get(chr, CMD.b);

    if (!vm_test(v, CMD.c))
        vm_skip(0xC38A, 0, 0, 0);
}

void op_if_c36a(void)
{
    int v = st_chr_level(st_id_to_chr(CMD.a));

    if (!vm_cmp(v, CMD.b, CMD.c))
        vm_skip(0xC38B, 0, 0, 0);
}

void op_if_c36b(void)
{
    StoryVm *const vm = &story_vm;
    int n = inv_get_count(CMD.a) != 0;

    if (!vm_test2(n, vm->script[vm->pc].b))
        vm_skip(0xC38C, 0, 0, 0);
}

void op_if_c36c(void)
{
    int v = inv_get_bit3(CMD.a);

    if (!vm_test2(v, CMD.b))
        vm_skip(0xC38D, 0, 0, 0);
}

void op_if_c36d(void)
{
    StoryVm *const vm = &story_vm;
    int set = deck_name_get(CMD.a)->set == 1;

    if (!vm_test2(set, vm->script[vm->pc].b))
        vm_skip(0xC38E, 0, 0, 0);
}

void op_if_c36e(void)
{
    int v = deck_equals_recipe(CMD.a);

    if (!vm_test2(v, CMD.b))
        vm_skip(0xC38E, 0, 0, 0);
}

void op_if_c36f(void)
{
    int v = st_mail_has(CMD.a);

    if (!vm_test2(v, CMD.b))
        vm_skip(0xC390, 0, 0, 0);
}

void op_if_c370(void)
{
    StoryVm *const vm = &story_vm;
    int v = st_chr_look(st_id_to_chr(CMD.a));

    if (!vm_test(v == CMD.b, vm->script[vm->pc].c))
        vm_skip(0xC391, 0, 0, 0);
}

void op_if_c371(void)
{
    int v = scal_get_month();

    if (!vm_cmp(v, CMD.a, CMD.b))
        vm_skip(0xC392, 0, 0, 0);
}

void op_if_c372(void)
{
    int v = scal_get_day();

    if (!vm_cmp(v, CMD.a, CMD.b))
        vm_skip(0xC393, 0, 0, 0);
}

void op_if_c373(void)
{
    int v = sdate_get_0c();

    if (!vm_cmp(v, CMD.a, CMD.b))
        vm_skip(0xC394, 0, 0, 0);
}

void op_if_c374(void)
{
    int v = st_pl_points_get();

    if (!vm_cmp(v, CMD.a, CMD.b))
        vm_skip(0xC395, 0, 0, 0);
}

void op_if_c375(void)
{
    int v = st_pl_x14_is(st_id_to_chr(CMD.a));

    if (!vm_test(v, CMD.b))
        vm_skip(0xC396, 0, 0, 0);
}

void op_if_c376(void)
{
    int v = st_pl_x154_get();

    if (!vm_cmp(v, CMD.a, CMD.b))
        vm_skip(0xC397, 0, 0, 0);
}

void op_if_c377(void)
{
    int v = st_pl_x156_get();

    if (!vm_cmp(v, CMD.a, CMD.b))
        vm_skip(0xC398, 0, 0, 0);
}

void op_if_c378(void)
{
    StoryVm *const vm = &story_vm;
    int v = st_pl_x158_get();
    int n;

    if (v == 0)
        v = 1;
    switch (vm->script[vm->pc].a) {
    case 1: n = 1; break;
    case 2: n = 2; break;
    case 3: n = 3; break;
    case 4: n = 4; break;
    case 5: n = 5; break;
    case 6: n = 6; break;
    case 7: n = 7; break;
    case 8: n = 8; break;
    case 9: n = 9; break;
    case 10: n = 10; break;
    case 11: n = 11; break;
    default: n = 1; break;
    }
    if (!vm_test(v == n, vm->script[vm->pc].b))
        vm_skip(0xC399, 0, 0, 0);
}

void op_if_c379(void)
{
    StoryVm *const vm = &story_vm;
    int i = 2;
    int found;

    do {
        switch (deck_slot_get_level(i)) {
        case 1: case 2: case 3: found = 1; break;
        default: found = 0; break;
        }
        if (found)
            break;
    } while (++i < 0x98);
    if (!vm_test2(found, vm->script[vm->pc].a))
        vm_skip(0xC39A, 0, 0, 0);
}

void op_if_c37a(void)
{
    int v = cfg_is_mode1();

    if (!vm_test2(v, CMD.a))
        vm_skip(0xC39B, 0, 0, 0);
}

void op_if_c37b(void)
{
    unsigned short v = sysstate2_rec_get_u16(0, st_id_to_rec(CMD.a));

    if (!vm_cmp(v, CMD.b, CMD.c))
        vm_skip(0xC39C, 0, 0, 0);
}

void op_if_c37c(void)
{
    unsigned short v = sysstate2_rec_get_u16(1, st_id_to_rec(CMD.a));

    if (!vm_cmp(v, CMD.b, CMD.c))
        vm_skip(0xC39D, 0, 0, 0);
}

void op_if_c37d(void)
{
    unsigned short v = sysstate2_rec_get_u16(2, st_id_to_rec(CMD.a));

    if (!vm_cmp(v, CMD.b, CMD.c))
        vm_skip(0xC39E, 0, 0, 0);
}

void op_if_c37e(void)
{
    int v = !st_chr_named(st_id_to_chr(CMD.a));

    if (!vm_test(v, CMD.b))
        vm_skip(0xC39F, 0, 0, 0);
}
