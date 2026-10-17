// @range 0x08840478-0x08840DC4
// @asm 0x08840A78-0x08840D08
/* Save-state flag records, options bitfields and the session game state (player slots filled from
   recipes / saved decks). Continues the game.c unit. */
#include "psp.h"
#include "libc.h"
#include "ehsys.h"
#include "game.h"

unsigned short *chara_get_name(int i);
void wtext_strip_tags(unsigned short *dst, const unsigned short *src);
unsigned short *recipe_get_name(int n);
void recipe_load(int n, RecipeData *out);

void sysflag_rec_set_byte0(int id, unsigned char v)
{
    FlagRec *r = flagrec_get(id);

    if (r)
        r->byte0 = v;
}

unsigned char sysflag_rec_get_byte0(int id)
{
    FlagRec *r = flagrec_get(id);

    if (r)
        return r->byte0;
    return 0;
}

void sysflag_set2(int id, int v)
{
    FlagRec *r = flagrec_get(id);

    if (r)
        r->val = v & 3;
}

int sysflag_get2(int id)
{
    FlagRec *r = flagrec_get(id);

    if (r)
        return r->val;
    return 0;
}

unsigned short *sysflag_rec_get_data(int id)
{
    FlagRec *r = flagrec_get(id);

    if (r)
        return r->data;
    return 0;
}

void sysflag_rec_set_u16(int id, int i, unsigned short v)
{
    FlagRec *r = flagrec_get(id);

    if (r && i < 0x14)
        r->data[i] = v;
}

unsigned short sysflag_rec_get_u16(int id, int i)
{
    FlagRec *r = flagrec_get(id);

    if (r == 0)
        return 0;
    if (i >= 0x14)
        return 0;
    return r->data[i];
}

void sysflag_recs_init(void)
{
    int i;

    for (i = 0; i < 0x30; i++) {
        sysflag_rec_set_byte0(i, 0xAA);
        memset(sysflag_rec_get_data(i), 0, 0x28);
    }
}

SysConfig *cfg_get(void) /* also called from asm (cfg_get_language, ...) */
{
    return &sysstate_get()->cfg;
}

unsigned char cfg_get_byte0_hi2(void)
{
    return cfg_get()->hi2;
}

void cfg_set_byte0_hi2(int v)
{
    SysConfig *c = cfg_get();
    c->hi2 = v & 3;
}

unsigned char cfg_get_bits_1_0_2(void)
{
    return cfg_get()->b1_0_2;
}

void cfg_set_bits_1_0_2(int v)
{
    SysConfig *c = cfg_get();
    c->b1_0_2 = v & 7;
}

unsigned int cfg_get_byte0_bits4_5(void)
{
    return cfg_get()->bits4_5;
}

void cfg_set_byte0_bits4_5(int v)
{
    SysConfig *c = cfg_get();
    c->bits4_5 = v & 3;
}

void cfg_set_byte3(int i, int v)
{
    unsigned char *c = (unsigned char *)cfg_get();
    c[i + 3] = v & 0xF;
}

unsigned char cfg_get_byte3_b(int i)
{
    unsigned char *c = (unsigned char *)cfg_get();
    return c[i + 3];
}

void cfg_set_byte0_bit3(int on)
{
    SysConfig *c = cfg_get();

    if (on)
        c->bit3 = 1;
    else
        c->bit3 = 0;
}

int cfg_is_mode1(void)
{
    return cfg_get()->bit3 == 1;
}

void cfg_set_flag(int which, int on)
{
    SysConfig *c = cfg_get();
    int b = on != 0;

    switch (which) {
    case 0:
        c->flag0 = b;
        break;
    case 1:
        c->flag1 = b;
        break;
    case 2:
        c->flag2 = b;
        break;
    case 3:
        c->flag3 = b;
        break;
    }
}

