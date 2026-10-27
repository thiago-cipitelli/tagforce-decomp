// @range 0x08818FFC-0x0881BA90
// @asm 0x08819424-0x08819634
// @asm 0x08819644-0x08819740
// @asm 0x088197B0-0x0881A078
// @asm 0x0881B268-0x0881B608
// @asm 0x0881B7B8-0x0881B8C0
// @rodata 0x08838B90-0x0883904C
// @data 0x0883C8B0-0x0883C9C8
/* The campus map (field_bm, bm_*.ehp): the island with its 14 places, the cursor moving between them
   with the d-pad or the stick, the marks of the people to meet, the list of the academy rooms, the
   save on the map and the fades of the screen. */
#include "libfield.h"

typedef struct FieldBmLink {           /* where a direction leads from a place, 8 */
    unsigned int valid : 1;
    unsigned int to : 8;               /* 0xFF: this entry, else another entry of the place */
    unsigned int right : 6;            /* 0x3F none */
    unsigned int up : 6;
    unsigned int down : 6;
    unsigned int pad : 5;
    unsigned int left : 6;
    unsigned int pad2 : 26;
} FieldBmLink;

typedef union FieldBmLinkDef {
    unsigned int w[2];
    FieldBmLink l;
} FieldBmLinkDef;

typedef struct FieldBmPt {
    int x, y;
} FieldBmPt;

int msd_open(int mode, void *heap);
int msd_update(void);
void msd_update_dialog(void);
int cfg_get_language(void);
void *fld_gim_get(int i);
FieldStory *fld_story_get(void);
int fld_time_index(int t);
float fld_stick_axis(int v);
void *ehf_tex_or_flags(void *ehf, void *tmf);
void *gim_set_texture(void *list, void *gim);
void *dl_ambient_color(void *list, unsigned int color);
void *dl_blend_func(void *list, int op, int src, int dst, unsigned int fixa, unsigned int fixb);
void *fld_draw_cell(void *list, int x, int y, int u, int v, int tw, int th, int w, int h);
void *fld_draw_cell1(void *list, int x, int y, int u, int v, int w, int h);
Vec2 *vec2_sub(Vec2 *out, Vec2 *a, Vec2 *b);
Vec2 *vec2_normalize(Vec2 *out, Vec2 *a);
float vec2_dot(Vec2 *a, Vec2 *b);
float cosf_vfpu(float x);
float sqrtf_vfpu(float x);
int menu_update(MenuWidget *w);
void fld_dlg_open(unsigned short *text, int a, int b);
int fld_dlg_update(void);
void *fld_dlg_draw(void *list);
void *fld_dl_clear(void *list, short z, unsigned int color);

void fbm_nop(void);
void fbm_input(void);
void fbm_move_dir(int dir, int i);
void fbm_move(void);
void fbm_sub_update(void);
void fbm_slide(void);
static int fbm_person_slot(int kind);
void fbm_mark_people(FieldList *l);
int fbm_place_id(int place, int sub);
static void fbm_place_of(int id, int *place, int *sub);
static int fbm_name_id(int place, int sub);
void fbm_loaded(int *file);
void fbm_sub_open(void);
static int fbm_sub_width(void);
void *fbm_sub_item(void *list, unsigned short i, int x, int y);
void *fbm_draw_map(void *list);
void *fbm_draw_pic(void *list);
void *fbm_draw_marks(void *list);
void *fbm_icon(void *list, int i, int x, int y, int w, int h);
void *fbm_draw_arrows(void *list);
void *fbm_draw_cursor(void *list);
void fbm_fade(int out, unsigned int color, int len, int layer);
void fbm_fade_tick(void);

const signed char fbm_lang_dx[8] = { 4, 2, 4, 4, 3, 0, 0, 0 };
const FieldBmPt fbm_pos[14] = { { 122, 91 }, { 127, 148 }, { 206, 166 }, { 275, 177 }, { 211, 87 },
                                { 90, 130 }, { 32, 155 }, { 400, 141 }, { 324, 137 }, { 320, 50 },
                                { 349, 188 }, { 306, 204 }, { 82, 83 }, { 17, 225 } };
