// @range 0x0882A918-0x0882ABD4
/* Ambient sounds of the rooms (field ambience): the loop started on entering each room, and the
   volume of the fountain of room 0x1D by the distance of the player. */
#include "libfield.h"
#include "allegrex.h"

FieldStory *fld_story_get(void);
int fld_bg_id(void);
void fld_bgm_volume(int a, int b);
void fld_player_get_pos(Vec3 *out);

void famb_start(void)
{
    int early = *(int *)fld_story_get()->w[3] < 0x16;

    switch (fld_bg_id()) {
    case 0x0D:
    case 0x0E:
    case 0x0F:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x26:
    case 0x15:
        if (!early) {
            fld_story_flag(0x6256, 0);
            fld_bgm_volume(0x6256, 0x60);
        }
        break;
    case 0x0C:
    case 0x25:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x01:
        if (!early)
            fld_story_flag(0x6256, 0);
        break;
    case 0x1A:
    case 0x19:
    case 0x1F:
        fld_story_flag(0x6257, 0);
        break;
    case 0x1D:
        if (early)
            fld_story_flag(0x6254, 0);
        else
            fld_story_flag(0x6256, 0);
        fld_story_flag(0x6253, 0);
        break;
    case 0x1E:
        fld_story_flag(0x6258, 0);
        break;
    case 0x20:
        fld_story_flag(0x6255, 0);
        break;
    case 0x27:
        fld_story_flag(0x6258, 0);
        break;
    }
}

void snd_cmd_fef9_arg(int arg);

void famb_stop(void)
{
    snd_cmd_fef9_arg(1500);
}

void famb_update(void)
{
    Vec3 pos;
    float dx;
    float dz;
    float d;
    int v;

    memset(&pos, 0, sizeof(pos));
    fld_player_get_pos(&pos);
    switch (fld_bg_id()) {
    case 0x1D:
        dx = pos.x - 15.75f;
        dz = pos.z - 9.75f;
        d = sqrtf_vfpu(dx * dx + dz * dz);
        v = 7.0555553f * (18.0f - d);
        v = ALLEGREX_MAX(v, 0);
        bgm_set_volume(0x6253, ALLEGREX_MIN(v, 0x7F), 0);
        break;
    }
}
