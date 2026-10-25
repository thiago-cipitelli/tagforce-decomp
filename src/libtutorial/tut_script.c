// @range 0x08807E24-0x08809A5C
// @asm 0x08808518-0x088088CC
// @asm 0x08809944-0x08809A5C
// @rodata 0x0880F884-0x0880F938
// @data 0x0880FAA8-0x0880FAC0
/* The lesson script and its hooks into the duel: %02d_duel_command.bin is a list of steps
   (0x67 = tutorial command, 0x64/0x65 = jumps, the rest go to the duel engine); the field set-up
   text (CardIn lines) starts the duel; the action filter and the event hook let only the expected
   player action / duel event through while the script waits for it. */
#include "libtutorial.h"
#include "game.h"

typedef struct TutStep {         /* one script step, 8 bytes */
    unsigned short cmd;          /* low 12 bits: 0x64 jump, 0x65 conditional jump, 0x67 tutorial
                                    command (bit 15 = opponent), else a duel command */
    unsigned short a;
    unsigned short b;
    unsigned short c;
} TutStep;

typedef struct TutScript {       /* tut_script 0x088130BC */
    int n;
    TutStep step[0x800];
} TutScript;

typedef struct TutWait {         /* tut_wait 0x088170E0 */
    int on[2];
    int val[2];
    int mode;                    /* +0x10 0 none, 1 check the answer, 2 locked, 3 confirm, 4 replay */
} TutWait;

typedef struct TutArgs {         /* a blocked player action or duel event */
    int kind;
    int arg[3];
} TutArgs;

typedef struct TutZoneRef {      /* card position packed in 16 bits (libduel_draw) */
    unsigned short player : 1;
    unsigned short zone : 5;
    unsigned short index : 10;
} TutZoneRef;

typedef struct TutDeckSetup {    /* card lists of the field set-up, 0x108 bytes */
    int n;
    int pad04;
    int n_extra;                 /* +0x08 */
    unsigned short ids[95];      /* +0x0C */
    unsigned short extra[31];    /* +0xCA */
} TutDeckSetup;

typedef struct TutDuelSetup {    /* argument of libduel_draw_08805D2C, 0x40 bytes */
    int mode;                    /* +0x00 2 */
    int unk04;
    int partner;                 /* +0x08 */
    int pad0c[6];
    TutDeckSetup *deck[2];       /* +0x24 */
    int pad2c[2];
    int unk34;
    int unk38;
    int unk3c;
} TutDuelSetup;

typedef struct TutDuelView {     /* the fields of libduel_eng's Duel used here (duel_get) */
    unsigned char pad0[0x3778];
    int field_turn;              /* +0x3778 */
    unsigned char pad377c[0x3798 - 0x377C];
    int busy3798;                /* +0x3798 */
    unsigned char pad379c[0x37A0 - 0x379C];
    int step;                    /* +0x37A0 */
    int wait;                    /* +0x37A4 */
    unsigned char pad37a8[0x37B0 - 0x37A8];
    int flag37b0;                /* +0x37B0 */
    unsigned char pad37b4[0x37D4 - 0x37B4];
    int unk37d4;                 /* +0x37D4 */
    int unk37d8;                 /* +0x37D8 */
    unsigned char pad37dc[0x3808 - 0x37DC];
    int busy3808;                /* +0x3808 */
} TutDuelView;

typedef struct TutReqView {      /* duel_get_14a30 */
    unsigned char pad0[0x808];
    int n808;
    int n80c;
} TutReqView;

