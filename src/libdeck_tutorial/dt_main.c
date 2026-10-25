// @range 0x08804000-0x088058E0
// @asm 0x088047F4-0x08804C40
// @data 0x088210AC-0x08821D18
/* Entry points of the deck-building lesson (init / per-frame update / draw, exported to libtitle,
   libstory, liblabo and libdeck), the unload steps, the darkened screen with a highlighted area cut
   out through the stencil, and the button hints drawn over the editor. */
#include "libdeck_tutorial.h"

typedef struct DtVert {          /* 2D vertex: color, 16-bit position (GE vtype 0x80011C) */
    unsigned int color;
    short x, y, z, pad;
} DtVert;

typedef struct DtShape {
    unsigned short n;
    DtVert *v;
} DtShape;

void inv_set_flag7(unsigned short id, int on);
int inv_get_flag7(unsigned short id);
void *dl_clear_color(void *list, unsigned int color);
void *dl_clear_depth(void *list, unsigned int z);
void *dl_clear_stencil(void *list, unsigned int s);
void *dl_clear(void *list, unsigned int flags);
void *dl_alloc_inline(void **list, int bytes);
void *dl_draw_array(void *list, int prim, int vtype, int count, void *indices, void *verts);
int dt_pack_poll(void);
unsigned int dt_tab_pos(int which);
void dt_lists_free(void);
void dt_text_term(void);
void dt_text_draw(void **list);
void dt_editor_draw(void);
void dt_editor_update(void);
void dt_editor_input(void);
void dt_script_update(void *s);
void dt_pause_open(void);
void dt_editor_start(unsigned short lesson);
void dt_files_init(void);
int dt_files_poll(void);
int dt_res_cmp(const void *a, const void *b);
extern ScPack dt_files[9];

int module_entry(void)
{
    return 0;
}

/* export: lesson number; cfg = parent heap and card reader (or 0) */
void dt_init(unsigned short lesson, DtCfg *cfg)
{
    int id;

    memset(&dt, 0, 0x107C);
    dt.lesson = lesson;
    for (id = 0xFA7; id < 0x1B2E; id++) {
        dt.saved_flags[id - 0xFA7] = inv_get_flag7(id);
        inv_set_flag7(id, 0);
    }
    dt.saved_cfg = cfg_get_byte0_hi2();
    cfg_set_byte0_hi2(0);
    memset(&dts, 0, 0x3D4);
    memset(&dtv, 0, 0x94);
    if (cfg != 0 && cfg->heap != 0) {
        dt.parent_heap = cfg->heap;
        dt.fpl_mem = heap_alloc(cfg->heap, 0x500000);
        dts.heap = heap_create(dt.fpl_mem, 0x500000);
    } else {
        dt.fpl = sceKernelCreateFpl("", 2, 0, 0x500000, 1, 0);
        sceKernelAllocateFpl(dt.fpl, &dt.fpl_mem, 0);
        dts.heap = heap_create(dt.fpl_mem, 0x500000);
    }
    if (cfg != 0)
        dts.cfg_cip = cfg->cip;
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    dt_res_init();
}

/* export: once per frame; 1 when the lesson is over */
int dt_update(void)
{
    int r;

    switch (dt.state) {
    case 0:
        if (dt_load_step()) {
            dt_editor_start(dt.lesson);
            fade_start(1, 0xFF000000, 16);
            gfx_overlay_enable(0);
            dt.state = 1;
        }
        break;
    case 1:
        if (fade_is_done())
            dt.state = 2;
        break;
    case 2:
        if (dt.quit_open) {
            r = menu_update(&dt.quit);
            if (r != -1) {
                if (r == 2) {
                    dt.state = 3;
                    fade_start_in(1, 0xFF000000, 16);
                } else {
                    dt.quit_open = 0;
                }
                menu_reset(&dt.quit);
            }
        } else {
            dt_editor_update();
            dt_editor_input();
            dt_script_update(&dt.pad);
            if (pad_get()->b.trig & 8)
                dt_pause_open();
        }
        break;
    case 3:
        if (fade_is_done())
            dt.state = 4;
        break;
    case 4:
        if (dt_unload())
            return 1;
        break;
    }
    return 0;
}

