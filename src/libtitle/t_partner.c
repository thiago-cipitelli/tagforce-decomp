// @range 0x0881F02C-0x08821D3C
// @asm 0x0881F6E0-0x0881F724
// @asm 0x0881F744-0x0881FB00
// @asm 0x0881FB00-0x0881FD30
// @asm 0x0881FD30-0x0881FF2C
// @asm 0x0881FF2C-0x088200C4
// @asm 0x088200C4-0x08820150
// @asm 0x08820150-0x08820188
// @asm 0x08820188-0x088201C0
// @asm 0x08820250-0x0882069C
// @asm 0x0882069C-0x08820A4C
// @asm 0x08820A4C-0x08820B48
// @asm 0x08820B48-0x08820EDC
// @asm 0x08820EDC-0x08821198
// @asm 0x08821198-0x08821340
// @asm 0x08821504-0x08821630
// @asm 0x088216FC-0x08821A14
// @asm 0x08821A9C-0x08821BEC
// @data 0x0882F398-0x08830140
// @rodata 0x0882B40C-0x0882B47C
/* Partner / versus panel of the duel setup (the same code as libduel_mgr's dmgr_partner.c, with a
   few small extras): who plays with and against whom (faces, names, decks), the slide-in animations
   of its EHF layers and the small text and sprite helpers it draws with. */
#include "libtitle.h"

typedef struct TPtSpr {               /* a sprite of the panel textures, 10 bytes */
    unsigned char file;                /* index into the gim names, 0 = by language */
    unsigned char pad;
    short r[4];                        /* u, v, w, h */
} TPtSpr;

typedef struct TChr {               /* face sprites of a character, 0x18 */
    unsigned short id : 10;
    unsigned short grp : 3;
    unsigned short nvar : 2;
    unsigned short b15 : 1;
    unsigned short pad;
    const char *code[3];
    int face;                          /* +0x10 first face index, -1 none */
    int pad14;
} TChr;

/* &tpt->ent[i]; written as pointer arithmetic, which adds the index first as the original does */
#define PT_ENT(i) ((TPtEnt *)tpt->ent + (i))

/* BOOT */
int ehf_set_pos(void *e, int a, int b);
void *recipe_get_name(int id);
void *chara_get_name(int i);
int charmap_lookup(unsigned short c, int a);
int charmap_lookup2(int c, unsigned char *out);
void *gim_set_texture_sub(void *list, void *g, int a, int b, int c, int d);
int font_print(void *text);

/* this module */
void tpt_setup(int mode);
void tpt_fill(int mode);
void tpt_set(short i, int kind, int type2, int src, int deck);
void tpt_set_name(short i, int kind, int type2, int src, int deck, void *name);
void tpt_set_all(short i, int kind, int type2, int src, int deck, void *name, void *deckname);
void tpt_anim_start(int on, int len);
int tpt_flip(int mode, int i);
void tpt_load(int i, int mode, int from, int to);
void tpt_anim(int i, int mode, int from, int to);
int tpt_frame(int i);
int tpt_set_pos(int i, int a, int b);
void *tpt_draw_ehf(void *list, int i);
void *tpt_draw_faces(void *list, int mode);
void *tpt_draw_names(void *list, int mode);
void *tpt_draw_name(void *list, TPtEnt *e, int x, int y, int w, int small);
void *tpt_draw_decks(void *list, int mode);
void *tpt_draw_deck(void *list, int i, TPtEnt *e, int x, int y, int w);
void *tpt_draw_frames(void *list, int mode);
void *tpt_draw_face(void *list, int mode, unsigned short kind, unsigned short var, short x, short y);
void *tpt_draw_body(void *list, unsigned short kind, short x, short y);
int tpt_face_uv(int mode, unsigned short kind, unsigned short var, short *u, short *v);
void *tpt_sprite_def(void *list, int i, short x, short y);
void *tpt_gfx_setup(void *list);
void *tpt_sprite(void *list, int i, short x, short y, short w, short h, short u, short v,
                             short tw, short th);
