// @range 0x08812510-0x08812880
/* Per frame: the time limit of the match (the time-out popup when it runs out), the cursor timer
   ("new" mark cleared after 16 frames), the info text when the card changes, and the card info
   screen (same source as libselect_card sc_state.c). */
#include "libdeck_swap.h"

int sysvar_get_a98(void);
void fade_finish(void);
void cardinfo_open(int id, void *gim, void *bg);
int cardinfo_update(void);
void cardinfo_set_lists(void *gim, void *names, void *list);
int cardinfo_has_alt_pict(void);
DsEnt *ds_cur_entry(unsigned short side);
void ds_info_update(void);
void ds_popup_open(unsigned short id, unsigned short arg);
void ds_info_anim(void);

void ds_update_anim(void)
{
    DsEnt *e;

    if (sysvar_get_a98() && (--ds.time_limit, ds.state < 9) && ds.time_limit <= 0 && !dsv.busy && dsv.popup != 5) {
        fade_finish();
        dsv.info = 0;
        ds.info_open = 0;
        ds_popup_open(5, 0);
    }
    if (ds.info_open) {
        ds_info_anim();
    } else {
        if (++dsv.cursor.timer >= 0x10) {
            if (dsv.cursor.ent != 0)
                dsv.cursor.ent->marked = 1;
            if (dsv.cursor.timer > 0x8000)
                dsv.cursor.timer = 0x100;
        }
        if (!dsv.lock && (e = ds_cur_entry(dsv.side)) != 0 && e->id != dsv.cursor.id)
            ds_info_update();
    }
}

/* the card info screen: fade out, open, run until closed, fade back */
void ds_info_anim(void)
{
    DsEnt *e;

    switch (ds.info_step) {
    case 0:
        fade_start_in(1, 0xFF000000, 16);
        ds.info_step++;
    case 1:
        if (fade_is_done() == 0)
            return;
        e = ds_cur_entry(dsv.side);
        cardinfo_open(e->id, res_get(0x800C), res_get(0x8012));
        cardinfo_set_lists(res_get(0x8015), res_get(0x8036), res_get(0x8037));
        cardinfo_has_alt_pict();
        dsv.info = 1;
        ds.info_step++;
        return;
    case 2:
        if (cardinfo_update() == 0)
            return;
        dsv.info = 0;
        fade_start(1, 0xFF000000, 16);
        ds.info_step++;
    case 3:
        if (fade_is_done()) {
            ds.info_step = 0;
            ds.info_open = 0;
        }
        return;
    default:
        return;
    }
}
