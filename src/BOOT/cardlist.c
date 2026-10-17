// @range 0x08854920-0x08859044
// @data 0x088EF468-0x088EFFF8
// @rodata 0x088B499C-0x088B4B04
// @asm 0x08854A68-0x08854D94
// @asm 0x08854DA8-0x088552B0
// @asm 0x088554E8-0x08855690
// @asm 0x08855710-0x08855888
// @asm 0x088569D4-0x08856C88
// @asm 0x08856E08-0x08857148
// @asm 0x08857DD8-0x088580AC
// @asm 0x08858C7C-0x08858EE4
/* Card list / card panel drawing: the card picture (read through cip.c into VRAM and drawn with
   a cross-fade when the card changes), its frame from cil_0000<l>.gim, level stars, attribute
   and type icons, ATK/DEF, the description box with scroll bar and blinking arrows, and the
   comparison column used by the deck editor. Everything is drawn into the caller's display
   list from tables of 16-bit vertices {u, v, x, y, z}. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "font.h"
#include "card_db.h"
#include "allegrex.h"

typedef struct CardListCfg {           /* 0x24, copied by cardlist_init */
    void *gim;                         /* +0x00 main sprite sheet */
    void *digits_gim;                  /* +0x04 */
    void *frame_ehf;                   /* +0x08 animated card frame */
    void *attr_gim;                    /* +0x0C */
    void *kind_gim;                    /* +0x10 */
    void *cil_gim;                     /* +0x14 cil_0000<l>.gim, loaded when 0 */
    void *icon_gim;                    /* +0x18 */
    void *cip;                         /* +0x1C CIP reader handle */
    int frame_view;                    /* +0x20 */
} CardListCfg;

typedef struct CardList {              /* 0x08BE9F2C, 0x44 */
    CardListCfg cfg;
    int arrows[4];                     /* +0x24 blink timers of the scroll arrows */
    int pad[4];
} CardList;

typedef struct CardStats {             /* 0x3C */
    int attr;                          /* card_get_attribute: 0xD/0xE spell/trap */
    int attr_icon;
    int atk;                           /* -1 = "?" */
    int def;
    int type;
    int kind;
    int level;
    int prop5;
    int extra[6];                      /* +0x20 */
    int mask;                          /* +0x38 */
} CardStats;

typedef struct CardView {              /* 0x118 */
    int id;                            /* +0x000 -1 none */
    int variant;                       /* +0x004 */
    int vram;                          /* +0x008 picture buffer offset */
    CardStats cur;                     /* +0x00C */
    CardStats cmp;                     /* +0x048 compared card */
    int show_mask;                     /* +0x084 */
    int desc_scroll;                   /* +0x088 */
    int desc_max;                      /* +0x08C */
    int shown_id;                      /* +0x090 */
    int shown_variant;                 /* +0x094 */
    int fade;                          /* +0x098 cross-fade frame, 10 = done */
    CardStats shown;                   /* +0x09C */
    CardStats shown_cmp;               /* +0x0D8 */
    int shown_mask;                    /* +0x114 */
} CardView;

typedef struct CardPanel {             /* the 7-int record callers keep */
    int id;
    int vram;
    int scroll;
    int desc_max;
    int shown_id;
    int shown_variant;
    int fade;
} CardPanel;

extern CardList cardlist;              /* 0x08BE9F2C */
extern unsigned short cardlist_desc[]; /* 0x08BE8F2C type line buffer */

const unsigned short cardlist_sep[] = { 0x002F, 0 };    /* "/" */
const unsigned short cardlist_sep_jp[] = { 0x30FB, 0 }; /* katakana middle dot */

void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_tex_func(void *list, int func, int rgba);
void *dl_set_tex_filter(void *list, int min, int mag);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *dl_tex_mode(void *list, int pixfmt, int maxmips, int a2, int swizzle);
void *dl_tex_image(void *list, int level, int w, int h, int tbw, void *addr);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_color(void *list, unsigned int color);
void *dl_scissor(void *list, int x, int y, int w, int h);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
void *dl_alloc_inline(void **list, int size);
void *dl_push_const_09c(void *list);
void *dl_push_const_0a0(void *list);
void *draw_set_offsets(void *list, int tw, int th, int w, int h);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);
void *gim_set_texture(void *list, void *gim);
void *gim_set_texture_sub(void *list, void *gim, int a, int b, int c, int frame);
void *gfx_set_draw_target(void *list, int addr, int a, int w, int h, int fmt);
void *gfx_set_default_target(void *list);
unsigned int *gfx_get_state(void);
void *gfx_begin_2d(void *list);
void *gfx_end_2d(void *list);
void ehf_get_size(void *ehf, int *w, int *h);
void *ehf_draw(void *list, void *ehf, void *frame);
void ehf_set_anim(void *ehf, int first, int last);
void ehsys_0882FD74(float a, float b, float c, float d, void *ehf, int view);
void *ehp_find(void *pack, const char *name, int flags);
unsigned char *syspack_get(void);
int region_get(void);
int cfg_get_language(void);
int inv_get_hi(unsigned short id);
int int_abs(int x);
float sinf_vfpu(float x);
int ceil_to_int(float f);
unsigned short *wcscpy(unsigned short *dst, unsigned short *src);
unsigned short *wcscat(unsigned short *dst, unsigned short *src);
int wsprintf(unsigned short *out, unsigned short *fmt, ...);
int cip_is_loaded(void *h);
void *cip_read_card(void *h, int id, int variant);
int cip_query(void *h, int id, int variant);
int memcmp2(const void *a, const void *b, int n);

short cardlist_v_quad[10] = { /* 0x088EF468 */
    0, 0, 108, 0, 0, 154, 108, 0, 154, 0,
};

short cardlist_v_attr[40] = { /* 0x088EF47C */
    0, 0, 0, 0, 0, 108, 34, 108, 34, 0,
    78, 64, 108, 34, 0, 0, 49, 93, 112, 0,
    78, 49, 15, 34, 0, 0, 34, 0, 112, 0,
    0, 64, 0, 112, 0, 108, 106, 108, 154, 0,
};

short cardlist_v_pict[30] = { /* 0x088EF4CC */
    0, 0, 0, 0, 0, 78, 60, 78, 60, 0,
    80, 0, 0, 60, 0, 128, 18, 48, 78, 0,
    80, 20, 48, 60, 0, 110, 38, 78, 78, 0,
};

short cardlist_v_pict2[30] = { /* 0x088EF508 */
    80, 40, 0, 0, 0, 128, 52, 48, 12, 0,
    80, 52, 48, 0, 0, 128, 60, 96, 8, 0,
    0, 60, 48, 8, 0, 48, 64, 96, 12, 0,
};

short cardlist_v_stars[110] = { /* 0x088EF544 */
    56, 120, 88, 22, 0, 64, 128, 96, 30, 0,
    56, 120, 80, 22, 0, 64, 128, 88, 30, 0,
    56, 120, 72, 22, 0, 64, 128, 80, 30, 0,
    56, 120, 64, 22, 0, 64, 128, 72, 30, 0,
    56, 120, 56, 22, 0, 64, 128, 64, 30, 0,
    56, 120, 48, 22, 0, 64, 128, 56, 30, 0,
    56, 120, 40, 22, 0, 64, 128, 48, 30, 0,
    56, 120, 32, 22, 0, 64, 128, 40, 30, 0,
    56, 120, 24, 22, 0, 64, 128, 32, 30, 0,
    56, 120, 16, 22, 0, 64, 128, 24, 30, 0,
    56, 120, 8, 22, 0, 64, 128, 16, 30, 0,
};

short cardlist_v_stars12[130] = { /* 0x088EF620 */
    56, 120, 90, 22, 0, 64, 128, 98, 30, 0,
    56, 120, 83, 22, 0, 64, 128, 91, 30, 0,
    56, 120, 76, 22, 0, 64, 128, 84, 30, 0,
    56, 120, 69, 22, 0, 64, 128, 77, 30, 0,
    56, 120, 62, 22, 0, 64, 128, 70, 30, 0,
    56, 120, 55, 22, 0, 64, 128, 63, 30, 0,
    56, 120, 48, 22, 0, 64, 128, 56, 30, 0,
    56, 120, 41, 22, 0, 64, 128, 49, 30, 0,
    56, 120, 34, 22, 0, 64, 128, 42, 30, 0,
    56, 120, 27, 22, 0, 64, 128, 35, 30, 0,
    56, 120, 20, 22, 0, 64, 128, 28, 30, 0,
    56, 120, 13, 22, 0, 64, 128, 21, 30, 0,
    56, 120, 6, 22, 0, 64, 128, 14, 30, 0,
};

short cardlist_v_kind[180] = { /* 0x088EF724 */
    30, 0, 0, 0, 0, 128, 8, 98, 8, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    30, 8, 0, 0, 0, 128, 16, 98, 8, 0,
    16, 8, 76, 0, 0, 24, 16, 84, 8, 0,
    30, 8, 0, 0, 0, 128, 16, 98, 8, 0,
    16, 0, 76, 0, 0, 24, 8, 84, 8, 0,
    30, 16, 0, 0, 0, 128, 24, 98, 8, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    30, 24, 0, 0, 0, 128, 32, 98, 8, 0,
    8, 0, 76, 0, 0, 16, 8, 84, 8, 0,
    30, 24, 0, 0, 0, 128, 32, 98, 8, 0,
    0, 0, 76, 0, 0, 8, 8, 84, 8, 0,
    30, 24, 0, 0, 0, 128, 32, 98, 8, 0,
    16, 0, 76, 0, 0, 24, 8, 84, 8, 0,
    30, 24, 0, 0, 0, 128, 32, 98, 8, 0,
    0, 8, 76, 0, 0, 8, 16, 84, 8, 0,
    30, 24, 0, 0, 0, 128, 32, 98, 8, 0,
    8, 8, 76, 0, 0, 16, 16, 84, 8, 0,
};

