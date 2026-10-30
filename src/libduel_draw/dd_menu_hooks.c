// @range 0x0882BF00-0x0882BF84
/* Hooks of the duel menu layer: start, frame, a no-op and the cleanup of the open pickers. */
#include "dd_d3.h"

void func_08820F28(void);
void popup_init(void);
void func_08820F58(void);
void declare_free(void);
void popup_term(void);
void dd_sel_reset(void);
void func_08825588(void);
void dd_pick_term(void);

void dd_menu_hook_start(void)
{
    func_08820F28();
    popup_init();
}

void dd_menu_hook_frame(void)
{
    func_08820F58();
    declare_free();
    popup_term();
}

void dd_menu_hook_nop(void)
{
}

void dd_menu_hook_end(void)
{
    dd_sel_reset();
    func_08825588();
    dd_pick_term();
}