extern TutScript tut_script;
extern int tut_pc;               /* 0x088170C0 */
extern int tut_hold_timer;       /* 0x088170C4 */
extern int tut_busy;             /* 0x088170C8 */
extern int tut_act_kind;         /* 0x088170CC expected player action, 0 none */
extern int tut_act_arg;          /* 0x088170D0 */
extern int tut_ev_kind;          /* 0x088170D4 expected duel event, 0 none */
extern int tut_ev_arg;           /* 0x088170D8 */
extern void (*tut_act_reject)(void); /* 0x088170DC */
extern TutWait tut_wait;
extern void (*tut_host)(void);   /* 0x088170F4 libduel_mgr's event sink */
extern int tut_allow;            /* 0x088170F8 */
extern int tut_nowait;           /* 0x088170FC */
extern int tut_answer_flag;      /* 0x08817100 */
extern int tut_hold;             /* 0x08817104 hold the duel events */
extern TutArgs tut_act_saved;    /* 0x08817108 */
extern TutArgs tut_ev_saved;     /* 0x08817118 */
extern unsigned short tut_ev_text[0x400]; /* 0x08817128 */
extern unsigned short tut_answer; /* 0x08817928 */
extern unsigned short tut_watch;  /* 0x0881792A card watched by the $1..$4 variables */
extern int tut_reset_pending;    /* 0x0881792C */

void *duel_get_14a30(void);
TutDuelView *duel_get(void);
int duel_net_cmd();
void duel_08835F34(unsigned int p, int z, int i, int k);
unsigned short duel_zone_card_id(unsigned int p, int z, int i);
int duel_hand_uid(unsigned int p, int i);
int duel_list_id(int i);
int duel_list_card_level(unsigned int p, int i);
unsigned int duel_turn_player_get(void);
int duel_view_player_get(void);
void duel_view_player_set(int p);
int duel_phase_get(void);
void duel_player_type_set(int p, int t);
int duel_setup_224_get(void);
int duel_setup_228_get2(void);
int duel_w624_get(void);
void duel_w624_set(int v);
void duel_host_cb_set(void (*cb)(void));
void duel_host_args_get(int *out);
void duel_start_puzzle(char *text);
void libduel_draw_08805D2C(TutDuelSetup *s);
void libduel_draw_08805EE4(void);
int libduel_draw_08805F88(void);
void libduel_draw_08806030(void);
void libduel_draw_08806278(int on);
void libduel_draw_08806284(int (*cb)(int, int, int, int));
void libduel_draw_0880628C(void);
void libduel_draw_088062DC(int (*cb)(unsigned short));
void libduel_draw_08806360(TutZoneRef *a, TutZoneRef *b);
void libduel_draw_088063F0(void);
void libduel_draw_0880642C(int on);
void libduel_draw_088176A0(int kind, int a, int b, int c);
void gfx_overlay_enable(int on);
void gamestate_load(const void *src);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
int memcmp(const void *a, const void *b, unsigned int n);
int sscanf(const char *s, const char *fmt, ...);
TutFace *tut_face_load(void *heap, unsigned short chara);
int tut_face_ready(TutFace *f);

void tut_script_load(int lesson);
void tut_event_hook(void);
void tut_script_preload_faces(void);
void tut_act_clear(void);
void tut_ev_clear(void);
void tut_set_answer_flag(unsigned short v);
void tut_set_allow(int on);
void tut_replay_event(void);
void tut_hold_events(int on);
int duel_w624_get_thunk(void);
void duel_w624_set_thunk(int v);

void *tut_duel_req(void)
{
    return duel_get_14a30();
}

TutDuelView *tut_duel(void)
{
    return duel_get();
}

void tut_wait_on(int i, int v)
{
    tut_wait.val[i] = v;
    tut_wait.on[i] = 1;
}

static void tut_wait_off(int i)
{
    tut_wait.on[i] = 0;
}

void tut_set_allow(int on)
{
    tut_allow = on;
}

void tut_set_answer(int v)
{
    tut_answer = v;
}

void tut_set_nowait(int on)
{
    tut_nowait = on;
}

void tut_set_answer_flag(unsigned short v)
{
    tut_answer_flag = v;
}

void tut_duel_clear(void)
{
    TutDuelView *d = tut_duel();

    d->unk37d4 = 0;
    d->unk37d8 = 0;
}

void tut_duel_set_turn(int v)
{
    tut_duel()->field_turn = v;
}

