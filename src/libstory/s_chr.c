// @range 0x08804270-0x088062FC
// @asm 0x088044EC-0x08804570
// @asm 0x08804728-0x088047E0
// @asm 0x088048C8-0x08804908
// @asm 0x08805694-0x08805708
// @asm 0x08805708-0x088057EC
// @asm 0x088057EC-0x08805860
// @asm 0x08805860-0x088058B0
// @asm 0x088058B0-0x08805934
// @asm 0x08805974-0x08805B68
// @asm 0x08805B68-0x08805BD0
// @asm 0x08805BD0-0x08805E24
// @asm 0x08805E24-0x08806074
// @asm 0x08806074-0x08806170
/* Story characters (150): the script tables of each one (Scr<i>_G/Step/Deck/DH/DN/Sche/...), their
   saved state (level, affinity, flags, position, look, deck in use), the conditions of the tables
   (date, level, flags, weekday, daytime) and the daily update that picks look, place and deck. */
#include "libstory.h"
#include "card_db.h"

/* the original indexes both tables by byte offset, which fixes the order of the loads */
#define ST_CHR_SAVE(i) ((StoryChrSave *)((char *)st_chr_save + (i) * sizeof(StoryChrSave)))
#define ST_CHR(i) (&((StoryChr *)(char *)st_chr)[i])

unsigned short deck_slot_get_value(int i);
void deck_slot_set_value(int i, int v);
void deck_slot_add_value(int i, int delta);
int deck_find_best_slot(void);
unsigned short *chara_get_name(int i, int j);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
int st_id_to_item(int id);
int st_id_to_deck(int id);
int st_id_to_chr(int id);

void st_chr_clear_ext_all(void);
void st_chr_clear_ext(int i);
void st_chr_update(int i, int mode);
void st_chr_away_set(int i, int on);
int st_chr_step_value(int i);
int st_chr_affinity(int i);
int st_chr_best(void);
int st_chr_named(int n);
void st_chr_name_default(int i);
void st_chr_name_real(int i);
int st_chr_level_is(int i, unsigned int level);
int st_cond_daytime(int c);
int st_cond_partner_flag(int c, unsigned int bit, int mode);
int st_cond_weekday(int c, int n);
int st_cond_date(int c);
int st_cond_day(int c);
int st_cond_affinity(int i, unsigned int v);
int st_chr_dn_find(int i, int mode);
int st_chr_dn_pick(int i);
int st_chr_g_update(int i);
int st_chr_deck_find(int i);
void st_chr_deck_update(int i);
void st_chr_deck_name_set(int i);
void st_chr_deck_name_default(int i);

void st_chr_init(void)
{
    memset(st_chr, 0, sizeof(st_chr));
}

void st_chr_term(void)
{
    memset(st_chr, 0, sizeof(st_chr));
}

void st_chr_attach(StoryChrSave *save, StoryChrFlags *flags)
{
    st_chr_save = save;
    st_chr_flags = flags;
}

void st_chr_reset(int i)
{
    StoryChrSave *s = ST_CHR_SAVE(i);

    memset(s, 0, sizeof(StoryChrSave));
    s->level = 1;
    st_chr_clear_ext_all();
}

void st_scr_set_g(int i, void *gp, void *gs, void *gc, void *g)
{
    StoryChr *c = &st_chr[i];

    c->gp = gp;
    c->gs = gs;
    c->gc = gc;
    c->g = g;
}

void st_scr_set_step(int i, void *p)
{
    ST_CHR(i)->step = p;
}

void st_scr_set_deck(int i, void *p)
{
    ST_CHR(i)->deck = p;
}

void st_scr_set_dh(int i, void *p)
{
    ST_CHR(i)->dh = p;
}

void st_scr_set_dn(int i, void *p)
{
    ST_CHR(i)->dn = p;
}

void st_scr_set_sche(int i, void *p)
{
    ST_CHR(i)->sche = p;
}

void st_scr_set_es(int i, void *p)
{
    ST_CHR(i)->es = p;
}

void st_chr_level_up(int i)
{
    StoryChrSave *s = ST_CHR_SAVE(i);

    s->level++;
    if (s->level >= 100)
        s->level = 100;
}

