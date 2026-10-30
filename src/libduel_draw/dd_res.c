// @range 0x08804000-0x08804788
// @data 0x08872BE0-0x08874410
/* Resource tables of the duel scene: the packs (basic/cutin EHPs loaded on demand by region and
   language) and the models, textures, EHF animations and string tables found inside them. */
#include "dd_d1.h"

DdModelRes dd_models[0x26] = {
    { 0x10001, 0x80001, "duel-bg-fmagic05-base.tms", 0, "duel-bg-fmagic05-base.tma", 0 },
    { 0x10002, 0x80001, "field-duelsheet01-base.tms", 0, 0, 0 },
    { 0x10003, 0x80001, "field_card-yamahuda01-base.tms", "field_card-yamahuda01-base.tmt", 0, 0 },
    { 0x10008, 0x80003, "cutin-chara-aska01-base.tms", "cutin_aska01-ptrs.txt", 0, 0 },
    { 0x10009, 0x80001, "field_card-single02h-base.tms", "field_card-single02h-ptrs.txt", 0, 0 },
    { 0x1000C, 0x80001, "field-cursor011-base.tms", 0, 0, 0 },
    { 0x1000D, 0x80001, "field-cursor012-base.tms", 0, 0, 0 },
    { 0x1000E, 0x80001, "field-cursor013-base.tms", 0, 0, 0 },
    { 0x1000F, 0x80001, "field-cursor014-base.tms", 0, 0, 0 },
    { 0x10010, 0x80001, "field-cursor015-base.tms", 0, 0, 0 },
    { 0x10011, 0x80001, "field-cursor016-base.tms", 0, 0, 0 },
    { 0x10012, 0x80001, "field-cursor017-base.tms", 0, 0, 0 },
    { 0x10013, 0x80001, "field-cursor018-base.tms", 0, 0, 0 },
    { 0x10014, 0x80001, "field-cursor021-base.tms", 0, 0, 0 },
    { 0x10015, 0x80001, "field-cursor022-base.tms", 0, 0, 0 },
    { 0x10016, 0x80001, "field-cursor023-base.tms", 0, 0, 0 },
    { 0x10017, 0x80001, "field-cursor024-base.tms", 0, 0, 0 },
    { 0x10018, 0x80001, "field-cursor025-base.tms", 0, 0, 0 },
    { 0x10019, 0x80001, "field-cursor026-base.tms", 0, 0, 0 },
    { 0x1001A, 0x80001, "field-cursor027-base.tms", 0, 0, 0 },
    { 0x1001B, 0x80001, "field-cursor028-base.tms", 0, 0, 0 },
    { 0x1001C, 0x80001, "sord01.tms", 0, 0, 0 },
    { 0x1001D, 0x80001, "coin01.tms", "coin01_anm.tmt", "coin01_anm.tma", 0 },
    { 0x1001E, 0x80001, "dice01.tms", "dice01_anm1.tmt", "dice01_anm1.tma", 0 },
    { 0x1001F, 0x80001, "dice01.tms", "dice01_anm2.tmt", "dice01_anm2.tma", 0 },
    { 0x10020, 0x80001, "field_card-shuffle01-base.tms", "field_card-shuffle01-base.tmt", 0, 0 },
    { 0x10021, 0x80001, "coin01.tms", "coin01.tmt", 0, 0 },
    { 0x10022, 0x80001, "coin01.tms", "coin01.tmt", 0, 0 },
    { 0x10023, 0x80001, "coin01.tms", "coin01.tmt", 0, 0 },
    { 0x10024, 0x80001, "coin01.tms", "coin01.tmt", 0, 0 },
    { 0x10025, 0x80001, "coin01.tms", "coin01.tmt", 0, 0 },
    { 0x10026, 0x80001, "dice01.tms", "dice01.tmt", 0, 0 },
    { 0x10027, 0x80001, "dice01.tms", "dice01.tmt", 0, 0 },
    { 0x10028, 0x80001, "dice01.tms", "dice01.tmt", 0, 0 },
    { 0x10029, 0x80001, "dice01.tms", "dice01.tmt", 0, 0 },
    { 0x1002A, 0x80001, "dice01.tms", "dice01.tmt", 0, 0 },
    { 0x10100, 0x80003, "cutin-chara-aska01-base.tms", "cutin_aska01-ptrs.txt", 0, 0 },
    { 0x10101, 0x80004, "cutin-chara-jyudai01-base.tms", "cutin_jyudai01-ptrs.txt", 0, 0 },
};
DdPack dd_packs[0x4] = {
    { 0x80001, {
        { "basic_jj", 0, 0, 0, 0, 0 },
        { 0, "basic_ue", "basic_ug", "basic_uf", "basic_ui", "basic_us" },
        { 0, "basic_ge", "basic_gg", "basic_gf", "basic_gi", "basic_gs" },
        { 0, "basic_fe", "basic_fg", "basic_ff", "basic_fi", "basic_fs" },
        { 0, "basic_ie", "basic_ig", "basic_if", "basic_ii", "basic_is" },
        { 0, "basic_se", "basic_sg", "basic_sf", "basic_si", "basic_ss" },
        { 0, "basic_be", "basic_bg", "basic_bf", "basic_bi", "basic_bs" }
    }, 0, 0 },
    { 0x80002, {
        { "cutin_cmn", 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 }
    }, 0, 0 },
    { 0x80003, {
        { "cutin_aska01", 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 }
    }, 0, 0 },
    { 0x80004, {
        { "cutin_jyudai01", 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0 }
    }, 0, 0 },
};
DdGimRes dd_gims[0x23] = {
    { 0x40001, 0x80001, "cil_0000.gim", 0 },
    { 0x40002, 0x80001, "cim_0000.gim", 0 },
    { 0x40003, 0x80001, "cis_0000.gim", 0 },
    { 0x40004, 0x80001, "card_icon01.gim", 0 },
    { 0x40005, 0x80001, "card_icon02.gim", 0 },
    { 0x40006, 0x80001, "card_icon03.gim", 0 },
    { 0x4001B, 0x80001, "card_icon06.gim", 0 },
    { 0x40007, 0x80001, "card_frame01.gim", 0 },
    { 0x40008, 0x80001, "card_frame02.gim", 0 },
    { 0x40009, 0x80001, "card_frame03.gim", 0 },
    { 0x40010, 0x80001, "card_frame04.gim", 0 },
    { 0x40011, 0x80001, "card_frame05.gim", 0 },
    { 0x40012, 0x80001, "card_frame06.gim", 0 },
    { 0x40013, 0x80001, "card_frame07.gim", 0 },
    { 0x40014, 0x80001, "card_frame08.gim", 0 },
    { 0x40015, 0x80001, "card_frame09.gim", 0 },
    { 0x40016, 0x80001, "card_frame10.gim", 0 },
    { 0x40017, 0x80001, "window01.gim", 0 },
    { 0x40018, 0x80001, "card_spell_trap.gim", 0 },
    { 0x40019, 0x80001, "card_frame_all.gim", 0 },
    { 0x4001A, 0x80001, "dialog_tab0.gim", 0 },
    { 0x4001C, 0x80001, "card_spec_bg.gim", 0 },
    { 0x4001D, 0x80001, "hand_icon01.gim", 0 },
    { 0x4001E, 0x80001, "duel_thing01.gim", 0 },
    { 0x4001F, 0x80001, "duel_bup_frame01.gim", 0 },
    { 0x40020, 0x80001, "help_menu01.gim", 0 },
    { 0x40021, 0x80001, "field_cursor02.gim", 0 },
    { 0x40022, 0x80001, "field_cursor03.gim", 0 },
    { 0x40023, 0x80001, "field_cursor04.gim", 0 },
    { 0x40024, 0x80001, "card_spec01.gim", 0 },
    { 0x40025, 0x80001, "phase_icon01.gim", 0 },
    { 0x40026, 0x80001, "s_cut01.gim", 0 },
    { 0x40027, 0x80001, "field_cursor05.gim", 0 },
    { 0x40028, 0x80001, "field_cursor06.gim", 0 },
    { 0x4FFFF, 0x80001, "Image1.gim", 0 },
};
DdEhfRes dd_ehfs[0xd] = {
    { 0x20001, 0x80001, "score-atkdef10.ehf", "score-atkdef10.tmf", 0 },
    { 0x20002, 0x80001, "score-top10.ehf", "score-top10.tmf", 0 },
    { 0x20003, 0x80001, "score-phase10.ehf", 0, 0 },
    { 0x20004, 0x80001, "start_duel10.ehf", 0, 0 },
    { 0x20005, 0x80001, "duel_win10.ehf", "duel_win10.tmf", 0 },
    { 0x20006, 0x80001, "duel_lose10.ehf", 0, 0 },
    { 0x20007, 0x80001, "duel_draw10.ehf", 0, 0 },
    { 0x20008, 0x80001, "next_player10.ehf", 0, 0 },
    { 0x20009, 0x80001, "phase_t_10.ehf", "phase_t_10.tmf", 0 },
    { 0x2000A, 0x80002, "ryu_sen01.ehf", 0, 0 },
    { 0x2000B, 0x80002, "ryu_sen02.ehf", 0, 0 },
    { 0x2000C, 0x80001, "chain01.ehf", "chain01.tmf", 0 },
    { 0x2000D, 0x80001, "dilect_atk01.ehf", 0, 0 },
};
DdGimRes dd_bins[0x23] = {
    { 0x80001, 0x80001, "ddstrtbl_jpn_idx.bin", 0 },
    { 0x80002, 0x80001, "ddstrtbl_eng_idx.bin", 0 },
    { 0x80003, 0x80001, "ddstrtbl_ger_idx.bin", 0 },
    { 0x80004, 0x80001, "ddstrtbl_fra_idx.bin", 0 },
    { 0x80005, 0x80001, "ddstrtbl_ita_idx.bin", 0 },
    { 0x80006, 0x80001, "ddstrtbl_spa_idx.bin", 0 },
    { 0x80007, 0x80001, "ddstrtbl_jpn_body.bin", 0 },
    { 0x80008, 0x80001, "ddstrtbl_eng_body.bin", 0 },
    { 0x80009, 0x80001, "ddstrtbl_ger_body.bin", 0 },
    { 0x8000A, 0x80001, "ddstrtbl_fra_body.bin", 0 },
    { 0x8000B, 0x80001, "ddstrtbl_ita_body.bin", 0 },
    { 0x8000C, 0x80001, "ddstrtbl_spa_body.bin", 0 },
    { 0x8000D, 0x80001, "ddstrtbl_j.bin", 0 },
    { 0x8000E, 0x80001, "ddstrtbl_e.bin", 0 },
    { 0x8000F, 0x80001, "ddstrtbl_g.bin", 0 },
    { 0x80010, 0x80001, "ddstrtbl_f.bin", 0 },
    { 0x80011, 0x80001, "ddstrtbl_i.bin", 0 },
    { 0x80012, 0x80001, "ddstrtbl_s.bin", 0 },
    { 0x80013, 0x80001, "phase_select-ptrs.txt", 0 },
    { 0x80014, 0x80001, "coin01_anm-etc.tmt", 0 },
    { 0x80015, 0x80001, "dice01_anm1-etc.tmt", 0 },
    { 0x80016, 0x80001, "dice01_anm2-etc.tmt", 0 },
    { 0x80017, 0x80001, "help_strtbl_contents_duel_j.bin", 0 },
    { 0x80018, 0x80001, "help_strtbl_contents_duel_e.bin", 0 },
    { 0x80019, 0x80001, "help_strtbl_contents_duel_g.bin", 0 },
    { 0x8001A, 0x80001, "help_strtbl_contents_duel_f.bin", 0 },
    { 0x8001B, 0x80001, "help_strtbl_contents_duel_i.bin", 0 },
    { 0x8001C, 0x80001, "help_strtbl_contents_duel_s.bin", 0 },
    { 0x8001E, 0x80001, "help_strtbl_label_j.bin", 0 },
    { 0x8001F, 0x80001, "help_strtbl_label_e.bin", 0 },
    { 0x80020, 0x80001, "help_strtbl_label_g.bin", 0 },
    { 0x80021, 0x80001, "help_strtbl_label_f.bin", 0 },
    { 0x80022, 0x80001, "help_strtbl_label_i.bin", 0 },
    { 0x80023, 0x80001, "help_strtbl_label_s.bin", 0 },
    { 0x80024, 0x80001, "ocg_pack-ptrs.txt", 0 },
};

