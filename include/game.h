/* BOOT's game-level globals: one 0x107A4-byte block at 0x08A66A80 zeroed by loading_screen_init, holding
   runtime flags, the save state ("sysstate", 0x08A66B98) and the per-session game state. Field names
   are partial; offsets are the ground truth. */
#ifndef GAME_H
#define GAME_H

typedef struct SysSave {               /* 0x08A66B98, 0xFCA8 bytes */
    int unk0;
    unsigned int flags;                /* +0x04 bit flags (sysflag_test/set) */
    unsigned int play_counter;         /* +0x08 frames played, saturating */
    unsigned char unk0c[0x3DF8 - 0xC];
    unsigned char event_seen[0x40];    /* +0x3DF8 stat_event_record */
    unsigned char flag_3e38[0xFCA8 - 0x3E38];
} SysSave;

typedef struct GameGlobals {           /* 0x08A66A80 */
    unsigned int sysflags;             /* +0x000 */
    unsigned int rand_state;           /* +0x004 MSVC LCG */
    int unk08;
    int var_a8c;                       /* +0x00C set to 1 after config changes */
    int var_a90;                       /* +0x010 nonzero inhibits the soft reset */
    int var_a94;
    int var_a98;                       /* +0x018 gates net start */
    int overlay_enabled;               /* +0x01C loading overlay drawn each frame */
    int unk20;
    void *loading_ehf;                 /* +0x024 "loading.ehf" from pack_loading */
    unsigned char info[0x8C];          /* +0x028 sys_info_get() */
    unsigned int frame_counter;        /* +0x0B4 */
    unsigned int req_flags[24];        /* +0x0B8 */
    SysSave save;                      /* +0x118 */
    unsigned char game_state[0x740];   /* +0xFDC0 slots of 0x188 */
    unsigned char game_state2[0x2A0];  /* +0x10500 */
    void *heap;                        /* +0x107A0 heap_create(game_heap_mem, 0x4000) */
} GameGlobals;

extern GameGlobals g_game;
extern unsigned char game_heap_mem[0x4000];   /* 0x08A62A80 */
extern unsigned char dl_heap[];               /* 0x08906A80, 0x15C000 bytes */

/* EHP archives embedded in BOOT's .data */
extern unsigned char pack_chr_name[];   /* chr_name_?.bin */
extern unsigned char pack_sysgfx[];     /* icons, fonts */
extern unsigned char pack_recipes[];    /* RCP*.ydc */
extern unsigned char pack_loading[];    /* loading.ehf + gims */
extern unsigned char pack_sysmsg[];     /* sysmsg_?.bin */

SysSave *sysstate_get(void);
unsigned int rand_next(void);
void srand(unsigned int seed);

#endif
