// @range 0x0884E228-0x0884F038
/* Effect descriptors of the cards: four tables sorted by card id (by the b3_4 kind of the action),
   each entry with the handlers of the card. duel_card_desc finds the entry of an action; the other
   functions call one handler or count the zones its target test accepts. */
#include "duel.h"

typedef struct DuelCardDesc {          /* 0x18 */
    unsigned int id;
    int (*resolve)(DuelAct *a, int b);              /* +0x04 */
    int (*target)(DuelAct *a, unsigned int p, int z); /* +0x08 */
    int (*cond)(DuelAct *a, int b);                 /* +0x0C */
    int (*fn10)(DuelAct *a, int b);                 /* +0x10 */
    int (*fn14)(DuelAct *a, int b);                 /* +0x14 */
} DuelCardDesc;

extern DuelCardDesc duel_desc_main[];  /* 0x08907FB4, 0x2FC entries */
extern DuelCardDesc duel_desc_1[];     /* 0x0890C76C, 0x1C6 entries */
extern DuelCardDesc duel_desc_2[];     /* 0x0890F214, 0xAF entries */
extern DuelCardDesc duel_desc_3[];     /* 0x08910294, 0xD5 entries */
extern int duel_targeting_off;
extern DuelAct *duel_w_089147DC;

void duel_088327F8(unsigned int p, int id, int n);
int duel_088619FC();
int duel_08861D44();
int duel_08863060();
int duel_088647E8();
int duel_088657D8();
int duel_08866A3C();
int duel_088681A8();
int duel_08866608();

int duel_0884E228(void)
{
    return 0;
}

int duel_0884E230(void)
{
    return 2;
}

static DuelCardDesc *duel_card_desc(DuelAct *a)
{
    int n;
    int lo = 0;
    DuelCardDesc *t;
    unsigned int id = a->id;

    switch (a->b3_4) {
    case 0:
        t = duel_desc_main;
        n = 0x2FC;
        break;
    case 1:
        t = duel_desc_1;
        n = 0x1C6;
        break;
    case 2:
        if (a->kind == 0x23) {
            t = duel_desc_main;
            n = 0x2FC;
        } else {
            t = duel_desc_2;
            n = 0xAF;
        }
        break;
    case 3:
        t = duel_desc_3;
        n = 0xD5;
        break;
    }
    while (lo < n) {
        int mid = (lo + n) / 2;

        if (t[mid].id < id) {
            lo = mid + 1;
        } else {
            n = mid;
        }
    }
    if (t[lo].id == id) {
        return &t[lo];
    }
    return 0;
}

int duel_0884E340(unsigned short id)
{
    DuelAct t = {0};

    t.b3_4 = 3;
    t.id = id;
    return duel_card_desc(&t) != 0;
}

int duel_0884E398(DuelAct *a, int b)
{
    DuelCardDesc *d = duel_card_desc(a);

    if (d != 0 && d->cond != 0) {
        return d->cond(a, b);
    }
    return 1;
}

int duel_0884E3E4(DuelAct *a, unsigned int p, int z)
{
    DuelCardDesc *d = duel_card_desc(a);

    int (*fn)(DuelAct *a, unsigned int p, int z);

    if (d != 0 && (fn = d->target) != 0) {
        duel_targeting_off = 0;
        return fn(a, p, z);
    }
    return 1;
}

int duel_0884E440(DuelAct *a, unsigned int p, int z)
{
    DuelCardDesc *d = duel_card_desc(a);
    int r = 1;

    int (*fn)(DuelAct *a, unsigned int p, int z);

    if (d != 0 && (fn = d->target) != 0) {
        duel_targeting_off = 1;
        r = fn(a, p, z);
    }
    duel_targeting_off = 0;
    return r;
}

unsigned int duel_0884E538(DuelAct *a, int (*fn)(DuelAct *a, unsigned int p, int z));

unsigned int duel_0884E4A0(DuelAct *a)
{
    DuelCardDesc *d = duel_card_desc(a);

    if (d != 0 && d->target != 0) {
        return duel_0884E538(a, d->target);
    }
    return -1;
}

