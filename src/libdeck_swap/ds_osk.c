// @range 0x08815318-0x088157E0
// @data 0x0881A3D8-0x0881A3E4
/* System on-screen keyboard (SceUtilityOsk) used to name a deck: start it on a text ('$' is doubled
   in the game's strings and single in the keyboard's), then run it with the fade until it closes and
   copy the result back. */
#include "libdeck_swap.h"

typedef struct DsOskData {             /* SceUtilityOskData */
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
} DsOskData;

typedef struct DsOskParams {           /* SceUtilityOskParams */
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
    DsOskData *data;
    int state;
    int unk60;
} DsOskParams;

typedef struct DsOsk {                 /* ds_osk 0x0881A428 (.bss), 0x1B0 bytes */
    unsigned char state;               /* +0x00 0 idle, 1 fade in, 2 open, 3 fade out, 4 done */
    unsigned char pad1[3];
    DsOskParams params;                /* +0x04 */
    DsOskData data;                    /* +0x44 */
    unsigned short intext[24];         /* +0x78 */
    unsigned short outtext[128];       /* +0xA8 */
    unsigned short *target;            /* +0x1A8 */
    void (*done)(unsigned short *text, int r); /* +0x1AC */
} DsOsk;

extern DsOsk ds_osk;

int wcslen(const unsigned short *s);
void sysvar_set_a90(int v);
int sceUtilityOskInitStart(void *params);
int sceUtilityOskShutdownStart(void);
int sceUtilityOskUpdate(int n);
int sceUtilityOskGetStatus(void);
void sceGuSync(int mode, int what);

unsigned short ds_osk_lang[6] = { 0, 1, 4, 2, 5, 3 };   /* SceUtility language by cfg_get_language() */

/* starts the keyboard on `text` (at most 23 characters); `done` gets the typed text and the result */
int ds_osk_open(unsigned short *text, unsigned short *desc, int maxlen, void (*done)(unsigned short *, int))
{
    unsigned short *in = ds_osk.intext;
    int lang = cfg_get_language();
    int dollar = 0;
    unsigned int i;
    unsigned short *s;
    unsigned short *o;
    DsOskParams *p;
    DsOskData *d;
    int r;

    if (lang < 0 || lang >= 6)
        lang = 1;
    memset(&ds_osk, 0, sizeof(ds_osk));
    sysvar_set_a90(1);
    ds_osk.target = text;
    ds_osk.done = done;
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
    p = (DsOskParams *)(char *)&ds_osk.params;
    p->size = 0x40;
    p->language = ds_osk_lang[lang];
    d = (DsOskData *)(char *)&ds_osk.data;
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
    d->outtext = ds_osk.outtext;
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
        ds_osk.state = 1;
    }
    return r;
}

/* runs the keyboard (fade on display list layer `layer`); 1 once it is closed */
int ds_osk_update(int layer)
{
    switch (ds_osk.state) {
    case 1:
    case 2:
    case 3: {
        void *dl = dl_select_layer(layer, 0);
        if (fade_apply(&dl)) {
            if (ds_osk.state == 1)
                ds_osk.state = 2;
            if (ds_osk.state == 3)
                ds_osk.state = 4;
        }
        dl_set_list(dl);
    }
    }
    sceGuSync(0, 0);
    switch (ds_osk.state) {
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
            if (ds_osk.params.result == 0) {
                unsigned int i;
                int k;
                k = 0;
                i = 0;
                if (wcslen(ds_osk.outtext)) {
                    for (;;) {
                        unsigned short c = ds_osk.outtext[i];
                        if (c == 0)
                            break;
                        if (c == '$')
                            ds_osk.target[k++] = c;
                        ds_osk.target[k++] = c;
                        i++;
                        if (i >= wcslen(ds_osk.outtext))
                            break;
                    }
                }
                ds_osk.target[k] = 0;
            }
            if (ds_osk.done)
                ds_osk.done(ds_osk.outtext, r);
            fade_start(2, 0xA0000000, 8);
            ds_osk.state = 3;
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
