// @range 0x08805FF8-0x08808A20
// @asm 0x08806190-0x08806218
// @asm 0x088062E8-0x08806518
// @asm 0x08806678-0x088069F4
// @asm 0x08807430-0x088076F8
// @data 0x0881A1D4-0x0881A6F8
// @rodata 0x08819D90-0x08819F34
/* Duel presentation events: the queue of cut-ins, music and waits that the duel engine's host
   callback fills (character cut-ins for signature cards, life-point mood music, turn changes, the
   duel's end) and that dmgr_events_draw plays back frame by frame. */
#include "libduel_mgr.h"

typedef struct DmgrLoc {               /* where a card moved from/to, as the engine passes it */
    unsigned short player : 1;
    unsigned short zone : 5;           /* 15: deck? */
    unsigned short rest : 10;
} DmgrLoc;

typedef struct DmgrCardPair {          /* (character, card) that gets a cut-in */
    unsigned short chr;
    unsigned short card;
} DmgrCardPair;

/* libduel_eng */
int duel_view_player_get(void);
int duel_view_opponent_get(void);
int duel_lp_get(unsigned int p);
int duel_088FEE74();
unsigned int duel_turn_player_get(void);
int duel_setup_10_get(void);
int duel_slot_id(unsigned int p, int z);
unsigned int duel_field_attrs(unsigned int p, int z);
int duel_088FE87C();
int duel_list_id(int i);
#include "duel.h"
DuelCard *duel_card_by_uid(int uid);
unsigned int duel_card_locate(int uid);
int duel_phase_get(void);
int duel_088FE66C(int a);
unsigned int duel_player_flag(unsigned int p, int i);
int duel_n_d(unsigned int p);
int duel_player_protected(unsigned int p);
int duel_tag_slot(int side);
void duel_player_type_set(int p, int t);
void duel_host_args_get(int *out);

/* libduel_draw */
int libduel_draw_08819A70(void);
void libduel_draw_088199C8(int on);
void libduel_draw_08819AC4(int slot, int kind);
void libduel_draw_08819BFC(int a, int b);
void libduel_draw_08819BC4(int card);
void libduel_draw_08819BCC(int a);
int libduel_draw_08819A7C(void);
void libduel_draw_0885563C(int a, int b);
int libduel_draw_08855688(int a, int b, int c);
void libduel_draw_08806228(int p, int a, int b, void (*cb)(void));
void libduel_draw_088176A0(int cmd, int a, int b, int c);
void libtutorial_088044B8(int a, unsigned short b, unsigned short c, unsigned short d);

/* BOOT */
unsigned int card_get_attribute(int id);
int card_get_level(int id);
int card_is_attribute9(int id);
int card_get_attribute_icon(int id);
int card_is_monster(int id);
unsigned int card_get_prop5_hi(int id);
unsigned int cfg_get_flag(int which);
void frame_counter_reset(void);
unsigned int frame_counter_get(void);
void se_play(int id);
void snd_load_data();
int snd_is_muted(void);
void bgm_play(int id);
void bgm_fade_out(int x, int ms);
void stat_event_record(int card_id, int kind);
int gamestate_slot_snd(int slot, int arg);
int req_flag_get(int i);
void req_flag_set(int i);
void dmgr_answer_time_reset(void);

