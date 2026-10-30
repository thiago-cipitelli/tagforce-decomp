// @range 0x08838CDC-0x088395D4
// @rodata 0x088701F4-0x088707A4
/* Visual effects attached to a card model: four slots (dd_fx_table), the dispatcher that maps an effect
   id (13-376) to the task that draws it, and the two extra slots (+0xB0, +0xB4) for the special
   summon effects. */
#include "libduel_draw.h"

void *model_hook(void *m, int which, void *v);
void snd_cmd_fef9(void);

/* the effect tasks (start functions; the task's user data is the DdFxSlot) */
int dd_fx13_start(DdFxSlot *s, int a, int b);          /* 13-23 */
int dd_fx13_start_b(DdFxSlot *s, int a, int b);        /* 13-23 on a kind 19 owner */
int dd_fx27_start(DdFxSlot *s, int a, int b);              /* 27-31, 38-41, 54 */
int dd_fx32_start(DdFxSlot *s, int arg, int a, int b);     /* 32-37, 52-53 */
int dd_fx51_start(DdFxSlot *s, int a, int b);               /* 51, 55-62 */
int dd_fx42_start(DdFxSlot *s, int a, int b);               /* 42-50, 63-64, 71-77, 82 */
int dd_fx78_arg(DdFxOwner *o);
int dd_fx78_start(DdFxSlot *s, int arg, int a, int b);      /* 78-81 */
int dd_fx83_start(DdFxSlot *s, int arg, int a, int b);      /* 83-90, 118-128 */
int func_088477E8(DdFxSlot *s, int a, int b);               /* 24-26, 100-101, 113-114 */
int func_08847D80(DdFxSlot *s, int a, int b);               /* 102-106 */
int func_08848B40(DdFxSlot *s, int a, int b);               /* 247-281 */
int func_08849060(DdFxSlot *s, int a, int b);               /* 204-376 */
int func_0884929C(DdFxSlot *s, int a, int b);
int func_0884A86C(DdFxSlot *s, int a, int b);
int func_0884D394(DdFxSlot *s, int a, int b);
void func_0884D488(int a, int b);
void func_0884D4A8(void);                                   /* model hook of the kind 28 effects */
void func_08813FB4(DdFxOwner *o, int v);                    /* sets o->+0x158 */
void func_0884E0C0(int level, int flag);                    /* dims the field behind the effect */

void dd_fx_init(void)
{
    DdFxTable *const t = &dd_fx_table;

    memset(t, 0, sizeof(*t));
    t->slot[0].task = -1;
    t->slot[1].task = -1;
    t->slot[2].task = -1;
    t->slot[3].task = -1;
}

/* takes a free slot and records who asked for it */
static DdFxSlot *dd_fx_alloc(int type, int sub, int id, DdFxOwner *o)
{
    DdFxTable *t = &dd_fx_table;
    DdFxSlot *s;
    int i;

    for (i = 0; i < 4; i++) {
        s = &t->slot[i];
        if (s->task == -1) {
            s->param = 0;
            s->type = type;
            s->sub = sub;
            s->id = id;
            s->owner = o;
            return s;
        }
    }
    return 0;
}

