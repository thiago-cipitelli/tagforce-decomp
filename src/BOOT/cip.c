// @range 0x08853A2C-0x08854920
// @data 0x088EF28C-0x088EF468
// @rodata 0x088B4960-0x088B499C
// @asm 0x08853C94-0x08853D1C
// @asm 0x0885478C-0x0885488C
/* Scene stack (push/pop/reset of gmodule scenes by id, with the transition record the scene reads
   on start) and the CIP/CPM card image reader: slot ring filled by ehfile_read_async callbacks,
   either on demand (cip_read_card/cip_get_image) or sequentially 10 slots at a time (prefetch). */
#include "psp.h"
#include "libc.h"
#include "game.h"
#include "ehfile.h"
#include "ehsys.h"
#include "cip.h"

unsigned char *sys_info_get(void);
void *gim_set_texture(void *list, void *gim);
void *dl_tex_mode(void *list, int pixfmt, int maxmips, int a2, int swizzle);
void *dl_tex_image(void *list, int mip, int w, int h, int stride, const void *data);

int cip_read_cb(void *handle, int n);
int cip_header_ready(Cip *c, int n);
int cip_load_next(Cip *c);
unsigned char *cip_alloc_slot(Cip *c);
CipEntry *cip_table(Cip *c);                 /* not static: cip_prefetch_cb's asm references it */
int cip_load_cb(Cip *c, int n);
void cip_prefetch(Cip *c, int idx);
int cip_prefetch_cb(void *handle, int n);
static int cip_is_cpm(Cip *c);             /* static: lets the compiler keep t-registers live across the call in cip_draw */
void *cip_draw(void *list, Cip *c, void *data);

const char *scene_module_name(int id)
{
    switch (id) {
    case 1: return "libdebug_menu";
    case 2: return "libstory";
    case 3: return "libduel_draw";
    case 4: return "libdeck";
    case 5: return "libfield";
    case 6: return "libduel_mgr";
    case 7: return "libshop";
    case 8: return "libsdchr_viewer";
    case 9: return "liblabo";
    case 10: return "libmovie_viewer";
    case 11: return "libcutin_viewer";
    case 12: return "libsand_box";
    case 13: return "libtitle";
    case 14: return "libhtml_view";
    default: return "unknown";
    }
}

void scene_push(unsigned short from, unsigned short scene, unsigned short arg)
{
    SceneStack *s = (SceneStack *)sys_info_get();
    SceneTrans *e = &s->stack[s->count++];

    e->type = 1;
    e->from = from;
    e->scene = scene;
    e->arg = arg;
    memcpy(&s->cur, e, sizeof(SceneTrans));
    ehscript_set_module(scene_module_name(scene));
}

/* Returns to the scene recorded by the last scene_push (its `from`). */
void scene_pop_to(unsigned short from, unsigned short arg)
{
    SceneStack *s = (SceneStack *)sys_info_get();
    SceneTrans *e = &s->stack[s->count - 1];

    s->cur.type = 2;
    s->cur.from = from;
    s->cur.scene = e->from;
    s->cur.arg = arg;
    s->count--;
    ehscript_set_module(scene_module_name(s->cur.scene));
}

/* The `from` of the top entry when it was pushed for `scene`, else 0xF. */
unsigned short scene_stack_peek_arg(unsigned short scene)
{
    SceneStack *s = (SceneStack *)sys_info_get();
    SceneTrans *e;

    if (s->count == 0)
        return 0xF;
    e = &s->stack[s->count - 1];
    if (e->scene != scene)
        return 0xF;
    return e->from;
}

void scene_get_transition_info(SceneTrans *out)
{
    SceneStack *s = (SceneStack *)sys_info_get();
    memcpy(out, &s->cur, sizeof(SceneTrans));
}

#ifdef NON_MATCHING /* only register allocation differs: the original keeps (int)scene in a 5th s-register across memset */
void scene_reset_to(unsigned short from, unsigned short scene, unsigned short arg)
{
    SceneStack *s = (SceneStack *)sys_info_get();

    memset(s, 0, sizeof(SceneStack));
    s->cur.type = 3;
    s->cur.from = from;
    s->cur.scene = scene;
    s->cur.arg = arg;
    ehscript_set_module(scene_module_name(scene));
}

#endif

const char *cip_names_h[6] = {
    "card/cardh_j.cip", "card/cardh_e.cip", "card/cardh_g.cip",
    "card/cardh_f.cip", "card/cardh_i.cip", "card/cardh_s.cip",
};

void *cip_open_h(void *handle, int bufsize, int prefetch)
{
    Cip *c = (Cip *)handle;

    memset(c, 0, sizeof(Cip));
    c->dir = 3;
    c->bufsize = bufsize;
    c->prefetch = prefetch;
    strcpy(c->path, cip_names_h[cfg_get_language()]);
    ehfile_read_async(c->dir, c->path, &c->hdr, 0, 0x800, (EhfileCallback)cip_read_cb, c);
    return c;
}