unsigned short *st_chr_text_a(int i)
{
    if (ST_CHR(i)->name[0] == 0)
        return 0;
    return ST_CHR(i)->name;
}

unsigned short *st_chr_text_b(int i)
{
    if (ST_CHR(i)->deck_name[0] == 0)
        return 0;
    return ST_CHR(i)->deck_name;
}

int st_chr_level(int i)
{
    return ST_CHR_SAVE(i)->level;
}

int st_chr_deck(int i)
{
    return ST_CHR_SAVE(i)->recipe;
}

/* affinity cap of the current level: Step is a list of (level, cap) pairs */
#ifdef NON_MATCHING /* mesma forma; a alocação de registradores (a1/a2, v1/a0) não bate */
int st_chr_step_value(int i)
{
    unsigned char level = ST_CHR_SAVE(i)->level;
    int k = 1;
    unsigned short *t = ST_CHR(i)->step;

    do {
        if (t[k * 2 - 2] == 0)
            break;
        if (level == t[k * 2 - 2])
            return t[k * 2 - 1];
        k++;
    } while (k < 100);
    return 0xFFFF;
}
#endif

int st_chr_flag_get(int i, unsigned int bit)
{
    StoryChrSave *s = ST_CHR_SAVE(i);

    if (bit == 0)
        return 1;
    else if (bit > 32)
        return 1;
    else
        return (s->flags & (1 << (bit - 1))) != 0;
}

void st_chr_flag_set(int i, unsigned int bit, int on)
{
    StoryChrSave *s = ST_CHR_SAVE(i);

    if (bit != 0 && bit < 33) {
        if (on)
            s->flags |= 1 << (bit - 1);
        else
            s->flags &= ~(1 << (bit - 1));
    }
}

unsigned short *st_text_at(int i, int *offsets, unsigned short *text)
{
    return text + offsets[i];
}

void st_chr_update_all(int mode)
{
    int i;

    if (story_mode_is1()) {
        if (sdate_is_daytime())
            st_chr_away_set(4, 0);
        else
            st_chr_away_set(4, 1);
    }
    i = 0;
    do {
        st_chr_update(i, mode);
        i++;
    } while (i < 150);
}

int st_chr_look(int i)
{
    return ST_CHR_SAVE(i)->look;
}

int st_chr_x52(int i)
{
    return ST_CHR_SAVE(i)->x52;
}

#ifdef NON_MATCHING /* o contador sai em a0 e a cópia em s1; nenhuma ordem de declarações reproduz */
int st_chr_pick(int i)
{
    StoryChrSave *s = ST_CHR_SAVE(i);
    int k;
    int n;
    int r;

    if (s->state & 4) {
        k = 0;
        do {
            if (s->pick_a[k] == 0)
                break;
            k++;
            n = k;
        } while (k < 6);
        r = (int)rand_next() % n;
        s->pick = r;
        k = s->pick_a[r];
        s->state &= ~4;
        return k;
    }
    return s->pick_b[s->pick];
}
#endif

int st_chr_x30(int i)
{
    return ST_CHR_SAVE(i)->x30;
}

int st_chr_x32(int i)
{
    return ST_CHR_SAVE(i)->x32;
}

int st_chr_x38(int i)
{
    return ST_CHR_SAVE(i)->x38;
}

int st_chr_x34(int i)
{
    return ST_CHR_SAVE(i)->x34;
}

int st_chr_x36(int i)
{
    return ST_CHR_SAVE(i)->x36;
}

unsigned short *st_scr_get_es(int i)
{
    return ST_CHR(i)->es;
}

#ifdef NON_MATCHING /* o desvio para o laço sai bnezl com a primeira instrução no delay slot; o original tem bnez + nop */
unsigned short *st_scr_find_es(int i, unsigned int key)
{
    unsigned short *p = ST_CHR(i)->es;

    if (p == 0)
        return 0;
    while (1) {
        if (*p == key)
            return p;
        p += 5;
    }
}
#endif

void st_scr_set_ei(int i, void *p)
{
    ST_CHR(i)->ei = p;
}

void *st_scr_get_ei(int i)
{
    return ST_CHR(i)->ei;
}

void st_scr_set_el(int i, void *p)
{
    ST_CHR(i)->el = p;
}

