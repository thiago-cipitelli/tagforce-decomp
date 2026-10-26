// @range 0x088044D8-0x08805220
/* Bust-up portraits (field/bustup/bu_%s.ehp.gz): up to three on screen, each with a body and a
   mouth EHF, fading in and out, blinking at random intervals and talking while their line is shown.
   The field commands load, open, animate, close and unload them; the speaker is drawn on top. */
#include "libfield.h"

void fld_bu_reset(void);
void fld_bu_clear(FieldBuSlot *s);
FieldAnimFrame *fld_anim_frame(int chara, int anim);
void fld_bu_set_talk(int i, int talk);
void fld_bu_set_anim(int i, int anim);
void fld_bu_loaded(int *file, FieldBuSlot *s);
void fld_voice_loaded(int *file, void *arg);
void fld_bu_free_all(void);

extern const char str_body_ehf[];     /* "body.ehf" */
extern const char str_mouth_ehf[];    /* "mouth.ehf" */

void fld_bu_init(void)
{
    memset(&field_bu, 0, sizeof(FieldBu));
    fld_bu_reset();
}

void fld_bu_term(void)
{
    fld_bu_free_all();
    fld_bu_init();
}

void fld_bu_free_all(void)
{
    fld_bu_reset();
}

void fld_bu_update(void)
{
    FieldBu *b = &field_bu;
    FieldCtx *ctx = fld_ctx_get();
    int i;
    int j;
    FieldBuSlot *s;
    FieldAnimFrame *f;
    int o;
    int c;

    for (i = 0; i < 3; i++) {
        s = &b->slot[i];
        if (b->slot[i].loaded && s->file && *s->file) {
            switch (s->state) {
            case 0:
                break;
            case 1:
                if (b->ready) {
                    if (s->body_opts.color[3] < 0xFF) {
                        s->body_opts.color[3] += 0x11;
                        s->mouth_opts.color[3] += 0x11;
                    } else {
                        s->state = 2;
                        b->open[i] = 0;
                    }
                }
                break;
            case 2:
                if (ctx->speaker >= 0) {
                    for (j = 0; j < 3; j++) {
                        if (j == ctx->speaker)
                            fld_bu_set_talk(j, 1);
                        else
                            fld_bu_set_talk(j, 0);
                    }
                } else {
                    fld_bu_set_talk(3, 0);
                }
                break;
            case 3:
                if (s->body_opts.color[3] > 0) {
                    s->body_opts.color[3] -= 0x11;
                    s->mouth_opts.color[3] -= 0x11;
                } else {
                    s->state = 0;
                    b->close[i] = 0;
                    fld_bu_clear(s);
                }
                break;
            }
            if (s->blink) {
                f = fld_anim_frame(s->chara, s->anim);
                if (ehf_get_frame_count(s->body) >= f->frame + 14) {
                    s->blink = 0;
                    s->blink_wait = (unsigned short)rand() % 210 + 90;
                    s->body_opts.time = 0;
                } else {
                    s->body_opts.time++;
                }
            } else if (s->blink_wait == 0) {
                s->blink = 1;
                s->body_opts.time++;
            } else {
                s->blink_wait--;
            }
            if (s->talk)
                s->mouth_opts.time++;
        }
    }
    o = 0;
    c = 0;
    for (j = 0; j < 3; j++) {
        o |= b->open[j];
        c |= b->close[j];
    }
    if (!o)
        b->opening = 0;
    if (!c)
        b->closing = 0;
}

void fld_bu_draw(void)
{
    FieldBu *b = &field_bu;
    void *dl = dl_select_layer(3, 0);
    int i;
    FieldBuSlot *s;

    for (i = 0; i < 3; i++) {
        s = &b->slot[i];
        if (s->loaded && s->file && *s->file && i != b->speaker) {
            if (s->mouth) {
                dl = gfx_begin_2d(dl);
                dl = ehf_draw(dl, s->mouth, &s->mouth_opts);
                dl = gfx_end_2d(dl);
            }
            dl = gfx_begin_2d(dl);
            dl = ehf_draw(dl, s->body, &s->body_opts);
            dl = gfx_end_2d(dl);
        }
    }
    s = &b->slot[b->speaker];
    if (s->loaded && s->file && *s->file) {
        if (s->mouth) {
            dl = gfx_begin_2d(dl);
            dl = ehf_draw(dl, s->mouth, &s->mouth_opts);
            dl = gfx_end_2d(dl);
        }
        dl = gfx_begin_2d(dl);
        dl = ehf_draw(dl, s->body, &s->body_opts);
        dl = gfx_end_2d(dl);
    }
    dl_set_list(dl);
}