const char *cip_names_m[6] = {
    "card/cardm_j.cip", "card/cardm_e.cip", "card/cardm_g.cip",
    "card/cardm_f.cip", "card/cardm_i.cip", "card/cardm_s.cip",
};

void *cip_open_m(void *handle, int bufsize, int prefetch)
{
    Cip *c = (Cip *)handle;

    memset(c, 0, sizeof(Cip));
    c->dir = 3;
    c->bufsize = bufsize;
    c->prefetch = prefetch;
    strcpy(c->path, cip_names_m[cfg_get_language()]);
    ehfile_read_async(c->dir, c->path, &c->hdr, 0, 0x800, (EhfileCallback)cip_read_cb, c);
    return c;
}

int cip_is_loaded(Cip *c)
{
    return c->ready;
}

void cip_close(Cip *c)
{
    c->closed = 1;
}

/* Nonzero when no read is in flight (and, in prefetch mode, the whole file has been read). */
int cip_is_idle(Cip *c)
{
    if (c->busy)
        return 0;
    if (c->prefetch && c->prefetch_idx >= 0)
        return 0;
    return 1;
}

/* Returns the image pointer when card `id` (variant `variant`) is loaded, else queues it and returns 0. */
unsigned int cip_read_card(void *handle, int id, int variant)
{
    Cip *c = (Cip *)handle;
    CipEntry *e;
    unsigned char *base;
    int i, idx;

    if (c == 0 || !c->ready || c->closed)
        return 0;
    if (id < c->hdr.first_id || c->hdr.last_id < id)
        return 0;
    if (cip_is_cpm(c))
        return 0;
    idx = id - c->hdr.first_id;
    base = (unsigned char *)&c->hdr;
    e = &cip_table(c)[idx];
    if (e->chunk == 0)
        return 0;
    if (e->chunk & 0x80000000) {
        e = (CipEntry *)(base + (e->chunk & 0x7FFFFFFF));
        for (i = 0; i < variant && !(e->chunk & 0x80000000); i++)
            e++;
    }
    if (e->data > 0x10)
        return e->data;
    if (e->data == 2)
        return 0;
    e->data = 1;
    if (!c->busy) {
        c->busy = 1;
        if (cip_load_next(c) == 0)
            c->busy = 0;
    }
    return 0;
}

/* Appends the texture commands for card `id` to the display list when loaded (queues it otherwise). */
unsigned int *cip_get_image(unsigned int *list, void *handle, int id, int variant, int *loaded_out)
{
    Cip *c = (Cip *)handle;
    CipEntry *e;
    unsigned char *base;
    int i, first;

    if (loaded_out != 0)
        *loaded_out = 0;
    if (c == 0 || !c->ready || c->closed)
        return list;
    first = c->hdr.first_id;
    if (id < first || c->hdr.last_id < id)
        return list;
    base = (unsigned char *)&c->hdr;
    e = &cip_table(c)[id - first];
    if (e->chunk == 0) {
        *list = 0;
        return list + 1;
    }
    if (e->chunk & 0x80000000) {
        e = (CipEntry *)(base + (e->chunk & 0x7FFFFFFF));
        for (i = 0; i < variant && !(e->chunk & 0x80000000); i++)
            e++;
    }
    if (e->data > 0x10) {
        if (loaded_out != 0)
            *loaded_out = 1;
        return cip_draw(list, c, (void *)e->data);
    }
    if (e->data == 2)
        return list;
    e->data = 1;
    if (!c->busy) {
        c->busy = 1;
        if (cip_load_next(c) == 0)
            c->busy = 0;
    }
    return list;
}

/* 1 when the image is not loaded yet, 0 when it is; negative on bad arguments. */
int cip_query(void *handle, int id, int variant)
{
    Cip *c = (Cip *)handle;
    CipEntry *e;
    int i, first;

    if (handle == 0 || c->closed)
        return -1;
    if (!cip_is_loaded(c))
        return -2;
    first = c->hdr.first_id;
    if (id < first || c->hdr.last_id < id)
        return -3;
    e = &cip_table(c)[id - first];
    if (e == 0 || e->chunk == 0)
        return -3;
    if ((e->chunk & 0x80000000) == 0) {
        if (variant != 0)
            return -4;
    } else {
        e = (CipEntry *)((unsigned char *)&c->hdr + (e->chunk & 0x7FFFFFFF));
        for (i = 0; i < variant && !(e->chunk & 0x80000000); i++)
            e++;
        if (i != variant)
            return -4;
    }
    if (e->data < 0x11)
        return 1;
    return 0;
}

/* ehfile callback for the first 0x800 bytes: fetch the rest of the table if it is larger. */
int cip_read_cb(void *handle, int n)
{
    Cip *c;

    if (n <= 0)
        return 0;
    c = (Cip *)handle;
    if (c->hdr.table_size <= 0x800)
        return cip_header_ready(c, n);
    ehfile_read_async(c->dir, c->path, (unsigned char *)&c->hdr + 0x800, 0x800, c->hdr.table_size - 0x800,
                      (EhfileCallback)cip_header_ready, c);
    return 1;
}