short cardlist_v_atkdef[20] = { /* 0x088EF88C */
    0, 90, 1, 0, 0, 20, 99, 21, 9, 0,
    22, 90, 46, 0, 0, 42, 99, 66, 9, 0,
};

short cardlist_v_digits[100] = { /* 0x088EF8B4 */
    0, 99, 0, 0, 0, 7, 108, 7, 9, 0,
    7, 99, 0, 0, 0, 14, 108, 7, 9, 0,
    14, 99, 0, 0, 0, 21, 108, 7, 9, 0,
    21, 99, 0, 0, 0, 28, 108, 7, 9, 0,
    28, 99, 0, 0, 0, 35, 108, 7, 9, 0,
    35, 99, 0, 0, 0, 42, 108, 7, 9, 0,
    42, 99, 0, 0, 0, 49, 108, 7, 9, 0,
    49, 99, 0, 0, 0, 56, 108, 7, 9, 0,
    56, 99, 0, 0, 0, 63, 108, 7, 9, 0,
    63, 99, 0, 0, 0, 70, 108, 7, 9, 0,
};

short cardlist_v_unknown[10] = { /* 0x088EF97C */
    70, 99, 18, 0, 0, 77, 108, 25, 9, 0,
};

unsigned short cardlist_rects[97][4] = { /* 0x088EF990 */
    0, 0, 17, 17, 17, 0, 17, 17,
    34, 0, 17, 17, 51, 0, 17, 17,
    68, 0, 17, 17, 85, 0, 17, 17,
    102, 0, 17, 17, 0, 17, 17, 17,
    17, 17, 17, 17, 34, 17, 17, 17,
    51, 17, 17, 17, 68, 17, 17, 17,
    85, 17, 17, 17, 102, 17, 17, 17,
    0, 34, 17, 17, 17, 34, 17, 17,
    34, 34, 17, 17, 51, 34, 17, 17,
    68, 34, 17, 17, 85, 34, 17, 17,
    102, 34, 17, 17, 0, 51, 17, 17,
    17, 51, 17, 17, 34, 51, 17, 17,
    51, 51, 17, 17, 68, 51, 17, 17,
    85, 51, 17, 17, 102, 51, 17, 17,
    0, 68, 17, 17, 17, 68, 17, 17,
    34, 68, 17, 17, 51, 68, 17, 17,
    68, 68, 17, 17, 85, 68, 17, 17,
    102, 68, 17, 17, 85, 85, 17, 17,
    102, 85, 17, 17, 0, 120, 8, 8,
    8, 120, 8, 8, 16, 120, 8, 8,
    24, 120, 8, 8, 32, 120, 8, 8,
    40, 120, 8, 8, 48, 120, 8, 8,
    56, 120, 8, 8, 64, 120, 8, 8,
    72, 120, 8, 8, 80, 120, 8, 8,
    88, 120, 8, 8, 96, 120, 8, 8,
    104, 120, 8, 8, 0, 108, 12, 12,
    12, 108, 12, 12, 24, 108, 12, 12,
    36, 108, 12, 12, 48, 108, 12, 12,
    60, 108, 12, 12, 72, 108, 12, 12,
    84, 108, 12, 12, 96, 108, 12, 12,
    0, 99, 7, 9, 7, 99, 7, 9,
    14, 99, 7, 9, 21, 99, 7, 9,
    28, 99, 7, 9, 35, 99, 7, 9,
    42, 99, 7, 9, 49, 99, 7, 9,
    56, 99, 7, 9, 63, 99, 7, 9,
    0, 90, 20, 9, 20, 90, 20, 9,
    40, 90, 20, 9, 60, 90, 20, 9,
    119, 0, 8, 5, 119, 5, 8, 10,
    119, 15, 8, 5, 119, 20, 4, 5,
    119, 25, 4, 10, 119, 35, 4, 5,
    123, 20, 4, 5, 123, 25, 4, 10,
    123, 35, 4, 5, 119, 40, 4, 5,
    119, 45, 4, 10, 119, 55, 4, 5,
    123, 40, 4, 5, 123, 45, 4, 10,
    123, 55, 4, 5, 119, 60, 8, 7,
    119, 68, 8, 7, 119, 76, 8, 7,
    119, 84, 8, 7, 119, 92, 8, 7,
    119, 100, 8, 7, 119, 107, 8, 7,
    119, 116, 8, 7,
};

unsigned short cardlist_digit_rects[84][4] = { /* 0x088EFC98 */
    0, 0, 7, 9, 7, 0, 7, 9,
    14, 0, 7, 9, 21, 0, 7, 9,
    28, 0, 7, 9, 35, 0, 7, 9,
    42, 0, 7, 9, 49, 0, 7, 9,
    56, 0, 7, 9, 63, 0, 7, 9,
    70, 0, 7, 9, 77, 0, 7, 9,
    84, 0, 7, 9, 0, 9, 7, 9,
    7, 9, 7, 9, 14, 9, 7, 9,
    21, 9, 7, 9, 28, 9, 7, 9,
    35, 9, 7, 9, 42, 9, 7, 9,
    49, 9, 7, 9, 56, 9, 7, 9,
    63, 9, 7, 9, 70, 9, 7, 9,
    77, 9, 7, 9, 84, 9, 7, 9,
    91, 9, 7, 9, 0, 18, 9, 8,
    9, 18, 9, 8, 18, 18, 9, 8,
    27, 18, 9, 8, 36, 18, 9, 8,
    45, 18, 9, 8, 54, 18, 27, 8,
    0, 26, 9, 12, 9, 26, 9, 12,
    18, 26, 9, 12, 27, 26, 9, 12,
    36, 26, 9, 12, 45, 26, 9, 12,
    54, 26, 9, 12, 63, 26, 9, 12,
    72, 26, 9, 12, 81, 26, 9, 12,
    90, 26, 18, 12, 0, 38, 7, 8,
    7, 38, 7, 8, 14, 38, 7, 8,
    21, 38, 7, 8, 28, 38, 7, 8,
    35, 38, 7, 8, 42, 38, 7, 8,
    49, 38, 7, 8, 56, 38, 7, 8,
    63, 38, 7, 8, 70, 38, 28, 8,
    0, 46, 19, 10, 19, 46, 19, 10,
    38, 46, 19, 10, 57, 46, 19, 10,
    76, 46, 19, 10, 95, 46, 19, 10,
    0, 57, 19, 10, 19, 57, 19, 10,
    38, 57, 19, 10, 57, 57, 19, 10,
    76, 57, 19, 10, 95, 57, 19, 10,
    0, 67, 16, 16, 16, 67, 16, 16,
    32, 67, 16, 16, 48, 67, 16, 16,
    64, 67, 16, 16, 80, 67, 16, 16,
    96, 67, 16, 16, 112, 67, 16, 16,
    0, 83, 9, 9, 9, 83, 9, 9,
    18, 83, 9, 9, 27, 83, 9, 9,
    36, 83, 9, 9, 45, 83, 11, 11,
    56, 83, 11, 11, 67, 83, 11, 11,
};

unsigned short cardlist_icon_rects[6][4] = { /* 0x088EFF38 */
    4, 52, 12, 13, 16, 52, 12, 13,
    28, 52, 12, 13, 40, 52, 12, 13,
    52, 52, 12, 13, 64, 52, 12, 13,
};

typedef struct Vtx16 {                 /* GU_TEXTURE_16BIT | GU_VERTEX_16BIT */
    short u, v;
    short x, y, z;
} Vtx16;

#define VTYPE 0x800102                 /* 16-bit uv and xyz, transform 2D */

