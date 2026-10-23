// @range 0x0880F9C0-0x08815888
// @data 0x0881B9B0-0x0881BC88
// @asm 0x0880F9C0-0x0880FEAC
// @asm 0x0880FEAC-0x08810750
// @asm 0x0881093C-0x088109EC
// @asm 0x08810A5C-0x08810BA0
// @asm 0x08810C2C-0x08810DCC
// @asm 0x08810DCC-0x088111DC
// @asm 0x088111DC-0x08811270
// @asm 0x08811270-0x08811460
// @asm 0x08811460-0x088116F4
// @asm 0x088116F4-0x08811754
// @asm 0x08811754-0x0881185C
// @asm 0x088119B4-0x08811BC4
// @asm 0x08811BC4-0x08811C98
// @asm 0x08811C98-0x08812124
// @asm 0x08812124-0x088122B4
// @asm 0x088122B4-0x08812A98
// @asm 0x08812A98-0x08812D74
// @asm 0x08812D74-0x08812F2C
// @asm 0x08812F2C-0x088131A8
// @asm 0x088131A8-0x088133B8
// @asm 0x088133B8-0x088137FC
// @asm 0x088137FC-0x088139B4
// @asm 0x088139B4-0x08813C0C
// @asm 0x08813C0C-0x08814274
// @asm 0x08814274-0x088146E8
// @asm 0x088146E8-0x0881499C
// @asm 0x0881499C-0x08814C60
// @asm 0x08814C60-0x08814DCC
// @asm 0x08814DCC-0x08815050
// @asm 0x08815050-0x08815328
// @asm 0x08815328-0x088153E0
/* Result screen after a duel: bonus points, records and titles earned (three scrolling pages over a
   summary), the money paid out, the series score of a match and the continue / net wait dialogs. */
#include "libduel_mgr.h"

/* BOOT */
int cfg_get_language(void);
int sprintf(char *buf, const char *fmt, ...);
void *ehp_find(void *pack, const char *name, int *size);
int ehp_mount(void *cb, void *pack);
int ehf_set_anim(void *e, int target, int limit);
void ehf_get_size(void *e, int *target, int *limit);
int ehf_get_frame_count(void *e);
int fade_is_done(void);
unsigned short *table_get_entry(void *t, unsigned int i);
unsigned short region_pick_value(int flag);
void se_play(int id);
int net_get_alive(void);
void net_set_watch(int on);
int peers_all_match(int a, int b, int c);
int peer_count_get(void);
void local_record_set(unsigned char *rec);
void money_add(int delta);
SaveExtra *sysstate2_get_ptr_51c(void);
void font_set_flag1(int on);
void font_set_flag2(int on);
void font_set_flag4(int on);
void font_set_flag8(int on);
void font_set_size(unsigned char w, unsigned char h);
int font_line_height(void);
int font_wtext_lines(int w, void *text);
int font_print(void *text);
unsigned char font_measure_wtext(int w, void *text);
void font_set_line_extra(int n);
void font_text_set(void *list);
unsigned int *font_text_clear(void);
int font_draw_wtext(int x, int y, int w, void *text);
int font_draw_wtext_ex(int x, int y, int w, int h, void *text, unsigned int color);
int font_draw_text(int x, int y, int w, void *text);
void *gfx_get_state(void);
unsigned int *gfx_begin_2d(unsigned int *list);
unsigned int *gfx_end_2d(unsigned int *list);
unsigned int *ehf_draw(unsigned int *list, void *e, void *opts);
unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_scissor(unsigned int *list, int x, int y, int w, int h);
unsigned int *dl_tex_func(unsigned int *list, int func, int comp);
unsigned int *dl_blend_func(unsigned int *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
unsigned int *dl_clear_color(unsigned int *list, unsigned int color);
unsigned int *dl_clear_depth(unsigned int *list, unsigned int z);
unsigned int *dl_clear_stencil(unsigned int *list, unsigned int s);
unsigned int *dl_clear(unsigned int *list, unsigned int flags);
unsigned int *gim_set_texture(unsigned int *list, void *g);
unsigned int *dl_ambient_color(unsigned int *list, unsigned int color);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
void *libduel_draw_0881CA74(void);

/* this module */
void dmgr_res_free(void);
void dmgr_res_bg_anim(unsigned short mode);
int dmgr_res_bg_done(void);
void dmgr_res_result_anim(short mode);
int dmgr_res_result_done(void);
short dmgr_res_series(void);
void dmgr_res_input(void);
void dmgr_res_slide(void);
void dmgr_res_measure(void);
void dmgr_res_menu_open(short kind);
void dmgr_res_send(unsigned char a, unsigned char b);
void *dmgr_res_load(void);
int dmgr_res_load_done(void *pack, int r);
void *dmgr_res_file(unsigned short i);
unsigned int *dmgr_res_draw_bg(unsigned int *list);
unsigned int *dmgr_res_draw_bar(unsigned int *list);
unsigned int *dmgr_res_draw_result(unsigned int *list);
unsigned int *dmgr_res_draw_footer(unsigned int *list);
unsigned int *dmgr_res_draw_page(unsigned int *list);
unsigned int *dmgr_res_page_summary(unsigned int *list, int x);
unsigned int *dmgr_res_summary_row(unsigned int *list, int x, int y, int page);
unsigned int *dmgr_res_page_bonus(unsigned int *list, int x);
unsigned int *dmgr_res_bonus_row(unsigned int *list, int x, int y, int i);
unsigned int *dmgr_res_page_records(unsigned int *list, int x);
unsigned int *dmgr_res_record_row(unsigned int *list, int x, int y, int i);
unsigned int *dmgr_res_page_titles(unsigned int *list, int x);
unsigned int *dmgr_res_title_row(unsigned int *list, int x, int y, int i);
unsigned int *dmgr_res_draw_series(unsigned int *list);
unsigned int *dmgr_res_page_frame(unsigned int *list, int x, int u, int v);
unsigned int *dmgr_res_page_header(unsigned int *list, int x, int page);
unsigned int *dmgr_res_draw_box(unsigned int *list, int x, int y, int w, int h, int u, int v);
unsigned int *dmgr_res_draw_number(unsigned int *list, unsigned short x, short y, int n, unsigned short step,
                                   unsigned short digits, int pad);
unsigned int *dmgr_res_draw_arrow(unsigned int *list, int x, int y, char down, unsigned int period, int blink,
                                  int on, unsigned int frame, void *gim);
unsigned int *dmgr_res_draw_scrollbar(unsigned int *list, int x, int y, int h, int pos, int rows, int count);
void dmgr_res_text_fit(int x, int y, int w, int sw, int sh, void *text);
unsigned int *dmgr_res_series_name(unsigned int *list, int x, int y, int partner, void *name);
unsigned int *dmgr_res_series_mark(unsigned int *list, int x, int y, unsigned short result);

unsigned short dmgr_res_rec_text[40] = {
    0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F, 0x21, 0x23, 0x25, 0x27, 0x29, 0x2B, 0x2D, 0x1DD, 0x2F,
    0x31, 0x33, 0x35, 0x37, 0x39, 0x3B, 0x3D, 0x3F, 0x41, 0x43, 0x45, 0x47, 0x49, 0x4A, 0x4D, 0x4F,
    0x51, 0x53, 0x55, 0x57, 0x20B, 0x59, 0x5B, 0x5D,
};
unsigned short dmgr_res_bonus_text[0x53] = {
    0x5F, 0x61, 0x63, 0x65, 0x67, 0x69, 0x1E1, 0x6B, 0x6D, 0x6F, 0x1E3, 0x71, 0x73, 0x75, 0x77,
    0x79, 0x7B, 0x7D, 0x7F, 0x81, 0x83, 0x85, 0x87, 0x89, 0x8B, 0x8D, 0x8F, 0x91, 0x93, 0x95, 0x97,
    0x99, 0x9B, 0x9D, 0x9F, 0xA1, 0xA3, 0xA5, 0xA7, 0xA9, 0xAB, 0xAD, 0xAF, 0xB1, 0xB3, 0xB5, 0xB7,
    0xB9, 0xBB, 0xBD, 0xBF, 0x20D, 0xC1, 0xC3, 0xC5, 0xC7, 0xC9, 0xCB, 0xCD, 0xCF, 0xD1, 0xD3, 0xD5,
    0xD7, 0xD9, 0xDB, 0xDD, 0xDF, 0xE1, 0x20F, 0x211, 0xE3, 0xE5, 0xE7, 0xE9, 0xEB, 0xED, 0xEF,
    0xF1, 0xF3, 0x213, 0x215, 0x1DF,
};
unsigned short dmgr_res_title_text[0x76] = {
    0xF5, 0xF7, 0xF9, 0xFB, 0xFD, 0xFF, 0x101, 0x103, 0x105, 0x107, 0x109, 0x10B, 0x10D, 0x10F,
    0x111, 0x113, 0x115, 0x117, 0x119, 0x11B, 0x11D, 0x11F, 0x121, 0x123, 0x125, 0x127, 0x129,
    0x12B, 0x12D, 0x12F, 0x131, 0x133, 0x135, 0x137, 0x139, 0x13B, 0x13D, 0x13F, 0x141, 0x143,
    0x145, 0x147, 0x149, 0x14B, 0x14D, 0x14F, 0x151, 0x153, 0x217, 0x155, 0x157, 0x159, 0x15B,
    0x15D, 0x15F, 0x161, 0x219, 0x163, 0x165, 0x167, 0x169, 0x16B, 0x16D, 0x16F, 0x171, 0x173,
    0x175, 0x177, 0x179, 0x17B, 0x17D, 0x17F, 0x181, 0x183, 0x185, 0x187, 0x189, 0x18B, 0x18D,
    0x18F, 0x191, 0x193, 0x195, 0x197, 0x199, 0x19B, 0x19D, 0x19F, 0x1A1, 0x1A3, 0x1A5, 0x1A7,
    0x1A9, 0x1AB, 0x1AD, 0x1AF, 0x1B1, 0x1B3, 0x1B5, 0x1B7, 0x1B9, 0x1BB, 0x1BD, 0x1BF, 0x1C1,
    0x1C3, 0x1C5, 0x1C7, 0x1C9, 0x1CB, 0x1CD, 0x1CF, 0x1D1, 0x1D3, 0x1D5, 0x1D7, 0x1D9, 0x1DA,
};
char *dmgr_res_files[8] = {
    "menu_bg.ehf", "menu_bar.ehf", "win.ehf", "lose.ehf", "draw.ehf", "menu01.gim", "menu02.gim", "strTbl.bin",
};
unsigned short dmgr_res_pt[4] = { 'D', 'P', 0, 0 };

unsigned short dmgr_res_div[4] = { 0, 5, 6, 8 };

#ifdef NON_MATCHING /* função grande: alocação de registradores e ordem dos blocos diferentes */
void dmgr_result_init(void)
{
    void *heap = libduel_draw_0881CA74();
    DmgrResTab *t;
    DmgrResItem *it;
    SaveExtra *ex;
    unsigned short *div = dmgr_res_div;
    int partners;
    int total;
    int pay;
    int n;
    int i;
    int k;

    dmgr_res = heap_alloc(heap, sizeof *dmgr_res);
    memset(dmgr_res, 0, sizeof *dmgr_res);
    dmgr_res->heap = heap;
    t = &dmgr_res->tabs[1];
    t->ids = heap_alloc(heap, 0xA6);
    t->lines = heap_alloc(heap, 0xA6);
    t = &dmgr_res->tabs[2];
    t->ids = heap_alloc(heap, 0x50);
    t->lines = heap_alloc(heap, 0x50);
    t = &dmgr_res->tabs[3];
    t->ids = heap_alloc(heap, 0xEC);
    t->lines = heap_alloc(heap, 0xEC);
    dmgr_res->rec = (DmgrResRec *)((char *)game_state2_get() + 0x3C);
    t = &dmgr_res->tabs[1];
    n = 0;
    it = (DmgrResItem *)dmgr_res_bonus_text;
    for (i = 0, k = 0; i < 0x53; i++, it++) {
        if (it->on && dmgr_res->rec->bonus[i] != 0) {
            n++;
            dmgr_res->total[0] += dmgr_res->rec->bonus[i];
            t->ids[k++] = i;
        }
    }
    t->count = n;
    it = (DmgrResItem *)dmgr_res_rec_text;
    for (i = 0; i < 0x28; i++, it++) {
        if (it->on && dmgr_res->rec->rec[i] != 0) {
            dmgr_res->total[1] += 20;
        }
    }
    t = &dmgr_res->tabs[3];
    n = 0;
    it = (DmgrResItem *)dmgr_res_title_text;
    for (i = 0, k = 0; i < 0x76; i++, it++) {
        if (it->on && dmgr_res->rec->title[i] == 1) {
            n++;
            dmgr_res->total[2] += 30;
            t->ids[k++] = i;
        }
    }
    t->count = n;
    partners = 0;
    for (i = 0; i < partner_count_get(); i++) {
        k = ((DmgrGameHdr *)game_state_get())->extra[i];
        if (k == 3 || k == 1) {
            partners++;
        }
    }
    ex = sysstate2_get_ptr_51c();
    total = dmgr_res->total[0] + dmgr_res->total[1] + dmgr_res->total[2] + dmgr_res->total[3];
    pay = total;
    if (partners != 0) {
        pay = total * 4 / div[partners];
    }
    if (total < 0) {
        total = 0;
    }
    if (*(int *)&ex->unk0[8] < pay) {
        total += 20;
        dmgr_res->total[1] += 20;
        pay = total;
        if (partners != 0) {
            pay = total * 4 / div[partners];
        }
        dmgr_res->rec->rec[2] = pay;
        *(int *)&ex->unk0[8] = pay;
        ex->unk0[0xA2] = 1;
    }
    money_add(pay);
    dmgr_res->total[3] = -(total - pay);
    t = &dmgr_res->tabs[2];
    n = 0;
    for (i = 0, k = 0; i < 0x28; i++) {
        if (dmgr_res->rec->rec[i] != 0) {
            n++;
            t->ids[k++] = i;
        }
    }
    t->count = n;
    dmgr_res_load();
    dmgr_res->result_ehf = -1;
    font_set_flag4(1);
    font_set_flag2(1);
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
        font_set_flag8(0);
    } else {
        font_set_flag1(0);
        font_set_flag8(1);
    }
    if (sysvar_get_a98() != 0) {
        dmgr_res->n_peers = peer_count_get();
        net_set_watch(0);
        dmgr_res_send(1, 0);
        dmgr_res->net = 1;
    }
}
#endif

