// @range 0x08815B54-0x088165F8
/* The duel the story asks for: the GameState handed to the duel modules (rule, opponents and their
   decks and costumes, tag partner), filled piece by piece by the script; five ready bits say when it
   is complete. Also the lesson duels and the 8 words of lesson parameters. */
#include "libstory.h"

typedef struct StDuelSetup {           /* 0x08ACB36C, a GameState (0x740) with the header bytes named */
    RecipeData deck;                   /* +0x000 */
    unsigned short extra[3];           /* +0x108 */
    unsigned char pad10e[2];
    int arg110;                        /* +0x110 */
    int arg114;                        /* +0x114 */
    unsigned char kind;                /* +0x118 1 story, 2/3 lesson, 5 two named opponents */
    unsigned char rule;                /* +0x119 */
    unsigned char partner_mode;        /* +0x11A 2 tag duel */
    unsigned char lesson;              /* +0x11B */
    unsigned char x11c;                /* +0x11C */
    unsigned char x11d;                /* +0x11D */
    unsigned char x11e;                /* +0x11E */
    unsigned char x11f;                /* +0x11F */
    PlayerSlot slot[4];                /* +0x120 */
} StDuelSetup;

typedef struct StDuelHdr {             /* 0x08ACB364, 8 */
    int lesson;                        /* +0x0 lesson number of st_lesson_duel_start */
    unsigned char vs : 1;              /* +0x4 ready bits */
    unsigned char opp : 1;
    unsigned char partner : 1;
    unsigned char rule : 1;
    unsigned char params : 1;
    unsigned char pad : 3;
    unsigned char pad5[3];
} StDuelHdr;

extern int *st_duel_params;            /* 0x08ACB360 8 words of lesson parameters */
extern StDuelHdr st_duel_hdr;
extern StDuelSetup st_duel;

int st_id_to_chr(int id);
int st_id_to_rec(int id);
int st_id_to_item(int id);
int st_id_to_deck(int id);
int st_pl_x14_get(void);
int st_chr_deck(int chr);
void st_chr_note_deck(int chr);
int st_chr_costume(int chr);
void deck_slot_add_card(int i, unsigned int id);
unsigned short *chara_get_name(int i, int j);
unsigned short *recipe_get_name(int n);
void recipe_load(int n, RecipeData *out);
void gamestate_load(const void *src);

void st_costume_face(int *out, int costume);

void st_init_815b54(void)
{
    memset(&st_duel_hdr, 0, sizeof(StDuelHdr));
}

void st_term_815b68(void)
{
    memset(&st_duel_hdr, 0, sizeof(StDuelHdr));
}

void st_duel_set_params(int *p)
{
    st_duel_params = p;
}

void st_duel_clear_params(void)
{
    memset(st_duel_params, 0, 0x20);
}

void st_duel_clear(void)
{
    int *p = st_duel_params;

    memset(&st_duel, 0, sizeof(StDuelSetup));
    memset(&st_duel_hdr, 0, sizeof(StDuelHdr));
    memset(p, 0, 0x20);
}

/* story duel against one character: the player in slot 0 with the current deck */
void st_duel_set_vs(int id, int costume)
{
    StDuelHdr *h = &st_duel_hdr;
    int face = 0;
    int rec = st_id_to_rec(id);
    int chr = st_id_to_chr(id);
    int deck = st_chr_deck(chr);

    costume = costume ? costume : st_chr_costume(chr);
    st_costume_face(&face, costume);
    st_duel.kind = 1;
    st_duel.partner_mode = 1;
    st_duel.lesson = 0;
    st_duel.slot[0].type = 1;
    st_duel.slot[0].type2 = 0;
    st_duel.slot[0].deck_id = 0x162;
    st_duel.slot[1].type = rec;
    st_duel.slot[1].type2 = face;
    st_duel.slot[1].deck_id = deck;
    h->vs = 1;
    h->opp = 1;
    h->partner = 1;
}

