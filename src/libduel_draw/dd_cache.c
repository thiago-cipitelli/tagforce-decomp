// @range 0x08804788-0x08805BF4
// @rodata 0x0886FD90-0x0886FE34
// @data 0x08874410-0x08874A2C
// @asm 0x08804BBC-0x08804C44
// @asm 0x088051B4-0x0880529C
/* Data loaded for the duel: the card image cache (CIP, medium and large), the field backgrounds
   (duel/bg/fmagicNNNN.ehp) and the character cut-ins with their faces and voices; the binary
   search shared by the resource tables. */
#include "libduel_draw.h"

void dd_cards_term(void);
void dd_cutin_free(void);

void dd_cards_init(void);

/* image cache with the empty entry and every card 4000..6999 with attribute 9 */
void dd_cards_reset(void)
{
    DdCards *c = &dd_cards;
    int id;

    dd_cards_term();
    dd_cards_init();
    c->e[dd_cards.n].id = 0;
    dd_cards.n++;
    for (id = 4000; id < 7000; id++) {
        if (card_is_attribute9(id)) {
            c->e[c->n].id = id;
            c->n++;
        }
    }
}

/* adds the main and side deck cards of `deck` to the image cache (sorted, without repeats) */
void dd_cards_add_deck(RecipeData *deck)
{
    unsigned short ids[110];
    DdCards *c = &dd_cards;
    int n;
    int total;
    int count;
    int i;
    unsigned short prev;
    DdCardEnt *e;

    n = 0;
    memcpy(ids, deck->main_ids, deck->n_main * 2);
    n += deck->n_main;
    memcpy(&ids[n], deck->side_ids, deck->n_side * 2);
    total = n + deck->n_side;
    qsort(ids, total, 2, dd_u16_cmp);
    count = dd_cards.n;
    prev = 0;
    for (i = 0; i < total; i++) {
        if (prev != ids[i]) {
            prev = ids[i];
            if (dd_res_find(prev, c, count, sizeof(DdCardEnt)) < 0) {
                e = &c->e[c->n];
                memset(e, 0, sizeof(DdCardEnt));
                e->id = ids[i];
                c->n++;
            }
        }
    }
    qsort(c, c->n, sizeof(DdCardEnt), dd_res_cmp);
}

int dd_card_ready_m(int i, int unused);
int dd_card_ready_h(int i, int unused);

/* 1 when every medium card image is loaded */
int dd_cards_ready_m(void)
{
    DdCards *c = &dd_cards;
    int i = 0;
    int ok = 1;

    if (!cip_is_loaded(dd_cards.cip_m))
        return 0;
    for (; i < c->n; i++)
        ok &= dd_card_ready_m(i, 0);
    return ok;
}

/* 1 when every large card image is loaded */
int dd_cards_ready_h(void)
{
    DdCards *c = &dd_cards;
    int i;
    int ok = 1;

    if (!cip_is_loaded(dd_cards.cip_h))
        return 0;
    for (i = 0; i < c->n; i++)
        ok &= dd_card_ready_h(i, 0);
    return ok;
}

int dd_bg_loaded(int slot, int size);

/* background `id` (duel/bg/fmagicNNNN.ehp): starts its read in a free slot; 0 until loaded */
void *dd_bg_get(int id)
{
    char path[0x80];
    DdBg *bg = &dd_bg;
    int j;
    DdBgSlot *p;
    DdBgSlot *b;
    int i;
    int slot;
    void *buf;

    for (j = 0, p = bg->slot; j < 2; j++, p++) {
        if (p->used && p->id == id)
            return p->loaded ? p->buf : 0;
    }
    for (i = 0, b = bg->slot; i < 2; i++, b++) {
        if (!b->used) {
            slot = i;
            break;
        }
    }
    if (i >= 2)
        return 0;
    b = &bg->slot[slot];
    memset(b, 0, sizeof(DdBgSlot));
    buf = bg->buf[slot];
    b->id = id;
    b->buf = buf;
    b->used = 1;
    sprintf(path, "duel/bg/fmagic%04d.ehp", b->id);
    ehfile_read_async_alias(3, path, buf, 0x80000, (int (*)(void *, int))dd_bg_loaded, (void *)slot);
    return 0;
}