/* starts the voice file n (n <= 0 stops a short one that is playing) */
void fld_voice_play(int n)
{
    int *v = &field_bu.voice;

    if (n > 0) {
        if (n != *v) {
            *v = n;
            fld_load(n, fld_voice_loaded, 0, 1);
        }
    } else if (*v > 0 && *v < 60) {
        fld_unload(3, *v);
        *v = 0;
    }
}

void fld_bu_cmd_load(FieldList *l)
{
    FieldBu *b = &field_bu;
    int i;

    field_bu.loading = 1;
    for (i = 0; i < l->n; i++) {
        b->pending++;
        fld_load(((int *)l->items)[i], (void (*)(int *, void *))fld_bu_loaded, 0, 0);
    }
    if (l->n <= 0)
        b->loading = 0;
}

int fld_bu_load_done(void)
{
    return !field_bu.loading;
}

void fld_bu_cmd_unload(void)
{
    field_bu.unloading = 1;
    fld_load_cancel_all();
    fld_bu_reset();
    field_bu.unloading = 0;
}

int fld_bu_unload_done(void)
{
    return !field_bu.unloading;
}

void fld_bu_cmd_open(FieldList *l)
{
    FieldBu *b = &field_bu;
    int i;
    FieldBuOpen *e;
    FieldBuSlot *s;

    field_bu.opening = 1;
    field_bu.ready = 0;
    field_bu.to_load = l->n;
    for (i = 0; i < l->n; i++) {
        e = &((FieldBuOpen *)l->items)[i];
        s = &b->slot[e->index];
        b->open[e->index] = 1;
        fld_bu_clear(s);
        s->chara = e->chara;
        s->anim = e->anim;
        s->index = e->index;
        s->state = 1;
        s->body_opts.flags |= 0x10000;
        s->body_opts.flags |= 4;
        s->body_opts.color[0] = 0xFF;
        s->body_opts.color[1] = 0xFF;
        s->body_opts.color[2] = 0xFF;
        s->body_opts.color[3] = 0;
        s->body_opts.x = field_bu_xpos[e->index];
        s->mouth_opts.flags |= 0x10000;
        s->mouth_opts.flags |= 4;
        s->mouth_opts.color[0] = 0xFF;
        s->mouth_opts.color[1] = 0xFF;
        s->mouth_opts.color[2] = 0xFF;
        s->mouth_opts.color[3] = 0;
        s->mouth_opts.x = field_bu_xpos[e->index];
        fld_load(s->chara, (void (*)(int *, void *))fld_bu_loaded, s, 1);
    }
}

int fld_bu_open_done(void)
{
    return !field_bu.opening;
}

void fld_bu_cmd_anim(FieldList *l)
{
    FieldBu *b = &field_bu;
    int i;
    FieldBuAnim *e;
    FieldBuSlot *s;

    b->animating = 1;
    for (i = 0; i < l->n; i++) {
        e = &((FieldBuAnim *)l->items)[i];
        s = &b->slot[e->index];
        if (b->slot[e->index].loaded)
            fld_bu_set_anim(s->index, e->anim);
    }
    b->animating = 0;
}

int fld_bu_anim_done(void)
{
    return !field_bu.animating;
}

void fld_bu_cmd_close(FieldList *l)
{
    FieldBu *b = &field_bu;
    int i;
    int k;
    FieldBuSlot *s;

    field_bu.closing = 1;
    for (i = 0; i < l->n; i++) {
        k = ((int *)l->items)[i];
        s = &b->slot[k];
        if (b->slot[k].loaded) {
            b->close[k] = 1;
            s->state = 3;
        }
    }
}