const FieldBmLinkDef fbm_links[14][4] = {
    { { 0x00A0FFFF, 0x4 }, { 0x07FFFE01, 0x3F }, { 0x0180FFFF, 0x4 }, { 0x07FFFE05, 0x3F } },
    { { 0x00A681FF, 0x2 }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F } },
    { { 0x003F89FF, 0x3 }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F } },
    { { 0x005F91FF, 0xA }, { 0x005F91FF, 0xB }, { 0x07FFFE01, 0x3F }, { 0x07FFFE03, 0x3F } },
    { { 0x000113FF, 0x8 }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F } },
    { { 0x00C681FF, 0x1 }, { 0x07FFFE01, 0x3F }, { 0x00C699FF, 0x1 }, { 0x07FFFE05, 0x3F } },
    { { 0x07E6FFFF, 0x5 }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F } },
    { { 0x010513FF, 0x3F }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F } },
    { { 0x008513FF, 0x7 }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F } },
    { { 0x00847FFF, 0x7 }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F } },
    { { 0x007F91FF, 0x7 }, { 0x017F91FF, 0x7 }, { 0x07FFFE01, 0x3F }, { 0x07FFFE03, 0x3F } },
    { { 0x07FFFFFE, 0x3F }, { 0x007F91FF, 0xA }, { 0x07FFFFFE, 0x3F }, { 0x07FFFE03, 0x3F } },
    { { 0x07FFFFFE, 0x3F }, { 0x07FFFFFE, 0x3F }, { 0x07E2FFFF, 0x0 }, { 0x07FFFE05, 0x3F } },
    { { 0x07FF8DFF, 0x1 }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F }, { 0x07FFFE01, 0x3F } },
};
const FieldBmPt fbm_pics[20] = { { 0, 0 }, { 204, 0 }, { 306, 0 }, { 408, 0 }, { 0, 83 }, { 0, 166 }, { 102, 166 },
                                 { 204, 166 }, { 306, 166 }, { 408, 166 }, { 0, 249 }, { 102, 249 }, { 204, 249 },
                                 { 0, 0 }, { 0, 83 }, { 102, 83 }, { 204, 83 }, { 306, 83 }, { 408, 83 },
                                 { 102, 0 } };
const FieldBmPt fbm_icons[8] = { { 0, 332 }, { 23, 332 }, { 46, 332 }, { 69, 332 },
                                 { 115, 332 }, { 161, 332 }, { 138, 332 }, { 92, 332 } };

/* d-pad directions of fbm_input (kept in asm): right+down, right+up, left+down, left+up, up, down,
   left, right */
Vec2 fbm_dirs[8] = { { 1.0f, 1.0f },  { 1.0f, -1.0f }, { -1.0f, 1.0f }, { -1.0f, -1.0f },
                     { 0.0f, -1.0f }, { 0.0f, 1.0f },  { -1.0f, 0.0f }, { 1.0f, 0.0f } };

void fbm_clear(void)
{
    memset(&field_bm, 0, sizeof(field_bm));
}

void fbm_free(void)
{
    fbm_nop();
    fbm_clear();
}

void fbm_nop(void)
{
}

void fbm_update(void)
{
    FieldBm *m = &field_bm;
    int r;
    int done;
    int ok;
    int *p;

    fld_ctx_get();
    if (field_bm.loaded == 0)
        return;
    switch (m->state) {
    case 1:
        fbm_input();
        break;
    case 2:
        fbm_move();
        break;
    case 3:
        fbm_sub_update();
        break;
    case 4:
        p = m->res;
        p[0] = 0;
        p[1] = fbm_place_id(m->cur, m->sub);
        m->active = 0;
        break;
    case 5:
        if (m->cmd.on_save != 0)
            m->cmd.on_save();
        msd_open(5, fld_heap_get());
        m->state = 6;
        fbm_fade(0, 0xA0000000, 10, 4);
        break;
    case 6:
        switch (msd_update()) {
        case 1:
            break;
        case 2:
        case 3:
            m->state = 7;
            break;
        }
        break;
    case 7:
        fld_dlg_open(table_get_entry(fld_gim_get(5), 0x65), 3, 3);
        m->state = 8;
        break;
    case 8:
        done = 0;
        ok = 0;
        switch (fld_dlg_update()) {
        case -2:
        case 3:
            done = 1;
            m->state = 1;
            ok = 0;
            fbm_fade(1, 0xA0000000, 10, 4);
            break;
        case 2:
            m->state = 9;
            done = 1;
            ok = 1;
            break;
        }
        if (done && m->cmd.on_saved != 0)
            m->cmd.on_saved(ok);
        break;
    case 9:
        p = m->res;
        p[0] = 1;
        p[1] = fbm_place_id(m->home, m->sub);
        m->active = 0;
        break;
    }
    fbm_fade_tick();
    fbm_slide();
}

void fbm_draw(void)
{
    FieldBm *m = &field_bm;
    void *list;

    if (field_bm.loaded == 0)
        return;
    list = dl_select_layer(3, 0);
    list = fbm_draw_map(list);
    list = fbm_draw_marks(list);
    list = fbm_draw_arrows(list);
    list = fbm_draw_cursor(list);
    if (m->fade.layer == 3) {
        switch (m->state) {
        case 1:
        case 2:
        case 3:
        case 4:
            list = fld_dl_clear(list, 0, m->fade.cur);
            break;
        }
    }
    list = fbm_draw_pic(list);
    menu_layout(&list, &m->menu);
    if (m->state == 3)
        menu_draw(&list, &m->w);
    dl_set_list(list);
    list = dl_select_layer(4, 0);
    if (m->fade.layer == 4) {
        switch (m->state) {
        case 0:
        case 3:
        case 4:
            break;
        case 1:
        case 2:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            list = fld_dl_clear(list, 0, m->fade.cur);
            break;
        }
    }
    if (m->state == 8)
        list = fld_dlg_draw(list);
    dl_set_list(list);
    if (m->state == 6)
        msd_update_dialog();
}