/* 100 / 101: the card at one of libduel_draw's two cursor positions */
void tut_watch_card(int code)
{
    TutZoneRef a;
    TutZoneRef b;

    if (code >= 100 && code <= 1000) {
        libduel_draw_08806360(&a, &b);
        tut_watch = 0;
        switch (code) {
        case 100:
            tut_watch = duel_zone_card_id(a.player, a.zone, a.index);
            break;
        case 101:
            tut_watch = duel_zone_card_id(b.player, b.zone, b.index);
            break;
        }
    }
}

unsigned short tut_watch_get(void)
{
    return tut_watch;
}

/* a duel step of the script; 0xC..0x11 also put the engine in the matching wait */
void tut_duel_step(int cmd, int a, int b, int c)
{
    TutDuelView *d = tut_duel();

    switch ((unsigned short)cmd & 0xFFF) {
    case 0xC:
        d->step = 1;
        d->wait = 1;
        tut_duel_clear();
        tut_set_lock(1);
        break;
    case 0xD:
        d->step = 2;
        d->wait = 1;
        tut_duel_clear();
        break;
    case 0xE:
        d->step = 3;
        d->wait = 1;
        tut_duel_clear();
        break;
    case 0xF:
        d->step = 3;
        d->wait = 0x16;
        d->flag37b0 = 1;
        tut_duel_clear();
        break;
    case 0x11:
        d->step = 4;
        d->wait = 1;
        tut_duel_clear();
        tut_set_lock(1);
        break;
    }
    duel_net_cmd(cmd, a, b, c);
}

void tut_act_clear(void)
{
    tut_act_kind = 0;
}

void tut_act_reject_cb(void)
{
    if (tut_act_reject != 0)
        tut_act_reject();
}

void tut_ev_clear(void)
{
    tut_ev_kind = 0;
}

/* libduel_draw asks before carrying out a player action (player, zone, index, kind): 1 = allowed;
   a refused action is kept for tut_replay_action */
int tut_action_filter(int player, int zone, int index, int kind)
{
    TutWait *w = &tut_wait;
    int done;
    int block;
    unsigned short card;
    unsigned short want;
    int hit;
    TutZoneRef a;
    TutZoneRef b;

    if (tut_act_kind == 0)
        return 1;
    done = 0;
    block = 1;
    card = duel_zone_card_id(player, zone, index);
    switch (tut_act_kind) {
    case 1:
        if (kind == 12) {
            done = 1;
            block = 0;
        }
        break;
    case 2:
        if (kind == 4) {
            done = 1;
            block = 0;
        }
        break;
    case 9:
        if (kind == 6) {
            done = 1;
            block = 0;
        }
        break;
    case 6:
        if (kind == 0)
            return 1;
        if (kind == 11)
            done = 1;
        break;
    case 19:
        if (kind == 0)
            return 1;
        if (kind == 11) {
            done = 1;
            block = 0;
        }
        break;
    case 7:
        if (kind == 0)
            done = 1;
        break;
    case 8:
        block = 0;
        break;
    case 14:
        block = 0;
        break;
    case 10:
        done = 0;
        break;
    case 11:
        if (kind == 8) {
            done = 1;
            block = 0;
        }
        break;
    case 12:
        if (kind == 9) {
            done = 1;
            block = 0;
        }
        break;
    case 13:
        if (kind == 5) {
            done = 1;
            block = 0;
        }
        break;
    case 15:
        if (kind == 4 && duel_list_card_level(player, duel_hand_uid(player, index)) >= 5)
            block = 0;
        break;
    case 16:
        if (kind == 11 && (tut_act_arg == 0 || tut_act_arg == card)) {
            done = 1;
            block = 0;
        }
        break;
    case 17:
        if (kind == 3 && (tut_act_arg == 0 || tut_act_arg == card)) {
            done = 1;
            block = 0;
        }
        break;
    case 18:
        if (kind == 7 && (tut_act_arg == 0 || tut_act_arg == card)) {
            done = 1;
            block = 0;
        }
        break;
    case 20:
        if (kind == 3) {
            if (tut_act_arg == 0 || tut_act_arg == card)
                block = 0;
        } else if (kind == 11) {
            done = 1;
            block = 0;
        }
        break;
    case 22:
        if (kind == 11) {
            want = tut_act_arg;
            hit = 0;
            if (want >= 100 && want <= 1000) {
                libduel_draw_08806360(&a, &b);
                if (want == 101 && b.player == player && b.zone == zone && b.index == index)
                    hit = 1;
            } else if (want == 0 || want == card) {
                hit = 1;
            }
            if (hit) {
                done = 1;
                block = 0;
                tut_set_answer_flag(done);
            }
        }
        break;
    }
    if (done) {
        tut_act_clear();
        w->mode = 0;
        tut_wait_off(1);
        tut_set_lock(1);
    } else {
        tut_act_reject_cb();
    }
    if (block) {
        tut_act_saved.kind = player;
        tut_act_saved.arg[0] = zone;
        tut_act_saved.arg[1] = index;
        tut_act_saved.arg[2] = kind;
        return 0;
    }
    return 1;
}

