// @range 0x0880AC6C-0x0880B284
// @data 0x0880FAC0-0x0880FFA8
/* Bust-up faces of the talk windows: tutorial/face/bu_<name>.ehp read asynchronously into the duel
   heap, body.ehf and mouth.ehf animated (blinking at random intervals, mouth moving while the text is
   typed); the expression code of the script picks the animation frame through a table per face. */
#include "libtutorial.h"

struct TutFace {                 /* 0x18 bytes, heap */
    void *heap;
    void *pack;                  /* +0x04 bu_<name>.ehp */
    void *body;                  /* +0x08 body.ehf */
    void *mouth;                 /* +0x0C mouth.ehf */
    unsigned short ready : 1;    /* +0x10 */
    unsigned short chara : 8;
    unsigned short expr : 4;
    unsigned short talk : 1;     /* mouth animated */
    unsigned short show : 1;     /* blinking on */
    unsigned short pad10 : 1;
    unsigned short blink_t;      /* +0x12 frames since the last blink */
    unsigned short blink_at;     /* +0x14 */
    unsigned char index : 4;     /* +0x16 entry of tut_face_files */
    unsigned char pad16 : 4;
    unsigned char pad17;
};

typedef struct TutFaceFile {
    unsigned short chara;
    const char *path;
} TutFaceFile;

typedef struct TutExprKey {
    unsigned short expr;
    unsigned short frame;
} TutExprKey;

int ehf_set_anim(void *e, int target, int limit);
int ehf_get_frame_count(void *e);
void *ehf_draw(void *list, void *ehf, void *opts);
void *gfx_begin_2d(void *list);
void *gfx_end_2d(void *list);
unsigned int *gfx_get_state(void);
unsigned int rand_next_alias(void);
void tut_face_set(TutFace *f, unsigned int show, unsigned int talk, unsigned int expr);
unsigned short tut_face_frame(unsigned short face, unsigned short expr);
int tut_face_loaded(TutFace *f, int n);

TutFaceFile tut_face_files[11] = {
    { 2, "tutorial/face/bu_jdi.ehp" },
    { 3, "tutorial/face/bu_sho.ehp" },
    { 4, "tutorial/face/bu_hyt.ehp" },
    { 5, "tutorial/face/bu_ask.ehp" },
    { 6, "tutorial/face/bu_mab.ehp" },
    { 7, "tutorial/face/bu_mis.ehp" },
    { 8, "tutorial/face/bu_ryo.ehp" },
    { 9, "tutorial/face/bu_cro.ehp" },
    { 10, "tutorial/face/bu_dai.ehp" },
    { 39, "tutorial/face/bu_sam.ehp" },
    { 11, "tutorial/face/bu_maa.ehp" },
};