/* sorts the resource tables by id for dd_res_find */
void dd_res_sort(void)
{
    qsort(dd_models, 0x26, sizeof(DdModelRes), dd_res_cmp);
    qsort(dd_packs, 4, sizeof(DdPack), dd_res_cmp);
    qsort(dd_gims, 0x23, sizeof(DdGimRes), dd_res_cmp);
    qsort(dd_ehfs, 0xD, sizeof(DdEhfRes), dd_res_cmp);
}

void dd_res_free_all(void);
void dd_cards_term(void);
void dd_cutin_free(void);
void dd_res_release(void);

/* frees every resource and waits for the reads in flight */
void dd_res_term(void)
{
    dd_res_free_all();
    dd_cards_term();
    dd_cutin_free();
    dd_res_release();
    while (ehfile_busy())
        sys_frame_wait();
}

/* frees the models and the backgrounds (a background still loading is freed by its callback) */
void dd_res_release(void)
{
    unsigned int i;
    DdModelRes *m;
    DdBgSlot *b;
    int j;

    for (i = 0, m = dd_models; i < 0x26; i++, m++) {
        if (m->model) {
            model_free(m->model);
            m->model = 0;
        }
    }
    for (b = dd_bg.slot, j = 0; j < 2; j++, b++) {
        if (b->used) {
            if (b->busy)
                b->release = 1;
            else
                memset(b, 0, sizeof(DdBgSlot));
        }
    }
}

