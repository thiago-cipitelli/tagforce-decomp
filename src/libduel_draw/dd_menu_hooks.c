// @range 0x0882BF00-0x0882BF84
/* Hooks of the duel menu layer: start, frame, a no-op and the cleanup of the open pickers. */
#include "libduel_draw.h"

void func_08820F28(void);
void func_08866EA0(void);
void func_08820F58(void);
void func_08866074(void);
void func_08866EB4(void);
void func_08819C04(void);
void func_08825588(void);
void dd_pick_term(void);

void dd_menu_hook_start(void)
{
    func_08820F28();
    func_08866EA0();
}

void dd_menu_hook_frame(void)
{
    func_08820F58();
    func_08866074();
    func_08866EB4();
}

void dd_menu_hook_nop(void)
{
}

void dd_menu_hook_end(void)
{
    func_08819C04();
    func_08825588();
    dd_pick_term();
}