void tanim_stop(TAnim *a);
void tanim_start(TAnim *a, int on, int len);
int tanim_step(TAnim *a);
int tanim_get(TAnim *a, short *cur, short *len);
void *title_text_frame(void *list, int x, int y, int w, void *text, int align);
void *title_text_wide(void *list, int x, int y, int w, unsigned short *text);
void *title_text_left(void *list, int x, int y, int w, unsigned short *text);
void *tpt_draw_chr(void *list, unsigned short kind, int x, int y, int scale, void *face,
                               void *body);
TChr *tchr_find(unsigned short id);
int tchr_face(unsigned short id, unsigned short var);
extern TChr tchr_tab[41];

void tpartner_init(void *heap, void *pack, int mode)
{
    TPartner *p;

    p = tpt = heap_alloc(heap, sizeof *p);
    memset(p, 0, sizeof *p);
    p->heap = heap;
    p->pack = pack;
    tpt_setup(mode);
}

void tpartner_free(void)
{
    heap_set_level(tpt->heap, tpt);
}

void tpt_setup(int mode)
{
    TPartner *p = tpt;
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
    if (((TGameHdr *)game_state_get())->kind2 == 2) {
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
        tpt_fill(p->mode);
        p->state = 2;
        if (((TGameHdr *)game_state_get())->kind2 == 2) {
            tpt_load(3, 2, 100, 0x81);
        }
        tpt_load(1, 2, f1, t1);
        tpt_load(2, 2, f2, t2);
        tpt_load(0, 2, 0, 0);
        tpt_anim_start(1, 10);
        break;
    case 10:
        tpt_fill(p->mode);
        break;
    }
}

void tpt_fill(int mode)
{
    unsigned char *net = &tpt->net;
    TGameHdr *gs = (TGameHdr *)game_state_get();
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
                tpt_set_all(i, s->type, s->type2, 3, s->deck_id, s->name, s->deck_name);
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
                    tpt_set_all(i, kind, s->type2, 3, s->deck_id, s->name, s->deck_name);
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
                tpt_set_all(i, s->type, s->type2, 3, s->deck_id, s->name, s->deck_name);
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
            tpt_set_all(k, kind, s->type2, 3, s->deck_id, s->name, s->deck_name);
        }
        break;
    }
}

void tpt_set_mode(int mode)
{
    tpt->mode = mode;
}