#ifdef NON_MATCHING /* o lui/addiu de dd_bg sai depois do sw ra; nenhuma forma testada reproduziu o agendamento */
/* releases background `id` (later, if its read is still running) */
void dd_bg_release(int id)
{
    DdBg *bg = &dd_bg;
    DdBgSlot *b;
    int i;
    DdBgSlot *p;

    for (i = 0, p = bg->slot; i < 2; i++, p++) {
        b = p;
        if (p->used && p->id == id)
            break;
    }
    if (i == 2)
        return;
    if (b->loaded == 0 && b->busy)
        b->release = 1;
    else
        memset(b, 0, sizeof(DdBgSlot));
}
#endif

/* the four words of filelist-ptrs.txt in background `id` (0 and all zero while not loaded) */
int dd_bg_filelist(int id, int *a, int *b, int *c, int *d)
{
    int *fl;
    void *pack = dd_bg_get(id);

    if (pack) {
        fl = ehp_find(pack, "filelist-ptrs.txt", 0);
        if (fl) {
            *a = fl[0];
            *b = fl[1];
            *c = fl[2];
            *d = fl[3];
            return 1;
        }
    }
    *a = 0;
    *b = 0;
    *c = 0;
    *d = 0;
    return 0;
}

/* registers the n cut-ins (character, variant) of the duel; allocates their heap on first use */
void dd_cutin_setup(int *ids, int *subs, int n)
{
    DdCutins *c = &dd_cutins;
    int i;

    if (c->heap == 0)
        c->heap = func_0881CA54(func_08806D04() ? 0x2D0000 : 0x5A0000);
    c->n = 0;
    for (i = 0; i < n; i++) {
        c->slot[c->n].id = ids[i];
        c->slot[c->n].sub = subs[i];
        c->slot[c->n].buf = (char *)c->heap + i * 0x168000;
        c->slot[c->n].loaded = 0;
        c->slot[c->n].req = 0;
        c->n++;
    }
}

/* frees the cut-in heap and resets the slots, keeping character and variant */
void dd_cutin_free(void)
{
    DdCutins *c = &dd_cutins;
    int i;
    DdCutinSlot *s;
    int id;
    int sub;

    if (c->heap) {
        func_0881CA64(c->heap);
        c->heap = 0;
    }
    for (i = 0; i < c->n; i++) {
        s = &c->slot[i];
        id = s->id;
        sub = s->sub;
        memset(s, 0, sizeof(DdCutinSlot));
        s->id = id;
        s->sub = sub;
    }
}

const char *dd_cutin_name(int id, int sub);
int dd_cutin_loaded(DdCutinSlot *s, int size);

/* starts the read of every cut-in not loaded yet */
void dd_cutin_load(void)
{
    DdCutins *c = &dd_cutins;
    DdCutinSlot *s;
    const char *name;
    int i;

    for (i = 0; i < c->n; i++) {
        s = &c->slot[i];
        if (s->loaded == 0) {
            if (s->req == 0) {
                name = dd_cutin_name(s->id, s->sub);
                if (charmap_lookup(s->id, 0) >= 0)
                    s->faces = 1;
                else if (s->id == 0x12 && s->sub != 0)
                    s->faces = 1;
                else
                    s->faces = 0;
                s->req = 1;
                ehfile_read_async_alias(4, name, s->buf, 0x168000, (int (*)(void *, int))dd_cutin_loaded, s);
            }
        }
    }
}

void dd_cutin_faces_load(DdCutinSlot *s);

