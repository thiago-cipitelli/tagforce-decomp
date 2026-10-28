// @range 0x0880DE58-0x0880EC38
// @asm 0x0880DE58-0x0880E634
// @asm 0x0880E850-0x0880EB1C
// @data 0x0881D330-0x0881D608
/* Field setup commands of the story (second half): the characters standing in the current room (with
   the scripted walks of the cafeteria/classroom regulars and the crowd of the special days), the
   partner, the story events and pickups of the room, the night flag, the room links, and the fade
   in/out of the field (libfield commands 8, 0xB, 6, 7, 1, 2, 0x1F and 0x20). */
#include "libstory.h"

int st_pl_room_get(void);
int st_pl_x14_get(void);
int st_pl_x18_get(void);
int st_chr_place(int chr);
int st_id_to_item(int chr);
int st_id_to_deck(int id);
int st_chr_model(int chr);
int st_chr_costume(int chr);
int st_chr_x(int chr);
int st_chr_y(int chr);
int st_chr_x_b(int chr);
int st_chr_y_b(int chr);
int st_chr_dir(int chr);
int st_chr_dir_b(int chr);
int st_chr_flag_a(int chr);
int st_chr_flag_b(int chr);
int st_chr_is_away(int chr);
unsigned short *st_chr_text_a(int chr);
unsigned short *st_chr_text_b(int chr);
void st_chr_set_pos(int chr, int x, int y, int dir);
int st_req_is_night(void);
int st_sv_alt(void);
int st_sv_crowd(void);
int st_room_to_bg(int room);
int st_room_variant(void);
int scal_flag2_get(int bit);
int story_mode_is1(void);
int sdate_check(int a);
int sev_collect(int place, void *out);
int st_pickup_list(int place, void *out);
int st_req_get_8(void);
void st_req_set_8(int v);
int scal_period_now(void);
void st_req_set_exit(int v);
void st_fld_on_link(void);
void st_fld_on_back(int v);

typedef struct StRouteNode {           /* step of a scripted walk, 0x18 */
    int x;
    int y;
    int a, b, c, d;
} StRouteNode;

typedef struct StNpc {                 /* character placed by fld command 8, 0x40 */
    int item;                          /* +0x00 */
    int model;                         /* +0x04 */
    int costume;                       /* +0x08 */
    int x;                             /* +0x0C */
    int y;                             /* +0x10 */
    int x2;                            /* +0x14 */
    int y2;                            /* +0x18 */
    int visible;                       /* +0x1C */
    int crowd;                         /* +0x20 extra of the crowd */
    int flag_a;                        /* +0x24 */
    int dir;                           /* +0x28 */
    unsigned short *text_a;            /* +0x2C */
    unsigned short *text_b;            /* +0x30 */
    int flag_b;                        /* +0x34 */
    StRouteNode *route;                /* +0x38 */
    int nroute;                        /* +0x3C */
} StNpc;

typedef struct StNpcArg {              /* argument of fld command 8 */
    StNpc *list;
    int n;
    int target;                        /* character to face (deck id), -1 none */
    int target_dir;
} StNpcArg;

typedef struct StCrowdSpot {           /* 0x0881D4D8 spot of the crowd, 0x10 */
    int x;
    int y;
    int n;                             /* characters in a row from (x, y) */
    int dir;
} StCrowdSpot;

typedef struct StRoomArg {             /* argument of fld command 2 */
    int (*links)[2];                   /* item and bg of each character */
    int nlinks;
    int bg;
    int *doors;
    int ndoors;
    int variant;
    void (*on_link)(void);
    void (*on_back)(int v);
} StRoomArg;

