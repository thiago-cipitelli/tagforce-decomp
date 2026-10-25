// @range 0x088046B4-0x08805034
// @rodata 0x0880F6E0-0x0880F760
/* Script commands of a lesson (tut_command, called by the duel engine for each 0x67 step): message
   windows with a face, notes and boards, the two kinds of pointing arrows, screen highlights and the
   waits for the player's action or a duel event. cmd = kind | sub << 8; a, b are the arguments. */
#include "libtutorial.h"

extern int tut_arrow_a;          /* 0x08810104 arrow ids picked by the *_set commands */
extern int tut_arrow_b;          /* 0x08810108 */
extern int tut_arrow_dir;        /* 0x0881010C */

void tut_window_open(int slot, unsigned short face, unsigned short x, unsigned short y, unsigned char pic);
void tut_window_close(int slot);
void tut_window_set_text(int slot, unsigned short *text);
unsigned short *tut_msg_get(int id);
void tut_marker_show(int i, int on);
void tut_marker_place(int i, int x, int y, int dir);
void tut_expect_action(int kind, int arg);
void tut_expect_event(int kind, int arg);
void tut_duel_pause(void);
void tut_replay_action(void);
void tut_replay_event(void);
void tut_hold_events(int on);
void tut_field_reset(int file, int turn);
void tut_highlight_set(int i, int x0, int y0, int x1, int y1);
void tut_highlight_off(int i);
void tut_set_allow(int on);
void tut_set_answer(unsigned short v);
void tut_set_nowait(int on);
void tut_watch_card(int code);
void libduel_draw_0880631C(int player, int a, int b);
void duel_player_type_set(int player, int type);

/* windows 0..5 are the talk windows (0..2 player side, 3..5 opponent side); sub picks one */
void tut_cmd_talk(int player, unsigned char sub, unsigned short face, unsigned short text)
{
    int slot = 3;

    if (player == 0) {
        if (sub < 3)
            slot = sub + 3;
    } else {
        if (sub < 3)
            slot = sub;
    }
    tut_window_open(slot, face, 0, 0, 0);
    tut_window_set_text(slot, tut_msg_get(text));
    tut_wait_window(slot);
}

void tut_cmd_talk2(int player, unsigned char sub, unsigned short face, unsigned short text)
{
    int slot = 3;

    if (player == 0) {
        if (sub < 3)
            slot = sub + 3;
    } else {
        if (sub < 3)
            slot = sub;
    }
    tut_window_open(slot, face, 0, 0, 0);
    tut_window_set_text(slot, tut_msg_get(text));
    tut_wait_window(slot);
}

void tut_cmd_talk_close(int player, unsigned char sub, unsigned short a, unsigned short b)
{
    int slot = 3;

    if (player == 0) {
        if (sub < 3)
            slot = sub + 3;
    } else {
        if (sub < 3)
            slot = sub;
    }
    tut_window_close(slot);
}

/* window 7: free note at (x, y) */
void tut_cmd_note(int player, unsigned char y, unsigned short text, unsigned short x)
{
    tut_window_open(7, text, x, y, 0);
    tut_window_set_text(7, tut_msg_get(text));
    tut_wait_window(7);
}

void tut_cmd_note_close(int player, unsigned char sub, unsigned short a, unsigned short b)
{
    tut_window_close(7);
}

/* window 8: full-screen board with picture pic */
void tut_cmd_board(int player, unsigned char sub, unsigned short text, unsigned short pic)
{
    tut_window_open(8, text, 0, 0, pic);
    tut_window_set_text(8, tut_msg_get(text));
    tut_wait_window(8);
}

void tut_cmd_board_close(int player, unsigned char sub, unsigned short a, unsigned short b)
{
    tut_window_close(8);
}

/* window 6: question with a list of answers */
void tut_cmd_choice(int player, unsigned char sub, unsigned short text, unsigned short b)
{
    tut_window_open(6, text, 0, 0, 0);
    tut_window_set_text(6, tut_msg_get(text));
    tut_wait_window(6);
}

void tut_cmd_arrow_a_set(int player, unsigned char sub, unsigned short id, unsigned short dir)
{
    tut_arrow_a = id;
    tut_arrow_dir = dir;
}

void tut_cmd_arrow_a_show(int player, unsigned char sub, unsigned short x, unsigned short y)
{
    int i = tut_arrow_a + 5;

    tut_marker_show(i, 1);
    tut_marker_place(i, x, y, tut_arrow_dir);
}

void tut_cmd_arrow_a_hide(int player, unsigned char sub, unsigned short a, unsigned short b)
{
    tut_marker_show(tut_arrow_a + 5, 0);
}

void tut_cmd_arrow_b_set(int player, unsigned char sub, unsigned short id, unsigned short dir)
{
    tut_arrow_b = id;
    tut_arrow_dir = dir;
}

void tut_cmd_arrow_b_show(int player, unsigned char sub, unsigned short x, unsigned short y)
{
    int i = tut_arrow_b + 10;

    tut_marker_show(i, 1);
    tut_marker_place(i, x, y, tut_arrow_dir);
}

void tut_cmd_arrow_b_hide(int player, unsigned char sub, unsigned short a, unsigned short b)
{
    tut_marker_show(tut_arrow_b + 10, 0);
}

/* let the player act until the expected action (kind, arg) happens */
void tut_cmd_wait_action(int player, unsigned char sub, unsigned short kind, unsigned short arg)
{
    tut_set_lock(0);
    tut_expect_action(kind, arg);
    tut_set_state(2);
    tut_duel_pause();
}