unsigned int duel_0884E4E4(DuelAct *a)
{
    DuelCardDesc *d = duel_card_desc(a);
    int (*fn)(DuelAct *a, unsigned int p, int z);
    unsigned int r;

    if (d != 0 && (fn = d->target) != 0) {
        duel_targeting_off = 1;
        r = duel_0884E538(a, fn);
        duel_targeting_off = 0;
        return r;
    }
    return -1;
}

unsigned int duel_0884E538(DuelAct *a, int (*fn)(DuelAct *a, unsigned int p, int z))
{
    unsigned int m = 0;
    int p;
    int z;

    for (p = 0; p < 2; p++) {
        for (z = 0; z <= 10; z++) {
            if (fn(a, p, z) != 0) {
                m |= 1 << (z + p * 16);
            }
        }
    }
    return m;
}

int duel_0884E5E0(DuelAct *a)
{
    int n = 0;
    DuelCardDesc *d = duel_card_desc(a);
    int p;
    int z;

    duel_targeting_off = 0;
    if (d != 0 && d->target != 0) {
        for (p = 0; p < 2; p++) {
            for (z = 0; z <= 10; z++) {
                if (d->target(a, p, z) != 0) {
                    n++;
                }
            }
        }
    }
    return n;
}

int duel_0884E698(DuelAct *a, int (*fn)(DuelAct *a, unsigned int p, int z), unsigned int skip);

int duel_0884E690(DuelAct *a, int (*fn)(DuelAct *a, unsigned int p, int z))
{
    return duel_0884E698(a, fn, -1);
}

int duel_0884E698(DuelAct *a, int (*fn)(DuelAct *a, unsigned int p, int z), unsigned int skip)
{
    int n = 0;
    int p;
    int z;

    duel_targeting_off = 0;
    for (p = 0; p < 2; p++) {
        for (z = 0; z <= 10; z++) {
            if (skip != (unsigned short)((unsigned char)p | (unsigned char)z << 8) && fn(a, p, z) != 0) {
                n++;
            }
        }
    }
    return n;
}

int duel_0884E760(DuelAct *a, unsigned int p, int (*fn)(DuelAct *a, unsigned int p, int z))
{
    int n = 0;
    int z;

    duel_targeting_off = 0;
    for (z = 0; z <= 10; z++) {
        if (fn(a, p, z) != 0) {
            n++;
        }
    }
    return n;
}

int duel_act_0884E7E8(DuelAct *a)
{
    int n = 0;
    DuelCardDesc *d = duel_card_desc(a);
    int p;
    int z;

    duel_targeting_off = 0;
    if (d != 0 && d->target != 0) {
        for (p = 0; p < 2; p++) {
            for (z = 0; z <= 4; z++) {
                if (d->target(a, p, z) != 0) {
                    n++;
                }
            }
        }
    }
    return n;
}

int duel_act_0884E898(DuelAct *a)
{
    int n = 0;
    DuelCardDesc *d = duel_card_desc(a);
    int p;
    int z;

    duel_targeting_off = 0;
    if (d != 0 && d->target != 0) {
        for (p = 0; p < 2; p++) {
            for (z = 5; z <= 10; z++) {
                if (d->target(a, p, z) != 0) {
                    n++;
                }
            }
        }
    }
    return n;
}

int duel_0884E948(DuelAct *a, unsigned int p)
{
    int n = 0;
    DuelCardDesc *d = duel_card_desc(a);
    int z;

    duel_targeting_off = 0;
    if (d != 0 && d->target != 0) {
        for (z = 0; z <= 10; z++) {
            if (d->target(a, p, z) != 0) {
                n++;
            }
        }
    }
    return n;
}