void tpt_set(short i, int kind, int type2, int src, int deck)
{
    TPtEnt *e = PT_ENT(i);

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

void tpt_set_name(short i, int kind, int type2, int src, int deck, void *name)
{
    TPtEnt *e = PT_ENT(i);

    tpt_set(i, kind, type2, src, deck);
    if (name != 0) {
        e->name = name;
    }
}

void tpt_set_all(short i, int kind, int type2, int src, int deck, void *name, void *deckname)
{
    TPtEnt *e = PT_ENT(i);

    tpt_set_name(i, kind, type2, src, deck, name);
    if (deckname != 0) {
        e->deckname = deckname;
    }
}

#ifdef NON_MATCHING /* o original soma 16 à base e lê os campos em +4; não reproduzido */
/* copy of the entry i (the last one for i >= 4) */
void tpt_get_ent(TPtEnt *dst, int i)
{
    TPtEnt *e = tpt->ent;

    if (i >= 4)
        i = 3;
    *dst = e[i];
}
#endif

void tpt_anim_start(int on, int len)
{
    tanim_start(&tpt->anim, on, len);
}

int tpartner_update_net(void)
{
    return tpartner_update(1);
}

#ifdef NON_MATCHING /* função grande: o original guarda seis limites de quadro em $s e um na pilha; o mwcc aloca outros */
int tpartner_update(int net)
{
    TPartner *p = tpt;
    int a1 = 0x1D;
    int b2 = 0x3C;
    int a2 = 0x3B;
    TAnim *anim = &p->anim;
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
    if (((TGameHdr *)game_state_get())->kind2 == 2) {
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
        if (f1 == tpt_frame(1)) {
            tpt_anim(1, 2, f1 + 1, t1);
            tpt_anim(2, 2, a1 + 1, a2);
            p->state = 4;
            if (((TGameHdr *)game_state_get())->kind2 == 2) {
                tpt_anim(3, 2, 0x82, 0x9F);
            }
        }
        break;
    case 4:
        if ((unsigned int)(p->mode - 8) < 3) {
            if (tanim_step(anim) != 0) {
                if (p->mode == 9) {
                    tpt_anim(0, 2, 0, 0xE);
                    p->state = 5;
                } else {
                    p->state = 6;
                }
            }
        } else if (tanim_step(anim) != 0) {
            return 1;
        }
        break;
    case 5:
        if (tpt_frame(0) == 0xE) {
            tpt_anim(0, 0, 0xE, 0xE);
            p->state = 6;
        }
        break;
    case 6:
        if ((unsigned int)(p->mode - 8) < 2) {
            pad_get();
            if (net != 0 && p->timer > 0x77) {
                tpt_anim_start(2, 10);
                se_play(0x628A);
                p->state = 7;
                tpt_anim(1, 2, f2, e1);
                tpt_anim(2, 2, b2, b1);
                if (((TGameHdr *)game_state_get())->kind2 == 2) {
                    tpt_anim(3, 2, 0xA0, 0xAA);
                }
            }
            if (p->timer < 0x78) {
                p->timer++;
            }
        }
        break;
    case 7:
        if (((TGameHdr *)game_state_get())->kind2 == 2 && tpt_frame(3) == 0xAA) {
            tpt_anim(3, 2, 0xAA, 0xAA);
        }
        if (e1 == tpt_frame(1)) {
            tpt_anim(1, 0, e1, e1);
            tpt_anim(2, 0, b1, b1);
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
void *tpartner_draw(void *list)
{
    TPartner *p = tpt;
    int from = 0;
    int mode = p->mode;
    int to = 0;
    int on = 0;

    switch (mode) {
    case 8:
    case 9:
        if (p->state > 1) {
            list = tpt_draw_ehf(list, 1);
            list = tpt_draw_ehf(list, 2);
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
        if (from == tpt_frame(1)) {
            se_play(0x628C);
        } else if (to == tpt_frame(1)) {
            se_play(0x628D);
        }
    }
    list = tpt_draw_faces(list, mode);
    list = tpt_draw_names(list, mode);
    list = tpt_draw_decks(list, mode);
    list = tpt_draw_frames(list, mode);
    if (p->mode == 9) {
        tpt_set_pos(0, 0, 10);
        list = tpt_draw_ehf(list, 0);
    }
    if (((TGameHdr *)game_state_get())->kind2 == 2 && (p->mode == 9 || p->mode == 8)) {
        list = tpt_draw_ehf(list, 3);
        if (p->state > 0 && p->state < 2 && fade_apply(&list) != 0) {
            p->state = 2;
        }
    }
    return list;
}
#endif

#ifdef NON_MATCHING /* o original testa os modos numa cadeia de if e os lados num switch esparso; ordem dos blocos diferente */
int tpt_flip(int mode, int i)
{
    short cur;
    short len = 1;
    int on;
    int side;
    int r = 0;

    on = tanim_get(&tpt->anim, &cur, &len);
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

char tpt_names[38][0x40] = {
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
void tpt_load(int i, int mode, int from, int to)
{
    TPartner *p = tpt;
    void **slot = &tpt->ehf[i];
    int n;
    int k = i;

    if (i == 0 && (unsigned int)(tpt->mode - 8) < 2) {
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
    *slot = ehp_find(p->pack, tpt_names[k], 0);
    tpt_anim(i, mode, from, to);
}
#endif

#ifdef NON_MATCHING /* o original reaproveita a0 para o índice multiplicado; o mwcc usa outro temporário */
void tpt_anim(int i, int mode, int from, int to)
{
    void *e = tpt->ehf[i];

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
int tpt_frame(int i)
{
    void *e = tpt->ehf[i];

    if (e != 0) {
        return ehf_get_frame_count(e);
    }
    return -1;
}
#endif

#ifdef NON_MATCHING /* só os temporários do índice trocam */
int tpt_set_pos(int i, int a, int b)
{
    void *e = tpt->ehf[i];

    if (e != 0) {
        return ehf_set_pos(e, a, b);
    }
    return 0;
}
#endif

void *tpt_draw_ehf(void *list, int i)
{
    void **slot = &tpt->ehf[i];
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

short tpt_face_pos2[4] = { 0x25, 0x9E, 0x175, 0x3D };
short tpt_face_spr2[2] = { 1, 0 };
short tpt_face_pos10[4] = { 3, 0xC7, 0x197, 2 };
short tpt_face_pos4[8] = { 0x11, 0xA8, 0x11, 0x47, 0x189, 0xA8, 0x189, 0x47 };
short tpt_face_spr4[4] = { 1, 0, 1, 0 };

#ifdef NON_MATCHING /* os vetores de posição do original são locais contíguos percorridos por um ponteiro só */
void *tpt_draw_faces(void *list, int mode)
{
    TPtEnt *e = tpt->ent;
    short dy = 0;
    short x;
    int r;
    int i;

    if ((unsigned int)(mode - 8) < 2) {
        dy = 10;
    }
    if (mode == 1 || mode == 5 || mode == 8) {
        short *pos = tpt_face_pos2;
        short *spr = tpt_face_spr2;
        short *p = pos;
        short *k = spr;

        for (i = 0; i < 2; i++, p += 2, k++, e++) {
            r = tpt_flip(mode, i);
            if (r != 0) {
                x = r + p[0];
            } else {
                x = p[0];
            }
            list = tpt_sprite_def(list, *k, x, (short)(dy + p[1]));
            if (e->kind > 0) {
                list = tpt_draw_face(list, mode, e->kind, e->type2, (short)(x + 3), (short)(dy + p[1] + 3));
            }
        }
    } else if (mode == 10) {
        short *pos = tpt_face_pos10;
        short *p = pos;

        for (i = 0; i < 2; i++, p += 2, e++) {
            r = tpt_flip(10, i);
            if (r != 0) {
                x = r + p[0];
            } else {
                x = p[0];
            }
            if (e->kind > 0) {
                list = tpt_draw_face(list, 10, e->kind, e->type2, (short)(x + 3), (short)(p[1] + 3));
            }
        }
    } else {
        short *pos = tpt_face_pos4;
        short *spr = tpt_face_spr4;
        short *p = pos;
        short *k = spr;

        if (mode == 3) {
            for (i = 0; i < 4; i++, p += 2, k++, e++) {
                if (i % 2 == 0) {
                    r = tpt_flip(3, i);
                    if (r != 0) {
                        x = r + p[0];
                    } else {
                        x = p[0];
                    }
                    list = tpt_sprite_def(list, *k, x, p[1]);
                    if (e->kind == 1) {
                        list = tpt_draw_face(list, 3, e->kind, e->type2, (short)(x + 3), (short)(dy + p[1] + 3));
                    }
                }
            }
        } else {
            for (i = 0; i < 4; i++, p += 2, k++, e++) {
                r = tpt_flip(mode, i);
                if (r != 0) {
                    x = r + p[0];
                } else {
                    x = p[0];
                }
                list = tpt_sprite_def(list, *k, x, (short)(dy + p[1]));
                if (e->kind > 0) {
                    list = tpt_draw_face(list, mode, e->kind, e->type2, (short)(x + 3), (short)(dy + p[1] + 3));
                }
            }
        }
    }
    return list;
}
#endif

short tpt_name_pos2[4] = { 0x6E, 0xB6, 0xEF, 0x55 };
short tpt_name_pos10[4] = { 0x4C, 0xF7, 0x111, 2 };
short tpt_name_pos4[8] = { 0x5A, 0xC0, 0x5A, 0x5F, 0x103, 0xC0, 0x103, 0x5F };

#ifdef NON_MATCHING /* os vetores de posição do original são locais contíguos percorridos por um ponteiro só */
void *tpt_draw_names(void *list, int mode)
{
    TPtEnt *e = tpt->ent;
    int dy = 0;
    int x;
    int r;
    int i;

    if ((unsigned int)(mode - 8) < 2) {
        dy = 10;
    }
    if (mode == 1 || mode == 5 || mode == 8) {
        short *pos = tpt_name_pos2;
        short *p = pos;

        for (i = 0; i < 2; i++, p += 2, e++) {
            r = tpt_flip(mode, i);
            if (r != 0) {
                x = r + p[0];
            } else {
                x = p[0];
            }
            list = tpt_sprite_def(list, 4, (short)x, (short)(dy + p[1]));
            if (e->kind > 0) {
                list = tpt_draw_name(list, e, x + 0x12, dy + p[1] + 5, 0x6D, 0);
            }
        }
    } else if (mode == 10) {
        short *pos = tpt_name_pos10;
        short *p = pos;

        for (i = 0; i < 2; i++, p += 2, e++) {
            r = tpt_flip(10, i);
            if (r != 0) {
                x = r + p[0];
            } else {
                x = p[0];
            }
            if (e->kind > 0) {
                list = tpt_draw_name(list, e, x + 0x12, p[1] + 5, 0x6D, 0);
            }
        }
    } else {
        short *pos = tpt_name_pos4;
        short *p = pos;

        if (mode == 3) {
            for (i = 0; i < 4; i++, p += 2, e++) {
                if (i % 2 == 0) {
                    r = tpt_flip(3, i);
                    if (r != 0) {
                        x = r + p[0];
                    } else {
                        x = p[0];
                    }
                    list = tpt_sprite_def(list, 4, (short)x, (short)(dy + p[1]));
                    if (e->kind == 1) {
                        list = tpt_draw_name(list, e, x + 0x12, dy + p[1] + 5, 0x6D, 0);
                    }
                }
            }
        } else {
            for (i = 0; i < 4; i++, p += 2, e++) {
                r = tpt_flip(mode, i);
                if (r != 0) {
                    x = r + p[0];
                } else {
                    x = p[0];
                }
                list = tpt_sprite_def(list, 4, (short)x, (short)(dy + p[1]));
                if (mode != 3 && e->kind > 0) {
                    list = tpt_draw_name(list, e, x + 0x12, dy + p[1] + 5, 0x6D, 0);
                }
            }
        }
    }
    return list;
}
#endif

#ifdef NON_MATCHING /* o teste do tipo do original cai no epílogo comum por outro caminho */
void *tpt_draw_name(void *list, TPtEnt *e, int x, int y, int w, int small)
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
    return title_text_frame(list, x, y, w, name, 2);
}
#endif

short tpt_deck_pos2[4] = { 0x6E, 0xCE, 0xDF, 0x6D };
short tpt_deck_spr2[2] = { 3, 2 };
short tpt_deck_pos4[8] = { 0x5A, 0xD8, 0x5A, 0x77, 0xF3, 0xD8, 0xF3, 0x77 };
short tpt_deck_spr4[2] = { 3, 2 };

#ifdef NON_MATCHING /* os vetores de posição do original são locais contíguos percorridos por um ponteiro só */
void *tpt_draw_decks(void *list, int mode)
{
    TPtEnt *e = tpt->ent;
    int dy = 0;
    int x;
    int r;
    int i;

    if ((unsigned int)(mode - 8) < 2) {
        dy = 10;
    }
    if (mode == 1 || mode == 5 || mode == 8) {
        short *pos = tpt_deck_pos2;
        short *spr = tpt_deck_spr2;
        short *p = pos;
        short *k = spr;

        for (i = 0; i < 2; i++, p += 2, k++, e++) {
            r = tpt_flip(mode, i);
            if (r != 0) {
                x = r + p[0];
            } else {
                x = p[0];
            }
            list = tpt_sprite_def(list, *k, (short)x, (short)(dy + p[1]));
            if (e->kind > 0) {
                list = tpt_draw_deck(list, i, e, x + 0x12, dy + p[1] + 5, 0x7B);
            }
        }
    } else if (mode != 10) {
        short *spr = tpt_deck_spr4;
        short *pos = tpt_deck_pos4;
        short *p = pos;

        if (mode == 3) {
            for (i = 0; i < 4; i++, p += 2, e++) {
                if (i % 2 == 0) {
                    r = tpt_flip(3, i);
                    if (r != 0) {
                        x = r + p[0];
                    } else {
                        x = p[0];
                    }
                    list = tpt_sprite_def(list, spr[0], (short)x, (short)(dy + p[1]));
                    if (e->kind == 1) {
                        list = tpt_draw_deck(list, i, e, x + 0x12, dy + p[1] + 5, 0x7B);
                    }
                }
            }
        } else {
            for (i = 0; i < 4; i++, p += 2, e++) {
                r = tpt_flip(mode, i);
                if (r != 0) {
                    x = r + p[0];
                } else {
                    x = p[0];
                }
                list = tpt_sprite_def(list, spr[i % 2], (short)x, (short)(dy + p[1]));
                if (mode != 3 && e->kind > 0) {
                    list = tpt_draw_deck(list, i, e, x + 0x12, dy + p[1] + 5, 0x7B);
                }
            }
        }
    }
    return list;
}
#endif

unsigned short tpt_no_deck[16] = { '?', '?', '?' };

#ifdef NON_MATCHING /* cadeia de testes do tipo de deck em outra ordem */
void *tpt_draw_deck(void *list, int i, TPtEnt *e, int x, int y, int w)
{
    unsigned short name[32];
    unsigned short *none = tpt_no_deck;
    int mode;
    int hide = 0;
    void *text;

    font_set_size(0xE, 0xE);
    font_set_color(0xFFFFFFFF);
    font_set_flag4(1);
    mode = tpt->mode;
    if ((mode == 7 || mode == 6 || mode == 5) && i % 2 != 0) {
        hide = 1;
    }
    if (hide) {
        return title_text_frame(list, x, y, w, none, 2);
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
            return title_text_frame(list, x, y, w, e->deckname, 2);
        case 0:
            if (e->deck != 0) {
                return title_text_frame(list, x, y, w, e->deckname, 2);
            }
            memset(name, 0, sizeof name);
            deck_name_copy_out(name);
            if (name[0] == 0) {
                return list;
            }
            return title_text_frame(list, x, y, w, name, 2);
        default:
            return list;
        }
        return title_text_frame(list, x, y, w, text, 2);
    }
    if (e->src == 3) {
        return title_text_frame(list, x, y, w, e->deckname, 2);
    }
    e->deckname = recipe_get_name(e->deck);
    return title_text_frame(list, x, y, w, e->deckname, 2);
}
#endif

short tpt_frame_pos[8] = { 0x5A, 0xA8, 0x5A, 0x47, 0x138, 0xA8, 0x138, 0x47 };
short tpt_frame_spr[4] = { 7, 5, 8, 6 };

#ifdef NON_MATCHING /* os vetores de posição do original são locais contíguos percorridos por um ponteiro só */
void *tpt_draw_frames(void *list, int mode)
{
    short *pos = tpt_frame_pos;
    short *spr = tpt_frame_spr;
    short *p;
    short *k = spr;
    short x;
    int r;
    int i;

    if (mode == 3) {
        for (i = 0, p = pos; i < 4; i++, p += 2, k++) {
            if (i % 2 == 0) {
                r = tpt_flip(3, i);
                if (r != 0) {
                    x = r + p[0];
                } else {
                    x = p[0];
                }
                list = tpt_sprite_def(list, *k, x, p[1]);
            }
        }
    } else if (mode == 6 || mode == 4 || mode == 2) {
        for (i = 0, p = pos, k = spr; i < 4; i++, p += 2, k++) {
            r = tpt_flip(mode, i);
            if (r != 0) {
                x = r + p[0];
            } else {
                x = p[0];
            }
            list = tpt_sprite_def(list, *k, x, p[1]);
        }
    }
    return list;
}
#endif

void *tpt_draw_face(void *list, int mode, unsigned short kind, unsigned short var, short x, short y)
{
    short u;
    short v;

    if (tpt_face_uv(mode, kind, var, &u, &v) != 0) {
        return tpt_sprite(list, 0x17, x, y, 0x40, 0x40, u, v, 0x40, 0x40);
    }
    return tpt_draw_body(list, kind, x, y);
}

void *tpt_draw_body(void *list, unsigned short kind, short x, short y)
{
    TPartner *p = tpt;
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
        list = tpt_draw_chr(list, kind, sx - 9, y - 0x15, 0xA5, face, body);
        list = dl_disable(list, 2);
    }
    return list;
}

#ifdef NON_MATCHING /* o original lê o modo de rede pelo endereço do campo e põe o retorno 1 em blocos separados */
int tpt_face_uv(int mode, unsigned short kind, unsigned short var, short *u, short *v)
{
    int f;

    if (kind == 1) {
        if (tpt->net == 0) {
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
    f = tchr_face(kind, var);
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

void *tpt_sprite_def(void *list, int i, short x, short y)
{
    return tpt_sprite(list, i, x, y, -1, -1, -1, -1, -1, -1);
}

void *tpt_gfx_setup(void *list)
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

char tpt_gims[10][0x40] = {
    "d_menu01.gim", "d_menu01_j.gim", "d_menu01_e.gim", "d_menu01_g.gim", "d_menu01_f.gim",
    "d_menu01_i.gim", "d_menu01_s.gim", "dmenu_recipe.gim", "recipe_panel.gim", "mdl_bup01.gim",
};
TPtSpr tpt_sprites[24] = {
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
void *tpt_sprite(void *list, int i, short x, short y, short w, short h, short u, short v,
                             short tw, short th)
{
    TPtSpr spr[24];
    TPartner *p = tpt;

    memcpy(spr, tpt_sprites, sizeof spr);
    list = tpt_gfx_setup(list);
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
    list = gim_set_texture(list, ehp_find(p->pack, tpt_gims[spr[i].file], 0));
    list = draw_set_offsets(list, tw, th, w, h);
    list = draw_sprite(list, x, y, 0, u, v, 0, 0);
    if (i == 0x11 || i == 0x10 || i == 0xF || i == 0xE) {
        list = draw_set_offsets(list, 0xC, 0x17, 0xC, 0x17);
        list = draw_sprite(list, (short)(x + w), (short)(y + 9), 0, (short)((i - 0xE) * 0xC + 0x79), 0xC0, 0, 0);
    }
    return list;
}
#endif

void tanim_stop(TAnim *a)
{
    a->on = 0;
    a->cur = 0;
    a->len = 0;
}

void tanim_start(TAnim *a, int on, int len)
{
    a->on = on;
    a->cur = 0;
    a->len = len;
}

int tanim_step(TAnim *a)
{
    if (a->on != 0) {
        if (a->cur < a->len) {
            a->cur++;
            return 0;
        }
        tanim_stop(a);
    }
    return 1;
}

int tanim_get(TAnim *a, short *cur, short *len)
{
    *cur = a->cur;
    *len = a->len;
    return a->on;
}

#ifdef NON_MATCHING /* alocação dos parâmetros em $s diferente */
void *title_text_frame(void *list, int x, int y, int w, void *text, int align)
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
        list = title_text_wide(list, x, y, w, text);
    } else if (align == 1 || align == 0) {
        list = title_text_left(list, x, y, w, text);
    }
    return list;
}
#endif

void *title_text_wide(void *list, int x, int y, int w, unsigned short *text)
{
    font_text_set(list);
    font_draw_wtext(x << 6, y << 6, (w + 2) * 0x40, text);
    return font_text_clear();
}

void *title_text_box(void *list, int x, int y, int w, int h, unsigned short *text)
{
    font_text_set(list);
    font_draw_wtext_ex(x << 6, y << 6, (w + 2) << 6, (h + 2) << 6, text, -1);
    return font_text_clear();
}

void *title_text_left(void *list, int x, int y, int w, unsigned short *text)
{
    font_text_set(list);
    font_draw_text(x << 6, y << 6, (w + 2) * 0x40, text);
    return font_text_clear();
}
