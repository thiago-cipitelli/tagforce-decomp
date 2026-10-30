// @range 0x0881351C-0x088143A4
// @data 0x0887541C-0x088755B8
// @asm 0x08813554-0x08813840
// @asm 0x08813AB8-0x08813C40
// @asm 0x08813CF4-0x08813D48
// @asm 0x08813D8C-0x08813F1C
// @asm 0x088141A4-0x088143A4
/* Duel characters (partner/rival model from the cut-in pack): setup and free, the model with its
   animations (gzip'd TGMT), the voice track, the per-frame animation and draw, bone matrices for
   attaching effects, and the visibility mask of the model parts from the animation tracks. */
#include "dd_d1.h"

void mtx_set_mode(int mode);
void mtx_get34(DdMtx *m);
void mtx_identity(void);
void mtx_get_view_dir(DdXyzw *out);
void mat4_identity(DdMtx *m);
void mat4_mul(DdMtx *out, DdMtx *a, DdMtx *b);
void mat4_translate(DdMtx *out, DdMtx *m, DdXyzw *v);
void quat_to_mat4(DdMtx *out, DdXyzw *q);
int stricmp(const char *a, const char *b);
int gzip_decompress(void *dst, int dst_len, const void *src, int *out_size);
int is_gzip(const void *p);
int model_anim_play(Model *m, int n, unsigned int flags);
void model_update(Model *m, float *dt);
void model_draw(void *m, void *params);
void model_get_bone(void *model, DdMtx *out, int bone);
float model_anim_frame(Model *m);
int model_anim_frame_count(Model *m);
void model_set_mask(Model *m, unsigned int mask);
void *model_hook(Model *m, int which, void *fn);
void *tgm_relocate(void *p);
float keyframe_eval(void *tr, float t);
unsigned int *gfx_get_state(void);
int dl_get_layer(void);
void zone_view_save(int a);
void dd_camera_from_anim(float t, void *anim);
void dd_fx_set_arg4(int on);
void dd_fx_lines_set(int on);
int dd_lp_busy(void);
Model *dd_hook_anim(DdChr *c);
void *dd_cutin_get(int id, int sub);
int dd_disc_tex_hook();
int dd_hook_bone();
int dd_hook_true(DdChr *c);
int dd_hook_8147d0(DdChr *c);

char dd_chr_s_info[] = "_info-ptrs.txt";
char *dd_chr_info = dd_chr_s_info;
char dd_chr_s_all[] = "all-ptrs.txt";
char *dd_chr_all = dd_chr_s_all;
char dd_chr_s_alletc[] = "alletc-ptrs.txt";
char *dd_chr_alletc = dd_chr_s_alletc;
char *dd_chr_words[6] = {
    "chr_wordstbl_j.bin", "chr_wordstbl_e.bin", "chr_wordstbl_g.bin",
    "chr_wordstbl_f.bin", "chr_wordstbl_i.bin", "chr_wordstbl_s.bin",
};
char dd_chr_s_voice[] = "voice_#10000.txt";
char dd_chr_s_voiceid[] = "chr_voiceidchgtbl.bin";
char dd_chr_s_boy_r[] = "DA_boy_R01.iff";
char dd_chr_s_boy_y[] = "DA_boy_Y01.iff";
char dd_chr_s_boy_b[] = "DA_boy_B01.iff";
char dd_chr_s_girl[] = "DA_girl01.iff";
char dd_chr_s_tex[] = "texture_#10500.txt";
char dd_chr_s_disc[] = "duel_disc.gim";
char dd_chr_s_vtmt[] = "voice01.vtmt.gz";

typedef struct DdChrBone {             /* attach point -> model bone */
    int id;
    int bone;
} DdChrBone;

DdChrBone dd_chr_bones[6] = { { 0x25, 0x00 }, { 0x26, 0x1D }, { 0x27, 0x0D },
                              { 0x28, 0x1D }, { 0x29, 0x1E }, { 0x2A, 0x00 } };

extern DdChrMgr dd_chr_mgr;            /* 0x08C544A0 */

void dd_chr_free(DdChr *c, int all);
void dd_chr_anim_set(DdChr *c, int anim);
void dd_chr_voice_load(DdChr *c);
void dd_chr_begin(DdChr *c);
void *dd_chr_model_draw(void *list, DdChr *c);
void dd_chr_mask(DdChr *c, Model *m);
int dd_chr_anim_done(DdChr *c);
void dd_chr_flag_set(DdChr *c, int on);
int dd_chr_anim_cb(DdChr *c, int index, int arg);
int dd_chr_cb_stop(DdChr *c, int index, int a, int b);
int dd_chr_cb_tick(DdChr *c, int index, int frame, int anim_frame, int arg);
int dd_chr_cb_map(DdChr *c, int anim);
int dd_chr_cb_bone(DdChr *c, int bone);
void dd_chr_cb_pos(DdChr *c, DdXyzw *pos, DdXyzw *rot);