int cip_header_ready(Cip *c, int n)
{
    if (n <= 0)
        return 0;
    c->slots = (unsigned char *)c + c->hdr.table_size + 0xAC;
    c->slots = (unsigned char *)(((int)c->slots + 15) & ~15);
    c->nslots = (unsigned int)(c->bufsize - (int)c->slots + (int)c) / (unsigned int)(c->hdr.chunks << 11);
    c->next_slot = 0;
    c->unka0 = 0;
    c->ready = 1;
    if (c->prefetch)
        cip_prefetch(c, 0);
    return 1;
}

/* Starts reading the first requested entry; 0 when there is none. */
int cip_load_next(Cip *c)
{
    CipEntry *e;
    unsigned char *buf;

    if (c->closed)
        return 0;
    for (e = cip_table(c); e->chunk != 0xFFFFFFFF; e++) {
        if (e->data == 1)
            break;
    }
    if (e->chunk == 0xFFFFFFFF)
        return 0;
    if (c->prefetch)
        buf = c->slots + (((e->chunk & 0x7FFFFFFF) - (((c->hdr.table_size + 0x7FF) & ~0x7FF) >> 11)) << 11);
    else
        buf = cip_alloc_slot(c);
    c->cur = e;
    c->cur_buf = buf;
    e->data = 2;
    ehfile_read_async(c->dir, c->path, buf, (e->chunk & 0x7FFFFFFF) << 11, c->hdr.chunks << 11,
                      (EhfileCallback)cip_load_cb, c);
    return 1;
}

/* Next slot of the ring; any entry still pointing at it is unloaded. */
unsigned char *cip_alloc_slot(Cip *c)
{
    CipEntry *e;
    unsigned char *buf;

    buf = c->slots + ((c->next_slot * c->hdr.chunks) << 11);
    c->next_slot++;
    if (c->next_slot >= c->nslots)
        c->next_slot = 0;
    for (e = cip_table(c); e->chunk != 0xFFFFFFFF; e++) {
        if (e->data == (unsigned int)buf)
            e->data = 0;
    }
    return buf;
}

CipEntry *cip_table(Cip *c)
{
    return (CipEntry *)((unsigned char *)&c->hdr + c->hdr.table_off);
}

int cip_load_cb(Cip *c, int n)
{
    if (n <= 0) {
        if (!c->closed)
            return 0;
    } else {
        c->cur->data = (unsigned int)c->cur_buf;
        c->cur = 0;
        c->cur_buf = 0;
        if (cip_load_next(c) != 0)
            goto done;                  /* the original shares one `busy = 0` block between both paths */
    }
    c->busy = 0;
done:
    return 1;
}

/* Reads 10 slots starting at slot `idx` straight into the ring (prefetch mode). */
void cip_prefetch(Cip *c, int idx)
{
    int off;

    if (c->closed) {
        c->prefetch_idx = -1;
        return;
    }
    c->prefetch_idx = idx;
    off = (idx * c->hdr.chunks) << 11;
    ehfile_read_async(c->dir, c->path, c->slots + off, ((c->hdr.table_size + 0x7FF) & ~0x7FF) + off,
                      c->hdr.chunks * 0x5000, (EhfileCallback)cip_prefetch_cb, c);
}

#ifdef NON_MATCHING /* with cip_table static (mwcc then keeps t-registers across the call; impossible while this asm references it) the frame matches and only buf/chunk swap a2/a3 */
int cip_prefetch_cb(void *handle, int n)
{
    Cip *c = (Cip *)handle;
    CipEntry *e;
    unsigned char *buf;
    unsigned int chunk, base;
    int nread, slot, i;
    unsigned int chunks;

    if (n < 0) {
        if (!c->closed)
            return 0;
    } else {
        base = ((c->hdr.table_size + 0x7FF) & ~0x7FF) >> 11;
        nread = n / (unsigned int)(c->hdr.chunks << 11);
        for (i = 0; i < nread; i++) {
            chunks = c->hdr.chunks;
            slot = c->prefetch_idx + i;
            buf = c->slots + slot * (chunks << 11);
            chunk = base + chunks * slot;
            for (e = cip_table(c); e->chunk != 0xFFFFFFFF; e++) {
                if ((e->chunk & 0x7FFFFFFF) == chunk) {
                    e->data = (unsigned int)buf;
                    break;
                }
            }
        }
        if (nread == 10) {
            cip_prefetch(c, c->prefetch_idx + 10);
            return 1;
        }
    }
    c->prefetch_idx = -1;
    return 1;
}

#endif

static int cip_is_cpm(Cip *c)
{
    return c->hdr.magic == 0x1A4D5043;   /* "CPM\x1a" */
}

/* CIP images are GIMs; CPM images are raw 64x64 CLUT textures. */
void *cip_draw(void *list, Cip *c, void *data)
{
    if (!cip_is_cpm(c))
        return gim_set_texture(list, data);
    list = dl_tex_mode(list, 8, 0, 0, 0);
    return dl_tex_image(list, 0, 0x40, 0x40, 0x40, data);
}