int fld_bu_close_done(void)
{
    return !field_bu.closing;
}

void fld_bu_set_anim(int i, int anim)
{
    FieldBuSlot *s = &field_bu.slot[i];
    FieldAnimFrame *f;

    s->anim = anim;
    s->blink = 0;
    s->blink_wait = (unsigned short)rand() % 210 + 90;
    f = fld_anim_frame(s->chara, s->anim);
    ehf_set_anim(s->body, f->frame + 4, f->frame + 15);
    ehf_set_frame(s->body, f->frame + 4);
    if (s->mouth) {
        ehf_set_anim(s->mouth, f->frame + 4, f->frame + 15);
        ehf_set_frame(s->mouth, f->frame + 4);
    }
}

void fld_bu_set_talk(int i, int talk)
{
    FieldBu *b = &field_bu;
    int j;
    FieldBuSlot *s;
    FieldAnimFrame *f;

    if (i != 3) {
        s = &b->slot[i];
        if (!b->slot[i].loaded)
            return;
        f = fld_anim_frame(s->chara, s->anim);
        if (s->talk != talk) {
            s->talk = talk;
            if (s->mouth)
                ehf_set_frame(s->mouth, f->frame + 4);
        }
    } else {
        for (j = 0; j < 3; j++) {
            s = &b->slot[j];
            if (b->slot[j].loaded) {
                f = fld_anim_frame(s->chara, s->anim);
                if (s->talk != talk) {
                    s->talk = talk;
                    if (s->mouth)
                        ehf_set_frame(s->mouth, f->frame + 4);
                }
            }
        }
    }
    if (talk) {
        if (i != 3)
            b->speaker = i;
        else
            b->speaker = 1;
    }
}

void fld_bu_reset(void)
{
    FieldBu *b = &field_bu;
    int i;

    for (i = 0; i < 3; i++)
        fld_bu_clear(&b->slot[i]);
}

void fld_bu_clear(FieldBuSlot *s)
{
    if (s->pack) {
        fld_free(s->pack);
        s->pack = 0;
    }
    memset(s, 0, sizeof(FieldBuSlot));
}

/* frame of an animation id in the table of the current mode (the first entry if missing) */
FieldAnimFrame *fld_anim_frame(int chara, int anim)
{
    int mode = fld_mode_get();
    int i = 0;
    FieldAnimFrame *f = field_anim_frames[mode];

    for (; i < 19; i++, f++) {
        if (f->id == anim)
            return f;
    }
    return field_anim_frames[mode];
}

void fld_voice_loaded(int *file, void *arg)
{
}

void fld_bu_loaded(int *file, FieldBuSlot *s)
{
    FieldBu *b = &field_bu;
    void *src;
    unsigned int size;
    void *pack;
    float y;
    int *e;

    if (file != 0) {
        if (s != 0) {
            s->file = file;
            src = (void *)*file;
            if (ehfile_is_gzip(src)) {
                size = read_u32_before(src, fld_load_size(3, s->chara));
                s->pack = fld_alloc(size);
                gzip_decompress_alias(s->pack, size, src);
                pack = s->pack;
                ehp_mount(0, pack);
                s->body = ehp_find(pack, str_body_ehf, 0);
                s->mouth = ehp_find(pack, str_mouth_ehf, 0);
                b->to_load--;
                s->loaded = 1;
                fld_bu_set_anim(s->index, s->anim);
                fld_bu_set_talk(s->index, 0);
                e = ehf_resolve(s->body);
                y = 240.0f - 0.5f * ((float)(((int *)e[2])[5] - ((int *)e[2])[4]) / 20.0f);
                ((short *)e)[10] = y;
                if (s->mouth) {
                    e = ehf_resolve(s->mouth);
                    ((short *)e)[10] = y;
                }
            } else {
                b->to_load--;
            }
            if (b->to_load <= 0)
                b->ready = 1;
        } else {
            b->pending--;
            if (b->pending <= 0)
                b->loading = 0;
        }
    }
}
