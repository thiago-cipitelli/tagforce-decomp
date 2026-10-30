// @range 0x0885F76C-0x0885F980
/* Field browse mode of the duel scene: opens the panel (0x0881DBC8 entry 48), moves the field
   cursor with the d-pad until confirm/square, then waits for the panel to close and plays the
   closing sound. */
#include "dd_duelui.h"

typedef struct DdPad {                 /* 0x0881DE50 */
    int f0;
    unsigned int trig;                 /* 0x04 */
    int f8;
    unsigned int held;                 /* 0x0C */
} DdPad;

extern int browse_state;               /* 0x08C5B438 1 browsing, 2 closing */
extern int browse_closed;              /* 0x08C5B43C */

int func_0881D06C(void);
void func_0881CB90(int mode);
void func_0881D044(int mode);
void *func_0881DBC8(int id);
DdPad *func_0881DE50(void);
void func_08814E38(int button);
void func_0881CC60(void);
void func_0882308C(void *panel);
void func_08823328(int (*done)(void));
typedef struct DdPanel { int f0, f4; unsigned char mode; } DdPanel;
DdPanel *func_08823310(void);       /* current panel */
int func_088232E0(void);
unsigned short region_pick_value(int flag);
void se_play(int id);

void browse_open(void *panel);
int browse_is_closed(void);
void browse_set_closed(void);
int browse_panel_done(void);
void browse_close(void);

void browse_mode_set7(void)
{
    int old = func_0881D06C();

    func_0881CB90(7);
    func_0881D044(old);
}

void browse_start(void)
{
    browse_open(func_0881DBC8(48));
    browse_state = 1;
}

void browse_nop(void)
{
}

void *browse_identity(void *p)
{
    return p;
}

void browse_update(void)
{
    DdPad *pad = func_0881DE50();

    switch (browse_state) {
    case 1:
        if (pad->held & 0x80)
            func_08814E38(0x80);
        else if (pad->held & 0x10)
            func_08814E38(0x10);
        else if (pad->held & 0x40)
            func_08814E38(0x40);
        else if (pad->held & 0x20)
            func_08814E38(0x20);
        if ((pad->trig & region_pick_value(1)) || (pad->trig & 0x8000))
            browse_close();
        break;
    case 2:
        if (browse_panel_done()) {
            browse_state = 0;
            func_0881CC60();
            se_play(0x621A);
        }
        break;
    }
}

int browse_active(void)
{
    return browse_state == 1;
}

void browse_close(void)
{
    browse_state = 2;
    browse_set_closed();
}

void browse_open(void *panel)
{
    DdPanel *p;

    memset(&browse_closed, 0, sizeof(browse_closed));
    func_0882308C(panel);
    func_08823328(browse_is_closed);
    p = func_08823310();
    p->mode = 3;
}

void browse_set_closed(void)
{
    browse_closed = 1;
}

int browse_panel_done(void)
{
    return func_088232E0();
}

int browse_is_closed(void)
{
    return browse_closed;
}