/* duel event sink installed with duel_host_cb_set: events the script waits for are held back and
   replayed later (tut_replay_event), the others go on to libduel_mgr */
#ifdef NON_MATCHING
/* tut_ev_saved = ev copia o struct da pilha por lwc1/swc1 com outra ordem de lw/lui nos dois
   pontos de cópia (15 palavras); nenhuma forma de cópia testada bate */
void tut_event_hook(void)
{
    TutWait *w = &tut_wait;
    int held = 0;
    TutArgs ev;

    duel_host_args_get(&ev.kind);
    switch (ev.kind) {
    case 0x31:
        switch (ev.arg[0]) {
        case 4:
            if (w->on[1] != 0) {
                if (tut_act_kind == 4)
                    tut_act_clear();
                w->mode = 1;
            }
            break;
        case 1:
            switch (tut_act_kind) {
            case 16:
                duel_w624_set(1);
                held = 1;
                break;
            case 15:
                if (ev.arg[1] == 1)
                    w->mode = 3;
                break;
            }
            break;
        case 2:
            if (tut_answer) {
                switch (tut_answer) {
                case 1:
                    duel_w624_set(0);
                    break;
                case 2:
                    duel_w624_set(1);
                    break;
                }
                held = 1;
            }
            if (tut_act_kind == 17) {
                duel_w624_set(1);
                held = 1;
            }
            if (tut_ev_kind == 4) {
                tut_ev_saved = ev;
                if (ev.arg[1] >= 0x1000) {
                    wcscpy(tut_ev_text, (unsigned short *)ev.arg[1]);
                    tut_ev_saved.arg[1] = (int)tut_ev_text;
                }
                tut_ev_clear();
            }
            break;
        }
        break;
    case 1:
        if (tut_hold != 0)
            held = 1;
        break;
    case 4:
        if (tut_hold != 0)
            held = 1;
        if (tut_ev_kind == 2 && ev.arg[1] == tut_ev_arg) {
            tut_set_answer_flag(1);
            tut_ev_clear();
        }
        break;
    case 3:
        if (tut_act_kind == 0 && duel_turn_player_get() != duel_view_player_get())
            held = 1;
        if (tut_ev_kind == 5)
            tut_ev_clear();
        break;
    case 0x20:
        if (tut_ev_kind == 1) {
            unsigned short id = ev.arg[1];

            if (tut_ev_arg == 0 || tut_ev_arg == id) {
                tut_set_answer_flag(1);
                tut_ev_clear();
            }
        }
        break;
    case 0x33:
        if (tut_ev_kind == 3) {
            unsigned short id = ev.arg[1];

            if (tut_ev_arg == 0 || tut_ev_arg == id) {
                tut_set_answer_flag(1);
                tut_ev_clear();
            }
        }
        break;
    }
    if (held == 0) {
        if (ev.kind == 0x14 && tut_reset_pending == 1) {
            libduel_draw_0880628C();
            tut_reset_pending = 0;
        }
        tut_host();
    } else {
        tut_ev_saved = ev;
    }
}
#endif