int cardlist_ready(void);
void *cardlist_draw_at(void *list, int x, int y, CardView *v);
void *cardlist_draw_pair_at(void *list, int x, int y, CardView *a, CardView *b);
void cardlist_view_update(CardView *v);
void *cardlist_draw_picture(void *list, int x, int y, void *tex, int vram, unsigned int fade, unsigned short *name, unsigned short *type_line, unsigned short *desc, CardStats *s);
void *cardlist_draw_picture_small(void *list, int x, int y, void *tex, int vram, unsigned int fade);
void *card_draw_attr_frame(void *list, int x, int y, int attr, int kind);
void *card_draw_pict(void *list, int x, int y, void *tex);
void *card_draw_pict2(void *list, int x, int y, void *tex);
void *card_draw_attr_icon(void *list, int x, int y, int attr);
void *card_draw_kind_frame(void *list, int x, int y, int attr, int sub);
void *cardlist_draw_icon(void *list, int x, int y, int attr, int sub);
void *card_draw_frame_special(void *list, int x, int y, int attr);
void *card_draw_stars(void *list, int x, int y, int n);
void *card_draw_atkdef(void *list, int x, int y, int atk, int def);
void *card_draw_ehf_frame(void *list, int x, int y, int attr, void *ehf);
void *card_draw_desc_box(void *list, int x, int y, int w, int h, int scroll, unsigned short *name, unsigned short *type_line, unsigned short *desc, int *max);
void *cardlist_draw_scrollbar(void *list, int x, int y, int h, int pos, int size);
void *cardlist_draw_arrow(void *list, int x, int y, int up, unsigned int period, unsigned int t0, unsigned int t1, unsigned int now);
void *cardlist_draw_bar(void *list, int x, int y, int w, int pos, int size);
void *card_draw_glow(void *list, int x, int y, int up, int unused, int on);
void *card_draw_atk(void *list, int x, int y, unsigned int color, int value);
void *card_draw_def(void *list, int x, int y, unsigned int color, int value);
void *card_draw_kind_icon(void *list, int x, int y, int kind);
void *card_draw_type_icon(void *list, int x, int y, int type);
void *cardlist_draw_stars(void *list, int x, int y, int n, unsigned int color);
unsigned short *card_type_line(int id, int mask, CardStats *s, CardStats *cmp);
void *card_draw_counter0(void *list, int x, int y, int n);
void *card_draw_counter1(void *list, int x, int y, int n);
void *card_draw_counter2(void *list, int x, int y, int n);
void *card_draw_counter3(void *list, int x, int y, int n);
void *card_draw_counter4(void *list, int x, int y, int n);
void *card_draw_counter5(void *list, int x, int y, int n);
void *card_draw_stats(void *list, int x, int y, unsigned int mask, CardStats *s, CardStats *cmp, int scroll);
void *card_draw_icons_row(void *list, int x, int y, int w, unsigned int mask, CardStats *s, CardStats *cmp);
void *card_draw_icons_col(void *list, int x, int y, unsigned int mask, CardStats *s, CardStats *cmp);
Vtx16 *cardlist_verts(void **list, short *tab, int n, int x, int y);
void *cardlist_sprite(void *list, int x, int y, unsigned short *r);
void *cardlist_sprite_flip(void *list, int x, int y, unsigned short *r);
void cardlist_number(int x, int y, int value);
void cardlist_text(int x, int y, unsigned short *text);
void card_draw_desc(void *list, int x, int y, int w, int h, unsigned short *line, unsigned short *text);
void *card_draw_card(void *list, int x, int y, void *tex, unsigned short *name, unsigned short *line, unsigned short *desc, CardStats *s);
void *cardlist_draw_quad(void *list, int x, int y);
void *cardlist_get_picture(int id, int variant);
void cardlist_view_from_panel(CardView *v, CardPanel *p);
void cardlist_view_to_panel(CardPanel *p, CardView *v);
void *card_draw_desc_arrows(void *list, int x, int y, int h, int w, CardView *v, int *timers);
void *card_draw_counter_icon(void *list, int x, int y, int i);
void *card_draw_counter_text(void *list, int x, int y, int n);

void cardlist_init(CardListCfg *cfg)
{
    CardList *c = &cardlist;
    unsigned char *pack;

    memset(c, 0, 0x44);
    c->cfg = *cfg;
    if (cardlist.cfg.cil_gim == 0) {
        pack = syspack_get();
        c->cfg.cil_gim = ehp_find(pack, region_get() == 0 ? "cil_0000j.gim" : "cil_0000e.gim", 0);
    }
    if (c->cfg.frame_view != 0) {
        ehsys_0882FD74(480.0f, 272.0f, 240.0f, 136.0f, c->cfg.frame_ehf, c->cfg.frame_view);
    }
}

void cardlist_clear(void)
{
    memset(&cardlist, 0, 0x44);
}

void *cardlist_draw(void *list, CardView *v)
{
    return cardlist_draw_at(list, 0, 0, v);
}

#ifdef NON_MATCHING /* register allocation of the vertex pointers differs */
void *cardlist_draw_at(void *list, int x, int y, CardView *v)
{
    unsigned short empty[1] = { 0 };
    void *tex;
    unsigned short *name;
    unsigned short *desc;
    unsigned short *line;
    CardStats *s;
    CardStats *cs;
    int mask;
    int id;

    if (cardlist_ready() && list != 0 && v != 0) {
        list = dl_enable(list, 9);
        list = dl_enable(list, 4);
        list = dl_tex_func(list, 0, 1);
        list = dl_set_tex_filter(list, 0, 0);
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        cardlist_view_update(v);
        id = v->shown_id;
        name = card_get_name(id);
        desc = card_get_desc(id);
        line = card_type_line(id, v->shown_mask, &v->shown, &v->shown_cmp);
        if (id == -1) {
            tex = 0;
        } else {
            tex = cardlist_get_picture(id, v->shown_variant);
        }
        list = cardlist_draw_picture(list, x + 5, y + 4, tex, v->vram, v->fade, name, line, desc, &v->shown);
        id = v->id;
        name = card_get_name(id);
        desc = card_get_desc(id);
        mask = v->show_mask;
        s = &v->cur;
        cs = &v->cmp;
        line = card_type_line(id, mask, s, cs);
        if (id == -1 || tex == 0) {
            list = card_draw_ehf_frame(list, x, y + 0x9D, -1, 0);
            list = card_draw_desc_box(list, x + 1, y + 0xAE, 0x87, 0x61, 0, empty, empty, empty, &v->desc_max);
            list = cardlist_draw_scrollbar(list, x + 0x8A, y + 0xB8, 0x4D, 0, 0x4B);
        } else {
            list = card_draw_ehf_frame(list, x, y + 0x9D, s->attr, 0);
            list = card_draw_stats(list, x, y + 0x9D, mask, s, cs, v->fade);
            list = card_draw_icons_col(list, x + 0x73, y + 0x9B, mask, s, cs);
            list = card_draw_desc_box(list, x + 1, y + 0xAE, 0x87, 0x61, v->desc_scroll, name, line, desc, &v->desc_max);
            list = card_draw_desc_arrows(list, x + 0x8A, y + 0xB0, 0x5D, 0x61, v, cardlist.arrows);
        }
    }
    return list;
}
#endif

void *cardlist_draw_pair(void *list, CardView *a, CardView *b)
{
    return cardlist_draw_pair_at(list, 0, 0, a, b);
}

/* Two cards side by side (deck editor comparison). */
/* Initial positions of cardlist_draw_pair_at's local block (picture, frame, icons per side). */
int cardlist_pair_init[12] = { 1, 3, 0, 107, 0, 125, 1, 167, 0, 150, 0, 137 };

#ifdef NON_MATCHING /* register allocation of the vertex pointers differs */
void *cardlist_draw_pair_at(void *list, int x, int y, CardView *a, CardView *b)
{
    struct {
        CardView *v[2];
        int pos[2][6];                 /* picture x, y; frame x, y; icons x, y */
    } t = { { 0, 0 }, { { 1, 3, 0, 107, 0, 125 }, { 1, 167, 0, 150, 0, 137 } } };
    unsigned short empty[1] = { 0 };
    CardView **pv;
    CardView *v;
    int *pos;
    void *tex;
    void *ehf;
    unsigned short *name;
    unsigned short *desc;
    unsigned short *line;
    CardStats *s;
    CardStats *cs;
    int mask;
    int id;
    int i;

    t.v[0] = a;
    t.v[1] = b;
    if (cardlist_ready()) {
        for (i = 0, pv = t.v, pos = t.pos[0]; i < 2; i++, pv++, pos += 6) {
            v = *pv;
            if (v != 0) {
                cardlist_view_update(v);
                id = v->shown_id;
                card_get_name(id);
                card_get_desc(id);
                card_type_line(id, 0, &v->shown, &v->shown);
                if (id == -1) {
                    tex = 0;
                } else {
                    tex = cardlist_get_picture(id, v->shown_variant);
                }
                list = dl_push_const_09c(cardlist_draw_picture_small(list, x + pos[0] + 0x1C, y + pos[1] + 0xD, tex, v->vram, v->fade));
            }
        }
        list = dl_disable(list, 9);
        list = dl_tex_func(list, 4, 0);
        list = dl_ambient_color(list, 0x80000000);
        list = draw_set_offsets(list, 0, 0, 0x8E, t.pos[1][5] + 0xF - t.pos[0][5]);
        list = draw_sprite(list, x + 1, y + t.pos[0][5], 0, 0, 0, 0, 0);
        for (i = 0, pv = t.v, pos = t.pos[0]; i < 2; i++, pv++, pos += 6) {
            v = *pv;
            if (v != 0) {
                ehf = i == 0 ? 0 : (void *)cardlist.cfg.frame_view;
                id = v->id;
                if (i == 0) {
                    list = dl_disable(list, 9);
                    list = dl_tex_func(list, 4, 0);
                    list = dl_ambient_color(list, 0x80000000);
                    list = draw_set_offsets(list, 0, 0, 0x87, 3);
                    list = draw_sprite(list, x + pos[0], y + pos[1] + 0x68, 0, 0, 0, 0, 0);
                }
                if (id < 1) {
                    list = card_draw_ehf_frame(list, x + pos[2], y + pos[3], -1, ehf);
                    list = card_draw_desc_box(list, x + pos[0], y + pos[1], 0x87, 0x68, 0, empty, empty, empty, &v->desc_max);
                    list = cardlist_draw_scrollbar(list, x + pos[0] + 0x89, y + pos[1] + 10, 0x54, 0, 0x52);
                } else {
                    mask = v->show_mask;
                    s = &v->cur;
                    cs = &v->cmp;
                    name = card_get_name(id);
                    desc = card_get_desc(id);
                    line = card_type_line(id, mask, s, cs);
                    list = card_draw_ehf_frame(list, x + pos[2], y + pos[3], s->attr, ehf);
                    list = card_draw_stats(list, x + pos[2], y + pos[3], mask, s, cs, v->fade);
                    list = card_draw_icons_row(list, x + pos[4], y + pos[5], 0x91, mask, s, cs);
                    list = card_draw_desc_box(list, x + pos[0], y + pos[1], 0x87, 0x68, v->desc_scroll, name, line, desc, &v->desc_max);
                    list = card_draw_desc_arrows(list, x + pos[0] + 0x89, y + pos[1] + 2, 100, 0x68, v, cardlist.arrows);
                }
            }
        }
    }
    return list;
}
#endif

