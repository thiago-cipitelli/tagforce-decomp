/* System menu/window layer (BOOT 0x088433F0..0x08845AC0): up to 16 items per Menu, each built
   from a caller-owned MenuItemDef and rendered into a display list on layer 4. */
#ifndef MENU_H
#define MENU_H

typedef struct MenuItemDef {           /* 0x78 bytes, owned by the caller */
    int visible;                       /* +0x00 nonzero: frame and text are drawn */
    void *gim;                         /* +0x04 custom texture, else the menu's window01.gim */
    unsigned char style;               /* +0x08 0..3 frame A, 4..7 frame B */
    unsigned char style2;              /* +0x09 texel row of the title pieces */
    unsigned char tab;                 /* +0x0A 0 none, 1..4 tab piece position */
    unsigned char pad0b;
    short x;                           /* +0x0C */
    short y;                           /* +0x0E */
    short z;                           /* +0x10 draw order (menu_layout sorts by it) */
    short pad12;
    int auto_size;                     /* +0x14 size the frame to the text */
    unsigned short max_w;              /* +0x18 0 = screen */
    unsigned short max_h;              /* +0x1A */
    unsigned short min_w;              /* +0x1C */
    unsigned short min_h;              /* +0x1E */
    short w;                           /* +0x20 */
    short h;                           /* +0x22 */
    unsigned int color;                /* +0x24 frame color */
    short tab_x;                       /* +0x28 */
    short pad2a;
    int show_title;                    /* +0x2C */
    int auto_title;                    /* +0x30 size the title bar to its text */
    short title_w;                     /* +0x34 */
    short title_h;                     /* +0x36 */
    unsigned short *text;              /* +0x38 UCS-2 */
    int font_flag4;                    /* +0x3C */
    int font_flag8000;                 /* +0x40 */
    short pad_l;                       /* +0x44 */
    short pad_r;                       /* +0x46 */
    short pad_t;                       /* +0x48 */
    short pad_b;                       /* +0x4A */
    unsigned char font_size;           /* +0x4C */
    unsigned char pad4d[3];
    unsigned int text_color;           /* +0x50 */
    unsigned short *title;             /* +0x54 UCS-2 */
    unsigned char title_font_size;     /* +0x58 */
    unsigned char pad59[3];
    unsigned int title_color;          /* +0x5C */
    int open_anim;                     /* +0x60 0 none, 1 fade, 2 fade + slide */
    int close_anim;                    /* +0x64 */
    float scale_x;                     /* +0x68 */
    float scale_y;                     /* +0x6C */
    int anchor;                        /* +0x70 0..6, see menu_anchor_rect */
    int pad74;
} MenuItemDef;

typedef struct WVertex {               /* GE vertex: 16-bit uv, 8888 color, 16-bit xyz (0x80011E) */
    short u, v;
    unsigned int color;
    short x, y, z;
    short pad;
} WVertex;

typedef struct MenuItem {              /* 0xC0 bytes, heap-allocated */
    WVertex *verts;                    /* +0x00 frame sprites in the display list */
    int nverts;                        /* +0x04 */
    MenuItemDef *def;                  /* +0x08 */
    MenuItemDef cur;                   /* +0x0C copy of *def taken at build time */
    int state;                         /* +0x84 0 new, 1 opening, 2 open, 3 closing, 4 closed */
    short saved_y;                     /* +0x88 def fields saved during the animations */
    short pad8a;
    int saved_show_title;              /* +0x8C */
    unsigned short *saved_text;        /* +0x90 */
    unsigned short *saved_title;       /* +0x94 */
    unsigned int saved_text_color;     /* +0x98 */
    unsigned int saved_title_color;    /* +0x9C */
    int counter;                       /* +0xA0 animation step */
    int ghost;                         /* +0xA4 draw the second (ghost) frame */
    WVertex *verts2;                   /* +0xA8 */
    int nverts2;                       /* +0xAC */
    short x2;                          /* +0xB0 ghost frame placement */
    short y2;                          /* +0xB2 */
    float scale_x2;                    /* +0xB4 */
    float scale_y2;                    /* +0xB8 */
    unsigned int color2;               /* +0xBC */
} MenuItem;

