// @range 0x0882ABD4-0x0882B238
// @asm 0x0882B0A4-0x0882B0F0
// @data 0x0883D6D8-0x0883DBC0
/* Tutorial faces (FieldFace): the animated EHF bust (body.ehf and mouth.ehf of
   tutorial/face/bu_xxx.ehp) of the character teaching, with blinking and the talking mouth. */
#include "libfield.h"

int rand_next_alias(void);
void ehfile_read_async_alias(int dir, const char *name, void *buf, int size, int (*cb)(void *, int), void *arg);

int fface_loaded(FieldFace *f, int size);
void fface_set(FieldFace *f, int blink, int talk, int anim);
unsigned short fface_frame(unsigned short face, unsigned short anim);

FaceFile fface_files[11] = {
    { 2, "tutorial/face/bu_jdi.ehp" },
    { 3, "tutorial/face/bu_sho.ehp" },
    { 4, "tutorial/face/bu_hyt.ehp" },
    { 5, "tutorial/face/bu_ask.ehp" },
    { 6, "tutorial/face/bu_mab.ehp" },
    { 7, "tutorial/face/bu_mis.ehp" },
    { 8, "tutorial/face/bu_ryo.ehp" },
    { 9, "tutorial/face/bu_cro.ehp" },
    { 10, "tutorial/face/bu_dai.ehp" },
    { 0x27, "tutorial/face/bu_sam.ehp" },
    { 11, "tutorial/face/bu_maa.ehp" },
};

/* first frame of each animation of each face (searched by the animation id; row[0] by default) */
FaceAnim fface_anims[11][19] = {
    { { 0, 0 }, { 1, 15 }, { 2, 45 }, { 4, 30 }, { 5, 30 }, { 7, 60 }, { 8, 60 }, { 10, 75 }, { 12, 90 }, { 13, 105 }, { 3, 120 }, { 11, 120 } },
    { { 0, 0 }, { 1, 15 }, { 2, 45 }, { 4, 30 }, { 5, 30 }, { 7, 60 }, { 14, 60 }, { 8, 75 }, { 10, 90 }, { 11, 90 }, { 12, 105 }, { 13, 120 }, { 3, 135 } },
    { { 0, 0 }, { 1, 15 }, { 2, 45 }, { 4, 30 }, { 5, 30 }, { 7, 60 }, { 14, 60 }, { 8, 60 }, { 10, 75 }, { 11, 75 }, { 12, 90 }, { 13, 105 }, { 3, 120 }, { 15, 135 }, { 9, 135 } },
    { { 0, 0 }, { 1, 15 }, { 2, 60 }, { 4, 30 }, { 5, 30 }, { 7, 45 }, { 8, 45 }, { 14, 120 }, { 10, 90 }, { 11, 90 }, { 12, 75 }, { 13, 105 } },
    { { 0, 0 }, { 10, 105 }, { 11, 105 }, { 3, 120 }, { 6, 135 }, { 15, 0 }, { 6, 0 } },
    { { 0, 0 }, { 1, 15 }, { 2, 45 }, { 4, 30 }, { 5, 30 }, { 7, 60 }, { 8, 60 }, { 10, 60 }, { 12, 75 }, { 13, 90 }, { 3, 105 }, { 6, 105 } },
    { { 0, 0 }, { 1, 15 }, { 2, 45 }, { 4, 30 }, { 5, 30 }, { 7, 60 }, { 8, 60 }, { 10, 75 }, { 13, 90 } },
    { { 0, 0 }, { 1, 15 }, { 4, 30 }, { 13, 45 }, { 3, 60 }, { 6, 75 }, { 15, 90 }, { 9, 90 }, { 11, 105 } },
    { { 0, 0 }, { 1, 15 }, { 2, 45 }, { 4, 30 }, { 5, 30 }, { 7, 60 }, { 10, 75 }, { 11, 75 }, { 15, 90 }, { 9, 90 } },
    { { 0, 0 }, { 1, 15 }, { 2, 45 }, { 10, 45 }, { 4, 30 }, { 5, 30 } },
    { { 0, 0 }, { 1, 15 }, { 10, 15 }, { 2, 45 }, { 4, 30 }, { 5, 30 }, { 7, 60 }, { 8, 60 }, { 12, 75 }, { 13, 90 } },
};

