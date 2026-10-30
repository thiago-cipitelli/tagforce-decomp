// @range 0x08811718-0x0881299C
// @rodata 0x0886FE80-0x0886FEA0
// @data 0x08874FA0-0x08875104
// @asm 0x08811718-0x08811BE0
// @asm 0x08811F88-0x088120FC
// @asm 0x08812214-0x0881230C
// @asm 0x088124A4-0x0881299C
/* The LP and phase panel: the frame (ehf), the names of the duelists, the LP counters counting
   to their new value, turn and card counters, the results of a series of duels, the phase
   animation (states of the phase ehf) and the turn switch. */
#include "dd_d1.h"

void *gfx_begin_2d(void *list);
void *gfx_end_2d(void *list);
unsigned int *gfx_get_state(void);
void *ehf_draw(void *list, void *ehf, void *opts);
unsigned int ehf_get_frame_count(void *e);
int ehf_set_frame_alias(void *e, unsigned int frame);
unsigned short *sysmsg_get(int i);
void font_text_set(void *list);
void *font_text_clear(void);
void font_set_size(int w, int h);
unsigned int font_set_color(unsigned int c);
void font_set_flag4(int on);
void font_set_flags(unsigned short flags);
unsigned short font_get_flags(void);
int font_measure_text(unsigned short *text);
int font_draw_text(int x, int y, int w, unsigned short *text);
int font_draw_wtext(int x, int y, int w, unsigned short *text);
void *draw_sprite(void *list, int x, int y, int z, int u, int v, int a, int b);
void *dl_ambient_color(void *list, unsigned int color);
void *dd_sprite_at(void *list, int x, int y, DdUv *uv);
int dd_cobjs_map_get(int side);
void *dd_game_slot(int i);
int dd_input_player(void);
int dd_game_flag119(void);
int dd_series_result(int player, int n);
int dd_08807024(void);
int dd_08806BA4(void);
void *func_08816FF8(void *list);
void func_088172BC(void);
int func_08864F58(void);
int func_0886677C(void);
int func_08866970(int side);
void *dd_face_panel(void *list, int x, int y, int side);
void *dd_turn_icon(void *list);

extern DdUv dd_uv[86];                 /* 0x0887516C sprites of duel_thing01.gim */

typedef struct DdInt6 {
    int v[6];
} DdInt6;

typedef struct DdInt2 {
    int v[2];
} DdInt2;

DdInt6 dd_lp_series_x = { { 0x9F, 0xAA, 0xB5, 0x1B8, 0x1C3, 0x1CE } };
DdInt2 dd_lp_x = { { 0xA6, 0x1B4 } };

void *dd_lp_frame(void *list);
void *dd_lp_counters(void *list);
void *dd_lp_turn(void *list, int x, int y);
void *dd_lp_count(void *list, int x, int y, int n, int active);
void dd_lp_count_tick(float dt);
void dd_lp_turn_tick(float dt);
void dd_lp_phase_tick(float dt);

