// @range 0x0880F438-0x0880F900
// @asm 0x0880F438-0x0880F900
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

typedef struct TOsk {         /* tosk 0x09433D40 (.bss), 0x1B0 bytes */
    unsigned char state;       /* +0x00 0 idle, 1 fade in, 2 open, 3 fade out, 4 done */
    unsigned char pad1[3];
    struct {                   /* +0x04 SceUtilityOskParams */
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
    } params;
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

#ifdef NON_MATCHING /* mesma sequência de stores dos parâmetros do SceUtilityOsk em outra ordem e registradores */
/* starts the on-screen keyboard on `text` ('$' are doubled in the game's strings) */
int tosk_open(unsigned short *text, unsigned short *desc, int maxlen, void (*done)(unsigned short *, int))
{
    unsigned short *out = tosk.intext;
    int lang = cfg_get_language();
    int dollar = 0;
    unsigned int i;
    unsigned short *s;
    int r;

    if (lang < 0 || lang > 5)
        lang = 1;
    memset(&tosk, 0, sizeof(tosk));
    sysvar_set_a90(1);
    s = text;
    tosk.target = text;
    tosk.done = done;
    for (i = 0; i < wcslen(text) && (int)i < 23; i++) {
        unsigned short c = *s;
        if (c == '$') {
            if (dollar) {
                dollar = 0;
            } else {
                dollar = 1;
                *out++ = '$';
            }
        } else {
            *out++ = c;
            if (c == 0)
                break;
        }
        s++;
    }
    tosk.params.size = 0x40;
    tosk.params.language = tosk_lang[lang];
    tosk.params.button_swap = region_pick_value(0) != 0x2000;
    tosk.params.graphics_thread = 0x11;
    tosk.params.access_thread = 0x13;
    tosk.params.font_thread = 0x12;
    tosk.params.sound_thread = 0x10;
    tosk.params.reserved[0] = 0;
    tosk.params.reserved[1] = 0;
    tosk.params.data = &tosk.data;
    tosk.params.datacount = 1;
    tosk.data.unk00 = lang == 0;
    tosk.data.lines = 1;
    tosk.data.unk24 = 1;
    tosk.data.outtext = tosk.outtext;
    tosk.data.intext = tosk.intext;
    tosk.data.unk04 = 0;
    tosk.data.language = 0;
    tosk.data.unk12 = 0;
    tosk.data.inputtype = 0;
    tosk.data.result = 0;
    tosk.data.outtextlimit = 0;
    tosk.data.desc = desc;
    tosk.data.outtextlength = maxlen;
    r = sceUtilityOskInitStart(&tosk.params);
    if (r == 0) {
        fade_start_in(2, 0xA0000000, 8);
        tosk.state = 1;
    }
    return r;
}
#endif

#ifdef NON_MATCHING /* laço de cópia do texto e testes de estado saem com outros desvios */
/* runs the keyboard on display list layer `layer`; 1 when it is closed */
int tosk_update(int layer)
{
    if (tosk.state == 3 || tosk.state == 2 || tosk.state == 1) {
        void *dl = dl_select_layer(layer, 0);
        if (fade_apply(&dl)) {
            if (tosk.state == 1)
                tosk.state = 2;
            if (tosk.state == 3)
                tosk.state = 4;
        }
        dl_set_list(dl);
    }
    sceGuSync(0, 0);
    if (tosk.state == 4) {
        sysvar_set_a90(0);
        return 1;
    }
    if (tosk.state == 3)
        return 0;
    if (tosk.state == 2 || tosk.state == 1) {
        int st = sceUtilityOskGetStatus();
        if (st != 4 && (st == 3 || (st == 2 && sceUtilityOskUpdate(1) != 0))) {
            int r = sceUtilityOskShutdownStart();
            if (tosk.params.result == 0) {
                int k = 0;
                unsigned int i = 0;
                if (wcslen(tosk.outtext)) {
                    unsigned short *s = tosk.outtext;
                    unsigned short c = *s;
                    while (c) {
                        if (c == '$')
                            tosk.target[k++] = '$';
                        tosk.target[k++] = c;
                        i++;
                        if (i >= wcslen(tosk.outtext))
                            break;
                        c = *++s;
                    }
                }
                tosk.target[k] = 0;
            }
            if (tosk.done)
                tosk.done(tosk.outtext, r);
            fade_start(2, 0xA0000000, 8);
            tosk.state = 3;
        }
    } else if (tosk.state == 0) {
        sysvar_set_a90(0);
        return 1;
    }
    return 0;
}
#endif