/* close the card reader, the menus, free the heap and give the inventory flags back */
int dt_unload(void)
{
    int id;

    switch (dts.unload_step) {
    case 0:
        if (dts.cip != 0) {
            cip_close(dts.cip);
            dts.unload_step++;
        } else {
            dts.unload_step += 2;
        }
        break;
    case 1:
        if (cip_is_idle(dts.cip)) {
            dts.cip = 0;
            dts.unload_step++;
        }
        break;
    case 2:
        dt_text_term();
        if (dt.win.state)
            menu_reset(&dt.win);
        if (dt.board_win.state)
            menu_reset(&dt.board_win);
        if (dt.menu.state)
            menu_reset(&dt.menu);
        dts.unload_step++;
        break;
    case 3:
        if (dts.mark != 0) {
            heap_set_level(dts.heap, dts.mark);
            dts.mark = 0;
        }
        dt_pack_free();
        dt_lists_free();
        if (dt.fpl > 0) {
            sceKernelFreeFpl(dt.fpl, dt.fpl_mem);
            sceKernelDeleteFpl(dt.fpl);
        } else {
            heap_free_block(dts.heap);
            heap_set_level(dt.parent_heap, dt.fpl_mem);
        }
        for (id = 0xFA7; id < 0x1B2E; id++)
            inv_set_flag7(id, dt.saved_flags[id - 0xFA7]);
        cfg_set_byte0_hi2(dt.saved_cfg);
        dts.unload_step = 0;
        return 1;
    }
    return 0;
}

/* export: once per frame after dt_update */
void dt_draw(void)
{
    void *list = dl_select_layer(0, 0);

    list = dl_clear_color(list, 0xFF000000);
    list = dl_clear_depth(list, 0);
    list = dl_clear_stencil(list, 0);
    list = dl_clear(list, 0x17);
    dl_set_list(list);
    if (dt.state != 4 && dt.state != 0) {
        dt_editor_draw();
        list = dl_select_layer(4, 0);
        list = dl_enable(list, 9);
        list = dl_enable(list, 4);
        list = dl_tex_func(list, 0, 1);
        list = dl_set_tex_filter(list, 0, 0);
        list = dl_blend_func(list, 0, 2, 3, 0, 0);
        list = dl_disable(list, 1);
        if (dt.quit_open) {
            menu_draw(&list, &dt.quit);
            dl_set_list(list);
        } else {
            if (dt.show & 2)
                list = dt_draw_highlight(list, dt.hl);
            if (dt.show & 4)
                list = dt_draw_hint_buttons(list);
            if (dt.step == 9)
                list = dt_draw_buttons(list);
            list = dt_draw_one_button(list);
            if (dt.text_on & 1) {
                if (dt.text_box)
                    dt_text_draw(&list);
                if (dt.show & 1) {
                    if (cfg_get_language() == 0)
                        font_set_line_extra(0xC0);
                    menu_draw(&list, &dt.win);
                    if (cfg_get_language() == 0)
                        font_set_line_extra(0);
                }
                if (dt.board)
                    list = dt_draw_board(list);
                if (dt.show & 0x40)
                    menu_draw(&list, &dt.menu);
            }
            fade_apply(&list);
            dl_set_list(list);
        }
    }
}

/* outlines of the highlighted areas (triangle lists) */
DtVert dt_shape_00[4] = {
    { 0x00101010, 0, 0, 0, 0 },
    { 0x00101010, 0, 0, 0, 0 },
    { 0x00101010, 0, 0, 0, 0 },
    { 0x00101010, 0, 0, 0, 0 },
};

DtVert dt_shape_01[4] = {
    { 0x00101010, 0, 0, 0, 0 },
    { 0x00101010, 0, 0, 0, 0 },
    { 0x00101010, 0, 0, 0, 0 },
    { 0x00101010, 0, 0, 0, 0 },
};

DtVert dt_shape_02[4] = {
    { 0x00101010, 0, 0, 0, 0 },
    { 0x00101010, 0, 0, 0, 0 },
    { 0x00101010, 0, 0, 0, 0 },
    { 0x00101010, 0, 0, 0, 0 },
};

DtVert dt_shape_03[4] = {
    { 0x00101010, 0, 0, 0, 0 },
    { 0x00101010, 0, 0, 0, 0 },
    { 0x00101010, 0, 0, 0, 0 },
    { 0x00101010, 0, 0, 0, 0 },
};

DtVert dt_shape_04[6] = {
    { 0x00101010, 0, 176, 0, 0 },
    { 0x00101010, 1, 188, 0, 0 },
    { 0x00101010, 1, 175, 0, 0 },
    { 0x00101010, 135, 189, 0, 0 },
    { 0x00101010, 135, 176, 0, 0 },
    { 0x00101010, 136, 188, 0, 0 },
};

DtVert dt_shape_05[6] = {
    { 0x00101010, 0, 201, 0, 0 },
    { 0x00101010, 1, 271, 0, 0 },
    { 0x00101010, 1, 200, 0, 0 },
    { 0x00101010, 135, 272, 0, 0 },
    { 0x00101010, 135, 201, 0, 0 },
    { 0x00101010, 136, 271, 0, 0 },
};