/* starts effect `id` on `o` (one at a time per owner) and dims the field while it plays */
void dd_fx_start(DdFxOwner *o, int id)
{
    int type;
    int sub;
    DdFxTable *t = &dd_fx_table;
    DdFxSlot *s = 0;
    int ok = 1;
    int dim = ok;
    int level = 96;
    int arg;

    func_08827F54(o, &type, &sub);
    if (o->fx != 0) {
        ok = 0;
    } else {
        switch (id) {
        case 13: case 14: case 15: case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
            s = dd_fx_alloc(type, sub, id, o);
            if (s == 0)
                break;
            if (type != 19)
                s->task = dd_fx13_start(s, 0, 0);
            else
                s->task = dd_fx13_start_b(s, 0, 0);
            break;
        case 27: case 28: case 29: case 30: case 31: case 38: case 39: case 40: case 41: case 54:
            s = dd_fx_alloc(type, sub, id, o);
            if (s != 0)
                s->task = dd_fx27_start(s, 0, 0);
            break;
        case 32: case 33: case 34: case 35: case 36: case 37: case 52: case 53:
            arg = t->arg0;
            s = dd_fx_alloc(type, sub, id, o);
            if (s != 0)
                s->task = dd_fx32_start(s, arg, 0, 0);
            break;
        case 51: case 55: case 56: case 57: case 58: case 59: case 60: case 61: case 62:
            s = dd_fx_alloc(type, sub, id, o);
            if (s != 0)
                s->task = dd_fx51_start(s, 0, 0);
            break;
        case 42: case 43: case 44: case 45: case 46: case 47: case 48: case 49: case 50: case 63: case 64:
        case 71: case 72: case 73: case 74: case 75: case 76: case 77: case 82:
            s = dd_fx_alloc(type, sub, id, o);
            if (s != 0)
                s->task = dd_fx42_start(s, 0, 0);
            break;
        case 78: case 79: case 80: case 81:
            arg = dd_fx78_arg(o);
            s = dd_fx_alloc(type, sub, id, o);
            if (s != 0)
                s->task = dd_fx78_start(s, arg, 0, 0);
            break;
        case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90: case 118: case 119:
        case 120: case 121: case 122: case 123: case 124: case 125: case 126: case 127: case 128:
            arg = t->arg4;
            s = dd_fx_alloc(type, sub, id, o);
            if (s != 0)
                s->task = dd_fx83_start(s, arg, 0, 0);
            break;
        case 24: case 25: case 26: case 100: case 101: case 113: case 114:
            s = dd_fx_alloc(type, sub, id, o);
            if (s != 0)
                s->task = func_088477E8(s, 0, 0);
            break;
        case 102: case 103: case 104: case 105: case 106:
            if (t->enable8 == 1) {
                s = dd_fx_alloc(type, sub, id, o);
                if (s != 0)
                    s->task = func_08847D80(s, 0, 0);
            } else {
                ok = 0;
                dim = 0;
            }
            break;
        case 247: case 248: case 249: case 250: case 251: case 253: case 254: case 255: case 256: case 257:
        case 258: case 259: case 260: case 261: case 262: case 263: case 264: case 266: case 267: case 268:
        case 269: case 270: case 271: case 272: case 273: case 274: case 276: case 277: case 279: case 280:
        case 281:
            s = dd_fx_alloc(type, sub, id, o);
            func_08813FB4(o, 0);
            if (type == 12)
                level = 0;
            if (s != 0)
                s->task = func_08848B40(s, 0, 0);
            break;
        case 252: case 278: case 282: case 283: case 284: case 286: case 287: case 288: case 289: case 332:
            func_08813FB4(o, 0);
            ok = 0;
            dim = 0;
            break;
        case 204: case 205: case 206: case 207: case 208: case 209: case 210: case 211: case 212: case 213:
        case 214: case 215: case 216: case 217: case 218: case 219: case 220: case 221: case 223: case 224:
        case 225: case 226: case 227: case 228: case 229: case 230: case 231: case 232: case 233: case 234:
        case 235: case 236: case 237: case 238: case 239: case 240: case 241: case 242: case 243: case 244:
        case 245: case 246: case 290: case 291: case 292: case 293: case 296: case 297: case 298: case 299:
        case 300: case 301: case 302: case 303: case 304: case 305: case 306: case 307: case 308: case 310:
        case 311: case 312: case 313: case 314: case 315: case 316: case 317: case 318: case 319: case 320:
        case 321: case 322: case 323: case 324: case 325: case 326: case 327: case 328: case 329: case 330:
        case 331: case 333: case 334: case 335: case 336: case 337: case 339: case 340: case 341: case 342:
        case 343: case 344: case 345: case 346: case 347: case 348: case 349: case 350: case 351: case 353:
        case 354: case 355: case 356: case 357: case 358: case 359: case 360: case 361: case 362: case 363:
        case 364: case 365: case 366: case 367: case 368: case 369: case 370: case 371: case 372: case 373:
        case 374: case 375: case 376:
            s = dd_fx_alloc(type, sub, id, o);
            dim = 0;
            if (s != 0)
                s->task = func_08849060(s, 0, 0);
            break;
        default:
            ok = 0;
            dim = 0;
            break;
        }
    }
    if (ok == 1 && (s == 0 || s->task == -1))
        ok = 0;
    if (ok == 1)
        o->fx = s;
    if (dim == 0) {
        if (type == 18 && sub == 0)
            dim = 1;
        if (id >= 91 && id < 102 && id != 101 && id != 100)
            dim = 1;
    }
    if (dim == 1)
        func_0884E0C0(level, 1);
    else
        func_0884E0C0(255, 1);
}