/* tag duel: the player in slot 3 */
void st_duel_set_tag(int id, int costume)
{
    StDuelHdr *h = &st_duel_hdr;
    int face = 0;
    int rec = st_id_to_rec(id);
    int chr = st_id_to_chr(id);
    int deck = st_chr_deck(chr);

    costume = costume ? costume : st_chr_costume(chr);
    st_costume_face(&face, costume);
    st_duel.kind = 1;
    st_duel.partner_mode = 2;
    st_duel.lesson = 0;
    st_duel.slot[2].type = 1;
    st_duel.slot[2].type2 = 0;
    st_duel.slot[2].deck_id = 0x162;
    st_duel.slot[1].type = rec;
    st_duel.slot[1].type2 = face;
    st_duel.slot[1].deck_id = deck;
    h->vs = 1;
}

/* the second opponent of a tag duel (slot 3) */
void st_duel_set_opp2(int id, int costume)
{
    StDuelHdr *h = &st_duel_hdr;
    int face = 0;
    int rec = st_id_to_rec(id);
    int chr = st_id_to_chr(id);
    int deck = st_chr_deck(chr);

    costume = costume ? costume : st_chr_costume(chr);
    st_costume_face(&face, costume);
    st_duel.slot[3].type = rec;
    st_duel.slot[3].type2 = face;
    st_duel.slot[3].deck_id = deck;
    h->opp = 1;
}

/* the partner of a tag duel (slot 0); id 0 is the current partner of the player */
void st_duel_set_partner(int id, int costume)
{
    StDuelHdr *h = &st_duel_hdr;
    int face = 0;
    int rec;
    int chr;
    int deck;

    if (id == 0) {
        chr = st_pl_x14_get();
        rec = st_id_to_item(chr);
    } else {
        rec = st_id_to_rec(id);
        chr = st_id_to_chr(id);
    }
    deck = st_chr_deck(chr);
    costume = costume ? costume : st_chr_costume(chr);
    st_costume_face(&face, costume);
    st_duel.slot[0].type = rec;
    st_duel.slot[0].type2 = face;
    st_duel.slot[0].deck_id = deck;
    h->partner = 1;
}

/* duel between two characters (slot 0 against slot 1) */
void st_duel_set_watch(int id, int costume)
{
    StDuelHdr *h = &st_duel_hdr;
    int face = 0;
    int rec = st_id_to_rec(id);
    int chr = st_id_to_chr(id);
    int deck = st_chr_deck(chr);

    costume = costume ? costume : st_chr_costume(chr);
    st_costume_face(&face, costume);
    st_duel.kind = 5;
    st_duel.partner_mode = 1;
    st_duel.lesson = 0;
    st_duel.slot[0].type = rec;
    st_duel.slot[0].type2 = face;
    st_duel.slot[0].deck_id = deck;
    h->vs = 1;
}

void st_duel_set_opp(int id, int costume)
{
    StDuelHdr *h = &st_duel_hdr;
    int face = 0;
    int rec = st_id_to_rec(id);
    int chr = st_id_to_chr(id);
    int deck = st_chr_deck(chr);

    costume = costume ? costume : st_chr_costume(chr);
    st_costume_face(&face, costume);
    st_duel.slot[1].type = rec;
    st_duel.slot[1].type2 = face;
    st_duel.slot[1].deck_id = deck;
    h->opp = 1;
}

void st_duel_set_args(int a, int b)
{
    st_duel.arg110 = a;
    st_duel.arg114 = (unsigned char)b;
    st_duel_hdr.partner = 1;
}

void st_duel_set_rule(int mode, unsigned char x, int rule)
{
    StDuelHdr *h = &st_duel_hdr;
    int v;
    unsigned char r;

    switch (mode) {
    case 2:
        v = 0;
        break;
    case 1:
    default:
        v = 1;
        break;
    }
    switch (rule) {
    case 1:
        r = 1;
        break;
    case 2:
    default:
        r = 2;
        break;
    }
    st_duel.x11d = v;
    st_duel.x11c = x;
    st_duel.rule = r;
    h->rule = 1;
}

