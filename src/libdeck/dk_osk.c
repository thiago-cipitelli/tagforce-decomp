// @range 0x0881A69C-0x0881AB64
// @data 0x08823178-0x08823184
/* System on-screen keyboard (SceUtilityOsk) used to name a deck: start it on a text ('$' is doubled
   in the game's strings and single in the keyboard's), then run it with the fade until it closes and
   copy the result back. */
#include "libdeck.h"

typedef struct DkOskData {             /* SceUtilityOskData */
    int unk00;
    int unk04;
    int language;
    int unk12;
    int inputtype;
    int lines;
    int unk24;
    unsigned short *desc;
    unsigned short *intext;
    int outtextlength;
    unsigned short *outtext;
    int result;
    int outtextlimit;
} DkOskData;

typedef struct DkOskParams {           /* SceUtilityOskParams */
    int size;
    int language;
    int button_swap;
    int graphics_thread;
    int access_thread;
    int font_thread;
    int sound_thread;
    int result;
    int reserved[4];
    int datacount;
    DkOskData *data;
    int state;
    int unk60;
} DkOskParams;

typedef struct DkOsk {                 /* dk_osk 0x095D2734 (.bss), 0x1B0 bytes */
    unsigned char state;               /* +0x00 0 idle, 1 fade in, 2 open, 3 fade out, 4 done */
    unsigned char pad1[3];
    DkOskParams params;                /* +0x04 */
    DkOskData data;                    /* +0x44 */
    unsigned short intext[24];         /* +0x78 */
    unsigned short outtext[128];       /* +0xA8 */
    unsigned short *target;            /* +0x1A8 */
    void (*done)(unsigned short *text, int r); /* +0x1AC */
} DkOsk;

extern DkOsk dk_osk;

int wcslen(const unsigned short *s);
void sysvar_set_a90(int v);
int sceUtilityOskInitStart(void *params);
int sceUtilityOskShutdownStart(void);
int sceUtilityOskUpdate(int n);
int sceUtilityOskGetStatus(void);
void sceGuSync(int mode, int what);

unsigned short dk_osk_lang[6] = { 0, 1, 4, 2, 5, 3 };   /* SceUtility language by cfg_get_language() */

/* starts the keyboard on `text` (at most 23 characters); `done` gets the typed text and the result */
int dk_osk_open(unsigned short *text, unsigned short *desc, int maxlen, void (*done)(unsigned short *, int))
{
    unsigned short *in = dk_osk.intext;
    int lang = cfg_get_language();
    int dollar = 0;
    unsigned int i;
    unsigned short *s;
    unsigned short *o;
    DkOskParams *p;
    DkOskData *d;
    int r;

    if (lang < 0 || lang >= 6)
        lang = 1;
    memset(&dk_osk, 0, sizeof(dk_osk));
    sysvar_set_a90(1);
    dk_osk.target = text;
    dk_osk.done = done;
    for (i = 0, s = text, o = in; i < wcslen(text) && (int)i < 23; s++, i++) {
        unsigned short c = *s;
        if (c == '$') {
            if (dollar) {
                dollar = 0;
                continue;
            }
            dollar = 1;
        }
        *o = c;
        o++;
        if (c == 0)
            break;
    }
    p = (DkOskParams *)(char *)&dk_osk.params;
    p->size = 0x40;
    p->language = dk_osk_lang[lang];
    d = (DkOskData *)(char *)&dk_osk.data;
    if (region_pick_value(0) == 0x2000)
        p->button_swap = 0;
    else
        p->button_swap = 1;
    p->graphics_thread = 0x11;
    p->access_thread = 0x13;
    p->font_thread = 0x12;
    p->sound_thread = 0x10;
    p->reserved[0] = 0;
    p->reserved[1] = 0;
    p->data = d;
    p->datacount = 1;
    if (lang == 0)
        d->unk00 = 1;
    else
        d->unk00 = 0;
    d->lines = 1;
    d->unk24 = 1;
    d->outtext = dk_osk.outtext;
    d->desc = desc;
    d->intext = in;
    d->outtextlength = maxlen;
    d->unk04 = 0;
    d->language = 0;
    d->unk12 = 0;
    d->inputtype = 0;
    d->result = 0;
    d->outtextlimit = 0;
    r = sceUtilityOskInitStart(p);
    if (r == 0) {
        fade_start_in(2, 0xA0000000, 8);
        dk_osk.state = 1;
    }
    return r;
}

/* runs the keyboard (fade on display list layer `layer`); 1 once it is closed */
int dk_osk_update(int layer)
{
    switch (dk_osk.state) {
    case 1:
    case 2:
    case 3: {
        void *dl = dl_select_layer(layer, 0);
        if (fade_apply(&dl)) {
            if (dk_osk.state == 1)
                dk_osk.state = 2;
            if (dk_osk.state == 3)
                dk_osk.state = 4;
        }
        dl_set_list(dl);
    }
    }
    sceGuSync(0, 0);
    switch (dk_osk.state) {
    case 0:
        sysvar_set_a90(0);
        return 1;
    case 1:
    case 2:
        switch (sceUtilityOskGetStatus()) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            if (sceUtilityOskUpdate(2) == 0)
                break;
        case 3: {
            int r = sceUtilityOskShutdownStart();
            if (dk_osk.params.result == 0) {
                unsigned int i;
                int k;
                k = 0;
                i = 0;
                if (wcslen(dk_osk.outtext)) {
                    for (;;) {
                        unsigned short c = dk_osk.outtext[i];
                        if (c == 0)
                            break;
                        if (c == '$')
                            dk_osk.target[k++] = c;
                        dk_osk.target[k++] = c;
                        i++;
                        if (i >= wcslen(dk_osk.outtext))
                            break;
                    }
                }
                dk_osk.target[k] = 0;
            }
            if (dk_osk.done)
                dk_osk.done(dk_osk.outtext, r);
            fade_start(2, 0xA0000000, 8);
            dk_osk.state = 3;
            break;
        }
        case 4:
            break;
        }
        break;
    case 3:
        break;
    case 4:
        sysvar_set_a90(0);
        return 1;
    }
    return 0;
}