FieldFace *fface_new(void *heap, unsigned short chara)
{
    FieldFace *f = heap_alloc(heap, sizeof(FieldFace));
    int idx;
    unsigned int i;
    const char *path;
    int size;

    if (f != 0) {
        memset(f, 0, sizeof(FieldFace));
        f->heap = heap;
        idx = -1;
        for (i = 0; i < 11; i++) {
            unsigned short id = fface_files[i].id;

            if (id == chara) {
                idx = i;
                break;
            }
        }
        if (idx >= 0) {
            path = fface_files[idx].path;
            size = ehfile_get_size(3, path);
            f->pack = heap_alloc(heap, size);
            ehfile_read_async_alias(3, path, f->pack, size, (int (*)(void *, int))fface_loaded, f);
            f->chara = chara;
            f->face = idx;
        }
    }
    return f;
}

int fface_ready(FieldFace *f)
{
    if (f != 0 && f->loaded)
        return 1;
    return 0;
}

void *fface_draw(void *list, FieldFace *f, int x, int y)
{
    EhfOpts o;
    int frame;

    if (f == 0)
        return list;
    if (!f->loaded)
        return list;
    memset(&o, 0, sizeof(o));
    o.time = *gfx_get_state();
    o.x = x;
    o.flags = 0x10000;
    o.y = y;
    if (f->body != 0) {
        list = gfx_begin_2d(list);
        list = gfx_end_2d(ehf_draw(list, f->body, &o));
    }
    if (f->mouth != 0) {
        void *l = gfx_begin_2d(list);

        list = gfx_end_2d(ehf_draw(l, f->mouth, &o));
    }
    if (f->blink) {
        if (f->count >= f->next) {
            if ((int)ehf_get_frame_count(f->body) % 15 == 14) {
                frame = fface_frame(f->face, f->anim);
                f->count = 0;
                f->next = rand_next_alias() % 0xD2 + 0x5A;
                ehf_set_anim(f->body, frame, frame + 1);
            }
        } else {
            f->count++;
            if (f->count >= f->next) {
                frame = fface_frame(f->face, f->anim);
                ehf_set_anim(f->body, frame + 4, frame + 0xF);
            }
        }
    }
    return list;
}

void fface_set(FieldFace *f, int blink, int talk, int anim)
{
    int anim_changed;
    int talk_changed;
    int frame;

    if (f == 0)
        return;
    anim_changed = anim != f->anim;
    talk_changed = talk != f->talk;
    if (blink != f->blink) {
        f->count = 0;
        if (blink != 0)
            f->next = rand_next_alias() % 0xD2 + 0x5A;
        else
            f->next = 0;
        f->blink = blink;
    }
    if (anim_changed) {
        frame = fface_frame(f->face, anim);
        if (f->body != 0)
            ehf_set_anim(f->body, frame, frame + 1);
        talk_changed = 1;
        f->anim = anim;
    }
    if (talk_changed) {
        frame = fface_frame(f->face, f->anim);
        if (f->mouth != 0) {
            if (talk != 0)
                ehf_set_anim(f->mouth, frame + 4, frame + 0xF);
            else
                ehf_set_anim(f->mouth, frame, frame + 1);
        }
        f->talk = talk;
    }
}

#ifdef NON_MATCHING /* the original copies f into $s1 only after loading the two fields */
void fface_free(FieldFace *f)
{
    void *heap;

    if (f != 0) {
        heap = f->heap;
        if (f->pack != 0)
            heap_set_level(heap, f->pack);
        heap_set_level(heap, f);
    }
}
#endif

unsigned short fface_frame(unsigned short face, unsigned short anim)
{
    FaceAnim *row = fface_anims[face];
    int i;

    for (i = 0; i < 19; i++) {
        if (anim == row[i].id)
            return row[i].frame;
    }
    return row[0].frame;
}

int fface_loaded(FieldFace *f, int size)
{
    int ok = size > 0;

    if (ok) {
        ehp_mount(0, f->pack);
        f->body = ehp_find(f->pack, "body.ehf", 0);
        f->mouth = ehp_find(f->pack, "mouth.ehf", 0);
        f->blink = 1;
        f->talk = 1;
        f->anim = 1;
        fface_set(f, 0, 0, 0);
        f->loaded = 1;
    }
    return ok;
}
