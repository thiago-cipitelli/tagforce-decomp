// @range 0x08828DD0-0x088299C8
// @asm 0x08828DD0-0x0882923C
// @data 0x08876710-0x088769E0
/* Window frames drawn along a polyline (the field zone highlights): for each segment an edge
   sprite and, at each turn, a corner sprite taken from window01.gim with one of 8 colour styles;
   plus a single stretched sprite between two points. */
#include "libduel_draw.h"

typedef struct DdWPiece {              /* texel rectangle of window01.gim */
    unsigned short u, v, w, h;
} DdWPiece;

typedef struct DdWVert {               /* GE vertex: 16-bit texture and position, through mode */
    short u, v;
    short x, y, z;
} DdWVert;

typedef DdWVert *(*DdWPieceFn)(DdWVert *v, int x0, int y0, int x1, int y1);

unsigned char *syspack_get(void);
void *ehp_find(void *pack, const char *name, int *size);
void *gim_set_texture(void *list, void *gim);
void *dl_alloc_inline(unsigned int **list, int size);
unsigned int *dl_draw_array(unsigned int *list, int prim, int vtype, int count, void *indices, void *verts);

extern int dd_wline_style;

DdWVert *dd_wline_corner_tl(DdWVert *v, int x0, int y0, int x1, int y1);
DdWVert *dd_wline_edge_top(DdWVert *v, int x0, int y0, int x1, int y1);
DdWVert *dd_wline_corner_tr(DdWVert *v, int x0, int y0, int x1, int y1);
DdWVert *dd_wline_edge_right(DdWVert *v, int x0, int y0, int x1, int y1);
DdWVert *dd_wline_corner_br(DdWVert *v, int x0, int y0, int x1, int y1);
DdWVert *dd_wline_edge_bottom(DdWVert *v, int x0, int y0, int x1, int y1);
DdWVert *dd_wline_corner_bl(DdWVert *v, int x0, int y0, int x1, int y1);
DdWVert *dd_wline_edge_left(DdWVert *v, int x0, int y0, int x1, int y1);
DdWVert *dd_wline_inner_up_right(DdWVert *v, int x0, int y0, int x1, int y1);
DdWVert *dd_wline_inner_down_right(DdWVert *v, int x0, int y0, int x1, int y1);
DdWVert *dd_wline_inner_up_left(DdWVert *v, int x0, int y0, int x1, int y1);
DdWVert *dd_wline_inner_down_left(DdWVert *v, int x0, int y0, int x1, int y1);

