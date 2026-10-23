// @range 0x08806D4C-0x08808A38
// @rodata 0x0880C620-0x0880C684
// @data 0x0880CD20-0x0880D314
/* The machine's states: intro message while the meter flickers and the lid opens, picking the cards
   with libselect_card (at least 10), cards flying into the slot, the conversion (cards burning in a
   spiral, the machine shaking and glowing, the new card flying out), the result with its name and
   card detail, and the "convert again?" question. The card that comes out is a random one of the
   group indexed by the number of cards fed in (cm_groups). */
#include "libconv_machine.h"

typedef struct RandSeed {       /* BOOT rand3_seeds entry, 12 bytes */
    unsigned int lcg;           /* x * 0x41C64E6D + 0x3039 */
    unsigned int b, c;
} RandSeed;

typedef struct CmGroup {        /* cards that can come out for a given number of cards fed in */
    unsigned short *cards;
    int count;
} CmGroup;

typedef struct CmAnim {         /* EHF animation slot: resource and playback speed (0 stopped) */
    unsigned short id;
    unsigned short speed;
} CmAnim;

extern int rand3_slot;          /* BOOT variable imports, 0 in the relocated ELF */
extern RandSeed rand3_seeds[];

unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
void bgm_stop(int x);
void bgm_cmd(int x);
int rand_next_alias(void);
float cosf_vfpu(float x);
float sinf_vfpu(float x);
unsigned short *card_get_name(int id);
void inv_adjust_count(int id, int dir);
int cardinfo_open(unsigned short card_id, void *window_gim, void *bg_gim);
void cardinfo_set_lists(void *list_gim, void *text_pack, void *index);
int cardinfo_has_alt_pict(void);
int cardinfo_update(void);
int ehf_set_anim(void *e, int target, int limit);
int ehf_set_frame_alias(void *e, int f);
int ehf_get_data_ptr(void *e);
int ehf_get_frame_count(void *e);
void libselect_card_08804008(int mode, void **args, int *n, unsigned short *cards);
int libselect_card_088041C4(void);

unsigned short cm_pick_card(int group, int n, unsigned short *cards);
void cm_meter_set(unsigned short v);
void cm_anim_set(unsigned short which, unsigned short speed);
int cm_anim_length(unsigned short which);
void cm_anim_sounds(void);

extern CmGroup cm_groups[90];
extern CmAnim cm_anims[9];

/* state 1: intro message, the meter flickers while it types; the lid opens */
void cm_state_intro(void)
{
    RandSeed *r;

    switch (cm.step) {
    case 0:
        wcscpy(cm.text, table_get_entry(cm_res_get(CM_STRINGS), 1));
        cm.text_len = wtext_count_tags(cm.text);
        cm.text_pos = 0;
        cm.timer = 0;
        cm.step++;
        rand3_seeds[rand3_slot].lcg = 0;
        break;
    case 1:
        if (fade_is_done()) {
            cm.timer = 0;
            cm.typing = 1;
            cm.step++;
            se_play(0x627D);
        }
        break;
    case 2:
        r = &rand3_seeds[rand3_slot];
        r->lcg *= 0x41C64E6D;
        r->lcg += 0x3039;
        cm_meter_set((int)(r->lcg >> 16 & 0x7FFF) % 100);
        if (++cm.timer >= 2) {
            if (++cm.text_pos >= cm.text_len) {
                cm.step++;
                cm.typing = 0;
            }
            cm.timer = 0;
        }
        break;
    case 3:
        r = &rand3_seeds[rand3_slot];
        r->lcg *= 0x41C64E6D;
        r->lcg += 0x3039;
        cm_meter_set((int)(r->lcg >> 16 & 0x7FFF) % 100);
        if (cm.timer++ > 24) {
            wcscpy(cm.text, table_get_entry(cm_res_get(CM_STRINGS), 2));
            cm_meter_set(0x65);
            cm_anim_set(0, 1);
            cm.text_len = wtext_count_tags(cm.text);
            cm.text_pos = 0;
            cm.timer = 0;
            cm.typing = 1;
            cm.step++;
            bgm_stop(0x627D);
            if (!cm.lid_done) {
                cm.lid = 1;
                se_play(0x626F);
            }
        }
        break;
    case 4:
        if (++cm.timer >= 2) {
            if (++cm.text_pos >= cm.text_len) {
                cm.step++;
                cm.typing = 0;
            }
            cm.timer = 0;
        }
        break;
    case 5:
        if (cm.lid == 0 && (pad_get()->b.trig & 0x6000)) {
            cm.state = 2;
            cm.step = 0;
            cm.timer = 0;
        }
        break;
    }
}