/* expression -> first frame of the animation, per face; entry 0 is the default */
TutExprKey tut_face_exprs[11][19] = {
    { { 0, 0 }, { 1, 15 }, { 2, 45 }, { 4, 30 }, { 5, 30 }, { 7, 60 }, { 8, 60 }, { 10, 75 }, { 12, 90 }, { 13, 105 }, { 3, 120 }, { 11, 120 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 1, 15 }, { 2, 45 }, { 4, 30 }, { 5, 30 }, { 7, 60 }, { 14, 60 }, { 8, 75 }, { 10, 90 }, { 11, 90 }, { 12, 105 }, { 13, 120 }, { 3, 135 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 1, 15 }, { 2, 45 }, { 4, 30 }, { 5, 30 }, { 7, 60 }, { 14, 60 }, { 8, 60 }, { 10, 75 }, { 11, 75 }, { 12, 90 }, { 13, 105 }, { 3, 120 }, { 15, 135 }, { 9, 135 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 1, 15 }, { 2, 60 }, { 4, 30 }, { 5, 30 }, { 7, 45 }, { 8, 45 }, { 14, 120 }, { 10, 90 }, { 11, 90 }, { 12, 75 }, { 13, 105 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 10, 105 }, { 11, 105 }, { 3, 120 }, { 6, 135 }, { 15, 0 }, { 6, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 1, 15 }, { 2, 45 }, { 4, 30 }, { 5, 30 }, { 7, 60 }, { 8, 60 }, { 10, 60 }, { 12, 75 }, { 13, 90 }, { 3, 105 }, { 6, 105 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 1, 15 }, { 2, 45 }, { 4, 30 }, { 5, 30 }, { 7, 60 }, { 8, 60 }, { 10, 75 }, { 13, 90 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 1, 15 }, { 4, 30 }, { 13, 45 }, { 3, 60 }, { 6, 75 }, { 15, 90 }, { 9, 90 }, { 11, 105 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 1, 15 }, { 2, 45 }, { 4, 30 }, { 5, 30 }, { 7, 60 }, { 10, 75 }, { 11, 75 }, { 15, 90 }, { 9, 90 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 1, 15 }, { 2, 45 }, { 10, 45 }, { 4, 30 }, { 5, 30 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
    { { 0, 0 }, { 1, 15 }, { 10, 15 }, { 2, 45 }, { 4, 30 }, { 5, 30 }, { 7, 60 }, { 8, 60 }, { 12, 75 }, { 13, 90 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
};

TutFace *tut_face_load(void *heap, unsigned short chara)
{
    TutFace *f = heap_alloc(heap, 0x18);
    int idx;
    unsigned int i;
    TutFaceFile *ff;
    const char *path;
    int size;

    if (f != 0) {
        memset(f, 0, 0x18);
        f->heap = heap;
        idx = -1;
        for (i = 0, ff = tut_face_files; i < 11; i++, ff++) {
            if (ff->chara == chara) {
                idx = i;
                break;
            }
        }
        if (idx >= 0) {
            path = tut_face_files[idx].path;
            size = ehfile_get_size(3, path);
            f->pack = heap_alloc(heap, size);
            ehfile_read_async_alias(3, path, f->pack, size, (int (*)(void *, int))tut_face_loaded, f);
            f->chara = chara;
            f->index = idx;
        }
    }
    return f;
}

int tut_face_ready(TutFace *f)
{
    if (f != 0 && f->ready)
        return 1;
    return 0;
}

void *tut_face_render(void *list, TutFace *f, int x, int y)
{
    unsigned int opts[0x18];
    int fr;

    if (f == 0)
        return list;
    if (!f->ready)
        return list;
    memset(opts, 0, 0x60);
    opts[0] = *gfx_get_state();
    ((float *)opts)[5] = x;
    opts[3] = 0x10000;
    ((float *)opts)[6] = y;
    if (f->body != 0) {
        list = gfx_begin_2d(list);
        list = gfx_end_2d(ehf_draw(list, f->body, opts));
    }
    if (f->mouth != 0) {
        list = gfx_begin_2d(list);
        list = gfx_end_2d(ehf_draw(list, f->mouth, opts));
    }
    if (f->show) {
        if (f->blink_t >= f->blink_at) {
            if (ehf_get_frame_count(f->body) % 15 == 14) {
                fr = tut_face_frame(f->index, f->expr);
                f->blink_t = 0;
                f->blink_at = (int)rand_next_alias() % 210 + 90;
                ehf_set_anim(f->body, fr, fr + 1);
            }
        } else {
            f->blink_t++;
            if (f->blink_t >= f->blink_at) {
                fr = tut_face_frame(f->index, f->expr);
                ehf_set_anim(f->body, fr + 4, fr + 15);
            }
        }
    }
    return list;
}

void tut_face_set(TutFace *f, unsigned int show, unsigned int talk, unsigned int expr)
{
    int changed;
    int talk_changed;
    int fr;

    if (f == 0)
        return;
    changed = expr != f->expr;
    talk_changed = talk != f->talk;
    if (show != f->show) {
        f->blink_t = 0;
        if (show != 0)
            f->blink_at = (int)rand_next_alias() % 210 + 90;
        else
            f->blink_at = 0;
        f->show = show;
    }
    if (changed) {
        fr = tut_face_frame(f->index, expr);
        if (f->body != 0)
            ehf_set_anim(f->body, fr, fr + 1);
        talk_changed = 1;
        f->expr = expr;
    }
    if (talk_changed) {
        fr = tut_face_frame(f->index, f->expr);
        if (f->mouth != 0) {
            if (talk != 0)
                ehf_set_anim(f->mouth, fr + 4, fr + 15);
            else
                ehf_set_anim(f->mouth, fr, fr + 1);
        }
        f->talk = talk;
    }
}

unsigned short tut_face_frame(unsigned short face, unsigned short expr)
{
    TutExprKey *t = tut_face_exprs[face];
    int i;

    for (i = 0; i < 19; i++) {
        if (expr == t[i].expr)
            return t[i].frame;
    }
    return t[0].frame;
}

int tut_face_loaded(TutFace *f, int n)
{
    int ok = n > 0;

    if (ok) {
        ehp_mount(0, f->pack);
        f->body = ehp_find(f->pack, "body.ehf", 0);
        f->mouth = ehp_find(f->pack, "mouth.ehf", 0);
        f->show = 1;
        f->talk = 1;
        f->expr = 1;
        tut_face_set(f, 0, 0, 0);
        f->ready = 1;
    }
    return ok;
}
