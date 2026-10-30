// @range 0x0884B094-0x0884C718
// @asm 0x0884C140-0x0884C588
// @data 0x089124E8-0x089125F0
/* Start of a duel: state reset, normal start (LP 8000, decks shuffled), and the tutorial/puzzle start
   from a text file of settings (NN_field_set.txt: LP, phase, cards of each zone, attached effects). */
#include "duel.h"
#include "duel_setup.h"

typedef struct DuelPick {              /* 0x0891524C (see duel_pick.c) */
    int p;
    int kind;                          /* +0x004 */
    int cur;                           /* +0x008 */
    unsigned int n;                    /* +0x00C also the number of effects read by the setup text */
    DuelCard c[512];                   /* +0x010 */
    unsigned short where[512];         /* +0x810 */
} DuelPick;

typedef struct DuelEffectLoc {         /* view of DuelEffect: source and target as player/zone bytes */
    unsigned short src;
    unsigned char kind : 4;
    unsigned char kind_hi : 4;
    unsigned char pad3;
    unsigned short dst_player : 8;     /* +0x04 */
    unsigned short dst_zone : 8;
    unsigned short next;
} DuelEffectLoc;

extern DuelPick duel_w_0891524C;
extern int duel_player_type[2];
extern unsigned int duel_view_player;
extern int duel_w_089195EC;
extern int duel_phase;
extern unsigned int duel_turn_player;
extern unsigned int duel_turn;
extern unsigned int duel_089195C8;
extern int duel_w_089195D8;
extern int duel_w_089195E8;            /* 2 normal, 3 tutorial, 4 puzzle */
extern int duel_w_089195FC;
extern int duel_w_08919600;
extern int duel_w_0891960C;
extern int duel_w_08913C14;
extern int duel_w_08913C18;
extern char duel_req[0x81C];
extern char duel_stats[0xD8];
extern char duel_w_08913CF4[0x21C];
extern char duel_w_08914040[0x310];

void duel_bzero(void *p, int n);
void duel_deck_load(unsigned int p, RecipeData *d);
void duel_d_shuffle(unsigned int p);
int duel_host_call(int a, int b, int c, int d);
void duel_0884D618(int a);
void duel_cmd(unsigned short cmd, unsigned short a, unsigned short b, unsigned short c);
void duel_deck_build_uids(unsigned int p, int a);
int memcmp(const void *a, const void *b, int n);
int sscanf(const char *s, const char *fmt, ...);
void duel_088BFF78(unsigned int p, int id, int a);
unsigned int duel_0884C718(void);
int duel_08809B50(unsigned int p, int id, int a);
int duel_08809BE8(unsigned int p, int id, int a, int n);
int duel_08809CA8(unsigned int p, int id, int a, int n);
int duel_08809DA8(unsigned int p, int id);
void duel_setup_load_text(char *text);

/* duel_player_type seen as members of duel_setup (+0x08) */
#define SETUP_TYPE ((int *)&duel_setup.unk8)

/* clears the whole duel state */
void duel_0884B094(void)
{
    duel_bzero(&duel, sizeof(Duel));
    duel_bzero(&duel_w_0891524C, 0xC10);
    duel_bzero(duel_req, 0x81C);
    duel_bzero(&duel_summon, 0x1C);
    duel_bzero(&duel_chain, 0x6C4);
    duel_bzero(&duel_special, 0x130);
    duel_bzero(duel_stats, 0xD8);
    duel_bzero(duel_w_08913CF4, 0x21C);
    duel_bzero(duel_w_08914040, 0x310);
    if (duel_player_type[0] == 2 || duel_player_type[1] == 2) {
        duel_w_089195EC = 1;
    }
    duel_phase = 7;
    duel_w_08913C14 = 0;
    duel_w_08913C18 = 0;
}