void dd_chr_term(void)
{
    dd_chr_free(&dd_chr_mgr.chr, 1);
    memset(&dd_chr_mgr, 0, sizeof(DdChrMgr));
}

typedef struct DdChrDesc {             /* the first 13 words of DdChr, copied from the caller */
    int w[13];
} DdChrDesc;

#ifdef NON_MATCHING /* lógica conferida; o cálculo de voice (ramos com bgezl/bnel no original) e os registradores diferem: 51 palavras */
/* takes the description (13 words) and the voice and word tables of the cut-in pack */
void dd_chr_init(DdChr *c, DdChr *desc)
{
    int voice;
    void *pack;

    memset(c, 0, sizeof(DdChr));
    *(DdChrDesc *)c = *(DdChrDesc *)desc;
    if (charmap_lookup(c->chr, 0) >= 0)
        voice = 1;
    else
        voice = (c->chr == 0x12 && c->sub == 2) ? 1 : 0;
    pack = dd_cutin_get(c->chr, c->sub);
    if (voice)
        pack = ehp_find(pack, dd_chr_s_voice, 0);
    if (c->voice_tbl == 0)
        c->voice_tbl = ehp_find(pack, dd_chr_s_voiceid, 0);
    if (c->words == 0)
        c->words = ehp_find(pack, dd_chr_words[cfg_get_language()], 0);
    c->tmt = 0;
    c->index = -1;
    c->time = 0.0f;
    c->buf = 0;
}
#endif

#ifdef NON_MATCHING /* forma/alocação não reproduzidas */
/* loads the model; the duel academy students take their face texture from the pack */
void dd_chr_load(DdChr *c)
{
    void *pack = dd_cutin_get(c->chr, c->sub);
    int **info = ehp_find(pack, dd_chr_info, 0);
    int *tms = *info;
    void *anims = ehp_find(pack, dd_chr_all, 0);
    void *etc = ehp_find(pack, dd_chr_alletc, 0);
    int i;
    char **tex;

    if (charmap_lookup(c->chr, 0) >= 0) {
        for (i = 0; i < tms[0x34 / 4]; i++) {
            tex = (char **)(tms[0x10 / 4] + i * 0x28);
            if (stricmp(dd_chr_s_boy_r, *tex) == 0 || stricmp(dd_chr_s_boy_y, *tex) == 0 ||
                stricmp(dd_chr_s_boy_b, *tex) == 0 || stricmp(dd_chr_s_girl, *tex) == 0 ||
                stricmp(dd_chr_s_girl, *tex) == 0)
                tex[1] = ehp_find(pack, dd_chr_s_tex, 0);
        }
    }
    c->model = model_load(tms, anims);
    c->anims = etc;
    c->index = -1;
    dd_chr_anim_set(c, 0);
    c->disc = ehp_find(pack, dd_chr_s_disc, 0);
}
#endif

/* frees the model and the decompression buffers */
void dd_chr_free(DdChr *c, int all)
{
    if (c->model) {
        model_free(c->model);
        c->model = 0;
    }
    if (c->buf) {
        dd_free(c->buf);
        c->buf = 0;
    }
    if (c->vbuf) {
        dd_free(c->vbuf);
        c->vbuf = 0;
    }
    memset(c, 0, sizeof(DdChr));
}

void *dd_chr_draw(void *list, DdChr *c)
{
    dd_chr_begin(c);
    return dd_chr_model_draw(list, c);
}

/* per-frame callback of the current animation */
void dd_chr_tick(DdChr *c)
{
    c->x168 = 0;
    if (dd_hook_anim(c))
        dd_chr_cb_tick(c, c->index, (int)(c->time * (float)(int)gfx_get_state()[1]),
                       (int)model_anim_frame(c->model), c->done);
}