DmgrCardPair dmgr_cutin_attack[22] = {
    { 0x2, 0x18C8 }, { 0x2, 0x1956 }, { 0x2, 0x1943 }, { 0x2, 0x19EF }, { 0x3, 0x18FA }, { 0x4, 0x17EF },
    { 0x5, 0x1955 }, { 0x6, 0x17EE }, { 0x6, 0x198C }, { 0x7, 0x1951 }, { 0x8, 0x18FC }, { 0x8, 0x18FD },
    { 0x9, 0x18AB }, { 0x28, 0x1894 }, { 0xC, 0x1895 }, { 0xF, 0x1788 }, { 0x10, 0x1441 }, { 0x11, 0x19F8 },
    { 0x12, 0x19A5 }, { 0x12, 0x19A4 }, { 0x12, 0x19A3 }, { 0x0, 0x0 },
};
DmgrCardPair dmgr_cutin_effect[137] = {
    { 0x2, 0x1956 }, { 0x2, 0x194D }, { 0x2, 0x197F }, { 0x2, 0x19FD }, { 0x2, 0x19EF }, { 0x2, 0x1957 },
    { 0x2, 0x1958 }, { 0x2, 0x1943 }, { 0x2, 0x18C9 }, { 0x2, 0x18C8 }, { 0x2, 0x1A6F }, { 0x2, 0x1A6E },
    { 0x2, 0x1906 }, { 0x2, 0x1A70 }, { 0x2, 0x1A75 }, { 0x2, 0x19FE }, { 0x3, 0x13A4 }, { 0x3, 0x18F2 },
    { 0x3, 0x18F3 }, { 0x3, 0x1A52 }, { 0x3, 0x1A53 }, { 0x3, 0x18FA }, { 0x3, 0x18FB }, { 0x4, 0x16B7 },
    { 0x4, 0x161D }, { 0x4, 0x17EF }, { 0x5, 0x1948 }, { 0x5, 0x1A0A }, { 0x5, 0x1952 }, { 0x5, 0x1955 },
    { 0x6, 0x1681 }, { 0x6, 0x16B3 }, { 0x6, 0x16B4 }, { 0x6, 0x198C }, { 0x6, 0x17DB }, { 0x6, 0x15B4 },
    { 0x6, 0x1953 }, { 0x6, 0x1954 }, { 0x6, 0x17EE }, { 0x7, 0x1757 }, { 0x7, 0x191D }, { 0x7, 0x147F },
    { 0x7, 0x1437 }, { 0x7, 0x1951 }, { 0x7, 0x1950 }, { 0x7, 0x194F }, { 0x7, 0x182D }, { 0x7, 0x138A },
    { 0x7, 0x1A40 }, { 0x8, 0x18F6 }, { 0x8, 0x19A9 }, { 0x8, 0x19A8 }, { 0x8, 0x18FD }, { 0x8, 0x18FC },
    { 0x9, 0x18AB }, { 0xA, 0x1331 }, { 0xA, 0x1369 }, { 0xA, 0x11F5 }, { 0xA, 0x1595 }, { 0xA, 0x1530 },
    { 0xA, 0x15DD }, { 0xB, 0x128F }, { 0xB, 0x16FC }, { 0xC, 0x1895 }, { 0xC, 0x188F }, { 0xD, 0x14A4 },
    { 0xD, 0x160F }, { 0xE, 0x1532 }, { 0xF, 0x1788 }, { 0x10, 0x1692 }, { 0x10, 0x1691 }, { 0x10, 0x1441 },
    { 0x11, 0x19F5 }, { 0x11, 0x19F8 }, { 0x12, 0x19A5 }, { 0x12, 0x19A3 }, { 0x12, 0x19A4 },
    { 0x13, 0x17D0 }, { 0x13, 0x1069 }, { 0x13, 0x1073 }, { 0x14, 0x1782 }, { 0x14, 0x183D }, { 0x15, 0xFE5 },
    { 0x15, 0x1477 }, { 0x15, 0x182A }, { 0x15, 0x182B }, { 0x15, 0x182C }, { 0x16, 0x167C },
    { 0x16, 0x189A }, { 0x16, 0x1816 }, { 0x16, 0x1823 }, { 0x16, 0x18E6 }, { 0x17, 0x1649 },
    { 0x18, 0x1439 }, { 0x19, 0x111C }, { 0x19, 0x111A }, { 0x19, 0x1647 }, { 0x19, 0x1118 },
    { 0x1A, 0x1119 }, { 0x1A, 0x111B }, { 0x1A, 0x1115 }, { 0x1A, 0x1118 }, { 0x1B, 0x129E },
    { 0x1B, 0x16ED }, { 0x1C, 0x1296 }, { 0x1C, 0x17DE }, { 0x1C, 0x12DC }, { 0x1D, 0x1434 },
    { 0x1D, 0x1433 }, { 0x1E, 0x1578 }, { 0x1E, 0x1756 }, { 0x1E, 0x16FA }, { 0x1F, 0x129E },
    { 0x20, 0x14E5 }, { 0x20, 0x1287 }, { 0x20, 0x160C }, { 0x20, 0x1879 }, { 0x21, 0x14E5 },
    { 0x21, 0x1287 }, { 0x21, 0x160C }, { 0x21, 0x1879 }, { 0x22, 0x16F9 }, { 0x22, 0x13C3 },
    { 0x22, 0x164A }, { 0x22, 0xFBB }, { 0x22, 0x179A }, { 0x23, 0x1286 }, { 0x23, 0x1369 }, { 0x23, 0x133B },
    { 0x24, 0x106C }, { 0x24, 0x13A7 }, { 0x24, 0x1876 }, { 0x25, 0x178F }, { 0x26, 0x158B },
    { 0x26, 0x158D }, { 0x28, 0x1894 }, { 0x0, 0x0 },
};
DmgrCardPair dmgr_cutin_spell[170] = {
    { 0x2, 0x12E5 }, { 0x2, 0x1303 }, { 0x2, 0x1907 }, { 0x2, 0x12EC }, { 0x2, 0x1920 }, { 0x2, 0x1A11 },
    { 0x2, 0x1A10 }, { 0x2, 0x1A12 }, { 0x2, 0x1A77 }, { 0x3, 0x12EC }, { 0x3, 0x12E5 }, { 0x3, 0x1303 },
    { 0x3, 0x18FE }, { 0x3, 0x1409 }, { 0x4, 0x12EC }, { 0x4, 0x10FC }, { 0x4, 0x16CE }, { 0x4, 0x12E5 },
    { 0x4, 0x1303 }, { 0x4, 0x1446 }, { 0x5, 0x12EC }, { 0x5, 0x12E5 }, { 0x5, 0x1303 }, { 0x5, 0x149C },
    { 0x5, 0x1A37 }, { 0x6, 0x1946 }, { 0x6, 0x1945 }, { 0x6, 0x12EC }, { 0x6, 0x12E5 }, { 0x6, 0x1303 },
    { 0x6, 0x166A }, { 0x7, 0x12EC }, { 0x7, 0x179E }, { 0x7, 0x1438 }, { 0x7, 0x195C }, { 0x7, 0x17A6 },
    { 0x7, 0x16CD }, { 0x7, 0x14EB }, { 0x7, 0x197A }, { 0x7, 0x1518 }, { 0x7, 0x17FE }, { 0x7, 0x135D },
    { 0x7, 0x140E }, { 0x7, 0x1928 }, { 0x7, 0x1929 }, { 0x7, 0x1927 }, { 0x7, 0x192A }, { 0x8, 0x12EC },
    { 0x8, 0x159C }, { 0x8, 0x12E5 }, { 0x8, 0x1303 }, { 0x8, 0x13FE }, { 0x8, 0x1409 }, { 0x8, 0x18FE },
    { 0x8, 0x1A7E }, { 0x9, 0x12EC }, { 0x9, 0x131B }, { 0x9, 0x1409 }, { 0x9, 0x19B2 }, { 0x9, 0x19AC },
    { 0x9, 0x19AD }, { 0x9, 0x19AE }, { 0x9, 0x1543 }, { 0xA, 0x12D2 }, { 0xA, 0x1710 }, { 0xA, 0x1600 },
    { 0xA, 0x15F5 }, { 0xA, 0x1543 }, { 0xB, 0x12EC }, { 0xB, 0x134D }, { 0xC, 0x1536 }, { 0xC, 0x143D },
    { 0xD, 0x12EC }, { 0xD, 0x143E }, { 0xD, 0x14A6 }, { 0xE, 0x169E }, { 0xE, 0x1440 }, { 0xE, 0x153C },
    { 0xE, 0x12EC }, { 0xE, 0x1546 }, { 0xE, 0x143F }, { 0xE, 0x1545 }, { 0xE, 0x16A3 }, { 0xF, 0x17AB },
    { 0xF, 0x12EC }, { 0xF, 0x153E }, { 0xF, 0x17AF }, { 0x10, 0x169F }, { 0x10, 0x169B }, { 0x10, 0x1442 },
    { 0x10, 0x1443 }, { 0x10, 0x16A2 }, { 0x11, 0x1A0E }, { 0x11, 0x12EC }, { 0x11, 0x1444 },
    { 0x11, 0x1A1A }, { 0x12, 0x12EC }, { 0x12, 0x19AF }, { 0x13, 0x1307 }, { 0x13, 0x12EC },
    { 0x13, 0x1321 }, { 0x14, 0x12E5 }, { 0x14, 0x1303 }, { 0x14, 0x12EC }, { 0x14, 0x17A8 },
    { 0x14, 0x1843 }, { 0x14, 0x17B2 }, { 0x15, 0x12EC }, { 0x15, 0x10E4 }, { 0x15, 0x183F },
    { 0x15, 0x1840 }, { 0x15, 0x1A42 }, { 0x16, 0x15E8 }, { 0x17, 0x16BF }, { 0x17, 0x1882 },
    { 0x17, 0x16CE }, { 0x17, 0x1886 }, { 0x17, 0x1357 }, { 0x18, 0x12EC }, { 0x18, 0x143A },
    { 0x18, 0x143B }, { 0x18, 0x143C }, { 0x19, 0x1474 }, { 0x19, 0x15ED }, { 0x19, 0x14F7 },
    { 0x1A, 0x1474 }, { 0x1A, 0x15ED }, { 0x1A, 0x17B7 }, { 0x1B, 0x12EC }, { 0x1B, 0x167E },
    { 0x1B, 0x1467 }, { 0x1B, 0x1818 }, { 0x1C, 0x12EC }, { 0x1C, 0x12D3 }, { 0x1C, 0x12C3 },
    { 0x1C, 0x1803 }, { 0x1C, 0x184D }, { 0x1D, 0x1435 }, { 0x1D, 0x12EC }, { 0x1D, 0x1436 },
    { 0x1E, 0x17A6 }, { 0x1E, 0x12EC }, { 0x1E, 0x134A }, { 0x1E, 0x15EE }, { 0x1E, 0x140E },
    { 0x1F, 0x10F8 }, { 0x20, 0x1921 }, { 0x20, 0x12E5 }, { 0x20, 0x1303 }, { 0x20, 0x14E1 },
    { 0x20, 0x12EC }, { 0x21, 0x1921 }, { 0x21, 0x12E5 }, { 0x21, 0x1303 }, { 0x21, 0x14E1 },
    { 0x21, 0x12EC }, { 0x21, 0x14E3 }, { 0x22, 0x161A }, { 0x22, 0x1638 }, { 0x22, 0x12EC },
    { 0x22, 0x1403 }, { 0x23, 0x12EC }, { 0x23, 0x1468 }, { 0x25, 0x150F }, { 0x26, 0x159D },
    { 0x26, 0x15A4 }, { 0x26, 0x15AC }, { 0x28, 0x12EC }, { 0x28, 0x17F6 }, { 0x0, 0x0 },
};