void tut_cmd_replay_action(int player, unsigned char sub, unsigned short a, unsigned short b)
{
    tut_replay_action();
}

void tut_cmd_hold_events(int player, unsigned char sub, unsigned short on, unsigned short b)
{
    tut_hold_events(on != 0);
}

void tut_cmd_replay_event(int player, unsigned char sub, unsigned short a, unsigned short b)
{
    tut_replay_event();
}

void tut_cmd_field_reset(int player, unsigned char sub, unsigned short file, unsigned short turn)
{
    tut_field_reset(file, turn);
}

void tut_cmd_lock(int player, unsigned char sub, unsigned short on, unsigned short b)
{
    if (on != 0)
        tut_set_lock(1);
    else
        tut_set_lock(0);
}

void tut_cmd_duel_draw(int player, unsigned char sub, unsigned short a, unsigned short b)
{
    libduel_draw_0880631C(player, sub + a, b);
}

/* rectangle (x0, y0)-(x1, y1); bits 9..11 of a are the ninth bit of y0, x1, y1 */
void tut_cmd_highlight(int player, unsigned char y0, unsigned short a, unsigned short b)
{
    int top = y0;
    int x0 = a & 0x1FF;
    int x1 = b >> 8;
    int y1 = b & 0xFF;

    if (a & 0x200)
        top += 0x100;
    if (a & 0x400)
        x1 += 0x100;
    if (a & 0x800)
        y1 += 0x100;
    tut_highlight_set(player, x0, top, x1, y1);
}

void tut_cmd_highlight_off(int player, unsigned char sub, unsigned short a, unsigned short b)
{
    tut_highlight_off(player);
}

void tut_cmd_mode(int player, unsigned char sub, unsigned short mode, unsigned short b)
{
    switch (mode) {
    case 0:
        tut_set_allow(0);
        break;
    case 1:
        tut_set_allow(1);
        break;
    case 2:
        tut_set_answer(0);
        break;
    case 3:
        tut_set_answer(1);
        break;
    case 4:
        tut_set_answer(2);
        break;
    }
}

/* the opponent is played by the script (1) or by the CPU (3) */
void tut_cmd_opponent(int player, unsigned char sub, unsigned short scripted, unsigned short b)
{
    if (scripted)
        duel_player_type_set(1, 1);
    else
        duel_player_type_set(1, 3);
}

/* wait for a duel event (kind, arg) */
void tut_cmd_wait_event(int player, unsigned char sub, unsigned short kind, unsigned short arg)
{
    tut_set_lock(1);
    tut_expect_event(kind, arg);
    tut_set_state(5);
}

void tut_cmd_nowait(int player, unsigned char sub, unsigned short on, unsigned short b)
{
    tut_set_nowait(on);
}

void tut_cmd_watch_card(int player, unsigned char sub, unsigned short code, unsigned short b)
{
    tut_watch_card(code);
}

void tut_arrows_clear(void)
{
    memset(&tut_arrow_a, 0, 0xC);
}

void tut_cmd_dispatch(int player, int cmd, unsigned short a, unsigned short b)
{
    int sub = (unsigned short)cmd >> 8;

    req_flag_set(0x3E);
    switch (cmd & 0xFF) {
    case 0:
        tut_cmd_talk(player, sub, a, b);
        break;
    case 1:
        tut_cmd_talk2(player, sub, a, b);
        break;
    case 2:
        tut_cmd_talk_close(player, sub, a, b);
        break;
    case 3:
        tut_cmd_note(player, sub, a, b);
        break;
    case 4:
        tut_cmd_note_close(player, sub, a, b);
        break;
    case 5:
        tut_cmd_board(player, sub, a, b);
        break;
    case 6:
        tut_cmd_board_close(player, sub, a, b);
        break;
    case 7:
        tut_cmd_choice(player, sub, a, b);
        break;
    case 8:
        tut_cmd_arrow_a_set(player, sub, a, b);
        break;
    case 9:
        tut_cmd_arrow_a_show(player, sub, a, b);
        break;
    case 10:
        tut_cmd_arrow_a_hide(player, sub, a, b);
        break;
    case 11:
        tut_cmd_arrow_b_set(player, sub, a, b);
        break;
    case 12:
        tut_cmd_arrow_b_show(player, sub, a, b);
        break;
    case 13:
        tut_cmd_arrow_b_hide(player, sub, a, b);
        break;
    case 14:
        tut_cmd_wait_action(player, sub, a, b);
        break;
    case 15:
        tut_cmd_replay_action(player, sub, a, b);
        break;
    case 16:
        tut_cmd_replay_event(player, sub, a, b);
        break;
    case 17:
        tut_cmd_hold_events(player, sub, a, b);
        break;
    case 18:
        tut_cmd_field_reset(player, sub, a, b);
        break;
    case 19:
        tut_cmd_lock(player, sub, a, b);
        break;
    case 20:
    case 21:
    case 22:
    case 23:
        break;
    case 24:
        tut_cmd_duel_draw(player, sub, a, b);
        break;
    case 27:
        tut_cmd_mode(player, sub, a, b);
        break;
    case 28:
        tut_cmd_opponent(player, sub, a, b);
        break;
    case 29:
        tut_cmd_wait_event(player, sub, a, b);
        break;
    case 30:
        tut_cmd_nowait(player, sub, a, b);
        break;
    case 31:
        tut_cmd_watch_card(player, sub, a, b);
        break;
    case 25:
        tut_cmd_highlight(player, sub, a, b);
        break;
    case 26:
        tut_cmd_highlight_off(player, sub, a, b);
        break;
    }
}