/* Exported: card panel record -> drawn panel. */
void *cardlist_build(void *list, CardPanel *p)
{
    CardView v;

    cardlist_view_from_panel(&v, p);
    list = cardlist_draw(list, &v);
    cardlist_view_to_panel(p, &v);
    return list;
}

/* Exported: just the card picture of `id` (any variant). */
void card_draw(void *list, int x, int y, unsigned int id)
{
    CardStats s;
    void *tex;
    unsigned short *name;
    unsigned short *desc;
    unsigned short *line;

    list = dl_enable(list, 9);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    if (id != 0xFFFFFFFF) {
        tex = cardlist_get_picture(id, inv_get_hi(id));
        if (tex != 0) {
            name = card_get_name(id);
            desc = card_get_desc(id);
            s.attr = card_get_attribute(id);
            s.attr_icon = card_get_attribute_icon(id);
            s.atk = card_get_atk_q(id) == 0xFFFF ? -1 : card_get_atk_q(id);
            s.def = card_get_def_q(id) == 0xFFFF ? -1 : card_get_def_q(id);
            s.type = card_get_type(id);
            s.kind = card_get_kind(id);
            s.level = card_get_level(id);
            s.prop5 = card_get_prop5_hi(id);
            line = card_type_line(id, 0, &s, 0);
            card_draw_card(list, x, y, tex, name, line, desc, &s);
            return;
        }
    }
    cardlist_draw_quad(list, x, y);
}

/* Exported: like cardlist_build but only the picture. */
#ifdef NON_MATCHING /* the original loads pv->fade before pv->vram */
void *card_panel_draw(void *list, int x, int y, CardPanel *p)
{
    CardView v;
    CardView *pv = &v;
    CardStats *sh;
    void *tex;
    unsigned short *name;
    unsigned short *desc;
    unsigned short *line;
    int variant;
    int id;

    if (!cardlist_ready()) {
        return list;
    }
    if (list == 0) {
        return list;
    }
    list = dl_enable(list, 9);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    cardlist_view_from_panel(pv, p);
    cardlist_view_update(pv);
    id = pv->shown_id;
    variant = pv->shown_variant;
    name = card_get_name(id);
    desc = card_get_desc(id);
    sh = &pv->shown;
    line = card_type_line(id, pv->shown_mask, sh, &pv->shown_cmp);
    if (id == -1) {
        tex = 0;
    } else {
        tex = cardlist_get_picture(id, variant);
    }
    list = cardlist_draw_picture(list, x, y, tex, pv->vram, pv->fade, name, line, desc, sh);
    cardlist_view_to_panel(p, pv);
    return list;
}
#endif

/* Grey placeholder when the picture is not available. */
void *cardlist_draw_quad(void *list, int x, int y)
{
    CardList *c = &cardlist;
    Vtx16 *vt;

    list = gim_set_texture(list, c->cfg.cil_gim);
    vt = cardlist_verts(&list, cardlist_v_quad, 2, x, y);
    list = dl_draw_array(list, 6, VTYPE, 2, 0, vt);
    return list;
}

/* The full card: picture, attribute frame, name, level stars, type icons, ATK/DEF, text. */
#ifdef NON_MATCHING /* the parameters land in other s registers */
void *card_draw_card(void *list, int x, int y, void *tex, unsigned short *name, unsigned short *line, unsigned short *desc, CardStats *s)
{
    list = card_draw_attr_frame(list, x, y, s->attr, s->attr_icon);
    if (tex != 0) {
        list = card_draw_pict(list, x + 0xF, y + 0x22, tex);
        list = card_draw_pict2(list, x + 8, y + 9, tex);
    }
    list = card_draw_attr_icon(list, x + 0x58, y + 9, s->kind);
    if (s->attr - 0xDU < 2) {
        list = card_draw_kind_frame(list, x + 10, y + 0x16, s->attr, s->prop5);
    } else {
        list = card_draw_stars(list, x, y, s->level);
    }
    if (s->attr != 0xD && s->attr != 0xE) {
        list = card_draw_atkdef(list, x + 10, y + 0x87, s->atk, s->def);
    }
    if (s->attr - 0xDU < 2) {
        card_draw_desc(list, x + 0xB, y + 0x79, 0x56, 0x18, 0, desc);
    } else {
        card_draw_desc(list, x + 0xB, y + 0x79, 0x56, 0xE, line, desc);
    }
    return list;
}
#endif

/* Card background by attribute (and monster kind), with the separator line under the name. */
void *card_draw_attr_frame(void *list, int x, int y, int attr, int kind)
{
    void *gim = cardlist.cfg.attr_gim;
    Vtx16 *vt;
    static short sep[6] = { 11, 133, 0, 96, 133, 0 };
    short *line;
    int frame;

    if (attr == 9) {
        frame = 6;
    } else {
        switch (kind) {
        default:
        case 0:
        case 4:
        case 5:
        case 6:
            frame = 0;
            break;
        case 1:
            frame = 1;
            break;
        case 2:
            frame = 2;
            break;
        case 3:
            frame = 3;
            break;
        case 7:
            frame = 4;
            break;
        case 8:
            frame = 5;
            break;
        }
    }
    if (gim == 0) {
        return list;
    }
    list = gim_set_texture_sub(list, gim, 0, 0, 0, frame);
    vt = cardlist_verts(&list, cardlist_v_attr, 8, x, y);
    list = dl_draw_array(list, 6, VTYPE, 8, 0, vt);
    switch (kind) {
    case 0:
    case 1:
    case 2:
    case 3:
        line = dl_alloc_inline(&list, 6);
        memcpy(line, sep, 0xC);
        line[0] += x;
        line[1] += y;
        line[3] += x;
        line[4] += y;
        list = dl_disable(list, 9);
        list = dl_color(list, 0xFF808080);
        list = dl_draw_array(list, 1, 0x800100, 2, 0, line);
        list = dl_enable(list, 9);
        list = dl_color(list, 0xFFFFFFFF);
        break;
    }
    return list;
}

/* Picks the picture to show; cross-fades over 10 frames when the card changes. */
void cardlist_view_update(CardView *v)
{
    CardList *c = &cardlist;
    unsigned int f;
    void *tex;

    if (v->shown_id == 0) {
        if (cardlist_get_picture(v->id, v->variant) == 0) {
            v->shown_id = -1;
            v->shown_variant = 0;
            v->fade = 10;
        } else {
            v->fade = 10;
            v->shown_mask = v->show_mask;
            v->shown_cmp = v->cmp;
            v->shown = v->cur;
            v->shown_id = v->id;
            v->shown_variant = v->variant;
        }
    }
    if (v->id != v->shown_id || v->variant != v->shown_variant) {
        if (cip_query(c->cfg.cip, v->id, v->variant) < 0) {
            v->id = -1;
            v->variant = 0;
        }
    }
    if (v->id == v->shown_id && v->variant == v->shown_variant && memcmp2(&v->cur, &v->shown, 0x3C) == 0) {
        v->fade++;
        return;
    }
    f = v->fade;
    if (f > 10) {
        v->fade = 0;
        if (v->id > 0) {
            cardlist_get_picture(v->id, v->variant);
        }
    } else if (f == 5) {
        if (v->id > 0) {
            tex = cardlist_get_picture(v->id, v->variant);
        } else {
            tex = 0;
        }
        if (tex == 0 && v->id != -1) {
            return;
        }
        v->fade++;
        v->shown_id = v->id;
        v->shown_variant = v->variant;
        v->shown_mask = v->show_mask;
        v->shown = v->cur;
        v->shown_cmp = v->cmp;
    } else {
        v->fade = f + 1;
    }
}

/* Renders the card into VRAM at `vram`, then draws it with the fade (width shrinks to 0 at 5). */
void *cardlist_draw_picture(void *list, int x, int y, void *tex, int vram, unsigned int fade, unsigned short *name, unsigned short *line, unsigned short *desc, CardStats *s)
{
    int w;
    int k;

    list = gfx_set_draw_target(list, vram, -1, 0x80, 0xC4, 3);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    if (tex == 0) {
        list = cardlist_draw_quad(list, 0, 0);
    } else {
        list = card_draw_card(list, 0, 0, tex, name, line, desc, s);
    }
    list = gfx_set_default_target(list);
    if (fade != 0 && fade <= 9) {
        k = int_abs(fade - 5);
        w = k * 0x6C / 5;
        list = dl_ambient_color(list, (k * 0xFF / 5) << 24 | 0xFFFFFF);
    } else {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        w = 0x6C;
    }
    list = dl_push_const_0a0(list);
    list = dl_tex_func(list, 0, 0);
    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_image(list, 0, 0x80, 0x100, 0x80, (char *)sceGeEdramGetAddr() + vram);
    list = draw_set_offsets(list, 0x6C, 0x9A, w, 0x9A);
    return draw_sprite(list, x + (0x6C - w) / 2, y, 0, 0, 0, 0, 0);
}

