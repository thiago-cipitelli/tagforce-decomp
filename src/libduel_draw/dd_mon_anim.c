// @range 0x088282E4-0x08828DD0
// @asm 0x088285D4-0x088288C0
// @data 0x0887653C-0x08876710
/* Callbacks of a monster model played by the duel scene: the callback table, the start/event
   hooks, the pick of an animation from the voice set of the character, the bone transform of the
   tracked bones, the partner voice line and the LP/ATK panels shown at fixed frames of the
   attack, damage and destruction animations. */
#include "dd_d3.h"

typedef struct DdQuat {
    float x, y, z, w;
} __attribute__((aligned(16))) DdQuat;

typedef struct DdAnimRange {           /* animations of one kind */
    int kind;
    int lo, hi;                        /* range of animation ids */
} DdAnimRange;

typedef struct DdTgmt {                /* animation data of a model (Tgmt) */
    int x00;
    unsigned short *bones;             /* +0x04 per bone: position, rotation, scale track or 0xFFFF */
    unsigned char *keys;               /* +0x08 position/scale tracks, 8 bytes each */
    int x0c;
    unsigned char *rots;               /* +0x10 rotation tracks, 8 bytes each */
} DdTgmt;

typedef struct DdModel {
    unsigned char pad00[0x54];
    DdTgmt *anim;                      /* +0x54 */
} DdModel;

void dd_fx3_start(void);
void dd_fx_start(int a, int b, int c);
void dd_fx3_stop(void);
void dd_fx_stop(int a, int anim, int c, int d);
void dd_fx3_set_param(void);
void dd_fx_set_param(DdMon *m, int anim, int c, int frame, int flag);
void voice_event(DdMon *m, int anim, int c, int frame, int flag);
void caption_enable(int on);
void caption_set(int voice, int id);
void func_08806434(int a, int b);
int func_08814118(DdModel *model, int bone);
void dd_lp_count(int x, int y, int a, int b);
void dd_lp_show(int x, int y, int atk, int def);
int dd_lp_busy(void);
void fade_start(unsigned char type, unsigned int color, unsigned short frames);
void fade_start_in(unsigned char type, unsigned int color, unsigned short frames);
int fade_is_done(void);
int voice_table_find(unsigned int chr, unsigned int variant);
int rand_next(void);
int gamestate_partner_mode(void);
unsigned short partner_table_lookup(int flag, unsigned short mode, unsigned short idx, unsigned short *table);
int snd_channel_get_u32(int slot);
int partner_voice_play(int slot, int arg);
int card_get_atk_q(int id);
int card_get_def_q(int id);
int model_anim_frame_count(void *m);
float keyframe_eval(void *tr, float t);
void anim_eval_track(DdQuat *out, DdTgmt *a, void *tr, float t);
DdMat4 *quat_to_mat4(DdMat4 *out, DdQuat *q);
DdMat4 *mat4_identity(DdMat4 *out);
DdMat4 *mat4_mul(DdMat4 *out, DdMat4 *a, DdMat4 *b);
float *vec3_scale_w(float *out, float *v, float s);

extern DdAnimRange *dd_mon_anim_cur;

int dd_mon_on_start(int a, int b, int c);
int dd_mon_on_event(int a, int anim, int c, int d);
int dd_mon_pick_anim(DdMon *m, int anim);
int dd_mon_cb_ret0(void);
int dd_mon_cb_ret0b(void);
void dd_mon_bone(DdModel *model, DdMat4 *out, DdMat4 *in, int bone, float t);
void dd_mon_bone_xform(DdModel *model, DdMat4 *out, DdMat4 *in, int i, float t);
void dd_mon_voice(DdMon *m, int anim, int c, int frame, int flag);
void dd_mon_panels(DdMon *m, int anim, int c, int frame, int flag);

void dd_mon_cb_init(int a, int b, DdMonCb *cb)
{
    memset(cb, 0, sizeof(*cb));
    cb->a = a;
    cb->b = b;
    cb->bone = dd_mon_bone;
    cb->cb24 = dd_mon_cb_ret0;
    cb->cb28 = dd_mon_cb_ret0b;
    cb->on_event = dd_mon_on_event;
    cb->on_start = dd_mon_on_start;
    cb->pick_anim = dd_mon_pick_anim;
}