/* 1 when every cut-in is loaded with its faces and voices (starts the face reads) */
int dd_cutin_ready(void)
{
    DdCutins *c = &dd_cutins;
    int i;
    DdCutinSlot *s;

    for (i = 0; i < c->n; i++) {
        s = &c->slot[i];
        if (!!s->req)
            return 0;
        if (!s->loaded)
            return 0;
    }
    for (i = 0; i < c->n; i++) {
        s = &c->slot[i];
        if (!!s->faces) {
            dd_cutin_faces_load(s);
            return 0;
        }
        if (s->faces_req || s->voice_req)
            return 0;
    }
    return 1;
}

/* pack of cut-in (id, sub), 0 while not loaded */
void *dd_cutin_get(int id, int sub)
{
    DdCutins *c = &dd_cutins;
    int i;
    DdCutinSlot *s;

    for (i = 0; i < c->n; i++) {
        s = &c->slot[i];
        if (id == s->id && sub == s->sub)
            return s->loaded ? s->buf : 0;
    }
    return 0;
}

int dd_res_cmp(const void *a, const void *b)
{
    if (*(unsigned int *)a < *(unsigned int *)b)
        return -1;
    return *(unsigned int *)b < *(unsigned int *)a;
}

int dd_u16_cmp(const void *a, const void *b)
{
    if (*(unsigned short *)a < *(unsigned short *)b)
        return -1;
    return *(unsigned short *)b < *(unsigned short *)a;
}

/* binary search of `id` in a table of n records of `size` bytes whose first word is the id */
int dd_res_find(unsigned int id, void *table, int n, int size)
{
    int stride = size / 4;
    unsigned int *p;
    int lo;
    int mid;
    unsigned int v;

    n++;
    p = (unsigned int *)table - stride;
    lo = 0;
    for (mid = n / 2; lo < mid; mid = (lo + n) / 2) {
        v = p[mid * stride];
        if (v == id)
            return mid - 1;
        if (v < id)
            lo = mid;
        else
            n = mid;
    }
    return -1;
}

#ifdef NON_MATCHING /* o original carrega e->id antes de c->cip_m para a chamada */
/* 1 when the medium image of cache entry i is loaded */
int dd_card_ready_m(int i, int unused)
{
    unsigned int list[0x20];
    DdCards *c = &dd_cards;
    DdCardEnt *e = &c->e[i];

    int hi;

    if (e->id == 0)
        return 1;
    hi = inv_get_hi(e->id);
    return cip_get_image(list, c->cip_m, e->id, hi, 0) != list;
}
#endif

#ifdef NON_MATCHING /* o original carrega e->id antes de c->cip_h para a chamada */
/* 1 when the large image of cache entry i is loaded */
int dd_card_ready_h(int i, int unused)
{
    unsigned int list[0x20];
    DdCards *c = &dd_cards;
    DdCardEnt *e = &c->e[i];

    int hi;

    if (e->id == 0)
        return 1;
    hi = inv_get_hi(e->id);
    return cip_get_image(list, c->cip_h, e->id, hi, 0) != list;
}
#endif

/* read callback of a background; retries with fmagic0000 when the file is too big */
int dd_bg_loaded(int slot, int size)
{
    DdBg *bg = &dd_bg;
    DdBgSlot *b = &bg->slot[slot];

    if (size <= 0) {
        if (b->release)
            goto clear;
        return 0;
    }
    if (size >= 0x80000) {
        ehfile_read_async_alias(3, "duel/bg/fmagic0000.ehp", bg->buf[slot], 0x80000,
                                (int (*)(void *, int))dd_bg_loaded, (void *)slot);
        return 1;
    }
    ehp_mount(0, bg->buf[slot]);
    b->loaded = 1;
    b->busy = 0;
    if (b->release) {
    clear:
        b->buf = 0;
        b->loaded = 0;
        b->busy = 0;
        b->id = 0;
        b->used = 0;
    }
    return 1;
}

/* clears the card image cache and opens the two CIP readers */
void dd_cards_init(void)
{
    memset(&dd_cards, 0, sizeof(DdCards));
    dd_cards.cip_h = cip_open_h(dd_cards.buf_h, 0x1D2000, 0);
    dd_cards.cip_m = cip_open_m(dd_cards.buf_m, 0xEC000, 0);
}

