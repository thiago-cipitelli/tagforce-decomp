// @range 0x0884BDB0-0x0884D488
/* Card effect "aura" task (halo + spell circle of dd_fx_aura.c) and the sound table of the card
   effects: the sounds 0x6225-0x623C to play at given frames of each effect id, some depending on
   the duelist (chara) and its variant. */
#include "libduel_draw.h"

int func_08827F68(int se, int t, int at, unsigned int *flags, unsigned int bit);
int func_0882623C(void *update, void *a, void *b, int size);
void *func_08826440(int task);
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
        func_08827F68(0x6232, t, 0x1A, flags, 1);
        func_08827F68(0x6233, t, 0x2C, flags, 2);
        break;
    case 1:
        func_08827F68(0x6232, t, 0x24, flags, 1);
        break;
    case 2:
        func_08827F68(0x6232, t, 0x12, flags, 1);
        func_08827F68(0x6233, t, 0x21, flags, 2);
        break;
    case 3:
        func_08827F68(0x6232, t, 10, flags, 1);
        break;
    case 4:
        func_08827F68(0x6233, t, 0xF, flags, 1);
        break;
    case 5:
        func_08827F68(0x6232, t, 0x21, flags, 1);
        break;
    case 6:
        func_08827F68(0x6232, t, 0xD, flags, 1);
        break;
    case 7:
        func_08827F68(0x6232, t, 0x12, flags, 1);
        break;
    case 8:
        func_08827F68(0x6234, t, 0x14, flags, 1);
        break;
    case 9:
        func_08827F68(0x6233, t, 0x20, flags, 1);
        break;
    case 10:
        func_08827F68(0x6233, t, 0x23, flags, 1);
        break;
    case 11:
        func_08827F68(0x6234, t, 0x22, flags, 1);
        break;
    case 12:
        func_08827F68(0x6234, t, 0x2B, flags, 1);
        break;
    case 24:
        func_08827F68(0x6225, t, 0x1C, flags, 1);
        break;
    case 25:
        func_08827F68(0x6225, t, 0x23, flags, 1);
        break;
    case 26:
        func_08827F68(0x6225, t, 0x21, flags, 1);
        break;
    case 42:
        func_08827F68(0x6232, t, 0x12, flags, 1);
        break;
    case 43:
        func_08827F68(0x6232, t, 0x10, flags, 1);
        break;
    case 44:
    case 45:
        func_08827F68(0x6232, t, 0xE, flags, 1);
        break;
    case 46:
        func_08827F68(0x6233, t, 0xB, flags, 1);
        break;
    case 47:
        func_08827F68(0x6234, t, 0xF, flags, 1);
        break;
    case 48:
        func_08827F68(0x6234, t, 0xE, flags, 1);
        break;
    case 51:
        func_08827F68(0x6238, t, 0x24, flags, 1);
        break;
    case 57:
        if (chara == 0x15) {
          func_08827F68(0x6232, t, 0xB, flags, 1);
        }
        break;
    case 63:
        func_08827F68(0x6232, t, 0x13, flags, 1);
        break;
    case 64:
        func_08827F68(0x6233, t, 0x15, flags, 1);
        break;
    case 65:
        func_08827F68(0x6232, t, 0x10, flags, 1);
        func_08827F68(0x6233, t, 0x22, flags, 2);
        func_08827F68(0x6234, t, 0x55, flags, 4);
        break;
    case 66:
        func_08827F68(0x6232, t, 0x46, flags, 1);
        break;
    case 67:
        func_08827F68(0x6233, t, 0x11, flags, 1);
        func_08827F68(0x6232, t, 0x49, flags, 2);
        break;
    case 68:
    case 69:
        func_08827F68(0x6232, t, 0x49, flags, 1);
        break;
    case 70:
        func_08827F68(0x6234, t, 0x5C, flags, 1);
        break;
    case 71:
        func_08827F68(0x6232, t, 0x4B, flags, 1);
        break;
    case 72:
        func_08827F68(0x6233, t, 0x11, flags, 1);
        func_08827F68(0x6232, t, 0x46, flags, 2);
        break;
    case 73:
        func_08827F68(0x6232, t, 0x4E, flags, 1);
        break;
    case 74:
        func_08827F68(0x6232, t, 0x11, flags, 1);
        func_08827F68(0x6234, t, 0x49, flags, 2);
        break;
    case 77:
        func_08827F68(0x6232, t, 0x52, flags, 1);
        break;
    case 78:
        func_08827F68(0x6232, t, 0x1B, flags, 1);
        break;
    case 79:
        func_08827F68(0x6233, t, 0x10, flags, 1);
        break;
    case 82:
        func_08827F68(0x6232, t, 0x49, flags, 1);
        break;
    case 83:
        if (chara == 4) {
          func_08827F68(0x6233, t, 0x1D, flags, 1);
        }
        else {
          func_08827F68(0x6233, t, 0x17, flags, 1);
        }
        func_08827F68(0x6232, t, 0x5A, flags, 2);
        break;
    case 84:
        func_08827F68(0x6233, t, 0x1E, flags, 1);
        if (chara == 7) {
          func_08827F68(0x6232, t, 0x5F, flags, 2);
        }
        else {
          func_08827F68(0x6232, t, 0x67, flags, 2);
        }
        break;
    case 85:
        func_08827F68(0x6232, t, 0x46, flags, 1);
        break;
    case 86:
        func_08827F68(0x6233, t, 0x73, flags, 1);
        break;
    case 87:
        func_08827F68(0x6233, t, 0x66, flags, 1);
        break;
    case 88:
        func_08827F68(0x6232, t, 0x48, flags, 1);
        break;
    case 89:
        func_08827F68(0x6232, t, 0x46, flags, 1);
        break;
    case 90:
        func_08827F68(0x6233, t, 0x1F, flags, 1);
        func_08827F68(0x6232, t, 0x68, flags, 2);
        break;
    case 91:
        func_08827F68(0x6232, t, 0xB, flags, 1);
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
        func_08827F68(0x6228, t, at, flags, 1);
        break;
    case 109:
    case 110:
        func_08827F68(0x6228, t, 0x37, flags, 1);
        break;
    case 111:
    case 112:
        func_08827F68(0x6228, t, 0x21, flags, 1);
        break;
    case 115:
    case 116:
    case 117:
        func_08827F68(0x622F, t, 0xF, flags, 1);
        break;
    case 118:
        func_08827F68(0x6232, t, 10, flags, 1);
        break;
    case 247:
        func_08827F68(0x6233, t, 0x16, flags, 1);
        break;
    case 248:
        func_08827F68(0x6232, t, 0x1C, flags, 1);
        func_08827F68(0x623A, t, 0x21, flags, 2);
        func_08827F68(0x6234, t, 0x52, flags, 4);
        break;
    case 251:
        func_08827F68(0x624B, t, 5, flags, 1);
        func_08827F68(0x624C, t, 0x14, flags, 2);
        func_08827F68(0x624B, t, 0x2A, flags, 4);
        func_08827F68(0x624C, t, 0x39, flags, 8);
        break;
    case 253:
        func_08827F68(0x6233, t, 0x18, flags, 1);
        break;
    case 254:
        func_08827F68(0x6233, t, 0x23, flags, 1);
        break;
    case 255:
        func_08827F68(0x6233, t, 0x6A, flags, 1);
        break;
    case 261:
        func_08827F68(0x6235, t, 0x79, flags, 1);
        func_08827F68(0x6234, t, 0xAA, flags, 2);
        break;
    case 270:
        func_08827F68(0x6233, t, 0x1C, flags, 1);
        break;
    case 271:
        func_08827F68(0x6233, t, 0x21, flags, 1);
        func_08827F68(0x624D, t, 0x26, flags, 2);
        func_088280B4(0x624D, t, 0x62, flags, 4);
        func_08827F68(0x6232, t, 0x6A, flags, 8);
        break;
    case 272:
        func_08827F68(0x6232, t, 0x6D, flags, 1);
        break;
    case 273:
        func_08827F68(0x6232, t, 0x55, flags, 1);
        break;
    case 275:
        func_08827F68(0x6232, t, 0x6D, flags, 1);
        break;
    case 276:
        func_08827F68(0x6234, t, 0x62, flags, 1);
        break;
    case 277:
        func_08827F68(0x6234, t, 0x54, flags, 1);
        break;
    case 278:
        func_08827F68(0x6232, t, 0x66, flags, 1);
        break;
    case 279:
        func_08827F68(0x6233, t, 0x9B, flags, 1);
        break;
    case 282:
        func_08827F68(0x6232, t, 0x66, flags, 1);
        break;
    case 285:
        func_08827F68(0x6232, t, 0x2E, flags, 1);
        break;
    case 286:
        func_08827F68(0x6232, t, 0x28, flags, 1);
        break;
    case 215:
        func_08827F68(0x6232, t, 0x35, flags, 1);
        break;
    case 239:
        func_08827F68(0x6233, t, 0x80, flags, 1);
        break;
    case 290:
        func_08827F68(0x623C, t, 0x17, flags, 1);
        break;
    case 291:
        func_08827F68(0x623C, t, 0x20, flags, 1);
        break;
    case 292:
        func_08827F68(0x623C, t, 0x32, flags, 1);
        break;
    case 293:
    case 301:
        func_08827F68(0x623C, t, 0x22, flags, 1);
        break;
    case 296:
        func_08827F68(0x623C, t, 0x1D, flags, 1);
        break;
    case 297:
        func_08827F68(0x623C, t, 0x37, flags, 1);
        break;
    case 298:
    case 331:
        func_08827F68(0x623C, t, 0x1C, flags, 1);
        break;
    case 299:
    case 306:
    case 323:
        func_08827F68(0x623C, t, 0x25, flags, 1);
        break;
    case 300:
    case 322:
        func_08827F68(0x623C, t, 0x28, flags, 1);
        break;
    case 302:
        func_08827F68(0x623C, t, 0x27, flags, 1);
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
        func_08827F68(0x623C, t, 0x23, flags, 1);
        break;
    case 307:
    case 311:
    case 313:
    case 330:
        func_08827F68(0x623C, t, 0x1A, flags, 1);
        break;
    case 308:
    case 312:
        func_08827F68(0x623C, t, 0x16, flags, 1);
        break;
    case 320:
    case 321:
        func_08827F68(0x623C, t, 0x1B, flags, 1);
        break;
    case 326:
        func_08827F68(0x623C, t, 0x3C, flags, 1);
        break;
    case 328:
        func_08827F68(0x623C, t, 0x2D, flags, 1);
        break;
    case 335:
        func_08827F68(0x6232, t, 0x15, flags, 1);
        func_08827F68(0x6233, t, 0x50, flags, 2);
        break;
    case 340:
        func_08827F68(0x6233, t, 0x5E, flags, 1);
        break;
    case 341:
        func_08827F68(0x6234, t, 0x6E, flags, 1);
        break;
    case 344:
        func_08827F68(0x6232, t, 0xD4, flags, 1);
        break;
    case 345:
        func_08827F68(0x6232, t, 0x9A, flags, 1);
        break;
    case 347:
        func_08827F68(0x6234, t, 0xC9, flags, 1);
        break;
    case 348:
        func_08827F68(0x6234, t, 0x8A, flags, 1);
        func_08827F68(0x6232, t, 0xE1, flags, 2);
        break;
    case 352:
        func_08827F68(0x6234, t, 0x98, flags, 1);
        break;
    case 357:
        func_08827F68(0x6233, t, 0xE, flags, 1);
        func_08827F68(0x6232, t, 0x11D, flags, 2);
        break;
    case 358:
        func_08827F68(0x6232, t, 0x5A, flags, 1);
        break;
    case 360:
        func_08827F68(0x6234, t, 0x73, flags, 1);
        break;
    case 364:
        func_08827F68(0x6234, t, 0x29, flags, 1);
        break;
    case 365:
        func_08827F68(0x6233, t, 0x89, flags, 1);
        break;
    case 367:
        func_08827F68(0x6233, t, 0x5C, flags, 1);
        break;
    case 368:
        func_08827F68(0x6232, t, 100, flags, 1);
        break;
    case 369:
        func_08827F68(0x6233, t, 0x4F, flags, 1);
        func_08827F68(0x6232, t, 0xB4, flags, 2);
        func_08827F68(0x6234, t, 0xD9, flags, 4);
        func_08827F68(0x6233, t, 0xF0, flags, 8);
        func_08827F68(0x6232, t, 0x109, flags, 0x10);
        func_08827F68(0x6234, t, 0x13E, flags, 0x20);
        break;
    case 372:
        func_08827F68(0x6233, t, 0x4E, flags, 1);
        break;
    }
}

int dd_fx_aura_start(DdFxArg *arg, void *a, void *b)
{
    int task = func_0882623C(dd_fx_aura_update, a, b, sizeof(DdFxAura));

    if (task != -1)
        ((DdFxAura *)func_08826440(task))->arg = arg;
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