void dmgr_event_reset(void)
{
    memset(&dmgr.queue, 0, sizeof dmgr.queue);
}

void dmgr_event_push(DmgrEvent *e)
{
    DmgrQueue *q = &dmgr.queue;
    int t;

    memcpy(&q->ev[q->tail], e, sizeof *e);
    t = q->tail + 1;
    q->tail = t < 16 ? t : 0;
}

int dmgr_event_peek(DmgrEvent *e)
{
    DmgrQueue *q = &dmgr.queue;
    unsigned int h = q->head;

    if (h == q->tail) {
        return 0;
    }
    memcpy(e, &q->ev[h], sizeof *e);
    return 1;
}

void dmgr_event_update(DmgrEvent *e)
{
    DmgrQueue *q = &dmgr.queue;

    memcpy(&q->ev[q->head], e, sizeof *e);
}

void dmgr_event_pop(void)
{
    DmgrQueue *q = &dmgr.queue;
    int h;

    if (q->head != q->tail) {
        h = q->head + 1;
        q->head = h < 16 ? h : 0;
    }
}

int dmgr_attack_flags(int p)
{
    DmgrSeq *q = &dmgr.seq;
    int r = 0;

    if (q->attack_on != 0) {
        if (q->attack_player == p) {
            r = 0x10;
        } else {
            r = 8;
        }
    }
    return r;
}

void dmgr_attr_cutin_set(unsigned short id)
{
    DmgrSeq *q = &dmgr.seq;

    switch (card_get_attribute(id)) {
    case 0xD:
        q->attr_cutin = 8;
        break;
    case 0xE:
        q->attr_cutin = 9;
        break;
    default:
        q->attr_cutin = 7;
        break;
    }
}

#ifdef NON_MATCHING /* o original testa n == 0 com beqzl direto ao fim e o n > 1 com bloco "li 7; b fim"; o mwcc inverte o primeiro teste */
int dmgr_attr_single(void)
{
    DmgrSeq *q = &dmgr.seq;
    int i = 0;
    int n = 0;
    int last = 0;

    for (; i <= 9; i++) {
        if (q->attrs & (1 << i)) {
            n++;
            last = i;
        }
    }
    if (n == 0) {
        return 7;
    }
    if (n > 1) {
        return 7;
    }
    switch (last) {
    case 0:
    case 7:
        return 7;
    }
    return last;
}
#endif

int dmgr_cutin_card(int kind, int chr, int card)
{
    DmgrCardPair *t;

    if (chr <= 40) {
        switch (kind) {
        case 0:
            t = dmgr_cutin_attack;
            break;
        case 1:
            t = dmgr_cutin_effect;
            break;
        case 2:
            t = dmgr_cutin_spell;
            break;
        default:
            return 0;
        }
        if (t->chr != 0) {
            do {
                if (t->chr == chr && t->card == card) {
                    return 1;
                }
                t++;
            } while (t->chr != 0);
        }
    } else if (kind == 1 && card_get_level(card) >= 5) {
        return 1;
    }
    return 0;
}

#ifdef NON_MATCHING /* o original mantém cópias mortas da primeira instrução dos blocos destino depois de cada b??l */
void dmgr_bgm_mood_update(void)
{
    DmgrSeq *q = &dmgr.seq;
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrEvent ev;
    int score;
    int lp;
    int lp2;
    int w;
    int d;
    unsigned short id;

    if (gs->kind == 2) {
        return;
    }
    score = 0;
    lp = duel_lp_get(duel_view_player_get());
    if (lp < 2000) {
        score -= 2;
    } else if (lp < 3000) {
        score -= 1;
    }
    if (lp > 4000) {
        score++;
    }
    lp2 = duel_lp_get(duel_view_opponent_get());
    if (lp2 < 2000) {
        score += 2;
    } else if (lp2 < 3000) {
        score++;
    }
    if (lp2 > 4000) {
        score--;
    }
    if (lp < 2000 && lp2 < 2000) {
        w = 2;
    } else {
        w = 1;
    }
    d = duel_088FEE74(duel_view_player_get());
    if (d < 0) {
        score -= w;
    }
    if (d > 0) {
        score += w;
    }
    if (score < -1) {
        q->mood = 2;
    } else if (score < 2) {
        q->mood = 0;
    } else {
        q->mood = 1;
    }
    if (duel_turn_player_get() == duel_view_player_get()) {
        if (score < -1) {
            if (q->cur_bgm != q->bgm.a[1]) {
                return;
            }
            id = q->bgm.a[0];
        } else if (score < 2) {
            id = q->bgm.a[0];
        } else {
            id = q->bgm.a[1];
        }
    } else {
        if (score >= 2) {
            if (q->cur_bgm != q->bgm.a[2]) {
                return;
            }
            id = q->bgm.a[0];
        } else if (score >= -1) {
            id = q->bgm.a[0];
        } else {
            id = q->bgm.a[2];
        }
    }
    if (id != q->cur_bgm) {
        bgm_cmd(1500);
        snd_load_data(id);
        memset(&ev, 0, sizeof ev);
        ev.type = 6;
        ev.a = id;
        dmgr_event_push(&ev);
    }
}
#endif

