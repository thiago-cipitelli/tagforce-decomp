// @range 0x0882308C-0x08823674
// @asm 0x0882308C-0x08823310
// @asm 0x08823328-0x08823650
/* Window listing the actions of the card under the cursor (menu with a cursor listener) and its exports for the duel manager.
   Functions under NON_MATCHING stay in asm; their C is the Ghidra pseudocode cleaned of the address
   names (Ghidra types: undefined4 = int-sized, code = function pointer), kept for the port. */
#include "dd_field_ui.h"

#ifdef NON_MATCHING
typedef unsigned char undefined;
typedef unsigned char undefined1;
typedef unsigned short undefined2;
typedef unsigned int undefined4;
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef void (*code)();
#endif

int dv_08832D08();
extern int D_08C58584;
extern int D_08C5880C;
int *func_08823310(void);
void func_0882331C(int v);
int func_08823650(void);

#ifdef NON_MATCHING
// 0882308c size=424
void func_0882308C(undefined4 param_1)
{
  undefined4 uVar1;
  byte local_4 [4];
  if (D_08C587FC == 0) {
    uVar1 = dd_heap_get();
    menu_init(&D_08C58538,uVar1);
    D_08C587FC = 6;
    D_08C58808 = func_08823650;
    D_08C58800 = 0;
    D_08C58804 = 0;
    D_08C5880C = 0;
    memcpy(&D_08C585FC,param_1,0x200);
    memset(&D_08C58584,0,0x78);
    D_08C58584 = 1;
    D_08C58598 = 1;
    D_08C585C0 = 1;
    D_08C585E4 = 1;
    D_08C585E8 = 1;
    D_08C585A8 = 0xffffffff;
    D_08C5859C = 0x109;
    D_08C585BC = &D_08C585FC;
    D_08C585C8 = 10;
    D_08C585CA = 10;
    D_08C585CC = 8;
    D_08C585CE = 8;
    D_08C585D0 = 0xc;
    D_08C585D4 = 0xff202020;
    D_08C585EC = 0x3f800000;
    D_08C585F0 = 0x3f800000;
    D_08C585F4 = 6;
    D_08C58588 = 0;
    D_08C5858C = 0;
    D_08C5858D = 0;
    D_08C5858E = 0;
    D_08C58590 = 0;
    D_08C58592 = 0;
    D_08C58594 = 0;
    D_08C585A4 = 0;
    D_08C585A6 = 0;
    D_08C585AC = 0;
    D_08C585B0 = 0;
    D_08C585B4 = 0;
    D_08C585B8 = 0;
    D_08C585BA = 0;
    D_08C585D8 = 0;
    D_08C585DC = 0;
    D_08C585E0 = 0;
    menu_add_item(&D_08C58538,&D_08C58584);
    menu_item_open(&D_08C58538,&D_08C58584);
    dd_cursor_get(local_4);
    if ((local_4[0] & 1) == 0) {
      D_08C587FC = 3;
    }
    else {
      D_08C587FC = 2;
    }
    dd_cursor_listen(func_088235C4);
    se_play(0x621a);
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 08823234 size=172
void func_08823234(void)
{
  int iVar1;
  undefined4 local_4;
  if (D_08C587FC == 6) {
    iVar1 = menu_item_state(&D_08C58538,&D_08C58584);
    if (iVar1 == 4) {
      return;
    }
  }
  else if (D_08C587FC == 0) {
    return;
  }
  func_08823344();
  local_4 = dl_select_layer(8,0);
  menu_layout(&local_4,&D_08C58538);
  if (D_08C5880C != (code *)0x0) {
    local_4 = (*D_08C5880C)(local_4,&D_08C58538,&D_08C58584);
  }
  dl_set_list(local_4);
  return;
}
#endif

#ifdef NON_MATCHING
int func_088232E0(void)
{
    int r;

    if (D_08C587FC == 6)
        r = 1;
    else if (D_08C587FC == 0)
        r = 1;
    else
        r = 0;
    return r;
}
#endif

int *func_08823310(void)
{
    return &D_08C58584;
}

void func_0882331C(int v)
{
    D_08C5880C = v;
}

#ifdef NON_MATCHING
void func_08823328(int (*f)(void))
{
    if (f == 0)
        f = func_08823650;
    D_08C58808 = (int)f;
}
#endif

#ifdef NON_MATCHING
// 08823344 size=640
void func_08823344(void)
{
  int iVar1;
  iVar1 = menu_item_state(&D_08C58538,&D_08C58584);
  if (iVar1 == 4) {
    if (D_08C587FC == 5) {
      D_08C587FC = 6;
    }
    else if (D_08C587FC == 4) {
      D_08C587FC = 2;
    }
    else if (D_08C587FC == 1) {
      D_08C587FC = 3;
    }
  }
  if ((D_08C587FC == 4) || (D_08C587FC == 3)) {
    iVar1 = (int)D_08C585A6;
    if (iVar1 < 0) {
      iVar1 = iVar1 + 1;
    }
    D_08C58804 = 0x42 - (iVar1 >> 1);
  }
  else if ((D_08C587FC == 2) || (D_08C587FC == 1)) {
    iVar1 = (int)D_08C585A6;
    if (iVar1 < 0) {
      iVar1 = iVar1 + 1;
    }
    D_08C58804 = 0xca - (iVar1 >> 1);
  }
  iVar1 = (int)D_08C585A4;
  if (iVar1 < 0) {
    iVar1 = iVar1 + 1;
  }
  D_08C58592 = (undefined2)D_08C58804;
  D_08C58800 = (int)(314.0 - (float)(iVar1 >> 1));
  D_08C58590 = (undefined2)D_08C58800;
  iVar1 = menu_item_state(&D_08C58538,&D_08C58584);
  if ((iVar1 == 4) && ((D_08C587FC == 3 || (D_08C587FC == 2)))) {
    menu_item_open(&D_08C58538,&D_08C58584);
  }
  if ((D_08C58808 == (code *)0x0) || (iVar1 = (*D_08C58808)(), iVar1 == 0)) {
    if (((D_08C587FC == 4) || (D_08C587FC == 1)) &&
       (iVar1 = menu_item_is_open(&D_08C58538,&D_08C58584), iVar1 != 0)) {
      menu_item_close(&D_08C58538,&D_08C58584);
    }
  }
  else if (D_08C587FC == 6) {
    iVar1 = menu_item_state(&D_08C58538,&D_08C58584);
    if (iVar1 == 4) {
      menu_free_items(&D_08C58538);
      dd_cursor_unlisten(func_088235C4);
      D_08C587FC = 0;
    }
  }
  else if (((D_08C587FC != 5) && (D_08C587FC != 0)) &&
          (iVar1 = menu_item_is_open(&D_08C58538,&D_08C58584), iVar1 != 0)) {
    menu_item_close(&D_08C58538,&D_08C58584);
    D_08C587FC = 5;
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 088235c4 size=140
void func_088235C4(byte param_1)
{
  if ((param_1 & 1) == 0) {
    if (D_08C587FC == 4) {
      D_08C587FC = 3;
    }
    else if (D_08C587FC == 2) {
      D_08C587FC = 1;
    }
  }
  else if (D_08C587FC == 3) {
    D_08C587FC = 4;
  }
  else if (D_08C587FC == 1) {
    D_08C587FC = 2;
  }
  return;
}
#endif

int func_08823650(void)
{
    return dv_08832D08() == 0;
}