/* places of the crowd of the festival (place 2): 53 cells */
int st_crowd_cells[53][2] = {
    { 0x1E, 0x19 }, { 0x1C, 0x19 }, { 0x1A, 0x19 }, { 0x11, 0x19 }, { 0x0F, 0x19 }, { 0x04, 0x19 },
    { 0x02, 0x19 }, { 0x1E, 0x16 }, { 0x1C, 0x16 }, { 0x1A, 0x16 }, { 0x11, 0x16 }, { 0x0F, 0x16 },
    { 0x06, 0x16 }, { 0x04, 0x16 }, { 0x02, 0x16 }, { 0x1E, 0x13 }, { 0x1C, 0x13 }, { 0x1A, 0x13 },
    { 0x11, 0x13 }, { 0x0F, 0x13 }, { 0x06, 0x13 }, { 0x04, 0x13 }, { 0x02, 0x13 }, { 0x1E, 0x10 },
    { 0x1C, 0x10 }, { 0x1A, 0x10 }, { 0x06, 0x10 }, { 0x04, 0x10 }, { 0x02, 0x10 }, { 0x1E, 0x0D },
    { 0x1C, 0x0D }, { 0x1A, 0x0D }, { 0x11, 0x0D }, { 0x0F, 0x0D }, { 0x06, 0x0D }, { 0x04, 0x0D },
    { 0x02, 0x0D }, { 0x1E, 0x0A }, { 0x1C, 0x0A }, { 0x1A, 0x0A }, { 0x11, 0x0A }, { 0x0F, 0x0A },
    { 0x06, 0x0A }, { 0x04, 0x0A }, { 0x02, 0x0A }, { 0x1E, 0x07 }, { 0x1C, 0x07 }, { 0x1A, 0x07 },
    { 0x11, 0x07 }, { 0x0F, 0x07 }, { 0x06, 0x07 }, { 0x04, 0x07 }, { 0x02, 0x07 },
};

/* rows of the crowd of the tournament (place 4), ended by x = y = 0 */
StCrowdSpot st_crowd_spots[19] = {
    { 0x2B, 0x10, 2, 3 }, { 0x2B, 0x20, 4, 3 }, { 0x10, 0x2B, 0x12, 5 }, { 0x08, 0x12, 0x10, 7 },
    { 0x12, 0x08, 0x10, 1 }, { 0x2D, 0x0F, 3, 3 }, { 0x2D, 0x20, 6, 3 }, { 0x0F, 0x2D, 0x17, 5 },
    { 0x06, 0x0E, 0x16, 7 }, { 0x10, 0x06, 0x12, 1 }, { 0x2F, 0x0E, 4, 3 }, { 0x2F, 0x20, 7, 3 },
    { 0x0D, 0x2F, 0x1A, 5 }, { 0x04, 0x0C, 0x1B, 7 }, { 0x0C, 0x04, 0x1B, 1 }, { 0x31, 0x0A, 8, 3 },
    { 0x31, 0x20, 0x0A, 3 }, { 0x0D, 0x31, 0x1D, 5 }, { 0, 0, 0, 0 },
};

int st_crowd_pick_name(int *name);

#ifdef NON_MATCHING /* forma igual; ordem dos stores das rotas, pilha 16 bytes menor e registradores dos laços diferem */
/* places the characters of the current room (fld command 8): the story characters standing here,
   the scripted walks of the regulars of the classroom (3), the cafeteria (5) and the dorm (25), and on
   crowd days the extras of the festival (place 2) or of the tournament rows (place 4) */