/* advances the animation and the voice; 1 when the animation ended */
int dd_chr_update(DdChr *c, float dt)
{
    Model *m = dd_hook_anim(c);
    int on;
    int f0;
    int f1;
    DdModelUpd upd;

    if (m) {
        c->done = 0;
        on = dd_hook_true(c);
        f0 = (int)model_anim_frame(m);
        memset(&upd, 0, sizeof(upd));
        upd.dt = on ? dt * (float)(int)gfx_get_state()[1] : 0.0f;
        upd.cb = dd_hook_bone;
        upd.chr = c->chr;
        upd.c = c;
        model_update(m, (float *)&upd);
        f1 = (int)model_anim_frame(m);
        if (f0 < f1)
            c->done = 1;
        else if (f1 == model_anim_frame_count(m))
            c->done = 1;
    }
    c->time += dt;
    if (c->voice)
        c->vtime += dt;
    if (dd_chr_anim_done(c) != 0)
        return 1;
    return 0;
}

#ifdef NON_MATCHING /* forma/alocação não reproduzidas */
/* plays animation `anim` (from the animation table, gzip'd TGMT decompressed) */
void dd_chr_anim_set(DdChr *c, int anim)
{
    int i;
    void *t;
    int size;
    Model *m;
    float upd[4];

    if (c->index >= 0)
        dd_chr_cb_stop(c, c->index, 1, 0);
    if (c->anims == 0)
        return;
    if (anim == -1) {
        c->tmt = 0;
        c->index = -1;
        c->anim = -1;
        return;
    }
    i = dd_chr_cb_map(c, anim);
    if (i < 0)
        return;
    t = c->anims[i];
    if (t == 0) {
        c->tmt = 0;
    } else {
        if (is_gzip(t)) {
            if (c->buf == 0)
                c->buf = dd_alloc(0x10000);
            gzip_decompress(c->buf, 0x10000, t, &size);
            c->buf = tgm_relocate(c->buf);
            t = c->buf;
        }
        c->tmt = t;
    }
    c->index = i;
    c->anim = anim;
    c->time = 0.0f;
    m = dd_hook_anim(c);
    model_anim_play(m, c->index, 0);
    memset(upd, 0, sizeof(upd));
    model_update(m, upd);
    dd_chr_voice_load(c);
    c->x15c = 0;
    c->x160 = 0;
    c->x16c = 0;
    dd_chr_flag_set(c, 1);
    dd_fx_lines_set(anim == 0x19);
    if (anim == 8)
        c->disc_on = 1;
    else
        c->disc_on = 0;
    dd_chr_anim_cb(c, i, 0);
}
#endif

/* voice track of the character (voice01.vtmt.gz) */
void dd_chr_voice_load(DdChr *c)
{
    void *t = (void *)dd_chr_s_vtmt;
    int size;

    t = ehp_find(dd_cutin_get(c->chr, c->sub), t, 0);
    if (t == 0)
        return;
    if (t != 0 && is_gzip(t)) {
        if (c->vbuf == 0)
            c->vbuf = dd_alloc(0x10000);
        gzip_decompress(c->vbuf, 0x10000, t, &size);
        t = c->vbuf = tgm_relocate(c->vbuf);
    }
    c->voice = t;
    c->vtime = 0.0f;
}

#ifdef NON_MATCHING /* forma/alocação não reproduzidas */
/* last frame of the current animation */
int dd_chr_anim_len(DdChr *c)
{
    Model *m;

    if (c->index < 0)
        return 0;
    m = dd_hook_anim(c);
    if (m == 0)
        return 0;
    return -((*(unsigned int *)((char *)m + 0x3C) & 2) != 0) + *(int *)(*(int *)((char *)m + 0x14) + 0x30);
}
#endif

void dd_chr_bone_mtx(DdChr *c, DdMtx *out, int bone)
{
    model_get_bone(c->model, out, dd_chr_cb_bone(c, bone));
}

#ifdef NON_MATCHING /* forma/alocação não reproduzidas */
/* matrix of attach point `id` (bones below 0x24, the named points from the table above) */
void dd_chr_attach_mtx(DdChr *c, DdMtx *out, int id)
{
    DdChrBone *b;
    DdChrBone *e;
    int i;
    DdXyzw pos;
    DdXyzw rot;
    DdMtx t;
    DdMtx r;
    DdMtx bm;

    if (id < 0x24) {
        dd_chr_bone_mtx(c, out, id);
        return;
    }
    b = 0;
    if (c->bones) {
        for (e = c->bones; e->id != 0; e++) {
            if (e->id == id) {
                b = e;
                break;
            }
        }
    }
    if (b == 0 && id < 6)
        b = &dd_chr_bones[id];
    if (b == 0 || b->id != id) {
        b = 0;
        for (i = 0; i < 6; i++) {
            if (id == dd_chr_bones[i].id) {
                b = &dd_chr_bones[i];
                break;
            }
        }
        if (b == 0 || b->id != id) {
            mat4_identity(out);
            return;
        }
    }
    dd_chr_cb_pos(c, &pos, &rot);
    pos.w = 1.0f;
    mat4_identity(&t);
    mat4_translate(&t, &t, &pos);
    quat_to_mat4(&r, &rot);
    mat4_mul(&t, &t, &r);
    dd_chr_bone_mtx(c, &bm, b->bone);
    mat4_mul(out, &bm, &t);
}
#endif