int dmgr_mood_of(int p)
{
    DmgrSeq *q = &dmgr.seq;

    if (p == duel_view_player_get()) {
        return q->mood;
    }
    if (q->mood == 2) {
        return 1;
    }
    if (q->mood == 1) {
        return 2;
    }
    return 0;
}

int dmgr_cutins_off(void)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();

    if (sysvar_get_a98() != 0) {
        return 1;
    }
    if (cfg_get_flag(0) == 0) {
        return 1;
    }
    switch (gs->kind) {
    case 2:
    case 3:
        return 1;
    }
    return 0;
}

int dmgr_is_npc(int chr)
{
    int r;

    if (chr < 2) {
        r = 0;
    } else {
        switch (chr) {
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        case 0x8D:
        case 0x8E:
            r = 0;
            break;
        default:
            r = 1;
            break;
        }
    }
    return r;
}

void dmgr_cutin_show(int on)
{
    if (on != libduel_draw_08819A70()) {
        libduel_draw_088199C8(on);
    }
}

#ifdef NON_MATCHING /* cópias mortas depois dos b??l no original (lbu/lhu/sh repetidos), que o mwcc remove */
void dmgr_ev_start(int *args)
{
    DmgrSeq *q = &dmgr.seq;
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DuelResults *res = game_state2_get();
    DmgrEvent ev;
    int order[2];
    int shown;
    int i;
    int t;

    frame_counter_reset();
    res->lp_left[1] = 0;
    res->lp_left[0] = 0;
    order[1] = -1;
    order[0] = -1;
    q->cur_bgm = 0;
    q->opening = 0;
    if (dmgr_cutins_off() != 0) {
        dmgr_cutin_show(0);
        libduel_draw_088176A0(args[0], args[1], args[2], args[3]);
        if (gs->kind == 2) {
            return;
        }
        memset(&ev, 0, sizeof ev);
        ev.type = 6;
        ev.a = q->bgm.a[0];
        dmgr_event_push(&ev);
        return;
    }
    switch (gs->partner_mode) {
    case 2:
        if (dmgr_is_npc(gs->slot[3].type) != 0) {
            order[1] = 3;
        }
        if (dmgr_is_npc(gs->slot[2].type) != 0) {
            order[0] = 2;
        }
    case 1:
        if (dmgr_is_npc(gs->slot[1].type) != 0) {
            order[1] = 1;
        }
        if (dmgr_is_npc(gs->slot[0].type) != 0) {
            order[0] = 0;
        }
        break;
    }
    if (duel_setup_10_get() == 0) {
        t = order[0];
        order[0] = order[1];
        order[1] = t;
    }
    if (order[0] >= 0 || order[1] >= 0) {
        shown = 0;
        for (i = 0; i < 2; i++) {
            if (order[i] >= 0) {
                if (shown == 0) {
                    dmgr_cutin_show(1);
                    libduel_draw_08819AC4(order[i], 1);
                    libduel_draw_08819BFC(0, 0);
                    shown++;
                } else {
                    memset(&ev, 0, sizeof ev);
                    ev.type = 1;
                    ev.b = 1;
                    ev.a = order[i];
                    dmgr_event_push(&ev);
                    memset(&ev, 0, sizeof ev);
                    ev.type = 15;
                    ev.a = 0;
                    ev.b = 0;
                    dmgr_event_push(&ev);
                }
                memset(&ev, 0, sizeof ev);
                ev.type = 2;
                dmgr_event_push(&ev);
            }
        }
    }
    memset(&ev, 0, sizeof ev);
    ev.type = 10;
    ev.a = args[0];
    ev.b = args[1];
    ev.c = args[2];
    ev.d = args[3];
    dmgr_event_push(&ev);
    memset(&ev, 0, sizeof ev);
    ev.type = 11;
    if (order[0] >= 0) {
        ev.a = order[0];
    } else {
        ev.a = 4;
    }
    if (order[1] >= 0) {
        ev.b = order[1];
    } else {
        ev.b = 4;
    }
    dmgr_event_push(&ev);
    memset(&ev, 0, sizeof ev);
    ev.type = 12;
    ev.a = q->bgm.a[0];
    dmgr_event_push(&ev);
    if (dmgr_is_npc(dmgr_slot(gs, duel_tag_slot(duel_setup_10_get()))->type) != 0) {
        q->opening = 1;
    }
}
#endif

void dmgr_ev_end(int *args)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrSeq *q = &dmgr.seq;
    DuelResults *res = game_state2_get();
    DmgrEvent ev;
    int order[2];
    unsigned short id;
    unsigned short kind;
    unsigned short pose;
    int t;
    int *o;
    int i;

    res->frames = frame_counter_get();
    bgm_cmd(100);
    se_play(0x6249);
    memset(&ev, 0, sizeof ev);
    ev.type = 13;
    ev.a = args[0];
    dmgr_event_push(&ev);
    switch (args[1]) {
    case 1:
        id = q->bgm.b[0];
        break;
    case 2:
        id = q->bgm.b[1];
        break;
    case 3:
        id = q->bgm.b[2];
        break;
    }
    memset(&ev, 0, sizeof ev);
    ev.type = 6;
    ev.a = id;
    dmgr_event_push(&ev);
    memset(&ev, 0, sizeof ev);
    ev.type = 14;
    ev.a = args[0];
    dmgr_event_push(&ev);
    if (dmgr_cutins_off() != 0) {
        return;
    }
    order[1] = -1;
    order[0] = -1;
    switch (gs->partner_mode) {
    case 2:
        if (dmgr_is_npc(gs->slot[3].type) != 0) {
            order[1] = 3;
        }
        if (dmgr_is_npc(gs->slot[2].type) != 0) {
            order[0] = 2;
        }
    case 1:
        if (dmgr_is_npc(gs->slot[1].type) != 0) {
            order[1] = 1;
        }
        if (dmgr_is_npc(gs->slot[0].type) != 0) {
            order[0] = 0;
        }
        break;
    }
    if (duel_view_opponent_get() != 0) {
        t = order[0];
        order[0] = order[1];
        order[1] = t;
    }
    if (order[0] >= 0 || order[1] >= 0) {
        for (i = 0, o = order; i < 2; i++, o++) {
            if (*o >= 0) {
                switch (args[1]) {
                case 1:
                    if (i == 0) {
                        kind = 0x11;
                        pose = 0x19;
                    } else {
                        kind = 0x10;
                        pose = 0x18;
                    }
                    break;
                case 2:
                    if (i == 0) {
                        kind = 0x10;
                        pose = 0x18;
                    } else {
                        kind = 0x11;
                        pose = 0x19;
                    }
                    break;
                case 3:
                    kind = 0x12;
                    pose = 0x1A;
                    break;
                default:
                    kind = 0;
                    break;
                }
                memset(&ev, 0, sizeof ev);
                ev.type = 1;
                ev.b = kind;
                ev.a = *o;
                dmgr_event_push(&ev);
                memset(&ev, 0, sizeof ev);
                ev.type = 15;
                ev.a = pose;
                ev.b = 0;
                dmgr_event_push(&ev);
                memset(&ev, 0, sizeof ev);
                ev.type = 2;
                dmgr_event_push(&ev);
            }
        }
    }
}

