/* libpassword: card password screen (Password Machine). Shared state and the BOOT (libehsys) calls it
   uses; prototypes copy the BOOT definitions (parameter types decide the caller's masking). */
#ifndef LIBPASSWORD_H
#define LIBPASSWORD_H

#include "ehsys.h"
#include "font.h"
#include "psp.h"

/* resource ids (res_table) */
#define RES_BG          0x1001  /* bg.gim */
#define RES_MONITOR     0x1002  /* monitor_shadow.gim */
#define RES_ICON01      0x1003
#define RES_ICON03      0x1004
#define RES_FRAME       0x1005
#define RES_SPELLTRAP   0x1006  /* spelltrap_%c.gim */
#define RES_KEYPAD      0x1010  /* key.ehf: the 3x4 keypad */
#define RES_DIGIT0      0x1011  /* nixi.ehf x8: the nixie-tube digits, copied per slot */
#define RES_SCORE       0x1019  /* score-atkdef10.ehf */
#define RES_STRINGS     0x1020  /* strTbl_%c.bin */

/* keys of the keypad (key_at): 0-9 digits, then */
#define KEY_CLEAR   10
#define KEY_BACK    11
#define KEY_ENTER   12

/* digit glyphs (digit_set_anim): 0-9, then */
#define GLYPH_BLANK  10
#define GLYPH_CURSOR 12

typedef struct PasswordState {         /* 0x08B0B670, 0x1140 bytes cleared by password_init */
    unsigned short scene;              /* +0x00 password_update state */
    unsigned short step;               /* +0x02 step inside the scene state */
    unsigned char key_anim : 4;        /* +0x04 0 idle, 1 key press animating, 3 reset */
    unsigned char col : 4;             /*       keypad cursor column 0-2 */
    unsigned char row : 4;             /* +0x05 keypad cursor row 0 (enter) - 4 */
    unsigned char pad05 : 4;
    unsigned char ndigits : 4;         /* +0x06 digits typed */
    unsigned char pad06 : 4;
    unsigned char digits[8];           /* +0x07 */
    int password;                      /* +0x10 */
    unsigned short text_len;           /* +0x14 glyph count of the message being typed out */
    unsigned short name_pos;           /* +0x16 */
    unsigned short text_pos;           /* +0x18 glyphs of text shown */
    unsigned short text2_pos;          /* +0x1A glyphs of text2 shown */
    unsigned short name_shown[0x45];   /* +0x1C card name typed out so far */
    unsigned short name_full[0x45];    /* +0xA6 */
    unsigned short text[0x400];        /* +0x130 message window */
    unsigned short text2[0x400];       /* +0x930 second message */
    unsigned short card_id;            /* +0x1130 */
    unsigned short timer;              /* +0x1132 */
    unsigned char text_skip : 1;       /* +0x1134 */
    unsigned char typing : 1;          /*         a message is being typed out */
    unsigned char pad1134 : 6;
    void *heap;                        /* +0x1138 */
    void *cip;                         /* +0x113C card image reader */
} PasswordState;

typedef struct PasswordLoad {          /* 0x08B177C0 */
    unsigned char ready : 1;           /* no file read in flight */
    unsigned char step : 7;
} PasswordLoad;

extern PasswordState pw;
extern PasswordLoad pw_load;
extern void *pw_pack;                  /* labo/password.ehp */

void *res_get(unsigned short id);
void keypad_reset(void);
void digits_refresh(void);
int card_is_forbidden(unsigned short id);
void key_set_anim(unsigned short key, short mode);
int key_anim_done(void);
void digit_set_anim(unsigned short slot, unsigned short glyph);
unsigned char key_at(short col, short row);
void key_select(unsigned short key);

/* BOOT */
extern void *memset(void *, int, unsigned int);
extern void *memcpy(void *, const void *, unsigned int);
extern int sprintf(char *, const char *, ...);
extern void qsort(void *, unsigned int, unsigned int, int (*)(const void *, const void *));
extern int cfg_get_language(void);
extern unsigned short region_pick_value(int flag);
extern void se_play(int id);
extern void bgm_play(int id);
extern void bgm_stop(int x);
extern void bgm_fade(int x);
extern void bgm_cmd(int x);
extern void bgm_cmd3(int x);
extern void snd_cmd_fefa(void);
extern int snd_count_se();
extern void fade_start(unsigned char type, unsigned int color, unsigned short frames);
extern void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
extern int fade_is_done(void);
extern int fade_apply(void **list);
extern void gfx_overlay_enable(int on);
extern void *heap_create(void *mem, unsigned int size);
extern void *heap_alloc(void *heap, int n);
extern int ehfile_get_size(int dir, const char *name);
extern int ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);
extern int ehp_mount(void (*cb)(void *), void *pack);
extern void *ehp_find(void *pack, const char *name, int *size);
extern int ehp_get_size_cur(void *pack, void *data);
extern void *cip_open_h(void *handle, int bufsize, int prefetch);
extern int cip_is_loaded(void *c);
extern void cip_close(void *c);
extern int cip_is_idle(void *c);
extern unsigned int cip_read_card(void *handle, int id, int variant);
extern void cardlist_init(void *cfg);
extern void cardlist_clear(void);
extern void card_draw(void *list, int x, int y, unsigned int id);
extern int card_id_from_password(unsigned int pass);
extern int card_get_flag31(int id);
extern unsigned short *card_get_name(int id);
extern void inv_set_bit0(unsigned short id);
extern int inv_get_hi(unsigned short id);
extern unsigned short *table_get_entry(void *t, unsigned int i);
extern unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
extern int wtext_count_tags(unsigned short *src);
extern void wtext_strip_tags(unsigned short *dst, unsigned short *src);
extern int wsprintf(unsigned short *out, unsigned short *fmt, ...);
extern int ehf_set_anim(void *e, int target, int limit);
extern int ehf_get_frame_count(void *e);
extern void ehf_get_size(void *e, int *target, int *limit);
extern void *ehf_draw(void *list, void *e, void *opts);

#endif