int dd_mon_on_start(int a, int b, int c)
{
    dd_fx3_start();
    dd_fx_start(a, b, c);
    caption_enable(0);
    return 0;
}

int dd_mon_on_event(int a, int anim, int c, int d)
{
    dd_fx3_stop();
    dd_fx_stop(a, anim, c, d);
    switch (anim) {
    case 52 ... 54:
        fade_start(1, 0xFFFFFFFF, 16);
        func_08806434(32, 1);
        break;
    }
    return 0;
}

int dd_mon_on_frame(DdMon *m, int anim, int c, int frame, int flag)
{
    dd_fx3_set_param();
    dd_fx_set_param(m, anim, c, frame, flag);
    voice_event(m, anim, c, frame, flag);
    dd_mon_voice(m, anim, c, frame, flag);
    dd_mon_panels(m, anim, c, frame, flag);
    return 0;
}

int dd_mon_bones[9] = { 4, 5, 6, 7, 8, 9, 10, 11, 12 };

void dd_mon_bone(DdModel *model, DdMat4 *out, DdMat4 *in, int bone, float t)
{
    unsigned int i;

    for (i = 0; i < 9; i++) {
        if (bone == func_08814118(model, dd_mon_bones[i])) {
            dd_mon_bone_xform(model, out, in, i, t);
            return;
        }
    }
}

DdAnimRange dd_mon_anim_ranges[26] = {
    { 0, 0x66, 0x6A }, { 1, 0xF7, 0x121 }, { 2, 0x81, 0x8B }, { 3, 0x0D, 0x1A },
    { 4, 0x6B, 0x72 }, { 5, 0x37, 0x40 }, { 6, 0x2A, 0x32 }, { 7, 0x33, 0x33 },
    { 8, 0x5B, 0x65 }, { 9, 0x76, 0x80 }, { 10, 0x76, 0x80 }, { 11, 0x00, 0x0C },
    { 12, 0x8C, 0x95 }, { 13, 0x20, 0x29 }, { 14, 0x34, 0x36 }, { 15, 0x1B, 0x1F },
    { 16, 0x14E, 0x178 }, { 17, 0x122, 0x14D }, { 18, 0xCC, 0xF6 }, { 19, 0x41, 0x46 },
    { 20, 0x47, 0x4D }, { 21, 0x4E, 0x52 }, { 22, 0x53, 0x5A }, { 23, 0x53, 0x5A },
    { 24, 0x73, 0x75 }, { 25, 0x66, 0x6A },
};

int dd_mon_voice_anims[6][4] = {
    { 0x11B, 0x172, 0x147, 0xF0 }, { 0x11C, 0x173, 0x148, 0xF1 }, { 0x11D, 0x174, 0x149, 0xF2 },
    { 0x11E, 0x175, 0x14A, 0xF3 }, { 0x11F, 0x176, 0x14B, 0xF4 }, { 0x120, 0x177, 0x14C, 0xF5 },
};

#ifdef NON_MATCHING /* agendamento: o original põe o sw de dd_mon_anim_cur no delay slot do jal, depois das cargas dos argumentos */
int dd_mon_pick_anim(DdMon *m, int anim)
{
    int list[64];
    int n;
    int i;
    int v;
    int kind;
    int sub;

    if (anim >= 0x1000) {
        anim -= 0x1000;
        if (m->anim_ok[anim] == 0)
            return -1;
        return anim;
    }
    dd_mon_anim_cur = &dd_mon_anim_ranges[anim];
    v = voice_table_find(m->chara, m->x04);
    kind = -1;
    switch (v) {
    case 228 ... 240:
        kind = 0;
        break;
    case 243 ... 255:
        kind = 1;
        break;
    case 258 ... 270:
        kind = 2;
        break;
    case 273 ... 285:
        kind = 3;
        break;
    case 288 ... 300:
        kind = 4;
        break;
    case 303 ... 315:
        kind = 5;
        break;
    }
    if (kind >= 0) {
        sub = -1;
        switch (anim) {
        case 1:
            sub = 0;
            break;
        case 16:
            sub = 1;
            break;
        case 17:
            sub = 2;
            break;
        case 18:
            sub = 3;
            break;
        }
        if (sub >= 0)
            return dd_mon_voice_anims[kind][sub];
    }
    n = 0;
    for (i = dd_mon_anim_cur->lo; i <= dd_mon_anim_cur->hi; i++) {
        if (m->anim_ok[i] != 0)
            list[n++] = i;
    }
    if (n == 0) {
        if (anim == 7)
            return dd_mon_pick_anim(m, 5);
        if (anim == 14)
            return dd_mon_pick_anim(m, 13);
        for (i = 0; i < 334; i++) {
            if (m->anim_ok[i] != 0)
                return i;
        }
        return -1;
    }
    return list[rand_next() % n];
}
#endif