void *cardlist_draw_picture_small(void *list, int x, int y, void *tex, int vram, unsigned int fade)
{
    int w;
    int k;

    if (tex == 0) {
        return list;
    }
    list = gfx_set_draw_target(list, vram, -1, 0x80, 0xC4, 3);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = card_draw_pict(list, 0, 0, tex);
    list = gfx_set_default_target(list);
    if (fade != 0 && fade <= 9) {
        k = int_abs(fade - 5);
        w = k * 0x4E / 5;
        list = dl_ambient_color(list, (k * 0xFF / 5) << 24 | 0xFFFFFF);
    } else {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        w = 0x4E;
    }
    list = dl_push_const_0a0(list);
    list = dl_tex_func(list, 0, 0);
    list = dl_tex_mode(list, 3, 0, 0, 0);
    list = dl_tex_image(list, 0, 0x80, 0x100, 0x80, (char *)sceGeEdramGetAddr() + vram);
    list = draw_set_offsets(list, 0x4E, 0x4E, w, 0x4E);
    return draw_sprite(list, x + (0x4E - w) / 2, y, 0, 0, 0, 0, 0);
}

void *card_draw_pict(void *list, int x, int y, void *tex)
{
    Vtx16 *vt;

    list = gim_set_texture(list, tex);
    vt = cardlist_verts(&list, cardlist_v_pict, 6, x, y);
    list = dl_draw_array(list, 6, VTYPE, 6, 0, vt);
    return list;
}

void *card_draw_pict2(void *list, int x, int y, void *tex)
{
    Vtx16 *vt;

    list = gim_set_texture(list, tex);
    vt = cardlist_verts(&list, cardlist_v_pict2, 6, x, y);
    list = dl_draw_array(list, 6, VTYPE, 6, 0, vt);
    return list;
}

void *card_draw_attr_icon(void *list, int x, int y, int attr)
{
    CardList *c = &cardlist;
    int k;

    switch (attr) {
    case 0:
        return list;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        k = attr + 0x32;
        break;
    default:
        k = 0x3B;
        break;
    case 8:
        k = 0x39;
        break;
    case 9:
        k = 0x3A;
        break;
    }
    list = gim_set_texture(list, c->cfg.gim);
    return cardlist_sprite(list, x, y, cardlist_rects[k]);
}

/* Spell/trap frame with its sub-type. */
void *card_draw_kind_frame(void *list, int x, int y, int attr, int sub)
{
    CardList *c = &cardlist;
    Vtx16 *vt;
    int k;
    int n = 4;

    switch (sub) {
    case 0:
        n = 2;
        if (attr == 0xD) {
            k = 3;
        } else if (attr == 0xE) {
            k = 0;
        } else {
            return list;
        }
        break;
    case 1:
        k = 1;
        break;
    case 2:
        k = 4;
        break;
    case 3:
        k = 5;
        break;
    case 4:
        if (attr == 0xD) {
            k = 6;
        } else if (attr == 0xE) {
            k = 2;
        } else {
            return list;
        }
        break;
    case 5:
        k = 7;
        break;
    case 6:
        k = 8;
        break;
    }
    list = gim_set_texture(list, c->cfg.kind_gim);
    vt = cardlist_verts(&list, cardlist_v_kind + k * 20, n, x, y);
    list = dl_draw_array(list, 6, VTYPE, n, 0, vt);
    return list;
}

void *cardlist_draw_icon(void *list, int x, int y, int attr, int sub)
{
    CardList *c = &cardlist;
    int k;

    switch (sub) {
    case 0:
        return list;
    case 1:
        k = 0x15;
        break;
    case 2:
        k = 0x16;
        break;
    case 3:
        k = 0x17;
        break;
    case 4:
        k = 0x18;
        break;
    case 5:
        k = 0x19;
        break;
    case 6:
        k = 0x1A;
        break;
    }
    list = gim_set_texture(list, c->cfg.gim);
    return cardlist_sprite(list, x, y, cardlist_rects[k]);
}

void *card_draw_frame_special(void *list, int x, int y, int attr)
{
    CardList *c = &cardlist;
    int k;

    switch (attr) {
    default:
        return list;
    case 0xD:
        k = 0x22;
        break;
    case 0xE:
        k = 0x23;
        break;
    }
    list = gim_set_texture(list, c->cfg.gim);
    return cardlist_sprite(list, x, y, cardlist_rects[k]);
}

void *card_draw_stars(void *list, int x, int y, int n)
{
    CardList *c = &cardlist;
    Vtx16 *vt;
    int cnt;

    if (n <= 0) {
        return list;
    }
    n = ALLEGREX_MIN(ALLEGREX_MAX(n, 1), 12);
    cnt = n << 1;
    list = gim_set_texture(list, c->cfg.gim);
    if (n < 11) {
        vt = cardlist_verts(&list, cardlist_v_stars, cnt, x, y + 1);
    } else {
        vt = cardlist_verts(&list, cardlist_v_stars12, cnt, x, y + 1);
    }
    list = dl_draw_array(list, 6, VTYPE, cnt, 0, vt);
    return list;
}

void *card_draw_atkdef(void *list, int x, int y, int atk, int def)
{
    CardList *c = &cardlist;
    Vtx16 *vt;
    int dx;

    list = gim_set_texture(list, c->cfg.gim);
    vt = cardlist_verts(&list, cardlist_v_atkdef, 4, x, y);
    list = dl_draw_array(list, 6, VTYPE, 4, 0, vt);
    if (atk < 0) {
        vt = cardlist_verts(&list, cardlist_v_unknown, 8, x + 0x13, y);
        list = dl_draw_array(list, 6, VTYPE, 2, 0, vt);
    } else {
        atk = ALLEGREX_MIN(ALLEGREX_MAX(atk, 0), 9999);
        dx = x + 0x25;
        do {
            vt = cardlist_verts(&list, cardlist_v_digits + (atk % 10) * 10, 2, dx, y);
            list = dl_draw_array(list, 6, VTYPE, 2, 0, vt);
            atk /= 10;
            dx -= 6;
        } while (atk != 0);
    }
    if (def < 0) {
        vt = cardlist_verts(&list, cardlist_v_unknown, 8, x + 0x3E, y);
        list = dl_draw_array(list, 6, VTYPE, 2, 0, vt);
    } else {
        dx = x + 0x50;
        def = ALLEGREX_MIN(ALLEGREX_MAX(def, 0), 9999);
        do {
            vt = cardlist_verts(&list, cardlist_v_digits + (def % 10) * 10, 2, dx, y);
            list = dl_draw_array(list, 6, VTYPE, 2, 0, vt);
            def /= 10;
            dx -= 6;
        } while (def != 0);
    }
    return list;
}

typedef struct EhfFrame {              /* 0x68 */
    unsigned int time;
    int unk04[2];
    int scale;                         /* +0x0C 16.16 */
    int unk10;
    float x;                           /* +0x14 */
    float y;                           /* +0x18 */
    int unk1c[19];                     /* 0x68 in all, only 0x60 cleared */
} EhfFrame;

/* Animated frame around the card (EHF), its loop range chosen by attribute. */
void *card_draw_ehf_frame(void *list, int x, int y, int attr, void *ehf)
{
    CardList *c = &cardlist;
    int w;
    int h;
    EhfFrame f;
    int lo;
    int hi;

    if (cardlist.cfg.frame_ehf == 0) {
        return list;
    }
    if (ehf == 0) {
        ehf = c->cfg.frame_ehf;
    }
    ehf_get_size(ehf, &w, &h);
    switch (attr) {
    case -1:
    case 10:
    case 11:
    case 12:
    default:
        lo = 0x1A4;
        hi = 0x1A5;
        break;
    case 0:
        lo = 0;
        hi = 0x3B;
        break;
    case 1:
    case 6:
    case 7:
    case 8:
        lo = 0x3C;
        hi = 0x77;
        break;
    case 2:
    case 3:
        lo = 0xB4;
        hi = 0xEF;
        break;
    case 4:
    case 5:
        lo = 0x78;
        hi = 0xB3;
        break;
    case 9:
        lo = 0x168;
        hi = 0x1A3;
        break;
    case 0xD:
        lo = 0xF0;
        hi = 0x12B;
        break;
    case 0xE:
        lo = 0x12C;
        hi = 0x167;
        break;
    }
    if (lo != w || hi != h) {
        ehf_set_anim(ehf, lo, hi);
    }
    memset(&f, 0, 0x60);
    f.time = gfx_get_state()[0];
    f.x = x;
    f.y = y - 0x9D;
    f.scale = 0x10000;
    list = gfx_begin_2d(list);
    list = ehf_draw(list, ehf, &f);
    list = gfx_end_2d(list);
    return dl_blend_func(list, 0, 2, 3, 0, 0);
}

/* Text box: name (magenta), type line (green), description (white), clipped and scrolled by
   `scroll` (26.6); *max receives how far it can scroll. */