void dmgr_ev_04(int *args)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrSeq *q = &dmgr.seq;
    DuelResults *res = game_state2_get();
    DmgrEvent ev;
    int lp;
    int k;
    int i;

    for (i = 0; i < 4; i++) {
        q->n_spells[i] = 0;
        q->n_monsters[i] = 0;
    }
    lp = duel_lp_get(duel_view_player_get());
    res->unk14 = lp < duel_lp_get(duel_view_opponent_get());
    if (dmgr_cutins_off() != 0) {
        return;
    }
    k = duel_tag_slot(args[1]);
    if (q->opening != 0) {
        q->opening = 0;
        memset(&ev, 0, sizeof ev);
        ev.type = 1;
        ev.b = 2;
        ev.a = k;
        dmgr_event_push(&ev);
        memset(&ev, 0, sizeof ev);
        ev.type = 15;
        ev.a = 2;
        ev.b = 0;
        dmgr_event_push(&ev);
        memset(&ev, 0, sizeof ev);
        ev.type = 2;
        dmgr_event_push(&ev);
    }
    if (dmgr_is_npc(dmgr_slot(gs, k)->type) != 0) {
        switch (args[2]) {
        case 0:
            break;
        case 3:
            dmgr_cutin_show(1);
            libduel_draw_08819AC4(k, 2);
            libduel_draw_08819BFC(0x1B, 0);
            memset(&ev, 0, sizeof ev);
            ev.type = 2;
            dmgr_event_push(&ev);
            memset(&ev, 0, sizeof ev);
            ev.type = 4;
            dmgr_event_push(&ev);
            break;
        }
    }
}

void dmgr_tag_swap_done(void)
{
    dmgr.unk28 = 0;
}

void dmgr_ev_turn(int *args)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrEvent ev;
    PlayerSlot *s;
    unsigned int p;
    int k;
    int wait;

    dmgr_answer_time_reset();
    dmgr_bgm_mood_update();
    p = duel_turn_player_get();
    if (gs->partner_mode == 2) {
        k = duel_tag_slot(p);
        duel_088FE66C(p);
        s = &gs->slot[k];
        if (sysvar_get_a98() != 0) {
            if (p != netsync_local_side()) {
                duel_player_type_set(p, 2);
            } else {
                switch (s->type) {
                case 0:
                    duel_player_type_set(p, 2);
                    break;
                case 1:
                    duel_player_type_set(p, 0);
                    break;
                default:
                    duel_player_type_set(p, 1);
                    break;
                }
            }
        } else if (s->type > 1) {
            duel_player_type_set(p, 1);
        } else {
            duel_player_type_set(p, 0);
        }
        dmgr.unk28 = 1;
        switch (k) {
        case 0:
            libduel_draw_08806228(p, 2, 0, dmgr_tag_swap_done);
            break;
        case 1:
            libduel_draw_08806228(p, 3, 1, dmgr_tag_swap_done);
            break;
        case 2:
            libduel_draw_08806228(p, 0, 2, dmgr_tag_swap_done);
            break;
        case 3:
            libduel_draw_08806228(p, 1, 3, dmgr_tag_swap_done);
            break;
        }
        memset(&ev, 0, sizeof ev);
        ev.type = 5;
        dmgr_event_push(&ev);
        wait = 0;
    } else {
        wait = 30;
    }
    if (duel_player_flag(p, 0x18) == 0 && (duel_n_d(p) != 0 || duel_player_protected(p) != 0) &&
        dmgr_cutins_off() == 0) {
        k = duel_tag_slot(p);
        if (dmgr_is_npc(dmgr_slot(gs, k)->type) != 0) {
            if (wait != 0) {
                memset(&ev, 0, sizeof ev);
                ev.type = 3;
                ev.a = wait;
                dmgr_event_push(&ev);
            }
            memset(&ev, 0, sizeof ev);
            ev.type = 1;
            ev.b = 2;
            ev.a = k;
            dmgr_event_push(&ev);
            memset(&ev, 0, sizeof ev);
            ev.type = 15;
            ev.a = 2;
            ev.b = dmgr_mood_of(p);
            dmgr_event_push(&ev);
            memset(&ev, 0, sizeof ev);
            ev.type = 2;
            dmgr_event_push(&ev);
            memset(&ev, 0, sizeof ev);
            ev.type = 4;
            dmgr_event_push(&ev);
        }
    }
}

void dmgr_ev_attack(int *args)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrSeq *q = &dmgr.seq;
    DmgrEvent ev;
    int st[8];
    PlayerSlot *s;
    int k;
    int id;
    int kind;
    int fl;
    int pose;

    if (dmgr_cutins_off() == 0 && args[3] >= 0) {
        duel_tag_slot(args[2]);
        if (args[3] < 5) {
            q->target = duel_slot_id(args[2], args[3]);
        } else {
            q->target = 0;
        }
        q->attrs = duel_field_attrs(q->turn_player, args[1]);
        k = duel_tag_slot(q->turn_player);
        s = dmgr_slot(gs, k);
        if (dmgr_is_npc(s->type) != 0) {
            id = duel_slot_id(q->turn_player, args[1]);
            if (dmgr_cutin_card(0, s->type, id) != 0) {
                kind = 0x13;
                fl = 4;
            } else {
                kind = 0xB;
                fl = 0;
            }
            dmgr_cutin_show(1);
            libduel_draw_08819AC4(k, kind);
            libduel_draw_08819BC4(id);
            if (args[3] >= 5) {
                pose = 0x11;
            } else {
                pose = 0x10;
            }
            duel_088FE87C(q->turn_player, args[1], args[3], 1, st);
            if (duel_lp_get(args[2]) <= st[7]) {
                fl |= 0x80;
            }
            libduel_draw_08819BFC(pose, fl);
            memset(&ev, 0, sizeof ev);
            ev.type = 2;
            dmgr_event_push(&ev);
            if (dmgr_cutin_card(0, s->type, id) != 0) {
                libduel_draw_0885563C(args[2], 1);
                memset(&ev, 0, sizeof ev);
                ev.type = 9;
                ev.b = 1;
                ev.a = id;
                dmgr_event_push(&ev);
            }
            memset(&ev, 0, sizeof ev);
            ev.type = 4;
            dmgr_event_push(&ev);
        }
    }
}

