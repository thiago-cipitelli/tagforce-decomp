// @range 0x088058E0-0x08805BE4
// @rodata 0x08820F00-0x08820F08
// @data 0x08821D18-0x08821E34
/* Resources of the lesson: deck/tutorial.ehp read asynchronously into the heap and mounted; the
   screen's textures and the lesson's string table looked up by id on demand. */
#include "libdeck_tutorial.h"

int dt_pack_loaded(void *pack, int size);
int dt_bsearch(unsigned short key, void *base, int n, int size);

static const unsigned char dt_lang[8] = "jegfis";   /* language letters by cfg_get_language() */

ScPack dt_files[9] = {
    { 0x1010, 0, "story_sys01.gim" },
    { 0x1011, 0, "window.gim" },
    { 0x1012, 0, "button.gim" },
    { 0x1013, 0, "large_wnd.gim" },
    { 0x1014, 0, "large_pict00_%c.gim" },
    { 0x1015, 0, "large_pict01_%c.gim" },
    { 0x1016, 0, "large_pict02_%c.gim" },
    { 0x1040, 0, "strTbl_%c.bin" },
    { 0x1041, 0, "deck_tutorial%02d.bin" },
};

void dt_pack_free(void)
{
    dt_pack_release();
}

void dt_pack_release(void)
{
    if (dt_pack) {
        heap_set_level(dts.heap, dt_pack);
        dt_pack = 0;
    }
}

void *dt_pack_load(void)
{
    int size;

    if (dt_pack)
        return dt_pack;
    if (dt_res.ready) {
        dt_res.ready = 0;
        size = ehfile_get_size(3, "deck/tutorial.ehp");
        dt_pack = heap_alloc(dts.heap, size);
        ehfile_read_async_alias(3, "deck/tutorial.ehp", dt_pack, size, dt_pack_loaded, dt_pack);
    }
    return 0;
}

/* %02d = lesson for the lesson's script, %c = language letter for the others */
void *dt_res_get(unsigned short id)
{
    ScPack *f;
    void *pack;
    char name[256];
    int i = dt_bsearch(id, dt_files, 9, sizeof(ScPack));

    if (i < 0)
        return 0;
    f = &dt_files[i];
    if (f->data)
        return f->data;
    pack = dt_pack_load();
    if (pack == 0)
        return 0;
    if (id == 0x1041)
        sprintf(name, f->name, dt.lesson);
    else
        sprintf(name, f->name, dt_lang[cfg_get_language()]);
    return f->data = ehp_find(pack, name, 0);
}

int dt_pack_loaded(void *pack, int size)
{
    int ok = size > 0;

    if (ok) {
        if (pack)
            ehp_mount(0, pack);
        dt_res.step++;
    }
    dt_res.ready = 1;
    return ok;
}

int dt_res_cmp(const void *a, const void *b)
{
    return *(const unsigned short *)a - *(const unsigned short *)b;
}

/* binary search over records of `size` bytes keyed by their first u16 */
int dt_bsearch(unsigned short key, void *base, int n, int size)
{
    int half = size / 2;
    int lo = 0;
    int hi = n - 1;
    int mid;
    unsigned short v;

    while (lo <= hi) {
        mid = (lo + hi) / 2;
        v = ((unsigned short *)base)[mid * half];
        if (key == v)
            return mid;
        if (v < key)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return -1;
}