/* closes the CIP readers (waiting for them) and clears the cache */
void dd_cards_term(void)
{
    DdCards *c = &dd_cards;

    if (c->cip_h) {
        cip_close(c->cip_h);
        while (!cip_is_idle(c->cip_h))
            sys_frame_wait();
    }
    memset(c, 0, sizeof(DdCards));
}

/* read callback of a pack: mounts it */
int dd_pack_loaded(DdPack *p, int size)
{
    if (size < 0)
        return 0;
    ehp_mount(0, p->buf);
    p->loading = 0;
    return 1;
}

/* EHP of the cut-in of character `id` (variant `sub`); the duel academy students by colour */
const char *dd_cutin_name(int id, int sub)
{
    CharmapPos pos = { 0 };
    int n;

    switch (id) {
    case 0:
        return "cutin_player01.ehp";
    case 1:
        return "cutin_player01.ehp";
    case 2:
        return "cutin_jyudai01.ehp";
    case 3:
        return "cutin_shou01.ehp";
    case 4:
        return "cutin_hayato01.ehp";
    case 5:
        return "cutin_aska01.ehp";
    case 6:
        if (sub == 0)
            return "cutin_thunder_blk01.ehp";
        return "cutin_thunder01.ehp";
    case 7:
        return "cutin_misawa01.ehp";
    case 8:
        return "cutin_ryo01.ehp";
    case 9:
        return "cutin_kuronosu01.ehp";
    case 10:
        return "cutin_daitokuji01.ehp";
    case 11:
        return "cutin_fubuki01.ehp";
    case 12:
        return "cutin_kamula01.ehp";
    case 13:
        return "cutin_tania01.ehp";
    case 14:
        if (region_get() == 0)
            return "cutin_zaloog01.ehp";
        return "cutin_zaloog01_a.ehp";
    case 15:
        return "cutin_abdos01.ehp";
    case 16:
        return "cutin_titan01.ehp";
    case 17:
        return "cutin_amunael01.ehp";
    case 18:
        if (sub == 0) {
            if (region_get() == 0)
                return "cutin_kagemaru02.ehp";
            return "cutin_kagemaru02_a.ehp";
        }
        return "cutin_kagemaru01.ehp";
    case 19:
        return "cutin_taizan01.ehp";
    case 20:
        return "cutin_mokeo01.ehp";
    case 21:
        return "cutin_junko01.ehp";
    case 22:
        return "cutin_momoe01.ehp";
    case 23:
        return "cutin_sal01.ehp";
    case 24:
        if (sub == 0)
            return "cutin_rei01.ehp";
        return "cutin_rei02.ehp";
    case 25:
        return "cutin_meikyu_m01.ehp";
    case 26:
        return "cutin_meikyu_k01.ehp";
    case 27:
        return "cutin_bmgirl01.ehp";
    case 28:
        return "cutin_psychoshocker01.ehp";
    case 30:
        return "cutin_seiko01.ehp";
    case 31:
        return "cutin_tome01.ehp";
    case 32:
        return "cutin_chousaku01.ehp";
    case 33:
        return "cutin_shouji01.ehp";
    case 36:
        return "cutin_ayukawa01.ehp";
    case 37:
        return "cutin_boy01.ehp";
    case 38:
        return "cutin_sala01.ehp";
    case 40:
        return "cutin_fubuki_drk01.ehp";
    }
    n = charmap_lookup(id, 0);
    charmap_lookup2(n, &pos);
    switch (n) {
    case 1000:
        return "cutin_da_teacher01.ehp";
    }
    switch (pos.a + pos.b * 10) {
    case 0:
        return "cutin_da_boy_r01.ehp";
    case 1:
        return "cutin_da_boy_r02.ehp";
    case 2:
        return "cutin_da_boy_r03.ehp";
    case 3:
        return "cutin_da_boy_r04.ehp";
    case 4:
        return "cutin_da_boy_r05.ehp";
    case 10:
        return "cutin_da_boy_y01.ehp";
    case 11:
        return "cutin_da_boy_y02.ehp";
    case 12:
        return "cutin_da_boy_y03.ehp";
    case 13:
        return "cutin_da_boy_y04.ehp";
    case 14:
        return "cutin_da_boy_y05.ehp";
    case 20:
        return "cutin_da_boy_b01.ehp";
    case 21:
        return "cutin_da_boy_b02.ehp";
    case 22:
        return "cutin_da_boy_b03.ehp";
    case 23:
        return "cutin_da_boy_b04.ehp";
    case 24:
        return "cutin_da_boy_b05.ehp";
    case 35:
        return "cutin_da_girl01.ehp";
    case 36:
        return "cutin_da_girl02.ehp";
    case 37:
        return "cutin_da_girl03.ehp";
    case 38:
        return "cutin_da_girl04.ehp";
    case 39:
        return "cutin_da_girl05.ehp";
    default:
        return "cutin_jyudai01.ehp";
    }
}