/* state 2: libselect_card picks the cards; fewer than 10 → message and back to the question */
void cm_state_select(void)
{
    void *args[4];
    int i;

    switch (cm.step) {
    case 0:
        fade_start_in(1, 0xFF000000, 16);
        cm.step++;
    case 1:
        if (fade_is_done())
            cm.step++;
        break;
    case 2:
        args[0] = cm.heap;
        args[1] = cm.cip;
        args[2] = cm_pack_get(1);
        args[3] = cm_pack_get(2);
        cm.n_cards = 0;
        libselect_card_08804008(1, args, &cm.n_cards, cm.cards);
        cm.selecting = 1;
        cm.step++;
        break;
    case 3:
        if (libselect_card_088041C4() == 1) {
            gfx_overlay_enable(0);
            fade_start(1, 0xFF000000, 16);
            cm.step++;
            cm.selecting = 0;
        }
        break;
    case 4:
        if (fade_is_done()) {
            if (cm.n_cards < 10) {
                cm.step++;
                break;
            }
            for (i = 0; i < cm.n_cards; i++)
                inv_adjust_count(cm.cards[i], 0);
            memset(cm.flying, 0, sizeof(cm.flying));
            cm.state = 3;
            cm.step = 0;
            cm.timer = 0;
        }
        break;
    case 5:
        wcscpy(cm.text, table_get_entry(cm_res_get(CM_STRINGS), 5));
        cm.text_len = wtext_count_tags(cm.text);
        cm.text_pos = 0;
        cm.step = 0;
        cm.timer = 0;
        cm.typing = 1;
        cm.state = 7;
        cm_anim_set(0, 0);
        break;
    }
}

/* state 3: the cards fly one after the other into the slot, the lid closes, the meter counts them */
void cm_state_insert(void)
{
    CmCard *c;
    int i;

    switch (cm.step) {
    case 0:
        for (i = 0; i < cm.n_cards; i++) {
            cm.flying[i].x = 0;
            cm.flying[i].y = 250.0f + 0.01f * i;
            cm.flying[i].z = 20.0f;
            cm.flying[i].state = 1;
            cm.flying[i].t = -3 * i - 4;
        }
        cm.step++;
        break;
    case 1:
        for (i = 0; i < cm.n_cards; i++) {
            cm.flying[i].t++;
            c = &cm.flying[i];
            switch (c->state) {
            case 1:
                if (c->t > 21) {
                    c->t = 0;
                    c->state = 2;
                } else if (c->t >= 0) {
                    if (c->t == 0)
                        se_play(0x6270 + rand_next_alias() % 4);
                    c->y -= 10.476191f;
                    c->z -= 1.4285715f;
                }
                break;
            }
        }
        if (c->state == 2) {
            cm_anim_set(0, 0);
            cm.lid = 2;
            cm.step++;
            se_play(0x6275);
        }
        break;
    case 2:
        if (cm.lid == 0) {
            cm.timer = 0;
            cm.step++;
        }
        break;
    case 3:
        if (++cm.timer >= cm.n_cards) {
            cm_meter_set(cm.n_cards);
            cm.state = 4;
            cm.step = 0;
        } else {
            cm_meter_set(cm.timer);
        }
        break;
    }
}