#ifdef NON_MATCHING /* mwcc stores the first two counters absolutely, the original through the base register */
void fbm_setup(FieldBmCmd *c)
{
    FieldBm *m = &field_bm;
    FieldStory *st = fld_story_get();
    int place;
    int sub;
    int any[5];
    int i;
    int k;
    int all;

    fbm_place_of(c->home, &place, &sub);
    memcpy(&m->cmd, c, sizeof(*c));
    m->prev = place;
    m->home = place;
    m->cur = place;
    m->state = 0;
    m->alt = c->alt;
    fbm_mark_people((FieldList *)c);
    m->n_people = 0;
    any[1] = 0;
    m->blocked = 0;
    any[2] = 0;
    any[4] = 0;
    any[3] = 0;
    any[0] = 0;
    for (i = 0; i < c->n_people; i++) {
        int pl;
        int sb;
        int id = c->people[i];

        m->people[i] = id;
        m->n_people++;
        fbm_place_of(id, &pl, &sb);
        if (pl != 4)
            m->blocked |= 1 << pl;
        else
            any[sb] = 1;
    }
    all = 1;
    for (k = 0; k < 5; k++)
        all &= any[k];
    if (all)
        m->blocked |= 0x10;
    switch (fld_time_index(*(int *)st->w[3])) {
    case 0:
        fld_load_gim(6, (void (*)(int *, void *))fbm_loaded, 0);
        break;
    case 1:
        fld_load_gim(7, (void (*)(int *, void *))fbm_loaded, 0);
        break;
    case 2:
        fld_load_gim(8, (void (*)(int *, void *))fbm_loaded, 0);
        break;
    }
}
#endif

int fbm_ready(void)
{
    return field_bm.pack != 0;
}

#ifdef NON_MATCHING /* the base of field_bm is kept in $s0 from the start though only the last call uses it */
void fbm_cmd_open(void)
{
    FieldBm *m = &field_bm;
    MenuItemDef *d;

    field_bm.active = 1;
    memset(field_bm.res, 0, 8);
    field_bm.state = 1;
    menu_init(&field_bm.menu, fld_heap_get());
    d = &field_bm.item;
    memset(d, 0, sizeof(*d));
    d->visible = 1;
    d->auto_size = 1;
    d->font_flag4 = 1;
    d->style = 2;
    d->x = 4;
    d->y = 4;
    d->color = 0xFFFFFFFF;
    d->text = 0;
    d->font_flag8000 = cfg_get_language() == 0;
    d->font_size = 15;
    d->text_color = 0xFFFFFFFF;
    d->pad_b = -1;
    d->pad_t = 1;
    d->pad_l = 3;
    d->pad_r = 2;
    menu_add_item(&m->menu, d);
}
#endif

int fbm_cmd_done(int *out)
{
    FieldBm *m = &field_bm;
    int *r = field_bm.res;

    if (m->active == 0) {
        menu_free_items(&m->menu);
        out[0] = r[0];
        out[1] = r[1];
    }
    return (m->active != 0) ^ 1;
}