typedef struct Menu {                  /* 0x4C bytes */
    MenuItem *items[16];
    int count;                         /* +0x40 */
    void *window_gim;                  /* +0x44 "window01.gim" from the system pack */
    void *heap;                        /* +0x48 */
} Menu;

typedef struct WPiece {                /* a window01.gim sub-rectangle, in texels */
    short x, y, w, h;
} WPiece;

/* .rodata of the menu unit (0x08875600..0x088756D8), defined in menu.c */
extern const char window01_gim[];
extern const WPiece wframe_a[10];      /* style 0..3 frame: 4 corners, top, left, right, bottom, center, tab */
extern const WPiece wtitle[4];         /* title bar pieces */
extern const int wstep_a[2];           /* texel offset per style (x, y) for frame A */
extern const WPiece wframe_b[9];       /* style 4..7 frame */
extern const int wstep_b[2];
/* frame border sizes used for the content rectangle */
#define MENU_PAD_Y (wframe_a[4].h)
#define MENU_PAD_X (wframe_a[5].w)
#define MENU_PAD_W (wframe_a[6].w)
#define MENU_PAD_H (wframe_a[7].h)

void menu_init(Menu *m, void *heap);
int menu_add_item(Menu *m, MenuItemDef *def);
void menu_free_items(Menu *m);
void menu_layout(void **dl, Menu *m);
int menu_remove_item(Menu *m, MenuItemDef *def);
int menu_item_open(Menu *m, MenuItemDef *def);
int menu_item_close(Menu *m, MenuItemDef *def);
int menu_item_is_open(Menu *m, MenuItemDef *def);
int menu_cursor_y(Menu *m, MenuItemDef *def);
void menu_item_rect(Menu *m, MenuItemDef *def, int *x, int *y, int *w, int *h);
int menu_item_state(Menu *m, MenuItemDef *def);
void menu_slot_free(MenuItem **slot, void *heap);
/* menu_draw.c */
void menu_item_build(void **dl, MenuItem *it, Menu *m);
void menu_item_build_text(void **dl, MenuItem *it, Menu *m);
void menu_item_build_frame(void *dl, MenuItem *it, Menu *m, int ghost);
int menu_item_sprite_count(MenuItem *it, int ghost);
void menu_anchor_rect(float *tl, float *br, int x, int y, int w, int h, int anchor);
void menu_sprite(WVertex *v, void *heap, const WPiece *p, int x, int y, int z, int w, int h, unsigned int color, const int *uvoff, int flip);
int menu_find_free(Menu *m);
int menu_find_item(Menu *m, MenuItemDef *def);
int mem_differs(const void *a, const void *b, unsigned int n);
void mem_swap(void *heap, void *a, void *b, int n);
unsigned int color_lerp(float t, unsigned int a, unsigned int b);

#endif

/* ---- menu widget (menu_widget.c): a window with a list and/or OK/YES/NO/CANCEL buttons ---- */
typedef struct MenuWidget MenuWidget;
typedef void *(*MenuDrawItemFn)(void *dl, int index, int x, int y);
typedef void *(*MenuDrawRectFn)(void *dl, int x, int y, int w, int h);
typedef void *(*MenuDrawCursorFn)(void *dl, MenuWidget *w, int x, int y, int cw, int ch);