/* state 4: the conversion; any button from step 2 to 7 skips to the flash */
void cm_state_convert(void)
{
    CmCard *c;
    float r;
    float a;
    int i;
    int t;

    if (cm.step >= 2 && cm.step < 8 && (pad_get()->b.trig & 0x6000)) {
        for (i = 0; i < cm.n_cards; i++) {
            c = &cm.flying[i];
            c->state = 4;
            c->t = 0;
        }
        cm_anim_set(6, 0);
        cm.result = cm_pick_card(cm.n_cards, cm.n_cards, cm.cards);
        cm_anim_set(7, 0);
        bgm_stop(0x6276);
        cm.flash_fade = 2;
        cm.flash = 0xFF;
        cm_anim_set(1, 1);
        cm_anim_set(2, 1);
        cm.glow = 0;
        cm.zoom = 0;
        cm.timer = 0;
        cm.step = 8;
    }
    switch (cm.step) {
    case 0:
        wcscpy(cm.text, table_get_entry(cm_res_get(CM_STRINGS), 4));
        cm.text_len = wtext_count_tags(cm.text);
        cm.text_pos = 0;
        cm.timer = 0;
        cm.typing = 1;
        cm.step++;
        cm.result = 0;
        cm.glow = 0;
        cm.zoom = 0;
        break;
    case 1:
        if (++cm.timer >= 2) {
            if (++cm.text_pos >= cm.text_len) {
                cm.step++;
                cm.typing = 0;
                cm.flash_fade = 1;
                cm.flash = 0;
                cm_anim_set(4, 3);
                cm_anim_set(5, 1);
                se_play(0x6276);
            }
            cm.timer = 0;
        }
        break;
    case 2:
        for (i = 0; i < cm.n_cards; i++) {
            cm.flying[i].state = 3;
            cm.flying[i].t = -3 * i - 4;
        }
        cm.step++;
        break;
    case 3:
        for (i = 0; i < cm.n_cards; i++) {
            t = ++cm.flying[i].t;
            c = &cm.flying[i];
            if (t > 21) {
                c->state = 4;
                c->t = 0;
            } else if (t > 0) {
                r = 100 - t * 100 / 21;
                a = 3.1415927f * (float)(c->t * (20 - rand_next_alias() % 10) % 360) / 180.0f;
                c->x = r * cosf_vfpu(a);
                c->y = r * sinf_vfpu(a);
            }
        }
        if (c->state == 4) {
            cm_anim_set(4, 4);
            cm.step++;
        }
    case 4:
        cm.timer++;
        if (cm.timer == 45)
            cm_anim_set(6, 1);
        if (cm.timer > 336) {
            cm_anim_set(4, 5);
            cm_anim_set(6, 0);
            cm_anim_set(7, 1);
            cm.timer = 0;
            cm.step++;
        }
        break;
    case 5:
        if (++cm.timer >= cm_anim_length(7)) {
            cm.result = cm_pick_card(cm.n_cards, cm.n_cards, cm.cards);
            cm.timer = 0;
            cm.step++;
        }
        break;
    case 6:
        cm.timer++;
        cm.glow = cm.timer * (128 / cm_anim_length(7));
        cm.zoom = cm.timer * (64 / cm_anim_length(7)) + 64;
        if (cm.timer >= cm_anim_length(7)) {
            cm_anim_set(7, 0);
            cm.glow = 128;
            cm.timer = 0;
            cm.step++;
            bgm_stop(0x6276);
            se_play(0x6279);
        }
        break;
    case 7:
        cm.timer++;
        cm.glow = 128 - cm.timer * 5;
        cm.zoom = cm.timer * 2 + 128;
        if (cm.timer >= 24) {
            cm.flash_fade = 2;
            cm.flash = 0xFF;
            cm_anim_set(1, 1);
            cm_anim_set(2, 1);
            cm.glow = 0;
            cm.zoom = 0;
            cm.timer = 0;
            cm.step++;
        }
        break;
    case 8:
        if (++cm.timer >= 64) {
            cm_anim_set(5, 0);
            cm.timer = 0;
            cm.step++;
        }
        break;
    case 9:
        if (++cm.timer >= 32) {
            cm.out.shown = 1;
            cm.out.x = -0.3f;
            cm.out.y = 52.4f;
            cm.out.z = 9.9f;
            cm.timer = 0;
            cm.step++;
            se_play(0x627B);
        }
        break;
    case 10:
        cm.out.y += 0.6666667f;
        cm.out.z -= 0.03f;
        if (++cm.timer >= 24) {
            cm_anim_set(4, 0);
            cm.timer = 0;
            cm.step++;
        }
        break;
    case 11:
        cm.timer++;
        if (cm.timer >= 16) {
            cm_anim_set(8, 1);
            cm.timer = 0;
            cm.step++;
            se_play(0x627C);
        }
        break;
    case 12:
        cm.timer++;
        cm.out.x += 0.44583333f;
        cm.out.y += 4.6666665f;
        cm.out.z -= 0.26666668f;
        cm.out.rx += 3.75f;
        cm.out.rz += 7.5f;
        if (cm.timer >= 21)
            cm_anim_set(8, 0);
        if (cm.timer >= 24) {
            cm.text_pos = 0;
            cm_anim_set(1, 0);
            cm_anim_set(2, 0);
            cm.state = 5;
            cm.step = 0;
            cm.timer = 0;
            cm.name_x = 480;
        }
        break;
    }
    cm_anim_sounds();
}

