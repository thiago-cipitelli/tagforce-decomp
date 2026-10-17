/* System menu/window layer (BOOT 0x088433F0..0x08845AC0): up to 16 items per Menu, each built
   from a caller-owned MenuItemDef and rendered into a display list on layer 4. */
#ifndef MENU_H
#define MENU_H

typedef struct MenuItemDef {           /* 0x78 bytes, owned by the caller */
    int has_text;                      /* +0x00 nonzero: text is drawn */
    int unk04;
    int unk08;
    short x;                           /* +0x0C */
    short y;                           /* +0x0E */
    short order;                       /* +0x10 draw order (menu_layout sorts by it) */
    short unk12;
    int unk14[3];
    short w;                           /* +0x20 */
    short h;                           /* +0x22 */
    int unk24[5];
    unsigned short *text;              /* +0x38 UCS-2 */
    int font_flag4;                    /* +0x3C */
    int font_flag8000;                 /* +0x40 */
    short pad_l;                       /* +0x44 */
    short pad_r;                       /* +0x46 */
    short pad_t;                       /* +0x48 */
    short pad_b;                       /* +0x4A */
    unsigned char font_size;           /* +0x4C */
    unsigned char unk4d[3];
    int unk50[10];
} MenuItemDef;

typedef struct MenuItem {              /* 0xC0 bytes, heap-allocated */
    int unk00;
    int unk04;
    MenuItemDef *def;                  /* +0x08 */
    MenuItemDef cur;                   /* +0x0C copy of *def taken at build time */
    int state;                         /* +0x84 1 opening, 2/4 open, 3 closing */
    int unk88[6];
    int unkA0;                         /* +0xA0 animation counter */
    int unkA4[2];
    int unkAC;
    int unkB0[4];
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
void menu_layout(void *dl, Menu *m);
int menu_remove_item(Menu *m, MenuItemDef *def);
int menu_item_open(Menu *m, MenuItemDef *def);
int menu_item_close(Menu *m, MenuItemDef *def);
int menu_item_is_open(Menu *m, MenuItemDef *def);
int menu_cursor_y(Menu *m, MenuItemDef *def);
void menu_item_rect(Menu *m, MenuItemDef *def, int *x, int *y, int *w, int *h);
int menu_item_state(Menu *m, MenuItemDef *def);
void menu_slot_free(MenuItem **slot, void *heap);
/* menu_draw.c */
void menu_item_build(void *dl, MenuItem *it, Menu *m);
void menu_item_build_text(void *dl, MenuItem *it, Menu *m);
int menu_find_free(Menu *m);
int menu_find_item(Menu *m, MenuItemDef *def);

#endif
