// @range 0x0884D488-0x0884D65C
/* Fade of the duel character (layer 11 draw callback): the model is drawn with an alpha that
   grows over `len` frames after `delay` frames, with the sound 0x6225 when it starts. */
#include "libduel_draw.h"

void *dl_enable(void *list, int state);
void *dl_disable(void *list, int state);
void *dl_ambient_light(void *list, unsigned int color);
float func_088279A4(int t, int len);
int func_08827F68(int se, int t, int at, unsigned int *flags, unsigned int bit);
void *func_088269D8(void *list, int on, int op, int src, int dst, unsigned int fixa, unsigned int fixb);

typedef struct DdFxFade {              /* 0x08C5ABE0 */
    int wait;                          /* +0x00 */
    int delay;                         /* +0x04 */
    int t;                             /* +0x08 */
    int len;                           /* +0x0C */
    unsigned int se;                   /* +0x10 */
} DdFxFade;

extern DdFxFade dd_fx_fade;

void dd_fx_fade_set(int delay, int len)
{
    DdFxFade *const f = &dd_fx_fade;

    f->delay = delay;
    f->len = len;
    f->wait = 0;
    f->t = 0;
    f->se = 0;
}

int dd_fx_fade_cb(void **plist, int a, int layer, int b, int pass)
{
    void *list;
    DdFxFade *f;
    unsigned int alpha;
    int r = 0;

    if (layer == 11) {
        list = *plist;
        f = &dd_fx_fade;
        if (pass == 0) {
            alpha = func_088279A4(f->t, f->len) * 255.0f;
            list = dl_enable(list, 10);
            list = dl_disable(list, 11);
            list = dl_disable(list, 12);
            list = dl_disable(list, 13);
            list = dl_disable(list, 14);
            list = dl_ambient_light(list, alpha << 24 | 0xFFFFFF);
            list = func_088269D8(list, 1, 0, 2, 3, 0, 0xFFFFFFFF);
            func_08827F68(0x6225, f->wait, f->delay, &f->se, 1);
        } else {
            list = dl_disable(list, 10);
            list = dl_ambient_light(list, 0xFFFFFFFF);
            list = func_088269D8(list, 0, 0, 2, 3, 0, 0xFFFFFFFF);
            if (f->wait < f->delay)
                f->wait++;
            else if (f->t < f->len)
                f->t++;
        }
        *plist = list;
        r = 1;
    }
    return r;
}