/* frees the models, the packs and the textures loaded outside a pack */
void dd_res_free_all(void)
{
    unsigned int i;
    DdModelRes *m;
    DdPack *p;
    DdGimRes *g;
    DdEhfRes *e;

    for (i = 0, m = dd_models; i < 0x26; i++, m++) {
        if (m->model) {
            model_free(m->model);
            m->model = 0;
        }
    }
    for (i = 0, p = dd_packs; i < 4; i++, p++) {
        if (p->buf) {
            dd_free(p->buf);
            p->buf = 0;
        }
    }
    for (i = 0, g = dd_gims; i < 0x23; i++, g++) {
        if (g->pack == -1)
            dd_free(g->data);
        g->data = 0;
    }
    i = 0;
    e = dd_ehfs;
    do {
        e->data = 0;
        e++;
    } while (++i < 0xD);
}

/* model `id`, loaded from its pack on first use (0 while the pack loads) */
Model *dd_model_get(int id)
{
    int i = dd_res_find(id, dd_models, 0x26, sizeof(DdModelRes));
    DdModelRes *m;
    void *pack;
    void *tms;
    int *anim;
    void *tma;

    if (i < 0)
        return 0;
    m = &dd_models[i];
    if (m->model)
        return m->model;
    if (m->pack == -1)
        return 0;
    pack = dd_pack_get(m->pack);
    if (pack == 0)
        return 0;
    tms = ehp_find(pack, m->tms, 0);
    if (tms == 0)
        return 0;
    anim = m->anim == 0 ? 0 : ehp_find(pack, m->anim, 0);
    tma = m->tma == 0 ? 0 : ehp_find(pack, m->tma, 0);
    if (anim == 0) {
        m->model = model_load(tms, 0);
    } else if (*anim != 0x544D4754) {  /* "TGMT" */
        m->model = model_load(tms, (void **)anim);
    } else {
        m->model = model_load(tms, 0);
        model_anim_set(m->model, anim);
    }
    if (tma)
        model_set_tgma(m->model, tma);
    return m->model;
}