#ifdef NON_MATCHING /* o teste de dmgr_cutin_card sai bnezl + "b fim" com cópia morta no original; o mwcc gera beqz direto */
void dmgr_ev_effect(int *args)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrSeq *q = &dmgr.seq;
    DuelResults *res = game_state2_get();
    DmgrEvent ev;
    DmgrLoc from;
    DmgrLoc to;
    DuelCard *card;
    PlayerSlot *s;
    unsigned int uid;
    unsigned int act;
    unsigned int p;
    int id;
    int k;

    if (dmgr_cutins_off() != 0) {
        return;
    }
    uid = args[1] & 0xFFFF;
    act = (args[1] >> 16) & 0xFFFF;
    id = duel_list_id(uid);
    p = uid & 1;
    k = duel_tag_slot(p);
    s = dmgr_slot(gs, k);
    from = *(DmgrLoc *)&args[2];
    to = *(DmgrLoc *)&args[3];
    card = duel_card_by_uid(uid);
    if (dmgr_is_npc(s->type) == 0) {
        return;
    }
    if ((*(unsigned char *)&from & 1) != p) {
        return;
    }
    if (dmgr_cutin_card(1, s->type, id) != 0) {
        switch (act) {
        case 0:
            if ((*(unsigned char *)&to & 1) == p) {
                if (to.zone != 15) {
                    return;
                }
                if (card->b18) {
                    goto count;
                }
                return;
            }
            if (q->act_player == p) {
                return;
            }
            goto count;
        case 5:
            if (card->b18) {
                goto count;
            }
            return;
        case 6:
            if (q->turn_player != p) {
                goto count;
            }
            if (q->act_player != p) {
                goto count;
            }
            return;
        case 7:
        case 8:
            return;
        default:
            return;
        }
    }
    return;
count:
    res->lp_left[p]++;
    memset(&ev, 0, sizeof ev);
    ev.type = 3;
    ev.a = 0x10;
    dmgr_event_push(&ev);
    memset(&ev, 0, sizeof ev);
    ev.type = 1;
    ev.b = 0x18;
    ev.a = k;
    dmgr_event_push(&ev);
    memset(&ev, 0, sizeof ev);
    ev.type = 15;
    ev.a = 0x1C;
    ev.b = 0;
    dmgr_event_push(&ev);
    memset(&ev, 0, sizeof ev);
    ev.type = 2;
    dmgr_event_push(&ev);
    memset(&ev, 0, sizeof ev);
    ev.type = 4;
    dmgr_event_push(&ev);
}
#endif

void dmgr_ev_activate(int *args)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrSeq *q = &dmgr.seq;
    DmgrEvent ev;
    unsigned int loc;
    int k;
    int fl;
    int kind;

    q->skip_card = 0;
    if (dmgr_cutins_off() != 0) {
        return;
    }
    if (q->pending_card != args[2]) {
        return;
    }
    k = duel_tag_slot(q->act_player);
    if (dmgr_is_npc(dmgr_slot(gs, k)->type) == 0) {
        return;
    }
    fl = 0;
    loc = duel_card_locate(args[3]);
    switch (args[2]) {
    case 0x19C7:
    case 0x1A92:
        return;
    case 0x11CF:
    case 0x1578:
    case 0x144A:
    case 0x1639:
    case 0x168C:
    case 0x168D:
    case 0x168E:
    case 0x168F:
    case 0x1691:
    case 0x1692:
        if (q->act_player != duel_turn_player_get()) {
            break;
        }
        if (duel_phase_get() != 1) {
            break;
        }
        return;
    case 0x161B:
    case 0x1690:
        if (duel_phase_get() != 1) {
            break;
        }
        return;
    case 0x150E:
        if (q->act_player == duel_turn_player_get()) {
            break;
        }
        if (duel_phase_get() == 5) {
            break;
        }
        return;
    case 0x1174:
    case 0x163F:
    case 0x1914:
        if ((unsigned char)((loc & 0xFFFF) >> 8) == 0xE) {
            return;
        }
        break;
    }
    if ((unsigned char)((loc & 0xFFFF) >> 8) == 0xB) {
        kind = 5;
        fl |= 0x20;
    } else if ((unsigned char)((loc & 0xFFFF) >> 8) == 0x10 && args[2] == 0x12A1) {
        q->skip_card = 0x12A1;
        return;
    } else {
        kind = 6;
    }
    dmgr_cutin_show(1);
    libduel_draw_08819AC4(k, kind);
    libduel_draw_08819BC4(args[2]);
    libduel_draw_08819BFC(7, fl | dmgr_attack_flags(q->act_player));
    memset(&ev, 0, sizeof ev);
    ev.type = 2;
    dmgr_event_push(&ev);
    memset(&ev, 0, sizeof ev);
    ev.type = 4;
    dmgr_event_push(&ev);
    q->pending_card = 0;
}

void dmgr_ev_3f(int *args)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrEvent ev;
    int k;

    if (dmgr_cutins_off() == 0 && args[3] == 0) {
        k = duel_tag_slot(args[1]);
        if (dmgr_is_npc(dmgr_slot(gs, k)->type) != 0) {
            memset(&ev, 0, sizeof ev);
            ev.type = 1;
            ev.b = 3;
            ev.a = k;
            dmgr_event_push(&ev);
            memset(&ev, 0, sizeof ev);
            ev.type = 15;
            ev.a = 3;
            ev.b = dmgr_mood_of(args[1]);
            dmgr_event_push(&ev);
            memset(&ev, 0, sizeof ev);
            ev.type = 2;
            dmgr_event_push(&ev);
            memset(&ev, 0, sizeof ev);
            ev.type = 4;
            dmgr_event_push(&ev);
        }
    }
}