DtVert dt_shape_06[8] = {
    { 0x00101010, 5, 4, 0, 0 },
    { 0x00101010, 113, 38, 0, 0 },
    { 0x00101010, 5, 38, 0, 0 },
    { 0x00101010, 20, 116, 0, 0 },
    { 0x00101010, 98, 38, 0, 0 },
    { 0x00101010, 113, 116, 0, 0 },
    { 0x00101010, 5, 116, 0, 0 },
    { 0x00101010, 113, 158, 0, 0 },
};

DtVert dt_shape_07[4] = {
    { 0x00101010, 17, 26, 0, 0 },
    { 0x00101010, 102, 35, 0, 0 },
    { 0x00101010, 114, 142, 0, 0 },
    { 0x00101010, 145, 155, 0, 0 },
};

DtVert dt_shape_08[6] = {
    { 0x00101010, 0, 162, 0, 0 },
    { 0x00101010, 1, 172, 0, 0 },
    { 0x00101010, 1, 161, 0, 0 },
    { 0x00101010, 53, 173, 0, 0 },
    { 0x00101010, 53, 162, 0, 0 },
    { 0x00101010, 54, 172, 0, 0 },
};

DtVert dt_shape_09[6] = {
    { 0x00101010, 56, 162, 0, 0 },
    { 0x00101010, 57, 172, 0, 0 },
    { 0x00101010, 57, 161, 0, 0 },
    { 0x00101010, 109, 173, 0, 0 },
    { 0x00101010, 109, 162, 0, 0 },
    { 0x00101010, 110, 172, 0, 0 },
};

DtVert dt_shape_10[6] = {
    { 0x00101010, 127, 158, 0, 0 },
    { 0x00101010, 128, 173, 0, 0 },
    { 0x00101010, 128, 157, 0, 0 },
    { 0x00101010, 143, 174, 0, 0 },
    { 0x00101010, 143, 158, 0, 0 },
    { 0x00101010, 144, 173, 0, 0 },
};

DtVert dt_shape_11[6] = {
    { 0x00101010, 110, 158, 0, 0 },
    { 0x00101010, 111, 173, 0, 0 },
    { 0x00101010, 111, 157, 0, 0 },
    { 0x00101010, 126, 174, 0, 0 },
    { 0x00101010, 126, 158, 0, 0 },
    { 0x00101010, 127, 173, 0, 0 },
};

DtVert dt_shape_12[12] = {
    { 0x00101010, 121, 136, 0, 0 },
    { 0x00101010, 122, 143, 0, 0 },
    { 0x00101010, 122, 135, 0, 0 },
    { 0x00101010, 138, 144, 0, 0 },
    { 0x00101010, 138, 136, 0, 0 },
    { 0x00101010, 139, 143, 0, 0 },
    { 0x00101010, 443, 45, 0, 0 },
    { 0x00101010, 444, 52, 0, 0 },
    { 0x00101010, 444, 44, 0, 0 },
    { 0x00101010, 460, 53, 0, 0 },
    { 0x00101010, 460, 45, 0, 0 },
    { 0x00101010, 461, 52, 0, 0 },
};

DtVert dt_shape_13[6] = {
    { 0x00101010, 148, 1, 0, 0 },
    { 0x00101010, 149, 22, 0, 0 },
    { 0x00101010, 149, 0, 0, 0 },
    { 0x00101010, 283, 23, 0, 0 },
    { 0x00101010, 283, 1, 0, 0 },
    { 0x00101010, 284, 22, 0, 0 },
};

DtVert dt_shape_14[6] = {
    { 0x00101010, 284, 1, 0, 0 },
    { 0x00101010, 285, 22, 0, 0 },
    { 0x00101010, 285, 0, 0, 0 },
    { 0x00101010, 433, 23, 0, 0 },
    { 0x00101010, 433, 1, 0, 0 },
    { 0x00101010, 434, 22, 0, 0 },
};

DtVert dt_shape_15[6] = {
    { 0x00101010, 120, 20, 0, 0 },
    { 0x00101010, 121, 132, 0, 0 },
    { 0x00101010, 121, 19, 0, 0 },
    { 0x00101010, 145, 133, 0, 0 },
    { 0x00101010, 145, 20, 0, 0 },
    { 0x00101010, 146, 132, 0, 0 },
};