DdWPiece dd_wline_pieces[8][11] = {
    {
        { 0, 0, 4, 4 }, { 27, 0, 5, 4 }, { 0, 12, 4, 5 }, { 27, 12, 5, 5 },
        { 4, 0, 1, 4 }, { 0, 4, 4, 1 }, { 27, 4, 5, 1 }, { 4, 12, 1, 5 },
        { 9, 12, 7, 11 }, { 4, 4, 1, 1 }, { 11, 12, 4, 4 },
    },
    {
        { 0, 24, 4, 4 }, { 27, 24, 5, 4 }, { 0, 36, 4, 5 }, { 27, 36, 5, 5 },
        { 4, 24, 1, 4 }, { 0, 28, 4, 1 }, { 27, 28, 5, 1 }, { 4, 36, 1, 5 },
        { 9, 36, 7, 11 }, { 4, 28, 1, 1 }, { 11, 36, 4, 4 },
    },
    {
        { 0, 48, 4, 4 }, { 27, 48, 5, 4 }, { 0, 60, 4, 5 }, { 27, 60, 5, 5 },
        { 4, 48, 1, 4 }, { 0, 52, 4, 1 }, { 27, 52, 5, 1 }, { 4, 60, 1, 5 },
        { 9, 60, 7, 11 }, { 4, 52, 1, 1 }, { 11, 60, 4, 4 },
    },
    {
        { 0, 72, 4, 4 }, { 27, 72, 5, 4 }, { 0, 84, 4, 5 }, { 27, 84, 5, 5 },
        { 4, 72, 1, 4 }, { 0, 76, 4, 1 }, { 27, 76, 5, 1 }, { 4, 84, 1, 5 },
        { 9, 84, 7, 11 }, { 4, 76, 1, 1 }, { 11, 84, 4, 4 },
    },
    {
        { 0, 96, 4, 4 }, { 11, 96, 5, 4 }, { 0, 107, 4, 5 }, { 11, 107, 5, 5 },
        { 4, 96, 1, 4 }, { 0, 100, 4, 1 }, { 11, 100, 5, 1 }, { 4, 107, 1, 5 },
        { 0, 96, 7, 11 }, { 4, 100, 1, 1 }, { 0, 96, 4, 4 },
    },
    {
        { 16, 96, 4, 4 }, { 27, 96, 5, 4 }, { 16, 107, 4, 5 }, { 27, 107, 5, 5 },
        { 20, 96, 1, 4 }, { 16, 100, 4, 1 }, { 27, 100, 5, 1 }, { 20, 107, 1, 5 },
        { 16, 96, 7, 11 }, { 20, 100, 1, 1 }, { 16, 96, 4, 4 },
    },
    {
        { 32, 96, 4, 4 }, { 43, 96, 5, 4 }, { 32, 107, 4, 5 }, { 43, 107, 5, 5 },
        { 36, 96, 1, 4 }, { 32, 100, 4, 1 }, { 43, 100, 5, 1 }, { 36, 107, 1, 5 },
        { 32, 96, 7, 11 }, { 36, 100, 1, 1 }, { 32, 96, 4, 4 },
    },
    {
        { 46, 96, 4, 4 }, { 57, 96, 5, 4 }, { 46, 107, 4, 5 }, { 57, 107, 5, 5 },
        { 50, 96, 1, 4 }, { 46, 100, 4, 1 }, { 57, 100, 5, 1 }, { 50, 107, 1, 5 },
        { 46, 96, 7, 11 }, { 50, 100, 1, 1 }, { 46, 96, 4, 4 },
    },
};

char dd_wline_gim_name[] = "window01.gim";

#ifdef NON_MATCHING /* alocação de registradores e ordem dos índices do laço (forma igual) */
/* direction of a segment: 1 right, 2 down, 3 left, 4 up */
unsigned int *dd_wline_draw(unsigned int *list, int style, int n, float *pts, int closed)
{
    DdWVert *edges;
    DdWVert *corners;
    DdWVert *ep, *cp;
    int cnt;
    int i, j;
    int dir, nd;
    int ox;
    int next, prev;
    float *a, *b;
    DdWPieceFn fn;
    void *gim;

    dd_wline_style = style;
    edges = dl_alloc_inline(&list, n * 20 * 2);
    corners = edges + n * 2;
    ep = edges;
    cp = corners;
    if (closed) {
        prev = n - 1;
        next = 0;
        cnt = n;
    } else {
        cnt = n - 1;
        prev = 0;
        next = 1;
    }
    b = &pts[next * 3];
    a = &pts[prev * 3];
    if (a[0] < b[0])
        dir = 1;
    else if (a[0] > b[0])
        dir = 3;
    else if (a[1] < b[1])
        dir = 2;
    else
        dir = 4;
    for (i = 0, a = pts; i < cnt; i++, a += 3) {
        j = i + 1;
        if (j >= n)
            j = 0;
        b = &pts[j * 3];
        if (a[0] < b[0])
            nd = 1;
        else if (a[0] > b[0])
            nd = 3;
        else if (a[1] < b[1])
            nd = 2;
        else
            nd = 4;
        fn = 0;
        ox = 0;
        switch (dir) {
        case 1:
            if (nd == 4)
                fn = dd_wline_inner_up_right;
            else if (nd == 2)
                fn = dd_wline_corner_tr;
            break;
        case 2:
            if (nd == 1)
                fn = dd_wline_inner_down_right;
            else if (nd == 3)
                fn = dd_wline_corner_br;
            break;
        case 3:
            if (nd == 4)
                fn = dd_wline_corner_bl;
            else if (nd == 2)
                fn = dd_wline_inner_down_left;
            break;
        case 4:
            if (nd == 1) {
                fn = dd_wline_corner_tl;
            } else if (nd == 3) {
                fn = dd_wline_inner_up_left;
                ox = -4;
            }
            break;
        }
        if (fn != 0)
            cp = fn(cp, a[0], a[1], b[0], b[1]);
        dir = nd;
        fn = 0;
        switch (nd) {
        case 1:
            fn = dd_wline_edge_top;
            break;
        case 2:
            fn = dd_wline_edge_right;
            break;
        case 3:
            fn = dd_wline_edge_bottom;
            break;
        case 4:
            fn = dd_wline_edge_left;
            break;
        }
        if (fn != 0)
            ep = fn(ep, ox + a[0], 0 + a[1], b[0], b[1]);
    }
    gim = ehp_find(syspack_get(), dd_wline_gim_name, 0);
    if (gim != 0)
        list = gim_set_texture(list, gim);
    list = dl_draw_array(list, 6, 0x800102, ((unsigned int)ep - (unsigned int)edges) / 10, 0, edges);
    list = dl_draw_array(list, 6, 0x800102, ((unsigned int)cp - (unsigned int)corners) / 10, 0, corners);
    return list;
}
#endif