void *st_scr_get_el(int i)
{
    return ST_CHR(i)->el;
}

int st_chr_affinity(int i)
{
    return deck_slot_get_value(st_id_to_item(i));
}

void st_chr_affinity_clear_all(void)
{
    int i = 2;

    do {
        deck_slot_set_value(i, 0);
        i++;
    } while (i < 152);
}

void st_chr_affinity_set(int i, int v)
{
    int slot = st_id_to_item(i);
    int max;

    if (v < 0)
        v = 0;
    else if (v > 0xFFFF)
        v = 0xFFFF;
    if ((max = st_chr_step_value(i)) < v)
        v = max;
    deck_slot_set_value(slot, v);
}

void st_chr_affinity_add(int i, int d)
{
    int v;
    int old;
    int slot;
    int max;

    slot = st_id_to_item(i);
    old = deck_slot_get_value(slot);
    v = old + d;
    if (v < 0) {
        se_play_delta(d);
        deck_slot_set_value(slot, 0);
    } else if (v > 0xFFFF) {
        se_play_delta(d);
        deck_slot_set_value(slot, 0xFFFF);
    } else {
        max = st_chr_step_value(i);
        if (max < v) {
            if (old != max)
                se_play_delta(d);
            deck_slot_set_value(slot, max);
        } else {
            se_play_delta(d);
            deck_slot_add_value(slot, d);
        }
    }
}

int st_chr_is_best(int i)
{
    return i == st_chr_best();
}

int st_chr_best(void)
{
    return st_id_to_deck(deck_find_best_slot());
}

void st_chr_x1c_set(int i, int v)
{
    ST_CHR_SAVE(i)->x1c = v;
}

int st_chr_x28(int i)
{
    return ST_CHR_SAVE(i)->x28;
}

int st_chr_x6c(int i)
{
    return ST_CHR_SAVE(i)->x6c;
}

int st_chr_x24(int i)
{
    return ST_CHR_SAVE(i)->x24;
}

int st_chr_x68(int i)
{
    return ST_CHR_SAVE(i)->x68;
}

void st_chr_set_pos(int i, int x, int y, unsigned char dir)
{
    StoryChrSave *s = ST_CHR_SAVE(i);

    s->x04 = x;
    s->x08 = y;
    s->x14 = dir;
}

void st_chr_set_pos_b(int i, int x, int y)
{
    StoryChrSave *s = ST_CHR_SAVE(i);

    s->x54 = x;
    s->x58 = y;
}

void st_chr_set_dir_b(int i, unsigned char dir)
{
    ST_CHR_SAVE(i)->x64 = dir;
}

int st_chr_x04(int i)
{
    return ST_CHR_SAVE(i)->x04;
}

int st_chr_x0c(int i)
{
    return ST_CHR_SAVE(i)->x0c;
}

int st_chr_x54(int i)
{
    return ST_CHR_SAVE(i)->x54;
}

int st_chr_x5c(int i)
{
    return ST_CHR_SAVE(i)->x5c;
}

int st_chr_x08(int i)
{
    return ST_CHR_SAVE(i)->x08;
}

int st_chr_x10(int i)
{
    return ST_CHR_SAVE(i)->x10;
}

int st_chr_x58(int i)
{
    return ST_CHR_SAVE(i)->x58;
}

int st_chr_x60(int i)
{
    return ST_CHR_SAVE(i)->x60;
}

int st_chr_x14(int i)
{
    return ST_CHR_SAVE(i)->x14;
}

int st_chr_attr(int i)
{
    return ST_CHR_SAVE(i)->attr;
}

int st_chr_x64(int i)
{
    return ST_CHR_SAVE(i)->x64;
}

int st_chr_x65(int i)
{
    return ST_CHR_SAVE(i)->x65;
}

int st_chr_x16(int i)
{
    return ST_CHR_SAVE(i)->x16;
}

int st_chr_x17(int i)
{
    return ST_CHR_SAVE(i)->x17;
}

int st_chr_x66(int i)
{
    return ST_CHR_SAVE(i)->x66;
}

int st_chr_x67(int i)
{
    return ST_CHR_SAVE(i)->x67;
}

void st_chr_clear_ext_all(void)
{
    int i = 0;

    do {
        st_chr_clear_ext(i);
        i++;
    } while (i < 150);
}