DtVert dt_shape_16[24] = {
    { 0x00101010, 185, 29, 0, 0 },
    { 0x00101010, 186, 37, 0, 0 },
    { 0x00101010, 186, 28, 0, 0 },
    { 0x00101010, 203, 38, 0, 0 },
    { 0x00101010, 203, 29, 0, 0 },
    { 0x00101010, 204, 37, 0, 0 },
    { 0x00101010, 245, 29, 0, 0 },
    { 0x00101010, 246, 37, 0, 0 },
    { 0x00101010, 246, 28, 0, 0 },
    { 0x00101010, 263, 38, 0, 0 },
    { 0x00101010, 263, 29, 0, 0 },
    { 0x00101010, 264, 37, 0, 0 },
    { 0x00101010, 305, 29, 0, 0 },
    { 0x00101010, 306, 37, 0, 0 },
    { 0x00101010, 306, 28, 0, 0 },
    { 0x00101010, 323, 38, 0, 0 },
    { 0x00101010, 323, 29, 0, 0 },
    { 0x00101010, 324, 37, 0, 0 },
    { 0x00101010, 366, 27, 0, 0 },
    { 0x00101010, 367, 37, 0, 0 },
    { 0x00101010, 367, 26, 0, 0 },
    { 0x00101010, 414, 38, 0, 0 },
    { 0x00101010, 414, 27, 0, 0 },
    { 0x00101010, 415, 37, 0, 0 },
};

DtVert dt_shape_17[6] = {
    { 0x00101010, 150, 255, 0, 0 },
    { 0x00101010, 151, 267, 0, 0 },
    { 0x00101010, 151, 254, 0, 0 },
    { 0x00101010, 182, 268, 0, 0 },
    { 0x00101010, 182, 255, 0, 0 },
    { 0x00101010, 183, 267, 0, 0 },
};

DtVert dt_shape_18[42] = {
    { 0x00101010, 179, 45, 0, 0 },
    { 0x00101010, 180, 54, 0, 0 },
    { 0x00101010, 180, 44, 0, 0 },
    { 0x00101010, 443, 55, 0, 0 },
    { 0x00101010, 443, 45, 0, 0 },
    { 0x00101010, 444, 54, 0, 0 },
    { 0x00101010, 179, 75, 0, 0 },
    { 0x00101010, 180, 84, 0, 0 },
    { 0x00101010, 180, 74, 0, 0 },
    { 0x00101010, 443, 85, 0, 0 },
    { 0x00101010, 443, 75, 0, 0 },
    { 0x00101010, 444, 84, 0, 0 },
    { 0x00101010, 179, 105, 0, 0 },
    { 0x00101010, 180, 114, 0, 0 },
    { 0x00101010, 180, 104, 0, 0 },
    { 0x00101010, 443, 115, 0, 0 },
    { 0x00101010, 443, 105, 0, 0 },
    { 0x00101010, 444, 114, 0, 0 },
    { 0x00101010, 179, 135, 0, 0 },
    { 0x00101010, 180, 144, 0, 0 },
    { 0x00101010, 180, 134, 0, 0 },
    { 0x00101010, 443, 145, 0, 0 },
    { 0x00101010, 443, 135, 0, 0 },
    { 0x00101010, 444, 144, 0, 0 },
    { 0x00101010, 179, 165, 0, 0 },
    { 0x00101010, 180, 174, 0, 0 },
    { 0x00101010, 180, 164, 0, 0 },
    { 0x00101010, 443, 175, 0, 0 },
    { 0x00101010, 443, 165, 0, 0 },
    { 0x00101010, 444, 174, 0, 0 },
    { 0x00101010, 179, 195, 0, 0 },
    { 0x00101010, 180, 204, 0, 0 },
    { 0x00101010, 180, 194, 0, 0 },
    { 0x00101010, 443, 205, 0, 0 },
    { 0x00101010, 443, 195, 0, 0 },
    { 0x00101010, 444, 204, 0, 0 },
    { 0x00101010, 179, 225, 0, 0 },
    { 0x00101010, 180, 234, 0, 0 },
    { 0x00101010, 180, 224, 0, 0 },
    { 0x00101010, 443, 235, 0, 0 },
    { 0x00101010, 443, 225, 0, 0 },
    { 0x00101010, 444, 234, 0, 0 },
};