void dd_fx_stop(DdFxOwner *o)
{
    DdFxSlot *s = o->fx;

    if (s != 0) {
        func_0882636C(s->task);
        s->task = -1;
        o->fx = 0;
    }
}

/* the value arrives in the fifth argument register; the other three are not read */
void dd_fx_set_param(DdFxOwner *o, int a, int b, int c, int v)
{
    if (o->fx != 0)
        o->fx->param = v;
}

/* second slot: the summon effect of kind 18 (sub 0) owners and effect 250 of kind 12 owners */
void dd_fx2_start(DdFxOwner *o, int id)
{
    int type;
    int sub;
    DdFxSlot *s = 0;
    int ok = 1;

    if (o->fx2 != 0) {
        ok = 0;
    } else {
        func_08827F54(o, &type, &sub);
        switch (type) {
        case 12:
            if (id == 250) {
                s = dd_fx_alloc(type, sub, id, o);
                if (s != 0)
                    s->task = func_0884A86C(s, 0, 0);
            } else {
                ok = 0;
            }
            break;
        case 18:
            if (sub == 0) {
                s = dd_fx_alloc(type, sub, id, o);
                if (s != 0)
                    s->task = func_0884929C(s, 0, 0);
            } else {
                ok = 0;
            }
            break;
        default:
            ok = 0;
            break;
        }
    }
    if (ok == 1 && (s == 0 || s->task == -1))
        ok = 0;
    if (ok == 1)
        o->fx2 = s;
}

void dd_fx2_stop(DdFxOwner *o)
{
    DdFxSlot *s = o->fx2;

    if (s != 0) {
        func_0882636C(s->task);
        s->task = -1;
        o->fx2 = 0;
    }
}

void dd_fx2_set_param(DdFxOwner *o, int a, int b, int c, int v)
{
    if (o->fx2 != 0)
        o->fx2->param = v;
}

/* third slot: any owner; kind 28 owners also get a model hook for effects 100, 101, 113 and 114 */
void dd_fx3_start(DdFxOwner *o, int id)
{
    int type;
    int sub;
    DdFxSlot *s = 0;
    int ok = 1;

    func_08827F54(o, &type, &sub);
    if (o->fx3 != 0) {
        ok = 0;
    } else {
        s = dd_fx_alloc(type, sub, id, o);
        if (s != 0)
            s->task = func_0884D394(s, 0, 0);
    }
    if (ok == 1 && (s == 0 || s->task == -1))
        ok = 0;
    if (ok == 1)
        o->fx3 = s;
    switch (type) {
    case 28:
        switch (id) {
        case 100:
            func_0884D488(71, 15);
            model_hook(o->model, 1, func_0884D4A8);
            break;
        case 101:
            func_0884D488(61, 15);
            model_hook(o->model, 1, func_0884D4A8);
            break;
        case 113: case 114:
            func_0884D488(1, 15);
            model_hook(o->model, 1, func_0884D4A8);
            break;
        default:
            model_hook(o->model, 1, 0);
            break;
        }
        break;
    default:
        model_hook(o->model, 1, 0);
        break;
    }
}

void dd_fx3_stop(DdFxOwner *o)
{
    DdFxSlot *s = o->fx3;

    if (s != 0) {
        snd_cmd_fef9();
        func_0882636C(s->task);
        s->task = -1;
        o->fx3 = 0;
    }
}

void dd_fx3_set_param(DdFxOwner *o, int a, int b, int c, int v)
{
    if (o->fx3 != 0)
        o->fx3->param = v;
}