/* libduel_draw asks before a card is picked from the hand (code = player | zone << 1 | index << 6) */
int tut_card_filter(unsigned short code)
{
    union {
        unsigned short raw;
        TutZoneRef z;
    } u;

    u.raw = code;
    if (tut_act_kind == 10) {
        if (u.z.player == 0 && u.z.zone == 11) {
            unsigned short id = duel_list_id(duel_hand_uid(u.z.player, u.z.index));

            if (tut_act_arg == 0 || tut_act_arg == id) {
                tut_act_clear();
                tut_set_lock(1);
            }
        }
        return 0;
    }
    return 0;
}

/* answers of the duel's yes/no questions while the script waits */
void tut_wait_update(void)
{
    TutWait *const w = &tut_wait;
    int r;

    pad_get();
    switch (w->mode) {
    case 0:
        break;
    case 1:
        if (req_flag_get(0x31) == 0) {
            r = duel_w624_get_thunk();
            if (w->val[1] != r) {
                duel_w624_set_thunk(-1);
                tut_act_reject_cb();
            } else if (tut_act_kind == 3 || tut_act_kind == 5) {
                tut_wait_off(1);
                tut_set_allow(0);
                tut_act_clear();
            }
            w->mode = 0;
        }
        break;
    case 2:
        break;
    case 3:
        if (req_flag_get(0x31) == 0) {
            if (duel_w624_get_thunk() == 0)
                tut_act_reject_cb();
            else
                tut_act_clear();
            tut_set_allow(0);
            w->mode = 0;
        }
        break;
    case 4:
        if (req_flag_get(0x31) == 0) {
            if (duel_w624_get_thunk() == 0) {
                duel_w624_set_thunk(-1);
                tut_replay_event();
            } else {
                w->mode = 0;
                tut_act_clear();
                tut_set_allow(0);
                w->mode = 0;
            }
        }
        break;
    }
}

/* actions that are checked every frame instead of through the filter */
void tut_act_poll(void)
{
    switch (tut_act_kind) {
    case 8:
        if (duel_setup_228_get2()) {
            if (duel_setup_224_get() == 1)
                duel_w624_set_thunk(1);
            else
                duel_w624_set_thunk(0);
            tut_act_clear();
        }
        break;
    case 14:
        if (duel_setup_228_get2() || (duel_turn_player_get() == 0 && duel_phase_get() == 5)) {
            tut_act_clear();
            if (tut_act_arg != 0)
                tut_set_answer_flag(1);
        }
        break;
    }
}

/* next line of *pp into buf (n bytes), 0 at the end or when the line does not fit */
int tut_read_line(char *buf, int n, char **pp)
{
    char *p = *pp;
    int i = 0;

    while (*p != 0) {
        buf[i] = *p;
        i++;
        p++;
        if (i >= n - 1)
            return 0;
        if (p[-1] == '\n')
            break;
    }
    buf[i] = 0;
    *pp = p;
    return i;
}

void tut_setup_add(TutDuelSetup *s, int player, int id)
{
    TutDeckSetup *d = s->deck[player];

    d->ids[d->n] = id;
    d->n++;
}

void tut_setup_add_extra(TutDuelSetup *s, int player, int id)
{
    TutDeckSetup *d = s->deck[player];

    d->extra[d->n_extra] = id;
    d->n_extra++;
}

/* one line of %02d_field_set.txt: CardInGr<p>..=<id>, CardInGrave, CardInE<p>..., CardInF<p>...,
   CardInD<p>..., CardInH<p>...; 1 at [END] */