DtVert dt_shape_19[42] = {
    { 0x00101010, 179, 55, 0, 0 },
    { 0x00101010, 180, 70, 0, 0 },
    { 0x00101010, 180, 54, 0, 0 },
    { 0x00101010, 370, 71, 0, 0 },
    { 0x00101010, 370, 55, 0, 0 },
    { 0x00101010, 371, 70, 0, 0 },
    { 0x00101010, 179, 85, 0, 0 },
    { 0x00101010, 180, 100, 0, 0 },
    { 0x00101010, 180, 84, 0, 0 },
    { 0x00101010, 370, 101, 0, 0 },
    { 0x00101010, 370, 85, 0, 0 },
    { 0x00101010, 371, 100, 0, 0 },
    { 0x00101010, 179, 115, 0, 0 },
    { 0x00101010, 180, 130, 0, 0 },
    { 0x00101010, 180, 114, 0, 0 },
    { 0x00101010, 370, 131, 0, 0 },
    { 0x00101010, 370, 115, 0, 0 },
    { 0x00101010, 371, 130, 0, 0 },
    { 0x00101010, 179, 145, 0, 0 },
    { 0x00101010, 180, 160, 0, 0 },
    { 0x00101010, 180, 144, 0, 0 },
    { 0x00101010, 370, 161, 0, 0 },
    { 0x00101010, 370, 145, 0, 0 },
    { 0x00101010, 371, 160, 0, 0 },
    { 0x00101010, 179, 175, 0, 0 },
    { 0x00101010, 180, 190, 0, 0 },
    { 0x00101010, 180, 174, 0, 0 },
    { 0x00101010, 370, 191, 0, 0 },
    { 0x00101010, 370, 175, 0, 0 },
    { 0x00101010, 371, 190, 0, 0 },
    { 0x00101010, 179, 205, 0, 0 },
    { 0x00101010, 180, 220, 0, 0 },
    { 0x00101010, 180, 204, 0, 0 },
    { 0x00101010, 370, 221, 0, 0 },
    { 0x00101010, 370, 205, 0, 0 },
    { 0x00101010, 371, 220, 0, 0 },
    { 0x00101010, 179, 235, 0, 0 },
    { 0x00101010, 180, 250, 0, 0 },
    { 0x00101010, 180, 234, 0, 0 },
    { 0x00101010, 370, 251, 0, 0 },
    { 0x00101010, 370, 235, 0, 0 },
    { 0x00101010, 371, 250, 0, 0 },
};

DtVert dt_shape_20[42] = {
    { 0x00101010, 381, 55, 0, 0 },
    { 0x00101010, 382, 70, 0, 0 },
    { 0x00101010, 382, 54, 0, 0 },
    { 0x00101010, 462, 71, 0, 0 },
    { 0x00101010, 462, 55, 0, 0 },
    { 0x00101010, 463, 70, 0, 0 },
    { 0x00101010, 381, 85, 0, 0 },
    { 0x00101010, 382, 100, 0, 0 },
    { 0x00101010, 382, 84, 0, 0 },
    { 0x00101010, 462, 101, 0, 0 },
    { 0x00101010, 462, 85, 0, 0 },
    { 0x00101010, 463, 100, 0, 0 },
    { 0x00101010, 381, 115, 0, 0 },
    { 0x00101010, 382, 130, 0, 0 },
    { 0x00101010, 382, 114, 0, 0 },
    { 0x00101010, 462, 131, 0, 0 },
    { 0x00101010, 462, 115, 0, 0 },
    { 0x00101010, 463, 130, 0, 0 },
    { 0x00101010, 381, 145, 0, 0 },
    { 0x00101010, 382, 160, 0, 0 },
    { 0x00101010, 382, 144, 0, 0 },
    { 0x00101010, 462, 161, 0, 0 },
    { 0x00101010, 462, 145, 0, 0 },
    { 0x00101010, 463, 160, 0, 0 },
    { 0x00101010, 381, 175, 0, 0 },
    { 0x00101010, 382, 190, 0, 0 },
    { 0x00101010, 382, 174, 0, 0 },
    { 0x00101010, 462, 191, 0, 0 },
    { 0x00101010, 462, 175, 0, 0 },
    { 0x00101010, 463, 190, 0, 0 },
    { 0x00101010, 381, 205, 0, 0 },
    { 0x00101010, 382, 220, 0, 0 },
    { 0x00101010, 382, 204, 0, 0 },
    { 0x00101010, 462, 221, 0, 0 },
    { 0x00101010, 462, 205, 0, 0 },
    { 0x00101010, 463, 220, 0, 0 },
    { 0x00101010, 381, 235, 0, 0 },
    { 0x00101010, 382, 250, 0, 0 },
    { 0x00101010, 382, 234, 0, 0 },
    { 0x00101010, 462, 251, 0, 0 },
    { 0x00101010, 462, 235, 0, 0 },
    { 0x00101010, 463, 250, 0, 0 },
};