struct MenuWidget {                    /* 0x120 */
    Menu menu;                         /* +0x000 */
    MenuItemDef def;                   /* +0x04C the window */
    void *heap;                        /* +0x0C4 */
    unsigned char no_input : 1;        /* +0x0C8 */
    unsigned char no_cancel : 1;       /* cancel does not close */
    unsigned char cancel_blocked : 1;  /* cancel only beeps */
    unsigned char keep_open : 1;       /* stays open after accept/cancel */
    unsigned char f0_4 : 1;
    unsigned char cancel_resets : 1;   /* cancel returns to the initial cursor/button */
    unsigned char accept_blocked : 1;
    unsigned char no_wrap : 1;         /* the cursor stops at the ends instead of wrapping */
    unsigned char skip_disabled : 1;   /* +0x0C9 */
    unsigned char center : 1;
    unsigned char keep_dl : 1;         /* menu_draw restarts the list at the saved position */
    unsigned char style : 4;           /* 0 none, odd = horizontal list, even = vertical */
    unsigned char f1_7 : 1;
    unsigned char buttons : 4;         /* +0x0CA 0 none, 1 OK, 2 OK/CANCEL, 3 YES/NO, 4 YES/NO/CANCEL */
    unsigned char f2_hi : 4;
    unsigned char pad0cb;
    unsigned short count;              /* +0x0CC list entries */
    unsigned char rows;                /* +0x0CE visible entries, 0 = all */
    unsigned char pad0cf;
    unsigned short scroll;             /* +0x0D0 */
    unsigned char cursor : 5;          /* +0x0D2 */
    unsigned char pad0d2 : 3;
    unsigned char button : 4;          /* +0x0D3 */
    unsigned char pad0d3 : 4;
    unsigned char initial;             /* +0x0D4 initial cursor / button */
    unsigned char pad0d5[3];
    unsigned int selected;             /* +0x0D8 bit per entry: drawn highlighted */
    unsigned int disabled;             /* +0x0DC bit per entry */
    short arrow_off;                   /* +0x0E0 */
    short pad0e2;
    void *icon_gim;                    /* +0x0E4 buttom_icon1.gim */
    unsigned char cancelled : 1;       /* +0x0E8 */
    unsigned char accepted : 1;
    unsigned char pad0e8 : 6;
    unsigned char state;               /* +0x0E9 0 idle, 1 opening, 2 open, 3 active, 4 blink, 5 closing, 6 closed */
    unsigned char pad0ea[2];
    int blink;                         /* +0x0EC */
    unsigned short item_w;             /* +0x0F0 */
    unsigned short item_h;             /* +0x0F2 */
    unsigned short item_x;             /* +0x0F4 */
    unsigned short item_y;             /* +0x0F6 */
    unsigned short cursor_w;           /* +0x0F8 */
    unsigned short cursor_h;           /* +0x0FA */
    MenuDrawItemFn draw_item;          /* +0x0FC */
    MenuDrawRectFn draw_all;           /* +0x100 replaces the whole content */
    MenuDrawRectFn draw_before;        /* +0x104 */
    MenuDrawRectFn draw_after;         /* +0x108 */
    MenuDrawCursorFn draw_cursor;      /* +0x10C */
    MenuDrawCursorFn draw_cursor_post; /* +0x110 */
    MenuDrawCursorFn draw_selected;    /* +0x114 */
    MenuDrawCursorFn draw_selected_post; /* +0x118 */
    PadState *pad;                     /* +0x11C input override */
};

void menu_open(MenuWidget *w);
void menu_draw(void **dl, MenuWidget *w);
int menu_update(MenuWidget *w);
void menu_set_button(MenuWidget *w, unsigned char b);
void menu_set_cursor(MenuWidget *w, int index);
void menu_finish(MenuWidget *w);
void menu_reset(MenuWidget *w);
void *menu_draw_button(void *dl, MenuWidget *w, int x, int y, int which, int lit);
void *menu_draw_cursor(void *dl, MenuWidget *w, int x, int y, int cw, int ch);
void menu_draw_selected(void *dl, MenuWidget *w, int x, int y, int cw, int ch);
void *menu_draw_box(void *dl, int x, int y, int w, int h); /* devolve a lista (menu_draw_cursor repassa) */
int menu_item_counter(MenuWidget *w);
void menu_cursor_prev(MenuWidget *w);
void menu_cursor_prev1(MenuWidget *w);
void menu_cursor_next(MenuWidget *w);
void menu_cursor_next1(MenuWidget *w);
void *menu_draw_arrow(void *dl, int x, int y, int dir);