int tut_setup_line(char *s, TutDuelSetup *setup)
{
    int p;
    int h;
    int d;
    int f;
    int e;
    int gr;
    int g0;
    int g;

    switch (*s) {
    case '[':
        if (memcmp(s, "[END]", 5) == 0)
            return 1;
        break;
    case 'C':
        if (memcmp(s, "CardIn", 6) == 0) {
            switch (s[6]) {
            case 'H':
                p = s[10] - '0';
                if (s[11] != '=') {
                    sscanf(s + 0x10, "%d", &h);
                    tut_setup_add(setup, p, h);
                }
                break;
            case 'D':
                p = s[10] - '0';
                if (s[11] != '=') {
                    sscanf(s + 0x10, "%d", &d);
                    tut_setup_add(setup, p, d);
                }
                break;
            case 'F':
                p = s[12] - '0';
                if (s[13] != '=') {
                    sscanf(s + 0x12, "%d", &f);
                    tut_setup_add_extra(setup, p, f);
                }
                break;
            case 'E':
                p = s[13] - '0';
                if (s[14] != '=') {
                    sscanf(s + 0x13, "%d", &e);
                    tut_setup_add(setup, p, e);
                }
                break;
            case 'G':
                if (s[7] == 'r') {
                    p = s[11] - '0';
                    if (s[12] != '=') {
                        sscanf(s + 0x11, "%d", &gr);
                        tut_setup_add(setup, p, gr);
                    }
                } else {
                    sscanf(s + 0xC, "%d", &g0);
                    if (memcmp(s + 0xF, "=", 1) == 0) {
                        sscanf(s + 0x10, "%d", &g);
                        tut_setup_add(setup, 0, g);
                    }
                }
                break;
            }
        }
        break;
    }
    return 0;
}

void tut_setup_parse(char *text, TutDuelSetup *setup)
{
    char line[0x100];

    while (tut_read_line(line, 0x100, &text) != 0) {
        if (tut_setup_line(line, setup) != 0)
            break;
    }
}

/* the tutorial duel: player and opponent slots, decks from the field set-up, the libduel_draw
   callbacks; start() is libduel_mgr's, host its event sink */
void tut_duel_start(void (*start)(void), void (*host)(void), int lesson)
{
    GameState *gs = game_state_get();
    GameState g;
    TutDuelSetup setup;
    TutDeckSetup decks[2];
    char *text;
    int i;

    tut_host = host;
    memset(&g, 0, 0x740);
    g.pad11b[0] = gs->pad11b[0];
    g.pad10e[10] = 2;
    g.pad10e[11] = 1;
    g.partner_mode = 1;
    g.pad11b[4] = 0;
    for (i = 0; i < 4; i++) {
        g.slot[i].type = 1;
        g.slot[i].type2 = 0;
        g.slot[i].deck_id = 1;
        if (i == 1)
            g.slot[i].type = tutbox.faces[0].chara;
    }
    gamestate_load(&g);
    cfg_set_flag(0, 0);
    cfg_set_flag(2, 0);
    memset(&setup, 0, 0x40);
    memset(decks, 0, 0x210);
    setup.mode = 2;
    setup.deck[0] = &decks[0];
    setup.deck[1] = &decks[1];
    setup.unk3c = 0;
    text = tut_field_set(lesson);
    tut_setup_parse(text, &setup);
    tut_release(text);
    setup.unk04 = 1;
    setup.unk38 = 1;
    setup.unk34 = 0;
    setup.partner = tutbox.faces[0].chara;
    libduel_draw_08805D2C(&setup);
    libduel_draw_08805EE4();
    while (!libduel_draw_08805F88())
        sys_frame_wait();
    libduel_draw_08806284(tut_action_filter);
    libduel_draw_088062DC(tut_card_filter);
    libduel_draw_08806278(1);
    libduel_draw_0880642C(1);
    libduel_draw_08806030();
    gfx_overlay_enable(0);
    duel_host_cb_set(tut_event_hook);
    start();
    tut_hold_events(1);
    duel_start_puzzle(tut_field_set(lesson));
    tut_duel_set_turn(0);
    duel_player_type_set(0, 0);
    duel_player_type_set(1, 3);
    duel_view_player_set(0);
    tut_set_lock(1);
}