void st_chr_clear_ext(int i)
{
    StoryChrSave *s = ST_CHR_SAVE(i);

    s->x68 = -1;
    s->x6c = -1;
    s->x70 = 65000;
    s->x72 = 65000;
    s->x74 = 65000;
    s->x76 = 65000;
    s->x78 = 65000;
    s->x52 = 0;
    s->x54 = 0;
    s->x58 = 0;
    s->x5c = 0;
    s->x60 = 0;
    s->x64 = 0;
    s->x65 = 0;
    s->x66 = 0;
    s->x67 = 0;
    s->x7a[0] = 0;
    s->x7a[1] = 0;
    s->x7a[2] = 0;
    s->x7a[3] = 0;
    s->x7a[4] = 0;
    s->x7a[5] = 0;
}

void st_chr_x68_set(int i, int v)
{
    ST_CHR_SAVE(i)->x68 = v;
}

void st_chr_x6c_set(int i, int v)
{
    ST_CHR_SAVE(i)->x6c = v;
}

void st_chr_x70_set(int i, unsigned short v)
{
    ST_CHR_SAVE(i)->x70 = v;
}

void st_chr_x72_set(int i, unsigned short v)
{
    ST_CHR_SAVE(i)->x72 = v;
}

void st_chr_x74_set(int i, unsigned short a, unsigned short b)
{
    StoryChrSave *s = ST_CHR_SAVE(i);

    s->x74 = a;
    s->x76 = b;
}

void st_chr_x78_set(int i, unsigned short v)
{
    ST_CHR_SAVE(i)->x78 = v;
}

void st_chr_x5c_set(int i, int a, int b, unsigned short c)
{
    StoryChrSave *s = ST_CHR_SAVE(i);

    s->x52 = c;
    s->x5c = a;
    s->x60 = b;
}

void st_chr_x65_set(int i, int a, unsigned char b)
{
    StoryChrSave *s = ST_CHR_SAVE(i);

    s->x65 = chr_attr_remap(a);
    s->x66 = b;
}

void st_chr_x67_set(int i, unsigned char v)
{
    ST_CHR_SAVE(i)->x67 = v;
}

void st_chr_x7a_set(int i, unsigned short a, unsigned short b)
{
    StoryChrSave *s = ST_CHR_SAVE(i);

    s->x7a[0] = a;
    s->x7a[1] = b;
}

int st_chr_x70(int i)
{
    int v = ST_CHR_SAVE(i)->x70;

    if (v == 65000)
        v = -1;
    return v;
}

int st_chr_x72(int i)
{
    int v = ST_CHR_SAVE(i)->x72;

    if (v == 65000)
        v = -1;
    return v;
}

int st_chr_x74(int i)
{
    int v = ST_CHR_SAVE(i)->x74;

    if (v == 65000)
        v = -1;
    return v;
}

int st_chr_x76(int i)
{
    int v = ST_CHR_SAVE(i)->x76;

    if (v == 65000)
        v = -1;
    return v;
}

/* one of the first four x7a entries at random, falling back to a lower non-empty one */
int st_chr_x7a_rand(int i)
{
    StoryChrSave *s = ST_CHR_SAVE(i);
    int k;

    switch ((int)rand_next() % 4) {
    case 0:
        k = 0;
        break;
    case 1:
        k = 1;
        break;
    case 2:
        k = 2;
        break;
    case 3:
        k = 3;
        break;
    }
    if (s->x7a[k] == 0)
        k = 2;
    if (s->x7a[k] == 0)
        k = 1;
    if (s->x7a[k] == 0)
        k = 0;
    return s->x7a[k];
}

int st_chr_x78(int i)
{
    int v = ST_CHR_SAVE(i)->x78;

    if (v == 65000)
        v = -1;
    return v;
}

void st_chr_named_set(int n)
{
    StoryChrFlags *f = st_chr_flags;
    int w = 0;
    unsigned int *p;

loop:
    if (n >= 32) {
        n -= 32;
        w++;
        goto loop;
    }
    p = &f->named[w];
    *p |= 1 << n;
}