void dmgr_ev_40(int *args)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrSeq *q = &dmgr.seq;
    DmgrEvent ev;
    PlayerSlot *s;
    int k;
    int m;
    int hi;
    int sig;
    int kind;
    int pose;
    int fl;

    if (dmgr_cutins_off() != 0) {
        return;
    }
    k = duel_tag_slot(args[1]);
    s = dmgr_slot(gs, k);
    if (dmgr_is_npc(s->type) == 0) {
        return;
    }
    if (card_is_attribute9(args[2]) != 0) {
        return;
    }
    if (q->skip_card == args[2]) {
        q->skip_card = 0;
        return;
    }
    hi = (unsigned char)((args[3] & 0xFFFF) >> 8);
    m = args[3] & 0xFF;
    sig = dmgr_cutin_card(0, s->type, args[2]);
    if (m == 4) {
        return;
    }
    if (m >= 5) {
        if (sig != 0) {
            kind = 0x17;
        } else {
            kind = 10;
        }
    } else if (sig != 0) {
        kind = 0x16;
    } else {
        kind = 9;
    }
    fl = 0;
    switch (m) {
    case 0:
        pose = 8;
        break;
    case 1:
    case 2:
    case 3:
        pose = 8;
        fl |= 0x100;
        break;
    case 4:
        return;
    case 5:
        pose = 10;
        break;
    case 6:
        switch (card_get_attribute_icon(args[2])) {
        case 2:
            pose = 0xF;
            break;
        case 3:
            pose = 0xE;
            break;
        default:
            pose = 10;
            break;
        }
        break;
    }
    if (sig != 0) {
        fl |= 4;
    }
    if (hi != 0) {
        if (pose == 8) {
            fl |= 0x200;
        }
        if (sig == 0) {
            kind = 8;
        }
    }
    dmgr_cutin_show(1);
    libduel_draw_08819AC4(k, kind);
    libduel_draw_08819BC4(args[2]);
    libduel_draw_08819BFC(pose, fl);
    memset(&ev, 0, sizeof ev);
    ev.type = 2;
    dmgr_event_push(&ev);
    if (dmgr_cutin_card(0, s->type, args[2]) != 0) {
        libduel_draw_0885563C(args[2], 0);
        memset(&ev, 0, sizeof ev);
        ev.type = 9;
        ev.a = args[2];
        ev.b = 0;
        dmgr_event_push(&ev);
    }
    memset(&ev, 0, sizeof ev);
    ev.type = 4;
    dmgr_event_push(&ev);
}

void dmgr_ev_summon(int *args)
{
    DmgrSeq *q = &dmgr.seq;
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrEvent ev;
    PlayerSlot *s;
    int k;
    int kind;
    int pose;
    int fl;

    q->pending_card = 0;
    if (dmgr_cutins_off() != 0) {
        return;
    }
    q->act_player = args[1];
    k = duel_tag_slot(args[1]);
    s = dmgr_slot(gs, k);
    if (dmgr_is_npc(s->type) == 0) {
        return;
    }
    switch (args[3]) {
    case 0:
    case 1:
        if (card_is_monster(args[2]) == 0) {
            goto clear;
        }
        if (q->set_card == args[2]) {
            goto clear;
        }
        q->pending_card = args[2];
        q->set_card = 0;
        return;
    case 2:
        q->set_card = args[2];
        if (card_get_prop5_hi(args[2]) == 2) {
            kind = 7;
            pose = 4;
            fl = 0x20;
            if (s->type == 2 && args[2] == 0x18FF) {
                fl = 4;
            }
        } else {
            kind = 5;
            fl = 0x20;
            switch (card_get_attribute(args[2])) {
            case 0xD:
                pose = 4;
                break;
            case 0xE:
                pose = 5;
                break;
            default:
                pose = 7;
                break;
            }
        }
        if (dmgr_cutin_card(2, s->type, args[2]) != 0) {
            kind = 0x15;
        }
        break;
    }
    dmgr_cutin_show(1);
    libduel_draw_08819AC4(k, kind);
    libduel_draw_08819BC4(args[2]);
    libduel_draw_08819BFC(pose, fl | dmgr_attack_flags(args[1]));
    memset(&ev, 0, sizeof ev);
    ev.type = 2;
    dmgr_event_push(&ev);
    memset(&ev, 0, sizeof ev);
    ev.type = 4;
    dmgr_event_push(&ev);
    return;
clear:
    q->set_card = 0;
}

void dmgr_ev_44(int *args)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrSeq *q = &dmgr.seq;
    DmgrEvent ev;
    int k;
    int kind;
    int pose;

    if (dmgr_cutins_off() == 0) {
        k = duel_tag_slot(args[1]);
        if (dmgr_is_npc(dmgr_slot(gs, k)->type) != 0 && args[3] == 0) {
            if (card_is_monster(args[2]) != 0) {
                q->n_monsters[k]++;
                if (q->n_monsters[k] != 1) {
                    return;
                }
                kind = 8;
                pose = 9;
            } else {
                q->n_spells[k]++;
                if (q->n_spells[k] != 1) {
                    return;
                }
                kind = 4;
                pose = 6;
            }
            dmgr_cutin_show(1);
            libduel_draw_08819AC4(k, kind);
            libduel_draw_08819BFC(pose, 0);
            memset(&ev, 0, sizeof ev);
            ev.type = 2;
            dmgr_event_push(&ev);
            memset(&ev, 0, sizeof ev);
            ev.type = 4;
            dmgr_event_push(&ev);
        }
    }
}

void dmgr_ev_47(int *args)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrEvent ev;
    PlayerSlot *s;
    int k;
    int kind;
    int pose;
    int fl;

    if (dmgr_cutins_off() == 0 && args[3] != 0) {
        k = duel_tag_slot(args[1]);
        s = dmgr_slot(gs, k);
        if (dmgr_is_npc(s->type) != 0) {
            switch (card_get_attribute(args[2])) {
            case 0xD:
                kind = 6;
                fl = 0;
                pose = 4;
                break;
            case 0xE:
                kind = 6;
                fl = 0x40;
                pose = 5;
                break;
            default:
                return;
            }
            if (dmgr_cutin_card(2, s->type, args[2]) != 0) {
                kind = 0x14;
            }
            dmgr_cutin_show(1);
            libduel_draw_08819AC4(k, kind);
            libduel_draw_08819BC4(args[2]);
            libduel_draw_08819BFC(pose, fl | dmgr_attack_flags(args[1]));
            memset(&ev, 0, sizeof ev);
            ev.type = 2;
            dmgr_event_push(&ev);
            memset(&ev, 0, sizeof ev);
            ev.type = 4;
            dmgr_event_push(&ev);
        }
    }
}

void dmgr_ev_48(void)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrEvent ev;
    int k;

    if (dmgr_cutins_off() == 0) {
        k = duel_tag_slot(duel_turn_player_get());
        if (dmgr_is_npc(dmgr_slot(gs, k)->type) != 0) {
            dmgr_cutin_show(1);
            libduel_draw_08819AC4(k, 0xC);
            libduel_draw_08819BFC(0x12, 0);
            memset(&ev, 0, sizeof ev);
            ev.type = 2;
            dmgr_event_push(&ev);
            memset(&ev, 0, sizeof ev);
            ev.type = 4;
            dmgr_event_push(&ev);
        }
    }
}