/* state 5: the screen dims, the card's name slides in; △ opens the card detail, ○/× takes the card */
void cm_state_result(void)
{
    unsigned short *name;
    unsigned char size;
    unsigned int w;

    switch (cm.step) {
    case 0:
        cm.dim += 10;
        if (cm.dim >= 160) {
            name = card_get_name(cm.result);
            font_set_size(0x12, 0x12);
            size = font_measure_wtext(0x7800, name);
            font_set_size(size, 0x12);
            w = font_measure_text(name);
            cm.name_size = size;
            cm.name_w = (w >> 6) + 2;
            cm.dim = 160;
            cm.step++;
        }
        break;
    case 1:
        cm.timer++;
        cm.name_x = 480 - (480 - (480 - cm.name_w) / 2) * (cm.timer * cm.timer) / 256;
        if (cm.timer >= 16) {
            cm.name_x = (480 - cm.name_w) / 2;
            cm.name_shown = 1;
            cm.timer = 0;
            cm.step++;
        }
        break;
    case 2:
        if (pad_get()->b.trig & 0x1000) {
            fade_start_in(1, 0xFF000000, 16);
            cm.step = 10;
        }
        if (pad_get()->b.trig & 0x6000) {
            inv_adjust_count(cm.result, 1);
            cm.name_x = 480;
            cm.name_shown = 0;
            cm.step++;
            cm.timer = 0;
        }
        break;
    case 3:
        cm.out.x += 1.25f;
        cm.dim -= 10;
        if (cm.dim >= 160) {
            cm.dim = 0;
            memset(&cm.out, 0, sizeof(cm.out));
            wcscpy(cm.text, table_get_entry(cm_res_get(CM_STRINGS), 5));
            cm.text_len = wtext_count_tags(cm.text);
            cm.text_pos = 0;
            cm.timer = 0;
            cm.typing = 1;
            cm.step = 0;
            cm.state = 7;
        }
        break;
    case 10:
        if (fade_is_done()) {
            cm.step++;
            cm.detail = 1;
            cardinfo_open(cm.result, cm_res_get(0x800C), cm_res_get(0x8012));
            cardinfo_set_lists(cm_res_get(0x8015), cm_res_get(0x8036), cm_res_get(0x8037));
            cardinfo_has_alt_pict();
        }
        break;
    case 11:
        if (cardinfo_update()) {
            cm.detail = 0;
            cm.step++;
            fade_start(1, 0xFF000000, 16);
        }
        break;
    case 12:
        if (fade_is_done())
            cm.step = 2;
        break;
    }
}

/* states 6 and 7: "convert again?" (7, after a result) / (6) with the yes/no buttons swapped;
   yes → state 1, no → leave (state 8) */
