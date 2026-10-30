// @range 0x08859CC0-0x08859F18
// @asm 0x08859CC0-0x08859DF8
/* Field zone helpers of the duel scene: send a zone command to the engine and refresh the card
   object of the zone; the three saved view matrices; and the confirm button that hurries the
   opponent's animation. */
#include "dd_duelui.h"

typedef struct ZoneArg {               /* second argument of zone_cmd (u16) */
    unsigned short lo : 14;
    unsigned short big : 1;            /* +1 */
    unsigned short huge : 1;           /* +10 */
} ZoneArg;

extern DdMat4 zone_views[3];           /* 0x08C5B300 */

int dv_side_to_player(int side);
int dv_slot_uid(int side, int z);
void dv_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void *func_0880B7B8(int side, int zone, int index);
void func_0880C758(short arg, int value, int uid);
void func_0880C748(void *card, int value, int uid);
unsigned int *func_0881DE50(void);
void func_0884DB3C(void);
int mtx_get_mode(void);
void mat4_identity(DdMat4 *m);

#ifdef NON_MATCHING /* empacotamento dos campos de bits diferente; não insistido */
void zone_cmd(PopupLoc loc, ZoneArg arg, int value)
{
    int side = loc.side;
    int zone = loc.zone;
    int index = loc.index;
    char *card;

    dv_cmd((dv_side_to_player(side) ? 0x8000 : 0) | 0x44, zone | (index & 0xFF) << 8, value,
           arg.huge * 10 + arg.big);
    card = func_0880B7B8(side, zone, index);
    if (card == 0) {
        func_0880C758(*(short *)&arg, value, dv_slot_uid(side, zone));
        card = func_0880B7B8(side, zone, index);
        if (card == 0)
            return;
    }
    func_0880C748(card, value, *(unsigned short *)(card + 0x88));
}
#else
void zone_cmd(PopupLoc loc, ZoneArg arg, int value);
#endif

void zone_views_init(void)
{
    DdMat4 *m = zone_views;
    int i;

    memset(zone_views, 0, sizeof(zone_views));
    for (i = 0; i < 3; i++, m++)
        mat4_identity(m);
}

void zone_views_term(void)
{
    memset(zone_views, 0, sizeof(zone_views));
}

void zone_view_save(int i)
{
    int mode = mtx_get_mode();

    mtx_set_mode(1);
    mtx_get34(&zone_views[i]);
    mtx_set_mode(mode);
}

void zone_nop1(void)
{
}

void zone_nop2(void)
{
}

void *zone_identity(void *p)
{
    return p;
}

void zone_confirm_hurry(void)
{
    unsigned int *pad = func_0881DE50();

    if (pad[1] & region_pick_value(1))
        func_0884DB3C();
}