DtShape dt_shapes[22] = {
    { 4, dt_shape_00 },
    { 4, dt_shape_01 },
    { 4, dt_shape_02 },
    { 4, dt_shape_03 },
    { 6, dt_shape_04 },
    { 6, dt_shape_05 },
    { 8, dt_shape_06 },
    { 4, dt_shape_07 },
    { 6, dt_shape_08 },
    { 6, dt_shape_09 },
    { 6, dt_shape_10 },
    { 6, dt_shape_11 },
    { 6, dt_shape_10 },
    { 12, dt_shape_12 },
    { 6, dt_shape_13 },
    { 6, dt_shape_14 },
    { 6, dt_shape_15 },
    { 24, dt_shape_16 },
    { 6, dt_shape_17 },
    { 42, dt_shape_18 },
    { 42, dt_shape_19 },
    { 42, dt_shape_20 },
};


#ifdef NON_MATCHING
/* forma igual; alocação dos $s (forma, ponteiro do painel, flag) diferente */
/* darken the screen except the highlighted area (the card panes for kinds 0..3, fixed shapes for
   the others); the hole's border blinks */
void *dt_draw_highlight(void *list, unsigned short kind)
{
    DtLayout *pane = 0;
    DtVert *v = 0;
    int fixed = 0;
    int h = 0;
    int w = 0;
    int y = 0;
    int x = 0;
    DtShape *s;
    unsigned int size;
    unsigned int a;
    int i;

    switch (kind) {
    case 0:
        pane = &dtv.pane[0].lay;
        size = dt_tab_pos(0);
        x = pane->x + (size & 0xFFFF);
        w = size >> 16 & 0xFFFF;
        y = 0x17;
        h = 0xF;
        s = &dt_shapes[0];
        fixed = 1;
        break;
    case 1:
        if (dtv.mode < 2)
            pane = &dtv.pane[0].lay;
        else
            pane = &dtv.pane[1].lay;
        size = dt_tab_pos(3);
        x = pane->x + (size & 0xFFFF);
        w = size >> 16 & 0xFFFF;
        y = 0x17;
        h = 0xF;
        s = &dt_shapes[1];
        fixed = 1;
        break;
    case 2:
        pane = &dtv.pane[0].lay;
        size = dt_tab_pos(1);
        x = pane->x + (size & 0xFFFF);
        w = size >> 16 & 0xFFFF;
        y = 0x17;
        h = 0xF;
        s = &dt_shapes[2];
        fixed = 1;
        break;
    case 3:
        pane = &dtv.pane[0].lay;
        size = dt_tab_pos(2);
        x = pane->x + (size & 0xFFFF);
        w = size >> 16 & 0xFFFF;
        y = 0x17;
        h = 0xF;
        s = &dt_shapes[3];
        fixed = 1;
        break;
    default:
        s = &dt_shapes[kind];
        break;
    }
    if (s != 0) {
        v = dl_alloc_inline(&list, s->n * 12);
        memcpy(v, s->v, s->n * 12);
        if (fixed) {
            v[0].x = x;
            v[0].y = y;
            v[1].x = x + w;
            v[1].y = y + h;
            v[2].x = pane->x;
            v[2].y = pane->y;
            v[3].x = pane->x + pane->w;
            v[3].y = pane->y + pane->h;
        }
    }
    list = dl_clear_depth(list, 1);
    list = dl_clear(list, 2);
    list = dl_disable(list, 9);
    list = dl_disable(list, 0);
    list = dl_stencil_func(list, 0, 1, 1);
    list = dl_stencil_op(list, 3, 0, 0xF);
    list = dl_enable(list, 3);
    list = dl_tex_func(list, 4, 0);
    a = *gfx_get_state() & 0xF;
    if (a > 8)
        a = (unsigned short)(16 - a);
    a = (unsigned char)((int)(a * 0xA0) / 8);
    for (i = 0; i < s->n; i++)
        v[i].color |= a << 24;
    if (s != 0 && v != 0)
        list = dl_draw_array(list, 6, 0x80011C, s->n, 0, v);
    list = dl_stencil_op(list, 3, 0, 0xF);
    list = dl_ambient_color(list, 0xC0000000);
    list = draw_set_offsets(list, 0, 0, 0x1E0, 0x110);
    list = draw_sprite(list, 0, 0, 0, 0, 0, 0, 0);
    list = dl_disable(list, 3);
    list = dl_enable(list, 9);
    list = dl_blend_func(list, 0, 2, 3, 0, 0);
    list = dl_tex_func(list, 0, 1);
    list = dl_ambient_color(list, 0xFFFFFFFF);
    return list;
}
#endif