#ifdef NON_MATCHING /* função grande: alocação de registradores e ordem dos blocos diferentes */
int dmgr_result_update(void)
{
    DmgrRes *r;
    PadState *pad;
    int ret;
    int s;
    int ask;

    if (dmgr_res->menu_kind != 0) {
        ret = menu_update(&dmgr_res->menu);
        r = dmgr_res;
        switch (r->menu_kind) {
        case 1:
            if (ret != -1) {
                r->menu_kind = 0;
                menu_reset(&dmgr_res->menu);
                if (ret == 2) {
                    dmgr_res->menu_done = 1;
                    if (sysvar_get_a98() != 0) {
                        if (dmgr_res->n_peers == net_get_alive()) {
                            dmgr_res_menu_open(2);
                            dmgr_res_send(2, 0);
                        } else {
                            dmgr_res_menu_open(3);
                        }
                    }
                }
            } else if (sysvar_get_a98() != 0 && net_get_alive() < dmgr_res->n_peers) {
                dmgr_res_menu_open(3);
            }
            return 0;
        case 2:
            if (net_get_alive() < r->n_peers) {
                menu_reset(&r->menu);
                dmgr_res_menu_open(3);
            } else if (peers_all_match(8, 2, 0xFF) != 0) {
                dmgr_res->menu_done = 1;
                menu_reset(&dmgr_res->menu);
                dmgr_res->menu_kind = 0;
            }
            return 0;
        case 3:
            if (ret != -1) {
                r->menu_done = 0;
                dmgr_res->menu_kind = 0;
                dmgr_res->loading = 0;
                menu_reset(&dmgr_res->menu);
                return 0;
            }
            return 0;
        }
    }
    switch (dmgr_res->state) {
    case 0:
        if (!dmgr_res->loading) {
            gfx_overlay_enable(0);
            dmgr_res_measure();
            fade_start(1, 0xFF000000, 0x10);
            dmgr_res_bg_anim(0);
            dmgr_res->state = 1;
        }
        break;
    case 1:
        if (fade_is_done() != 0) {
            dmgr_res_bg_anim(1);
            dmgr_res->state = 2;
        }
        break;
    case 2:
        if (dmgr_res_bg_done() != 0) {
            dmgr_res_bg_anim(2);
            dmgr_res->state = 3;
        }
        break;
    case 3:
        dmgr_res_input();
        break;
    case 4:
        dmgr_res_slide();
        break;
    case 5:
        switch (dmgr_res->sub) {
        case 0:
            dmgr_res_bg_anim(3);
            dmgr_res->sub++;
            break;
        case 1:
            if (dmgr_res_bg_done() != 0) {
                dmgr_res_bg_anim(0);
                fade_start_in(1, 0xFF000000, 0x10);
                if (((DmgrGameHdr *)game_state_get())->kind2 != 2) {
                    bgm_cmd(600);
                }
                dmgr_res->sub++;
            }
            break;
        case 2:
            if (fade_is_done() != 0) {
                dmgr_res->sub++;
            }
            break;
        case 3:
            if (((DmgrGameHdr *)game_state_get())->kind2 == 2) {
                if (dmgr_res_series() == 0) {
                    dmgr_res->bar = 4;
                    fade_start(1, 0xFF000000, 0x10);
                    dmgr_res->state = 6;
                } else {
                    switch (dmgr_res_series()) {
                    case 1:
                        dmgr_res->result_ehf = 2;
                        break;
                    case 2:
                        dmgr_res->result_ehf = 3;
                        break;
                    case 3:
                        dmgr_res->result_ehf = 4;
                        break;
                    }
                    dmgr_res->bar = 5;
                    fade_start(1, 0xFF000000, 0x10);
                    dmgr_res->state = 6;
                }
            } else {
                dmgr_res->state = 10;
            }
            dmgr_res->sub = 0;
            break;
        }
        break;
    case 6:
        switch (dmgr_res->sub) {
        case 0:
            if (fade_is_done() != 0) {
                dmgr_res_bg_anim(1);
                dmgr_res->sub++;
            }
            break;
        case 1:
            if (dmgr_res_bg_done() != 0) {
                dmgr_res_bg_anim(2);
                if (dmgr_res->result_ehf < 0) {
                    dmgr_res->sub = 3;
                } else {
                    dmgr_res_result_anim(1);
                    dmgr_res->sub++;
                }
            }
            break;
        case 2:
            if (dmgr_res_result_done() != 0) {
                dmgr_res_result_anim(2);
                dmgr_res->sub++;
            }
            break;
        case 3:
            dmgr_res->state = 7;
            dmgr_res->sub = 0;
            break;
        }
        break;
    case 7:
        pad = pad_get();
        if (pad->b.trig & region_pick_value(0)) {
            s = dmgr_res_series();
            ask = 0;
            if (s != 0) {
                DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
                if (gs->rematch == 2) {
                    ask = 1;
                } else if (gs->rematch == 1 && s != 1) {
                    ask = 1;
                }
            }
            if (ask) {
                dmgr_res_menu_open(1);
                se_play(0x621A);
            } else {
                se_play(0x6213);
            }
            dmgr_res->state = 8;
            dmgr_res->sub = 0;
        }
        break;
    case 8:
        switch (dmgr_res->sub) {
        case 0:
            dmgr_res_bg_anim(3);
            dmgr_res_result_anim(3);
            dmgr_res->sub++;
            return 0;
        case 1:
            if (dmgr_res_result_done() != 0) {
                dmgr_res_result_anim(0);
                dmgr_res->sub++;
            }
        case 2:
            if (dmgr_res_bg_done() != 0) {
                dmgr_res_bg_anim(0);
                dmgr_res_result_anim(0);
                fade_start_in(1, 0xFF000000, 0x10);
                bgm_cmd(600);
                dmgr_res->state = 9;
                dmgr_res->sub = 0;
            }
            break;
        default:
            return 0;
        }
        break;
    case 9:
        if (fade_is_done() != 0) {
            dmgr_res->state = 10;
        }
        break;
    case 10:
        if (dmgr_res->menu_done) {
            ret = 2;
        } else {
            ret = 1;
        }
        if (sysvar_get_a98() != 0 && dmgr_res->net) {
            net_set_watch(1);
        }
        dmgr_res_free();
        return ret;
    }
    return 0;
}
#endif