/* 1 when the animation reached its end and nothing holds it */
int dd_chr_anim_done(DdChr *c)
{
    float f = model_anim_frame(dd_hook_anim(c));

    if (f < (float)dd_chr_anim_len(c))
        return 0;
    if (dd_hook_8147d0(c))
        return 0;
    if (dd_lp_busy())
        return 0;
    return !c->x168;
}

void dd_chr_flag_set(DdChr *c, int on)
{
    c->x158 = on;
}

int dd_chr_flag_get(DdChr *c)
{
    return c->x158;
}

/* 1 when model part `bit` is hidden */
int dd_chr_part_hidden(DdChr *c, int bit)
{
    return !(c->mask & 1 << bit);
}

/* start of animation `anim` (disc sounds of the duel disk animations) */
int dd_chr_anim_cb(DdChr *c, int index, int arg)
{
    int r = 0;

    switch (c->anim) {
    case 0x16:
    case 9:
        dd_fx_set_arg4(0);
        break;
    case 0x17:
    case 10:
        dd_fx_set_arg4(1);
        break;
    }
    if (c->on_start != 0)
        r = c->on_start(c, index, arg);
    return r;
}

int dd_chr_cb_stop(DdChr *c, int index, int a, int b)
{
    int r = 0;

    if (c->on_stop)
        r = c->on_stop(c, index, a, b);
    return r;
}

int dd_chr_cb_tick(DdChr *c, int index, int frame, int anim_frame, int arg)
{
    int r = 0;

    if (c->on_tick)
        r = c->on_tick(c, index, frame, anim_frame, arg);
    return r;
}

int dd_chr_cb_map(DdChr *c, int anim)
{
    if (c->map_anim)
        return c->map_anim(c, anim);
    return anim;
}

int dd_chr_cb_bone(DdChr *c, int bone)
{
    return c->map_bone(c, bone);
}

void dd_chr_cb_pos(DdChr *c, DdXyzw *pos, DdXyzw *rot)
{
    c->get_pos(c, pos, rot);
}

/* projection, view and direction for the effects attached to the character */
void dd_chr_begin(DdChr *c)
{
    dd_camera_from_anim(c->time * (float)(int)gfx_get_state()[1], c->tmt);
    mtx_set_mode(0);
    mtx_get34(&c->proj);
    mtx_set_mode(1);
    mtx_get34(&c->view);
    mtx_get_view_dir(&c->dir);
    zone_view_save(1);
    mtx_set_mode(2);
}

#ifdef NON_MATCHING /* forma/alocação não reproduzidas */
/* draws the model on the current layer */
void *dd_chr_model_draw(void *list, DdChr *c)
{
    Model *m = dd_hook_anim(c);
    int layer;
    int params[13];

    if (m == 0)
        return list;
    list = dl_clear(dl_clear_stencil(list, 0), 0x14);
    list = dl_enable(list, 1);
    layer = dl_get_layer();
    dl_set_list(list);
    dd_chr_mask(c, m);
    memset(params, 0, sizeof(params));
    params[11] = c->chr;
    params[0] = layer;
    params[1] = layer;
    params[12] = (int)c;
    model_hook(m, 0, dd_disc_tex_hook);
    mtx_set_mode(2);
    mtx_identity();
    model_draw(m, params);
    return dl_select_layer(layer, 0);
}
#endif

#ifdef NON_MATCHING /* forma/alocação não reproduzidas */
/* parts of the model shown at this frame (tracks 10..41 of the animation; forced bits) */
void dd_chr_mask(DdChr *c, Model *m)
{
    unsigned int mask = 0;
    int fps = gfx_get_state()[1];
    float t = c->time;
    void *a = c->tmt;
    unsigned int i;

    if (a == 0)
        return;
    for (i = 0; i < 32; i++) {
        if (keyframe_eval((char *)((int *)a)[2] + (i + 10) * 8, t * (float)fps) == 0.0f)
            mask |= 1 << i;
    }
    if (!(mask & 0x200)) {
        if (c->disc_on)
            mask = (mask & ~0x400) | 0x200;
    }
    mask = (mask & ~c->mask_sel) | (c->mask_val & c->mask_sel);
    model_set_mask(m, mask);
    c->mask = mask;
}
#endif