/* EHF animation `id` (with its texture flags) */
void *dd_ehf_get(int id)
{
    int i = dd_res_find(id, dd_ehfs, 0xD, sizeof(DdEhfRes));
    DdEhfRes *e;
    void *pack;
    void *tmf;

    if (i < 0)
        return 0;
    e = &dd_ehfs[i];
    if (e->data)
        return e->data;
    pack = dd_pack_get(e->pack);
    if (pack == 0)
        return 0;
    e->data = ehp_find(pack, e->ehf, 0);
    if (e->tmf) {
        tmf = ehp_find(pack, e->tmf, 0);
        if (tmf)
            ehf_tex_or_flags(e->data, tmf);
    }
    return e->data;
}

/* texture `id` */
void *dd_gim_get(int id)
{
    int i = dd_res_find(id, dd_gims, 0x23, sizeof(DdGimRes));
    DdGimRes *g;
    void *pack;

    if (i < 0)
        return 0;
    g = &dd_gims[i];
    if (g->data)
        return g->data;
    pack = dd_pack_get(g->pack);
    if (pack == 0)
        return 0;
    g->data = ehp_find(pack, g->name, 0);
    return g->data;
}

/* string table or other binary `id` */
void *dd_bin_get(int id)
{
    int i = dd_res_find(id, dd_bins, 0x23, sizeof(DdGimRes));
    DdGimRes *g;
    void *pack;

    if (i < 0)
        return 0;
    g = &dd_bins[i];
    if (g->data)
        return g->data;
    pack = dd_pack_get(g->pack);
    if (pack == 0)
        return 0;
    g->data = ehp_find(pack, g->name, 0);
    return g->data;
}

