// @range 0x08808A20-0x0880C8E8
// @asm 0x08808DA0-0x088095D8
// @asm 0x08809A04-0x08809BB4
// @asm 0x08809E74-0x0880A054
// @asm 0x0880A2B8-0x0880ADA8
// @asm 0x0880ADA8-0x0880B58C
// @asm 0x0880BC54-0x0880BEA4
// @asm 0x0880C2C8-0x0880C4EC
// @data 0x0881A6F8-0x0881A774
// @rodata 0x08819F6C-0x08819FC4
/* Start menu of a duel: rock-paper-scissors (janken) for the first turn, local or over ad hoc,
   then "go first / go second", the partner selection and the hand-over to the duel. */
#include "libduel_mgr.h"

/* BOOT */
void font_set_flag1(int on);
void font_set_flag2(int on);
void font_set_flag4(int on);
void font_set_flag8(int on);
int cfg_get_language(void);
int ehp_mount(void *cb, void *pack);
void *ehp_find(void *pack, const char *name, int *size);
int ehf_set_anim(void *e, int target, int limit);
void ehf_get_size(void *e, int *target, int *limit);
int ehf_get_frame_count(void *e);
int fade_is_done(void);
unsigned short *table_get_entry(void *t, unsigned int i);
int peers_all_match(int a, int b, int c);
int netsync_transfer(void);
void netsync_transfer_setup(unsigned char *dst, void *src, int size);
int netsync_side_player(int which);
int rand_next_alias(void);
void local_record_set1(unsigned char v);
unsigned char *peer_get(int i);
void bgm_play(int id);
void se_play(int id);
int sprintf(char *buf, const char *fmt, ...);
unsigned short region_pick_value(int flag);
int region_get(void);
void *gfx_get_state(void);
unsigned int *gfx_begin_2d(unsigned int *list);
unsigned int *gfx_end_2d(unsigned int *list);
unsigned int *ehf_draw(unsigned int *list, void *e, void *opts);
unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_tex_func(unsigned int *list, int func, int comp);
unsigned int *dl_blend_func(unsigned int *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
unsigned int *dl_clear_color(unsigned int *list, unsigned int color);
unsigned int *dl_clear_depth(unsigned int *list, unsigned int z);
unsigned int *dl_clear_stencil(unsigned int *list, unsigned int s);
unsigned int *dl_clear(unsigned int *list, unsigned int flags);
unsigned int *gim_set_texture(unsigned int *list, void *g);
unsigned int *dl_ambient_color(unsigned int *list, unsigned int color);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
unsigned int *draw_set_offsets();
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
void font_set_size(int w, int h);
void font_text_set(void *list);
unsigned int *font_text_clear(void);
int font_draw_wtext(int x, int y, int w, void *text);
float libm_cosf(float x);
void local_record_set(unsigned char *rec);

/* this module */
void dmgr_partner_init(void *heap, void *pack, int mode);
void dmgr_partner_free(void);
int dmgr_partner_update(int net);

void *dmgr_jk_file(unsigned short i);
extern char *dmgr_jk_names[6];
void dmgr_jk_bg_anim(unsigned short mode);
void dmgr_jk_title_anim(unsigned short mode);
int dmgr_jk_anim_done(unsigned short i);
void dmgr_jk_cards_reset(void);
int dmgr_jk_cards_move(unsigned short n, unsigned short side);
void dmgr_jk_cards_hover(void);
void dmgr_jk_pick(void);
void dmgr_jk_reveal(void);
void dmgr_jk_choose(void);
int dmgr_jk_done(void);
void dmgr_jk_cpu_pick(void);
void dmgr_jk_send(int a, int b, int c, unsigned char *data, int n);
void dmgr_jk_start_choice(void);
unsigned int *dmgr_jk_draw_ehf(unsigned int *list, unsigned short i);
unsigned int *dmgr_jk_draw_cards(unsigned int *list);
unsigned int *dmgr_jk_draw_card(unsigned int *list, unsigned short side, DmgrJkCard *c);
unsigned int *dmgr_jk_draw_results(unsigned int *list);
unsigned int *dmgr_jk_draw_cursor(unsigned int *list);
unsigned int *dmgr_jk_draw_wait(unsigned int *list, unsigned short side);
void dmgr_jk_free(void);
unsigned int *dmgr_partner_draw(unsigned int *list);


void dmgr_menu_init(void *pack, void *heap)
{
    DmgrJkNet *n;
    int peer;
    int k;

    dmgr_jk = heap_alloc(heap, sizeof *dmgr_jk);
    memset(dmgr_jk, 0, sizeof *dmgr_jk);
    dmgr_jk->heap = heap;
    ehp_mount(0, pack);
    dmgr_jk->pack = pack;
    dmgr_jk->first = 0;
    if (((DmgrGameHdr *)game_state_get())->kind2 == 2) {
        k = ((DuelResults *)game_state2_get())->n_duels;
        if (k > 0) {
            switch (((DuelResults *)game_state2_get())->duels[k - 1]) {
            case 1:
                dmgr_jk->won = 0;
                dmgr_jk->first = 1;
                break;
            case 3:
                dmgr_jk->first = 0;
                break;
            case 2:
                dmgr_jk->won = 1;
                dmgr_jk->first = 1;
                break;
            }
        }
    }
    if (sysvar_get_a98() != 0) {
        dmgr_jk->net.peer = peer_self_get();
        dmgr_jk->net.side = netsync_local_side();
        n = &dmgr_jk->net;
        peer = n->peer;
        if (peer == netsync_side_player(n->side)) {
            n->host = 1;
        } else {
            n->host = 0;
        }
    }
    if (((DmgrGameHdr *)game_state_get())->kind == 5) {
        dmgr_jk->first = 2;
    }
    font_set_flag4(1);
    font_set_flag2(1);
    if (cfg_get_language() == 0) {
        font_set_flag1(1);
        font_set_flag8(0);
    } else {
        font_set_flag1(0);
        font_set_flag8(1);
    }
    dmgr_jk->state = 1;
    fade_start(1, 0xFF000000, 0x10);
    gfx_overlay_enable(0);
    if (((DmgrGameHdr *)game_state_get())->kind2 != 2 || ((DuelResults *)game_state2_get())->n_duels == 0) {
        switch (((DmgrGameHdr *)game_state_get())->bgm_set) {
        case 4:
        case 5:
            bgm_play(13);
            break;
        default:
            bgm_play(12);
            break;
        }
    }
}

#ifdef NON_MATCHING /* o mwcc reordena os acessos a dmgr_jk e os blocos dos estados; forma geral igual */
int dmgr_menu_update(void)
{
    PlayerSlot *s;
    int mode;
    int i;
    int off;
    int a;
    int b;

    switch (dmgr_jk->state) {
    case 0:
        break;
    case 1:
        if (fade_is_done() != 0) {
            if (dmgr_jk->first == 2) {
                mode = 8;
                if (((DmgrGameHdr *)game_state_get())->partner_mode == 2) {
                    mode = 9;
                }
                dmgr_partner_init(dmgr_jk->heap, dmgr_jk->pack, mode);
                dmgr_jk->state = 11;
            } else {
                dmgr_jk_bg_anim(0);
                se_play(0x6287);
                dmgr_jk->state = 2;
            }
            dmgr_jk->sub = 0;
        }
        break;
    case 2:
        if (dmgr_jk_anim_done(2) != 0) {
            dmgr_partner_init(dmgr_jk->heap, dmgr_jk->pack, 10);
            dmgr_jk_bg_anim(1);
            if (sysvar_get_a98() != 0) {
                dmgr_jk->text = table_get_entry(dmgr_jk_file(5), 4);
                if (((DmgrGameHdr *)game_state_get())->kind2 == 2 &&
                    ((DuelResults *)game_state2_get())->n_duels > 0) {
                    dmgr_jk->state = 4;
                    dmgr_jk->net.synced = 0;
                } else {
                    dmgr_jk_send(3, 1, 0, 0, 0);
                    dmgr_jk->state = 3;
                }
            } else if (dmgr_jk->first == 1) {
                dmgr_jk->state = 8;
            } else {
                dmgr_jk_cards_reset();
                dmgr_jk->state = 5;
            }
            dmgr_jk->sub = 0;
            dmgr_jk->timer = 0;
        }
        break;
    case 3:
        if (peers_all_match(3, 1, 0xFF) != 0) {
            dmgr_jk_start_choice();
        }
        break;
    case 4:
        switch (dmgr_jk->sub) {
        case 0:
            dmgr_jk->netbuf = heap_alloc(dmgr_jk->heap, 0x600);
            netsync_transfer_setup(dmgr_jk->netbuf, game_state_get(), 0x180);
            dmgr_jk->sub++;
            break;
        case 1:
            if (netsync_transfer() != 0) {
                for (i = 0, off = 0; i < 4; i++, off += 0x188) {
                    s = (PlayerSlot *)((char *)game_state_get() + off + 0x120);
                    if (s->type < 2) {
                        memcpy(SLOT_RECIPE(s), dmgr_jk->netbuf + s->net_id * 0x180, 0x108);
                    }
                }
                if (dmgr_jk->netbuf != 0) {
                    heap_set_level(dmgr_jk->heap, dmgr_jk->netbuf);
                }
                dmgr_jk_start_choice();
                dmgr_jk->net.synced = 1;
            }
            break;
        }
        break;
    case 5:
        a = dmgr_jk_cards_move(0x18, 0);
        b = dmgr_jk_cards_move(0x18, 1);
        if (a != 0 && b != 0) {
            if (sysvar_get_a98() == 0 || dmgr_jk->net.host) {
                dmgr_jk->cursor_on = 1;
            } else {
                dmgr_jk->cursor_on = 0;
            }
            dmgr_jk->state = 6;
            dmgr_jk->text = table_get_entry(dmgr_jk_file(5), 1);
            dmgr_jk->anim[1] = 0;
            dmgr_jk->anim[0] = 0;
            dmgr_jk->t[1] = 0;
            dmgr_jk->t[0] = 0;
        }
        break;
    case 6:
        dmgr_jk_pick();
        break;
    case 7:
        dmgr_jk_reveal();
        break;
    case 8:
        dmgr_jk_title_anim(dmgr_jk->choice);
        if (sysvar_get_a98() != 0) {
            if (dmgr_jk->won) {
                if (dmgr_jk->net.host) {
                    dmgr_jk->text = table_get_entry(dmgr_jk_file(5), 2);
                    goto next;
                }
            }
            if (netsync_side_player((dmgr_jk->net.side + 1) & 1) == 4 && dmgr_jk->net.peer == 0) {
                dmgr_jk->target = rand_next_alias() & 1;
                dmgr_jk->text = table_get_entry(dmgr_jk_file(5), 3);
            } else {
                dmgr_jk->text = table_get_entry(dmgr_jk_file(5), 3);
            }
        } else if (dmgr_jk->won) {
            dmgr_jk->text = table_get_entry(dmgr_jk_file(5), 2);
        } else {
            dmgr_jk->target = rand_next_alias() & 1;
            dmgr_jk->text = table_get_entry(dmgr_jk_file(5), 3);
        }
    next:
        dmgr_jk->state = dmgr_jk->state + 1;
        break;
    case 9:
        dmgr_jk_choose();
        break;
    case 10:
        return dmgr_jk_done();
    case 11:
        a = dmgr_jk->closed;
        if (sysvar_get_a98() != 0 && a != 0) {
            local_record_set1(1);
            if (peers_all_match(4, 1, 1) == 0) {
                a = 0;
            }
        }
        if (dmgr_partner_update(a) != 0) {
            fade_start_in(1, 0xFFFFFFFF, 0x10);
            bgm_cmd(600);
            dmgr_jk->state = 12;
        }
        if (dmgr_jk->choice != 0) {
            return 2;
        }
        return 1;
    case 12:
        if (fade_is_done() != 0) {
            dmgr_jk->state = 13;
        }
        if (dmgr_jk->choice != 0) {
            return 2;
        }
        return 1;
    case 13:
        dmgr_partner_free();
        dmgr_jk_free();
        return 3;
    }
    return 0;
}
#endif

void dmgr_menu_draw(void)
{
    unsigned int *list;

    list = dl_select_layer(0, 0);
    list = dl_clear_color(list, 0xFF000000);
    list = dl_clear_depth(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear(list, 0x17);
    dl_set_list(list);
    list = dl_select_layer(4, 0);
    list = dmgr_jk_draw_ehf(list, 1);
    list = dmgr_jk_draw_ehf(list, 0);
    switch (dmgr_jk->state) {
    case 11:
    case 12:
        list = dmgr_partner_draw(list);
        break;
    case 2:
    case 3:
    case 4:
        if (dmgr_jk->bg_on) {
            list = dmgr_jk_draw_ehf(list, 2);
        }
        if (dmgr_jk->partner_on) {
            list = dmgr_partner_draw(list);
        }
        break;
    case 5:
    case 6:
    case 7:
        list = dmgr_jk_draw_cards(list);
        list = dmgr_jk_draw_results(list);
        if (dmgr_jk->cursor_on) {
            list = dmgr_jk_draw_cursor(list);
        }
        if (dmgr_jk->bg_on) {
            list = dmgr_jk_draw_ehf(list, 2);
        }
        if (dmgr_jk->partner_on) {
            list = dmgr_partner_draw(list);
        }
        if (dmgr_jk->wait[0] != 0) {
            list = dmgr_jk_draw_wait(list, 0);
        }
        if (dmgr_jk->wait[1] != 0) {
            list = dmgr_jk_draw_wait(list, 1);
        }
        break;
    case 8:
    case 9:
        if (dmgr_jk->title_on) {
            list = dmgr_jk_draw_ehf(list, 3);
        }
        if (dmgr_jk->bg_on) {
            list = dmgr_jk_draw_ehf(list, 2);
        }
        if (dmgr_jk->partner_on) {
            list = dmgr_partner_draw(list);
        }
        break;
    case 10:
        if (dmgr_jk->title_on) {
            list = dmgr_jk_draw_ehf(list, 3);
        }
        if (dmgr_jk->bg_on) {
            list = dmgr_jk_draw_ehf(list, 2);
        }
        if (dmgr_jk->partner_on) {
            list = dmgr_partner_draw(list);
        }
        break;
    }
    if (dmgr_jk->text != 0) {
        font_set_size(0xE, 0xE);
        font_set_color(0xFFFFFFFF);
        font_text_set(list);
        font_draw_wtext(0x3680, 0x3EC0, 0x3F40, dmgr_jk->text);
        list = font_text_clear();
    }
    fade_apply(&list);
    dl_set_list(list);
}

void dmgr_jk_free(void)
{
    heap_set_level(dmgr_jk->heap, dmgr_jk);
    dmgr_jk = 0;
}

void dmgr_menu_close(void)
{
    dmgr_jk->closed = 1;
}

int dmgr_partner_sync(void)
{
    if (sysvar_get_a98() == 0) {
        return 1;
    }
    if (((DmgrGameHdr *)game_state_get())->kind2 == 2 && ((DuelResults *)game_state2_get())->n_duels > 0) {
        return dmgr_jk->net.synced;
    }
    return 1;
}

#ifdef NON_MATCHING /* o original reaproveita o mesmo registrador para os dois bits gravados com 1 no caso 1 */
void dmgr_jk_bg_anim(unsigned short mode)
{
    void *e;
    int from = 0;
    int to = 0;

    e = dmgr_jk_file(2);
    if (e == 0) {
        return;
    }
    switch (mode) {
    case 0:
        dmgr_jk->bg_on = 1;
        to = 0xC;
        dmgr_jk->partner_on = 0;
        break;
    case 1:
        dmgr_jk->bg_on = 1;
        from = 0xD;
        to = 0x27;
        dmgr_jk->partner_on = 1;
        break;
    case 2:
        dmgr_jk->bg_on = 1;
        from = 0x28;
        to = 0x2D;
        dmgr_jk->partner_on = 0;
        break;
    case 3:
        dmgr_jk->bg_on = 0;
        to = 0x2D;
        from = to;
        dmgr_jk->partner_on = 0;
        break;
    }
    ehf_set_anim(e, from, to + 1);
}
#endif

void dmgr_jk_title_anim(unsigned short mode)
{
    void *e;
    int from = 0;
    int to = 0;

    e = dmgr_jk_file(3);
    if (e == 0) {
        return;
    }
    switch (mode) {
    case 0:
        dmgr_jk->title_on = 1;
        break;
    case 1:
        to = 0x28;
        from = 0x28;
        dmgr_jk->title_on = 1;
        break;
    case 2:
        from = 1;
        to = 0x27;
        dmgr_jk->title_on = 1;
        break;
    case 3:
        from = 0x29;
        to = 0x4F;
        dmgr_jk->title_on = 1;
        break;
    case 4:
        from = 0x78;
        to = 0x9F;
        dmgr_jk->title_on = 1;
        break;
    case 5:
        from = 0x50;
        to = 0x77;
        dmgr_jk->title_on = 1;
        break;
    case 6:
        to = 0x77;
        from = 0x77;
        dmgr_jk->title_on = 0;
        break;
    }
    ehf_set_anim(e, from, to + 1);
}

int dmgr_jk_anim_done(unsigned short i)
{
    void *e;
    int target;
    int limit;
    int n;

    e = dmgr_jk_file(i);
    if (e != 0) {
        ehf_get_size(e, &target, &limit);
        n = ehf_get_frame_count(e);
        if (limit > 0 && n >= limit - 1) {
            return 1;
        }
    }
    return 0;
}

const char dmgr_lang_chars[8] = "jegfis";

void *dmgr_jk_file(unsigned short i)
{
    char name[0x100];

    if (dmgr_jk->files[i] == 0) {
        sprintf(name, dmgr_jk_names[i], dmgr_lang_chars[cfg_get_language()]);
        dmgr_jk->files[i] = ehp_find(dmgr_jk->pack, name, 0);
    }
    return dmgr_jk->files[i];
}

#ifdef NON_MATCHING /* o original não reduz -45 * i no segundo laço e copia os retângulos com menos registradores */
void dmgr_jk_cards_reset(void)
{
    DmgrJkCard *c;
    int i;

    for (i = 0; i < 3; i++) {
        c = &dmgr_jk->card[0][i];
        c->from.x = 0x208 + i * 90;
        c->from.y = 0xD2;
        c->from.w = 100;
        c->from.h = 0;
        c->cur = c->from;
        c->to = c->from;
        c->to.x = 0x96 + i * 90;
        c->to.y = c->from.y;
        c->hand = i;
        c->on = 1;
    }
    dmgr_jk->pick[0] = 1;
    for (i = 0; i < 3; i++) {
        c = &dmgr_jk->card[1][i];
        c->from.x = -0x14 - i * 45;
        c->from.y = 0x40;
        c->from.w = 0x32;
        c->from.h = 0xB4;
        c->cur = c->from;
        c->to = c->from;
        c->to.x = 0x11D - i * 45;
        c->to.y = c->from.y;
        c->hand = i;
        c->on = 1;
    }
}
#endif

int dmgr_jk_cards_move(unsigned short n, unsigned short side)
{
    DmgrJkCard *c;
    int t;
    int i;

    t = dmgr_jk->t[side];
    dmgr_jk->t[side] = t + 1;
    if (n < t) {
        c = dmgr_jk->card[side];
        c[0].cur = c[0].to;
        c[1].cur = c[1].to;
        c[2].cur = c[2].to;
        dmgr_jk->t[side] = n;
        return 1;
    }
    for (i = 0; i < 3; i++) {
        c = &dmgr_jk->card[side][i];
        c->cur.x = c->from.x + (c->to.x - c->from.x) * t * t / (n * n);
        c->cur.y = c->from.y + (c->to.y - c->from.y) * t * t / (n * n);
        c->cur.w = c->from.w + (c->to.w - c->from.w) * t * t / (n * n);
        c->cur.h = c->from.h + (c->to.h - c->from.h) * t * t / (n * n);
    }
    return 0;
}

void dmgr_jk_cards_hover(void)
{
    DmgrJkCard *c;
    int i;

    for (i = 0; i < 3; i++) {
        c = &dmgr_jk->card[0][i];
        if (i == dmgr_jk->pick[0]) {
            c->to.y = 0xBE;
        } else {
            c->to.y = 0xD2;
        }
        if (c->cur.y < c->to.y) {
            c->cur.y += 4;
            if (c->to.y < c->cur.y) {
                c->cur.y = c->to.y;
            }
        } else if (c->to.y < c->cur.y) {
            c->cur.y -= 4;
            if (c->cur.y < c->to.y) {
                c->cur.y = c->to.y;
            }
        }
    }
}

#ifdef NON_MATCHING /* lógica conferida com o asm. 185 palavras: o original guarda o resultado de dmgr_jk_cards_move e as variáveis do laço final em t4-t6 (o chamado não os usa); o mwcc usa s0-s2 e desloca o resto; anim[i] testado com bgtzl no case 3 */
void dmgr_jk_pick(void)
{
    PadState *pad = pad_get();
    unsigned char hand;
    unsigned char msg[2];
    DmgrJkCard tmp;
    DmgrJkCard *a;
    DmgrJkCard *b;
    signed char *peer;
    int i;
    int j;
    int k;
    int m0;
    int m1;
    int n;

    switch (dmgr_jk->sub) {
    case 0:
        if (sysvar_get_a98() != 0 && !dmgr_jk->net.host) {
            dmgr_jk_send(3, 2, 1, 0, 0);
            dmgr_jk->text = table_get_entry(dmgr_jk_file(5), 3);
            dmgr_jk->sub = 2;
            break;
        }
        dmgr_jk->sub++;
    case 1:
        if ((pad->b.trig & region_pick_value(0)) != 0) {
            if (sysvar_get_a98() != 0) {
                msg[0] = dmgr_jk->pick[0];
                msg[1] = 0;
                if (netsync_side_player((dmgr_jk->net.side + 1) & 1) == 4 && dmgr_jk->net.peer == 0) {
                    dmgr_jk_cpu_pick();
                    msg[1] = dmgr_jk->pick[1];
                }
                dmgr_jk_send(3, 2, 1, msg, 2);
                dmgr_jk->wait[0] = 1;
                dmgr_jk->sub = 3;
            } else {
                dmgr_jk->text = 0;
                dmgr_jk_cpu_pick();
                dmgr_jk->sub = 4;
                dmgr_jk->anim[1] = 1;
            }
            dmgr_jk->anim[0] = 1;
            se_play(0x6213);
        } else {
            if (pad->b.rep & 0x80) {
                dmgr_jk->pick[0] = (dmgr_jk->pick[0] + 2) % 3;
                se_play(0x6212);
            } else if (pad->b.rep & 0x20) {
                dmgr_jk->pick[0] = (dmgr_jk->pick[0] + 1) % 3;
                se_play(0x6212);
            }
            dmgr_jk_cards_hover();
            if (sysvar_get_a98() != 0) {
                k = netsync_side_player((dmgr_jk->net.side + 1) & 1);
                if (k == 4) {
                    peer = (signed char *)peer_get(0);
                } else {
                    peer = (signed char *)peer_get(k);
                }
                if (peer[2] == 1 && dmgr_jk->anim[1] == 0) {
                    dmgr_jk->anim[1] = 1;
                    dmgr_jk->wait[1] = 1;
                }
                hand = dmgr_jk->pick[0];
                dmgr_jk_send(3, 2, 0, &hand, 1);
            }
        }
        break;
    case 2:
        for (i = 0; i < 2; i++) {
            k = netsync_side_player(i);
            j = i != dmgr_jk->net.side;
            if (k == 4) {
                peer = (signed char *)peer_get(0);
                dmgr_jk->pick[j] = peer[5];
            } else {
                peer = (signed char *)peer_get(k);
                dmgr_jk->pick[j] = peer[4];
            }
            if (peer[2] == 1) {
                if (i == 0) {
                    dmgr_jk->anim[0] = 1;
                    dmgr_jk->wait[0] = 1;
                    dmgr_jk->sub++;
                } else if (dmgr_jk->anim[1] == 0) {
                    dmgr_jk->anim[1] = 1;
                    dmgr_jk->wait[1] = 1;
                }
            }
        }
        dmgr_jk_cards_hover();
        break;
    case 3:
        if (peers_all_match(3, 2, 1) != 0) {
            for (i = 0; i < 2; i++) {
                k = netsync_side_player(i);
                j = i != dmgr_jk->net.side;
                if (k == 4) {
                    peer = (signed char *)peer_get(0);
                    dmgr_jk->pick[j] = peer[5];
                } else {
                    peer = (signed char *)peer_get(k);
                    dmgr_jk->pick[j] = peer[4];
                }
                if (dmgr_jk->anim[i] == 0) {
                    dmgr_jk->anim[i] = 1;
                }
            }
            dmgr_jk->wait[0] = 0;
            dmgr_jk->wait[1] = 0;
            dmgr_jk->sub++;
        }
        break;
    case 4:
        if (dmgr_jk->anim[0] == 3 && dmgr_jk->anim[1] == 3) {
            dmgr_jk->card[0][0].on = 0;
            dmgr_jk->card[0][1].on = 0;
            a = &dmgr_jk->card[0][2];
            a->hand = dmgr_jk->pick[0];
            a->from = a->cur;
            a->to.x = 0xBE;
            a->to.y = 0x88;
            a->to.w = 100;
            a->to.h = 0x168;
            a->on = 1;
            dmgr_jk->card[1][0].on = 0;
            dmgr_jk->card[1][1].on = 0;
            a = &dmgr_jk->card[1][2];
            a->hand = dmgr_jk->pick[1];
            a->from = a->cur;
            a->to.x = 0x122;
            a->to.y = 0x88;
            a->to.w = 100;
            a->to.h = 0x168;
            a->on = 1;
            dmgr_jk->t[1] = 0;
            dmgr_jk->t[0] = 0;
            dmgr_jk->sub++;
        }
        break;
    case 5:
        m0 = dmgr_jk_cards_move(0x18, 0);
        m1 = dmgr_jk_cards_move(0x18, 1);
        if (m0 != 0 && m1 != 0) {
            switch ((dmgr_jk->pick[0] - dmgr_jk->pick[1] + 3) % 3) {
            case 0:
                dmgr_jk->res_anim[0] = 3;
                dmgr_jk->res_anim[1] = 3;
                dmgr_jk->result = 3;
                break;
            case 1:
                dmgr_jk->res_anim[0] = 2;
                dmgr_jk->res_anim[1] = 1;
                dmgr_jk->result = 2;
                break;
            case 2:
                dmgr_jk->res_anim[0] = 1;
                dmgr_jk->res_anim[1] = 2;
                dmgr_jk->result = 1;
                break;
            }
            dmgr_jk->sub++;
            dmgr_jk->timer = 0;
            se_play(0x621A);
        }
        break;
    case 6:
        if (++dmgr_jk->timer > 0x18) {
            dmgr_jk->state = 7;
            dmgr_jk->res_anim[0] = 0;
            dmgr_jk->res_anim[1] = 0;
            dmgr_jk->sub = 0;
            dmgr_jk->timer = 0;
        }
        break;
    }
    for (n = 0; n < 2; n++) {
        switch (dmgr_jk->anim[n]) {
        case 1:
            a = &dmgr_jk->card[n][dmgr_jk->pick[n]];
            b = &dmgr_jk->card[n][2];
            tmp = *a;
            *a = *b;
            *b = tmp;
            b = dmgr_jk->card[n];
            if (n == 0) {
                b[0].from = b[0].cur;
                b[0].to.x = 0xF0;
                b[0].to.y = 0xD2;
                b[1].from = b[1].cur;
                b[1].to.x = 0xF0;
                b[1].to.y = 0xD2;
                b[2].from = b[2].cur;
                b[2].to.x = 0xF0;
                b[2].to.y = 0xD2;
                dmgr_jk->cursor_on = 0;
            } else {
                b[0].from = b[0].cur;
                b[0].to.x = 0xF0;
                b[0].to.y = 0x40;
                b[1].from = b[1].cur;
                b[1].to.x = 0xF0;
                b[1].to.y = 0x40;
                b[2].from = b[2].cur;
                b[2].to.x = 0xF0;
                b[2].to.y = 0x40;
            }
            dmgr_jk->anim[n]++;
            dmgr_jk->t[n] = 0;
            break;
        case 2:
            if (dmgr_jk_cards_move(8, n) != 0) {
                dmgr_jk->anim[n]++;
                dmgr_jk->t[n] = 0;
            }
            break;
        }
    }
}
#endif

#ifdef NON_MATCHING /* função grande: o original relê dmgr_jk a cada cópia de retângulo */
void dmgr_jk_reveal(void)
{
    DmgrJkCard *a = &dmgr_jk->card[0][2];
    DmgrJkCard *b = &dmgr_jk->card[1][2];
    int i;
    int k;

    switch (dmgr_jk->sub) {
    case 0:
        if (dmgr_jk->result == 3) {
            a->from = a->cur;
            a->to = a->cur;
            a->to.x = -0x50;
            b->from = b->cur;
            b->to = b->cur;
            b->to.x = 0x230;
            dmgr_jk->sub = 4;
        } else {
            a->from = a->cur;
            a->to = a->cur;
            if (dmgr_jk->result == 1) {
                a->to.x = 200;
            }
            b->from = b->cur;
            b->to = b->cur;
            if (dmgr_jk->result == 2) {
                b->to.x = 0x118;
            }
            dmgr_jk->sub++;
        }
        dmgr_jk->t[1] = 0;
        dmgr_jk->t[0] = 0;
        break;
    case 1:
        i = dmgr_jk_cards_move(2, 0);
        k = dmgr_jk_cards_move(2, 1);
        if (i != 0 && k != 0) {
            a->from = a->cur;
            a->to = a->cur;
            if (dmgr_jk->result == 1) {
                a->to.x = 0x8C;
            }
            b->from = b->cur;
            b->to = b->cur;
            if (dmgr_jk->result == 2) {
                b->to.x = 0x154;
            }
            dmgr_jk->sub++;
            dmgr_jk->t[1] = 0;
            dmgr_jk->t[0] = 0;
        }
        break;
    case 2:
        i = dmgr_jk_cards_move(4, 0);
        k = dmgr_jk_cards_move(4, 1);
        if (i != 0 && k != 0) {
            a->from = a->cur;
            a->to = a->cur;
            if (dmgr_jk->result == 1) {
                a->to.x = 0xD2;
            }
            b->from = b->cur;
            b->to = b->cur;
            if (dmgr_jk->result == 2) {
                b->to.x = 0x10E;
            }
            dmgr_jk->sub++;
            dmgr_jk->t[1] = 0;
            dmgr_jk->t[0] = 0;
        }
        break;
    case 3:
        i = dmgr_jk_cards_move(2, 0);
        k = dmgr_jk_cards_move(2, 1);
        if (i != 0 && k != 0) {
            switch (dmgr_jk->result) {
            case 1:
                a->from = a->cur;
                a->to = a->cur;
                a->to.x = 0xF0;
                b->from = b->cur;
                b->to = b->cur;
                b->to.x = 0x230;
                break;
            case 2:
                b->from = b->cur;
                b->to = b->cur;
                b->to.x = 0xF0;
                a->from = a->cur;
                a->to = a->cur;
                a->to.x = -0x50;
                break;
            }
            se_play(0x628B);
            dmgr_jk->sub++;
            dmgr_jk->t[1] = 0;
            dmgr_jk->t[0] = 0;
        }
        break;
    case 4:
        i = dmgr_jk_cards_move(0xC, 0);
        k = dmgr_jk_cards_move(0xC, 1);
        if (i != 0 && k != 0) {
            if (dmgr_jk->result == 3) {
                dmgr_jk_cards_reset();
                if (sysvar_get_a98() == 0) {
                    dmgr_jk->state = 5;
                } else {
                    dmgr_jk_send(3, 1, 0, 0, 0);
                    dmgr_jk->state = 3;
                }
                dmgr_jk->result = 0;
                dmgr_jk->sub = 0;
                dmgr_jk->timer = 0;
            } else {
                dmgr_jk->sub++;
            }
            dmgr_jk->t[1] = 0;
            dmgr_jk->t[0] = 0;
        }
        break;
    case 5:
        if (++dmgr_jk->timer > 0x18) {
            if (dmgr_jk->result == 1) {
                dmgr_jk->won = 1;
            } else {
                dmgr_jk->won = 0;
            }
            if (sysvar_get_a98() != 0) {
                dmgr_jk_send(4, 1, 0, 0, 0);
            }
            dmgr_jk->state = 8;
            dmgr_jk->sub = 0;
            dmgr_jk->timer = 0;
        }
        break;
    }
}
#endif

void dmgr_jk_choose(void)
{
    unsigned char c0;
    unsigned char c1;
    unsigned char msg[5];
    unsigned int r2;
    unsigned int r3;
    unsigned int r;
    unsigned char msg5[5];
    PadState *pad;
    signed char *peer;
    unsigned int rl;
    int input = 0;
    int k;

    if (sysvar_get_a98() != 0) {
        if (dmgr_jk->won && dmgr_jk->net.host) {
            input = 1;
            goto input;
        }
        if (netsync_side_player((dmgr_jk->net.side + 1) & 1) == 4 && dmgr_jk->net.peer == 0) {
            if (++dmgr_jk->timer > 0x2D && dmgr_jk->choice == dmgr_jk->target) {
                r2 = rand_next_alias();
                dmgr_jk->choice = dmgr_jk->target;
                msg[0] = dmgr_jk->choice;
                memcpy(&msg[1], &r2, 4);
                dmgr_jk_send(4, 2, 1, msg, 5);
                dmgr_jk_title_anim(dmgr_jk->choice + 2);
                ((DmgrGameHdr *)game_state_get())->mode = r2;
                dmgr_jk->state = 10;
                dmgr_jk->sub = 0;
                dmgr_jk->timer = 0;
                dmgr_jk->text = 0;
                se_play(0x6213);
            } else {
                dmgr_jk->choice = dmgr_jk->timer >> 1 & 1;
                dmgr_jk_title_anim(dmgr_jk->choice);
                c0 = dmgr_jk->choice;
                dmgr_jk_send(4, 2, 0, &c0, 1);
            }
        } else {
            if (dmgr_jk->won) {
                k = netsync_side_player(dmgr_jk->net.side);
                if (k == 4) {
                    k = 0;
                }
                peer = (signed char *)peer_get(k);
            } else {
                k = netsync_side_player((dmgr_jk->net.side + 1) & 1);
                if (k == 4) {
                    k = 0;
                }
                peer = (signed char *)peer_get(k);
            }
            if (peer[0] == 4 && peer[1] == 2) {
                dmgr_jk->choice = peer[4];
                if (peer[2] == 1) {
                    memcpy(&r3, peer + 5, 4);
                    ((DmgrGameHdr *)game_state_get())->mode = r3;
                    dmgr_jk_send(4, 2, 1, 0, 0);
                    dmgr_jk_title_anim(dmgr_jk->choice + 2);
                    dmgr_jk->state = 10;
                    dmgr_jk->sub = 0;
                    dmgr_jk->timer = 0;
                    dmgr_jk->text = 0;
                    se_play(0x6213);
                } else {
                    dmgr_jk_title_anim(dmgr_jk->choice);
                }
            }
        }
    } else {
        if (dmgr_jk->won) {
            input = 1;
        } else {
            if (++dmgr_jk->timer > 0x2D && dmgr_jk->choice == dmgr_jk->target) {
                rl = rand_next_alias();
                dmgr_jk->choice = dmgr_jk->target;
                dmgr_jk_title_anim(dmgr_jk->choice + 2);
                dmgr_jk->state = 10;
                dmgr_jk->sub = 0;
                dmgr_jk->timer = 0;
                dmgr_jk->text = 0;
                ((DmgrGameHdr *)game_state_get())->mode = rl;
                se_play(0x6213);
            } else {
                dmgr_jk->choice = dmgr_jk->timer >> 1 & 1;
                dmgr_jk_title_anim(dmgr_jk->choice);
            }
        }
    }
input:
    if (input) {
        pad = pad_get();
        if ((pad->b.trig & region_pick_value(0)) != 0) {
            r = rand_next_alias();
            dmgr_jk_title_anim(dmgr_jk->choice + 2);
            if (sysvar_get_a98() != 0) {
                msg5[0] = dmgr_jk->choice;
                memcpy(&msg5[1], &r, 4);
                dmgr_jk_send(4, 2, 1, msg5, 5);
            }
            ((DmgrGameHdr *)game_state_get())->mode = r;
            dmgr_jk->state = 10;
            dmgr_jk->sub = 0;
            se_play(0x6213);
            dmgr_jk->text = 0;
        } else {
            if (pad->b.rep & 0x50) {
                dmgr_jk->choice = (dmgr_jk->choice + 1) & 1;
                dmgr_jk_title_anim(dmgr_jk->choice);
                se_play(0x6212);
            }
            if (sysvar_get_a98() != 0) {
                c1 = dmgr_jk->choice;
                dmgr_jk_send(4, 2, 0, &c1, 1);
            }
        }
    }
}

#ifdef NON_MATCHING /* o caso 0 do original sai por "li v0,0; b fim; nop", que o mwcc funde no atraso */
int dmgr_jk_done(void)
{
    int mode;

    switch (dmgr_jk->sub) {
    case 0:
        if (dmgr_jk_anim_done(3) != 0) {
            if (!dmgr_jk->won) {
                dmgr_jk->choice = (dmgr_jk->choice + 1) & 1;
            }
            dmgr_jk_title_anim(dmgr_jk->choice + 4);
            dmgr_jk->timer = 0;
            dmgr_jk->sub++;
            return 0;
        }
        return 0;
    case 1:
        if (dmgr_jk_anim_done(3) != 0) {
            dmgr_jk_title_anim(6);
            dmgr_jk_bg_anim(2);
            dmgr_jk->timer = 0;
            dmgr_jk->sub++;
        }
        if (dmgr_jk->timer++ == 0x1E) {
            se_play(0x6288);
        }
        break;
    case 2:
        if (dmgr_jk_anim_done(2) != 0) {
            dmgr_jk_bg_anim(3);
            dmgr_jk->timer = 0;
            dmgr_jk->sub++;
        }
        break;
    case 3:
        if (++dmgr_jk->timer > 0x10) {
            mode = 8;
            if (((DmgrGameHdr *)game_state_get())->partner_mode == 2) {
                mode = 9;
            }
            dmgr_partner_free();
            dmgr_partner_init(dmgr_jk->heap, dmgr_jk->pack, mode);
            dmgr_jk->state = 11;
            dmgr_jk->sub = 0;
            dmgr_jk->timer = 0;
        }
        break;
    }
    if (dmgr_jk->choice == 0) {
        return 1;
    }
    return 2;
}
#endif

void dmgr_jk_cpu_pick(void)
{
    int r = rand_next_alias() % 100;
    unsigned char h = dmgr_jk->pick[0];

    if (r < 20) {
        dmgr_jk->pick[1] = h;
    } else if (r & 1) {
        dmgr_jk->pick[1] = (h + 1) % 3;
    } else {
        dmgr_jk->pick[1] = (h + 2) % 3;
    }
}

void dmgr_jk_send(int a, int b, int c, unsigned char *data, int n)
{
    int buf[0x40];
    unsigned char *p = (unsigned char *)buf;

    memset(p, 0, sizeof buf);
    p[0] = a;
    p[1] = b;
    p[2] = c;
    if (n > 0) {
        memcpy(p + 4, data, n);
    }
    local_record_set(p);
}

void dmgr_jk_start_choice(void)
{
    if (dmgr_jk->first == 1) {
        dmgr_jk_send(4, 1, 0, 0, 0);
        dmgr_jk->state = 8;
    } else {
        dmgr_jk_cards_reset();
        dmgr_jk->state = 5;
        if (dmgr_jk->net.host) {
            dmgr_jk->text = table_get_entry(dmgr_jk_file(5), 1);
        } else {
            dmgr_jk->text = table_get_entry(dmgr_jk_file(5), 3);
        }
        dmgr_jk->t[1] = 0;
        dmgr_jk->t[0] = 0;
        dmgr_jk->wait[0] = 0;
        dmgr_jk->wait[1] = 0;
    }
    dmgr_jk->sub = 0;
    dmgr_jk->timer = 0;
}

unsigned int *dmgr_jk_draw_ehf(unsigned int *list, unsigned short i)
{
    int opts[0x18];
    void *e;

    e = dmgr_jk_file(i);
    if (e == 0) {
        return list;
    }
    memset(opts, 0, sizeof opts);
    opts[0] = *(int *)gfx_get_state();
    if (i == 3 && !dmgr_jk->won) {
        opts[3] |= 8;
        opts[12] = -0x81;
        opts[13] = -1;
    }
    list = gfx_begin_2d(list);
    list = ehf_draw(list, e, opts);
    list = gfx_end_2d(list);
    list = dl_enable(list, 4);
    list = dl_tex_func(list, 0, 1);
    return dl_blend_func(list, 0, 2, 3, 0, 0);
}

unsigned int *dmgr_jk_draw_cards(unsigned int *list)
{
    void *g;
    int side;
    int i;

    g = dmgr_jk_file(4);
    if (g == 0) {
        return list;
    }
    list = gim_set_texture(list, g);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    for (side = 0; side < 2; side++) {
        for (i = 0; i < 3; i++) {
            list = dmgr_jk_draw_card(list, side, &dmgr_jk->card[side][i]);
        }
    }
    return list;
}

#ifdef NON_MATCHING /* só os temporários da largura e da altura trocam de registrador */
unsigned int *dmgr_jk_draw_card(unsigned int *list, unsigned short side, DmgrJkCard *c)
{
    float f;
    int u;
    int v;
    int sw;
    int w;
    int h;

    if (!c->on) {
        return list;
    }
    if (c->cur.h > 0x5A && c->cur.h <= 0x10E) {
        if (region_get() == 0) {
            u = 0xF0;
            v = 0;
        } else {
            u = 0xF0;
            v = 0x74;
        }
    } else if (side == 0) {
        v = 0;
        u = c->hand * 0x50;
    } else {
        v = 0x74;
        u = c->hand * 0x50;
    }
    sw = c->cur.w * 0x50 / 100;
    f = libm_cosf(c->cur.h * 3.1415927f / 180.0f);
    if (f < 0.0f) {
        f *= -1.0f;
    }
    w = (int)((float)sw * f);
    h = c->cur.w * 0x74 / 100;
    w = c->cur.x - w / 2;
    h = c->cur.y - h / 2;
    list = draw_set_offsets(list, 0x50, 0x74);
    return draw_sprite(list, w, h, 0, u, v, 0, 0);
}
#endif

unsigned int *dmgr_jk_draw_results(unsigned int *list)
{
    void *g;

    g = dmgr_jk_file(4);
    if (g == 0) {
        return list;
    }
    list = gim_set_texture(list, g);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 0x50, 0x18, 0x50, 0x18);
    if (dmgr_jk->res_anim[0] != 0) {
        list = draw_sprite(list, 0x96, 0xC2, 0, (dmgr_jk->res_anim[0] - 1) * 0x50, 0xE8, 0, 0);
    }
    if (dmgr_jk->res_anim[1] != 0) {
        list = draw_sprite(list, 0xFA, 0xC2, 0, (dmgr_jk->res_anim[1] - 1) * 0x50, 0xE8, 0, 0);
    }
    return list;
}

unsigned int *dmgr_jk_draw_cursor(unsigned int *list)
{
    DmgrJkCard *c = &dmgr_jk->card[0][dmgr_jk->pick[0]];
    void *g;
    int x;
    int y;
    int l;
    int t;

    g = dmgr_jk_file(4);
    if (g == 0) {
        return list;
    }
    list = gim_set_texture(list, g);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    x = c->cur.x;
    y = c->cur.y;
    l = x - 0x2C;
    t = y - 0x3E;
    list = draw_set_offsets(list, 6, 6, 6, 6);
    list = draw_sprite(list, l, t, 0, 0x120, 0xE8, 0, 0);
    list = draw_sprite(list, l + 0x53, t, 0, 0x132, 0xE8, 0, 0);
    list = draw_sprite(list, l, t + 0x77, 0, 0x120, 0xFA, 0, 0);
    list = draw_sprite(list, l + 0x53, t + 0x77, 0, 0x132, 0xFA, 0, 0);
    list = draw_set_offsets(list, 1, 6, 0x4D, 6);
    list = draw_sprite(list, l + 6, t, 0, 0x126, 0xE8, 0, 0);
    list = draw_sprite(list, l + 6, t + 0x77, 0, 0x126, 0xFA, 0, 0);
    list = draw_set_offsets(list, 6, 1, 6, 0x71);
    list = draw_sprite(list, l, t + 6, 0, 0x120, 0xEE, 0, 0);
    return draw_sprite(list, l + 0x53, t + 6, 0, 0x132, 0xEE, 0, 0);
}

unsigned int *dmgr_jk_draw_wait(unsigned int *list, unsigned short side)
{
    void *g;
    int x;
    int y;
    int v;

    switch (side) {
    case 0:
        x = 0x36;
        y = 0xBA;
        v = 0;
        break;
    case 1:
        x = 0x188;
        y = 0x33;
        v = 0x25;
        break;
    default:
        return list;
    }
    g = dmgr_jk_file(4);
    if (g == 0) {
        return list;
    }
    list = gim_set_texture(list, g);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = draw_set_offsets(list, 0x24, 0x24, 0x24, 0x24);
    return draw_sprite(list, x, y, 0, 0x140, v, 0, 0);
}

char *dmgr_jk_names[6] = {
    "yugi_bg01.ehf", "yugi_bg02.ehf", "jyanken10.ehf", "jyanken20_%c.ehf", "jnk_card01.gim", "strTbl_%c.bin",
};
