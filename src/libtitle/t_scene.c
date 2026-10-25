// @range 0x08809DC4-0x0880A41C
// @asm 0x08809E24-0x0880A074
// @data 0x0882E414-0x0882E44C
/* Leaving the title for the duel scene: the GameState setups of the random duel, the lessons, the
   free duel (players and decks chosen in the setup screen) and the deck tutorials. */
/* the callers see `arg` as unsigned short (they mask it); the definition takes an int */
#define title_scene_push title_scene_push_proto
#include "libtitle.h"
#undef title_scene_push

unsigned short title_scene_push(int scene, int arg);

#define SLOT_RECIPE(s) ((RecipeData *)((int)(s) + 0x80))

unsigned short title_scene_push(int scene, int arg)
{
    sys_state_set_callbacks(0, 0, 0);
    gfx_overlay_enable(1);
    sys_frame_wait();
    scene_push(13, scene, arg);
    return scene;
}

/* templates of the random duel's local tables (the function stays in asm) */
int title_rand_chars[7] = { 2, 3, 4, 5, 6, 7, 8 };
int title_rand_decks[7] = { 1, 7, 13, 19, 25, 31, 37 };

#ifdef NON_MATCHING /* os ponteiros dos dois vetores trocam de $s nos laços */
/* random duel: two random characters with their decks */
void title_random_duel_start(void)
{
    TGameHdr *gs = (TGameHdr *)game_state_get();
    int i;
    int j;
    int c;
    int d;

    gs->kind = 5;
    gs->kind2 = 1;
    gs->partner_mode = 1;
    gs->lp_level = 0;
    gs->time_level = 0;
    gs->mode = rand_next();
    gs->side = (unsigned char)((int)rand_next() % 2);
    {
    int chars[7];
    int decks[7];

    memcpy(chars, title_rand_chars, sizeof(chars));
    memcpy(decks, title_rand_decks, sizeof(decks));

    if ((int)rand_next() % 2)
        gs->bgm_set = 1;
    else
        gs->bgm_set = 3;
    for (i = 0; i < 7; i++) {
        c = chars[i];
        d = decks[i];
        j = (int)rand_next() % 6;
        if (i <= j)
            j++;
        if (j >= 7)
            j = 6;
        decks[i] = decks[j];
        chars[i] = chars[j];
        chars[j] = c;
        decks[j] = d;
    }
    for (i = 0; i < 2; i++) {
        PlayerSlot *p;

        gs->slot[i].type = chars[i];
        gs->slot[i].type2 = 0;
        p = &gs->slot[i];
        if (p->type == 6)
            p->type2 = 1;
        p->deck_id = decks[i];
    }
    }
    gamestate_load(gs);
    title_scene_push(6, 0);
}
#endif

/* lesson duel (the lesson id is title.step) */
void title_lesson_start(void)
{
    TGameHdr *gs = (TGameHdr *)game_state_get();

    gs->kind = 3;
    gs->tutorial = title.step;
    gs->bgm_set = 3;
    title_scene_push(6, 0);
}

int title_lp_level(int x)
{
    int r = 0;

    switch (x) {
    case 0:
        r = 1;
        break;
    case 1:
        r = 2;
        break;
    case 2:
        break;
    case 3:
        r = 3;
        break;
    case 4:
        r = 4;
        break;
    }
    return r;
}

/* free duel: players, decks and options from the setup screen */
void title_free_duel_start(void)
{
    TDuelOpts *const o = &title.duel.opts;
    unsigned char *q = (unsigned char *)&title.duel + 0x1180;
    TGameHdr *gs = (TGameHdr *)game_state_get();
    TSlotInfo info __attribute__((aligned(16)));
    int last;

    int i;
    int n;
    PlayerSlot *p;

    gs->slot[0].type = 0;
    gs->slot[1].type = 0;
    gs->slot[2].type = 0;
    gs->slot[3].type = 0;
    n = 2;
    gs->rematch = 2;
    gs->kind = 1;
    last = -1;
    if (o->tag) {
        gs->partner_mode = 2;
        n = 4;
    } else {
        gs->partner_mode = 1;
    }
    if (o->swap)
        gs->kind2 = 2;
    else
        gs->kind2 = 1;
    gs->lp_level = title_lp_level(o->lp);
    gs->time_level = o->time;
    gs->bgm_set = 1;
    gs->mode = rand_next();
    gs->side = (unsigned char)((int)rand_next() % 2);
    for (i = 0; i < n; i++, q += sizeof(TDeckRec)) {
        tpt_get_ent(&info, i);
        gs->slot[i].type = info.type;
        gs->slot[i].type2 = info.type2;
        p = &gs->slot[i];
        if (p->type == 1 || p->type == 41) {
            switch (info.deck_kind) {
            case 0:
                if (info.deck == 0) {
                    p->deck_id = 0x162;
                    p->rec_index = 0;
                } else {
                    p->deck_id = 0x160;
                    p->rec_index = info.deck - 1;
                }
                break;
            case 1:
                p->deck_id = info.deck;
                p->rec_index = 0;
                break;
            case 2:
                p->deck_id = 0x15F;
                p->rec_index = 0;
                p->voice_obj = 0;
                memcpy(p->deck_name, ((TDeckRec *)(q + 0xFF0))->name, sizeof(p->deck_name));
                memcpy(&p->recipe, &((TDeckRec *)(q + 0xFF0))->recipe, sizeof(RecipeData));
                break;
            case 3:
                break;
            }
        } else {
            p->deck_id = info.deck;
            p->rec_index = 0;
        }
        if (p->type == 1)
            last = i;
    }
    gamestate_load(gs);
    if (last >= 0) {
        PlayerSlot *ls = &gs->slot[last];
        int side = 0;

        if (gs->kind2 == 2)
            side = 1;
        gamestate_fill_partner_list(SLOT_RECIPE(ls), side);
    }
    tpartner_free();
    title_scene_push(6, 0);
}

/* deck tutorial duel (title.arg = 7 + tutorial id) */
void title_dtut_duel_start(void)
{
    TGameHdr *gs = (TGameHdr *)game_state_get();

    gs->tutorial = title.arg - 7;
    gs->rematch = 0;
    gs->kind = 2;
    gs->partner_mode = 1;
    gs->kind2 = 1;
    gs->lp_level = 0;
    gs->time_level = 0;
    gs->bgm_set = 1;
    gs->mode = rand_next();
    gs->side = (unsigned char)((int)rand_next() % 2);
    title_scene_push(6, 0);
}