#ifdef NON_MATCHING /* função grande de desenho; forma não reproduzida */
#ifdef NON_MATCHING /* forma/alocação não reproduzidas */
/* draws the whole panel on layer 6 */
void dd_lp_draw(void)
{
    int opts[24];
    DdInt6 sx;
    unsigned short flags;
    unsigned short *name;
    int w;
    void *list;
    void *gim;
    int p;
    int n;
    int r;
    int s;
    int pct;

    memset(opts, 0, 0x60);
    opts[0] = *gfx_get_state();
    list = dl_select_layer(6, 0);
    list = gfx_begin_2d(list);
    list = dd_lp_frame(list);
    list = func_08816FF8(list);
    list = gfx_end_2d(list);
    dl_set_list(list);
    list = dl_select_layer(6, 0);
    flags = font_get_flags();
    font_text_set(list);
    font_set_size(12, 12);
    font_set_color(0xFFFFFFFF);
    font_set_flag4(1);
    font_draw_wtext(0x3480, 0x40, 0x1580, dd_game_slot(dd_cobjs_map_get(0)));
    name = dd_game_slot(dd_cobjs_map_get(1));
    w = font_measure_text(name) >> 6;
    if (w > 0x57)
        w = 0x57;
    font_draw_wtext((0x1A2 - w) * 64, 0x40, (w + 2) * 64, name);
    list = font_text_clear();
    font_set_flags(flags);
    dl_set_list(list);
    list = dl_select_layer(6, 0);
    gim = dd_gim_get(0x40006);
    if (gim)
        list = gim_set_texture(list, gim);
    list = dd_lp_counters(list);
    list = dd_lp_turn(list, 0x12E, 8);
    list = dd_lp_count(list, 0xBF, 0xF, func_08866970(0), dd_input_player() == 0);
    list = dd_lp_count(list, 0x186, 0xF, func_08866970(1), dd_input_player());
    list = dl_ambient_color(list, 0xFFFFFFFF);
    if (dd_game_flag119() == 0) {
        sx = dd_lp_series_x;
        for (p = 0; p < 2; p++) {
            for (n = 0; n < 3; n++) {
                r = dd_series_result(p, n);
                if (r == -1)
                    continue;
                if (r == 2)
                    s = 0x20;
                else if (r == 1)
                    s = 0x1F;
                else if (r == 0)
                    s = 0x1E;
                list = dd_sprite_at(list, sx.v[p * 3 + n], 0xF, &dd_uv[s]);
            }
        }
    } else {
        list = dd_sprite_at(list, 0xA0, 0xF, &dd_uv[0x21]);
        list = dd_sprite_at(list, 0x1BD, 0xF, &dd_uv[0x21]);
    }
    pct = dd_08807024();
    if (pct >= 0x47)
        list = dd_sprite_at(list, 0x73, 2, &dd_uv[0x44]);
    else if (pct >= 0x1F)
        list = dd_sprite_at(list, 0x73, 2, &dd_uv[0x45]);
    else if (pct >= 1)
        list = dd_sprite_at(list, 0x73, 2, &dd_uv[0x46]);
    else if (pct == 0)
        list = dd_sprite_at(list, 0x73, 2, &dd_uv[0x47]);
    if (func_08864F58()) {
        gim = dd_gim_get(0x4001F);
        if (gim)
            list = gim_set_texture(list, gim);
        list = draw_set_offsets(list, 0x2B, 0xC, 0x2B, 0xC);
        list = draw_sprite(list, 0x197, 0x104, 0, 0, 0x34, 0, 0);
        name = sysmsg_get(0x15);
        font_text_set(list);
        font_set_size(11, 11);
        font_set_color(0xFFFFFFFF);
        font_draw_text(0x70C0, 0x4100, 0x740, name);
        list = font_text_clear();
    }
    list = dd_face_panel(list, 0x93, 0x1B, 0);
    list = dd_face_panel(list, 0x196, 0x1B, 1);
    list = dd_turn_icon(list);
    dl_set_list(list);
}
#endif
#endif

/* the phase panel (ehf 0x20003) */
void *dd_lp_phase_draw(void *list)
{
    int opts[24];
    void *e;

    memset(opts, 0, 0x60);
    opts[0] = *gfx_get_state();
    list = gfx_begin_2d(list);
    e = dd_ehf_get(0x20003);
    if (e)
        list = ehf_draw(list, e, opts);
    return gfx_end_2d(list);
}

void dd_lp_update(float dt)
{
    func_088172BC();
    dd_lp_count_tick(dt);
    dd_lp_turn_tick(dt);
    dd_lp_phase_tick(dt);
    dd_lp.frame = (unsigned int)((float)dd_lp.frame + dt * 30.0f);
}

/* LP of player p (now: no count) */
void dd_lp_set(int p, int lp, int now)
{
    DdLp *l = &dd_lp;

    dd_lp.lp_to[p] = lp;
    if (now)
        l->lp[p] = lp;
}