#ifdef NON_MATCHING /* the original duplicates the `*max = 0` tails */
void *card_draw_desc_box(void *list, int x, int y, int w, int h, int scroll, unsigned short *name, unsigned short *type_line, unsigned short *desc, int *max)
{
    int tx = (x + 2) * 0x40;
    int ty = (y + 3) * 0x40 - scroll;
    int tw = (w - 3) * 0x40;
    unsigned short flags;
    unsigned int used;

    list = dl_disable(list, 9);
    list = dl_tex_func(list, 4, 0);
    list = dl_ambient_color(list, 0x80000000);
    list = draw_set_offsets(list, 0, 0, w, h);
    list = draw_sprite(list, x, y, 0, 0, 0, 0, 0);
    list = dl_tex_func(list, 0, 1);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_scissor(list, x + 2, y + 3, w - 3, h - 6);
    list = dl_enable(list, 2);
    if (name == 0) {
        goto zero;
    }
    if (type_line == 0) {
        *max = 0;
        goto done;
    }
    if (desc == 0) {
        goto zero;
    }
    flags = font_get_flags();
    font_text_set(list);
    font_set_flag4(0);
    font_set_size(0xC, 0xC);
    font_set_color(0xFF00FFFF);
    font_set_color(0xFF00FFFF);
    if (cfg_get_language() != 0) {
        font_set_flag4000(1);
    }
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
    } else {
        font_set_flag8(1);
    }
    font_draw_wtext_ex(tx, ty, tw, 0xFFFF, name, -1);
    ty = font_get_cursor_y();
    font_set_color(0xFF00FF00);
    font_draw_wtext_ex(tx, ty, tw, 0xFFFF, type_line, -1);
    ty = font_get_cursor_y();
    font_set_flag4000(0);
    font_set_color(0xFFFFFFFF);
    font_draw_wtext_ex(tx, ty, tw, 0xFFFF, desc, -1);
    used = font_get_cursor_y() - ((y + 3) * 0x40 - scroll);
    if (used < (unsigned int)((h - 6) * 0x40)) {
        *max = 0;
    } else {
        *max = used - (h - 6) * 0x40;
    }
    font_set_flags(flags);
    goto done;
zero:
    *max = 0;
done:
    return dl_disable(font_text_clear(), 2);
}
#endif

/* Vertical scroll bar: track of height h, thumb at pos with size. */
void *cardlist_draw_scrollbar(void *list, int x, int y, int h, int pos, int size)
{
    CardList *c = &cardlist;
    int t;

    list = gim_set_texture(list, c->cfg.gim);
    list = cardlist_sprite(list, x, y, cardlist_rects[0x4A]);
    list = cardlist_sprite(list, x, y + h - 5, cardlist_rects[0x4C]);
    list = draw_set_offsets(list, cardlist_rects[0x4B][2], cardlist_rects[0x4B][3], cardlist_rects[0x4B][2], h - 10);
    list = draw_sprite(list, x, y + 5, 0, cardlist_rects[0x4B][0], cardlist_rects[0x4B][1], 0, 0);
    pos = ALLEGREX_MIN(ALLEGREX_MAX(pos, 0), h - 12);
    size = ALLEGREX_MIN(ALLEGREX_MAX(size, 10), h - 2 - pos);
    pos += y;
    list = cardlist_sprite(list, x + 2, pos + 2, cardlist_rects[0x4D]);
    list = cardlist_sprite(list, x + 2, size + (pos - 3), cardlist_rects[0x4F]);
    list = draw_set_offsets(list, cardlist_rects[0x4E][2], cardlist_rects[0x4E][3], cardlist_rects[0x4E][2], size - 10);
    return draw_sprite(list, x + 2, pos + 7, 0, cardlist_rects[0x4E][0], cardlist_rects[0x4E][1], 0, 0);
}

/* Blinking scroll arrow: alpha from a triangle wave of `period` frames, faded out after a
   scroll stopped (t0 start, t1 stop, now current frame). */
#ifdef NON_MATCHING /* the original keeps dead copies in the slots after branch-likely */
void *cardlist_draw_arrow(void *list, int x, int y, int up, unsigned int period, unsigned int t0, unsigned int t1, unsigned int now)
{
    unsigned int half = period >> 1;
    unsigned int ph;
    float a;

    list = cardlist_sprite(list, x, y, cardlist_rects[!up + 95]);
    if (t0 != 0 && (t1 == 0 || now <= t1 + period)) {
        if (t1 == 0 || t1 <= now) {
            if (t1 == 0) {
                if (now - t0 >= half) {
                    ph = now % period;
                } else {
                    ph = (now - t0) % period;
                    if (ph >= (period - t0 % period) >> 1) {
                        ph = now % period;
                    }
                }
            } else {
                ph = t1 % period;
            }
            if (ph < half) {
                a = (float)ph / (float)half;
            } else {
                a = (float)(period - ph) / (float)half;
            }
            if (t1 != 0) {
                a = a - (float)(now - t1) / (float)half * 2.0f;
            }
            if (a < 0.0f) {
                a = 0.0f;
            }
            if (a > 1.0f) {
                a = 1.0f;
            }
            list = dl_ambient_color(list, (unsigned int)(a * 255.0f) << 24 | 0xFFFFFF);
            list = cardlist_sprite(list, x, y, cardlist_rects[((up != 0) ^ 1) + 0x59]);
            list = dl_ambient_color(list, 0xFFFFFFFF);
        }
    }
    return list;
}
#endif

/* Horizontal bar (sprites flipped). */
void *cardlist_draw_bar(void *list, int x, int y, int w, int pos, int size)
{
    CardList *c = &cardlist;
    int p;
    int bx;

    list = gim_set_texture(list, c->cfg.gim);
    list = cardlist_sprite_flip(list, x, y, cardlist_rects[0x4A]);
    list = cardlist_sprite_flip(list, x + w - 5, y, cardlist_rects[0x4C]);
    list = draw_set_offsets(list, cardlist_rects[0x4B][2], cardlist_rects[0x4B][3], w - 10, cardlist_rects[0x4B][2]);
    list = draw_sprite(list, x + 5, y, 0, cardlist_rects[0x4B][0], cardlist_rects[0x4B][1], 0, 1);
    p = ALLEGREX_MIN(ALLEGREX_MAX(pos, 0), w - 12);
    size = ALLEGREX_MIN(ALLEGREX_MAX(size, 10), w - 2 - pos);
    bx = p + x;
    list = cardlist_sprite_flip(list, bx + 2, y + 2, cardlist_rects[0x4D]);
    list = cardlist_sprite_flip(list, size + (bx - 3), y + 2, cardlist_rects[0x4F]);
    list = draw_set_offsets(list, cardlist_rects[0x4E][2], cardlist_rects[0x4E][3], size - 10, cardlist_rects[0x4E][2]);
    return draw_sprite(list, p + 7 + x, y + 2, 0, cardlist_rects[0x4E][0], cardlist_rects[0x4E][1], 0, 1);
}

/* Pulsing highlight (sine of the frame counter). */
void *card_draw_glow(void *list, int x, int y, int up, int unused, int on)
{
    unsigned int period;
    float a;

    list = cardlist_sprite_flip(list, x, y, cardlist_rects[!up + 95]);
    if (on == 0) {
        return list;
    }
    {
        period = gfx_get_state()[1];
        a = (sinf_vfpu((float)(gfx_get_state()[0] % period) / (float)period * 2.0f * 3.1415927f) + 1.0f) / 2.0f;
        if (a < 0.0f) {
            a = 0.0f;
        }
        if (a > 1.0f) {
            a = 1.0f;
        }
        list = dl_ambient_color(list, (unsigned int)(a * 255.0f) << 24 | 0xFFFFFF);
        list = cardlist_sprite_flip(list, x, y, cardlist_rects[((up != 0) ^ 1) + 0x59]);
        list = dl_ambient_color(list, 0xFFFFFFFF);
    }
    return list;
}

void *card_draw_atk(void *list, int x, int y, unsigned int color, int value)
{
    unsigned short label[] = { 'A', 'T', 'K', '/', 0 };
    unsigned short *p;
    unsigned short label_short[] = { 'A', '/', 0 };

    font_text_set(list);
    font_set_color(0xFFFFFFFF);
    font_set_size(9, 0xC);
    if (value >= 10000) {
        p = label_short;
    } else {
        p = label;
    }
    cardlist_text(x, y, p);
    font_set_color(color);
    cardlist_number(x + 10, y, value);
    return font_text_clear();
}

void *card_draw_def(void *list, int x, int y, unsigned int color, int value)
{
    unsigned short label[] = { 'D', 'E', 'F', '/', 0 };
    unsigned short *p;
    unsigned short label_short[] = { 'D', '/', 0 };

    font_text_set(list);
    font_set_color(0xFFFFFFFF);
    font_set_size(9, 0xC);
    if (value >= 10000) {
        p = label_short;
    } else {
        p = label;
    }
    cardlist_text(x, y, p);
    font_set_color(color);
    cardlist_number(x + 10, y, value);
    return font_text_clear();
}

void *card_draw_kind_icon(void *list, int x, int y, int kind)
{
    CardList *c = &cardlist;
    int k;

    switch (kind) {
    default:
        return list;
    case 1:
        k = 0x1C;
        break;
    case 2:
        k = 0x1D;
        break;
    case 3:
        k = 0x1E;
        break;
    case 4:
        k = 0x1F;
        break;
    case 5:
        k = 0x20;
        break;
    case 6:
        k = 0x21;
        break;
    case 7:
        k = 0x24;
        break;
    }
    list = gim_set_texture(list, c->cfg.gim);
    return cardlist_sprite(list, x, y, cardlist_rects[k]);
}