void dmgr_result_draw(void)
{
    unsigned int *list;

    list = dl_select_layer(0, 0);
    list = dl_clear_color(list, 0xFF000000);
    list = dl_clear_depth(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear(list, 0x17);
    dl_set_list(list);
    if (dmgr_res->state != 10 && dmgr_res->state != 0) {
        list = dl_select_layer(4, 0);
        list = dmgr_res_draw_bg(list);
        list = dmgr_res_draw_bar(list);
        list = dmgr_res_draw_page(list);
        if (dmgr_res->result_on) {
            list = dmgr_res_draw_result(list);
        }
        list = dmgr_res_draw_footer(list);
        if (dmgr_res->menu_kind != 0) {
            menu_draw((void **)&list, &dmgr_res->menu);
        }
        fade_apply(&list);
        dl_set_list(list);
    }
}

void dmgr_res_free(void)
{
    void *heap = dmgr_res->heap;
    DmgrResTab *t;
    int i;

    for (i = 0; i < 4; i++) {
        t = &dmgr_res->tabs[i];
        if (t->ids != 0) {
            heap_set_level(heap, t->ids);
        }
        if (t->lines != 0) {
            heap_set_level(heap, t->lines);
        }
    }
    if (dmgr_res->pack != 0) {
        heap_set_level(heap, dmgr_res->pack);
    }
    heap_set_level(heap, dmgr_res);
    dmgr_res = 0;
}

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
void dmgr_res_bg_anim(unsigned short mode)
{
    void *e = dmgr_res_file(1);
    int from;
    int to;

    if (e != 0) {
        switch (mode) {
        case 3:
            from = 0x60;
            to = 0x65;
            break;
        case 2:
            from = 0x10;
            to = 0x60;
            break;
        case 1:
            from = 0;
            to = 0x10;
            break;
        default:
            to = 0;
            from = 0;
            mode = 0;
            break;
        }
        ehf_set_anim(e, from, to + 1);
        dmgr_res->bg = mode;
    }
}
#endif

int dmgr_res_bg_done(void)
{
    void *e;
    int target;
    int limit;
    int n;

    e = dmgr_res_file(1);
    if (e != 0) {
        ehf_get_size(e, &target, &limit);
        n = ehf_get_frame_count(e);
        if (limit > 0 && n >= limit - 1) {
            return 1;
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
void dmgr_res_result_anim(short mode)
{
    void *e;
    int from = 0;
    int to = 0;

    if (dmgr_res->result_ehf < 0) {
        return;
    }
    e = dmgr_res_file(dmgr_res->result_ehf);
    if (e == 0) {
        return;
    }
    switch (mode) {
    case 3:
        from = 0x46;
        to = 0x4A;
        dmgr_res->result_on = 1;
        break;
    case 2:
        to = 0x32;
        from = 0x32;
        dmgr_res->result_on = 1;
        break;
    case 1:
        from = 0;
        switch (dmgr_res->result_ehf) {
        case 4:
            to = 0x14;
            break;
        case 3:
            to = 0x1B;
            break;
        case 2:
            to = 0x31;
            break;
        }
        dmgr_res->result_on = 1;
        break;
    default:
        from = 0;
        dmgr_res->result_on = 0;
        break;
    }
    ehf_set_anim(e, from, to + 1);
}
#endif

int dmgr_res_result_done(void)
{
    void *e;
    int target;
    int limit;
    int n;

    if (dmgr_res->result_ehf < 0) {
        return 1;
    }
    e = dmgr_res_file(dmgr_res->result_ehf);
    if (e != 0) {
        ehf_get_size(e, &target, &limit);
        n = ehf_get_frame_count(e);
        if (limit > 0 && n >= limit - 1) {
            return 1;
        }
    }
    return 0;
}

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
short dmgr_res_series(void)
{
    int n;
    int sum;
    int i;

    if (((DuelResults *)game_state2_get())->mode == 8) {
        n = ((DuelResults *)game_state2_get())->n_duels;
        return ((DuelResults *)game_state2_get())->duels[n - 1];
    }
    switch (((DuelResults *)game_state2_get())->n_duels) {
    case 3:
        sum = 0;
        for (i = 0; i < 3; i++) {
            if (((DuelResults *)game_state2_get())->duels[i] == 1) {
                sum++;
            } else if (((DuelResults *)game_state2_get())->duels[i] == 2) {
                sum--;
            }
        }
        if (sum > 0) {
            return 1;
        }
        if (sum < 0) {
            return 2;
        }
        return 3;
    case 2:
        if (((DuelResults *)game_state2_get())->duels[0] == 1 && ((DuelResults *)game_state2_get())->duels[1] == 1) {
            return 1;
        }
        if (((DuelResults *)game_state2_get())->duels[0] == 2 && ((DuelResults *)game_state2_get())->duels[1] == 2) {
            return 2;
        }
        return 0;
    case 1:
        return 0;
    case 0:
        return 0;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* função grande: alocação de registradores e ordem dos blocos diferentes */
void dmgr_res_input(void)
{
    PadState *pad = pad_get();
    unsigned int prev = dmgr_res->tab;
    DmgrResTab *t;
    unsigned int k;
    int sum;
    int last;
    int ask;
    int i;
    int j;

    if (pad->b.trig & region_pick_value(0)) {
        ask = 0;
        if (((DmgrGameHdr *)game_state_get())->kind2 != 2) {
            DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
            if (gs->rematch == 2) {
                ask = 1;
            } else if (gs->rematch == 1 && ((DuelResults *)game_state2_get())->result != 1) {
                ask = 1;
            }
        }
        if (ask) {
            dmgr_res_menu_open(1);
            se_play(0x621A);
        } else {
            se_play(0x6213);
        }
        dmgr_res->state = 5;
        dmgr_res->sub = 0;
        return;
    }
    if (pad->b.rep & 0x180) {
        for (i = 1, j = -1;; j = -i) {
            k = (prev + 4 + j) % 4;
            if (k == 0) {
                dmgr_res->tab = 0;
                break;
            }
            if (dmgr_res->total[k - 1] > 0) {
                dmgr_res->tab = k;
                break;
            }
            if (++i > 3) {
                break;
            }
        }
        if (prev != dmgr_res->tab) {
            dmgr_res->state = 4;
            dmgr_res->slide = 0;
            dmgr_res->prev_tab = prev;
            dmgr_res->timer = 0;
            dmgr_res_bg_anim(3);
            se_play(0x6212);
        }
    } else if (pad->b.rep & 0x220) {
        k = prev;
        for (i = 1; i < 4; i++) {
            k++;
            if (k % 4 == 0) {
                dmgr_res->tab = 0;
                break;
            }
            if (dmgr_res->total[k % 4 - 1] > 0) {
                dmgr_res->tab = k % 4;
                break;
            }
        }
        if (prev != dmgr_res->tab) {
            dmgr_res->state = 4;
            dmgr_res->slide = 1;
            dmgr_res->prev_tab = prev;
            dmgr_res->timer = 0;
            dmgr_res_bg_anim(3);
            se_play(0x6212);
        }
    } else if (pad->b.rep & 0x10) {
        k = dmgr_res->tab;
        if (k != 0 && dmgr_res->tabs[k].scroll != 0) {
            dmgr_res->tabs[k].scroll--;
        }
    } else if (pad->b.rep & 0x40) {
        k = dmgr_res->tab;
        if (k != 0) {
            t = &dmgr_res->tabs[k];
            sum = 0;
            last = 0;
            for (i = t->count - 1; i >= 0; i--) {
                sum += t->lines[i];
                last = i;
                if (sum > 9) {
                    break;
                }
                last = 0;
            }
            if (t->scroll < last) {
                t->scroll++;
            }
        }
    }
}
#endif

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
void dmgr_res_slide(void)
{
    if (dmgr_res_bg_done() != 0) {
        dmgr_res_bg_anim(0);
    }
    if (dmgr_res->timer++ > 0xF) {
        dmgr_res_bg_anim(1);
        dmgr_res->bar = dmgr_res->tab;
        dmgr_res->state = 2;
        dmgr_res->timer = 0;
    }
}
#endif

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
void dmgr_res_measure(void)
{
    DmgrRes *r;
    int lh;
    int i;

    font_set_size(0xE, 0xE);
    lh = font_line_height();
    r = dmgr_res;
    for (i = 0; i < r->tabs[1].count; i++) {
        r->tabs[1].lines[i] = (font_wtext_lines(0x5D80, table_get_entry(dmgr_res_file(7),
                                   ((DmgrResItem *)dmgr_res_bonus_text)[r->tabs[1].ids[i]].text)) + 0x40) / lh;
    }
    r = dmgr_res;
    for (i = 0; i < r->tabs[2].count; i++) {
        r->tabs[2].lines[i] = (font_wtext_lines(0x5080, table_get_entry(dmgr_res_file(7),
                                   ((DmgrResItem *)dmgr_res_rec_text)[r->tabs[2].ids[i]].text)) + 0x40) / lh;
    }
    r = dmgr_res;
    for (i = 0; i < r->tabs[3].count; i++) {
        r->tabs[3].lines[i] = (font_wtext_lines(0x5F80, table_get_entry(dmgr_res_file(7),
                                   ((DmgrResItem *)dmgr_res_title_text)[r->tabs[3].ids[i]].text)) + 0x40) / lh;
    }
}
#endif

#ifdef NON_MATCHING /* função grande: alocação de registradores e ordem dos blocos diferentes */
void dmgr_res_menu_open(short kind)
{
    DmgrRes *r = dmgr_res;
    MenuWidget *w = &r->menu;
    MenuItemDef *d = &r->menu.def;

    memset(w, 0, sizeof *w);
    d->visible = 1;
    d->style = 0;
    d->max_w = 0xF0;
    d->auto_size = 1;
    d->color = 0xFFFFFFFF;
    d->font_flag4 = 1;
    d->pad_l = 10;
    d->pad_t = 8;
    d->pad_r = 10;
    d->pad_b = 0;
    d->font_size = 0xC;
    d->text_color = 0xFF000000;
    switch (kind) {
    case 3:
        d->text = table_get_entry(dmgr_res_file(7), 0x104);
        r->menu.buttons = 1;
        r->menu.initial = 1;
        r->menu.cancel_resets = 1;
        break;
    case 2:
        d->text = table_get_entry(dmgr_res_file(7), 0x103);
        d->pad_b = 8;
        break;
    case 1:
        r->menu.buttons = 3;
        r->menu.cancel_blocked = 1;
        switch (((DmgrGameHdr *)game_state_get())->rematch) {
        case 2:
            d->text = table_get_entry(dmgr_res_file(7), 0x102);
            r->menu.button = 3;
            break;
        case 1:
            d->text = table_get_entry(dmgr_res_file(7), 0x101);
            r->menu.button = 2;
            break;
        }
        break;
    }
    d->open_anim = 2;
    d->close_anim = 2;
    r->menu.heap = dmgr_res->heap;
    r->menu.no_cancel = 1;
    menu_open(w);
    d->x = (0x1E0 - d->w) / 2;
    d->y = (0x110 - d->h) / 2;
    dmgr_res->menu_kind = kind;
}
#endif

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
void dmgr_res_send(unsigned char a, unsigned char b)
{
    unsigned char rec[0x100];

    memset(rec, 0, sizeof rec);
    rec[0] = 8;
    rec[1] = a;
    rec[2] = b;
    local_record_set(rec);
}
#endif

char dmgr_res_lang[8] = "jegfis";
char dmgr_res_path[28] = "duel/result/result_%c.ehp";

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
void *dmgr_res_load(void)
{
    char name[0x100];
    int size;

    if (dmgr_res->pack == 0) {
        if (!dmgr_res->loading) {
            dmgr_res->loading = 1;
            sprintf(name, dmgr_res_path, dmgr_res_lang[cfg_get_language()]);
            size = ehfile_get_size(3, name);
            dmgr_res->pack = heap_alloc(dmgr_res->heap, size);
            ehfile_read_async_alias(3, name, dmgr_res->pack, size, dmgr_res_load_done, dmgr_res->pack);
        }
        return 0;
    }
    return dmgr_res->pack;
}
#endif

int dmgr_res_load_done(void *pack, int r)
{
    int ok = r > 0;

    if (ok) {
        ehp_mount(0, pack);
    }
    dmgr_res->loading = 0;
    return ok;
}

void *dmgr_res_file(unsigned short i)
{
    if (dmgr_res->files[i] == 0) {
        dmgr_res->files[i] = ehp_find(dmgr_res->pack, dmgr_res_files[i], 0);
    }
    return dmgr_res->files[i];
}

unsigned int *dmgr_res_draw_bg(unsigned int *list)
{
    int opts[0x18];
    void *e;

    e = dmgr_res_file(0);
    if (e == 0) {
        return list;
    }
    memset(opts, 0, sizeof opts);
    opts[0] = *(int *)gfx_get_state();
    list = gfx_begin_2d(list);
    list = ehf_draw(list, e, opts);
    return gfx_end_2d(list);
}

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
unsigned int *dmgr_res_draw_bar(unsigned int *list)
{
    int opts[0x18];
    int edge = 0;
    void *e;
    int f;
    int v;
    int i;
    int x;
    int u;

    e = dmgr_res_file(1);
    if (e == 0) {
        return list;
    }
    memset(opts, 0, sizeof opts);
    opts[0] = *(int *)gfx_get_state();
    list = gfx_begin_2d(list);
    list = ehf_draw(list, e, opts);
    list = gfx_end_2d(list);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    f = ehf_get_frame_count(e);
    if (f > 5 && f < 0x62) {
        if (f == 6 || f == 0x61) {
            edge = 1;
        }
        e = dmgr_res_file(6);
        if (e != 0) {
            list = gim_set_texture(list, e);
            list = dl_set_tex_filter(list, 1, 1);
            if (edge) {
                list = dl_ambient_color(list, 0x80FFFFFF);
                list = draw_set_offsets(list, 0x40, 0x15, 0x40, 0xB);
                v = 0xD;
            } else {
                list = dl_ambient_color(list, 0xFFFFFFFF);
                list = draw_set_offsets(list, 0x40, 0x15, 0x40, 0x15);
                v = 8;
            }
            for (i = 0, u = 0, x = 0x70; i < 4; i++, u += 0x40, x += 0x40) {
                list = draw_sprite(list, x, v, 0, u, dmgr_res->bar * 0x15, 0, 0);
            }
        }
    }
    return list;
}
#endif

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
unsigned int *dmgr_res_draw_result(unsigned int *list)
{
    int opts[0x18];
    void *e;

    if (dmgr_res->result_ehf >= 0 && (e = dmgr_res_file(dmgr_res->result_ehf)) != 0) {
        memset(opts, 0, sizeof opts);
        opts[0] = *(int *)gfx_get_state();
        list = gfx_begin_2d(list);
        list = ehf_draw(list, e, opts);
        list = gfx_end_2d(list);
        list = dl_enable(list, 4);
        list = dl_tex_func(list, 0, 1);
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
    }
    return list;
}
#endif

unsigned int *(*dmgr_res_pages[4])(unsigned int *list, int x) = {
    dmgr_res_page_summary, dmgr_res_page_bonus, dmgr_res_page_records, dmgr_res_page_titles,
};
unsigned short dmgr_res_tab_text[4] = { 0xF6, 0xF7, 0xF8, 0xF9 };

#ifdef NON_MATCHING /* função grande: alocação de registradores e ordem dos blocos diferentes */
unsigned int *dmgr_res_draw_footer(unsigned int *list)
{
    void *left = 0;
    void *right = 0;
    void *mid;
    void *g;
    unsigned int k;
    int w;
    int tw;
    int i;
    int j;

    list = dl_disable(list, 9);
    list = dl_tex_func(list, 4, 0);
    list = dl_ambient_color(list, 0xA0000000);
    list = draw_set_offsets(list, 0, 0, 0x1E0, 0x11);
    list = draw_sprite(list, 0, 0xFF, 0, 0, 0, 0, 0);
    list = dl_enable(list, 9);
    list = dl_tex_func(list, 0, 1);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    switch (dmgr_res->state) {
    case 0:
    case 4:
        return list;
    case 1:
    case 2:
    case 3:
    case 5:
        for (i = 1, j = -1;; j = -i) {
            k = (dmgr_res->tab + 4 + j) % 4;
            if (k == 0) {
                left = table_get_entry(dmgr_res_file(7), 0xF6);
                break;
            }
            if (dmgr_res->total[k - 1] > 0) {
                left = table_get_entry(dmgr_res_file(7), dmgr_res_tab_text[k]);
                break;
            }
            if (++i > 3) {
                break;
            }
        }
        k = dmgr_res->tab;
        for (i = 1; i < 4; i++) {
            k++;
            if (k % 4 == 0) {
                right = table_get_entry(dmgr_res_file(7), 0xF6);
                break;
            }
            if (dmgr_res->total[k % 4 - 1] > 0) {
                right = table_get_entry(dmgr_res_file(7), dmgr_res_tab_text[k % 4]);
                break;
            }
        }
        mid = table_get_entry(dmgr_res_file(7), 0xFA);
        w = 0x78;
        break;
    case 6:
    case 7:
    case 8:
    case 9:
        if (dmgr_res_series() == 0) {
            mid = table_get_entry(dmgr_res_file(7), 0xFB);
        } else {
            mid = table_get_entry(dmgr_res_file(7), 0xFA);
        }
        w = 0x1E0;
        break;
    default:
        return list;
    }
    font_set_size(0xE, 0xE);
    font_set_color(0xFFFFFFFF);
    font_text_set(list);
    if (left != 0) {
        font_draw_wtext(0x400, 0x4040, ((0x1E0 - w) / 2 - 0x10) * 0x40, left);
    }
    if (mid != 0) {
        tw = (font_print(mid) >> 6) + 1;
        font_draw_wtext((0xF0 - tw / 2) * 0x40, 0x4040, w << 6, mid);
    }
    if (right != 0) {
        w = ((0x1E0 - w) / 2 - 0x10) * 0x40;
        font_set_size(font_measure_wtext(w, right), 0xE);
        font_draw_wtext((0x1D0 - ((font_print(right) >> 6) + 1)) * 0x40, 0x4040, w, right);
        font_set_size(0xE, 0xE);
    }
    list = font_text_clear();
    if ((left != 0 || right != 0) && (g = dmgr_res_file(5)) != 0) {
        list = gim_set_texture(list, g);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(list, 9, 0xD, 9, 0xD);
        if (left != 0) {
            list = draw_sprite(list, 2, 0x101, 0, 0xB8, 0x33, 0, 0);
        }
        if (right != 0) {
            list = draw_sprite(list, 0x1D3, 0x101, 0, 0xC1, 0x33, 0, 0);
        }
    }
    return list;
}
#endif

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
unsigned int *dmgr_res_draw_page(unsigned int *list)
{
    int x;

    switch (dmgr_res->state) {
    case 0:
        break;
    case 1:
    case 2:
    case 3:
    case 5:
        list = dmgr_res_pages[dmgr_res->tab](list, 0);
        break;
    case 4:
        x = dmgr_res->timer * 0x1E0 >> 4;
        switch (dmgr_res->slide) {
        case 1:
            list = dmgr_res_pages[dmgr_res->prev_tab](list, -x);
            list = dmgr_res_pages[dmgr_res->tab](list, 0x1E0 - x);
            break;
        case 0:
            list = dmgr_res_pages[dmgr_res->tab](list, x - 0x1E0);
            list = dmgr_res_pages[dmgr_res->prev_tab](list, x);
            break;
        }
        break;
    case 6:
    case 7:
    case 8:
    case 9:
        list = dmgr_res_draw_series(list);
        break;
    }
    return list;
}
#endif

short dmgr_res_bob[4] = { 0, 1, 2, 1 };

#ifdef NON_MATCHING /* função grande: alocação de registradores e ordem dos blocos diferentes */
unsigned int *dmgr_res_page_summary(unsigned int *list, int x)
{
    void *g;
    int idx[4];
    int *p;
    int sum;
    int i;
    int y;
    int l;

    g = dmgr_res_file(5);
    if (g == 0) {
        return list;
    }
    list = gim_set_texture(list, g);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    l = x + 0x3E;
    list = draw_set_offsets(list, 10, 0x14, 10, 0x14);
    list = draw_sprite(list, l, 0x4A, 0, 0, 0, 0, 0);
    list = draw_sprite(list, x + 0x199, 0x4A, 0, 0xD, 0, 0, 0);
    list = draw_set_offsets(list, 1, 0x14, 0x106, 0x14);
    list = draw_sprite(list, x + 0x48, 0x4A, 0, 10, 0, 0, 0);
    list = draw_set_offsets(list, 3, 0x14, 3, 0x14);
    list = draw_sprite(list, x + 0x14E, 0x4A, 0, 0xF, 0, 0, 0);
    list = draw_sprite(list, x + 0x151, 0x4A, 0, 0x11, 0, 1, 0);
    list = draw_set_offsets(list, 1, 0x14, 0x45, 0x14);
    list = draw_sprite(list, x + 0x154, 0x4A, 0, 10, 0, 0, 0);
    list = draw_set_offsets(list, 10, 0x1B, 10, 0x1B);
    list = draw_sprite(list, l, 0xA9, 0, 0, 0x14, 0, 0);
    list = draw_sprite(list, x + 0x199, 0xA9, 0, 0xD, 0x14, 0, 0);
    list = draw_set_offsets(list, 1, 0x1B, 0x151, 0x1B);
    list = draw_sprite(list, x + 0x48, 0xA9, 0, 10, 0x14, 0, 0);
    list = draw_set_offsets(list, 6, 1, 6, 0x4B);
    list = draw_sprite(list, l, 0x5E, 0, 0, 0x14, 0, 0);
    list = draw_sprite(list, x + 0x19D, 0x5E, 0, 0x11, 0x14, 0, 0);
    list = draw_set_offsets(list, 2, 2, 2, 2);
    list = draw_sprite(list, x + 0x42, 0x5D, 0, 0x17, 0x10, 0, 0);
    list = draw_sprite(list, x + 0x19C, 0x5D, 0, 0x21, 0x10, 0, 0);
    list = draw_sprite(list, x + 0x42, 0xA8, 0, 0x17, 0x1A, 0, 0);
    list = draw_sprite(list, x + 0x19C, 0xA8, 0, 0x21, 0x1A, 0, 0);
    list = draw_set_offsets(list, 1, 2, 0x158, 2);
    list = draw_sprite(list, x + 0x44, 0x5D, 0, 0x19, 0x10, 0, 0);
    list = draw_sprite(list, x + 0x44, 0xA8, 0, 0x19, 0x10, 0, 0);
    list = draw_set_offsets(list, 2, 1, 2, 0x49);
    list = draw_sprite(list, x + 0x42, 0x5F, 0, 0x17, 0x12, 0, 0);
    list = draw_sprite(list, x + 0x19C, 0x5F, 0, 0x17, 0x12, 0, 0);
    list = draw_set_offsets(list, 1, 1, 0x158, 0x49);
    list = draw_sprite(list, x + 0x44, 0x5F, 0, 0x19, 0x12, 0, 0);
    if (x == 0) {
        short bob[4] = { 0, 1, 2, 1 };

        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(list, 0xF, 0x17, 0xF, 0x17);
        list = draw_sprite(list, bob[*(unsigned int *)gfx_get_state() >> 2 & 2] + 0x18, 0x7B, 0, 0x90, 0x18, 0, 0);
        list = draw_sprite(list, 0x1BB - bob[*(unsigned int *)gfx_get_state() >> 2 & 2], 0x7B, 0, 0x9F, 0x18, 0, 0);
    }
    sum = 0;
    memset(idx, -1, sizeof idx);
    for (i = 0, p = idx; i < 4; i++) {
        if (dmgr_res->total[i] != 0) {
            *p++ = i;
            sum += dmgr_res->total[i];
        }
    }
    for (i = 0, p = idx, y = 0x5F; i < 4; i++, p++, y += 0x13) {
        list = dmgr_res_summary_row(list, x + 0x44, y, *p);
    }
    list = gim_set_texture(list, g);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 1, 0x10, 1, 0x10);
    list = draw_sprite(list, x + 0x153, 0xB0, 0, 0x1A, 0, 0, 0);
    list = draw_sprite(list, x + 0x19B, 0xB0, 0, 0x1C, 0, 0, 0);
    list = draw_set_offsets(list, 1, 0x10, 0x47, 0x10);
    list = draw_sprite(list, x + 0x154, 0xB0, 0, 0x1B, 0, 0, 0);
    font_text_set(list);
    font_set_size(0xE, 0xE);
    font_set_color(0xFFFFFFFF);
    font_draw_wtext((x + 0x47) * 0x40, 0x1340, 0x41C0, table_get_entry(dmgr_res_file(7), 2));
    font_draw_wtext((x + 0x155) * 0x40, 0x1340, 0x11C0, table_get_entry(dmgr_res_file(7), 3));
    font_draw_wtext((x + 0x44) * 0x40, 0x2C40, 0x41C0, table_get_entry(dmgr_res_file(7), 8));
    font_set_color(0xFF00FFFF);
    font_set_size(0xC, 0xC);
    font_draw_text((x + 0x189) * 0x40, 0x2D00, (0x1E0 - (x + 0x189)) * 0x40, dmgr_res_pt);
    list = font_text_clear();
    font_set_size(0xE, 0xF);
    font_set_color(0xFFFFFFFF);
    return dmgr_res_draw_number(list, (short)(x + 0x188), 0xB1, sum, 10, 5, 0);
}
#endif

unsigned short dmgr_res_page_text[4] = { 4, 5, 6, 7 };

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
unsigned int *dmgr_res_summary_row(unsigned int *list, int x, int y, int page)
{
    void *g;

    g = dmgr_res_file(5);
    if (g == 0) {
        return list;
    }
    list = gim_set_texture(list, g);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 1, 0x10, 1, 0x10);
    list = draw_sprite(list, x, y, 0, 0x17, 0, 0, 0);
    list = draw_sprite(list, x + 0x10C, y, 0, 0x19, 0, 0, 0);
    list = draw_set_offsets(list, 1, 0x10, 0x10B, 0x10);
    list = draw_sprite(list, x + 1, y, 0, 0x18, 0, 0, 0);
    list = draw_set_offsets(list, 1, 0x10, 1, 0x10);
    list = draw_sprite(list, x + 0x10F, y, 0, 0x1A, 0, 0, 0);
    list = draw_sprite(list, x + 0x157, y, 0, 0x1C, 0, 0, 0);
    list = draw_set_offsets(list, 1, 0x10, 0x47, 0x10);
    list = draw_sprite(list, x + 0x110, y, 0, 0x1B, 0, 0, 0);
    if (page >= 0 && page < 4) {
        font_text_set(list);
        font_set_size(0xE, 0xE);
        font_set_color(0xFF000000);
        font_draw_text((x + 1) * 0x40, (y + 2) * 0x40, (0x1E0 - (x + 1)) * 0x40,
                       table_get_entry(dmgr_res_file(7), dmgr_res_page_text[page]));
        font_set_color(0xFF00FFFF);
        font_set_size(0xC, 0xC);
        font_draw_text((x + 0x145) * 0x40, (y + 4) * 0x40, (0x1E0 - (x + 0x145)) * 0x40, dmgr_res_pt);
        list = font_text_clear();
        font_set_size(0xE, 0xF);
        font_set_color(0xFFFFFFFF);
        list = dmgr_res_draw_number(list, (short)(x + 0x144), (short)(y + 1), dmgr_res->total[page], 10, 5, 0);
    }
    return list;
}
#endif

/* the scrolling list of a page between x + 0x17 and the right edge (written out in each page) */
#define DMGR_RES_PAGE_LIST(list, x, t, row)                                                                \
    do {                                                                                                  \
        int l_ = (x) + 0x17;                                                                              \
        int w_;                                                                                           \
        int i_;                                                                                           \
        int n_;                                                                                           \
        int k_;                                                                                           \
        if (l_ < 0) {                                                                                     \
            l_ = 0;                                                                                       \
        }                                                                                                 \
        if (l_ > 0x1E0) {                                                                                 \
            l_ = 0x1E0;                                                                                   \
        }                                                                                                 \
        w_ = 0x1AA;                                                                                       \
        if (0x1E0 - l_ < 0x1AA) {                                                                         \
            w_ = 0x1E0 - l_;                                                                              \
        }                                                                                                 \
        if (l_ > 0 && l_ < 0x1E0) {                                                                       \
            list = dl_scissor(list, l_, 0x3D, w_, 0xA0);                                                  \
            list = dl_enable(list, 2);                                                                    \
        }                                                                                                 \
        for (i_ = 0, n_ = 0; n_ < 10; i_++) {                                                             \
            k_ = (t)->scroll + i_;                                                                        \
            if (k_ >= (t)->count) {                                                                       \
                k_ = -1;                                                                                  \
            }                                                                                             \
            list = row(list, (x) + 0x17, n_ * 0x10 + 0x3D, k_);                                           \
            if (k_ < 0) {                                                                                 \
                n_++;                                                                                     \
            } else {                                                                                      \
                n_ += (t)->lines[k_];                                                                     \
            }                                                                                             \
        }                                                                                                 \
    } while (0)

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
unsigned int *dmgr_res_page_bonus(unsigned int *list, int x)
{
    DmgrResTab *t = &dmgr_res->tabs[1];

    list = dmgr_res_page_frame(list, x, 0x48, 0);
    list = dmgr_res_draw_box(list, x + 0x1A, 0x2B, 0x177, 0xE, 0x50, 3);
    list = dmgr_res_draw_box(list, x + 0x194, 0x2B, 0x2D, 0xE, 0x65, 0);
    font_set_size(0xE, 0xE);
    font_set_color(0xFF000000);
    font_text_set(list);
    font_draw_text((x + 0x1A0) * 0x40, 0xB00, (0x1E0 - (x + 0x1A0)) * 0x40, dmgr_res_pt);
    list = font_text_clear();
    list = dmgr_res_page_header(list, x, 0);
    DMGR_RES_PAGE_LIST(list, x, t, dmgr_res_bonus_row);
    return dl_disable(list, 2);
}
#endif

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
unsigned int *dmgr_res_bonus_row(unsigned int *list, int x, int y, int i)
{
    DmgrRes *r = dmgr_res;
    void *g;
    int h;
    int id;
    int n;

    g = dmgr_res_file(5);
    if (g == 0) {
        return list;
    }
    list = gim_set_texture(list, g);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    if (i < 0) {
        h = 0xE;
    } else {
        h = r->tabs[1].lines[i] * 0x10 - 2;
    }
    id = r->tabs[1].ids[i];
    list = dmgr_res_draw_box(list, x, y, 0x17A, h, 0x5F, 0);
    list = dmgr_res_draw_box(list, x + 0x17C, y, 0x2E, h, 0x62, 0);
    if (i >= 0) {
        font_set_line_extra(0x40);
        font_set_size(0xE, 0xE);
        font_text_set(list);
        font_set_color(0xFF000000);
        font_draw_wtext_ex((x + 2) * 0x40, y << 6, 0x5D80, (h + 2) * 0x40,
                           table_get_entry(dmgr_res_file(7), ((DmgrResItem *)dmgr_res_bonus_text)[id].text),
                           0xFFFFFFFF);
        y += h / 2;
        font_set_color(0xFF00FFFF);
        font_set_size(0xB, 0xB);
        font_draw_text((x + 0x198) * 0x40, (y - 4) * 0x40, (0x1E0 - (x + 0x198)) * 0x40, dmgr_res_pt);
        list = font_text_clear();
        font_set_size(0xC, 0xD);
        font_set_color(0xFFFFFFFF);
        n = dmgr_res->rec->bonus[id];
        if (n > 999) {
            n = 999;
        }
        list = dmgr_res_draw_number(list, (short)(x + 0x198), (short)(y - 6), n, 9, 3, 0);
        font_set_line_extra(0);
    }
    return list;
}
#endif

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
unsigned int *dmgr_res_page_records(unsigned int *list, int x)
{
    DmgrResTab *t = &dmgr_res->tabs[2];

    list = dmgr_res_page_frame(list, x, 0x24, 0);
    list = dmgr_res_draw_box(list, x + 0x1A, 0x2B, 0x143, 0xE, 0x2C, 3);
    list = dmgr_res_draw_box(list, x + 0x15F, 0x2B, 0x3A, 0xE, 0x3E, 0);
    list = dmgr_res_draw_box(list, x + 0x19B, 0x2B, 0x26, 0xE, 0x41, 0);
    font_set_color(0xFFFFFFFF);
    font_text_set(list);
    dmgr_res_text_fit(x + 0x160, 0x2B, 0x38, 0xE, 0xE, table_get_entry(dmgr_res_file(7), 0xF2));
    font_set_size(0xE, 0xE);
    font_set_color(0xFF000000);
    font_draw_text((x + 0x1A4) * 0x40, 0xB00, (0x1E0 - (x + 0x1A4)) * 0x40, dmgr_res_pt);
    list = font_text_clear();
    list = dmgr_res_page_header(list, x, 1);
    DMGR_RES_PAGE_LIST(list, x, t, dmgr_res_record_row);
    return dl_disable(list, 2);
}
#endif

unsigned short dmgr_res_colon[2] = { ':', 0 };

#ifdef NON_MATCHING /* função grande: alocação de registradores e ordem dos blocos diferentes */
unsigned int *dmgr_res_record_row(unsigned int *list, int x, int y, int i)
{
    DmgrRes *r = dmgr_res;
    unsigned short *colon = dmgr_res_colon;
    void *g;
    int h;
    int id;
    int n;

    g = dmgr_res_file(5);
    if (g == 0) {
        return list;
    }
    list = gim_set_texture(list, g);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    if (i < 0) {
        h = 0xE;
    } else {
        h = r->tabs[2].lines[i] * 0x10 - 2;
    }
    id = r->tabs[2].ids[i];
    list = dmgr_res_draw_box(list, x, y, 0x146, h, 0x3B, 0);
    list = dmgr_res_draw_box(list, x + 0x148, y, 0x3A, h, 0x3E, 0);
    list = draw_set_offsets(list, 0xC, 0xC, 0xC, 0xC);
    list = draw_sprite(list, x + 0x14A, y + 1, 0, 0x3B, 0x1D, 0, 0);
    list = dmgr_res_draw_box(list, x + 0x184, y, 0x26, h, 0x3E, 0);
    if (i >= 0) {
        font_set_line_extra(0x40);
        font_set_size(0xE, 0xE);
        font_text_set(list);
        font_set_color(0xFF000000);
        font_draw_wtext_ex((x + 2) * 0x40, y << 6, 0x5080, (h + 2) * 0x40,
                           table_get_entry(dmgr_res_file(7), ((DmgrResItem *)dmgr_res_rec_text)[id].text),
                           0xFFFFFFFF);
        y += h / 2;
        font_set_color(0xFF00FFFF);
        font_set_size(0xB, 0xB);
        font_draw_text((x + 0x198) * 0x40, (y - 4) * 0x40, (0x1E0 - (x + 0x198)) * 0x40, dmgr_res_pt);
        list = font_text_clear();
        font_set_size(0xC, 0xD);
        font_set_color(0xFFFFFFFF);
        list = dmgr_res_draw_number(list, (short)(x + 0x198), (short)(y - 6), 20, 9, 3, 0);
        font_set_line_extra(0);
        font_set_color(0xFFFFFFFF);
        n = dmgr_res->rec->rec[id];
        if (n > 999999999) {
            n = 999999999;
        }
        if (id == 0x25) {
            font_set_size(0xB, 0xD);
            list = dmgr_res_draw_number(list, (short)(x + 0x180), (short)(y - 6), n % 60, 8, 2, 1);
            font_text_set(list);
            font_draw_text((x + 0x16B) * 0x40, (y - 7) * 0x40, (0x1E0 - (x + 0x16B)) * 0x40, colon);
            list = font_text_clear();
            list = dmgr_res_draw_number(list, (short)(x + 0x168), (short)(y - 6), n / 60, 8, 2, 0);
        } else if (n < 100000) {
            font_set_size(0xB, 0xD);
            list = dmgr_res_draw_number(list, (short)(x + 0x180), (short)(y - 6), n, 8, 5, 0);
        } else {
            font_set_size(6, 0xD);
            list = dmgr_res_draw_number(list, (short)(x + 0x182), (short)(y - 6), n, 5, 9, 0);
        }
    }
    return list;
}
#endif

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
unsigned int *dmgr_res_page_titles(unsigned int *list, int x)
{
    DmgrResTab *t = &dmgr_res->tabs[3];

    list = dmgr_res_page_frame(list, x, 0x6C, 0);
    list = dmgr_res_draw_box(list, x + 0x1A, 0x2B, 0x17F, 0xE, 0x74, 3);
    list = dmgr_res_draw_box(list, x + 0x19B, 0x2B, 0x26, 0xE, 0x89, 0);
    font_set_size(0xE, 0xE);
    font_set_color(0xFF000000);
    font_text_set(list);
    font_draw_text((x + 0x1A4) * 0x40, 0xB00, (0x1E0 - (x + 0x1A4)) * 0x40, dmgr_res_pt);
    list = font_text_clear();
    list = dmgr_res_page_header(list, x, 2);
    DMGR_RES_PAGE_LIST(list, x, t, dmgr_res_title_row);
    return dl_disable(list, 2);
}
#endif

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
unsigned int *dmgr_res_title_row(unsigned int *list, int x, int y, int i)
{
    DmgrRes *r = dmgr_res;
    void *g;
    int h;
    unsigned short id;

    g = dmgr_res_file(5);
    if (g == 0) {
        return list;
    }
    list = gim_set_texture(list, g);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    if (i < 0) {
        h = 0xE;
    } else {
        h = r->tabs[3].lines[i] * 0x10 - 2;
    }
    id = r->tabs[3].ids[i];
    list = dmgr_res_draw_box(list, x, y, 0x182, h, 0x83, 0);
    list = dmgr_res_draw_box(list, x + 0x184, y, 0x26, h, 0x86, 0);
    if (i >= 0) {
        font_set_line_extra(0x40);
        font_set_size(0xE, 0xE);
        font_text_set(list);
        font_set_color(0xFF000000);
        font_draw_wtext_ex((x + 2) * 0x40, y << 6, 0x5F80, (h + 2) * 0x40,
                           table_get_entry(dmgr_res_file(7), ((DmgrResItem *)dmgr_res_title_text)[id].text),
                           0xFFFFFFFF);
        y += h / 2;
        font_set_color(0xFF00FFFF);
        font_set_size(0xB, 0xB);
        font_draw_text((x + 0x198) * 0x40, (y - 4) * 0x40, (0x1E0 - (x + 0x198)) * 0x40, dmgr_res_pt);
        list = font_text_clear();
        font_set_size(0xC, 0xD);
        font_set_color(0xFFFFFFFF);
        list = dmgr_res_draw_number(list, (short)(x + 0x198), (short)(y - 6), 30, 9, 2, 0);
        font_set_line_extra(0);
    }
    return list;
}
#endif

#ifdef NON_MATCHING /* função grande: alocação de registradores e ordem dos blocos diferentes */
unsigned int *dmgr_res_draw_series(unsigned int *list)
{
    void *g;
    int side = 0;
    unsigned int other;
    short r;
    short mine;
    int i;
    int x;

    g = dmgr_res_file(5);
    if (g == 0) {
        return list;
    }
    list = gim_set_texture(list, g);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 10, 0x14, 10, 0x14);
    list = draw_sprite(list, 0x52, 0x4A, 0, 0, 0, 0, 0);
    list = draw_sprite(list, 0x185, 0x4A, 0, 0xD, 0, 0, 0);
    list = draw_set_offsets(list, 1, 0x14, 0x129, 0x14);
    list = draw_sprite(list, 0x5C, 0x4A, 0, 10, 0, 0, 0);
    list = draw_set_offsets(list, 10, 0x12, 10, 0x12);
    list = draw_sprite(list, 0x52, 0xA9, 0, 0, 0x1D, 0, 0);
    list = draw_sprite(list, 0x185, 0xA9, 0, 0xD, 0x1D, 0, 0);
    list = draw_set_offsets(list, 1, 0x12, 0x129, 0x12);
    list = draw_sprite(list, 0x5C, 0xA9, 0, 10, 0x1D, 0, 0);
    list = draw_set_offsets(list, 4, 1, 4, 0x4B);
    list = draw_sprite(list, 0x52, 0x5E, 0, 0, 0x14, 0, 0);
    list = draw_sprite(list, 0x18B, 0x5E, 0, 0x13, 0x14, 0, 0);
    list = draw_set_offsets(list, 1, 1, 0x135, 0x27);
    list = draw_sprite(list, 0x56, 0x5E, 0, 10, 0x14, 0, 0);
    list = draw_set_offsets(list, 1, 1, 0x135, 0x1C);
    list = draw_sprite(list, 0x56, 0x8E, 0, 10, 0x14, 0, 0);
    list = draw_set_offsets(list, 3, 9, 3, 9);
    list = draw_sprite(list, 0x56, 0x85, 0, 4, 0x14, 0, 0);
    list = draw_sprite(list, 0x188, 0x85, 0, 0x10, 0x14, 0, 0);
    list = draw_set_offsets(list, 1, 9, 0x12F, 9);
    list = draw_sprite(list, 0x59, 0x85, 0, 7, 0x14, 0, 0);
    font_text_set(list);
    font_set_size(0xE, 0xE);
    font_set_color(0xFFFFFFFF);
    font_draw_text(0x1700, 0x1340, 0x6100, table_get_entry(dmgr_res_file(7), 0xFD));
    font_set_size(0xE, 0xE);
    dmgr_res_text_fit(0xDC, 0x4D, 0x37, 0xE, 0xE, table_get_entry(dmgr_res_file(7), 0xFE));
    font_set_size(0xE, 0xE);
    dmgr_res_text_fit(0x117, 0x4D, 0x37, 0xE, 0xE, table_get_entry(dmgr_res_file(7), 0xFF));
    font_set_size(0xE, 0xE);
    dmgr_res_text_fit(0x152, 0x4D, 0x37, 0xE, 0xE, table_get_entry(dmgr_res_file(7), 0x100));
    list = font_text_clear();
    if (sysvar_get_a98() != 0 && netsync_local_side() != 0) {
        side = 1;
    }
    if (((DmgrGameHdr *)game_state_get())->partner_mode == 2) {
        other = (side + 1) & 1;
        list = dmgr_res_series_name(list, 0x57, 0x5F, 1, ((DmgrGameHdr *)game_state_get())->slot[other].name);
        list = dmgr_res_series_name(list, 0x57, 0x73, 1, ((DmgrGameHdr *)game_state_get())->slot[other + 2].name);
        list = dmgr_res_series_name(list, 0x57, 0x8E, 0, ((DmgrGameHdr *)game_state_get())->slot[side].name);
        list = dmgr_res_series_name(list, 0x57, 0xA2, 0, ((DmgrGameHdr *)game_state_get())->slot[side + 2].name);
    } else {
        list = dmgr_res_series_name(list, 0x57, 0x69, 1,
                                    ((DmgrGameHdr *)game_state_get())->slot[(side + 1) & 1].name);
        list = dmgr_res_series_name(list, 0x57, 0x98, 0, ((DmgrGameHdr *)game_state_get())->slot[side].name);
    }
    for (i = 0, x = 0xDC; i < 3; i++, x += 0x3B) {
        r = ((DuelResults *)game_state2_get())->duels[i];
        if (r == 2) {
            mine = 1;
        } else if (r == 1) {
            mine = 2;
        } else {
            mine = r;
        }
        list = dmgr_res_series_mark(list, x, 0x5F, mine);
        list = dmgr_res_series_mark(list, x, 0x8E, r);
    }
    return list;
}
#endif

#ifdef NON_MATCHING /* função grande: alocação de registradores e ordem dos blocos diferentes */
unsigned int *dmgr_res_page_frame(unsigned int *list, int x, int u, int v)
{
    void *g;
    int l;

    g = dmgr_res_file(5);
    if (g == 0) {
        return list;
    }
    list = gim_set_texture(list, g);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    l = x + 0x11;
    list = draw_set_offsets(list, 9, 0x14, 9, 0x14);
    list = draw_sprite(list, l, 0x28, 0, u, v, 0, 0);
    list = draw_set_offsets(list, 0xD, 0x14, 0xD, 0x14);
    list = draw_sprite(list, x + 0x1C3, 0x28, 0, u + 10, v, 0, 0);
    list = draw_set_offsets(list, 1, 0x14, 0x1AB, 0x14);
    list = draw_sprite(list, x + 0x1A, 0x28, 0, u + 0xB, v, 0, 0);
    list = draw_set_offsets(list, 7, 0x1A, 7, 0x1A);
    list = draw_sprite(list, l, 0xDD, 0, u, v + 0x15, 0, 0);
    list = draw_set_offsets(list, 0xE, 0x1A, 0xE, 0x1A);
    list = draw_sprite(list, x + 0x1C2, 0xDD, 0, u + 9, v + 0x15, 0, 0);
    list = draw_set_offsets(list, 1, 0x1A, 0x1AA, 0x1A);
    list = draw_sprite(list, x + 0x18, 0xDD, 0, u + 7, v + 0x15, 0, 0);
    list = draw_set_offsets(list, 7, 1, 7, 0xA1);
    list = draw_sprite(list, l, 0x3C, 0, u, v + 0x14, 0, 0);
    list = draw_set_offsets(list, 0xD, 1, 0xD, 0xA1);
    list = draw_sprite(list, x + 0x1C3, 0x3C, 0, u + 10, v + 0x14, 0, 0);
    list = draw_set_offsets(list, 2, 2, 2, 2);
    list = draw_sprite(list, x + 0x15, 0x3B, 0, u + 0x17, v + 0x11, 0, 0);
    list = draw_sprite(list, x + 0x15, 0xDB, 0, u + 0x17, v + 0x1B, 0, 0);
    list = draw_sprite(list, x + 0x1C1, 0x3B, 0, u + 8, v + 0x1E, 0, 0);
    list = draw_sprite(list, x + 0x1C1, 0x3B, 0, u + 0x21, v + 0x11, 0, 0);
    list = draw_sprite(list, x + 0x1C1, 0xDB, 0, u + 8, v + 0x1E, 0, 0);
    list = draw_sprite(list, x + 0x1C1, 0xDB, 0, u + 0x21, v + 0x1B, 0, 0);
    list = draw_set_offsets(list, 1, 2, 0x1AA, 2);
    list = draw_sprite(list, x + 0x17, 0x3B, 0, u + 0x19, v + 0x11, 0, 0);
    list = draw_sprite(list, x + 0x17, 0xDB, 0, u + 0x19, v + 0x11, 0, 0);
    list = draw_set_offsets(list, 2, 1, 2, 0x9E);
    list = draw_sprite(list, x + 0x15, 0x3D, 0, u + 0x17, v + 0x13, 0, 0);
    list = draw_sprite(list, x + 0x1C1, 0x3D, 0, u + 0x17, v + 0x13, 0, 0);
    list = draw_set_offsets(list, 1, 1, 0x1AA, 0x9E);
    list = draw_sprite(list, x + 0x17, 0x3D, 0, u + 0x19, v + 0x13, 0, 0);
    return dmgr_res_draw_box(list, x + 0x18A, 0xE4, 0x37, 0xE, u + 0x1A, v);
}
#endif

unsigned short dmgr_res_head_text[3] = { 0xF3, 0xF4, 0xF5 };
short dmgr_res_bob2[4] = { 0, 1, 2, 1 };

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
unsigned int *dmgr_res_page_header(unsigned int *list, int x, int page)
{
    DmgrResTab *t = &dmgr_res->tabs[page + 1];
    int tx;
    int tw;

    font_set_size(0xE, 0xE);
    font_text_set(list);
    font_set_color(0xFFFFFFFF);
    tw = (0x1E0 - (x + 0x1A)) * 0x40;
    tx = (x + 0x1A) * 0x40;
    font_draw_text(tx, 0xAC0, tw, table_get_entry(dmgr_res_file(7), 2));
    font_draw_text(tx, 0x3940, tw, table_get_entry(dmgr_res_file(7), dmgr_res_head_text[page]));
    font_set_color(0xFF00FFFF);
    font_set_size(0xB, 0xB);
    font_draw_text((x + 0x1AF) * 0x40, 0x39C0, (0x1E0 - (x + 0x1AF)) * 0x40, dmgr_res_pt);
    list = font_text_clear();
    font_set_size(0xC, 0xD);
    font_set_color(0xFFFFFFFF);
    list = dmgr_res_draw_number(list, (short)(x + 0x1AF), 0xE5, dmgr_res->total[page], 9, 5, 0);
    list = dmgr_res_draw_scrollbar(list, x + 0x1C5, 0x45, 0x8E, t->scroll, 10, t->count);
    if (x == 0) {
        short *bob = dmgr_res_bob2;

        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(list, 0xF, 0x17, 0xF, 0x17);
        list = draw_sprite(list, bob[*(unsigned int *)gfx_get_state() >> 2 & 2] + 1, 0x84, 0, 0x90, 0x18, 0, 0);
        list = draw_sprite(list, 0x1D2 - bob[*(unsigned int *)gfx_get_state() >> 2 & 2], 0x84, 0, 0x9F, 0x18, 0, 0);
    }
    return list;
}
#endif

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
unsigned int *dmgr_res_draw_box(unsigned int *list, int x, int y, int w, int h, int u, int v)
{
    int r;
    int b;

    if (h == 0xE) {
        list = draw_set_offsets(list, 2, 0xE, 2, 0xE);
        list = draw_sprite(list, x, y, 0, u, v, 0, 0);
        list = draw_sprite(list, x + w - 1, y, 0, u + 2, v, 0, 0);
        list = draw_set_offsets(list, 1, 0xE, w - 2, 0xE);
        return draw_sprite(list, x + 1, y, 0, u + 1, v, 0, 0);
    }
    list = draw_set_offsets(list, 2, 2, 2, 2);
    list = draw_sprite(list, x, y, 0, u, v, 0, 0);
    r = x + w - 2;
    list = draw_sprite(list, r, y, 0, u + 1, v, 0, 0);
    b = y + h - 2;
    list = draw_sprite(list, x, b, 0, u, v + 0xC, 0, 0);
    list = draw_sprite(list, r, b, 0, u + 1, v + 0xC, 0, 0);
    list = draw_set_offsets(list, 1, 2, w - 4, 2);
    list = draw_sprite(list, x + 2, y, 0, u + 1, v, 0, 0);
    list = draw_sprite(list, x + 2, b, 0, u + 1, v + 0xC, 0, 0);
    list = draw_set_offsets(list, 2, 1, 2, h - 4);
    list = draw_sprite(list, x, y + 2, 0, u, v + 1, 0, 0);
    list = draw_sprite(list, r, y + 2, 0, u + 1, v + 1, 0, 0);
    list = draw_set_offsets(list, 1, 1, w - 4, h - 4);
    return draw_sprite(list, x + 2, y + 2, 0, u + 1, v + 1, 0, 0);
}
#endif

unsigned short dmgr_res_digit[2] = { '0', 0 };

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
unsigned int *dmgr_res_draw_number(unsigned int *list, unsigned short x, short y, int n, unsigned short step,
                                   unsigned short digits, int pad)
{
    unsigned short ch[2];
    int neg = n < 0;
    int i;

    memcpy(ch, dmgr_res_digit, sizeof ch);
    if (neg) {
        n = -n;
    }
    font_text_set(list);
    for (i = digits - 1; i >= 0; i--) {
        x -= step;
        ch[0] = n % 10 + '0';
        font_draw_text(x << 6, y << 6, (0x1E0 - x) * 0x40, ch);
        n /= 10;
        if (pad == 0 && n == 0) {
            break;
        }
    }
    if (neg) {
        x -= (unsigned short)(step + 2);
        ch[0] = 0xFF0D;
        font_draw_text(x << 6, y << 6, (0x1E0 - x) * 0x40, ch);
    }
    return font_text_clear();
}
#endif

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
unsigned int *dmgr_res_draw_arrow(unsigned int *list, int x, int y, char down, unsigned int period, int blink,
                                  int on, unsigned int frame, void *gim)
{
    unsigned int half = period >> 1;
    int v;
    float a;

    if (down == 1) {
        v = 8;
    } else {
        v = 0;
    }
    list = gim_set_texture(list, gim);
    list = dl_set_tex_filter(list, 0, 0);
    if (on != 0) {
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = draw_set_offsets(list, 8, 8, 8, 8);
        list = draw_sprite(list, x, y, 0, 0xEE, v + 0x30, 0, 0);
    }
    if (blink != 0) {
        frame %= period;
        if (frame < half) {
            a = (float)frame / (float)half;
        } else {
            a = (float)(period - frame) / (float)half;
        }
        if (a < 0.0f) {
            a = 0.0f;
        }
        if (a > 1.0f) {
            a = 1.0f;
        }
        list = dl_ambient_color(list, (unsigned int)(a * 255.0f) << 24 | 0xFFFFFF);
        list = draw_set_offsets(list, 8, 8, 8, 8);
        list = draw_sprite(list, x, y, 0, 0xEE, v, 0, 0);
    }
    return list;
}
#endif

#ifdef NON_MATCHING /* função grande: alocação de registradores e ordem dos blocos diferentes */
unsigned int *dmgr_res_draw_scrollbar(unsigned int *list, int x, int y, int h, int pos, int rows, int count)
{
    int track = h - 2;
    int len;
    int at;
    void *g;

    g = dmgr_res_file(5);
    if (g == 0) {
        return list;
    }
    if (rows < count) {
        len = rows * track / count;
        if (len < 10) {
            len = 10;
        }
        at = pos * (track - len) / (count - rows);
        if (at < 0) {
            at = 0;
        }
        if (at > track - len) {
            at = track - len;
        }
    } else {
        at = 0;
        len = track;
    }
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = gim_set_texture(list, g);
    list = draw_set_offsets(list, 8, 5, 8, 5);
    list = draw_sprite(list, x, y, 0, 0xF7, 0, 0, 0);
    list = draw_sprite(list, x, y + h - 5, 0, 0xF7, 0xF, 0, 0);
    list = draw_set_offsets(list, 8, 5, 8, h - 10);
    list = draw_sprite(list, x, y + 5, 0, 0xF7, 5, 0, 0);
    list = draw_set_offsets(list, 4, 5, 4, 5);
    at += y;
    list = draw_sprite(list, x + 2, at + 2, 0, 0xF7, 0x28, 0, 0);
    list = draw_sprite(list, x + 2, len + at - 3, 0, 0xF7, 0x37, 0, 0);
    list = draw_set_offsets(list, 4, 5, 4, len - 10);
    list = draw_sprite(list, x + 2, at + 7, 0, 0xF7, 0x2D, 0, 0);
    list = dmgr_res_draw_arrow(list, x, y - 7, 0, ((unsigned int *)gfx_get_state())[1], pos > 0, 1,
                               *(unsigned int *)gfx_get_state(), g);
    list = dmgr_res_draw_arrow(list, x, y + h + 2, 1, ((unsigned int *)gfx_get_state())[1],
                               pos < count - rows, 1, *(unsigned int *)gfx_get_state(), g);
    return list;
}
#endif

#ifdef NON_MATCHING /* ordem das instruções e registradores diferentes do original */
void dmgr_res_text_fit(int x, int y, int w, int sw, int sh, void *text)
{
    font_set_size(sw, sh);
    font_set_size(font_measure_wtext(w << 6, text), sh);
    w -= (font_print(text) >> 6) + 1;
    x += w / 2;
    font_draw_text(x * 0x40, y << 6, (0x1E0 - x) * 0x40, text);
}
#endif

unsigned int *dmgr_res_series_name(unsigned int *list, int x, int y, int partner, void *name)
{
    void *g;
    int u;

    g = dmgr_res_file(5);
    if (g == 0) {
        return list;
    }
    list = gim_set_texture(list, g);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 3, 0x12, 3, 0x12);
    list = draw_sprite(list, x, y, 0, 0xD8, 0, 0, 0);
    list = draw_sprite(list, x + 0x7F, y, 0, 0xE1, 0, 0, 0);
    list = draw_set_offsets(list, 1, 0x12, 0x7C, 0x12);
    list = draw_sprite(list, x + 3, y, 0, 0xDB, 0, 0, 0);
    if (partner != 0) {
        u = 0xAD;
    } else {
        u = 0xA2;
    }
    list = draw_set_offsets(list, 0xB, 0x11, 0xB, 0x11);
    list = draw_sprite(list, x + 4, y + 1, 0, u, 0x2F, 0, 0);
    font_set_size(0xE, 0xE);
    font_set_color(0xFF000000);
    font_text_set(list);
    font_draw_wtext((x + 0x11) * 0x40, (y + 3) * 0x40, 0x1BC0, name);
    return font_text_clear();
}

unsigned int *dmgr_res_series_mark(unsigned int *list, int x, int y, unsigned short result)
{
    void *g;
    int u;
    int u2;

    g = dmgr_res_file(5);
    if (g == 0) {
        return list;
    }
    list = gim_set_texture(list, g);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 2, 2, 2, 2);
    list = draw_sprite(list, x, y, 0, 0x17, 0x1C, 0, 0);
    list = draw_sprite(list, x + 0x35, y, 0, 0x18, 0x1C, 0, 0);
    list = draw_sprite(list, x, y + 0x24, 0, 0x17, 0x1D, 0, 0);
    list = draw_sprite(list, x + 0x35, y + 0x24, 0, 0x18, 0x1D, 0, 0);
    list = draw_set_offsets(list, 1, 2, 0x33, 2);
    list = draw_sprite(list, x + 2, y, 0, 0x18, 0x1C, 0, 0);
    list = draw_sprite(list, x + 2, y + 0x24, 0, 0x18, 0x1D, 0, 0);
    list = draw_set_offsets(list, 2, 1, 2, 0x22);
    list = draw_sprite(list, x, y + 2, 0, 0x17, 0x1D, 0, 0);
    list = draw_sprite(list, x + 0x35, y + 2, 0, 0x18, 0x1D, 0, 0);
    list = draw_set_offsets(list, 1, 1, 0x33, 0x22);
    list = draw_sprite(list, x + 2, y + 2, 0, 0x18, 0x1D, 0, 0);
    switch (result) {
    case 1:
        u = 0x90;
        u2 = 0;
        break;
    case 2:
        u = 0xA8;
        u2 = 0x6C;
        break;
    case 3:
        u = 0xC0;
        u2 = 0x36;
        break;
    default:
        return list;
    }
    list = draw_set_offsets(list, 0x18, 0x18, 0x18, 0x18);
    list = draw_sprite(list, x + 0xF, y, 0, u, 0, 0, 0);
    list = draw_set_offsets(list, 0x36, 0x10, 0x36, 0x10);
    return draw_sprite(list, x, y + 0x16, 0, u2, 0x2F, 0, 0);
}
