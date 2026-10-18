#ifndef DUEL_SETUP_H
#define DUEL_SETUP_H

#include "game.h"

typedef struct DuelSetup {             /* 0x089139F0: how the duel was started */
    int mode;                          /* +0x000 */
    int view_player;                   /* +0x004 duel_view_player */
    int unk8;
    int unkC;
    int unk10;                         /* +0x010 tag duel */
    RecipeData deck[2];                /* +0x014 decks of players 0 and 1 */
    int unk224;                        /* +0x224 */
    int unk228;                        /* +0x228 */
} DuelSetup;

extern DuelSetup duel_setup;
extern RecipeData duel_decks2[2];      /* 0x08919BF4 decks of players 2 and 3 */

#endif
