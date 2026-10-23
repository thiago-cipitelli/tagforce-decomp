// @range 0x0880C8E8-0x0880F9C0
// @asm 0x0880CFA4-0x0880D360
// @asm 0x0880D360-0x0880D590
// @asm 0x0880D590-0x0880D78C
// @asm 0x0880D78C-0x0880D924
// @asm 0x0880D924-0x0880D9B0
// @asm 0x0880D9B0-0x0880D9E8
// @asm 0x0880D9E8-0x0880DA20
// @asm 0x0880DAB0-0x0880DEFC
// @asm 0x0880DEFC-0x0880E2AC
// @asm 0x0880E2AC-0x0880E3A8
// @asm 0x0880E3A8-0x0880E73C
// @asm 0x0880E73C-0x0880E9F8
// @asm 0x0880E9F8-0x0880EBA0
// @asm 0x0880ED64-0x0880EE90
// @asm 0x0880EF5C-0x0880F274
// @asm 0x0880F2FC-0x0880F44C
// @asm 0x0880F51C-0x0880F8E8
// @data 0x0881A774-0x0881B9B0
// @rodata 0x08819FF8-0x0881A068
/* Partner / versus panel of the start menu: who plays with and against whom (faces, names, decks),
   the slide-in animations of its EHF layers and the small text and sprite helpers it draws with. */
#include "libduel_mgr.h"

typedef struct DmgrSpr {               /* a sprite of the panel textures, 10 bytes */
    unsigned char file;                /* index into the gim names, 0 = by language */
    unsigned char pad;
    short r[4];                        /* u, v, w, h */
} DmgrSpr;

typedef struct DmgrChr {               /* face sprites of a character, 0x18 */
    unsigned short id : 10;
    unsigned short grp : 3;
    unsigned short nvar : 2;
    unsigned short b15 : 1;
    unsigned short pad;
    const char *code[3];
    int face;                          /* +0x10 first face index, -1 none */
    int pad14;
} DmgrChr;

/* &dmgr_pt->ent[i]; written as pointer arithmetic, which adds the index first as the original does */
#define PT_ENT(i) ((DmgrPtEnt *)dmgr_pt->ent + (i))

/* BOOT */
int cfg_get_language(void);
void *ehp_find(void *pack, const char *name, int *size);
int ehf_set_anim(void *e, int target, int limit);
int ehf_get_frame_count(void *e);
int ehf_set_pos(void *e, int a, int b);
int fade_apply(unsigned int **list);
void se_play(int id);
void *recipe_get_name(int id);
SaveRec *sysstate_rec_get(int i);
void *chara_get_name(int i);
void deck_name_copy_out(void *dst);
int charmap_lookup(unsigned short c, int a);
int charmap_lookup2(int c, unsigned char *out);
void *gfx_get_state(void);
unsigned int *gfx_begin_2d(unsigned int *list);
unsigned int *gfx_end_2d(unsigned int *list);
unsigned int *ehf_draw(unsigned int *list, void *e, void *opts);
unsigned int *dl_enable(unsigned int *list, int state);
unsigned int *dl_scissor(unsigned int *list, int x, int y, int w, int h);
unsigned int *dl_blend_func(unsigned int *list, int op, int src, int dst, unsigned int srcfix, unsigned int dstfix);
unsigned int *dl_clear_color(unsigned int *list, unsigned int color);
unsigned int *dl_clear_depth(unsigned int *list, unsigned int z);
unsigned int *dl_clear_stencil(unsigned int *list, unsigned int s);
unsigned int *dl_color(unsigned int *list, unsigned int color);
unsigned int *gim_set_texture(unsigned int *list, void *g);
unsigned int *gim_set_texture_sub(unsigned int *list, void *g, int a, int b, int c, int d);
unsigned int *dl_ambient_color(unsigned int *list, unsigned int color);
unsigned int *dl_set_tex_filter(unsigned int *list, int min, int mag);
unsigned int *draw_set_offsets(unsigned int *list, int tw, int th, int w, int h);
unsigned int *draw_sprite(unsigned int *list, int x, int y, int z, int u, int v, int flip_uv, int flip_xy);
void font_set_size(int w, int h);
void font_set_flag4(int on);
unsigned int *dl_disable(unsigned int *list, int state);
int font_print(void *text);
void font_text_set(void *list);
unsigned int *font_text_clear(void);
int font_draw_wtext(int x, int y, int w, void *text);
int font_draw_text(int x, int y, int w, void *text);

/* this module */
void dmgr_pt_setup(int mode);
void dmgr_pt_fill(int mode);
void dmgr_pt_set(short i, int kind, int type2, int src, int deck);
void dmgr_pt_set_name(short i, int kind, int type2, int src, int deck, void *name);
void dmgr_pt_set_all(short i, int kind, int type2, int src, int deck, void *name, void *deckname);
void dmgr_pt_anim_start(int on, int len);
int dmgr_pt_flip(int mode, int i);
void dmgr_pt_load(int i, int mode, int from, int to);
void dmgr_pt_anim(int i, int mode, int from, int to);
int dmgr_pt_frame(int i);
int dmgr_pt_set_pos(int i, int a, int b);
unsigned int *dmgr_pt_draw_ehf(unsigned int *list, int i);
unsigned int *dmgr_pt_draw_faces(unsigned int *list, int mode);
unsigned int *dmgr_pt_draw_names(unsigned int *list, int mode);
unsigned int *dmgr_pt_draw_name(unsigned int *list, DmgrPtEnt *e, int x, int y, int w, int small);
unsigned int *dmgr_pt_draw_decks(unsigned int *list, int mode);
unsigned int *dmgr_pt_draw_deck(unsigned int *list, int i, DmgrPtEnt *e, int x, int y, int w);
unsigned int *dmgr_pt_draw_frames(unsigned int *list, int mode);
unsigned int *dmgr_pt_draw_face(unsigned int *list, int mode, unsigned short kind, unsigned short var, short x, short y);
unsigned int *dmgr_pt_draw_body(unsigned int *list, unsigned short kind, short x, short y);
int dmgr_pt_face_uv(int mode, unsigned short kind, unsigned short var, short *u, short *v);
unsigned int *dmgr_pt_sprite_def(unsigned int *list, int i, short x, short y);
unsigned int *dmgr_pt_gfx_setup(unsigned int *list);
unsigned int *dmgr_pt_sprite(unsigned int *list, int i, short x, short y, short w, short h, short u, short v,
                             short tw, short th);