int st_fld_place_chrs(void)
{
    StNpc list[150];
    StRouteNode route_b[4];
    StRouteNode route_a[4];
    StNpcArg arg;
    int res;
    int last_name;
    int perm[3];
    int i;
    StNpc *e;
    int n;
    int place;
    int partner;
    int at;
    int x, y;
    int seq;
    StCrowdSpot *spot;
    int j;
    int dir;
    int r;

    last_name = -1000;
    n = -1;
    place = st_pl_room_get();
    partner = st_pl_x14_get();
    e = list - 1;
    for (i = 0; i < 150; i++) {
        at = st_chr_place(i);
        if (place != at || at == 0 || partner == i)
            continue;
        e++;
        n++;
        e->item = st_id_to_item(i);
        e->model = st_chr_model(i);
        e->costume = st_chr_costume(i);
        e->x = st_chr_x(i);
        e->y = st_chr_y(i);
        if (st_req_is_night()) {
            e->x2 = st_chr_x_b(i);
            e->y2 = st_chr_y_b(i);
            e->dir = st_chr_dir_b(i);
        } else {
            e->x2 = st_chr_x(i);
            e->y2 = st_chr_y(i);
            e->dir = st_chr_dir(i);
            st_chr_set_pos(i, st_chr_x(i), st_chr_y(i), st_chr_dir(i));
        }
        e->flag_a = st_chr_flag_a(i);
        e->flag_b = st_chr_flag_b(i);
        e->visible = !st_chr_is_away(i);
        e->crowd = 0;
        e->text_a = st_chr_text_a(i);
        e->text_b = st_chr_text_b(i);
        e->route = 0;
        e->nroute = 0;
        switch (at) {
        case 3:
            if (e->x == 10000 && e->y == 10000) {
                route_a[0].x = 0x16;
                route_a[0].a = 1;
                route_a[0].b = 1;
                route_a[0].c = 1;
                route_a[0].d = 1;
                route_a[1].a = 1;
                route_a[1].b = 1;
                route_a[1].c = 1;
                route_a[1].d = 1;
                route_a[1].x = 0x15;
                route_a[1].y = 0xD;
                e->x = 0x13;
                e->y = 0xC;
                e->x2 = 0x13;
                e->y2 = 0xC;
                e->dir = 7;
                e->flag_a = 0;
                e->route = route_a;
                route_a[0].y = 0xC;
                e->nroute = 2;
            }
            break;
        case 5:
            if (e->x == 10000 && e->y == 10000) {
                e->x = 5;
                e->y = 10;
                e->x2 = 5;
                route_a[0].y = 10;
                route_a[3].y = 10;
                e->y2 = 10;
                e->dir = 7;
                route_a[0].a = 1;
                route_a[0].b = 1;
                route_a[0].c = 1;
                route_a[0].d = 1;
                route_a[1].a = 1;
                route_a[1].b = 1;
                route_a[1].c = 1;
                route_a[1].d = 1;
                route_a[2].a = 1;
                route_a[2].b = 1;
                route_a[2].c = 1;
                route_a[2].d = 1;
                route_a[3].a = 1;
                route_a[3].b = 1;
                route_a[3].c = 1;
                route_a[3].d = 1;
                route_a[1].y = 0xB;
                route_a[2].y = 9;
                route_a[0].x = 7;
                route_a[1].x = 7;
                e->flag_a = 0;
                e->route = route_a;
                route_a[2].x = 4;
                route_a[3].x = 4;
                e->nroute = 4;
            }
            break;
        case 25:
            if (e->x == 10000 && e->y == 10000) {
                route_a[0].x = 0xC;
                route_a[1].x = 0xC;
                route_a[2].y = 0xC;
                route_a[0].y = 10;
                route_a[0].a = 1;
                route_a[0].b = 1;
                route_a[0].c = 1;
                route_a[0].d = 1;
                route_a[1].a = 1;
                route_a[1].b = 1;
                route_a[1].c = 1;
                route_a[1].d = 1;
                route_a[2].a = 1;
                route_a[2].b = 1;
                route_a[2].c = 1;
                route_a[2].d = 1;
                e->x = 0xB;
                e->y = 0xB;
                e->x2 = 0xB;
                route_a[1].y = 0xB;
                e->y2 = 0xB;
                route_a[2].x = 0xD;
                e->dir = 7;
                e->flag_a = 0;
                e->route = route_a;
                e->nroute = 2;
            }
            if (e->x == 10001 && e->y == 10001) {
                e->x = 0x1B;
                e->y = 1;
                e->x2 = 0x1B;
                e->y2 = 1;
                e->dir = 1;
                route_b[0].a = 1;
                route_b[0].b = 1;
                route_b[0].c = 1;
                route_b[0].d = 1;
                route_b[1].a = 1;
                route_b[1].b = 1;
                route_b[1].c = 1;
                route_b[1].d = 1;
                route_b[2].a = 1;
                route_b[2].b = 1;
                route_b[2].c = 1;
                route_b[2].d = 1;
                route_b[1].x = 0x1C;
                route_b[2].x = 0x1D;
                e->flag_a = 0;
                e->route = route_b;
                route_b[0].x = 0x1B;
                route_b[0].y = 2;
                route_b[1].y = 2;
                route_b[2].y = 2;
                e->nroute = 2;
            }
            break;
        default:
            if (e->x >= 10000 && e->y >= 10000) {
                e->route = 0;
                e->nroute = 0;
            }
            break;
        }
    }
    if (st_sv_alt()) {
        switch (st_sv_crowd()) {
        case 0:
            break;
        case 1:
            switch (place) {
            case 2:
                if (n < 53) {
                    e = &list[n];
                    for (i = 0; i < 53; i++) {
                        e++;
                        e->item = 0x68;
                        n++;
                        e->model = st_crowd_pick_name(&last_name);
                        e->x = st_crowd_cells[i][0];
                        e->y = st_crowd_cells[i][1];
                        e->visible = 1;
                        e->crowd = 1;
                        e->dir = 1;
                    }
                }
                break;
            case 4:
                seq = -1;
                for (spot = st_crowd_spots;; spot++) {
                    e = &list[n];
                    for (j = 0; j < spot->n; j++) {
                        if (n + 1 >= 60)
                            goto done;
                        if (spot->x == 0 && spot->y == 0)
                            goto done;
                        seq = (seq + 1) % 3;
                        if (seq == 0) {
                            r = (int)rand_next() % 3;
                            perm[0] = r % 3;
                            perm[1] = (r + 1) % 3;
                            perm[2] = (r + 2) % 3;
                        }
                        if (perm[seq] != 0)
                            continue;
                        y = spot->y;
                        dir = spot->dir;
                        x = spot->x;
                        switch (dir) {
                        case 5:
                        case 1:
                            x += j;
                            break;
                        case 7:
                        case 3:
                            y += j;
                            break;
                        }
                        e++;
                        e->item = 0x68;
                        n++;
                        e->model = st_crowd_pick_name(&last_name);
                        e->x = x;
                        e->y = y;
                        e->visible = 1;
                        e->crowd = 1;
                        e->dir = dir;
                    }
                }
            }
            break;
        }
    }
done:
    arg.n = n + 1;
    arg.list = list;
    if ((arg.target = st_pl_x18_get()) > 0) {
        i = st_id_to_deck(arg.target);
        if (st_chr_flag_a(i)) {
            arg.target = -1;
            arg.target_dir = 1;
        } else {
            arg.target_dir = st_chr_dir(i);
        }
    }
    if (arg.n <= 0)
        return 0;
    fld_cmd_start(8, &arg);
    while (!fld_cmd_poll(8, &res))
        sys_frame_wait();
    return 1;
}