/* normal start: LP, decks, shuffle, LP and pile counts to the host, first player */
int duel_0884B184(void)
{
    duel_0884B094();
    duel.players[0].lp = 8000;
    duel.players[1].lp = 8000;
    duel_deck_load(0, &duel_setup.deck[0]);
    duel_deck_load(1, &duel_setup.deck[1]);
    duel_089195C8 = duel_setup.mode;
    duel_d_shuffle(duel_setup.unk10);
    duel_d_shuffle(1 - duel_setup.unk10);
    duel_host_call(0xD, 0, duel.players[0].lp, 0);
    duel_host_call(0xD, 1, duel.players[1].lp, 0);
    duel_host_call(0x14, 0, 0xD, 0);
    duel_host_call(0x14, 1, 0xD, 0);
    duel_host_call(0x14, 0, 0xC, 0);
    duel_host_call(0x14, 1, 0xC, 0);
    duel_turn_player = duel_setup.unk10;
    duel_w_089195D8 = 0xFFFF;
    if (duel_player_type[0] == 3) {
        duel_0884D618(0);
    } else if (duel_w_089195EC == 0 || duel_player_type[duel_setup.unk10] != 2) {
        duel_cmd(1, 0, 0, 0);
        duel_cmd((duel_turn_player ? 0x8000 : 0) | 0x55, 0, 5, 0);
        duel_cmd((1 - duel_turn_player ? 0x8000 : 0) | 0x55, 0, 5, 0);
    }
    return 1;
}

int duel_0884B368(int a)
{
    duel_0884B184();
    duel_deck_build_uids(duel_setup.unk10, a);
    duel_deck_build_uids(1 - duel_setup.unk10, a);
    duel_w_089195E8 = 2;
    return 1;
}

/* tutorial: the local player 0 against the computer, from turn 9 */
int duel_start_tutorial(char *text)
{
    duel_0884B094();
    duel_w_089195E8 = 3;
    duel_setup.mode = 0;
    duel_setup.view_player = 0;
    SETUP_TYPE[duel_setup.view_player] = 0;
    SETUP_TYPE[duel_setup.view_player ^ 1] = 1;
    duel_setup.unk10 = duel_setup.view_player;
    duel_turn = 9;
    duel_w_089195D8 = 0xFFFF;
    duel_089195C8 = duel_setup.mode;
    duel_setup_load_text(text);
    return 1;
}

/* puzzle duel: as the tutorial, from turn 1 */
int duel_start_puzzle(char *text)
{
    duel_0884B094();
    duel_w_089195E8 = 4;
    duel_setup.mode = 0;
    duel_setup.view_player = 0;
    SETUP_TYPE[duel_setup.view_player] = 0;
    SETUP_TYPE[duel_setup.view_player ^ 1] = 1;
    duel_setup.unk10 = duel_setup.view_player;
    duel_turn = 1;
    duel_w_089195D8 = 0xFFFF;
    duel_089195C8 = duel_setup.mode;
    duel_setup_load_text(text);
    return 1;
}

int duel_0884B530(void)
{
    return 1;
}

/* copies the next line of *pos (with its '\n') into buf; 0 at the end or if the line is too long */
int duel_read_line(char *buf, int size, char **pos)
{
    char *s = *pos;
    int n = 0;

    if (*s != 0) {
        do {
            buf[n] = *s;
            n++;
            s++;
            if (n >= size - 1) {
                return 0;
            }
        } while (s[-1] != '\n' && *s != 0);
    }
    buf[n] = 0;
    *pos = s;
    return n;
}

/* a new card id of player p (next instance id) */
static DuelCard duel_0884B59C(unsigned int p, int id)
{
    DuelCard c = { 0 };
    int n;

    n = duel.players[p & 1].unk4;
    c.owner = p;
    c.id = id;
    c.uid_hi = n;
    duel.players[p & 1].unk4 = n + 1;
    duel.list[CARD_UID(c)].id = id;
    return c;
}