int dd_lp_get(int p)
{
    return dd_lp.lp_to[p];
}

void dd_lp_add(int p, int d)
{
    DdLp *l = &dd_lp;

    if ((dd_lp.lp_to[p] += d) < 0)
        l->lp_to[p] = 0;
}

/* turn of side `side` (now: no animation) */
void dd_lp_turn_set(int side, int now)
{
    DdLp *l = &dd_lp;

    dd_lp.turn = side;
    dd_lp.phase = 7;
    if (now)
        l->turn_shown = side;
}

void dd_lp_phase_set(int phase, int now)
{
    DdLp *l = &dd_lp;
    void *e;

    dd_lp.phase = phase;
    if (now) {
        l->phase_state = 0x1A;
        e = dd_ehf_get(0x20003);
        if (e)
            ehf_set_frame_alias(e, 0x20E);
    }
}

/* phase animation state of `phase` for side `side` */
void dd_lp_phase_state(int side, int phase)
{
    DdLp *l = &dd_lp;

    switch (phase) {
    default:
        l->phase_state = 1;
        break;
    case 1:
        l->phase_state = 3;
        break;
    case 2:
        l->phase_state = 5;
        break;
    case 3:
        l->phase_state = 7;
        break;
    case 4:
        l->phase_state = 9;
        break;
    case 5:
        l->phase_state = 11;
        break;
    case 7:
        l->phase_state = 0x1A;
        break;
    }
    if (side == 1 && phase != 7)
        l->phase_state += 13;
}

/* number n (0..99999) right-aligned at x+32, digits of the LP font */
void *dd_lp_digits(void *list, int x, int y, int n)
{
    int dx;

    if (n < 0)
        n = 0;
    if (n > 99999)
        n = 99999;
    dx = x + 32;
    do {
        list = dd_sprite_at(list, dx, y, &dd_uv[n % 10 + 0x22]);
        n /= 10;
        dx -= 8;
        if (n <= 0)
            break;
    } while (dx >= x);
    return list;
}

#ifdef NON_MATCHING /* forma/alocação não reproduzidas */
/* card count of a deck (blinking under 31) with its icon */
void *dd_lp_count(void *list, int x, int y, int n, int active)
{
    int dx;
    int first;
    int d;

    if (n >= 0) {
        if (n > 999)
            n = 999;
        if (active == 0) {
            list = dl_ambient_color(list, 0x80FFFFFF);
        } else if (n < 31) {
            if (*gfx_get_state() >> 3 & 1)
                list = dl_ambient_color(list, 0xFF00FFFF);
            else
                list = dl_ambient_color(list, 0xFF000000);
        } else {
            list = dl_ambient_color(list, 0xFFFFFFFF);
        }
        list = dd_sprite_at(list, x, y, &dd_uv[0x5D - 0x2D]);
        dx = x + 0x29;
        first = 1;
        do {
            d = n % 10;
            if (first || d != 0 || n != 0)
                list = dd_sprite_at(list, dx, y, &dd_uv[d + 0x2D]);
            first = 0;
            n /= 10;
            dx -= 8;
        } while (n > 0 && dx >= x);
    }
    return list;
}
#endif

/* turn number centred at x */
void *dd_lp_turn(void *list, int x, int y)
{
    int n = dd_08806BA4();
    int dx;
    int first;
    int d;

    if (n > 999)
        n = 999;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    if (n >= 100)
        dx = x + 16;
    else if (n >= 10)
        dx = x + 12;
    else
        dx = x + 8;
    first = 1;
    do {
        d = n % 10;
        if (first || d != 0 || n != 0)
            list = dd_sprite_at(list, dx, y, &dd_uv[d + 0x22]);
        first = 0;
        n /= 10;
        dx -= 8;
    } while (n > 0 && dx >= x);
    return list;
}