#endif

/* shows the partner standing next to the player (fld command 0xB) */
int st_fld_partner(int chr)
{
    int arg[2];
    int res;

    arg[0] = st_id_to_item(chr);
    arg[1] = st_chr_model(chr);
    if (fld_cmd_start(0xB, arg) == 0)
        return 0;
    while (!fld_cmd_poll(0xB, &res))
        sys_frame_wait();
    return 1;
}

/* places the story events of the room (fld command 6) */
int st_fld_events(void)
{
    char buf[0x60];
    struct {
        void *list;
        int n;
    } arg;
    int res;
    int n;

    if ((n = sev_collect(st_pl_room_get(), buf)) <= 0)
        return 0;
    arg.n = n;
    arg.list = buf;
    fld_cmd_start(6, &arg);
    while (!fld_cmd_poll(6, &res))
        sys_frame_wait();
    return 1;
}

/* places the pickups of the room (fld command 7) */
int st_fld_pickups(void)
{
    char buf[0x180];
    struct {
        void *list;
        int n;
    } arg;
    int res;
    int n;

    if ((n = st_pickup_list(st_pl_room_get(), buf)) <= 0)
        return 0;
    arg.n = n;
    arg.list = buf;
    fld_cmd_start(7, &arg);
    while (!fld_cmd_poll(7, &res))
        sys_frame_wait();
    return 1;
}