#ifdef NON_MATCHING /* the insertion into the three nearest places is unrolled by 8 in the original */
void fbm_input(void)
{
    FieldBm *m = &field_bm;
    PadState *pad = pad_get();
    unsigned int down = pad->b.down;
    unsigned int dir = pad->b.dir;
    Vec2 v = { 0 };
    int d;

    if (down & region_pick_value(0)) {
        fld_story_flag(0x6213, 0);
        if (m->cur == 4) {
            fbm_sub_open();
            m->state = 3;
            fbm_fade(0, 0xA0000000, 10, 3);
        } else if (m->cur == 0xD) {
            m->state = 5;
        } else {
            m->state = 4;
            m->dest = m->cur;
        }
        return;
    }
    if (down & region_pick_value(1)) {
        fld_story_flag(0x6212, 0);
        m->prev = m->cur;
        m->cur = m->home;
        m->state = 2;
        ehf_set_frame(m->arrow[3], 0);
        return;
    }
    if (down & 0x1000) {
        fld_story_flag(0x6212, 0);
        m->prev = m->cur;
        m->cur = 0xD;
        m->state = 2;
        return;
    }
    d = -1;
    if (down & 0x10)
        d = 4;
    else if (down & 0x40)
        d = 0;
    else if (down & 0x80)
        d = 2;
    else if (down & 0x20)
        d = 6;
    else if (dir & 0x10)
        d = 4;
    else if (dir & 0x40)
        d = 0;
    else if (dir & 0x80)
        d = 2;
    else if (dir & 0x20)
        d = 6;
    if (m->blocked == 0) {
        fbm_move_dir(d, 3);
    } else if (m->blocked == 0x1000) {
        fbm_move_dir(d, 1);
    } else if (m->blocked == 0x800) {
        fbm_move_dir(d, 2);
    } else if (m->blocked == 0x1800) {
        fbm_move_dir(d, 0);
    } else {
        /* other maps: the place nearest to the cursor inside a cone of 45, 60 then 75 degrees around the
           d-pad (diagonals included) or the stick direction */
        float cone[3];
        Vec2 here;
        struct {
            int id;
            float dist;
        } best[3];
        int a, j, q;
        unsigned int k;

        if ((down & 0x20) && (down & 0x40))
            v = fbm_dirs[0];
        else if ((down & 0x20) && (down & 0x10))
            v = fbm_dirs[1];
        else if ((down & 0x80) && (down & 0x40))
            v = fbm_dirs[2];
        else if ((down & 0x80) && (down & 0x10))
            v = fbm_dirs[3];
        else if (down & 0x10)
            v = fbm_dirs[4];
        else if (down & 0x40)
            v = fbm_dirs[5];
        else if (down & 0x80)
            v = fbm_dirs[6];
        else if (down & 0x20)
            v = fbm_dirs[7];
        else {
            /* the original measures a Vec2 with fld_vec3_len: z is the following v.x, still 0 here */
            Vec3 st;

            st.x = fld_stick_axis(pad_get()->b.analog_x);
            st.y = fld_stick_axis(pad_get()->b.analog_y);
            st.z = 0.0f;
            if (fld_vec3_len(&st) >= 0.5f) {
                v.x = st.x;
                v.y = st.y;
            }
        }
        if (v.x == 0.0f && v.y == 0.0f)
            return;
        memset(cone, 0, sizeof(cone));
        cone[0] = cosf_vfpu(0.7853982f);
        cone[1] = cosf_vfpu(1.0471976f);
        cone[2] = cosf_vfpu(1.3089969f);
        here.x = fbm_pos[m->cur].x;
        here.y = fbm_pos[m->cur].y;
        for (j = 0; j < 3; j++) {
            best[j].id = -1;
            best[j].dist = 99999.0f;
        }
        vec2_normalize(&v, &v);
        for (a = 0; a < 3; a++) {
            for (k = 0; (int)k < 14; k++) {
                Vec2 p;
                Vec2 dv;
                Vec2 n;
                float dist;

                if (k == m->cur || (m->blocked & (1 << k)))
                    continue;
                p.x = fbm_pos[k].x;
                p.y = fbm_pos[k].y;
                vec2_sub(&dv, &p, &here);
                vec2_normalize(&n, &dv);
                if (!(vec2_dot(&v, &n) >= cone[a]))
                    continue;
                dist = fabsf_vfpu(sqrtf_vfpu(dv.x * dv.x + dv.y * dv.y));
                for (j = 0; j < 3; j++) {
                    if (dist < best[j].dist) {
                        for (q = 2; q > j; q--)
                            best[q] = best[q - 1];
                        best[j].id = k;
                        best[j].dist = dist;
                        break;
                    }
                }
            }
            if (best[0].id != -1)
                break;
        }
        if (best[0].id != -1) {
            fld_story_flag(0x6212, 0);
            m->state = 2;
            m->prev = m->cur;
            m->cur = best[0].id;
            if (best[0].id != 0xD)
                ehf_set_frame(m->arrow[3], 0);
        }
    }
}
#endif

void fbm_move_dir(int dir, int i)
{
    FieldBm *m = &field_bm;
    const FieldBmLinkDef *row;
    const FieldBmLinkDef *e;
    unsigned int to;

    if (dir == -1)
        return;
    row = fbm_links[m->cur];
    e = &row[i];
    if (e->l.to != 0xFF)
        e = &row[e->l.to];
    if (e->l.valid == 1) {
        switch (dir) {
        case 4:
        default:
            to = e->l.right;
            break;
        case 0:
            to = e->l.up;
            break;
        case 2:
            to = e->l.down;
            break;
        case 6:
            to = e->l.left;
            break;
        }
        if (to != 0x3F) {
            fld_story_flag(0x6212, 0);
            m->state = 2;
            m->prev = m->cur;
            m->cur = to;
            if (to != 0xD)
                ehf_set_frame(m->arrow[3], 0);
        }
    }
}

void fbm_move(void)
{
    FieldBm *m = &field_bm;

    if (field_bm.move_t >= 1.0f) {
        m->prev = m->cur;
        m->move_t = 0.0f;
        m->state = 1;
    } else {
        m->move_t = m->move_t + (0.025f + 0.5f * (1.0f - m->move_t));
        if (m->move_t > 1.0f)
            m->move_t = 1.0f;
    }
}

