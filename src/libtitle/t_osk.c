// @range 0x0880F438-0x0880F900
// @data 0x0882E7F0-0x0882E7FC
/* On-screen keyboard of the title (the same code as librecipe_viewer's and libselect_card's): opens
   the SceUtility OSK on a text and copies the result back when it is closed. */
#include "libtitle.h"

typedef struct TOskData {     /* SceUtilityOskData */
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
} TOskData;

typedef struct TOskParams {           /* SceUtilityOskParams */
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
    TOskData *data;
    int state;
    int unk60;
} TOskParams;

typedef struct TOsk {         /* tosk 0x09433CC8 (.bss), 0x1B0 bytes */
    unsigned char state;       /* +0x00 0 idle, 1 fade in, 2 open, 3 fade out, 4 done */
    unsigned char pad1[3];
    TOskParams params;         /* +0x04 */
    TOskData data;            /* +0x44 */
    unsigned short intext[24]; /* +0x78 */
    unsigned short outtext[128]; /* +0xA8 */
    unsigned short *target;    /* +0x1A8 */
    void (*done)(unsigned short *text, int r); /* +0x1AC */
} TOsk;

extern TOsk tosk;

int sceUtilityOskInitStart(void *params);
int sceUtilityOskShutdownStart(void);
int sceUtilityOskUpdate(int n);
int sceUtilityOskGetStatus(void);
void sceGuSync(int mode, int what);

unsigned short tosk_lang[6] = { 0, 1, 4, 2, 5, 3 };   /* SceUtility language by cfg_get_language() */

/* starts the keyboard on `text` (at most 23 characters); `done` gets the typed text and the result */
int tosk_open(unsigned short *text, unsigned short *desc, int maxlen, void (*done)(unsigned short *, int))
{
    unsigned short *in = tosk.intext;
    int lang = cfg_get_language();
    int dollar = 0;
    unsigned int i;
    unsigned short *s;
    unsigned short *o;
    TOskParams *p;
    TOskData *d;
    int r;

    if (lang < 0 || lang >= 6)
        lang = 1;
    memset(&tosk, 0, sizeof(tosk));
    sysvar_set_a90(1);
    tosk.target = text;
    tosk.done = done;
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
    p = (TOskParams *)(char *)&tosk.params;
    p->size = 0x40;
    p->language = tosk_lang[lang];
    d = (TOskData *)(char *)&tosk.data;
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
    d->outtext = tosk.outtext;
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
        tosk.state = 1;
    }
    return r;
}

/* runs the keyboard (fade on display list layer `layer`); 1 once it is closed */
int tosk_update(int layer)
{
    switch (tosk.state) {
    case 1:
    case 2:
    case 3: {
        void *dl = dl_select_layer(layer, 0);
        if (fade_apply(&dl)) {
            if (tosk.state == 1)
                tosk.state = 2;
            if (tosk.state == 3)
                tosk.state = 4;
        }
        dl_set_list(dl);
    }
    }
    sceGuSync(0, 0);
    switch (tosk.state) {
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
            if (tosk.params.result == 0) {
                unsigned int i;
                int k;
                k = 0;
                i = 0;
                if (wcslen(tosk.outtext)) {
                    for (;;) {
                        unsigned short c = tosk.outtext[i];
                        if (c == 0)
                            break;
                        if (c == '$')
                            tosk.target[k++] = c;
                        tosk.target[k++] = c;
                        i++;
                        if (i >= wcslen(tosk.outtext))
                            break;
                    }
                }
                tosk.target[k] = 0;
            }
            if (tosk.done)
                tosk.done(tosk.outtext, r);
            fade_start(2, 0xA0000000, 8);
            tosk.state = 3;
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