unsigned int cfg_get_flag(int which)
{
    SysConfig *c = cfg_get();

    switch (which) {
    case 0:
        return c->flag0;
    case 1:
        return c->flag1;
    case 2:
        return c->flag2;
    case 3:
        return c->flag3;
    }
    return 0;
}

void cfg_set_bits_1_3_4(int v)
{
    SysConfig *c = cfg_get();
    c->b1_3_4 = v & 3;
}

unsigned int cfg_get_bits_1_3_4(void)
{
    return cfg_get()->b1_3_4;
}

GameState *game_state_get(void)
{
    return &g_game.game_state;
}

#ifdef NON_MATCHING
/* Loads a 0x740 game-state image and resolves each player slot's name and deck. Equivalent to
   0x08840A78 except one temporary register (type in $a1); kept in asm via @asm. */
void gamestate_load(const void *src)
{
    SysSave *s = sysstate_get();
    GameState *gs = game_state_get();
    PlayerSlot *slot;
    SaveRec *rec;
    unsigned short *name;
    int type, deck;
    int i;

    memcpy(gs, src, sizeof *gs);
    for (i = 0; i < 4; i++) {
        slot = &gs->slot[i];
        type = slot->type;
        switch (type) {
        case 0:
            break;
        case 1:
            memcpy(slot->name, sysstate_get_name(), sizeof slot->name);
            break;
        default:
            if (type < 0x98)
                wtext_strip_tags(slot->name, chara_get_name(0));
            break;
        }
        deck = slot->deck_id;
        if (deck < 0x163) {
            switch (deck) {
            case 0x15F:
                break;
            case 0x160:
                rec = sysstate_rec_get(slot->rec_index);
                if (rec->used != 0) {
                    memcpy(slot->deck_name, rec->name, sizeof slot->deck_name);
                    memcpy(&slot->recipe, &rec->recipe, sizeof slot->recipe);
                } else {
                    memset(slot->deck_name, 0, sizeof slot->deck_name);
                    memset(&slot->recipe, 0, sizeof slot->recipe);
                }
                break;
            case 0x161:
                memset(slot->deck_name, 0, sizeof slot->deck_name);
                memset(&slot->recipe, 0, sizeof slot->recipe);
                break;
            case 0x162:
                deck_name_copy_out(slot->deck_name);
                deck_copy_out(&slot->recipe);
                break;
            default:
                name = recipe_get_name(deck);
                if (name != 0) {
                    memcpy(slot->deck_name, name, sizeof slot->deck_name);
                    recipe_load(slot->deck_id, &slot->recipe);
                } else {
                    memset(slot->deck_name, 0, sizeof slot->deck_name);
                    memset(&slot->recipe, 0, sizeof slot->recipe);
                }
                break;
            }
            if (slot->type == 1) {
                if (slot->deck_id == 0x162) {
                    deck_copy_out(&gs->deck);
                    memcpy(gs->extra, s->partner_state, sizeof gs->extra);
                } else {
                    memcpy(&gs->deck, &slot->recipe, sizeof gs->deck);
                    gs->extra[0] = 4;
                    gs->extra[1] = 4;
                    gs->extra[2] = 4;
                }
            }
        } else {
            memset(slot->deck_name, 0, sizeof slot->deck_name);
            memset(&slot->recipe, 0, sizeof slot->recipe);
        }
    }
}
#endif

unsigned char *deck_cur_get(void)
{
    return sysstate_get()->deck_cur;
}

unsigned char *deck_slots_get(void)
{
    return sysstate_get()->deck_slots;
}

unsigned char *deck_names_get(void)
{
    return sysstate_get()->deck_names;
}

unsigned char *game_state2_get(void)
{
    return g_game.game_state2;
}

unsigned char *sysstate_get_ptr_97f0(void)
{
    return sysstate_get()->unk97f0;
}

unsigned int sysstate_get_u32_97e8(void)
{
    return sysstate_get()->unk97e8;
}