void tut_script_load(int lesson)
{
    unsigned short *p = tut_duel_commands(lesson);
    TutScript *s = &tut_script;
    int i;

    tut_script.n = *p;
    p++;
    for (i = 0; i < s->n; i++) {
        s->step[i].cmd = p[0];
        s->step[i].a = p[1];
        s->step[i].b = p[2];
        s->step[i].c = p[3];
        p += 4;
    }
    tut_pc = 0;
}

/* jump to the label step (0x63, id) */
void tut_script_jump(int id)
{
    int i;
    TutStep *st;

    for (i = 0; i < tut_script.n; i++) {
        st = &tut_script.step[i];
        if (st->cmd == 0x63 && st->a == id) {
            tut_pc = i;
            return;
        }
    }
    tut_pc = 0;
}

int tut_script_step(void)
{
    TutStep *st;

    if (tut_pc >= tut_script.n)
        return 0;
    st = &tut_script.step[tut_pc];
    switch (st->cmd & 0xFFF) {
    case 0x64:
        tut_script_jump(st->a);
        return 1;
    case 0x65:
        if (st->b > 1000 && st->b == tut_watch) {
            tut_script_jump(st->a);
            return 1;
        }
        if (st->b == duel_w624_get_thunk())
            tut_script_jump(st->a);
        else
            tut_pc++;
        return 1;
    case 0x67:
        tut_command((st->cmd & 0x8000) != 0, st->a, st->b, st->c);
        tut_pc++;
        return 1;
    default:
        tut_duel_step(st->cmd, st->a, st->b, st->c);
        tut_pc++;
        return 1;
    }
}

void tut_script_tick(void)
{
    if (tut_script_busy() == 0 && tut_get_state() == 0)
        tut_script_step();
}

/* busy while a duel request is pending or the duel is animating */
void tut_script_check_busy(void)
{
    int i;
    TutReqView *r;
    TutDuelView *d;

    if (tut_nowait != 0) {
        tut_busy = 0;
        return;
    }
    for (i = 0; i <= 0x4A; i++) {
        if ((i != 0x31 || tut_allow == 0) && req_flag_get(i) != 0) {
            tut_hold_timer = 5;
            tut_busy = 1;
            return;
        }
    }
    r = tut_duel_req();
    d = tut_duel();
    if (duel_setup_228_get2() == 0 &&
        (d->busy3808 != 0 || d->busy3798 != 0 || r->n80c != 0 || r->n808 != 0)) {
        tut_hold_timer = 5;
        tut_busy = 1;
    } else if (tut_hold_timer > 0) {
        tut_busy = 1;
    } else {
        tut_busy = 0;
    }
}

void tut_script_init(int lesson, void (*start)(void), void (*host)(void))
{
    memset(&tut_script, 0, 0x4874);
    tut_act_kind = 0;
    tut_script_load(lesson);
    tut_script_preload_faces();
    tut_duel_start(start, host, lesson);
    tut_wait_on(1, -1);
}

void tut_duel_draw_term(void)
{
    libduel_draw_08805E90();
}

void tut_script_update(void)
{
    tut_script_check_busy();
    if (tut_answer_flag != 0)
        req_flag_set(0);
    tut_script_tick();
    tut_wait_update();
    tut_act_poll();
    if (tut_hold_timer > 0)
        tut_hold_timer--;
}

void duel_w624_set_thunk(int v)
{
    duel_w624_set(v);
}

int duel_w624_get_thunk(void)
{
    return duel_w624_get();
}

int tut_script_busy(void)
{
    return tut_busy;
}

int tut_script_done(void)
{
    if (tut_pc >= tut_script.n && tut_script_busy() == 0 && tut_is_busy() == 0)
        return 1;
    return 0;
}