void cm_state_ask(void)
{
    int go = 0;
    int quit = 0;

    switch (cm.step) {
    case 0:
        if (++cm.timer >= 2) {
            if (++cm.text_pos >= cm.text_len) {
                cm.step++;
                cm.typing = 0;
            }
            cm.timer = 0;
        }
        break;
    case 1:
        if (cm.state == 7) {
            if (pad_get()->b.trig & region_pick_value(1))
                go = 1;
            else if (pad_get()->b.trig & region_pick_value(0))
                quit = 1;
        } else if (cm.state == 6) {
            if (pad_get()->b.trig & region_pick_value(1))
                quit = 1;
            else if (pad_get()->b.trig & region_pick_value(0))
                go = 1;
        }
        if (quit) {
            cm.step = 0;
            cm.timer = 0;
            cm.state = 1;
        }
        if (go) {
            cm.step = 0;
            cm.timer = 0;
            cm.state = 8;
            fade_start_in(1, 0xFF000000, 16);
            bgm_cmd(600);
        }
        break;
    }
}

/* a random card of group `group` */
unsigned short cm_pick_card(int group, int n, unsigned short *cards)
{
    unsigned short *list = cm_groups[group - 10].cards;
    int count = cm_groups[group - 10].count;

    return list[rand_next_alias() % count];
}

/* the machine's meter (EHF 0x1013): 0x64 empty, 0x65 full, else the value */
void cm_meter_set(unsigned short v)
{
    void *e = cm_res_get(0x1013);
    unsigned short start;
    unsigned short end;

    if (e) {
        switch (v) {
        case 0x64:
            end = start = 0x65;
            break;
        case 0x65:
            start = 0x72;
            end = 0x94;
            break;
        default:
            start = end = v + 1;
            break;
        }
        ehf_set_anim(e, start, end + 1);
    }
}

/* starts animation slot `which` at `speed` (0 stops it, rewinding to frame 0) */
void cm_anim_set(unsigned short which, unsigned short speed)
{
    CmAnim *a = &cm_anims[which];
    void *e;

    if (speed == 0) {
        e = cm_res_get(a->id);
        if (e)
            ehf_set_frame_alias(e, 0);
    }
    a->speed = speed;
}

void cm_anim_get(unsigned short which, unsigned short *id, unsigned short *speed)
{
    CmAnim *a = &cm_anims[which];

    *id = a->id;
    *speed = a->speed;
}

/* frames slot `which` takes at its speed */
int cm_anim_length(unsigned short which)
{
    CmAnim *a = &cm_anims[which];
    void *e = cm_res_get(a->id);

    if (e == 0)
        return 0;
    if (a->speed != 0)
        return ehf_get_data_ptr(e) / a->speed;
    return 0;
}

/* sound effects when slots 1, 6 and 7 come back to their first frame */
void cm_anim_sounds(void)
{
    CmAnim *a;

    a = &cm_anims[1];
    if (a->speed != 0 && ehf_get_frame_count(cm_res_get(a->id)) == 0)
        se_play(0x627A);
    a = &cm_anims[6];
    if (a->speed != 0 && ehf_get_frame_count(cm_res_get(a->id)) == 0)
        se_play(0x6277);
    a = &cm_anims[7];
    if (a->speed != 0 && ehf_get_frame_count(cm_res_get(a->id)) == 0)
        se_play(0x6278);
}