int dd_mon_cb_ret0(void)
{
    return 0;
}

int dd_mon_cb_ret0b(void)
{
    return 0;
}

void dd_mon_bone_xform(DdModel *model, DdMat4 *out, DdMat4 *in, int i, float t)
{
    DdQuat q;
    DdMat4 tm;
    DdMat4 rm;
    DdTgmt *a = model->anim;
    unsigned short *trk;
    unsigned char *k;
    float x, y, z;

    if (a == 0)
        return;
    trk = &a->bones[i * 4];
    *out = *in;
    if (trk[0] != 0xFFFF) {
        k = a->keys + trk[0] * 8;
        x = keyframe_eval(k, t);
        y = keyframe_eval(k + 8, t);
        z = keyframe_eval(k + 16, t);
        mat4_identity(&tm);
        tm.m[15] = 1.0f;
        tm.m[12] = x;
        tm.m[13] = y;
        tm.m[14] = z;
        mat4_mul(out, out, &tm);
    }
    if (trk[1] == 0xFFFF) {
        q = (DdQuat){ 0.0f, 0.0f, 0.0f, 1.0f };
    } else {
        anim_eval_track(&q, a, a->rots + trk[1] * 8, t);
        quat_to_mat4(&rm, &q);
        mat4_mul(out, out, &rm);
    }
    if (trk[2] != 0xFFFF) {
        k = a->keys + trk[2] * 8;
        x = keyframe_eval(k, t);
        y = keyframe_eval(k + 8, t);
        z = keyframe_eval(k + 16, t);
        vec3_scale_w(&out->m[0], &out->m[0], x);
        vec3_scale_w(&out->m[4], &out->m[4], y);
        vec3_scale_w(&out->m[8], &out->m[8], z);
    }
}

void dd_mon_voice(DdMon *m, int anim, int c, int frame, int flag)
{
    int v;
    int played;
    int story;
    int id;

    if (flag == 0)
        return;
    v = snd_channel_get_u32(m->chan);
    played = partner_voice_play(m->chan, 0);
    if (v == 0) {
        caption_enable(0);
        return;
    }
    story = m->chara < 41;
    id = partner_table_lookup(story, gamestate_partner_mode(), v - played, (unsigned short *)m->x0c);
    if (id >= 0) {
        caption_enable(1);
        caption_set(m->voice, id);
    }
}

void dd_mon_panels(DdMon *m, int anim, int c, int frame, int flag)
{
    int last;
    int atk, def;

    if (flag == 0)
        return;
    last = model_anim_frame_count(m->model) - 1;
    switch (anim) {
    case 83 ... 90:
    case 118 ... 128:
        if (frame == last && m->card > 0) {
            atk = card_get_atk_q(m->card);
            def = card_get_def_q(m->card);
            if (atk == 0xFFFF)
                atk = -1;
            if (def == 0xFFFF)
                def = -1;
            dd_lp_show(96, 215, atk, def);
        }
        break;
    case 32 ... 41:
    case 27 ... 31:
        if (frame == 25)
            dd_lp_count(94, 212, m->lp_from, m->lp_to);
        break;
    case 52 ... 54:
        if (frame == 25)
            dd_lp_count(94, 212, m->lp_from, m->lp_to);
        if (m->faded) {
            if (!fade_is_done())
                m->hold = 1;
        } else if (frame >= 25 && dd_lp_busy() == 0) {
            m->hold = 1;
            m->faded = 1;
            fade_start_in(1, 0xFFFFFFFF, 60);
            func_08806434(32, 1);
        } else {
            m->hold = 1;
        }
        break;
    case 102 ... 106:
        if (m->lp_from != m->lp_to && frame == 10)
            dd_lp_count(94, 212, m->lp_from, m->lp_to);
        break;
    }
}