/* one button of button.gim (0..3 face buttons, 4/5 L/R, 6 START); arrow = bouncing pointer */
void *dt_draw_button(void *list, int x, int y, unsigned short kind, int lit, int arrow)
{
    int u;
    int v;
    int w;
    int h;
    int dy = 0;
    void *gim = dt_res_get(0x1012);

    if (gim == 0)
        return list;
    switch (kind) {
    case 4:
        u = lit ? 0x61 : 0x3C;
        v = 0x50;
        w = 0x25;
        h = 0xF;
        break;
    case 5:
        u = lit ? 0x61 : 0x3C;
        v = 0x5F;
        w = 0x25;
        h = 0xF;
        break;
    case 6:
        u = lit ? 0x63 : 0x3E;
        v = 0x6F;
        w = 0x20;
        h = 0xE;
        dy += 8;
        break;
    default:
        u = lit ? 0x1E : 0;
        v = kind * 0x1B;
        w = 0x1E;
        h = 0x1B;
        break;
    }
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    list = draw_set_offsets(list, w, h, w, h);
    y += dy;
    list = draw_sprite(list, x, y, 0, u, v, 0, 0);
    if (arrow) {
        unsigned short f = (*gfx_get_state() >> 3) % 3;

        list = draw_set_offsets(list, 0x1A, 0x20, 0x1A, 0x20);
        list = draw_sprite(list, x, y - 0x20, 0, f * 0x1A + 0x62, 0, 0, 0);
    }
    return list;
}

/* the d-pad: bit 0 up, 1 down, 2 left, 3 right */
void *dt_draw_dpad(void *list, int x, int y, unsigned short lit, unsigned short arrow)
{
    unsigned short f = (*gfx_get_state() >> 3) % 3;
    void *gim = dt_res_get(0x1012);
    int u;

    if (gim == 0)
        return list;
    list = dl_ambient_color(list, 0xFFFFFFFF);
    list = dl_set_tex_filter(list, 0, 0);
    list = gim_set_texture(list, gim);
    u = (lit & 1) ? 0x4F : 0x3C;
    list = draw_set_offsets(list, 0x13, 0x14, 0x13, 0x14);
    list = draw_sprite(list, x + 0x11, y, 0, u, 0, 0, 0);
    if (arrow & 1) {
        list = draw_set_offsets(list, 0x1A, 0x20, 0x1A, 0x20);
        list = draw_sprite(list, x + 0xE, y - 0x20, 0, f * 0x1A + 0x62, 0, 0, 0);
    }
    u = (lit & 2) ? 0x4F : 0x3C;
    list = draw_set_offsets(list, 0x13, 0x14, 0x13, 0x14);
    list = draw_sprite(list, x + 0x11, y + 0x14, 0, u, 0x14, 0, 0);
    if (arrow & 2) {
        list = draw_set_offsets(list, 0x1A, 0x20, 0x1A, 0x20);
        list = draw_sprite(list, x + 0xE, y - 0xC, 0, f * 0x1A + 0x62, 0, 0, 0);
    }
    u = (lit & 4) ? 0x4F : 0x3C;
    list = draw_set_offsets(list, 0x13, 0x14, 0x13, 0x14);
    list = draw_sprite(list, x, y + 10, 0, u, 0x28, 0, 0);
    if (arrow & 4) {
        list = draw_set_offsets(list, 0x1A, 0x20, 0x1A, 0x20);
        list = draw_sprite(list, x - 3, y - 0x16, 0, f * 0x1A + 0x62, 0, 0, 0);
    }
    u = (lit & 8) ? 0x4F : 0x3C;
    list = draw_set_offsets(list, 0x13, 0x14, 0x13, 0x14);
    list = draw_sprite(list, x + 0x20, y + 10, 0, u, 0x3C, 0, 0);
    if (arrow & 8) {
        list = draw_set_offsets(list, 0x1A, 0x20, 0x1A, 0x20);
        list = draw_sprite(list, x + 0x1D, y - 0x16, 0, f * 0x1A + 0x62, 0, 0, 0);
    }
    return list;
}

/* step 9: the buttons the player may press, right to left; lit while the mode is 1 or 2 */
void *dt_draw_buttons(void *list)
{
    int x;
    int lit = 0;
    unsigned short pad;

    if (dt.btn_mode - 1U < 2)
        lit = 1;
    x = 0x1D8;

    if (dt.btn_mask & 0x8000) {
        list = dt_draw_button(list, x - 0x1E, 0xE6, 3, lit, 0);
        x -= 0x20;
    }
    if (dt.btn_mask & 0x1000) {
        list = dt_draw_button(list, x - 0x1E, 0xE6, 2, lit, 0);
        x -= 0x20;
    }
    if (dt.btn_mask & 0x4000) {
        list = dt_draw_button(list, x - 0x1E, 0xE6, 1, lit, 0);
        x -= 0x20;
    }
    if (dt.btn_mask & 0x2000) {
        list = dt_draw_button(list, x - 0x1E, 0xE6, 0, lit, 0);
        x -= 0x20;
    }
    if (dt.btn_mask & 1) {
        list = dt_draw_button(list, x - 0x1E, 0xE6, 6, lit, 0);
        x -= 0x20;
    }
    pad = 0;
    if (dt.btn_mask & 0x10)
        pad |= 1;
    if (dt.btn_mask & 0x40)
        pad |= 2;
    if (dt.btn_mask & 0x80)
        pad |= 4;
    if (dt.btn_mask & 0x20)
        pad |= 8;
    if (pad != 0) {
        if (!lit)
            pad = 0;
        list = dt_draw_dpad(list, x - 0x35, 0xE0, pad, 0);
    }
    return list;
}