void tut_duel_pause(void)
{
    libduel_draw_08806278(0);
}

void tut_duel_resume(void)
{
    libduel_draw_08806278(1);
}

/* the player may act until the action kind (with argument arg) is done */
void tut_expect_action(int kind, int arg)
{
    TutWait *w = &tut_wait;
    TutDuelView *d;

    tut_set_answer_flag(0);
    tut_act_kind = kind;
    tut_act_arg = arg;
    tut_act_reject = 0;
    switch (kind) {
    case 1:
        w->mode = 2;
        tut_set_lock(0);
        return;
    case 6:
    case 7:
    case 10:
    case 17:
    case 18:
    case 19:
    case 20:
        tut_wait_on(1, -1);
        break;
    case 2:
    case 9:
    case 16:
        tut_wait_on(1, -1);
        tut_set_lock(0);
        break;
    case 3:
        tut_wait_on(1, 5);
        break;
    case 4:
        tut_wait_on(1, -1);
        tut_set_allow(1);
        break;
    case 5:
        tut_wait_on(1, 3);
        break;
    case 8:
        d = tut_duel();
        tut_wait_off(1);
        duel_player_type_set(1, 1);
        d->wait = 0;
        tut_duel_clear();
        break;
    case 15:
        tut_wait_on(1, -1);
        tut_set_allow(1);
        break;
    case 11:
    case 12:
    case 13:
        tut_wait_on(1, -1);
        tut_set_lock(0);
        break;
    case 21:
        w->mode = 4;
        tut_set_lock(0);
        tut_set_answer_flag(1);
        break;
    }
}

int tut_action_done(void)
{
    return tut_act_kind == 0;
}

void tut_replay_action(void)
{
    TutArgs *a = (TutArgs *)(char *)&tut_act_saved;

    duel_08835F34(a->kind, a->arg[0], a->arg[1], a->arg[2]);
}

void tut_replay_event(void)
{
    TutArgs *e = (TutArgs *)(char *)&tut_ev_saved;

    libduel_draw_088176A0(e->kind, e->arg[0], e->arg[1], e->arg[2]);
}

void tut_expect_event(int kind, int arg)
{
    tut_ev_kind = kind;
    tut_ev_arg = arg;
    tut_set_answer_flag(0);
}

int tut_event_done(void)
{
    return tut_ev_kind == 0;
}

void tut_hold_events(int on)
{
    tut_hold = on;
}

/* restart the duel from field_reset_<file>.txt at the given turn */
void tut_field_reset(int file, int turn)
{
    char *text;

    tut_hold_events(1);
    libduel_draw_08806278(1);
    libduel_draw_0880642C(1);
    text = tut_field_reset_file(file);
    tut_reset_pending = 1;
    duel_start_puzzle(text);
    tut_duel_set_turn(turn);
    duel_player_type_set(0, 0);
    duel_player_type_set(1, 3);
    libduel_draw_088063F0();
    tut_set_state(4);
    tut_set_lock(1);
}

/* the leading face steps (0x67 command 0x17, character * 100) load the faces before the duel */
#ifdef NON_MATCHING
/* laço com flag done: o original guarda &tutbox em $s0 e testa done com sltu/xori no fim;
   aqui os $s trocam (15 palavras) em todas as formas de laço testadas */
void tut_script_preload_faces(void)
{
    int done;
    int i;
    TutStep *st;
    int c;
    TutFace **pf;

    for (done = 0, i = 0; !done; tut_pc++) {
        st = &tut_script.step[tut_pc];
        if ((st->cmd & 0xFFF) != 0x67)
            return;
        if (st->a != 0x17)
            return;
        pf = &tutbox.faces[i].face;
        c = st->b / 100;
        tutbox.faces[i].chara = c;
        *pf = tut_face_load(tut_heap_get(), c);
        if (tut_face_ready(*pf))
            sys_frame_wait();
        i++;
        if (i >= 8)
            done = 1;
    }
}
#endif