int duel_0884E9E8(DuelAct *a, int b)
{
    DuelCardDesc *d = duel_card_desc(a);
    DuelAct *save;
    int r;
    int p;
    int z;
    int (*c)(DuelAct *a, int b);

    if (d != 0) {
        c = d->cond;
        save = duel_w_089147DC;
        if (c != 0) {
            duel_w_089147DC = a;
            r = c(a, b);
            duel_w_089147DC = save;
            if (r == 0) {
                return 0;
            }
            if (r > 1) {
                return 1;
            }
        }
        duel_targeting_off = 0;
        if (d->target != 0) {
            duel_w_089147DC = a;
            for (p = 0; p < 2; p++) {
                for (z = 0; z <= 10; z++) {
                    r = !d->target(a, p, z);
                    if (!r) {
                        duel_w_089147DC = save;
                        return 1;
                    }
                }
            }
            duel_w_089147DC = save;
            return 0;
        }
    }
    return 1;
}

int duel_0884EB10(DuelAct *a, int b)
{
    DuelCardDesc *d = duel_card_desc(a);

    if (d != 0 && d->fn10 != 0) {
        return d->fn10(a, b);
    }
    return 1;
}

int duel_0884EB5C(DuelAct *a)
{
    DuelCardDesc *d = duel_card_desc(a);

    return d != 0 && d->fn10 != 0;
}

int duel_0884EB90(DuelAct *a, int b)
{
    DuelCardDesc *d = duel_card_desc(a);

    if (d != 0 && d->fn14 != 0) {
        return d->fn14(a, b);
    }
    return 1;
}

int duel_0884EBDC(DuelAct *a)
{
    DuelCardDesc *d = duel_card_desc(a);

    return d != 0 && d->fn14 != 0;
}

int duel_0884EC10(DuelAct *a, int b)
{
    DuelCardDesc *d;

    if (a->b4_5 && a[1].id == 0x1975) {
        if (!a->b4_2) {
            duel_088327F8(1 - a->player, 1, 1);
        }
        a->b4_3 = 1;
        return 0;
    }
    d = duel_card_desc(a);
    if (d != 0 && d->resolve != 0) {
        return d->resolve(a, b);
    }
    return 0;
}

int duel_0884ECD4(DuelAct *a)
{
    DuelCardDesc *d = duel_card_desc(a);

    return d != 0 && d->resolve != 0;
}

int duel_0884ED08(DuelAct *a)
{
    DuelCardDesc *d;
    int (*f)(DuelAct *, int);

    switch (a->b3_4) {
    case 0:
        break;
    case 3:
        break;
    default:
        goto ok;
    }
    return 0;
ok:
    d = duel_card_desc(a);
    if (d != 0) {
        f = d->fn10;
        if (f == duel_088619FC) {
            return 1;
        }
        if (f == duel_08861D44) {
            return 1;
        }
        if (f == duel_08863060) {
            return 1;
        }
        if (f == duel_088647E8) {
            return 1;
        }
        if (f == duel_088657D8) {
            return 1;
        }
        if (f == duel_08866A3C) {
            return 1;
        }
        if (f == duel_088681A8) {
            return 1;
        }
        if (f == duel_08866608) {
            return 1;
        }
        switch (a->id) {
        case 0x1147:
        case 0x1181:
        case 0x19CB:
        case 0x16C1:
        case 0x16F8:
        case 0x16F9:
        case 0x17C5:
        case 0x18E0:
        case 0x18EC:
        case 0x1A28:
        case 0x1A58:
            return 1;
        case 0x10D6:
        case 0x16CD:
        case 0x1A61:
            return 1;
        case 0x13C7:
        case 0x188F:
            if (a->kind == 0x16) {
                return 1;
            }
            break;
        case 0x1503:
            if (a->kind != 5) {
                return 1;
            }
            break;
        case 0x15D0:
            if (a->kind != 0xF) {
                return 1;
            }
            break;
        case 0x17D5:
        case 0x1817:
            if (a->kind == 2) {
                return 1;
            }
            break;
        case 0x11CF:
        case 0x17D7:
        case 0x17D8:
            if (a->kind == 5) {
                return 1;
            }
            break;
        case 0x19D3:
            if (a->kind == 0xF) {
                return 1;
            }
            break;
        case 0x1A6D:
        case 0x1A6E:
        case 0x1A6F:
            if (a->idx == 0) {
                return 1;
            }
            break;
        }
    }
    return 0;
}