void fbm_sub_update(void)
{
    MenuWidget *w = &field_bm.w;
    FieldBm *m = &field_bm;
    int r = menu_update(w);

    field_bm.sub = w->scroll + w->cursor;
    if (r != -1) {
        menu_reset(w);
        switch (r) {
        case -2:
            m->state = 1;
            fbm_fade(1, 0xA0000000, 10, 3);
            break;
        default:
            m->state = 4;
            m->dest = m->sub + 0xE;
            break;
        }
    }
}

void fbm_slide(void)
{
    FieldBm *m = &field_bm;
    MenuItemDef *d = &field_bm.item;
    Vec2 from = { -100.0f, 4.0f };
    Vec2 to = { 4.0f, 4.0f };
    Vec2 p;
    int id;

    switch (field_bm.slide) {
    case 0:
        vec2_lerp(&p, &from, &to, m->slide_t);
        d->x = p.x;
        d->y = p.y;
        if (m->slide_t >= 1.0f) {
            m->slide = 1;
        } else {
            m->slide_t = m->slide_t + (0.1f + 0.1f * m->slide_t);
            if (m->slide_t > 1.0f)
                m->slide_t = 1.0f;
        }
        break;
    case 1:
        break;
    case 2:
        vec2_lerp(&p, &from, &to, m->slide_t);
        d->x = p.x;
        d->y = p.y;
        if (m->slide_t <= 0.0f) {
            m->slide = 3;
        } else {
            m->slide_t = m->slide_t - (0.1f + 0.1f * m->slide_t);
            if (m->slide_t < 0.0f)
                m->slide_t = 0.0f;
        }
        break;
    }
    id = fbm_name_id(m->cur, -1);
    d->text = table_get_entry(fld_gim_get(8), id);
}

static int fbm_person_slot(int kind)
{
    int r;

    switch (kind) {
    case 0:
    case 1:
    case 9:
    case 10:
    default:
        r = -1;
        break;
    case 2:
        r = 0;
        break;
    case 3:
        r = 1;
        break;
    case 4:
        r = 2;
        break;
    case 5:
        r = 3;
        break;
    case 6:
        r = 4;
        break;
    case 7:
        r = 5;
        break;
    case 8:
        r = 6;
        break;
    }
    return r;
}

void fbm_mark_people(FieldList *l)
{
    FieldBm *m = &field_bm;
    int i;
    int *e;
    int slot;
    int place;
    int sub;

    for (i = 0; i < l->n; i++) {
        e = (int *)l->items + i * 2;
        slot = fbm_person_slot(e[0]);
        if (e[1] == -1)
            place = -1;
        else
            fbm_place_of(e[1], &place, &sub);
        if (place != -1 && slot != -1) {
            m->marks[place] |= 1 << slot;
            if (place == 4)
                m->sub_marks[sub] |= 1 << slot;
        }
    }
}

int fbm_place_id(int place, int sub)
{
    int id = -1;

    switch (place) {
    case 0:
        id = 0xC;
        break;
    case 1:
        id = 0x16;
        break;
    case 2:
        id = 0x17;
        break;
    case 3:
        id = 0x18;
        break;
    case 5:
        id = 0x1A;
        break;
    case 6:
        id = 0x19;
        break;
    case 7:
        id = 0x1B;
        break;
    case 8:
        id = 0x1D;
        break;
    case 9:
        id = 0x1E;
        break;
    case 10:
        id = 0x1F;
        break;
    case 11:
        id = 0x20;
        break;
    case 12:
        id = 0x24;
        break;
    case 13:
        id = -1;
        break;
    case 4:
        switch (sub) {
        case 0:
            id = 1;
            break;
        case 1:
            id = 2;
            break;
        case 2:
            id = 3;
            break;
        case 3:
            id = 4;
            break;
        case 4:
            id = 5;
            break;
        }
        break;
    }
    return id;
}

static void fbm_place_of(int id, int *place, int *sub)
{
    switch (id) {
    case 0:
    case 1:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 0x1C:
    case 0x21:
    case 0x22:
    case 0x23:
    default:
        *place = 4;
        *sub = 0;
        break;
    case 2:
        *place = 4;
        *sub = 1;
        break;
    case 3:
        *place = 4;
        *sub = 2;
        break;
    case 4:
        *place = 4;
        *sub = 3;
        break;
    case 5:
        *place = 4;
        *sub = 4;
        break;
    case 0xC:
        *place = 0;
        break;
    case 0xD:
        *place = 0;
        break;
    case 0xE:
        *place = 0;
        break;
    case 0xF:
        *place = 0;
        break;
    case 0x10:
        *place = 0;
        break;
    case 0x11:
        *place = 0;
        break;
    case 0x12:
        *place = 0;
        break;
    case 0x13:
        *place = 0;
        break;
    case 0x14:
        *place = 0;
        break;
    case 0x15:
        *place = 0;
        break;
    case 0x16:
        *place = 1;
        break;
    case 0x17:
        *place = 2;
        break;
    case 0x18:
        *place = 3;
        break;
    case 0x19:
        *place = 6;
        break;
    case 0x1A:
        *place = 5;
        break;
    case 0x1B:
        *place = 7;
        break;
    case 0x1D:
        *place = 8;
        break;
    case 0x1E:
        *place = 9;
        break;
    case 0x1F:
        *place = 10;
        break;
    case 0x20:
        *place = 11;
        break;
    case 0x24:
        *place = 12;
        break;
    case 0x25:
        *place = 0;
        break;
    case 0x26:
        *place = 0;
        break;
    case 0x27:
        *place = 12;
        break;
    }
}