void *card_draw_type_icon(void *list, int x, int y, int type)
{
    CardList *c = &cardlist;
    int k;

    switch (type) {
    default:
        return list;
    case 1:
        k = 0;
        break;
    case 2:
        k = 1;
        break;
    case 3:
        k = 2;
        break;
    case 4:
        k = 3;
        break;
    case 5:
        k = 4;
        break;
    case 6:
        k = 5;
        break;
    case 7:
        k = 6;
        break;
    case 8:
        k = 7;
        break;
    case 9:
        k = 8;
        break;
    case 10:
        k = 9;
        break;
    case 11:
        k = 10;
        break;
    case 12:
        k = 11;
        break;
    case 13:
        k = 12;
        break;
    case 14:
        k = 13;
        break;
    case 15:
        k = 14;
        break;
    case 16:
        k = 15;
        break;
    case 17:
        k = 16;
        break;
    case 18:
        k = 17;
        break;
    case 19:
        k = 18;
        break;
    case 20:
        k = 19;
        break;
    case 21:
        k = 20;
        break;
    }
    list = gim_set_texture(list, c->cfg.gim);
    return cardlist_sprite(list, x, y, cardlist_rects[k]);
}

/* Level: star icon and the count in digits. */
void *cardlist_draw_stars(void *list, int x, int y, int n, unsigned int color)
{
    CardList *c = &cardlist;

    if (n <= 0) {
        return list;
    }
    n = ALLEGREX_MIN(ALLEGREX_MAX(n, 1), 12);
    list = gim_set_texture(list, c->cfg.gim);
    list = cardlist_sprite(list, x, y + 1, cardlist_rects[0x2B]);
    list = dl_ambient_color(list, color);
    list = gim_set_texture(list, c->cfg.digits_gim);
    return cardlist_sprite(list, x + 9, y, cardlist_digit_rects[n + 0x37]);
}

/* Exported: type line of card `id` (uses its own stats). */
unsigned short *card_info_fill(int id)
{
    CardStats s;

    s.attr = card_get_attribute(id);
    s.attr_icon = card_get_attribute_icon(id);
    s.atk = card_get_atk_q(id) == 0xFFFF ? -1 : card_get_atk_q(id);
    s.def = card_get_def_q(id) == 0xFFFF ? -1 : card_get_def_q(id);
    s.type = card_get_type(id);
    s.kind = card_get_kind(id);
    s.level = card_get_level(id);
    s.prop5 = card_get_prop5_hi(id);
    return card_type_line(id, 0, &s, &s);
}

/* "[Type / Effect]" style line in cardlist_desc. */
unsigned short *card_type_line(int id, int mask, CardStats *s, CardStats *cmp)
{
    unsigned short *open = card_db_table_sel(0x19);
    unsigned short *close = card_db_table_sel(0x1A);
    unsigned short *sep = cfg_get_language() == 0 ? (unsigned short *)cardlist_sep_jp : (unsigned short *)cardlist_sep;

    wcscpy(cardlist_desc, open);
    switch (s->attr_icon) {
    case 0:
    case 1:
    case 2:
    case 3:
        wcscat(cardlist_desc, s->type == 0 ? dlg_get_text(0x20F) : card_db_table_sel(s->type));
        if (s->attr != 0) {
            wcscat(cardlist_desc, card_db_table_sel2(s->attr));
        }
        break;
    case 7:
    case 8:
        wcscat(cardlist_desc, dlg_table_sel(s->kind + 2));
        if (s->prop5 != 0) {
            open = dlg_table_sel2(s->prop5);
            wcscat(cardlist_desc, sep);
            wcscat(cardlist_desc, open);
        }
        break;
    }
    wcscat(cardlist_desc, close);
    return cardlist_desc;
}

void *card_draw_counter0(void *list, int x, int y, int n)
{
    return card_draw_counter_text(card_draw_counter_icon(list, x, y, 0), x + 0xC, y + 1, n);
}

void *card_draw_counter1(void *list, int x, int y, int n)
{
    return card_draw_counter_text(card_draw_counter_icon(list, x, y, 1), x + 0xC, y + 1, n);
}

void *card_draw_counter2(void *list, int x, int y, int n)
{
    return card_draw_counter_text(card_draw_counter_icon(list, x, y, 2), x + 0xC, y + 1, n);
}

void *card_draw_counter3(void *list, int x, int y, int n)
{
    return card_draw_counter_text(card_draw_counter_icon(list, x, y, 3), x + 0xC, y + 1, n);
}

void *card_draw_counter4(void *list, int x, int y, int n)
{
    return card_draw_counter_text(card_draw_counter_icon(list, x, y, 4), x + 0xC, y + 1, n);
}

void *card_draw_counter5(void *list, int x, int y, int n)
{
    return card_draw_counter_text(card_draw_counter_icon(list, x, y, 5), x + 0xC, y + 1, n);
}

int cardlist_ready(void)
{
    int bad = 0;

    bad |= !cardlist.cfg.gim;
    bad |= !cardlist.cfg.kind_gim;
    bad |= !cardlist.cfg.cil_gim;
    bad |= !cardlist.cfg.cip;
    return !bad;
}

/* Stats strip under the picture: spell/trap kind icon and scrolling name, or ATK/DEF (red or
   blue against the compared card), kind and type icons. Bits of `mask` pick cmp fields. */
#ifdef NON_MATCHING /* register allocation differs throughout */
void *card_draw_stats(void *list, int x, int y, unsigned int mask, CardStats *s, CardStats *cmp, int scroll)
{
    unsigned short *name;
    unsigned int color;
    int attr;
    int sub;
    int w;
    int tw;
    int ty;

    list = dl_color(list, 0xFFFFFFFF);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    attr = (mask & 1) ? cmp->attr : s->attr;
    if (attr - 0xDU < 2) {
        sub = (mask & 0x80) ? cmp->prop5 : s->prop5;
        list = cardlist_draw_icon(list, x + 0x7F, y, attr, sub);
        w = sub == 0 ? 0x7F : 0x6E;
        list = card_draw_frame_special(list, x + w, y, attr);
        if (mask & 0x20000) {
            name = card_get_name(cmp->mask);
            list = dl_scissor(list, x, y, w, 0x18);
            list = dl_enable(list, 2);
            font_text_set(list);
            font_set_color(0xFFFFFFFF);
            font_set_size(9, 0xC);
            tw = (font_measure_text(name) + 0x3F) >> 6;
            if (w < tw) {
                scroll %= tw + 0x14;
            } else {
                scroll = 0;
            }
            ty = (y + 4) * 0x40;
            x -= scroll;
            font_draw_text(x * 0x40, ty, (0x1E0 - x) * 0x40, name);
            if (scroll > 0) {
                tw += x + 0x14;
                font_draw_text(tw * 0x40, ty, (0x1E0 - tw) * 0x40, name);
            }
            list = dl_disable(font_text_clear(), 2);
        }
    } else {
        attr = (mask & 4) ? cmp->atk : s->atk;
        if (attr == s->atk) {
            color = 0xFFFFFFFF;
        } else if (attr < s->atk) {
            color = 0xFF8888FF;
        } else {
            color = 0xFFFF8888;
        }
        list = card_draw_atk(list, x + 1, y + 4, color, attr);
        attr = (mask & 8) ? cmp->def : s->def;
        if (attr == s->def) {
            color = 0xFFFFFFFF;
        } else if (attr < s->def) {
            color = 0xFF8888FF;
        } else {
            color = 0xFFFF8888;
        }
        list = card_draw_def(list, x + 0x39, y + 4, color, attr);
        attr = (mask & 0x20) ? cmp->kind : s->kind;
        list = dl_color(list, 0xFFFFFFFF);
        list = card_draw_kind_icon(list, x + 0x6E, y, attr);
        attr = (mask & 0x10) ? cmp->type : s->type;
        list = card_draw_type_icon(list, x + 0x7F, y, attr);
    }
    return list;
}
#endif

/* Extra icons laid out to the right. */
void *card_draw_icons_row(void *list, int x, int y, int w, unsigned int mask, CardStats *s, CardStats *cmp)
{
    int attr;
    int i;

    attr = (mask & 1) ? cmp->attr : s->attr;
    if (attr != 0xD && attr != 0xE) {
        list = cardlist_draw_stars(list, x + w - 0x1C, y + 2, (mask & 0x40) ? cmp->level : s->level, 0xFFFFFFFF);
    }
    if (mask & 0x4000) {
        list = card_draw_type_icon(list, x, y, cmp->mask);
        x += 0x12;
    } else if (mask & 0x8000) {
        list = card_draw_kind_icon(list, x, y, cmp->mask);
        x += 0x12;
    } else if (mask & 0x10000) {
        for (i = 1; i <= 7; i++) {
            if (cmp->mask & (1 << i)) {
                list = card_draw_kind_icon(list, x, y, i);
                x += 0x12;
            }
        }
    }
    if ((mask & 0x200) && cmp->extra[1] >= 0) {
        list = card_draw_counter0(list, x, y, cmp->extra[1]);
        x += 0x1E;
    }
    if ((mask & 0x100) && cmp->extra[0] >= 0) {
        list = card_draw_counter1(list, x, y, cmp->extra[0]);
        x += 0x1E;
    }
    if ((mask & 0x400) && cmp->extra[2] >= 0) {
        list = card_draw_counter2(list, x, y, cmp->extra[2]);
        x += 0x1E;
    }
    if ((mask & 0x800) && cmp->extra[2] >= 0) {
        list = card_draw_counter3(list, x, y, cmp->extra[3]);
        x += 0x1E;
    }
    if ((mask & 0x1000) && cmp->extra[2] >= 0) {
        list = card_draw_counter4(list, x, y, cmp->extra[4]);
        x += 0x1E;
    }
    if ((mask & 0x2000) && cmp->extra[2] >= 0) {
        list = card_draw_counter5(list, x, y, cmp->extra[5]);
    }
    return list;
}

