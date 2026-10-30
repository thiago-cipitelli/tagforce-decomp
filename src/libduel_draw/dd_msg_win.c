// @range 0x08825088-0x08825DDC
// @asm 0x08825088-0x088252B0
// @asm 0x088252C0-0x08825580
// @asm 0x08825588-0x08825764
// @asm 0x08825778-0x088257B8
// @asm 0x088257D0-0x08825818
// @asm 0x08825824-0x088259D8
// @asm 0x08825A14-0x08825A70
// @asm 0x08825A7C-0x08825B7C
// @asm 0x08825B9C-0x08825DDC
/* The message window system: formatted texts, the shared menu window (open/update/draw/close), its string buffers, the LP/phase marks over the field and the action-list window opener (message kind 13).
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

extern int D_08C58A40;
extern int D_08C58B60;
extern int D_08C58B64;
extern int D_08C58B68;
extern int D_08C58B6C;
extern int D_08C58B70;
extern int D_08C58B7C;
extern int D_08C58B80;
void func_088252B0(void);
void func_088252B8(void);
void func_08825580(void);
int func_08825764(void);
void func_088257B8(void);
int func_088257C4(void);
int *func_08825818(void);
void func_088259D8(void);
void func_088259E4(int v);
void func_088259F0(int v);
void func_088259FC(int v);
void func_08825A08(int v);
void func_08825A70(int v);
void func_08825B7C(int v);
void func_08825B88(int on);

#ifdef NON_MATCHING
// 08825088 size=96
undefined * func_08825088(int param_1)
{
  undefined4 uVar1;
  uVar1 = dlg_get_text(D_08C58A38);
  wsprintf(&D_08C5882C,uVar1,D_08C58A2C + param_1 * D_08C58A34);
  return &D_08C5882C;
}
#endif

#ifdef NON_MATCHING
// 088250e8 size=456
void func_088250E8(undefined4 param_1,int param_2,int param_3)
{
  ushort uVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  undefined4 uVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  iVar2 = D_08C58A3C;
  iVar11 = (uint)*(ushort *)(D_08C58A3C + 0xd0) + (*(byte *)(D_08C58A3C + 0xd2) & 0x1f);
  iVar3 = ((int)*(short *)(D_08C58A3C + 0x6c) - (int)*(short *)(D_08C58A3C + 0x90)) -
          (int)*(short *)(D_08C58A3C + 0x92);
  if (iVar3 < 0) {
    iVar3 = iVar3 + 1;
  }
  iVar3 = iVar3 >> 1;
  font_set_size(0xc,0xc);
  font_set_color(0xff000000);
  uVar4 = func_08825088(iVar11);
  iVar5 = font_measure_text(uVar4);
  iVar10 = iVar5 >> 6;
  iVar6 = font_line_height();
  iVar9 = iVar6 >> 6;
  iVar6 = iVar6 >> 7;
  if (iVar9 < 0) {
    iVar6 = iVar9 + 1 >> 1;
  }
  iVar7 = param_3 + (0x14 - iVar6);
  iVar5 = iVar5 >> 7;
  if (iVar10 < 0) {
    iVar5 = iVar10 + 1 >> 1;
  }
  iVar5 = param_2 + (iVar3 - iVar5);
  uVar8 = menu_draw_cursor(param_1,iVar2,iVar5 + -10,iVar7 + -5,iVar10 + 0x14,iVar9 + 10);
  font_text_set(uVar8);
  font_draw_text(iVar5 * 0x40,iVar7 * 0x40,(0x1e0 - iVar5) * 0x40,uVar4);
  uVar4 = font_text_clear();
  uVar4 = gim_set_texture(uVar4,*(undefined4 *)(iVar2 + 0xe4));
  if (iVar11 == 0) {
    uVar1 = *(ushort *)(iVar2 + 0xcc);
  }
  else {
    uVar4 = menu_draw_arrow(uVar4,param_2 + iVar3 + -4,(param_3 + 0x14) - iVar6,1);
    uVar1 = *(ushort *)(iVar2 + 0xcc);
  }
  if (iVar11 < (int)(uVar1 - 1)) {
    menu_draw_arrow(uVar4,param_2 + iVar3 + -4,iVar6 + param_3 + 0x15,0);
  }
  return;
}
#endif

void func_088252B0(void)
{
}

void func_088252B8(void)
{
}

#ifdef NON_MATCHING
// 088252c0 size=212
void func_088252C0(void)
{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  iVar1 = duel_088FE8A4();
  if ((iVar1 == 3) && (iVar1 = func_0882092C(), iVar1 == 0)) {
    uVar2 = func_0886677C();
    iVar1 = 0;
    do {
      iVar3 = func_08866AA0(uVar2,iVar1,1);
      if ((((iVar3 != 0) && (iVar3 = func_0880B7B8(uVar2,iVar1,0), iVar3 != 0)) &&
          (*(float *)(iVar3 + 0x74) <= *(float *)(iVar3 + 0x70))) &&
         (iVar3 = func_0884E41C(uVar2,iVar1), iVar3 == 0)) {
        func_08825394(uVar2,iVar1);
      }
      iVar1 = iVar1 + 1;
    } while (iVar1 < 5);
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 08825394 size=360
void func_08825394(byte param_1,byte param_2)
{
  uint *puVar1;
  undefined4 uVar2;
  float fVar3;
  undefined1 auStack_70 [64];
  undefined1 auStack_30 [16];
  undefined1 auStack_20 [4];
  float local_1c;
  ushort local_4;
  local_4 = (ushort)(byte)(param_1 & 1 | (param_2 & 0x1f) << 1);
  func_0880A8E0(auStack_20,auStack_30,local_4);
  quat_to_mat4(auStack_70,auStack_30);
  puVar1 = (uint *)gfx_get_state();
  fVar3 = (float)sinf_vfpu((float)*puVar1 / 10.0);
  local_1c = local_1c + fVar3 * 5.0 + 10.0;
  mtx_push();
  mtx_identity();
  mtx_translate(auStack_20);
  mtx_mult(auStack_70);
  uVar2 = dl_select_layer(4,0);
  uVar2 = dl_set_color(uVar2,0xffffffff);
  uVar2 = func_088254FC(uVar2);
  dl_set_list(uVar2);
  mtx_pop();
  return;
}
#endif

#ifdef NON_MATCHING
// 088254fc size=132
void func_088254FC(undefined4 param_1)
{
  undefined4 uVar1;
  uVar1 = func_088044B0(0x40006);
  uVar1 = gim_set_texture(param_1,uVar1);
  uVar1 = dl_enable(uVar1,4);
  uVar1 = dl_blend_func(uVar1,0,2,3,0,0);
  draw_array(uVar1,4,0x183,4,0,&D_08876228);
  return;
}
#endif

void func_08825580(void)
{
}

#ifdef NON_MATCHING
// 08825588 size=96
void func_08825588(void)
{
  menu_reset(&D_08C58A40);
  if (D_08C58B74 != 0) {
    dd_free(D_08C58B74);
    D_08C58B74 = 0;
  }
  if (D_08C58B78 != 0) {
    dd_free();
    D_08C58B78 = 0;
  }
  return;
}
#endif

#ifdef NON_MATCHING
// 088255e8 size=140
undefined4 func_088255E8(undefined4 param_1)
{
  undefined2 uVar1;
  int iVar2;
  undefined4 local_4;
  local_4 = param_1;
  uVar1 = font_get_flags();
  iVar2 = cfg_get_language();
  if (iVar2 == 0) {
    font_set_flag1(1);
  }
  else {
    font_set_flag8(1);
  }
  if (D_08C58B64 != (code *)0x0) {
    local_4 = (*D_08C58B64)(local_4);
  }
  menu_draw(&local_4,&D_08C58A40);
  font_set_flags(uVar1);
  return local_4;
}
#endif

#ifdef NON_MATCHING
// 08825674 size=180
uint func_08825674(void)
{
  uint uVar1;
  int iVar2;
  int iVar3;
  if (D_08C58B6C == (code *)0x0) {
    iVar3 = (uint)D_08C58B10 + (D_08C58B12 & 0x1f);
    iVar2 = dd_input_on();
    if (iVar2 != 0) {
      D_08C58B70 = menu_update(&D_08C58A40);
    }
    if ((iVar3 != (uint)D_08C58B10 + (D_08C58B12 & 0x1f)) && (D_08C58B68 != (code *)0x0)) {
      (*D_08C58B68)(&D_08C58A40,iVar3);
    }
    iVar2 = func_08825728();
    uVar1 = iVar2 != 0 ^ 1;
  }
  else {
    uVar1 = (*D_08C58B6C)();
  }
  return uVar1;
}
#endif

#ifdef NON_MATCHING
// 08825728 size=60
undefined4 func_08825728(void)
{
  undefined4 uVar1;
  if (D_08C58B29 == '\x06') {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
    if ((D_08C58B29 == '\x01') || (D_08C58B29 == '\0')) {
      uVar1 = 0;
    }
  }
  return uVar1;
}
#endif

int func_08825764(void)
{
    return *((unsigned char *)&D_08C58A40 + 0xE9) == 3;
}

#ifdef NON_MATCHING
void func_08825778(void)
{
    if (D_08C58B60 != 0) {
        ((void (*)(void))D_08C58B60)();
        D_08C58B60 = 0;
    }
}
#endif

void func_088257B8(void)
{
    menu_finish((MenuWidget *)&D_08C58A40);
}

int func_088257C4(void)
{
    return D_08C58B7C;
}

#ifdef NON_MATCHING
int func_088257D0(void)
{
    return D_08C58B80 == 0;
}
#endif

#ifdef NON_MATCHING
// 088257e4 size=52
void func_088257E4(short param_1,short param_2)
{
  D_08C58A98 = D_08C58A98 + param_1;
  D_08C58A9A = D_08C58A9A + param_2;
  return;
}
#endif

int *func_08825818(void)
{
    return &D_08C58A40;
}

#ifdef NON_MATCHING
// 08825824 size=436
void func_08825824(undefined4 param_1,undefined4 param_2,int param_3,code *param_4)
{
  undefined2 uVar1;
  int iVar2;
  memset(&D_08C58A40,0,0x144);
  func_08825B9C(&D_08C58A40);
  func_08825A7C(param_1);
  func_08825AFC(param_2);
  if ((param_3 == 0x30) || (param_3 == 0x20)) {
    if (param_3 < 0) {
      param_3 = param_3 + 0xf;
    }
    D_08C58B09 = D_08C58B09 & 0x87 | (byte)((param_3 >> 4 & 0xfU) << 3);
  }
  else if ((param_3 == 3) || (param_3 == 1)) {
    D_08C58B0A = D_08C58B0A & 0xf0 | (byte)param_3 & 0xf;
  }
  D_08C58AA4 = 0x109;
  if (param_4 != (code *)0x0) {
    (*param_4)(&D_08C58A40,1);
  }
  uVar1 = font_get_flags();
  iVar2 = cfg_get_language();
  if (iVar2 == 0) {
    font_set_flag1(1);
  }
  else {
    font_set_flag8(1);
  }
  menu_open(&D_08C58A40);
  font_set_flags(uVar1);
  iVar2 = (int)D_08C58AAC;
  if (iVar2 < 0) {
    iVar2 = iVar2 + 1;
  }
  D_08C58A98 = 0x13a - (short)(iVar2 >> 1);
  iVar2 = (int)D_08C58AAE;
  if (iVar2 < 0) {
    iVar2 = iVar2 + 1;
  }
  D_08C58A9A = 0x84 - (short)(iVar2 >> 1);
  if (param_4 != (code *)0x0) {
    (*param_4)(&D_08C58A40,0);
  }
  return;
}
#endif

void func_088259D8(void)
{
    menu_reset((MenuWidget *)&D_08C58A40);
}

void func_088259E4(int v)
{
    D_08C58B64 = v;
}

void func_088259F0(int v)
{
    D_08C58B68 = v;
}

void func_088259FC(int v)
{
    D_08C58B60 = v;
}

void func_08825A08(int v)
{
    D_08C58B6C = v;
}

#ifdef NON_MATCHING
// 08825a14 size=88
int func_08825A14(void)
{
  int iVar1;
  iVar1 = D_08C58B70;
  if (((((D_08C58B0A & 0xf) != 0) && (D_08C58B70 != 3)) && (D_08C58B70 != 2)) &&
     (D_08C58B70 != 1)) {
    iVar1 = 0;
  }
  return iVar1;
}
#endif

void func_08825A70(int v)
{
    D_08C58B70 = v;
}

#ifdef NON_MATCHING
// 08825a7c size=128
void func_08825A7C(int param_1)
{
  int iVar1;
  if (D_08C58B74 != 0) {
    dd_free(D_08C58B74);
    D_08C58B74 = 0;
  }
  if (param_1 != 0) {
    iVar1 = wcslen(param_1);
    D_08C58B74 = dd_alloc((iVar1 + 1) * 2);
    if (D_08C58B74 != 0) {
      wcscpy(D_08C58B74,param_1);
    }
  }
  D_08C58AE0 = D_08C58B74;
  return;
}
#endif

#ifdef NON_MATCHING
// 08825afc size=128
void func_08825AFC(int param_1)
{
  int iVar1;
  if (D_08C58B78 != 0) {
    dd_free(D_08C58B78);
    D_08C58B78 = 0;
  }
  if (param_1 != 0) {
    iVar1 = wcslen(param_1);
    D_08C58B78 = dd_alloc((iVar1 + 1) * 2);
    if (D_08C58B78 != 0) {
      wcscpy(D_08C58B78,param_1);
    }
  }
  D_08C58AC4 = D_08C58B78;
  return;
}
#endif

void func_08825B7C(int v)
{
    D_08C58B7C = v;
}

void func_08825B88(int on)
{
    D_08C58B80 = !on;
}

#ifdef NON_MATCHING
// 08825b9c size=576
void func_08825B9C(int param_1)
{
  undefined4 uVar1;
  memset(param_1,0,0x120);
  *(undefined4 *)(param_1 + 0x4c) = 1;
  *(undefined4 *)(param_1 + 0x50) = 0;
  *(undefined1 *)(param_1 + 0x54) = 0;
  *(undefined1 *)(param_1 + 0x55) = 2;
  *(undefined1 *)(param_1 + 0x56) = 0;
  *(undefined2 *)(param_1 + 0x58) = 0;
  *(undefined2 *)(param_1 + 0x5a) = 0;
  *(undefined2 *)(param_1 + 0x5c) = 0;
  *(undefined4 *)(param_1 + 0x60) = 1;
  *(undefined2 *)(param_1 + 0x6c) = 0;
  *(undefined2 *)(param_1 + 0x6e) = 0;
  *(undefined4 *)(param_1 + 0x70) = 0xffffffff;
  *(undefined2 *)(param_1 + 0x74) = 0;
  *(undefined4 *)(param_1 + 0x78) = 1;
  *(undefined4 *)(param_1 + 0x7c) = 1;
  *(undefined2 *)(param_1 + 0x80) = 0;
  *(undefined2 *)(param_1 + 0x82) = 0;
  *(undefined4 *)(param_1 + 0x84) = 0;
  *(undefined4 *)(param_1 + 0x88) = 1;
  *(undefined2 *)(param_1 + 0x90) = 10;
  *(undefined2 *)(param_1 + 0x92) = 10;
  *(undefined2 *)(param_1 + 0x94) = 8;
  *(undefined2 *)(param_1 + 0x96) = 8;
  *(undefined1 *)(param_1 + 0x98) = 0xc;
  *(undefined4 *)(param_1 + 0x9c) = 0xff000000;
  *(undefined4 *)(param_1 + 0xa0) = 0;
  *(undefined1 *)(param_1 + 0xa4) = 0xc;
  *(undefined4 *)(param_1 + 0xa8) = 0xffffffff;
  *(undefined4 *)(param_1 + 0xac) = 2;
  *(undefined4 *)(param_1 + 0xb0) = 2;
  *(undefined4 *)(param_1 + 0xb4) = 0x3f800000;
  *(undefined4 *)(param_1 + 0xb8) = 0x3f800000;
  *(undefined4 *)(param_1 + 0xbc) = 0;
  uVar1 = func_0881CA74();
  *(undefined4 *)(param_1 + 0xc4) = uVar1;
  *(byte *)(param_1 + 200) = *(byte *)(param_1 + 200) & 0xfe;
  *(byte *)(param_1 + 200) = *(byte *)(param_1 + 200) & 0xfd | 2;
  *(byte *)(param_1 + 200) = *(byte *)(param_1 + 200) & 0xfb | 4;
  *(byte *)(param_1 + 200) = *(byte *)(param_1 + 200) & 0xf7;
  *(byte *)(param_1 + 200) = *(byte *)(param_1 + 200) & 0xdf;
  *(byte *)(param_1 + 200) = *(byte *)(param_1 + 200) & 0xbf;
  *(byte *)(param_1 + 200) = *(byte *)(param_1 + 200) & 0x7f;
  *(byte *)(param_1 + 0xc9) = *(byte *)(param_1 + 0xc9) & 0xfe;
  *(byte *)(param_1 + 0xc9) = *(byte *)(param_1 + 0xc9) & 0x87;
  *(byte *)(param_1 + 0xca) = *(byte *)(param_1 + 0xca) & 0xf0;
  *(undefined2 *)(param_1 + 0xcc) = 0;
  *(undefined1 *)(param_1 + 0xce) = 0;
  *(undefined2 *)(param_1 + 0xd0) = 0;
  *(byte *)(param_1 + 0xd2) = *(byte *)(param_1 + 0xd2) & 0xe0;
  *(byte *)(param_1 + 0xd3) = *(byte *)(param_1 + 0xd3) & 0xf0;
  *(undefined1 *)(param_1 + 0xd4) = 0;
  *(undefined4 *)(param_1 + 0xd8) = 0;
  *(undefined4 *)(param_1 + 0xdc) = 0;
  *(undefined2 *)(param_1 + 0xe0) = 0;
  *(undefined2 *)(param_1 + 0xf0) = 0;
  *(undefined2 *)(param_1 + 0xf2) = 0;
  *(undefined2 *)(param_1 + 0xf4) = 0;
  *(undefined2 *)(param_1 + 0xf6) = 0;
  *(undefined2 *)(param_1 + 0xf8) = 0;
  *(undefined2 *)(param_1 + 0xfa) = 0;
  *(undefined4 *)(param_1 + 0xfc) = 0;
  *(undefined4 *)(param_1 + 0x100) = 0;
  *(undefined4 *)(param_1 + 0x10c) = 0;
  *(undefined4 *)(param_1 + 0x110) = 0;
  *(undefined4 *)(param_1 + 0x114) = 0;
  *(undefined4 *)(param_1 + 0x118) = 0;
  return;
}
#endif