void dmgr_anim_stop(DmgrAnim *a);
void dmgr_anim_start(DmgrAnim *a, int on, int len);
int dmgr_anim_step(DmgrAnim *a);
int dmgr_anim_get(DmgrAnim *a, short *cur, short *len);
unsigned int *dmgr_text_draw(unsigned int *list, int x, int y, int w, void *text, int align);
unsigned int *dmgr_text_wide(unsigned int *list, int x, int y, int w, void *text);
unsigned int *dmgr_text_sjis(unsigned int *list, int x, int y, int w, void *text);
unsigned int *dmgr_pt_draw_chr(unsigned int *list, unsigned short kind, int x, int y, int scale, void *face,
                               void *body);
DmgrChr *dmgr_chr_find(unsigned short id);
int dmgr_chr_face(unsigned short id, unsigned short var);
extern DmgrChr dmgr_chr_tab[41];

void dmgr_partner_init(void *heap, void *pack, int mode)
{
    DmgrPartner *p;

    p = dmgr_pt = heap_alloc(heap, sizeof *p);
    memset(p, 0, sizeof *p);
    p->heap = heap;
    p->pack = pack;
    dmgr_pt_setup(mode);
}

void dmgr_partner_free(void)
{
    heap_set_level(dmgr_pt->heap, dmgr_pt);
}

void dmgr_pt_setup(int mode)
{
    DmgrPartner *p = dmgr_pt;
    unsigned char *q = &p->net;
    int f1 = 0;
    int t1 = 0;
    int f2 = 0;
    int t2 = 0x1D;

    memset(q, 0, 0x44);
    q[1] = mode;
    q[0] = sysvar_get_a98();
    switch (p->mode) {
    case 0:
    case 1:
    case 5:
    case 8:
        t1 = t2;
        break;
    case 2:
    case 3:
    case 4:
    case 6:
    case 7:
    case 9:
        f1 = 100;
        t1 = 0x81;
        break;
    case 10:
        break;
    }
    if (((DmgrGameHdr *)game_state_get())->kind2 == 2) {
        f2 = 100;
        t2 = 0x81;
    }
    switch (p->mode) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        p->state = 4;
        break;
    case 8:
    case 9:
        dmgr_pt_fill(p->mode);
        p->state = 2;
        if (((DmgrGameHdr *)game_state_get())->kind2 == 2) {
            dmgr_pt_load(3, 2, 100, 0x81);
        }
        dmgr_pt_load(1, 2, f1, t1);
        dmgr_pt_load(2, 2, f2, t2);
        dmgr_pt_load(0, 2, 0, 0);
        dmgr_pt_anim_start(1, 10);
        break;
    case 10:
        dmgr_pt_fill(p->mode);
        break;
    }
}

void dmgr_pt_fill(int mode)
{
    unsigned char *net = &dmgr_pt->net;
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    PlayerSlot *s;
    int n = 4;
    int side;
    int kind;
    int i;
    int found;
    int j;
    int m;
    int k;

    *net = sysvar_get_a98();
    switch (mode) {
    case 5:
    case 8:
    case 10:
        if (*net == 0) {
            found = 0;
            for (i = 0; i < 2; i++) {
                s = &gs->slot[i];
                if (mode == 10 && gs->partner_mode == 2 && i == 0) {
                    for (j = 0; j < 4; j++) {
                        s = &gs->slot[j];
                        if (s->type == 1) {
                            found = 1;
                            break;
                        }
                    }
                    if (!found) {
                        s = &gs->slot[i];
                    }
                }
                dmgr_pt_set_all(i, s->type, s->type2, 3, s->deck_id, s->name, s->deck_name);
            }
            return;
        }
        side = peer_self_get() % 2;
        if (gs->partner_mode == 1) {
            n = 2;
        }
        for (i = 0; i < 2; i++) {
            for (m = 0; m < n; m++) {
                s = &gs->slot[m];
                kind = s->type;
                if (kind == 0) {
                    kind = 1;
                }
                if ((i == 0 && side == s->net_id && kind == 1) || (i == 1 && side != s->net_id && kind == 1)) {
                    dmgr_pt_set_all(i, kind, s->type2, 3, s->deck_id, s->name, s->deck_name);
                    break;
                }
            }
        }
        break;
    case 6:
    case 7:
    case 9:
        if (*net == 0) {
            for (i = 0; i < 4; i++) {
                s = &gs->slot[i];
                dmgr_pt_set_all(i, s->type, s->type2, 3, s->deck_id, s->name, s->deck_name);
            }
            break;
        }
        side = peer_self_get() % 2;
        for (i = 0; i < 4; i++) {
            s = &gs->slot[i];
            k = i;
            if (side != 0) {
                if (i % 2 != 0) {
                    k = i - 1;
                } else {
                    k = i + 1;
                }
            }
            kind = s->type;
            if (kind == 0) {
                kind = 1;
            }
            dmgr_pt_set_all(k, kind, s->type2, 3, s->deck_id, s->name, s->deck_name);
        }
        break;
    }
}

void dmgr_pt_set(short i, int kind, int type2, int src, int deck)
{
    DmgrPtEnt *e = PT_ENT(i);

    if (kind >= 0) {
        e->kind = kind;
    }
    if (type2 >= 0) {
        e->type2 = type2;
    }
    if (src >= 0) {
        e->src = src;
    }
    if (deck >= 0) {
        e->deck = deck;
    }
    if (e->kind == 1 || e->kind == 0x29) {
        switch (e->src) {
        case 0:
            if (deck == 0) {
                e->deckname = 0;
            } else {
                e->deckname = sysstate_rec_get(deck - 1)->name;
            }
            break;
        case 1:
            e->deckname = recipe_get_name(deck);
            break;
        case 2:
        case 3:
            break;
        }
    } else {
        e->deckname = recipe_get_name(deck);
    }
}

void dmgr_pt_set_name(short i, int kind, int type2, int src, int deck, void *name)
{
    DmgrPtEnt *e = PT_ENT(i);

    dmgr_pt_set(i, kind, type2, src, deck);
    if (name != 0) {
        e->name = name;
    }
}

void dmgr_pt_set_all(short i, int kind, int type2, int src, int deck, void *name, void *deckname)
{
    DmgrPtEnt *e = PT_ENT(i);

    dmgr_pt_set_name(i, kind, type2, src, deck, name);
    if (deckname != 0) {
        e->deckname = deckname;
    }
}