void st_duel_set_params_a(int a, int b, int c)
{
    int *p = st_duel_params;

    p[2] = a;
    p[3] = b;
    p[4] = c;
    st_duel_hdr.params = 1;
}

void st_duel_set_params_b(int a, int b, int c)
{
    int *p = st_duel_params;

    p[5] = a;
    p[6] = b;
    p[7] = c;
}

void st_lesson_set_a(int v)
{
    st_duel_params[0] = v;
}

void st_lesson_set_b(int v)
{
    st_duel_hdr.lesson = v;
}

void st_duel_set_x11e(unsigned char a, unsigned char b)
{
    st_duel.x11e = a;
    st_duel.x11f = b;
}

int st_duel_param0(void) { return st_duel_params[0]; }
int st_duel_param1(void) { return st_duel_params[1]; }
int st_duel_param2(void) { return st_duel_params[2]; }
int st_duel_param3(void) { return st_duel_params[3]; }
int st_duel_param4(void) { return st_duel_params[4]; }
int st_duel_param5(void) { return st_duel_params[5]; }
int st_duel_param6(void) { return st_duel_params[6]; }
int st_duel_param7(void) { return st_duel_params[7]; }

/* all five pieces of the duel are set */
int st_duel_pending(void)
{
    StDuelHdr *h = &st_duel_hdr;

    if (!h->vs)
        return 0;
    if (!h->opp)
        return 0;
    if (!h->partner)
        return 0;
    if (!h->rule)
        return 0;
    if (!h->params)
        return 0;
    return 1;
}

/* completes the names and decks of the slots, records the decks seen and hands the setup to BOOT */
void st_duel_prepare(void)
{
    int i;
    int all;

    if (!st_duel_pending())
        return;
    switch (st_duel.partner_mode) {
    case 1:
    default:
        switch (st_duel.kind) {
        case 1:
        default:
            all = 1;
            break;
        case 5:
            for (i = 0; i < 2; i++) {
                memcpy(st_duel.slot[i].name, chara_get_name(0, st_duel.slot[i].type), 0x30);
                memcpy(st_duel.slot[i].deck_name, recipe_get_name(st_duel.slot[i].deck_id), 0x40);
                recipe_load(st_duel.slot[i].deck_id, &st_duel.slot[i].recipe);
            }
            all = 0;
            break;
        }
        for (i = 0; i < 2; i++) {
            if (all && i == 0)
                continue;
            st_chr_note_deck(st_id_to_deck(st_duel.slot[i].type));
            deck_slot_add_card(st_duel.slot[i].type, st_duel.slot[i].deck_id);
        }
        break;
    case 2:
        for (i = 0; i < 4; i++) {
            if (i == 2)
                continue;
            st_chr_note_deck(st_id_to_deck(st_duel.slot[i].type));
            deck_slot_add_card(st_duel.slot[i].type, st_duel.slot[i].deck_id);
        }
        break;
    }
    gamestate_load(&st_duel);
}

void st_lesson_duel_start(void)
{
    StDuelSetup gs;

    memset(&gs, 0, sizeof(gs));
    gs.kind = 2;
    gs.lesson = st_duel_hdr.lesson;
    gamestate_load(&gs);
}

void st_lesson_duel_start2(void)
{
    StDuelSetup gs;

    memset(&gs, 0, sizeof(gs));
    gs.kind = 3;
    gs.lesson = st_duel_hdr.lesson;
    gamestate_load(&gs);
}

/* face variant of a costume */
void st_costume_face(int *out, int costume)
{
    switch (costume) {
    case 5:
    case 0x12:
    case 0x1F:
        *out = 1;
        break;
    case 0x3B:
    case 0x15:
        *out = 2;
        break;
    default:
        *out = 0;
        break;
    }
}