void dmgr_ev_damage(int *args)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrSeq *q = &dmgr.seq;
    DmgrEvent ev;
    int k;
    int bub;
    int kind;

    if (dmgr_cutins_off() != 0) {
        return;
    }
    k = duel_tag_slot(args[1]);
    if (dmgr_is_npc(dmgr_slot(gs, k)->type) == 0) {
        return;
    }
    if (duel_lp_get(args[1]) <= 0) {
        return;
    }
    if (args[2] > -1000) {
        bub = 0;
    } else if (args[2] > -2000) {
        bub = 1;
    } else {
        bub = 2;
    }
    if ((unsigned short)dmgr_mood_of(args[1]) == 2) {
        bub++;
    }
    switch (bub) {
    case 0:
        bub = 0x13;
        break;
    case 1:
        bub = 0x14;
        break;
    default:
        bub = 0x15;
        break;
    }
    switch (args[3]) {
    case 1:
        libduel_draw_08819BCC(dmgr_attr_single());
        kind = 0xD;
        break;
    case 0:
        libduel_draw_08819BCC(q->attr_cutin);
        kind = 0xD;
        break;
    case 2:
    case 3:
        kind = 0xF;
        bub = 0x17;
        break;
    case 4:
        kind = 0x19;
        bub = 0;
        break;
    default:
        return;
    }
    if (args[2] + duel_lp_get(args[1]) <= 0) {
        kind = 0xE;
        bub = 0x16;
    }
    dmgr_cutin_show(1);
    libduel_draw_08819AC4(k, kind);
    if (bub != 0) {
        libduel_draw_08819BFC(bub, 0);
    }
    memset(&ev, 0, sizeof ev);
    ev.type = 2;
    dmgr_event_push(&ev);
    memset(&ev, 0, sizeof ev);
    ev.type = 4;
    dmgr_event_push(&ev);
}

void dmgr_event_dispatch(void)
{
    DmgrSeq *q = &dmgr.seq;
    int args[4];

    duel_host_args_get(args);
    switch (args[0]) {
    case 1:
        dmgr_ev_start(args);
        break;
    case 2:
        dmgr_ev_end(args);
        break;
    case 3:
        dmgr_cutin_show(0);
        break;
    case 4:
        dmgr_ev_04(args);
        break;
    case 5:
        dmgr_ev_turn(args);
        break;
    case 9:
        dmgr.seq.act_player = dmgr.seq.turn_player = duel_turn_player_get();
        break;
    case 0xB:
        dmgr_ev_attack(args);
        break;
    case 0x18:
        dmgr_ev_effect(args);
        break;
    case 0x20:
        dmgr_ev_activate(args);
        dmgr_attr_cutin_set(args[2]);
        break;
    case 0x2F:
        q->attack_on = 0;
        break;
    case 0x3F:
        dmgr_ev_3f(args);
        break;
    case 0x40:
        dmgr_ev_40(args);
        break;
    case 0x42:
        q->attack_on = 1;
        q->attack_player = q->act_player;
        break;
    case 0x43:
        dmgr_ev_summon(args);
        break;
    case 0x44:
        dmgr_ev_44(args);
        break;
    case 0x47:
        dmgr_ev_47(args);
        break;
    case 0x48:
        dmgr_ev_48();
        break;
    case 0x49:
        dmgr_ev_damage(args);
        break;
    case 0x3E:
        libtutorial_088044B8(args[1], args[2], args[3], args[3] >> 16);
        break;
    case 0x0:
    case 0x6:
    case 0x7:
    case 0x8:
    case 0xA:
    case 0xC:
    case 0xD:
    case 0xE:
    case 0xF:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x1C:
    case 0x1D:
    case 0x1E:
    case 0x1F:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
    case 0x26:
    case 0x27:
    case 0x28:
    case 0x29:
    case 0x2A:
    case 0x2B:
    case 0x2C:
    case 0x2D:
    case 0x2E:
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3A:
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x41:
    case 0x45:
    case 0x46:
    case 0x4A:
        break;
    }
}

void dmgr_events_draw(void)
{
    DmgrGameHdr *gs = (DmgrGameHdr *)game_state_get();
    DmgrSeq *q = &dmgr.seq;
    DmgrEvent ev;
    int wait = 0;
    int r;

    while (dmgr_event_peek(&ev)) {
        switch (ev.type) {
        case 1:
            dmgr_cutin_show(1);
            libduel_draw_08819AC4(ev.a, ev.b);
            break;
        case 2:
            wait = libduel_draw_08819A7C();
            break;
        case 3:
            if (ev.a != 0) {
                ev.a--;
                dmgr_event_update(&ev);
                wait = 1;
            }
            break;
        case 4:
            dmgr_cutin_show(0);
            break;
        case 5:
            if (dmgr.unk28) {
                wait = 1;
            }
            break;
        case 6:
            if (snd_is_muted() == 0) {
                if (snd_count_bgm() != 0) {
                    wait = 1;
                } else {
                    bgm_cmd3(q->cur_bgm);
                    q->cur_bgm = ev.a;
                    bgm_play(q->cur_bgm);
                }
            }
            break;
        case 7:
            if (snd_is_muted() == 0 && snd_count_bgm() != 0) {
                wait = 1;
            }
            break;
        case 8:
            if (snd_is_muted() == 0 && snd_count_bgm() != 0) {
                return;
            }
            break;
        case 9:
            if (dmgr.fade != 2) {
                stat_event_record(ev.a, ev.b == 1);
                r = libduel_draw_08855688(ev.a, ev.b, 2);
                if (scene_stack_peek_arg(6) == 13 && r > 0 && gs->kind == 5) {
                    bgm_cmd(800);
                    dmgr.fade = 5;
                }
            }
            break;
        case 10:
            dmgr_cutin_show(0);
            libduel_draw_088176A0(ev.a, ev.b, ev.c, ev.d);
            break;
        case 11:
            if (snd_is_muted() == 0) {
                if (ev.a < 4) {
                    gamestate_slot_snd(ev.a, 1);
                }
                if (ev.b < 4) {
                    gamestate_slot_snd(ev.b, 1);
                }
            }
            break;
        case 12:
            if (snd_is_muted() == 0) {
                if (snd_count_se() == 0) {
                    q->cur_bgm = ev.a;
                    bgm_play(q->cur_bgm);
                    q->bgm_started = 1;
                } else {
                    wait = 1;
                }
            }
            break;
        case 13:
            if (req_flag_get(ev.a) == 0) {
                return;
            }
            break;
        case 14:
            if (req_flag_get(ev.a) != 0) {
                return;
            }
            break;
        case 15:
            libduel_draw_08819BFC(ev.a, ev.b);
            break;
        case 16:
            bgm_fade_out(ev.a, ev.b);
            break;
        }
        if (wait) {
            req_flag_set(0);
            return;
        }
        dmgr_event_pop();
    }
}