void dmgr_pt_anim_start(int on, int len)
{
    dmgr_anim_start(&dmgr_pt->anim, on, len);
}

#ifdef NON_MATCHING /* função grande: o original guarda seis limites de quadro em $s e um na pilha; o mwcc aloca outros */
int dmgr_partner_update(int net)
{
    DmgrPartner *p = dmgr_pt;
    int a1 = 0x1D;
    int b2 = 0x3C;
    int a2 = 0x3B;
    DmgrAnim *anim = &p->anim;
    int b1 = 0x5D;
    int f1;
    int t1;
    int f2;
    int e1;

    p->net = sysvar_get_a98();
    switch (p->mode) {
    case 1:
    case 5:
    case 8:
        f1 = a1;
        t1 = a2;
        f2 = 0x3C;
        e1 = b1;
        break;
    case 2:
    case 3:
    case 4:
    case 6:
    case 7:
    case 9:
        f1 = 0x81;
        t1 = 0x9F;
        f2 = 0xA0;
        e1 = 0xC1;
        break;
    case 0:
    case 10:
        break;
    }
    if (((DmgrGameHdr *)game_state_get())->kind2 == 2) {
        b2 = 0xA0;
        a1 = 0x81;
        a2 = 0x9F;
        b1 = 0xC1;
    }
    switch (p->state) {
    case 0:
        fade_start(1, 0xFF000000, 0xF);
        p->state = 1;
        break;
    case 1:
        break;
    case 2:
        se_play(0x6289);
        p->state = 3;
    case 3:
        if (f1 == dmgr_pt_frame(1)) {
            dmgr_pt_anim(1, 2, f1 + 1, t1);
            dmgr_pt_anim(2, 2, a1 + 1, a2);
            p->state = 4;
            if (((DmgrGameHdr *)game_state_get())->kind2 == 2) {
                dmgr_pt_anim(3, 2, 0x82, 0x9F);
            }
        }
        break;
    case 4:
        if ((unsigned int)(p->mode - 8) < 3) {
            if (dmgr_anim_step(anim) != 0) {
                if (p->mode == 9) {
                    dmgr_pt_anim(0, 2, 0, 0xE);
                    p->state = 5;
                } else {
                    p->state = 6;
                }
            }
        } else if (dmgr_anim_step(anim) != 0) {
            return 1;
        }
        break;
    case 5:
        if (dmgr_pt_frame(0) == 0xE) {
            dmgr_pt_anim(0, 0, 0xE, 0xE);
            p->state = 6;
        }
        break;
    case 6:
        if ((unsigned int)(p->mode - 8) < 2) {
            pad_get();
            if (net != 0 && p->timer > 0x77) {
                dmgr_pt_anim_start(2, 10);
                se_play(0x628A);
                p->state = 7;
                dmgr_pt_anim(1, 2, f2, e1);
                dmgr_pt_anim(2, 2, b2, b1);
                if (((DmgrGameHdr *)game_state_get())->kind2 == 2) {
                    dmgr_pt_anim(3, 2, 0xA0, 0xAA);
                }
            }
            if (p->timer < 0x78) {
                p->timer++;
            }
        }
        break;
    case 7:
        if (((DmgrGameHdr *)game_state_get())->kind2 == 2 && dmgr_pt_frame(3) == 0xAA) {
            dmgr_pt_anim(3, 2, 0xAA, 0xAA);
        }
        if (e1 == dmgr_pt_frame(1)) {
            dmgr_pt_anim(1, 0, e1, e1);
            dmgr_pt_anim(2, 0, b1, b1);
            p->state = 8;
        }
        break;
    case 8:
        return 1;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* o original copia mode num $s antes do primeiro switch e reordena os testes */
unsigned int *dmgr_partner_draw(unsigned int *list)
{
    DmgrPartner *p = dmgr_pt;
    int from = 0;
    int mode = p->mode;
    int to = 0;
    int on = 0;

    switch (mode) {
    case 8:
    case 9:
        if (p->state > 1) {
            list = dmgr_pt_draw_ehf(list, 1);
            list = dmgr_pt_draw_ehf(list, 2);
        }
        break;
    }
    switch (p->mode) {
    case 5:
    case 8:
        from = 0x17;
        to = 0x46;
        on = 1;
        break;
    case 6:
    case 7:
    case 9:
        from = 0x7B;
        to = 0xAA;
        on = 1;
        break;
    }
    if (on) {
        if (from == dmgr_pt_frame(1)) {
            se_play(0x628C);
        } else if (to == dmgr_pt_frame(1)) {
            se_play(0x628D);
        }
    }
    list = dmgr_pt_draw_faces(list, mode);
    list = dmgr_pt_draw_names(list, mode);
    list = dmgr_pt_draw_decks(list, mode);
    list = dmgr_pt_draw_frames(list, mode);
    if (p->mode == 9) {
        dmgr_pt_set_pos(0, 0, 10);
        list = dmgr_pt_draw_ehf(list, 0);
    }
    if (((DmgrGameHdr *)game_state_get())->kind2 == 2 && (p->mode == 9 || p->mode == 8)) {
        list = dmgr_pt_draw_ehf(list, 3);
        if (p->state > 0 && p->state < 2 && fade_apply(&list) != 0) {
            p->state = 2;
        }
    }
    return list;
}
#endif

#ifdef NON_MATCHING /* o original testa os modos numa cadeia de if e os lados num switch esparso; ordem dos blocos diferente */
int dmgr_pt_flip(int mode, int i)
{
    short cur;
    short len = 1;
    int on;
    int side;
    int r = 0;

    on = dmgr_anim_get(&dmgr_pt->anim, &cur, &len);
    if (on == 0 || len == 0) {
        r = 0;
    } else {
        side = 1;
        if (mode == 1 || mode == 5 || mode == 8) {
            if (i == 0) {
                side = 0;
            } else {
                side = 1;
            }
        } else if (mode == 3) {
            if (i < 2) {
                side = 0;
            }
        } else if (i < 2) {
            side = 0;
        }
        switch (side) {
        case 0:
            if (on == 1) {
                r = cur * 0x168 / len - 0x168;
            } else {
                r = cur * -0x168 / len;
            }
            break;
        case 1:
            if (on == 1) {
                r = 0x168 - cur * 0x168 / len;
            } else {
                r = cur * 0x168 / len;
            }
            break;
        case 2:
            r = -0x168;
            break;
        case 3:
            r = 0x168;
            break;
        }
    }
    return r;
}
#endif

char dmgr_pt_names[38][0x40] = {
    "d_menu_partner.ehf", "d_menu_vs01.ehf", "vs_bg01_1_j.ehf", "vs_bg01_1_e.ehf", "vs_bg01_1_g.ehf",
    "vs_bg01_1_f.ehf", "vs_bg01_1_i.ehf", "vs_bg01_1_s.ehf", "vs_bg01_2_j.ehf", "vs_bg01_2_e.ehf",
    "vs_bg01_2_g.ehf", "vs_bg01_2_f.ehf", "vs_bg01_2_i.ehf", "vs_bg01_2_s.ehf", "vs_much1_j.ehf",
    "vs_much1_e.ehf", "vs_much1_g.ehf", "vs_much1_f.ehf", "vs_much1_i.ehf", "vs_much1_s.ehf",
    "vs_much2_j.ehf", "vs_much2_e.ehf", "vs_much2_g.ehf", "vs_much2_f.ehf", "vs_much2_i.ehf",
    "vs_much2_s.ehf", "vs_much3_j.ehf", "vs_much3_e.ehf", "vs_much3_g.ehf", "vs_much3_f.ehf",
    "vs_much3_i.ehf", "vs_much3_s.ehf", "d_menu_partner_j.ehf", "d_menu_partner_e.ehf",
    "d_menu_partner_g.ehf", "d_menu_partner_f.ehf", "d_menu_partner_i.ehf", "d_menu_partner_s.ehf",
};

#ifdef NON_MATCHING /* só a ordem da multiplicação por 6 e da soma do idioma difere */
void dmgr_pt_load(int i, int mode, int from, int to)
{
    DmgrPartner *p = dmgr_pt;
    void **slot = &dmgr_pt->ehf[i];
    int n;
    int k = i;

    if (i == 0 && (unsigned int)(dmgr_pt->mode - 8) < 2) {
        k = cfg_get_language() + 0x20;
    }
    if (i == 1 && (unsigned int)(p->mode - 8) < 2) {
        k = cfg_get_language() + 2;
    }
    if (i == 2 && (unsigned int)(p->mode - 8) < 2) {
        k = cfg_get_language() + 8;
    } else if (i == 3 && (unsigned int)(p->mode - 8) < 2) {
        n = ((DuelResults *)game_state2_get())->n_duels;
        k = cfg_get_language();
        k = n * 6 + k + 0xE;
    }
    *slot = ehp_find(p->pack, dmgr_pt_names[k], 0);
    dmgr_pt_anim(i, mode, from, to);
}
#endif

#ifdef NON_MATCHING /* o original reaproveita a0 para o índice multiplicado; o mwcc usa outro temporário */
void dmgr_pt_anim(int i, int mode, int from, int to)
{
    void *e = dmgr_pt->ehf[i];

    if (e != 0) {
        switch (mode) {
        case 0:
            ehf_set_anim(e, from, from + 1);
            break;
        case 1:
            ehf_set_anim(e, from, to + 1);
            break;
        case 2:
            ehf_set_anim(e, from, to + 1);
            break;
        }
    }
}
#endif

#ifdef NON_MATCHING /* só os temporários do índice trocam */
int dmgr_pt_frame(int i)
{
    void *e = dmgr_pt->ehf[i];

    if (e != 0) {
        return ehf_get_frame_count(e);
    }
    return -1;
}
#endif

#ifdef NON_MATCHING /* só os temporários do índice trocam */
int dmgr_pt_set_pos(int i, int a, int b)
{
    void *e = dmgr_pt->ehf[i];

    if (e != 0) {
        return ehf_set_pos(e, a, b);
    }
    return 0;
}
#endif

unsigned int *dmgr_pt_draw_ehf(unsigned int *list, int i)
{
    void **slot = &dmgr_pt->ehf[i];
    int opts[0x18];

    if (*slot != 0) {
        memset(opts, 0, sizeof opts);
        opts[0] = *(int *)gfx_get_state();
        list = gfx_begin_2d(list);
        list = ehf_draw(list, *slot, opts);
        list = gfx_end_2d(list);
    }
    return list;
}

short dmgr_pt_face_pos2[4] = { 0x25, 0x9E, 0x175, 0x3D };
short dmgr_pt_face_spr2[2] = { 1, 0 };
short dmgr_pt_face_pos10[4] = { 3, 0xC7, 0x197, 2 };
short dmgr_pt_face_pos4[8] = { 0x11, 0xA8, 0x11, 0x47, 0x189, 0xA8, 0x189, 0x47 };
short dmgr_pt_face_spr4[4] = { 1, 0, 1, 0 };

#ifdef NON_MATCHING /* os vetores de posição do original são locais contíguos percorridos por um ponteiro só */
unsigned int *dmgr_pt_draw_faces(unsigned int *list, int mode)
{
    DmgrPtEnt *e = dmgr_pt->ent;
    short dy = 0;
    short x;
    int r;
    int i;

    if ((unsigned int)(mode - 8) < 2) {
        dy = 10;
    }
    if (mode == 1 || mode == 5 || mode == 8) {
        short *pos = dmgr_pt_face_pos2;
        short *spr = dmgr_pt_face_spr2;
        short *p = pos;
        short *k = spr;

        for (i = 0; i < 2; i++, p += 2, k++, e++) {
            r = dmgr_pt_flip(mode, i);
            if (r != 0) {
                x = r + p[0];
            } else {
                x = p[0];
            }
            list = dmgr_pt_sprite_def(list, *k, x, (short)(dy + p[1]));
            if (e->kind > 0) {
                list = dmgr_pt_draw_face(list, mode, e->kind, e->type2, (short)(x + 3), (short)(dy + p[1] + 3));
            }
        }
    } else if (mode == 10) {
        short *pos = dmgr_pt_face_pos10;
        short *p = pos;

        for (i = 0; i < 2; i++, p += 2, e++) {
            r = dmgr_pt_flip(10, i);
            if (r != 0) {
                x = r + p[0];
            } else {
                x = p[0];
            }
            if (e->kind > 0) {
                list = dmgr_pt_draw_face(list, 10, e->kind, e->type2, (short)(x + 3), (short)(p[1] + 3));
            }
        }
    } else {
        short *pos = dmgr_pt_face_pos4;
        short *spr = dmgr_pt_face_spr4;
        short *p = pos;
        short *k = spr;

        if (mode == 3) {
            for (i = 0; i < 4; i++, p += 2, k++, e++) {
                if (i % 2 == 0) {
                    r = dmgr_pt_flip(3, i);
                    if (r != 0) {
                        x = r + p[0];
                    } else {
                        x = p[0];
                    }
                    list = dmgr_pt_sprite_def(list, *k, x, p[1]);
                    if (e->kind == 1) {
                        list = dmgr_pt_draw_face(list, 3, e->kind, e->type2, (short)(x + 3), (short)(dy + p[1] + 3));
                    }
                }
            }
        } else {
            for (i = 0; i < 4; i++, p += 2, k++, e++) {
                r = dmgr_pt_flip(mode, i);
                if (r != 0) {
                    x = r + p[0];
                } else {
                    x = p[0];
                }
                list = dmgr_pt_sprite_def(list, *k, x, (short)(dy + p[1]));
                if (e->kind > 0) {
                    list = dmgr_pt_draw_face(list, mode, e->kind, e->type2, (short)(x + 3), (short)(dy + p[1] + 3));
                }
            }
        }
    }
    return list;
}
#endif

short dmgr_pt_name_pos2[4] = { 0x6E, 0xB6, 0xEF, 0x55 };
short dmgr_pt_name_pos10[4] = { 0x4C, 0xF7, 0x111, 2 };
short dmgr_pt_name_pos4[8] = { 0x5A, 0xC0, 0x5A, 0x5F, 0x103, 0xC0, 0x103, 0x5F };

#ifdef NON_MATCHING /* os vetores de posição do original são locais contíguos percorridos por um ponteiro só */
unsigned int *dmgr_pt_draw_names(unsigned int *list, int mode)
{
    DmgrPtEnt *e = dmgr_pt->ent;
    int dy = 0;
    int x;
    int r;
    int i;

    if ((unsigned int)(mode - 8) < 2) {
        dy = 10;
    }
    if (mode == 1 || mode == 5 || mode == 8) {
        short *pos = dmgr_pt_name_pos2;
        short *p = pos;

        for (i = 0; i < 2; i++, p += 2, e++) {
            r = dmgr_pt_flip(mode, i);
            if (r != 0) {
                x = r + p[0];
            } else {
                x = p[0];
            }
            list = dmgr_pt_sprite_def(list, 4, (short)x, (short)(dy + p[1]));
            if (e->kind > 0) {
                list = dmgr_pt_draw_name(list, e, x + 0x12, dy + p[1] + 5, 0x6D, 0);
            }
        }
    } else if (mode == 10) {
        short *pos = dmgr_pt_name_pos10;
        short *p = pos;

        for (i = 0; i < 2; i++, p += 2, e++) {
            r = dmgr_pt_flip(10, i);
            if (r != 0) {
                x = r + p[0];
            } else {
                x = p[0];
            }
            if (e->kind > 0) {
                list = dmgr_pt_draw_name(list, e, x + 0x12, p[1] + 5, 0x6D, 0);
            }
        }
    } else {
        short *pos = dmgr_pt_name_pos4;
        short *p = pos;

        if (mode == 3) {
            for (i = 0; i < 4; i++, p += 2, e++) {
                if (i % 2 == 0) {
                    r = dmgr_pt_flip(3, i);
                    if (r != 0) {
                        x = r + p[0];
                    } else {
                        x = p[0];
                    }
                    list = dmgr_pt_sprite_def(list, 4, (short)x, (short)(dy + p[1]));
                    if (e->kind == 1) {
                        list = dmgr_pt_draw_name(list, e, x + 0x12, dy + p[1] + 5, 0x6D, 0);
                    }
                }
            }
        } else {
            for (i = 0; i < 4; i++, p += 2, e++) {
                r = dmgr_pt_flip(mode, i);
                if (r != 0) {
                    x = r + p[0];
                } else {
                    x = p[0];
                }
                list = dmgr_pt_sprite_def(list, 4, (short)x, (short)(dy + p[1]));
                if (mode != 3 && e->kind > 0) {
                    list = dmgr_pt_draw_name(list, e, x + 0x12, dy + p[1] + 5, 0x6D, 0);
                }
            }
        }
    }
    return list;
}
#endif

#ifdef NON_MATCHING /* o teste do tipo do original cai no epílogo comum por outro caminho */
unsigned int *dmgr_pt_draw_name(unsigned int *list, DmgrPtEnt *e, int x, int y, int w, int small)
{
    void *name = 0;

    if (e->kind <= 0) {
        return list;
    }
    if (small == 0) {
        font_set_size(0xE, 0xE);
        font_set_color(0xFF000000);
        font_set_flag4(1);
    } else {
        font_set_size(0xC, 0xC);
        font_set_color(0xFFFFFFFF);
        font_set_flag4(1);
    }
    if (e->kind == 1) {
        if (e->name != 0) {
            name = e->name;
        }
    } else {
        name = chara_get_name(0);
    }
    return dmgr_text_draw(list, x, y, w, name, 2);
}
#endif

short dmgr_pt_deck_pos2[4] = { 0x6E, 0xCE, 0xDF, 0x6D };
short dmgr_pt_deck_spr2[2] = { 3, 2 };
short dmgr_pt_deck_pos4[8] = { 0x5A, 0xD8, 0x5A, 0x77, 0xF3, 0xD8, 0xF3, 0x77 };
short dmgr_pt_deck_spr4[2] = { 3, 2 };

#ifdef NON_MATCHING /* os vetores de posição do original são locais contíguos percorridos por um ponteiro só */
unsigned int *dmgr_pt_draw_decks(unsigned int *list, int mode)
{
    DmgrPtEnt *e = dmgr_pt->ent;
    int dy = 0;
    int x;
    int r;
    int i;

    if ((unsigned int)(mode - 8) < 2) {
        dy = 10;
    }
    if (mode == 1 || mode == 5 || mode == 8) {
        short *pos = dmgr_pt_deck_pos2;
        short *spr = dmgr_pt_deck_spr2;
        short *p = pos;
        short *k = spr;

        for (i = 0; i < 2; i++, p += 2, k++, e++) {
            r = dmgr_pt_flip(mode, i);
            if (r != 0) {
                x = r + p[0];
            } else {
                x = p[0];
            }
            list = dmgr_pt_sprite_def(list, *k, (short)x, (short)(dy + p[1]));
            if (e->kind > 0) {
                list = dmgr_pt_draw_deck(list, i, e, x + 0x12, dy + p[1] + 5, 0x7B);
            }
        }
    } else if (mode != 10) {
        short *spr = dmgr_pt_deck_spr4;
        short *pos = dmgr_pt_deck_pos4;
        short *p = pos;

        if (mode == 3) {
            for (i = 0; i < 4; i++, p += 2, e++) {
                if (i % 2 == 0) {
                    r = dmgr_pt_flip(3, i);
                    if (r != 0) {
                        x = r + p[0];
                    } else {
                        x = p[0];
                    }
                    list = dmgr_pt_sprite_def(list, spr[0], (short)x, (short)(dy + p[1]));
                    if (e->kind == 1) {
                        list = dmgr_pt_draw_deck(list, i, e, x + 0x12, dy + p[1] + 5, 0x7B);
                    }
                }
            }
        } else {
            for (i = 0; i < 4; i++, p += 2, e++) {
                r = dmgr_pt_flip(mode, i);
                if (r != 0) {
                    x = r + p[0];
                } else {
                    x = p[0];
                }
                list = dmgr_pt_sprite_def(list, spr[i % 2], (short)x, (short)(dy + p[1]));
                if (mode != 3 && e->kind > 0) {
                    list = dmgr_pt_draw_deck(list, i, e, x + 0x12, dy + p[1] + 5, 0x7B);
                }
            }
        }
    }
    return list;
}
#endif

unsigned short dmgr_pt_no_deck[16] = { '?', '?', '?' };

#ifdef NON_MATCHING /* cadeia de testes do tipo de deck em outra ordem */
unsigned int *dmgr_pt_draw_deck(unsigned int *list, int i, DmgrPtEnt *e, int x, int y, int w)
{
    unsigned short name[32];
    unsigned short *none = dmgr_pt_no_deck;
    int mode;
    int hide = 0;
    void *text;

    font_set_size(0xE, 0xE);
    font_set_color(0xFFFFFFFF);
    font_set_flag4(1);
    mode = dmgr_pt->mode;
    if ((mode == 7 || mode == 6 || mode == 5) && i % 2 != 0) {
        hide = 1;
    }
    if (hide) {
        return dmgr_text_draw(list, x, y, w, none, 2);
    }
    if (e->kind == 0x29 || e->kind == 1) {
        switch (e->src) {
        case 3:
            text = e->deckname;
            break;
        case 2:
            text = e->deckname;
            break;
        case 1:
            return dmgr_text_draw(list, x, y, w, e->deckname, 2);
        case 0:
            if (e->deck != 0) {
                return dmgr_text_draw(list, x, y, w, e->deckname, 2);
            }
            memset(name, 0, sizeof name);
            deck_name_copy_out(name);
            if (name[0] == 0) {
                return list;
            }
            return dmgr_text_draw(list, x, y, w, name, 2);
        default:
            return list;
        }
        return dmgr_text_draw(list, x, y, w, text, 2);
    }
    if (e->src == 3) {
        return dmgr_text_draw(list, x, y, w, e->deckname, 2);
    }
    e->deckname = recipe_get_name(e->deck);
    return dmgr_text_draw(list, x, y, w, e->deckname, 2);
}
#endif

short dmgr_pt_frame_pos[8] = { 0x5A, 0xA8, 0x5A, 0x47, 0x138, 0xA8, 0x138, 0x47 };
short dmgr_pt_frame_spr[4] = { 7, 5, 8, 6 };

#ifdef NON_MATCHING /* os vetores de posição do original são locais contíguos percorridos por um ponteiro só */
unsigned int *dmgr_pt_draw_frames(unsigned int *list, int mode)
{
    short *pos = dmgr_pt_frame_pos;
    short *spr = dmgr_pt_frame_spr;
    short *p;
    short *k = spr;
    short x;
    int r;
    int i;

    if (mode == 3) {
        for (i = 0, p = pos; i < 4; i++, p += 2, k++) {
            if (i % 2 == 0) {
                r = dmgr_pt_flip(3, i);
                if (r != 0) {
                    x = r + p[0];
                } else {
                    x = p[0];
                }
                list = dmgr_pt_sprite_def(list, *k, x, p[1]);
            }
        }
    } else if (mode == 6 || mode == 4 || mode == 2) {
        for (i = 0, p = pos, k = spr; i < 4; i++, p += 2, k++) {
            r = dmgr_pt_flip(mode, i);
            if (r != 0) {
                x = r + p[0];
            } else {
                x = p[0];
            }
            list = dmgr_pt_sprite_def(list, *k, x, p[1]);
        }
    }
    return list;
}
#endif

unsigned int *dmgr_pt_draw_face(unsigned int *list, int mode, unsigned short kind, unsigned short var, short x, short y)
{
    short u;
    short v;

    if (dmgr_pt_face_uv(mode, kind, var, &u, &v) != 0) {
        return dmgr_pt_sprite(list, 0x17, x, y, 0x40, 0x40, u, v, 0x40, 0x40);
    }
    return dmgr_pt_draw_body(list, kind, x, y);
}

unsigned int *dmgr_pt_draw_body(unsigned int *list, unsigned short kind, short x, short y)
{
    DmgrPartner *p = dmgr_pt;
    int sx = x;
    void *face;
    void *body;
    short w;

    if (sx >= -0x40 && sx < 0x1E0) {
        face = ehp_find(p->pack, "kao.gim", 0);
        body = ehp_find(p->pack, "karada.gim", 0);
        w = 0x1E0 - x;
        if (w > 0x40) {
            w = 0x40;
        }
        list = dl_scissor(list, sx, y, w, 0x40);
        list = dl_enable(list, 2);
        list = dmgr_pt_draw_chr(list, kind, sx - 9, y - 0x15, 0xA5, face, body);
        list = dl_disable(list, 2);
    }
    return list;
}

#ifdef NON_MATCHING /* o original lê o modo de rede pelo endereço do campo e põe o retorno 1 em blocos separados */
int dmgr_pt_face_uv(int mode, unsigned short kind, unsigned short var, short *u, short *v)
{
    int f;

    if (kind == 1) {
        if (dmgr_pt->net == 0) {
            *u = 0;
            *v = 0;
        } else {
            *u = var << 6;
            *v = 0x180;
        }
        return 1;
    }
    if (kind == 0x12 && var == 2) {
        var = 1;
    }
    f = dmgr_chr_face(kind, var);
    if (f >= 0) {
        if ((unsigned int)(mode - 5) < 2 && f == 0) {
            *u = var << 6;
            *v = 0x180;
            return 1;
        }
        *u = (f % 8) << 6;
        *v = (f / 8) << 6;
        return 1;
    }
    return 0;
}
#endif

unsigned int *dmgr_pt_sprite_def(unsigned int *list, int i, short x, short y)
{
    return dmgr_pt_sprite(list, i, x, y, -1, -1, -1, -1, -1, -1);
}

unsigned int *dmgr_pt_gfx_setup(unsigned int *list)
{
    list = dl_clear_color(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear_depth(list, 0);
    list = dl_set_tex_filter(list, 0, 0);
    list = dl_enable(list, 9);
    list = dl_disable(list, 1);
    list = dl_disable(list, 10);
    list = dl_enable(list, 4);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    return dl_color(list, 0xFFFFFFFF);
}

char dmgr_pt_gims[10][0x40] = {
    "d_menu01.gim", "d_menu01_j.gim", "d_menu01_e.gim", "d_menu01_g.gim", "d_menu01_f.gim",
    "d_menu01_i.gim", "d_menu01_s.gim", "dmenu_recipe.gim", "recipe_panel.gim", "mdl_bup01.gim",
};
DmgrSpr dmgr_pt_sprites[24] = {
    { 0, 0, { 0, 0, 71, 71 } }, { 0, 0, { 72, 0, 71, 71 } }, { 0, 0, { 0, 72, 148, 23 } },
    { 0, 0, { 0, 96, 148, 23 } }, { 0, 0, { 121, 169, 132, 23 } }, { 0, 0, { 149, 0, 78, 13 } },
    { 0, 0, { 149, 15, 78, 13 } }, { 0, 0, { 149, 30, 78, 13 } }, { 0, 0, { 149, 45, 78, 13 } },
    { 0, 0, { 148, 60, 60, 13 } }, { 0, 0, { 149, 72, 59, 13 } }, { 0, 0, { 149, 85, 59, 13 } },
    { 0, 0, { 121, 192, 135, 38 } }, { 0, 0, { 120, 128, 120, 32 } }, { 0, 0, { 0, 128, 120, 32 } },
    { 0, 0, { 0, 160, 120, 32 } }, { 0, 0, { 0, 192, 120, 32 } }, { 0, 0, { 0, 224, 120, 32 } },
    { 0, 0, { 156, 230, 24, 10 } }, { 0, 0, { 181, 230, 24, 10 } }, { 0, 0, { 206, 230, 24, 10 } },
    { 0, 0, { 231, 230, 24, 10 } }, { 0, 0, { 141, 241, 115, 15 } }, { 9, 0, { 0, 0, 64, 64 } },
};

#ifdef NON_MATCHING /* o original copia os modelos de nomes e sprites para a pilha palavra a palavra */
unsigned int *dmgr_pt_sprite(unsigned int *list, int i, short x, short y, short w, short h, short u, short v,
                             short tw, short th)
{
    DmgrSpr spr[24];
    DmgrPartner *p = dmgr_pt;

    memcpy(spr, dmgr_pt_sprites, sizeof spr);
    list = dmgr_pt_gfx_setup(list);
    if (u < 0) {
        u = spr[i].r[0];
    }
    if (v < 0) {
        v = spr[i].r[1];
    }
    if (tw < 0) {
        tw = spr[i].r[2];
    }
    if (th < 0) {
        th = spr[i].r[3];
    }
    if (w < 0) {
        w = tw;
    }
    if (h < 0) {
        h = th;
    }
    if ((p->mode == 9 || p->mode == 8) && spr[i].file == 0) {
        spr[i].file = cfg_get_language() + 1;
    }
    list = gim_set_texture(list, ehp_find(p->pack, dmgr_pt_gims[spr[i].file], 0));
    list = draw_set_offsets(list, tw, th, w, h);
    list = draw_sprite(list, x, y, 0, u, v, 0, 0);
    if (i == 0x11 || i == 0x10 || i == 0xF || i == 0xE) {
        list = draw_set_offsets(list, 0xC, 0x17, 0xC, 0x17);
        list = draw_sprite(list, (short)(x + w), (short)(y + 9), 0, (short)((i - 0xE) * 0xC + 0x79), 0xC0, 0, 0);
    }
    return list;
}
#endif

void dmgr_anim_stop(DmgrAnim *a)
{
    a->on = 0;
    a->cur = 0;
    a->len = 0;
}

void dmgr_anim_start(DmgrAnim *a, int on, int len)
{
    a->on = on;
    a->cur = 0;
    a->len = len;
}

int dmgr_anim_step(DmgrAnim *a)
{
    if (a->on != 0) {
        if (a->cur < a->len) {
            a->cur++;
            return 0;
        }
        dmgr_anim_stop(a);
    }
    return 1;
}

int dmgr_anim_get(DmgrAnim *a, short *cur, short *len)
{
    *cur = a->cur;
    *len = a->len;
    return a->on;
}

#ifdef NON_MATCHING /* alocação dos parâmetros em $s diferente */
unsigned int *dmgr_text_draw(unsigned int *list, int x, int y, int w, void *text, int align)
{
    int tw;

    if (align == 3) {
        tw = font_print(text);
        if (tw >> 6 < w) {
            x += (w - (tw >> 6)) / 2;
        }
    } else if (align == 1) {
        tw = font_print(text);
        w -= tw >> 6;
        x += w / 2;
        w = tw >> 6;
    }
    if (align == 3 || align == 2) {
        list = dmgr_text_wide(list, x, y, w, text);
    } else if (align == 1 || align == 0) {
        list = dmgr_text_sjis(list, x, y, w, text);
    }
    return list;
}
#endif

unsigned int *dmgr_text_wide(unsigned int *list, int x, int y, int w, void *text)
{
    font_text_set(list);
    font_draw_wtext(x << 6, y << 6, (w + 2) * 0x40, text);
    return font_text_clear();
}

unsigned int *dmgr_text_sjis(unsigned int *list, int x, int y, int w, void *text)
{
    font_text_set(list);
    font_draw_text(x << 6, y << 6, (w + 2) * 0x40, text);
    return font_text_clear();
}

#ifdef NON_MATCHING /* cadeia de testes do personagem e alocação dos temporários de escala diferentes */
unsigned int *dmgr_pt_draw_chr(unsigned int *list, unsigned short kind, int x, int y, int scale, void *face,
                               void *body)
{
    unsigned char m[4];
    int whole = 0;
    int u;
    int v;
    int c;
    int sw;

    if (face == 0) {
        return list;
    }
    if (body == 0) {
        return list;
    }
    c = charmap_lookup(kind, 0);
    u = 0;
    v = 0;
    switch (c) {
    case 1000:
        u = 0x54;
        break;
    case 0x3EF:
        v = 0x33;
        u = 0x54;
        break;
    case 0x3F0:
        v = 0x66;
        u = 0x54;
        break;
    case 0x3F1:
        v = 0x99;
        u = 0x54;
        break;
    case 0x3F2:
        v = 0xCC;
        u = 0x54;
        break;
    case 0x3F3:
        u = 0xA8;
        break;
    case 0x3F4:
        v = 0x33;
        u = 0xA8;
        break;
    default:
        if (charmap_lookup2(c, m) == 0) {
            return list;
        }
        goto draw;
    }
    whole = 1;
draw:
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 1, 1);
    if (whole) {
        list = gim_set_texture(list, body);
        list = draw_set_offsets(list, 0x33, 0x54, scale * 0x33 / 100, scale * 0x54 / 100);
        return draw_sprite(list, x, y - scale * 6 / 100, 0, v, u, 0, 0);
    }
    list = gim_set_texture(list, body);
    sw = scale * 0x33 / 100;
    list = draw_set_offsets(list, 0x33, 0x40, sw, (scale << 6) / 100);
    list = draw_sprite(list, x, y + (scale << 4) / 100, 0, m[1] * 0x33, 0, 0, 0);
    list = gim_set_texture_sub(list, face, 0, 0, 0, m[2] & 0xF);
    list = draw_set_offsets(list, 0x33, 0x32, sw, scale * 0x32 / 100);
    return draw_sprite(list, x, y, 0, m[0] % 5 * 0x33, m[0] / 5 * 0x32, 0, 0);
}
#endif

DmgrChr *dmgr_chr_find(unsigned short id)
{
    DmgrChr *c;
    int lo = 0;
    int hi = 0x28;
    int mid;
    int k;

    do {
        mid = (lo + hi) / 2;
        c = &dmgr_chr_tab[mid];
        k = c->id;
        if (k == id) {
            return c;
        }
        if (k < id) {
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    } while (lo <= hi);
    return 0;
}

int dmgr_chr_face(unsigned short id, unsigned short var)
{
    DmgrChr *c = dmgr_chr_find(id);

    if (c == 0) {
        return -1;
    }
    if (var > 0 && var >= c->nvar) {
        return -1;
    }
    return c->face + var;
}

DmgrChr dmgr_chr_tab[41] = {
    { 0, 0, 0, 0, 0, { "", "", "" }, -1, 0 },
    { 1, 0, 0, 0, 0, { "", "", "" }, 0, 0 },
    { 2, 1, 1, 0, 0, { "jdi", "", "" }, 1, 0 },
    { 3, 1, 1, 0, 0, { "sho", "", "" }, 2, 0 },
    { 4, 1, 1, 0, 0, { "hyt", "", "" }, 3, 0 },
    { 5, 3, 1, 0, 0, { "ask", "", "" }, 4, 0 },
    { 6, 1, 2, 1, 0, { "mab", "maa", "" }, 5, 0 },
    { 7, 2, 1, 0, 0, { "mis", "", "" }, 7, 0 },
    { 8, 3, 1, 0, 0, { "ryo", "", "" }, 8, 0 },
    { 9, 4, 1, 0, 0, { "cro", "", "" }, 9, 0 },
    { 10, 4, 1, 0, 0, { "dai", "", "" }, 10, 0 },
    { 11, 3, 2, 0, 0, { "fba", "fbb", "" }, 11, 0 },
    { 12, 5, 1, 0, 0, { "cmu", "", "" }, 13, 0 },
    { 13, 5, 1, 0, 0, { "tny", "", "" }, 14, 0 },
    { 14, 5, 2, 0, 0, { "zrg", "zmg", "" }, 15, 0 },
    { 15, 5, 1, 0, 0, { "ad3", "", "" }, 17, 0 },
    { 16, 5, 2, 0, 0, { "tiy", "tit", "" }, 18, 0 },
    { 17, 5, 1, 0, 0, { "amu", "", "" }, 20, 0 },
    { 18, 6, 3, 1, 0, { "kgy", "kgo", "kgw" }, 21, 0 },
    { 19, 3, 1, 0, 0, { "tai", "", "" }, 24, 0 },
    { 20, 0, 1, 0, 0, { "mtg", "", "" }, -1, 0 },
    { 21, 3, 1, 0, 0, { "jun", "", "" }, 26, 0 },
    { 22, 3, 1, 0, 0, { "moe", "", "" }, 27, 0 },
    { 23, 0, 1, 0, 0, { "sal", "", "" }, -1, 0 },
    { 24, 1, 2, 1, 0, { "rei", "rec", "" }, 29, 0 },
    { 25, 6, 1, 0, 0, { "ani", "", "" }, 31, 0 },
    { 26, 6, 1, 0, 0, { "oto", "", "" }, 32, 0 },
    { 27, 6, 1, 0, 0, { "bmg", "", "" }, 33, 0 },
    { 28, 6, 1, 0, 0, { "sas", "", "" }, 34, 0 },
    { 29, 0, 1, 0, 0, { "kmz", "", "" }, -1, 0 },
    { 30, 6, 1, 0, 0, { "sei", "", "" }, 36, 0 },
    { 31, 6, 2, 0, 0, { "tob", "tom", "" }, 37, 0 },
    { 32, 6, 1, 0, 0, { "cho", "", "" }, 39, 0 },
    { 33, 6, 1, 0, 0, { "shg", "", "" }, 40, 0 },
    { 34, 0, 0, 0, 0, { "", "", "" }, -1, 0 },
    { 35, 0, 1, 0, 0, { "kni", "", "" }, -1, 0 },
    { 36, 4, 1, 0, 0, { "ayu", "", "" }, 43, 0 },
    { 37, 6, 1, 0, 0, { "boy", "", "" }, 44, 0 },
    { 38, 6, 1, 0, 0, { "sra", "", "" }, 45, 0 },
    { 39, 0, 1, 0, 0, { "sam", "", "" }, -1, 0 },
    { 40, 5, 1, 0, 0, { "daa", "", "" }, 47, 0 },
};