int st_chr_named(int n)
{
    StoryChrFlags *f = st_chr_flags;
    int w = 0;
    unsigned int *p;

loop:
    if (n >= 32) {
        n -= 32;
        w++;
        goto loop;
    }
    p = &f->named[w];
    return (*p & (1 << n)) != 0;
}

void st_chr_name_default(int i)
{
    StoryChr *c = ST_CHR(i);

    memset(c->name, 0, sizeof(c->name));
    wcscpy(c->name, stext_get(1));
}

void st_chr_name_real(int i)
{
    StoryChr *c = ST_CHR(i);

    memset(c->name, 0, sizeof(c->name));
    wcscpy(c->name, chara_get_name(0, st_id_to_item(i)));
}

/* remembers the recipe the character uses in its list of recipes seen */
void st_chr_note_deck(int i)
{
    StoryChrFlags *f = st_chr_flags;
    unsigned int recipe = ST_CHR_SAVE(i)->recipe;
    int k = 0;

    do {
        if (f->recipes[i][k] == recipe)
            return;
        if (f->recipes[i][k] == 0) {
            f->recipes[i][k] = recipe;
            return;
        }
        k++;
    } while (k < 6);
}

void st_chr_away_set(int i, int on)
{
    StoryChrSave *s = ST_CHR_SAVE(i);

    if (on)
        s->state |= 1;
    else
        s->state &= ~1;
}

int st_chr_is_away(int i)
{
    return (ST_CHR_SAVE(i)->state & 1) != 0;
}

int st_chr_level_is(int i, unsigned int level)
{
    StoryChrSave *s = ST_CHR_SAVE(i);

    if (level == 0)
        return 1;
    return level == s->level;
}