unsigned int *dd_wline_sprite(unsigned int *list, int style, float *p0, float *p1)
{
    DdWPiece *p = &dd_wline_pieces[style][9];
    DdWVert *v;
    void *gim;

    gim = ehp_find(syspack_get(), dd_wline_gim_name, 0);
    if (gim != 0)
        list = gim_set_texture(list, gim);
    v = dl_alloc_inline(&list, 20);
    v[0].x = p0[0];
    v[0].y = p0[1];
    v[0].z = p0[2];
    v[1].x = p1[0];
    v[1].y = p1[1];
    v[1].z = p1[2];
    v[0].u = p->u;
    v[0].v = p->v;
    v[1].u = p->u + p->w;
    v[1].v = p->v + p->h;
    list = dl_draw_array(list, 6, 0x800102, 2, 0, v);
    return list;
}

#define PIECE(k) (&dd_wline_pieces[dd_wline_style][k])

DdWVert *dd_wline_corner_tl(DdWVert *v, int x0, int y0, int x1, int y1)
{
    DdWPiece *p = PIECE(0);

    v[0].u = p->u;
    v[0].v = p->v;
    v[0].x = x0 - p->w;
    v[0].y = y0 - p->h;
    v[0].z = 0;
    v[1].u = p->u + p->w;
    v[1].v = p->v + p->h;
    v[1].x = x0;
    v[1].y = y0;
    v[1].z = 0;
    return v + 2;
}

DdWVert *dd_wline_edge_top(DdWVert *v, int x0, int y0, int x1, int y1)
{
    DdWPiece *p = PIECE(4);

    v[0].u = p->u;
    v[0].v = p->v;
    v[0].x = x0;
    v[0].y = y0 - p->h;
    v[0].z = 0;
    v[1].u = p->u + p->w;
    v[1].v = p->v + p->h;
    v[1].x = x1;
    v[1].y = y0;
    v[1].z = 0;
    return v + 2;
}

DdWVert *dd_wline_corner_tr(DdWVert *v, int x0, int y0, int x1, int y1)
{
    DdWPiece *p = PIECE(1);

    v[0].u = p->u;
    v[0].v = p->v;
    v[0].x = x0;
    v[0].y = y0 - p->h;
    v[0].z = 0;
    v[1].u = p->u + p->w;
    v[1].v = p->v + p->h;
    v[1].x = x0 + p->w;
    v[1].y = y0;
    v[1].z = 0;
    return v + 2;
}

DdWVert *dd_wline_edge_right(DdWVert *v, int x0, int y0, int x1, int y1)
{
    DdWPiece *p = PIECE(6);

    v[0].u = p->u;
    v[0].v = p->v;
    v[0].x = x0;
    v[0].y = y0;
    v[0].z = 0;
    v[1].u = p->u + p->w;
    v[1].v = p->v + p->h;
    v[1].x = x0 + p->w;
    v[1].y = y1;
    v[1].z = 0;
    return v + 2;
}