/* the L or R button hint */
void *dt_draw_one_button(void *list)
{
    unsigned short kind = 0;
    int lit = 0;

    if (dt.show & 0x10) {
        lit = 1;
    } else if (dt.step != 0x10) {
        return list;
    }
    if (dt.btn_one & 0x100)
        kind = 4;
    else if (dt.btn_one & 0x200)
        kind = 5;
    return dt_draw_button(list, 0x193, 0xEC, kind, lit, 0);
}

/* the buttons of btn_mask2, blinking */
void *dt_draw_hint_buttons(void *list)
{
    int x;
    int lit = 0;
    unsigned short pad;

    if ((*gfx_get_state() >> 4) & 1)
        lit = 1;
    x = 0x1D8;

    if (dt.btn_mask2 & 0x8000) {
        list = dt_draw_button(list, x - 0x1E, 0xE6, 3, lit, 0);
        x -= 0x20;
    }
    if (dt.btn_mask2 & 0x1000) {
        list = dt_draw_button(list, x - 0x1E, 0xE6, 2, lit, 0);
        x -= 0x20;
    }
    if (dt.btn_mask2 & 0x4000) {
        list = dt_draw_button(list, x - 0x1E, 0xE6, 1, lit, 0);
        x -= 0x20;
    }
    if (dt.btn_mask2 & 0x2000) {
        list = dt_draw_button(list, x - 0x1E, 0xE6, 0, lit, 0);
        x -= 0x20;
    }
    pad = 0;
    if (dt.btn_mask2 & 0x10)
        pad |= 1;
    if (dt.btn_mask2 & 0x40)
        pad |= 2;
    if (dt.btn_mask2 & 0x80)
        pad |= 4;
    if (dt.btn_mask2 & 0x20)
        pad |= 8;
    if (pad != 0) {
        if (!lit)
            pad = 0;
        list = dt_draw_dpad(list, x - 0x35, 0xE0, pad, 0);
    }
    return list;
}

/* full-screen board (large_wnd.gim, 480 x 256 plus the bottom strip) and its window */
void *dt_draw_board(void *list)
{
    void *gim = dt_res_get(0x1013);
    int i;
    int x;
    int v;
    int j;

    if (gim == 0)
        return list;
    {
        list = gim_set_texture(list, gim);
        list = dl_ambient_color(list, 0xFFFFFFFF);
        list = dl_set_tex_filter(list, 0, 0);
        list = draw_set_offsets(list, 0x40, 0x100, 0x40, 0x100);
        for (i = 0, x = 0; i < 7; i++, x += 0x40)
            list = draw_sprite(list, x, 0, 0, x, 0, 0, 0);
        list = draw_set_offsets(list, 0x20, 0x100, 0x20, 0x100);
        list = draw_sprite(list, i * 0x40, 0, 0, i * 0x40, 0, 0, 0);
        list = draw_set_offsets(list, 0x10, 0x40, 0x40, 0x10);
        for (j = 0, v = 0; j < 4; j++, v += 0x40) {
            list = draw_sprite(list, 0x100 - (j + 1) * 0x40, 0x100, 0, 0x1E0, v, 0, 3);
            list = draw_sprite(list, 0x200 - (j + 1) * 0x40, 0x100, 0, 0x1F0, v, 0, 3);
        }
        menu_draw(&list, &dt.board_win);
    }
    return list;
}

void dt_res_init(void)
{
    dt_files_init();
    memset(&dt_res, 0, 1);
    dt_res.ready = 1;
    qsort(dt_files, 9, 0xC, dt_res_cmp);
}

/* the tutorial pack, then the lesson's files */
int dt_load_step(void)
{
    if (dt_res.ready) {
        switch (dt_res.step) {
        case 0:
            dt_pack_load();
            break;
        case 1:
            if (dt_files_poll())
                dt_res.step++;
            break;
        default:
            dt_res.step = 0;
            return 1;
        }
    }
    return 0;
}