#ifdef NON_MATCHING /* o bloco final move v0,zero é alcançado por bnez/beqz sem likely no original; o mwcc preenche o delay slot */
int st_cond_daytime(int c)
{
    switch (c) {
    case 1:
        if (sdate_is_daytime())
            return 1;
        break;
    case 2:
        if (!sdate_is_daytime())
            return 1;
        break;
    case 0:
        return 1;
    default:
        return 0;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* mesmo problema dos desvios sem likely para o bloco final */
int st_cond_partner_flag(int c, unsigned int bit, int mode)
{
    StoryChrSave *s;
    unsigned int mask;
    int r;

    if (c == 0)
        return 1;
    s = ST_CHR_SAVE(st_id_to_chr(c));
    if (bit == 0)
        return 1;
    if (bit > 32)
        return 1;
    r = 1;
    mask = 1 << (bit - 1);
    if (mode != 0) {
        switch (mode) {
        case 3:
            if (s->flags & mask)
                return r;
            break;
        case 4:
            if (!(s->flags & mask))
                return r;
            break;
        default:
            return 0;
        }
        r = 0;
    }
    return r;
}
#endif

#ifdef NON_MATCHING /* o bloco final move v0,zero é alcançado por bnez/beqz sem likely no original; o mwcc preenche o delay slot */
int st_cond_weekday(int c, int n)
{
    switch (c) {
    case 5:
        if (!sdate_weekday_flag(n))
            return 1;
        break;
    case 6:
        if (sdate_weekday_flag(n))
            return 1;
        break;
    case 0:
        return 1;
    default:
        return 0;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* o bloco final move v0,zero é alcançado por bltz sem likely no original; o mwcc preenche o delay slot */
int st_cond_date(int c)
{
    switch (c) {
    case 7:
        if (sdate_check(1) < 0)
            break;
        return 1;
    case 0:
        return 1;
    default:
        return 0;
    }
    return 0;
}
#endif

#ifdef NON_MATCHING /* o bloco final é alcançado por bnez sem likely no original; o mwcc preenche o delay slot */
int st_cond_day(int c)
{
    switch (c) {
    case 5:
    case 6:
        if (!st_cond_weekday(c, 1))
            return 0;
        break;
    case 7:
        if (!st_cond_date(c))
            return 0;
        break;
    case 0:
        return 1;
    default:
        return 1;
    }
    return 1;
}
#endif

int st_cond_affinity(int i, unsigned int v)
{
    if (v == 0)
        return 1;
    return v <= (unsigned int)st_chr_affinity(i);
}

/* first DH (mode 2) or DN (mode 3) entry whose conditions hold; its first word is the look id */
#ifdef NON_MATCHING /* o original lê locais não inicializados da pilha e mantém mode em 72(sp); layout de pilha não reproduzido */
int st_chr_dn_find(int i, int mode)
{
    unsigned short *dh = ST_CHR(i)->dh;
    unsigned short *dn = ST_CHR(i)->dn;
    unsigned int level, daytime, flag, partner, day, affinity;
    unsigned int bit, fmode, id;

    sdate_check(0);
    for (;;) {
        if (mode == 3) {
            level = dn[1];
            daytime = dn[2];
            bit = dn[5];
            flag = dn[3];
            partner = dn[4];
            fmode = dn[6];
            id = dn[0];
            day = dn[7];
            affinity = dn[8];
        } else if (mode == 2) {
            level = dh[1];
            daytime = dh[2];
            bit = dh[5];
            flag = dh[3];
            partner = dh[4];
            fmode = dh[6];
            id = dh[0];
            day = dh[7];
            affinity = dh[8];
        }
        if ((level == 0 || st_chr_level_is(i, level))
            && (daytime == 0 || st_cond_daytime(daytime))
            && (flag == 0 || story_flag_get(flag))
            && (partner == 0 || st_cond_partner_flag(partner, bit, fmode))
            && (day == 0 || st_cond_day(day))
            && (affinity == 0 || st_cond_affinity(i, affinity)))
            return id;
        dh += 9;
        dn += 9;
    }
}
#endif

#ifdef NON_MATCHING /* desvios sem likely para o bloco final move v0,zero (como os st_cond_*) */
int st_chr_dn_pick(int i)
{
    int id;

    if (sdate_weekday_flag(0)) {
        id = st_chr_dn_find(i, 2);
        if (id == 0)
            id = 0;
    } else {
        id = st_chr_dn_find(i, 3);
        if (id == 0)
            id = 0;
    }
    return id;
}
#endif

/* daily update of a character: look id (mode 0 picks it again), look entry, place, deck */
#ifdef NON_MATCHING /* laço de busca e cópia da entrada Sche com alocação de registradores diferente */
void st_chr_update(int i, int mode)
{
    StoryChrSave *s = ST_CHR_SAVE(i);
    unsigned short *t;
    unsigned short *p;
    int day;
    int k;
    unsigned int cur;
    unsigned int key;
    unsigned short look;

    if (mode != 1) {
        if (mode != 0) {
            t = ST_CHR(i)->sche;
            goto find;
        }
        s->look_id = st_chr_dn_pick(i);
    }
    t = ST_CHR(i)->sche;
find:
    while (*t != s->look_id)
        t += 25;
    cur = sdate_get_0c();
    k = 0;
    p = t;
    for (;;) {
        key = p[1];
        if (cur == key)
            break;
        if ((int)cur < (int)key || key == 0) {
            k--;
            break;
        }
        p += 25;
        k++;
    }
    look = t[k * 25 + 2];
    day = sdate_weekday();
    if (s->look != look)
        s->state |= 4;
    if (s->day != day) {
        s->state |= 4;
        s->day = day;
    }
    s->look = look;
    s->x0c = t[k * 25 + 3];
    s->x10 = t[k * 25 + 4];
    s->attr = chr_attr_remap(t[k * 25 + 5]);
    s->x16 = t[k * 25 + 6];
    s->x17 = t[k * 25 + 7];
    s->pick_a[0] = t[k * 25 + 8];
    s->pick_a[1] = t[k * 25 + 10];
    s->pick_a[2] = t[k * 25 + 12];
    s->pick_a[3] = t[k * 25 + 14];
    s->pick_a[4] = t[k * 25 + 16];
    s->pick_a[5] = t[k * 25 + 18];
    s->pick_b[0] = t[k * 25 + 9];
    s->pick_b[1] = t[k * 25 + 11];
    s->pick_b[2] = t[k * 25 + 13];
    s->pick_b[3] = t[k * 25 + 15];
    s->pick_b[4] = t[k * 25 + 17];
    s->pick_b[5] = t[k * 25 + 19];
    s->x30 = t[k * 25 + 20];
    s->x32 = t[k * 25 + 21];
    s->x38 = t[k * 25 + 22];
    s->x34 = t[k * 25 + 23];
    s->x36 = t[k * 25 + 24];
    st_chr_g_update(i);
    if (st_chr_named(i))
        st_chr_name_real(i);
    else
        st_chr_name_default(i);
    st_chr_deck_update(i);
}
#endif

/* place and x24 from the first G table entry that holds (date, then level, chr flag, story flag) */
#ifdef NON_MATCHING /* forma igual, alocação dos s0..s7 diferente (as variáveis dos laços trocam de registrador) */
int st_chr_g_update(int i)
{
    unsigned short *gp = ST_CHR(i)->gp;
    unsigned short *gs = ST_CHR(i)->gs;
    unsigned short *gc = ST_CHR(i)->gc;
    unsigned short *g = ST_CHR(i)->g;
    StoryChrSave *s = ST_CHR_SAVE(i);
    unsigned short *p;
    int k;
    unsigned short *q;
    int j;
    int lv;

    for (k = 0, p = gp; ; p += 6, k++) {
        if (p[4] == 0 && p[5] == 0)
            break;
        if (story_date_in_range(p[0], p[1], p[2], p[3])) {
            s->x28 = gp[k * 6 + 4];
            s->x24 = gp[k * 6 + 5];
            return 1;
        }
    }
    /* the original compares the level with the first entry of GS, not the current one */
    for (j = 0, q = gs; ; q += 3, j++) {
        if (q[1] == 0 && q[2] == 0)
            break;
        if ((lv = st_chr_level(i)) == gs[0] || lv == 0) {
            s->x28 = gs[j * 3 + 1];
            s->x24 = gs[j * 3 + 2];
            return 1;
        }
    }
    for (j = 0, q = gc; ; q += 3, j++) {
        if (q[1] == 0 && q[2] == 0)
            break;
        if (q[0] == 0 || st_chr_flag_get(i, q[0])) {
            s->x28 = gc[j * 3 + 1];
            s->x24 = gc[j * 3 + 2];
            return 1;
        }
    }
    for (j = 0, q = g; ; q += 3, j++) {
        if (q[1] == 0 && q[2] == 0)
            break;
        if (q[0] == 0 || story_flag_get(q[0])) {
            s->x28 = g[j * 3 + 1];
            s->x24 = g[j * 3 + 2];
            return 1;
        }
    }
    s->x28 = 0;
    s->x24 = 0;
    return 0;
}
#endif

/* recipe of the first Deck entry that holds (date range, level, chr flag, story flag) */
#ifdef NON_MATCHING /* o teste do fim da tabela no topo do laço sai bnezl/b em vez de beqzl direto para o epílogo */
int st_chr_deck_find(int i)
{
    unsigned short *t = ST_CHR(i)->deck;
    unsigned short *p = t;
    int k = 0;

    for (;;) {
        if (p[7] == 0)
            return 0;
        if (story_date_in_range(p[0], p[1], p[2], p[3])
            && (p[4] == 0 || st_chr_level_is(i, p[4]))
            && (p[5] == 0 || st_chr_flag_get(i, p[5]))
            && (p[6] == 0 || story_flag_get(p[6])))
            return t[k * 8 + 7];
        p += 8;
        k++;
    }
}
#endif

void st_chr_deck_update(int i)
{
    StoryChrFlags *f = st_chr_flags;
    StoryChrSave *s = ST_CHR_SAVE(i);
    int k;

    s->recipe = st_chr_deck_find(i);
    k = 0;
    do {
        if (f->recipes[i][k] == 0)
            break;
        if (f->recipes[i][k] == s->recipe) {
            st_chr_deck_name_set(i);
            return;
        }
        k++;
    } while (k < 6);
    st_chr_deck_name_default(i);
}

void st_chr_deck_name_set(int i)
{
    StoryChr *c = ST_CHR(i);
    StoryChrSave *s = ST_CHR_SAVE(i);

    memset(c->deck_name, 0, sizeof(c->deck_name));
    wcscpy(c->deck_name, recipe_get_name(s->recipe));
}

void st_chr_deck_name_default(int i)
{
    StoryChr *c = ST_CHR(i);

    memset(c->deck_name, 0, sizeof(c->deck_name));
    wcscpy(c->deck_name, stext_get(1));
}