/* one line of the setup text; 1 at "[END]" */
int duel_setup_parse_line(char *line)
{
    int lp;
    int h;
    int d;
    int f;
    int e;
    int g;
    int z;
    int id;
    int face;
    int pos;
    int param;
    int k;
    int sp;
    int sl;
    int sid;
    int dp;
    int dl;
    unsigned int p;
    DuelSlot *s;
    DuelEffectBits *ef;

    switch (line[0]) {
    case '[':
        if (memcmp(line, "[END]", 5) == 0) {
            return 1;
        }
        break;
    case 'T':
        if (memcmp(line, "TurnPlayer", 10) == 0) {
            duel_setup.unk10 = line[11] - '0';
        }
        break;
    case 'P':
        if (memcmp(line, "PlayerLP", 8) == 0) {
            p = line[8] - '0';
            sscanf(line + 10, "%d", &lp);
            duel.players[p & 1].lp = lp;
        } else if (memcmp(line, "Phase", 5) == 0) {
            switch (line[6]) {
            case 'D':
                duel_phase = 0;
                duel_w_089195FC = 1;
                duel_w_08919600 = 1;
                break;
            case 'S':
                duel_phase = 1;
                duel_w_089195FC = 2;
                duel_w_08919600 = 1;
                break;
            case 'M':
                duel_phase = 2;
                duel_w_089195FC = 3;
                duel_w_08919600 = 1;
                break;
            case 'B':
                duel_phase = 3;
                duel_w_089195FC = 3;
                duel_w_08919600 = 0x16;
                duel_w_0891960C = 1;
                break;
            case 'E':
                duel_phase = 5;
                duel_w_089195FC = 4;
                duel_w_08919600 = 1;
                break;
            }
        }
        break;
    case 'C':
        if (memcmp(line, "CardIn", 6) != 0) {
            break;
        }
        switch (line[6]) {
        case 'H': /* CardInHandN=id: on top of the deck, drawn at the start */
            p = line[10] - '0';
            if (line[11] == '=') {
                break;
            }
            sscanf(line + 16, "%d", &h);
            duel.players[p & 1].zone_d[duel.players[p & 1].n_d] = duel_0884B59C(p, h);
            duel.players[p & 1].n_d++;
            duel.players[p & 1].n_hand++;
            break;
        case 'D': /* CardInDeckN=id */
            p = line[10] - '0';
            if (line[11] == '=') {
                break;
            }
            sscanf(line + 16, "%d", &d);
            duel.players[p & 1].zone_d[duel.players[p & 1].n_d] = duel_0884B59C(p, d);
            duel.players[p & 1].n_d++;
            break;
        case 'F': /* CardInFusionN=id */
            p = line[12] - '0';
            if (line[13] == '=') {
                break;
            }
            sscanf(line + 18, "%d", &f);
            duel.players[p & 1].zone_c[duel.players[p & 1].n_18] = duel_0884B59C(p, f);
            duel.players[p & 1].n_18++;
            break;
        case 'E': /* CardInExcludeN=id */
            p = line[13] - '0';
            if (line[14] == '=') {
                break;
            }
            sscanf(line + 19, "%d", &e);
            duel.players[p & 1].zone_f[duel.players[p & 1].n_f] = duel_0884B59C(p, e);
            duel.players[p & 1].n_f++;
            break;
        case 'G':
            if (line[7] == 'r') { /* CardInGraveN=id */
                p = line[11] - '0';
                if (line[12] == '=') {
                    break;
                }
                sscanf(line + 17, "%d", &g);
                duel.players[p & 1].grave[duel.players[p & 1].n_grave] = duel_0884B59C(p, g);
                duel.players[p & 1].n_grave++;
                break;
            }
            /* CardInGroundN_Z=id, _Face=, _Turn=, _Param= */
            p = line[10] - '0';
            sscanf(line + 12, "%d", &z);
            if (memcmp(line + 15, "=", 1) == 0) {
                s = &duel.players[p & 1].field[z];
                sscanf(line + 16, "%d", &id);
                s->card.id = id;
            } else if (memcmp(line + 15, "_Face", 5) == 0) {
                s = &duel.players[p & 1].field[z];
                sscanf(line + 21, "%d", &face);
                s->faceup = face;
            } else if (memcmp(line + 15, "_Turn", 5) == 0) {
                s = &duel.players[p & 1].field[z];
                sscanf(line + 21, "%d", &pos);
                s->pos = pos;
            } else if (memcmp(line + 15, "_Param", 6) == 0) {
                s = &duel.players[p & 1].field[z];
                sscanf(line + 22, "%d", &param);
                s->link = param;
            }
            break;
        }
        break;
    case 'E': /* EquipActK_Field=value: effect entry K + 1 */
        if (memcmp(line, "EquipAct", 8) != 0) {
            break;
        }
        if (line[8] == '=') {
            break;
        }
        sscanf(line + 8, "%d", &k);
        ef = (DuelEffectBits *)&duel.effects[k + 1];
        duel_w_0891524C.n = (unsigned int)(k + 1) < duel_w_0891524C.n ? duel_w_0891524C.n : k + 1;
        if (memcmp(line + 12, "SrcPlayer", 9) == 0) {
            sscanf(line + 22, "%d", &sp);
            ef->src = (unsigned char)sp | (ef->src >> 8 & 0xFF) << 8;
        } else if (memcmp(line + 12, "SrcLocate", 9) == 0) {
            sscanf(line + 22, "%d", &sl);
            ef->src = (ef->src & 0xFF) | (unsigned char)sl << 8;
        } else if (memcmp(line + 12, "SrcCardID", 9) == 0) {
            sscanf(line + 22, "%d", &sid);
            ef->src = sid;
        } else if (memcmp(line + 12, "DstPlayer", 9) == 0) {
            sscanf(line + 22, "%d", &dp);
            ef->arg = (unsigned char)dp | (ef->arg >> 8 & 0xFF) << 8;
        } else if (memcmp(line + 12, "DstLocate", 9) == 0) {
            sscanf(line + 22, "%d", &dl);
            ef->arg = (ef->arg & 0xFF) | (unsigned char)dl << 8;
        } else if (memcmp(line + 12, "Type", 4) == 0) {
            if (memcmp(line + 17, "ONLYINTURN", 10) == 0) {
                ef->kind = 1;
            } else if (memcmp(line + 17, "ONLYFACETURN", 12) == 0) {
                ef->kind = 2;
            } else if (memcmp(line + 17, "ONLYFACE", 8) == 0) {
                ef->kind = 3;
            } else if (memcmp(line + 17, "FOREVER", 7) == 0) {
                ef->kind = 5;
            } else if (memcmp(line + 17, "PERMANENT", 9) == 0) {
                ef->kind = 6;
            } else if (memcmp(line + 17, "DURINGFACE", 10) == 0) {
                ef->kind = 7;
            } else if (memcmp(line + 17, "EQUIPMENT", 9) == 0) {
                ef->kind = 10;
            } else if (memcmp(line + 17, "UNION", 5) == 0) {
                ef->kind = 11;
            } else if (memcmp(line + 17, "CAPTURED", 8) == 0) {
                ef->kind = 13;
            }
        }
        break;
    }
    return 0;
}