/* Extra icons stacked upwards. */
void *card_draw_icons_col(void *list, int x, int y, unsigned int mask, CardStats *s, CardStats *cmp)
{
    int attr;
    int i;

    attr = (mask & 1) ? cmp->attr : s->attr;
    if (attr != 0xD && attr != 0xE) {
        list = cardlist_draw_stars(list, x, (y -= 0xD) + 2, (mask & 0x40) ? cmp->level : s->level, 0xFFFFFFFF);
    }
    if (mask & 0x4000) {
        y -= 0x12;
        list = card_draw_type_icon(list, x, y, cmp->mask);
    } else if (mask & 0x8000) {
        y -= 0x12;
        list = card_draw_kind_icon(list, x, y, cmp->mask);
    } else if (mask & 0x10000) {
        for (i = 1; i <= 7; i++) {
            if (cmp->mask & (1 << i)) {
                y -= 0x12;
                list = card_draw_kind_icon(list, x, y, i);
            }
        }
    }
    if ((mask & 0x200) && cmp->extra[1] >= 0) {
        y -= 0xD;
        list = card_draw_counter0(list, x, y, cmp->extra[1]);
    }
    if ((mask & 0x100) && cmp->extra[0] >= 0) {
        y -= 0xD;
        list = card_draw_counter1(list, x, y, cmp->extra[0]);
    }
    if ((mask & 0x400) && cmp->extra[2] >= 0) {
        y -= 0xD;
        list = card_draw_counter2(list, x, y, cmp->extra[2]);
    }
    if ((mask & 0x800) && cmp->extra[2] >= 0) {
        y -= 0xD;
        list = card_draw_counter3(list, x, y, cmp->extra[3]);
    }
    if ((mask & 0x1000) && cmp->extra[2] >= 0) {
        y -= 0xD;
        list = card_draw_counter4(list, x, y, cmp->extra[4]);
    }
    if ((mask & 0x2000) && cmp->extra[2] >= 0) {
        list = card_draw_counter5(list, x, y - 0xD, cmp->extra[5]);
    }
    return list;
}

/* Copies n vertices from tab into the display list, offset by (x, y). */
Vtx16 *cardlist_verts(void **list, short *tab, int n, int x, int y)
{
    Vtx16 *v = dl_alloc_inline(list, n * 10);
    int i;

    memcpy(v, tab, n * 10);
    for (i = 0; i < n; i++) {
        v[i].x += x;
        v[i].y += y;
    }
    return v;
}

void *cardlist_sprite(void *list, int x, int y, unsigned short *r)
{
    list = draw_set_offsets(list, r[2], r[3], r[2], r[3]);
    return draw_sprite(list, x, y, 0, r[0], r[1], 0, 0);
}

void *cardlist_sprite_flip(void *list, int x, int y, unsigned short *r)
{
    list = draw_set_offsets(list, r[2], r[3], r[3], r[2]);
    return draw_sprite(list, x, y, 0, r[0], r[1], 0, 1);
}

/* Right-aligned decimal number (max 99999), "?" when negative. */
void cardlist_number(int x, int y, int value)
{
    if (value > 99999) {
        value = 99999;
    }
    font_set_size(10, 0xC);
    if (value < 0) {
        unsigned short q[] = { '?', 0 };

        cardlist_text(x + 0x23, y, q);
    } else {
        x += 0x23;
        do {
            char d[4] = { 0 };

            d[0] = value % 10 + '0';
            cardlist_text(x, y, (unsigned short *)d);
            value /= 10;
            x -= 7;
        } while (value > 0);
    }
}

void cardlist_text(int x, int y, unsigned short *text)
{
    unsigned short flags = font_get_flags();

    font_set_flag4(1);
    font_draw_text(x << 6, y << 6, (0x1E0 - x) * 0x40, text);
    font_set_flags(flags);
}

void card_draw_desc(void *list, int x, int y, int w, int h, unsigned short *line, unsigned short *text)
{
    int f;

    font_text_set(list);
    font_set_size(4, 4);
    font_set_color(0x80000000);
    f = font_get_flag4();
    font_set_flag4(0);
    font_draw_wtext_ex(x << 6, y << 6, w << 6, h << 6, text, -1);
    font_set_flag4(f);
    font_text_clear();
}

void *cardlist_get_picture(int id, int variant)
{
    void *h = cardlist.cfg.cip;

    if (!cip_is_loaded(h)) {
        return 0;
    }
    return cip_read_card(h, id, variant);
}

void cardlist_view_from_panel(CardView *v, CardPanel *p)
{
    int k;

    memset(v, 0, 0x118);
    v->fade = p->fade;
    v->shown_id = p->shown_id;
    v->shown_variant = p->shown_variant;
    v->shown.attr = card_get_attribute(p->shown_id);
    v->shown.attr_icon = card_get_attribute_icon(p->shown_id);
    k = card_get_atk_q(p->shown_id);
    v->shown.atk = k == 0xFFFF ? -1 : card_get_atk_q(p->shown_id);
    k = card_get_def_q(p->shown_id);
    v->shown.def = k == 0xFFFF ? -1 : card_get_def_q(p->shown_id);
    v->shown.type = card_get_type(p->shown_id);
    v->shown.kind = card_get_kind(p->shown_id);
    v->shown.level = card_get_level(p->shown_id);
    v->shown.prop5 = card_get_prop5_hi(p->shown_id);
    v->shown_mask = 0;
    v->vram = p->vram == 0 ? 0x1E8000 : p->vram;
    if (p->id == -1) {
        v->id = -1;
    } else {
        v->id = p->id;
        v->variant = inv_get_hi(p->id);
        v->cur.attr = card_get_attribute(p->id);
        v->cur.attr_icon = card_get_attribute_icon(p->id);
        k = card_get_atk_q(p->id);
        v->cur.atk = k == 0xFFFF ? -1 : card_get_atk_q(p->id);
        k = card_get_def_q(p->id);
        v->cur.def = k == 0xFFFF ? -1 : card_get_def_q(p->id);
        v->cur.type = card_get_type(p->id);
        v->cur.kind = card_get_kind(p->id);
        v->cur.level = card_get_level(p->id);
        v->cur.prop5 = card_get_prop5_hi(p->id);
        v->show_mask = 0;
        v->desc_scroll = p->scroll;
    }
}

void cardlist_view_to_panel(CardPanel *p, CardView *v)
{
    p->desc_max = v->desc_max;
    p->fade = v->fade;
    p->shown_id = v->shown_id;
    p->shown_variant = v->shown_variant;
}

/* Scroll bar of the description plus the two blinking arrows; timers[0..1] up, [2..3] down. */
#ifdef NON_MATCHING /* register allocation and scheduling differ */
void *card_draw_desc_arrows(void *list, int x, int y, int h, int w, CardView *v, int *timers)
{
    int hh = h - 0x10;
    unsigned int scroll = v->desc_scroll;
    unsigned int total = ((unsigned int)v->desc_max >> 6) + w;
    int pos = (scroll >> 6) * (hh - 2) / total;

    list = cardlist_draw_scrollbar(list, x, y + 8, hh, pos, ceil_to_int((float)w * (float)(hh - 2) / (float)(int)total));
    if (v->desc_scroll != 0) {
        if (timers[0] == 0) {
            timers[0] = gfx_get_state()[0];
        }
    } else if (timers[0] != 0 && timers[1] == 0) {
        timers[1] = gfx_get_state()[0];
    }
    if (timers[0] != 0 && timers[1] != 0 && gfx_get_state()[0] - timers[1] > 0xF) {
        timers[0] = 0;
        timers[1] = 0;
    }
    list = cardlist_draw_arrow(list, x, y, 1, gfx_get_state()[1], timers[0], timers[1], gfx_get_state()[0]);
    if ((unsigned int)v->desc_scroll < (unsigned int)v->desc_max) {
        if (timers[2] == 0) {
            timers[2] = gfx_get_state()[0];
        }
    } else if (timers[2] != 0 && timers[3] == 0) {
        timers[3] = gfx_get_state()[0];
    }
    if (timers[2] != 0 && timers[3] != 0 && gfx_get_state()[0] - timers[3] > 0xF) {
        timers[2] = 0;
        timers[3] = 0;
    }
    return cardlist_draw_arrow(list, x, y + 9 + hh, 0, gfx_get_state()[1], timers[2], timers[3], gfx_get_state()[0]);
}
#endif

void *card_draw_counter_icon(void *list, int x, int y, int i)
{
    CardList *c = &cardlist;

    if (cardlist.cfg.icon_gim == 0) {
        return list;
    }
    list = gim_set_texture(list, c->cfg.icon_gim);
    return cardlist_sprite(list, x, y, cardlist_icon_rects[i]);
}

void *card_draw_counter_text(void *list, int x, int y, int n)
{
    unsigned short fmt[] = { 'x', '%', 'd', 0 };
    unsigned short buf[16];
    int flags;

    wsprintf(buf, fmt, n);
    flags = font_get_flags();
    font_text_set(list);
    font_set_size(9, 0xB);
    font_set_color(0xFFFFFFFF);
    font_set_flag4(1);
    font_draw_text(x << 6, y << 6, (0x1E0 - x) * 0x40, buf);
    list = font_text_clear();
    font_set_flags(flags);
    return list;
}