static int fbm_name_id(int place, int sub)
{
    int id = -1;

    switch (place) {
    case 0:
        id = 1;
        break;
    case 1:
        id = 0xB;
        break;
    case 2:
        id = 0xC;
        break;
    case 3:
        id = 0xD;
        break;
    case 5:
        id = 0x16;
        break;
    case 6:
        id = 0x17;
        break;
    case 7:
        id = 0x18;
        break;
    case 8:
        id = 0x1A;
        break;
    case 9:
        id = 0x1B;
        break;
    case 10:
        id = 0x1C;
        break;
    case 11:
        id = 0x1D;
        break;
    case 12:
        id = 0x1E;
        break;
    case 13:
        id = 0x1F;
        break;
    case 4:
        switch (sub) {
        case -1:
            id = 0xE;
            break;
        case 0:
            id = 0xF;
            break;
        case 1:
            id = 0x10;
            break;
        case 2:
            id = 0x11;
            break;
        case 3:
            id = 0x12;
            break;
        case 4:
            id = 0x13;
            break;
        }
        break;
    }
    return id;
}

void fbm_loaded(int *file)
{
    FieldBm *m = &field_bm;
    void *t;

    if (file == 0)
        return;
    m->pack = (int *)*file;
    m->island = ehp_find(m->pack, "bm_island.gim", 0);
    m->parts = ehp_find(m->pack, "bm_parts.gim", 0);
    m->whole = ehp_find(m->pack, "bm_efct_whole.ehf", 0);
    m->efct_m = ehp_find(m->pack, "bm_efct_m.ehf", 0);
    m->arrow[0] = ehp_find(m->pack, "bm_arrow11.ehf", 0);
    m->arrow[1] = ehp_find(m->pack, "bm_arrow12.ehf", 0);
    m->arrow[2] = ehp_find(m->pack, "bm_arrow13.ehf", 0);
    m->arrow[3] = ehp_find(m->pack, "bm_arrow14.ehf", 0);
    t = ehp_find(m->pack, "bm_efct_whole.tmf", 0);
    if (t != 0)
        ehf_tex_or_flags(m->whole, t);
    t = ehp_find(m->pack, "bm_efct_m.tmf", 0);
    if (t != 0)
        ehf_tex_or_flags(m->efct_m, t);
    t = ehp_find(m->pack, "bm_arrow11.tmf", 0);
    if (t != 0)
        ehf_tex_or_flags(m->arrow[0], t);
    t = ehp_find(m->pack, "bm_arrow13.tmf", 0);
    if (t != 0)
        ehf_tex_or_flags(m->arrow[2], t);
    ehf_set_anim(m->arrow[0], 0, 0x36);
    ehf_set_anim(m->arrow[1], 0, 0x31);
    ehf_set_anim(m->arrow[2], 0, 0x10);
    ehf_set_anim(m->arrow[3], 0, 0x36);
    m->loaded = 1;
}

void fbm_sub_open(void)
{
    FieldBm *m = &field_bm;
    MenuWidget *const w = &field_bm.w;
    MenuItemDef *const d = &field_bm.w.def;
    int i;
    int place;
    int sub;
    int dx;

    memset(w, 0, sizeof(*w));
    d->visible = 1;
    d->auto_size = 1;
    d->pad_t = 1;
    d->pad_b = 1;
    d->pad_l = 1;
    d->pad_r = 1;
    d->x = 4;
    d->y = 0x26;
    d->color = 0xFFFFFFFF;
    d->open_anim = 2;
    d->close_anim = 2;
    w->no_cancel = 1;
    w->style = 2;
    w->buttons = 0;
    w->count = 5;
    w->cursor = 0;
    d->style = 0;
    w->scroll = 0;
    w->button = 0;
    w->selected = 0;
    if (cfg_get_language() == 0)
        dx = 5;
    else
        dx = 0;
    i = fbm_sub_width() + 8;
    w->item_w = i + dx;
    w->item_h = 0x19;
    w->cursor_h = 0x17;
    w->draw_item = (MenuDrawItemFn)fbm_sub_item;
    w->heap = fld_heap_get();
    menu_open(w);
    m->sub = w->scroll + w->cursor;
    for (i = 0; i < m->n_people; i++) {
        fbm_place_of(m->people[i], &place, &sub);
        if (place == 4)
            w->disabled |= 1 << sub;
    }
}