DdWVert *dd_wline_corner_br(DdWVert *v, int x0, int y0, int x1, int y1)
{
    DdWPiece *p = PIECE(3);

    v[0].u = p->u;
    v[0].v = p->v;
    v[0].x = x0;
    v[0].y = y0;
    v[0].z = 0;
    v[1].u = p->u + p->w;
    v[1].v = p->v + p->h;
    v[1].x = x0 + p->w;
    v[1].y = y0 + p->h;
    v[1].z = 0;
    return v + 2;
}

DdWVert *dd_wline_edge_bottom(DdWVert *v, int x0, int y0, int x1, int y1)
{
    DdWPiece *p = PIECE(7);

    v[0].u = p->u;
    v[0].v = p->v;
    v[0].x = x1;
    v[0].y = y0;
    v[0].z = 0;
    v[1].u = p->u + p->w;
    v[1].v = p->v + p->h;
    v[1].x = x0;
    v[1].y = y0 + p->h;
    v[1].z = 0;
    return v + 2;
}

DdWVert *dd_wline_corner_bl(DdWVert *v, int x0, int y0, int x1, int y1)
{
    DdWPiece *p = PIECE(2);

    v[0].u = p->u;
    v[0].v = p->v;
    v[0].x = x0 - p->w;
    v[0].y = y0;
    v[0].z = 0;
    v[1].u = p->u + p->w;
    v[1].v = p->v + p->h;
    v[1].x = x0;
    v[1].y = y0 + p->h;
    v[1].z = 0;
    return v + 2;
}

DdWVert *dd_wline_edge_left(DdWVert *v, int x0, int y0, int x1, int y1)
{
    DdWPiece *p = PIECE(5);

    v[0].u = p->u;
    v[0].v = p->v;
    v[0].x = x0 - p->w;
    v[0].y = y1;
    v[0].z = 0;
    v[1].u = p->u + p->w;
    v[1].v = p->v + p->h;
    v[1].x = x0;
    v[1].y = y0;
    v[1].z = 0;
    return v + 2;
}

DdWVert *dd_wline_inner_up_right(DdWVert *v, int x0, int y0, int x1, int y1)
{
    DdWPiece *p = PIECE(10);

    v[0].u = p->u + p->w;
    v[0].v = p->v + p->h;
    v[0].x = x0 - p->w;
    v[0].y = y0 - p->h;
    v[0].z = 0;
    v[1].u = p->u;
    v[1].v = p->v;
    v[1].x = x0;
    v[1].y = y0;
    v[1].z = 0;
    return v + 2;
}

DdWVert *dd_wline_inner_down_right(DdWVert *v, int x0, int y0, int x1, int y1)
{
    DdWPiece *p = PIECE(10);

    v[0].u = p->u;
    v[0].v = p->v + p->h;
    v[0].x = x0;
    v[0].y = y0 - p->h;
    v[0].z = 0;
    v[1].u = p->u + p->w;
    v[1].v = p->v;
    v[1].x = x0 + p->w;
    v[1].y = y0;
    v[1].z = 0;
    return v + 2;
}

DdWVert *dd_wline_inner_up_left(DdWVert *v, int x0, int y0, int x1, int y1)
{
    DdWPiece *p = PIECE(10);

    v[0].u = p->u + p->w;
    v[0].v = p->v;
    v[0].x = x0 - p->w;
    v[0].y = y0;
    v[0].z = 0;
    v[1].u = p->u;
    v[1].v = p->v + p->h;
    v[1].x = x0;
    v[1].y = y0 + p->h;
    v[1].z = 0;
    return v + 2;
}

DdWVert *dd_wline_inner_down_left(DdWVert *v, int x0, int y0, int x1, int y1)
{
    DdWPiece *p = PIECE(10);

    v[0].u = p->u;
    v[0].v = p->v;
    v[0].x = x0;
    v[0].y = y0;
    v[0].z = 0;
    v[1].u = p->u + p->w;
    v[1].v = p->v + p->h;
    v[1].x = x0 + p->w;
    v[1].y = y0 + p->h;
    v[1].z = 0;
    return v + 2;
}
