// @range 0x0884BDB0-0x0884D488
/* Card effect "aura" task (halo + spell circle of dd_fx_aura.c) and the sound table of the card
   effects: the sounds 0x6225-0x623C to play at given frames of each effect id, some depending on
   the duelist (chara) and its variant. */
#include "libduel_draw.h"

int dd_cue_se(int se, int t, int at, unsigned int *flags, unsigned int bit);
int dd_task_create(void *update, void *a, void *b, int size);
void *dd_task_mem(int task);
int dd_fx_aura_halo(int phase, DdFxHalo *w, DdFxArg *arg);
int dd_fx_aura_circle(int phase, DdFxCircle *w, DdFxArg *arg);

typedef struct DdFxAura {              /* 0x430 */
    int t;                             /* +0x000 */
    int frame;                         /* +0x004 */
    DdFxArg *arg;                      /* +0x008 */
    unsigned int se;                   /* +0x00C */
    DdFxHalo halo;                     /* +0x010 */
    DdFxCircle circle;                 /* +0x2B0 */
} DdFxAura;

int dd_fx_aura_update(int phase, DdFxAura *w);

/* Plays the sounds of effect `id` at frame `t` (each once, bits of `flags`). */
void dd_fx_card_se(int chara, int variant, int id, int t, unsigned int *flags)
{
    int at;

    switch (id) {
    case 0:
        dd_cue_se(0x6232, t, 0x1A, flags, 1);
        dd_cue_se(0x6233, t, 0x2C, flags, 2);
        break;
    case 1:
        dd_cue_se(0x6232, t, 0x24, flags, 1);
        break;
    case 2:
        dd_cue_se(0x6232, t, 0x12, flags, 1);
        dd_cue_se(0x6233, t, 0x21, flags, 2);
        break;
    case 3:
        dd_cue_se(0x6232, t, 10, flags, 1);
        break;
    case 4:
        dd_cue_se(0x6233, t, 0xF, flags, 1);
        break;
    case 5:
        dd_cue_se(0x6232, t, 0x21, flags, 1);
        break;
    case 6:
        dd_cue_se(0x6232, t, 0xD, flags, 1);
        break;
    case 7:
        dd_cue_se(0x6232, t, 0x12, flags, 1);
        break;
    case 8:
        dd_cue_se(0x6234, t, 0x14, flags, 1);
        break;
    case 9:
        dd_cue_se(0x6233, t, 0x20, flags, 1);
        break;
    case 10:
        dd_cue_se(0x6233, t, 0x23, flags, 1);
        break;
    case 11:
        dd_cue_se(0x6234, t, 0x22, flags, 1);
        break;
    case 12:
        dd_cue_se(0x6234, t, 0x2B, flags, 1);
        break;
    case 24:
        dd_cue_se(0x6225, t, 0x1C, flags, 1);
        break;
    case 25:
        dd_cue_se(0x6225, t, 0x23, flags, 1);
        break;
    case 26:
        dd_cue_se(0x6225, t, 0x21, flags, 1);
        break;
    case 42:
        dd_cue_se(0x6232, t, 0x12, flags, 1);
        break;
    case 43:
        dd_cue_se(0x6232, t, 0x10, flags, 1);
        break;
    case 44:
    case 45:
        dd_cue_se(0x6232, t, 0xE, flags, 1);
        break;
    case 46:
        dd_cue_se(0x6233, t, 0xB, flags, 1);
        break;
    case 47:
        dd_cue_se(0x6234, t, 0xF, flags, 1);
        break;
    case 48:
        dd_cue_se(0x6234, t, 0xE, flags, 1);
        break;
    case 51:
        dd_cue_se(0x6238, t, 0x24, flags, 1);
        break;
    case 57:
        if (chara == 0x15) {
          dd_cue_se(0x6232, t, 0xB, flags, 1);
        }
        break;
    case 63:
        dd_cue_se(0x6232, t, 0x13, flags, 1);
        break;
    case 64:
        dd_cue_se(0x6233, t, 0x15, flags, 1);
        break;
    case 65:
        dd_cue_se(0x6232, t, 0x10, flags, 1);
        dd_cue_se(0x6233, t, 0x22, flags, 2);
        dd_cue_se(0x6234, t, 0x55, flags, 4);
        break;
    case 66:
        dd_cue_se(0x6232, t, 0x46, flags, 1);
        break;
    case 67:
        dd_cue_se(0x6233, t, 0x11, flags, 1);
        dd_cue_se(0x6232, t, 0x49, flags, 2);
        break;
    case 68:
    case 69:
        dd_cue_se(0x6232, t, 0x49, flags, 1);
        break;
    case 70:
        dd_cue_se(0x6234, t, 0x5C, flags, 1);
        break;
    case 71:
        dd_cue_se(0x6232, t, 0x4B, flags, 1);
        break;
    case 72:
        dd_cue_se(0x6233, t, 0x11, flags, 1);
        dd_cue_se(0x6232, t, 0x46, flags, 2);
        break;
    case 73:
        dd_cue_se(0x6232, t, 0x4E, flags, 1);
        break;
    case 74:
        dd_cue_se(0x6232, t, 0x11, flags, 1);
        dd_cue_se(0x6234, t, 0x49, flags, 2);
        break;
    case 77:
        dd_cue_se(0x6232, t, 0x52, flags, 1);
        break;
    case 78:
        dd_cue_se(0x6232, t, 0x1B, flags, 1);
        break;
    case 79:
        dd_cue_se(0x6233, t, 0x10, flags, 1);
        break;
    case 82:
        dd_cue_se(0x6232, t, 0x49, flags, 1);
        break;
    case 83:
        if (chara == 4) {
          dd_cue_se(0x6233, t, 0x1D, flags, 1);
        }
        else {
          dd_cue_se(0x6233, t, 0x17, flags, 1);
        }
        dd_cue_se(0x6232, t, 0x5A, flags, 2);
        break;
    case 84:
        dd_cue_se(0x6233, t, 0x1E, flags, 1);
        if (chara == 7) {
          dd_cue_se(0x6232, t, 0x5F, flags, 2);
        }
        else {
          dd_cue_se(0x6232, t, 0x67, flags, 2);
        }
        break;
    case 85:
        dd_cue_se(0x6232, t, 0x46, flags, 1);
        break;
    case 86:
        dd_cue_se(0x6233, t, 0x73, flags, 1);
        break;
    case 87:
        dd_cue_se(0x6233, t, 0x66, flags, 1);
        break;
    case 88:
        dd_cue_se(0x6232, t, 0x48, flags, 1);
        break;
    case 89:
        dd_cue_se(0x6232, t, 0x46, flags, 1);
        break;
    case 90:
        dd_cue_se(0x6233, t, 0x1F, flags, 1);
        dd_cue_se(0x6232, t, 0x68, flags, 2);
        break;
    case 91:
        dd_cue_se(0x6232, t, 0xB, flags, 1);
        break;
    case 107:
    case 108:
        at = 24;
        switch (chara) {
        case 13:
            at = 32;
            break;
        case 14:
        case 31:
            at = 28;
            break;
        case 15:
            at = 42;
            break;
        case 18:
            if (variant == 0)
                at = 28;
            break;
        case 19:
            if (id == 107)
                at = 42;
            else
                at = 34;
            break;
        }
        dd_cue_se(0x6228, t, at, flags, 1);
        break;
    case 109:
    case 110:
        dd_cue_se(0x6228, t, 0x37, flags, 1);
        break;
    case 111:
    case 112:
        dd_cue_se(0x6228, t, 0x21, flags, 1);
        break;
    case 115:
    case 116:
    case 117:
        dd_cue_se(0x622F, t, 0xF, flags, 1);
        break;
    case 118:
        dd_cue_se(0x6232, t, 10, flags, 1);
        break;
    case 247:
        dd_cue_se(0x6233, t, 0x16, flags, 1);
        break;
    case 248:
        dd_cue_se(0x6232, t, 0x1C, flags, 1);
        dd_cue_se(0x623A, t, 0x21, flags, 2);
        dd_cue_se(0x6234, t, 0x52, flags, 4);
        break;
    case 251:
        dd_cue_se(0x624B, t, 5, flags, 1);
        dd_cue_se(0x624C, t, 0x14, flags, 2);
        dd_cue_se(0x624B, t, 0x2A, flags, 4);
        dd_cue_se(0x624C, t, 0x39, flags, 8);
        break;
    case 253:
        dd_cue_se(0x6233, t, 0x18, flags, 1);
        break;
    case 254:
        dd_cue_se(0x6233, t, 0x23, flags, 1);
        break;
    case 255:
        dd_cue_se(0x6233, t, 0x6A, flags, 1);
        break;
    case 261:
        dd_cue_se(0x6235, t, 0x79, flags, 1);
        dd_cue_se(0x6234, t, 0xAA, flags, 2);
        break;
    case 270:
        dd_cue_se(0x6233, t, 0x1C, flags, 1);
        break;
    case 271:
        dd_cue_se(0x6233, t, 0x21, flags, 1);
        dd_cue_se(0x624D, t, 0x26, flags, 2);
        dd_cue_bgm_stop(0x624D, t, 0x62, flags, 4);
        dd_cue_se(0x6232, t, 0x6A, flags, 8);
        break;
    case 272:
        dd_cue_se(0x6232, t, 0x6D, flags, 1);
        break;
    case 273:
        dd_cue_se(0x6232, t, 0x55, flags, 1);
        break;
    case 275:
        dd_cue_se(0x6232, t, 0x6D, flags, 1);
        break;
    case 276:
        dd_cue_se(0x6234, t, 0x62, flags, 1);
        break;
    case 277:
        dd_cue_se(0x6234, t, 0x54, flags, 1);
        break;
    case 278:
        dd_cue_se(0x6232, t, 0x66, flags, 1);
        break;
    case 279:
        dd_cue_se(0x6233, t, 0x9B, flags, 1);
        break;
    case 282:
        dd_cue_se(0x6232, t, 0x66, flags, 1);
        break;
    case 285:
        dd_cue_se(0x6232, t, 0x2E, flags, 1);
        break;
    case 286:
        dd_cue_se(0x6232, t, 0x28, flags, 1);
        break;
    case 215:
        dd_cue_se(0x6232, t, 0x35, flags, 1);
        break;
    case 239:
        dd_cue_se(0x6233, t, 0x80, flags, 1);
        break;
    case 290:
        dd_cue_se(0x623C, t, 0x17, flags, 1);
        break;
    case 291:
        dd_cue_se(0x623C, t, 0x20, flags, 1);
        break;
    case 292:
        dd_cue_se(0x623C, t, 0x32, flags, 1);
        break;
    case 293:
    case 301:
        dd_cue_se(0x623C, t, 0x22, flags, 1);
        break;
    case 296:
        dd_cue_se(0x623C, t, 0x1D, flags, 1);
        break;
    case 297:
        dd_cue_se(0x623C, t, 0x37, flags, 1);
        break;
    case 298:
    case 331:
        dd_cue_se(0x623C, t, 0x1C, flags, 1);
        break;
    case 299:
    case 306:
    case 323:
        dd_cue_se(0x623C, t, 0x25, flags, 1);
        break;
    case 300:
    case 322:
        dd_cue_se(0x623C, t, 0x28, flags, 1);
        break;
    case 302:
        dd_cue_se(0x623C, t, 0x27, flags, 1);
        break;
    case 304:
    case 305:
    case 309:
    case 310:
    case 315:
    case 316:
    case 317:
    case 318:
    case 319:
    case 324:
    case 325:
    case 327:
    case 329:
    case 332:
        dd_cue_se(0x623C, t, 0x23, flags, 1);
        break;
    case 307:
    case 311:
    case 313:
    case 330:
        dd_cue_se(0x623C, t, 0x1A, flags, 1);
        break;
    case 308:
    case 312:
        dd_cue_se(0x623C, t, 0x16, flags, 1);
        break;
    case 320:
    case 321:
        dd_cue_se(0x623C, t, 0x1B, flags, 1);
        break;
    case 326:
        dd_cue_se(0x623C, t, 0x3C, flags, 1);
        break;
    case 328:
        dd_cue_se(0x623C, t, 0x2D, flags, 1);
        break;
    case 335:
        dd_cue_se(0x6232, t, 0x15, flags, 1);
        dd_cue_se(0x6233, t, 0x50, flags, 2);
        break;
    case 340:
        dd_cue_se(0x6233, t, 0x5E, flags, 1);
        break;
    case 341:
        dd_cue_se(0x6234, t, 0x6E, flags, 1);
        break;
    case 344:
        dd_cue_se(0x6232, t, 0xD4, flags, 1);
        break;
    case 345:
        dd_cue_se(0x6232, t, 0x9A, flags, 1);
        break;
    case 347:
        dd_cue_se(0x6234, t, 0xC9, flags, 1);
        break;
    case 348:
        dd_cue_se(0x6234, t, 0x8A, flags, 1);
        dd_cue_se(0x6232, t, 0xE1, flags, 2);
        break;
    case 352:
        dd_cue_se(0x6234, t, 0x98, flags, 1);
        break;
    case 357:
        dd_cue_se(0x6233, t, 0xE, flags, 1);
        dd_cue_se(0x6232, t, 0x11D, flags, 2);
        break;
    case 358:
        dd_cue_se(0x6232, t, 0x5A, flags, 1);
        break;
    case 360:
        dd_cue_se(0x6234, t, 0x73, flags, 1);
        break;
    case 364:
        dd_cue_se(0x6234, t, 0x29, flags, 1);
        break;
    case 365:
        dd_cue_se(0x6233, t, 0x89, flags, 1);
        break;
    case 367:
        dd_cue_se(0x6233, t, 0x5C, flags, 1);
        break;
    case 368:
        dd_cue_se(0x6232, t, 100, flags, 1);
        break;
    case 369:
        dd_cue_se(0x6233, t, 0x4F, flags, 1);
        dd_cue_se(0x6232, t, 0xB4, flags, 2);
        dd_cue_se(0x6234, t, 0xD9, flags, 4);
        dd_cue_se(0x6233, t, 0xF0, flags, 8);
        dd_cue_se(0x6232, t, 0x109, flags, 0x10);
        dd_cue_se(0x6234, t, 0x13E, flags, 0x20);
        break;
    case 372:
        dd_cue_se(0x6233, t, 0x4E, flags, 1);
        break;
    }
}

int dd_fx_aura_start(DdFxArg *arg, void *a, void *b)
{
    int task = dd_task_create(dd_fx_aura_update, a, b, sizeof(DdFxAura));

    if (task != -1)
        ((DdFxAura *)dd_task_mem(task))->arg = arg;
    return task;
}

int dd_fx_aura_update(int phase, DdFxAura *w)
{
    DdFxArg *arg = w->arg;
    int run = arg->run;

    dd_fx_aura_halo(phase, &w->halo, arg);
    dd_fx_aura_circle(phase, &w->circle, w->arg);
    dd_fx_card_se(((int *)arg)[2], ((int *)arg)[3], arg->id, w->t, &w->se);
    w->frame++;
    if (run == 1)
        w->t++;
    return 2;
}
