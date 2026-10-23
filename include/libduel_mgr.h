/* libduel_mgr: duel scene (scene 6). Loads libduel_draw / libduel_eng helpers, runs the duel, the
   in-duel menu and the result screen. Module state and the BOOT/gmodule functions it calls that no
   shared header declares. Offsets are the ground truth; field names are partial. */
#ifndef LIBDUEL_MGR_H
#define LIBDUEL_MGR_H

#include "ehsys.h"
#include "duel_setup.h"

typedef struct DmgrGameHdr {           /* view of GameState +0x108.. used by the duel scene */
    unsigned char deck[0x108];
    unsigned short extra[3];           /* +0x108 */
    unsigned short pad10e;
    int mode;                          /* +0x110 duel_setup_mode */
    int side;                          /* +0x114 player side / first player */
    unsigned char kind;                /* +0x118 1 story, 2 tutorial, 5 free duel ... */
    unsigned char kind2;               /* +0x119 2: deck swap before the duel */
    unsigned char partner_mode;        /* +0x11A 1 single, 2 tag duel */
    unsigned char tutorial;            /* +0x11B tutorial id */
    unsigned char bgm_set;             /* +0x11C index into dmgr_bgm_sets */
    unsigned char rematch;             /* +0x11D */
    unsigned char lp_level;            /* +0x11E starting life points */
    unsigned char time_level;          /* +0x11F answer time limit */
    PlayerSlot slot[4];                /* +0x120 */
} DmgrGameHdr;

/* the deck of a PlayerSlot; the original adds the offset as an integer (two addiu, not folded) */
#define SLOT_RECIPE(s) ((RecipeData *)((int)(s) + 0x80))

typedef struct DmgrBgm {               /* sound ids per duel kind */
    unsigned short a[3];
    unsigned short b[3];
} DmgrBgm;

typedef struct DmgrSeq {               /* dmgr +0x15C, zeroed at duel start */
    unsigned short opening;            /* +0x00 the first turn's cut-in is pending */
    unsigned short pending_card;       /* +0x02 card whose activation gets a cut-in */
    unsigned short set_card;           /* +0x04 card just set */
    unsigned short skip_card;          /* +0x06 */
    unsigned short attack_on;          /* +0x08 */
    unsigned short attack_player;      /* +0x0A */
    unsigned short turn_player;        /* +0x0C */
    unsigned short attrs;              /* +0x0E field attributes of the attacker */
    unsigned short act_player;         /* +0x10 */
    unsigned short attr_cutin;         /* +0x12 */
    unsigned short target;             /* +0x14 card id of the attack target */
    unsigned short pad16;
    unsigned short mood;               /* +0x18 0 even, 1 winning, 2 losing */
    unsigned short pad1a;
    unsigned short n_monsters[4];      /* +0x1C per slot */
    unsigned short n_spells[4];        /* +0x24 */
    DmgrBgm bgm;                       /* +0x2C */
    unsigned short cur_bgm;            /* +0x38 */
    unsigned short pad3a;
    unsigned char pad3c[0x48 - 0x3C];
    int bgm_started;                   /* +0x48 */
} DmgrSeq;

typedef struct DmgrEvent {             /* one queued presentation step, 0x10 */
    unsigned short type;
    unsigned short a, b, c, d;
    unsigned short pad[3];
} DmgrEvent;

typedef struct DmgrQueue {             /* dmgr +0x58, ring of 16 events */
    unsigned short head;
    unsigned short tail;
    DmgrEvent ev[16];
} DmgrQueue;

typedef struct DmgrDrawSetup {         /* passed to libduel_draw_08805D2C, 0x40 */
    int n;
    int type[4];                       /* +0x04 PlayerSlot.type */
    int type2[4];                      /* +0x14 */
    void *deck[4];                     /* +0x24 */
    int order[2];                      /* +0x34 slot of duel players 0 and 1 */
    int pad3c;
} DmgrDrawSetup;

typedef struct DmgrModArgs {           /* modmgr_load_ex option block */
    int size;
    int a, b, c, d;
} DmgrModArgs;