static int fbm_sub_width(void)
{
    int i;
    int max = 0;
    int w;

    font_set_size(15, 15);
    font_set_flag4(1);
    for (i = 0; i < 5; i++) {
        w = fbm_name_id(4, i);
        w = font_measure_text(table_get_entry(fld_gim_get(8), w)) >> 6;
        if (max < w)
            max = w;
    }
    return max + 1;
}

void *fbm_sub_item(void *list, unsigned short i, int x, int y)
{
    FieldBm *m = &field_bm;
    int dx;
    int dy;
    int id;
    unsigned int color;
    unsigned int k;
    int ix;
    unsigned short *t;

    switch (cfg_get_language()) {
    case 0:
        dx = fbm_lang_dx[i];
        break;
    default:
        dx = 4;
        break;
    }
    id = fbm_name_id(4, i);
    if ((m->w.disabled >> i) & 1)
        color = 0xFF808080;
    else
        color = 0xFF000000;
    font_set_flag8000(cfg_get_language() == 0);
    font_text_set(list);
    font_set_size(15, 15);
    font_set_flag4(1);
    font_set_color(color);
    t = table_get_entry(fld_gim_get(8), id);
    if (cfg_get_language() == 0)
        dy = 1;
    else
        dy = 5;
    font_draw_text((x + dx) * 64, (y + dy) * 64, (480 - (x + dx)) * 64, t);
    list = font_text_clear();
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, m->parts);
    for (k = 0, ix = 0; (int)k < 7; k++) {
        if (m->sub_marks[i] & (1 << k)) {
            int bx = m->w.def.x + m->w.def.w;

            list = fbm_icon(list, k, bx + ix, y, 0x17, 0x17);
            ix += 0x17;
        }
    }
    return list;
}

void *fbm_draw_map(void *list)
{
    FieldBm *m = &field_bm;
    EhfOpts o;
    int i;
    int x;

    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, m->island);
    list = draw_set_offsets(list, 0x40, 0x100, 0x40, 0x100);
    for (i = 0, x = 0; i < 7; i++, x += 0x40)
        list = draw_sprite(list, x, 0, 0, x, 0, 0, 0);
    list = draw_set_offsets(list, 0x20, 0x100, 0x20, 0x100);
    list = draw_sprite(list, 0x1C0, 0, 0, 0x1C0, 0, 0, 0);
    list = draw_set_offsets(list, 0x10, 0x100, 0x100, 0x10);
    list = draw_sprite(list, 0, 0x100, 0, 0x1E0, 0, 0, 3);
    list = draw_set_offsets(list, 0x10, 0x100, 0x100, 0x10);
    list = draw_sprite(list, 0x100, 0x100, 0, 0x1F0, 0, 0, 3);
    if ((m->blocked & 0x1000) == 0) {
        list = gim_set_texture(list, m->parts);
        list = fld_draw_cell1(list, 0x2C, 0x24, 0x132, 0xF9, 0x50, 0x50);
    }
    memset(&o, 0, sizeof(o));
    o.time = *gfx_get_state();
    list = gfx_begin_2d(list);
    list = ehf_draw(list, m->whole, &o);
    list = gfx_end_2d(list);
    if ((m->blocked & 0x1000) == 0) {
        list = gfx_begin_2d(list);
        list = ehf_draw(list, m->efct_m, &o);
        list = gfx_end_2d(list);
    }
    return list;
}

void *fbm_draw_pic(void *list)
{
    FieldBm *m = &field_bm;
    const FieldBmPt *p;

    switch (field_bm.state) {
    case 3:
        p = &fbm_pics[m->sub + 0xE];
        break;
    case 4:
        p = &fbm_pics[m->dest];
        break;
    default:
        switch (m->cur) {
        default:
            p = &fbm_pics[m->cur];
            break;
        case 4:
        case 0xD:
            return list;
        }
        break;
    }
    if (m->cur == 0 && m->alt != 0)
        p = &fbm_pics[19];
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, m->parts);
    list = fld_draw_cell1(list, 0x170, 0xB3, 0x193, 0xF9, 0x6D, 0x5A);
    return fld_draw_cell1(list, 0x173, 0xB6, p->x, p->y, 0x66, 0x53);
}