#ifdef NON_MATCHING /* o original testa o byte de destino com andi+ext sobre o halfword do efeito (6 palavras) */
/* tutorial/puzzle start: reads the settings text line by line, then shows the field to the host */
void duel_setup_load_text(char *text)
{
    char line[256];
    unsigned int p;
    int z;
    unsigned int i;
    unsigned int link;
    int n;

    duel.players[0].lp = 8000;
    duel.players[1].lp = 8000;
    duel.players[0].unk4 = 1;
    duel.players[1].unk4 = 1;
    duel_cmd(1, 0, 0, 0);
    for (n = duel_read_line(line, 0x100, &text); n != 0; n = duel_read_line(line, 0x100, &text)) {
        if (duel_setup_parse_line(line)) {
            break;
        }
    }
    duel.players[0].unk8 = duel.players[0].unk4 - 1;
    duel.players[1].unk8 = duel.players[1].unk4 - 1;
    for (p = 0; (int)p < 2; p++) {
        for (z = 0; z <= 10; z++) {
            int id = duel.players[p & 1].field[z].card.id;

            if (id == 0) {
                continue;
            }
            duel.players[p & 1].field[z].card.id = 0;
            duel_cmd((p ? 0x8000 : 0) | 0x44, z & 0xFF, id,
                     duel.players[p & 1].field[z].faceup + duel.players[p & 1].field[z].pos * 10);
            link = duel.players[p & 1].field[z].link;
            if (link != 0) {
                duel_cmd((p ? 0x8000 : 0) | 0x3A, z & 0xFF, link, link >> 16);
            }
        }
    }
    for (i = 0; i < duel_w_0891524C.n; i++) {
        DuelEffectLoc *ef = (DuelEffectLoc *)&duel.effects[i + 1];

        duel_cmd((ef->dst_player ? 0x8000 : 0) | 0x36, ef->dst_zone | ef->kind << 8, ef->src, 0);
        ef->kind = 0;
    }
    duel_w_0891524C.n = 0;
    duel_turn_player = duel_setup.unk10;
    duel_cmd((duel_turn_player ? 0x8000 : 0) | 0x55, 0, duel.players[duel_turn_player & 1].n_hand, 0);
    duel_cmd((1 - duel_turn_player ? 0x8000 : 0) | 0x55, 0, duel.players[(1 - duel_turn_player) & 1].n_hand, 0);
    duel.players[0].n_hand = 0;
    duel.players[1].n_hand = 0;
    if (duel_w_089195E8 == 3) {
        duel_cmd((duel_phase + 0xB) | (duel_view_player ? 0x8000 : 0), 0, 0, 0);
        duel_phase = 7;
        duel_cmd(0x66, 0, 5, 0);
    }
    duel_host_call(0xD, 0, duel.players[0].lp, 0);
    duel_host_call(0xD, 1, duel.players[1].lp, 0);
    duel_host_call(0x14, 0, 0xD, 0);
    duel_host_call(0x14, 1, 0xD, 0);
    duel_host_call(0x14, 0, 0xC, 0);
    duel_host_call(0x14, 1, 0xC, 0);
    duel_host_call(0x14, 0, 0xE, 0);
    duel_host_call(0x14, 1, 0xE, 0);
    duel_host_call(0x14, 0, 0xF, 0);
    duel_host_call(0x14, 1, 0xF, 0);
}
#endif

/* opens a card list for player p (kind a: 6..0x4A pick, else a message) and lets the computer answer */
void duel_0884C588(unsigned int p, int a, int id, int b)
{
    duel_w_0891524C.p = p;
    duel_w_0891524C.kind = a;
    duel_w_0891524C.cur = 0;
    duel_bzero(duel_w_0891524C.c, 0x200);
    duel_bzero(duel_w_0891524C.where, 0x100);
    duel_088BFF78(p, id, b);
    if (a >= 6 && duel_player_type[p] == 1) {
        if (a == 6 || a == 0x4A) {
            duel_08809B50(p, id, b);
        } else if (a > 6 && a <= 0x26) {
            duel_08809BE8(p, id, b, duel_0884C718());
        } else if (a > 0x27 && a <= 0x48) {
            duel_08809CA8(p, id, b, duel_0884C718());
        } else if (a >= 0x4B) {
            duel_08809DA8(p, id);
        }
    } else {
        duel_host_call(0x32, p, a, 0);
    }
}