typedef struct DuelMgr {               /* dmgr 0x0931BCB0, 0x92C bytes zeroed by dmgr_main */
    unsigned char pad00[0x10];
    int frame;                         /* +0x010 draw frames */
    int timer;                         /* +0x014 frames waited for input on the result */
    int pad18;
    int answered;                      /* +0x01C */
    int quit;                          /* +0x020 */
    int pad24;
    int unk28;                         /* +0x028 */
    unsigned short state;              /* +0x02C */
    unsigned short next_state;         /* +0x02E */
    unsigned short sub;                /* +0x030 */
    unsigned short fade;               /* +0x032 */
    unsigned char pad34[0x58 - 0x34];
    DmgrQueue queue;                   /* +0x058 */
    DmgrSeq seq;                       /* +0x15C */
    DmgrModArgs mod_args;              /* +0x1A8 */
    int start_loaded;                  /* +0x1BC start/start_all.ehp read */
    void *start_buf;                   /* +0x1C0 */
    int busy;                          /* +0x1C4 a module is still loading */
    int draw_loaded;                   /* +0x1C8 libduel_draw */
    int mod8_loaded;                   /* +0x1CC module 8 */
    int codecs_loaded;                 /* +0x1D0 duel_modules_load */
    int swap_loaded;                   /* +0x1D4 libdeck_swap */
    int load_step;                     /* +0x1D8 */
    unsigned char cfg_save[8];         /* +0x1DC SysSave +0x5C during free duels */
    unsigned char gs_save[0x740];      /* +0x1E4 game state at scene start */
    void *heap_big;                    /* +0x924 */
    void *heap;                        /* +0x928 */
} DuelMgr;

extern DuelMgr dmgr;

/* &gs->slot[k] the way the original adds it (base first): an accessor, not the inline expression */
inline PlayerSlot *dmgr_slot(DmgrGameHdr *gs, int k)
{
    return &gs->slot[k];
}

/* BOOT */
void *memset(void *, int, unsigned int);
void *memcpy(void *, const void *, unsigned int);
void *game_state2_get(void);
void *heap_create(void *mem, unsigned int size);
void *heap_alloc(void *heap, int size);
void heap_set_level(void *heap, void *p);
unsigned short scene_stack_peek_arg(unsigned short scene);
void scene_reset_to(unsigned short from, unsigned short scene, unsigned short arg);
void scene_pop_to(unsigned short from, unsigned short arg);
int sysvar_get_a98(void);
void sysvar_set_a8c(int v);
void cfg_set_bits_1_3_4(int v);
void cfg_set_flag(int which, int on);
void gfx_overlay_enable(int on);
void rand_seed_from_rtc(void);
void sceKernelChangeCurrentThreadAttr(int unk, int attr);
int net_get_state(void);
void net_set_state(int s);
int net_adhoc_stop(void);
void netsync_init(void);
void netsync_set_mode(int mode);
int netsync_local_side(void);
void local_record_set0(unsigned char v);
void local_record_set_u32(int v);
int peer_self_get(void);
int peer_self_u32();
void req_flags_clear(void);
int partner_count_get(void);
int partner_voice_id(unsigned int chr, unsigned int variant);
void bgm_fade(int x);
void bgm_cmd3(int x);
void bgm_cmd(int x);
void snd_cmd_fefa(void);
void snd_cmd_fefb(void);
void snd_cmd_fefb_0(void);
int snd_count_bgm(void);
int snd_count_se(void);
void snd_channel_update(int slot);
void snd_mute_off(void);
int duel_modules_load();
int duel_modules_unload(void);
int modmgr_load_by_id(int id);
int modmgr_load_ex(int id, int argsize, void *argp, int *status, void *opt);
int modmgr_update(int id);
int modmgr_none_loading(void);
int modmgr_none_loading_alias(void);
int ehfile_get_size(int dir, const char *name);
int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
int ehfile_busy(void);
unsigned int *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(unsigned int *list);
void dl_set_cur_list_base(void *p);
void *dl_get_cur_list_base(void);
unsigned int *dl_disable(unsigned int *list, int state);
int sceGuStart(int cid, void *list, int arg);
int sceGuFinish(void);
void gu_set_color_mask(unsigned int color);
void gu_set_param_50(unsigned int depth);
void gu_set_param_4c(unsigned int stencil);
void gu_set_param_494(int flags);
unsigned int font_set_color(unsigned int c);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_apply(unsigned int **list);

#endif