#ifdef NON_MATCHING /* forma/alocação não reproduzidas */
/* the LP of both players */
void *dd_lp_counters(void *list)
{
    DdInt2 x = dd_lp_x;
    DdLp *l = &dd_lp;
    int p;
    int *px;

    list = dd_sprite_at(list, 0x98, 1, &dd_uv[0x54]);
    list = dd_sprite_at(list, 0x9F, 1, &dd_uv[0x55]);
    list = dd_sprite_at(list, 0x1A6, 1, &dd_uv[0x54]);
    list = dd_sprite_at(list, 0x1AD, 1, &dd_uv[0x55]);
    for (p = 0, px = x.v; p < 2; p++, px++) {
        list = dl_ambient_color(list, 0xFF00FFFF);
        list = dd_lp_digits(list, *px, 1, l->lp[p]);
    }
    return list;
}
#endif

/* LP counting to their new value in 60 frames */
void dd_lp_count_tick(float dt)
{
    DdLp *l = &dd_lp;
    int p;
    int f;

    if (!(dt > 0.0f))
        return;
    for (p = 0; p < 2; p++) {
        if (l->lp[p] == l->lp_to[p])
            continue;
        if (l->lp_to[p] != l->lp_dest[p]) {
            l->lp_t0[p] = *gfx_get_state();
            l->lp_dest[p] = l->lp_to[p];
            l->lp_from[p] = l->lp[p];
        }
        f = *gfx_get_state() - l->lp_t0[p];
        if (f >= 60)
            l->lp[p] = l->lp_to[p];
        else
            l->lp[p] = (l->lp_from[p] * (60 - f) + l->lp_dest[p] * f) / 60;
    }
}

/* the panel frame (ehf 0x20002), grey in a tag duel */
void *dd_lp_frame(void *list)
{
    int opts[24];
    void *e = dd_ehf_get(0x20002);

    if (e == 0)
        return list;
    memset(opts, 0, 0x60);
    opts[0] = *gfx_get_state();
    if (dd_game_flag119()) {
        opts[3] |= 8;
        opts[12] = 0xFFFFFFEF;
        opts[13] = 0xFFFFFFFF;
    }
    return ehf_draw(list, e, opts);
}

#ifdef NON_MATCHING /* forma/alocação não reproduzidas */
/* turn switch animation of the panel frame */
void dd_lp_turn_tick(float dt)
{
    void *e = dd_ehf_get(0x20002);
    unsigned int f;

    if (e == 0)
        return;
    if (dd_lp.turn_shown == -1) {
        if (ehf_get_frame_count(e) > 0x80) {
            dd_lp.turn_shown = dd_lp.turn;
            if (dd_lp.turn == 0)
                ehf_set_frame_alias(e, 0);
            else
                ehf_set_frame_alias(e, 0x82);
        }
    } else if ((dd_lp.turn_shown == 1 || dd_lp.turn_shown == 0) && dd_lp.turn_shown != dd_lp.turn) {
        dd_lp.turn_shown = -1;
        ehf_set_frame_alias(e, 0x79);
    }
    f = ehf_get_frame_count(e);
    if (dd_lp.turn_shown == -1) {
        if (f > 0x80)
            ehf_set_frame_alias(e, 0x79);
    } else if (dd_lp.turn_shown == 1) {
        if (f > 0xF9)
            ehf_set_frame_alias(e, 0x82);
    } else if (dd_lp.turn_shown == 0 && f > 0x77) {
        ehf_set_frame_alias(e, 0);
    }
}
#endif

int dd_lp_phase_of[27] = {
    7, 0, 7, 1, 7, 2, 7, 3, 7, 4, 7, 5, 7, 7, 0, 7, 1, 7, 2, 7, 3, 7, 4, 7, 5, 7, 7,
};

typedef struct DdRange {
    int start;
    int end;
} DdRange;

