// @range 0x088589FC-0x08859CC0
// @asm 0x08858A64-0x08859170
// @asm 0x08859170-0x08859CC0
/* Debug test menu of the duel scene (UTF-16 labels MRK, Name, FaceChange, Up/Down, TurnChange,
   Attack/Defence, Undo, FildMagic, GameSet, Win/Lose, Draw): selects a card (4007..6957) and
   fires opponent-model events. Not reachable in the retail game; the draw (0x08858A64) and the
   input handler (0x08859170) stay in asm, without C. */
#include "dd_duelui.h"
#include "allegrex.h"

typedef struct DebugMenu {             /* 0x08C5B2D0, 0x28 */
    int card;                          /* +0x00 */
    unsigned short *name;              /* +0x04 */
    int f8[7];
    int sel;                           /* +0x24 -1 = card list */
} DebugMenu;

extern DebugMenu debug_menu;
extern int debug_last_card;            /* 0x08C5B2FC */

void dv_0883603C(void);

void debug_menu_init(void)
{
    memset(&debug_menu, 0, sizeof(debug_menu));
    debug_menu.sel = -1;
    debug_menu.card = ALLEGREX_MIN(ALLEGREX_MAX(debug_last_card, 4007), 6957);
    debug_menu.name = card_get_name(debug_menu.card);
}

void debug_menu_term(void)
{
    dv_0883603C();
}

#ifdef NON_MATCHING
void debug_menu_draw(void);   /* 0x08858A64, not decompiled (debug only) */
int debug_menu_update(void);  /* 0x08859170, not decompiled (debug only) */
#endif
