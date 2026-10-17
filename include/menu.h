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
    char *title;                       /* +0x54 8-bit text */
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
    char *saved_title;                 /* +0x94 */
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