int dd_pack_loaded(DdPack *p, int size);

/* pack `id`: starts the read of <name>.ehp (name by region and language) and returns 0 until it ends */
void *dd_pack_get(int id)
{
    char path[0x80];
    int i = dd_res_find(id, dd_packs, 4, sizeof(DdPack));
    DdPack *p;
    int region;
    int lang;
    const char *name;
    int size;

    if (i < 0)
        return 0;
    p = &dd_packs[i];
    if (p->loading)
        return 0;
    if (p->buf)
        return p->buf;
    region = region_get();
    lang = cfg_get_language();
    name = p->name[region][lang];
    if (name == 0)
        name = p->name[0][0];
    strcpy(path, name);
    strcat(path, ".ehp");
    size = ehfile_get_size(4, path);
    p->buf = dd_alloc(size);
    p->loading = 1;
    ehfile_read_async_alias(4, path, p->buf, size, (int (*)(void *, int))dd_pack_loaded, p);
    return 0;
}

/* export: large card images (CIP) */
void *dd_cards_cip_h(void)
{
    return dd_cards.cip_h;
}

/* sets the texture of card `id` (medium image), or the generic card back */
void *dd_card_texture(void *list, int id)
{
    DdCards *c = &dd_cards;
    int loaded;

    if (id > 0)
        list = cip_get_image(list, c->cip_m, id, inv_get_hi(id), &loaded);
    if (id <= 0 || loaded == 0)
        list = gim_set_texture(list, dd_gim_get(0x40002));
    return list;
}