DdRange dd_lp_phase_frames[27] = {
    { 0x00, 0x11 }, { 0x12, 0x31 }, { 0x32, 0x36 }, { 0x36, 0x55 }, { 0x56, 0x5A }, { 0x5A, 0x79 },
    { 0x7A, 0x7E }, { 0x7E, 0x9D }, { 0x9E, 0xA2 }, { 0xA2, 0xC1 }, { 0xC2, 0xC6 }, { 0xC6, 0xE5 },
    { 0xE6, 0x107 }, { 0x108, 0x119 }, { 0x11A, 0x139 }, { 0x13A, 0x13E }, { 0x13E, 0x15D },
    { 0x15E, 0x162 }, { 0x162, 0x181 }, { 0x182, 0x186 }, { 0x186, 0x1A5 }, { 0x1A6, 0x1AA },
    { 0x1AA, 0x1C9 }, { 0x1CA, 0x1CE }, { 0x1CE, 0x1ED }, { 0x1EE, 0x20F }, { 0x20E, 0x20F },
};

#ifdef NON_MATCHING /* switch com tabela e testes repetidos; forma não reproduzida */
#ifdef NON_MATCHING /* forma/alocação não reproduzidas */
/* state machine of the phase panel: plays each phase in turn up to the current one */
void dd_lp_phase_tick(float dt)
{
    void *e = dd_ehf_get(0x20003);
    int f;
    int s;

    if (e == 0)
        return;
    f = ehf_get_frame_count(e);
    if (dd_lp.phase_state < 0x1B) {
        switch (dd_lp.phase_state) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 9:
        case 11:
        case 14:
        case 16:
        case 18:
        case 20:
        case 22:
        case 24:
            if (dd_lp.phase != dd_lp_phase_of[dd_lp.phase_state] ||
                ((dd_lp.turn != 0 || dd_lp.phase_state > 12) && (dd_lp.turn != 1 || dd_lp.phase_state < 13))) {
                dd_lp.phase_state++;
                ehf_set_frame_alias(e, dd_lp_phase_frames[dd_lp.phase_state].start);
            }
            break;
        case 12:
            if (f >= dd_lp_phase_frames[dd_lp.phase_state].end) {
                if (dd_lp.phase == 7) {
                    ehf_set_frame_alias(e, dd_lp_phase_frames[dd_lp.phase_state].end - 1);
                } else {
                    dd_lp.phase_state = 13;
                    ehf_set_frame_alias(e, dd_lp_phase_frames[13].start);
                }
            }
            break;
        case 25:
            if (f >= dd_lp_phase_frames[dd_lp.phase_state].end) {
                if (dd_lp.phase == 7) {
                    ehf_set_frame_alias(e, dd_lp_phase_frames[dd_lp.phase_state].end - 1);
                } else {
                    dd_lp.phase_state = 0;
                    ehf_set_frame_alias(e, dd_lp_phase_frames[0].start);
                }
            }
            break;
        case 26:
            if (dd_lp.phase != dd_lp_phase_of[dd_lp.phase_state]) {
                dd_lp.phase_state = func_0886677C() == 0 ? 0 : 13;
                ehf_set_frame_alias(e, dd_lp_phase_frames[dd_lp.phase_state].start);
            }
            break;
        default:
            if (f >= dd_lp_phase_frames[dd_lp.phase_state].end) {
                s = dd_lp.phase_state + 1;
                if (dd_lp.phase != dd_lp_phase_of[s] || (dd_lp.turn == 0 && s > 12) ||
                    (dd_lp.turn == 1 && s < 13))
                    s = dd_lp.phase_state + 2;
                dd_lp.phase_state = s;
                ehf_set_frame_alias(e, dd_lp_phase_frames[dd_lp.phase_state].start);
            }
            break;
        }
    }
    f = ehf_get_frame_count(e);
    s = dd_lp.phase_state;
    if ((s == 26 || s == 24 || s == 22 || s == 20 || s == 18 || s == 16 || s == 14 || s == 11 ||
         s == 9 || s == 7 || s == 5 || s == 3 || s == 1) &&
        f >= dd_lp_phase_frames[s].end)
        ehf_set_frame_alias(e, dd_lp_phase_frames[s].start);
}
#endif
#endif