#ifdef NON_MATCHING /* register allocation: the original spills field_bm to the stack and keeps the list in $s1 */
void *fbm_draw_marks(void *list)
{
    int j;
    int n;
    float off;
    int x;
    int half;
    unsigned int i;
    unsigned int k;
    FieldBm *m = &field_bm;
    int size;
    const FieldBmPt *p;

    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, m->parts);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_disable(list, 4);
    for (i = 0, p = fbm_pos; (int)i < 14; i++, p++) {
        j = 0;
        if (m->blocked & (1 << i))
            continue;
        size = i == m->cur ? 0x17 : 0x12;
        n = 0;
        for (k = 0; (int)k < 7; k++) {
            if (m->marks[i] & (1 << k))
                n++;
        }
        if (n == 0)
            continue;
        if (n >= 1 && n <= 3) {
            for (k = 0, x = 0; (int)k < 7; k++) {
                if (m->marks[i] & (1 << k)) {
                    list = fbm_icon(list, k, p->x + (int)((x - 4) - 0.5f * (float)((n - 1) * size)), p->y + 8,
                                    size, size);
                    x += size - 1;
                }
            }
        } else {
            half = (int)ceilf_vfpu(0.5f * n);
            off = 0.5f * (float)((half - 1) * size);
            for (k = 0, x = 0; (int)k < 7; k++) {
                if (m->marks[i] & (1 << k)) {
                    if (j < half)
                        list = fbm_icon(list, k, p->x + (int)((x - 4) - off), p->y + 8, size, size);
                    else
                        list = fbm_icon(list, k,
                                        p->x + (int)((-4.0f - off) + (0.5f * size + (float)((size - 1) * (j - half)))),
                                        p->y + (size + 6), size, size);
                    x += size - 1;
                    j++;
                }
            }
        }
    }
    return list;
}

/* fbm_icon: the stack argument of fld_draw_cell is stored one instruction earlier */
void *fbm_icon(void *list, int i, int x, int y, int w, int h)
{
    FieldBm *m = &field_bm;
    const FieldBmPt *p = &fbm_icons[i];

    if (i == 4 && m->alt != 0)
        p = &fbm_icons[7];
    return fld_draw_cell(list, x, y, p->x, p->y, 0x17, 0x17, w, h);
}
#endif

void *fbm_draw_arrows(void *list)
{
    FieldBm *m = &field_bm;
    unsigned int i;
    const FieldBmPt *p;
    EhfOpts o;
    EhfOpts o2;

    memset(&o, 0, sizeof(o));
    o.time = *gfx_get_state();
    o.flags |= 0x10000;
    for (i = 0, p = fbm_pos; (int)i < 14; i++, p++) {
        if (m->blocked & (1 << i))
            continue;
        if (i == 0xD) {
            memset(&o2, 0, sizeof(o2));
            if (m->cur == 0xD)
                o2.time = *gfx_get_state();
            o2.x = p->x;
            o2.flags |= 0x10000;
            o2.y = p->y;
            list = gfx_begin_2d(list);
            list = ehf_draw(list, m->arrow[3], &o2);
            list = gfx_end_2d(list);
        }
        o.x = p->x;
        o.y = p->y;
        if (m->state == 4 && i == m->cur) {
            list = gfx_begin_2d(list);
            list = ehf_draw(list, m->arrow[2], &o);
            list = gfx_end_2d(list);
        }
        list = gfx_begin_2d(list);
        list = ehf_draw(list, m->arrow[0], &o);
        list = gfx_end_2d(list);
    }
    return list;
}

#ifdef NON_MATCHING /* the stack frame has 8 more bytes under the ehf options */
void *fbm_draw_cursor(void *list)
{
    Vec2 b;
    Vec2 a;
    Vec2 p;
    EhfOpts o;

    memset(&o, 0, sizeof(o));
    o.time = *gfx_get_state();
    o.flags |= 0x10000;
    b.x = fbm_pos[field_bm.cur].x;
    b.y = fbm_pos[field_bm.cur].y;
    a.x = fbm_pos[field_bm.prev].x;
    a.y = fbm_pos[field_bm.prev].y;
    vec2_lerp(&p, &a, &b, field_bm.move_t);
    o.x = p.x;
    o.y = p.y;
    list = gfx_begin_2d(list);
    list = ehf_draw(list, field_bm.arrow[1], &o);
    return gfx_end_2d(list);
}
#endif

void fbm_fade(int out, unsigned int color, int len, int layer)
{
    field_bm.fade.on = 1;
    field_bm.fade.out = out;
    field_bm.fade.t = 0;
    field_bm.fade.len = len;
    field_bm.fade.color = color;
    field_bm.fade.cur = 0;
    field_bm.fade.layer = layer;
}

void fbm_fade_tick(void)
{
    FieldBm *m = &field_bm;
    float r;
    unsigned int c;
    unsigned int k;
    unsigned char al;

    if (field_bm.fade.on == 0)
        return;
    r = (float)m->fade.t / (float)m->fade.len;
    c = m->fade.color;
    k = (c >> 24) & 0xFF;
    if (m->fade.out != 0)
        al = k * (1.0f - r);
    else
        al = k * r;
    m->fade.cur = (c & 0xFFFFFF) | al << 24;
    m->fade.t++;
    if (m->fade.len < m->fade.t)
        m->fade.on = 0;
}