/* cards that can come out, indexed by the number of cards fed in (10-99) */
static unsigned short cm_g10[] = { 0x0FA8, 0x0FB1, 0x0FB4, 0x10FD, 0x11D5, 0x18A6 };
static unsigned short cm_g11[] = { 0x0FC0, 0x0FC3, 0x0FC7, 0x18A7 };
static unsigned short cm_g12[] = { 0x0FD2, 0x0FD7, 0x10EB, 0x1A90 };
static unsigned short cm_g13[] = { 0x0FD9, 0x0FDB, 0x10E9, 0x1A92 };
static unsigned short cm_g14[] = { 0x0FE2, 0x0FEF, 0x10E8, 0x1621 };
static unsigned short cm_g15[] = { 0x0FF2, 0x0FF5, 0x10DA, 0x15A0 };
static unsigned short cm_g16[] = { 0x0FF7, 0x100A, 0x1010, 0x1014, 0x10D3 };
static unsigned short cm_g17[] = { 0x101C, 0x1025, 0x1028, 0x1297 };
static unsigned short cm_g18[] = { 0x1029, 0x1032, 0x1039, 0x129F };
static unsigned short cm_g19[] = { 0x103E, 0x1044, 0x1047, 0x1291 };
static unsigned short cm_g20[] = { 0x0FDE, 0x104B, 0x1054, 0x105B, 0x10A8 };
static unsigned short cm_g21[] = { 0x0FDF, 0x105E, 0x1065, 0x1069 };
static unsigned short cm_g22[] = { 0x1073, 0x1075, 0x1079, 0x11AA };
static unsigned short cm_g23[] = { 0x1080, 0x1083, 0x1088, 0x10A6 };
static unsigned short cm_g24[] = { 0x1089, 0x1092, 0x1094, 0x19B7 };
static unsigned short cm_g25[] = { 0x109A, 0x109D, 0x10A2, 0x1A06 };
static unsigned short cm_g26[] = { 0x10A3, 0x10A7, 0x10AA, 0x17D7 };
static unsigned short cm_g27[] = { 0x10AF, 0x10B4, 0x10BC, 0x1699 };
static unsigned short cm_g28[] = { 0x10C3, 0x10CB, 0x10F8, 0x15E1 };
static unsigned short cm_g29[] = { 0x10F9, 0x10FA, 0x10FB, 0x15E0 };
static unsigned short cm_g30[] = { 0x1018, 0x111D, 0x1120, 0x1124, 0x1459 };
static unsigned short cm_g31[] = { 0x113B, 0x1150, 0x1154, 0x13EF };
static unsigned short cm_g32[] = { 0x1156, 0x1157, 0x1158, 0x1294 };
static unsigned short cm_g33[] = { 0x115D, 0x1161, 0x1165, 0x12A0 };
static unsigned short cm_g34[] = { 0x1169, 0x116B, 0x116D, 0x127F };
static unsigned short cm_g35[] = { 0x1180, 0x1183, 0x118F, 0x127E, 0x1512 };
static unsigned short cm_g36[] = { 0x11A9, 0x11AC, 0x11AE, 0x127D };
static unsigned short cm_g37[] = { 0x11BD, 0x11DE, 0x11E3, 0x1716 };
static unsigned short cm_g38[] = { 0x1115, 0x11E9, 0x11F4, 0x1204 };
static unsigned short cm_g39[] = { 0x1205, 0x1206, 0x1212, 0x1715 };
static unsigned short cm_g40[] = { 0x1009, 0x1219, 0x1223, 0x1224, 0x1536 };
static unsigned short cm_g41[] = { 0x122F, 0x1265, 0x12B3, 0x14EA };
static unsigned short cm_g42[] = { 0x12E8, 0x12E9, 0x1311, 0x1400 };
static unsigned short cm_g43[] = { 0x1313, 0x1371, 0x1374, 0x13E6 };
static unsigned short cm_g44[] = { 0x1375, 0x1376, 0x137F, 0x14B2 };
static unsigned short cm_g45[] = { 0x111C, 0x1393, 0x139A, 0x13D0, 0x1513 };
static unsigned short cm_g46[] = { 0x13DE, 0x141D, 0x141F, 0x16FB };
static unsigned short cm_g47[] = { 0x1463, 0x1464, 0x1465, 0x166A };
static unsigned short cm_g48[] = { 0x146D, 0x1582, 0x1669, 0x17D8 };
static unsigned short cm_g49[] = { 0x1680, 0x16BB, 0x16BC, 0x19B4 };
static unsigned short cm_g50[] = { 0x0FEE, 0x16F5, 0x172E, 0x19B8 };
static unsigned short cm_g51[] = { 0x173B, 0x173C, 0x173D, 0x1A03 };
static unsigned short cm_g52[] = { 0x173E, 0x1750, 0x1751, 0x1A02 };
static unsigned short cm_g53[] = { 0x1783, 0x1786, 0x1787, 0x1A60 };
static unsigned short cm_g54[] = { 0x1792, 0x17F4, 0x17FC, 0x1A6C };
static unsigned short cm_g55[] = { 0x17FD, 0x181D, 0x181E, 0x18F7 };
static unsigned short cm_g56[] = { 0x181F, 0x1889, 0x188A, 0x19D3 };
static unsigned short cm_g57[] = { 0x1539, 0x159C, 0x16F7, 0x18EF };
static unsigned short cm_g58[] = { 0x14D2, 0x1A34, 0x1A41, 0x1AA6 };
static unsigned short cm_g59[] = { 0x161D, 0x19DF, 0x19E0, 0x1A0B };
static unsigned short cm_g60[] = { 0x12EB, 0x159D, 0x19DC, 0x19DD, 0x19DE };
static unsigned short cm_g61[] = { 0x1638, 0x19CF, 0x19D0, 0x19DB };
static unsigned short cm_g62[] = { 0x1696, 0x19B6, 0x19C1, 0x19C9 };
static unsigned short cm_g63[] = { 0x169C, 0x1977, 0x19A7, 0x19AC };
static unsigned short cm_g64[] = { 0x152F, 0x1925, 0x1939, 0x1976 };
static unsigned short cm_g65[] = { 0x151E, 0x1630, 0x18F8, 0x1922, 0x1924 };
static unsigned short cm_g66[] = { 0x1518, 0x187B, 0x18B3, 0x18F0 };
static unsigned short cm_g67[] = { 0x1315, 0x184D, 0x1863, 0x1865 };
static unsigned short cm_g68[] = { 0x1231, 0x17A7, 0x17ED, 0x17F3 };
static unsigned short cm_g69[] = { 0x0FBC, 0x1794, 0x17A0, 0x17A1 };
static unsigned short cm_g70[] = { 0x0FCC, 0x1718, 0x1719, 0x1793 };
static unsigned short cm_g71[] = { 0x0FE9, 0x16DC, 0x16FE, 0x1717 };
static unsigned short cm_g72[] = { 0x1122, 0x16A0, 0x16A1, 0x16A2 };
static unsigned short cm_g73[] = { 0x12E3, 0x1637, 0x1652, 0x1653 };
static unsigned short cm_g74[] = { 0x1524, 0x15F0, 0x15F2, 0x15F3 };
static unsigned short cm_g75[] = { 0x1542, 0x1578, 0x159F, 0x1662 };
static unsigned short cm_g76[] = { 0x14BC, 0x1515, 0x1516, 0x1617 };
static unsigned short cm_g77[] = { 0x14A7, 0x14B4, 0x14BB, 0x16EC };
static unsigned short cm_g78[] = { 0x1476, 0x1493, 0x1495, 0x1543 };
static unsigned short cm_g79[] = { 0x1450, 0x1451, 0x14B3, 0x1511 };
static unsigned short cm_g80[] = { 0x13E8, 0x16CE, 0x16D8, 0x188B };
static unsigned short cm_g81[] = { 0x1453, 0x179E, 0x179F, 0x1841 };
static unsigned short cm_g82[] = { 0x125C, 0x126B, 0x1293, 0x17A8 };
static unsigned short cm_g83[] = { 0x125A, 0x126C, 0x16D7, 0x18A8 };
static unsigned short cm_g84[] = { 0x124B, 0x1264, 0x12EA, 0x18A9 };
static unsigned short cm_g85[] = { 0x18BF, 0x18C0, 0x18C1, 0x18C2 };
static unsigned short cm_g86[] = { 0x1449, 0x1452, 0x1585, 0x1725 };
static unsigned short cm_g87[] = { 0x13F6, 0x142C, 0x1586, 0x1729 };
static unsigned short cm_g88[] = { 0x139E, 0x13F5, 0x1587, 0x1985 };
static unsigned short cm_g89[] = { 0x134E, 0x137E, 0x1588, 0x1986 };
static unsigned short cm_g90[] = { 0x130B, 0x134C, 0x134D, 0x1589 };
static unsigned short cm_g91[] = { 0x1221, 0x1307, 0x158A, 0x19AF };
static unsigned short cm_g92[] = { 0x11E2, 0x121A, 0x12CC, 0x158C };
static unsigned short cm_g93[] = { 0x1380, 0x158D, 0x19AB, 0x1A23 };
static unsigned short cm_g94[] = { 0x1927, 0x1928, 0x1929, 0x192A };
static unsigned short cm_g95[] = { 0x196B, 0x196C, 0x196D, 0x196E };
static unsigned short cm_g96[] = { 0x189E, 0x189F, 0x18A0, 0x18A3, 0x18A4 };
static unsigned short cm_g97[] = { 0x145C, 0x1631, 0x176A, 0x19CB };
static unsigned short cm_g98[] = { 0x13A4, 0x13A7, 0x145D, 0x1A9F };
static unsigned short cm_g99[] = { 0x18F6, 0x1A6D, 0x1A6E, 0x1A6F, 0x1A70, 0x1A74, 0x1A75 };
CmGroup cm_groups[90] = {
    { cm_g10, 6 },
    { cm_g11, 4 },
    { cm_g12, 4 },
    { cm_g13, 4 },
    { cm_g14, 4 },
    { cm_g15, 4 },
    { cm_g16, 5 },
    { cm_g17, 4 },
    { cm_g18, 4 },
    { cm_g19, 4 },
    { cm_g20, 5 },
    { cm_g21, 4 },
    { cm_g22, 4 },
    { cm_g23, 4 },
    { cm_g24, 4 },
    { cm_g25, 4 },
    { cm_g26, 4 },
    { cm_g27, 4 },
    { cm_g28, 4 },
    { cm_g29, 4 },
    { cm_g30, 5 },
    { cm_g31, 4 },
    { cm_g32, 4 },
    { cm_g33, 4 },
    { cm_g34, 4 },
    { cm_g35, 5 },
    { cm_g36, 4 },
    { cm_g37, 4 },
    { cm_g38, 4 },
    { cm_g39, 4 },
    { cm_g40, 5 },
    { cm_g41, 4 },
    { cm_g42, 4 },
    { cm_g43, 4 },
    { cm_g44, 4 },
    { cm_g45, 5 },
    { cm_g46, 4 },
    { cm_g47, 4 },
    { cm_g48, 4 },
    { cm_g49, 4 },
    { cm_g50, 4 },
    { cm_g51, 4 },
    { cm_g52, 4 },
    { cm_g53, 4 },
    { cm_g54, 4 },
    { cm_g55, 4 },
    { cm_g56, 4 },
    { cm_g57, 4 },
    { cm_g58, 4 },
    { cm_g59, 4 },
    { cm_g60, 5 },
    { cm_g61, 4 },
    { cm_g62, 4 },
    { cm_g63, 4 },
    { cm_g64, 4 },
    { cm_g65, 5 },
    { cm_g66, 4 },
    { cm_g67, 4 },
    { cm_g68, 4 },
    { cm_g69, 4 },
    { cm_g70, 4 },
    { cm_g71, 4 },
    { cm_g72, 4 },
    { cm_g73, 4 },
    { cm_g74, 4 },
    { cm_g75, 4 },
    { cm_g76, 4 },
    { cm_g77, 4 },
    { cm_g78, 4 },
    { cm_g79, 4 },
    { cm_g80, 4 },
    { cm_g81, 4 },
    { cm_g82, 4 },
    { cm_g83, 4 },
    { cm_g84, 4 },
    { cm_g85, 4 },
    { cm_g86, 4 },
    { cm_g87, 4 },
    { cm_g88, 4 },
    { cm_g89, 4 },
    { cm_g90, 4 },
    { cm_g91, 4 },
    { cm_g92, 4 },
    { cm_g93, 4 },
    { cm_g94, 4 },
    { cm_g95, 4 },
    { cm_g96, 5 },
    { cm_g97, 4 },
    { cm_g98, 4 },
    { cm_g99, 7 },
};

CmAnim cm_anims[9] = {
    { 0x1010, 0 }, { 0x1011, 0 }, { 0x1012, 0 }, { 0x1013, 1 }, { 0x1014, 0 },
    { 0x1015, 0 }, { 0x1016, 0 }, { 0x1017, 0 }, { 0x1018, 0 },
};