/* tells the field whether it is night (fld command 1) */
void st_fld_night(void)
{
    int arg;
    int res;

    arg = st_sv_alt();
    fld_cmd_start(1, &arg);
    while (!fld_cmd_poll(1, &res))
        sys_frame_wait();
}

#ifdef NON_MATCHING /* só o primeiro laço de busca troca v1/a0 (índice e ponteiro) */
/* room links: where each character can be met and the doors open today (fld command 2) */
void st_fld_links(void)
{
    StRoomArg arg;
    int res;
    int links[9][2];
    int doors[20];
    int partner;
    int place;
    int i;

    partner = st_pl_x14_get();
    for (i = 0; i < 9; i++) {
        place = st_chr_place(i);
        links[i][0] = st_id_to_item(i);
        if (st_chr_is_away(i) || partner == i || place == 0)
            links[i][1] = -1;
        else
            links[i][1] = st_room_to_bg(place);
    }
    arg.bg = st_room_to_bg(st_pl_room_get());
    arg.on_link = st_fld_on_link;
    arg.on_back = st_fld_on_back;
    arg.links = links;
    arg.nlinks = 9;
    arg.variant = st_room_variant();
    memset(doors, 0, sizeof(doors));
    arg.doors = doors;
    arg.ndoors = 0;
    if (scal_flag2_get(1) == 0) {
        for (i = 0; i < 20; i++) {
            if (doors[i] == 0) {
                doors[i] = 0x20;
                arg.ndoors++;
                break;
            }
        }
    }
    if (story_mode_is1()) {
        for (i = 0; i < 20; i++) {
            if (doors[i] == 0) {
                doors[i] = 0x24;
                arg.ndoors++;
                break;
            }
        }
    }
    if (sdate_check(0) == 1) {
        for (i = 0; i < 20; i++) {
            if (doors[i] == 0) {
                doors[i] = 1;
                doors[i + 1] = 3;
                doors[i + 2] = 4;
                doors[i + 3] = 0xC;
                doors[i + 4] = 0x16;
                doors[i + 5] = 0x17;
                doors[i + 6] = 0x18;
                doors[i + 7] = 0x19;
                doors[i + 8] = 0x1A;
                doors[i + 9] = 0x1B;
                doors[i + 10] = 0x1D;
                doors[i + 11] = 0x1E;
                doors[i + 12] = 0x1F;
                doors[i + 14] = 0x24;
                doors[i + 13] = 0x24;
                arg.ndoors += 15;
                break;
            }
        }
    }
    if (arg.ndoors > 20)
        arg.ndoors = 20;
    fld_cmd_start(2, &arg);
    while (!fld_cmd_poll(2, &res))
        sys_frame_wait();
}

#endif

/* starts the fade-in of the field (fld command 0x1F), with the pending event when on */
void st_fld_fade_in(int on)
{
    struct {
        int ev;
        int target;
    } arg;
    int ev;

    if (on != 0) {
        arg.ev = st_req_get_8();
        st_req_set_8(0);
        if ((ev = scal_period_now()) == 0)
            arg.target = -1;
        else
            arg.target = ev;
    } else {
        arg.ev = 0;
        arg.target = -1;
    }
    fld_cmd_start(0x1F, &arg);
}

int st_fld_fade_in_done(void)
{
    int res;

    return fld_cmd_poll(0x1F, &res) != 0;
}

void st_fld_fade_out(void)
{
    int arg;

    fld_cmd_start(0x20, &arg);
}

int st_fld_fade_out_done(void)
{
    int res;

    return fld_cmd_poll(0x20, &res) != 0;
}

void st_fld_fade_out_wait(void)
{
    st_fld_fade_out();
    while (!st_fld_fade_out_done())
        sys_frame_wait();
}
