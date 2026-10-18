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

typedef struct DuelResults {           /* game_state2_get(), 0x2A0 */
    unsigned char pad0[0x10];
    unsigned int frames;               /* +0x10 duel time in frames */
    int unk14;                         /* +0x14 */
    int lp_left[2];                    /* +0x18 */
    int result;                        /* +0x20 */
    int mode;                          /* +0x24 8: series of duels */
    int lp[2];                         /* +0x28 */
    unsigned char pad30[4];
    unsigned short n_duels;            /* +0x34 */
    unsigned short duels[3];           /* +0x36 result of each duel: 1, 2, 3 */
    int cur[0x28];                     /* +0x3C records set by this duel */
    int bonus[0x53];                   /* +0xDC [0x52]: total */
} DuelResults;

typedef struct DeckStats {             /* 0x68 */
    unsigned short n_monster;
    unsigned short attr[16];           /* +0x02 by card_get_attribute */
    unsigned short type[21];           /* +0x22 monsters by card_get_type */
    unsigned short kind[8];            /* +0x4C monsters by card_get_kind */
    unsigned short levels;             /* +0x5C bit per level */
    unsigned short flags;              /* +0x5E */
    unsigned short atk_max;            /* +0x60 */
    unsigned short atk_min;
    unsigned short def_max;            /* +0x64 */
    unsigned short def_min;
} DeckStats;

extern DeckStats duel_deck_stats_cur;  /* 0x0891A1A0 stats of the player's deck after a duel */
extern RecipeData duel_decks2[2];      /* 0x08919BF4 decks of players 2 and 3 */

#endif