/* read callback of a cut-in pack */
int dd_cutin_loaded(DdCutinSlot *s, int size)
{
    if (size <= 0)
        return 0;
    ehp_mount(0, s->buf);
    s->loaded = 1;
    s->req = 0;
    return 1;
}

int dd_cutin_faces_loaded(DdCutinSlot *s, int size);
int dd_cutin_voice_loaded(DdCutinSlot *s, int size);

/* reads the face texture (cutin_da_faces/DA_xxxNN.gim) and the voice pack of a cut-in */
void dd_cutin_faces_load(DdCutinSlot *s)
{
    CharmapPos pos = { 0 };
    char path[0x100] = { 0 };
    void *buf;
    int n;
    const char *kind;
    int voice;

    buf = ehp_find(s->buf, "texture_#10500.txt", 0);
    if (buf) {
        n = charmap_lookup(s->id, 0);
        charmap_lookup2(n, &pos);
        strcpy(path, "cutin_da_faces/");
        switch (pos.b) {
        case 0:
            kind = "DA_boy_R";
            break;
        case 1:
            kind = "DA_boy_Y";
            break;
        case 2:
            kind = "DA_boy_B";
            break;
        case 3:
            kind = "DA_girl";
            break;
        default:
            kind = "";
            break;
        }
        strcat(path, kind);
        sprintf(path + strlen(path), "%02d", pos.c + 1);
        strcat(path, ".gim");
        s->faces_req = 1;
        ehfile_read_async_alias(4, path, buf, 0x10500, (int (*)(void *, int))dd_cutin_faces_loaded, s);
    }
    buf = ehp_find(s->buf, "voice_#10000.txt", 0);
    if (buf) {
        if (s->id == 0x12) {
            strcpy(path, "cutin_kagemaru03msg.ehp");
        } else {
            voice = charmap_lookup3(s->id);
            if (n < 0x15E)
                voice += 0x32;
            else if (n < 0x1C2)
                voice += 0x41;
            else if (n == 1000)
                voice += 0x50;
            else
                voice = 0x32;
            sprintf(path, "zakov%02d.ehp", voice);
        }
        s->voice_req = 1;
        ehfile_read_async_alias(4, path, buf, 0x10000, (int (*)(void *, int))dd_cutin_voice_loaded, s);
    }
    s->faces = 0;
}

/* read callback of the face texture */
int dd_cutin_faces_loaded(DdCutinSlot *s, int size)
{
    if (size <= 0)
        return 0;
    s->faces_req = 0;
    return 1;
}

/* read callback of the voice pack: mounts it */
int dd_cutin_voice_loaded(DdCutinSlot *s, int size)
{
    if (size <= 0)
        return 0;
    ehp_mount(0, ehp_find(s->buf, "voice_#10000.txt", 0));
    s->voice_req = 0;
    return 1;
}
