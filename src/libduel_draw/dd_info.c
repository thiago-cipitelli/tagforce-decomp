// @range 0x08808D10-0x08809B1C
// @rodata 0x0886FE34-0x0886FE64
// @asm 0x08809474-0x08809508
// @asm 0x08809548-0x0880989C
/* Card info panel of the duel (BOOT cardlist): one panel, or two side by side when the player
   opens it or browses the field; the panel slides in and out, shows the printed stats of the card
   and its current values on the field (ATK/DEF, counters, field spell bonuses). */
#include "dd_d1.h"

typedef struct CardListCfg {           /* BOOT cardlist_init */
    void *gim;
    void *digits_gim;
    void *frame_ehf;
    void *attr_gim;
    void *kind_gim;
    void *cil_gim;
    void *icon_gim;
    void *cip;
    void *frame_view;
} CardListCfg;

int ehf_get_u32_1c(void *f);
void cardlist_init(CardListCfg *cfg);
void cardlist_clear(void);
void *cardlist_draw_at(void *list, int x, int y, DdInfoView *v);
void *cardlist_draw_pair_at(void *list, int x, int y, DdInfoView *a, DdInfoView *b);

/* other ranges of libduel_draw */
void *dd_cards_cip_h(void);
int dd_08806BC4(void);
int dv_list_id(int uid);
void *dv_chain(void);
int dv_47dc(void);
int dv_08835924(void);
int dv_field_locate(int uid);
int dv_hand_find_uid(int player, int uid);
int dv_list_card_level(int player, int uid);
void dv_field_stats(int player, int zone, int *out);
int dv_zone_faceup(int player, int zone, int index);
int dv_slot_value_1909(int player, int zone);
int dv_slot_e68a0(int player, int zone);
int dv_slot_link_1a0c(int player, int zone);
int dv_slot_counter_1a90(int player, int zone);
int dv_slot_counter_1439(int player, int zone);
int dv_slot_counter_1513(int player, int zone);
int dv_slot_link(int player, int zone);
int dv_field_card(int player, int zone);
int dd_mode_get(void);

void dd_info_select(int id);
void dd_info_show_card(int which, int id, int keep);
void dd_info_hide(int peek);
void dd_info_stats(DdInfoView *v);
DdInfoView *dd_info_cur(void);
void dd_info_set_card(DdInfoView *v, int id);
void dd_info_set_uid(DdInfoView *v, int uid);
int dd_info_dual(void);

void dd_info_init(void)
{
    CardListCfg cfg;

    memset(&dd_info, 0, sizeof(DdInfo));
    memset(&cfg, 0, sizeof(cfg));
    cfg.gim = dd_gim_get(0x40004);
    cfg.digits_gim = dd_gim_get(0x40006);
    cfg.frame_ehf = dd_ehf_get(0x20001);
    cfg.attr_gim = dd_gim_get(0x40019);
    cfg.kind_gim = dd_gim_get(0x40018);
    cfg.cil_gim = dd_gim_get(0x40001);
    cfg.icon_gim = dd_gim_get(0x4001E);
    cfg.cip = dd_cards_cip_h();
    dd_info.frame_buf = cfg.frame_view = dd_alloc(ehf_get_u32_1c(cfg.frame_ehf));
    cardlist_init(&cfg);
    dd_info_select(-1);
}

void dd_info_term(void)
{
    DdInfo *in = &dd_info;

    cardlist_clear();
    if (in->frame_buf) {
        dd_free(in->frame_buf);
        in->frame_buf = 0;
    }
}

/* draws the panel(s) on layer 6 */
void dd_info_draw(void)
{
    DdInfo *in = &dd_info;
    void *list = dl_disable(dl_select_layer(6, 0), 1);

    switch (in->state) {
    case 0:
    case 1:
    case 2:
        list = cardlist_draw_at(list, in->x, 0, &in->view[0]);
        break;
    case 3:
    case 4:
    case 5:
        list = cardlist_draw_pair_at(list, in->x, 0, &in->view[0], &in->view[1]);
        break;
    }
    dl_set_list(dl_enable(list, 1));
}

/* opens the pair while the player browses the field, slides the panel */
void dd_info_update(float dt)
{
    DdInfo *in = &dd_info;
    char *sel;
    int uid;

    dd_info_stats(&in->view[0]);
    if (dd_info_dual())
        dd_info_stats(&in->view[1]);
    sel = dv_chain();
    uid = dv_47dc();
    if (uid > 0 && (*(int *)(sel + 0x598) || *(int *)(sel + 0x5A0)) &&
        ((!dd_08806BC4() && dv_08835924() >= 6) || dd_mode_get() == 2)) {
        if (!in->peek)
            dd_info_show_card(1, uid, 1);
    } else if (in->peek) {
        dd_info_hide(1);
    }
    switch (in->state) {
    case 0:
    case 3:
        in->x = 0;
        if (in->target != in->state) {
            in->state++;
            in->timer = 0.0f;
        }
        break;
    case 1:
    case 4:
        in->timer += dt;
        in->x = -(int)(120.0f * in->timer / 0.25f);
        if (in->timer > 0.25f) {
            if (in->target == 0)
                in->state = 2;
            else
                in->state = 5;
            in->timer = 0.0f;
        }
        break;
    case 2:
    case 5:
        in->timer += dt;
        in->x = -(int)(120.0f * (0.25f - in->timer) / 0.25f);
        if (in->timer > 0.25f) {
            in->state -= 2;
            in->timer = 0.0f;
        }
        break;
    }
}

/* scrolls the description of the panel(s) by d lines */
void dd_info_scroll(int d)
{
    DdInfo *in = &dd_info;
    int n = dd_info_dual() ? 2 : 1;
    int k;
    DdInfoView *v;

    for (k = 0, v = in->view; k < n; k++, v++) {
        if ((d < 0 && v->desc_scroll < (unsigned int)-d) || v->desc_max == 0) {
            v->desc_scroll = 0;
        } else {
            v->desc_scroll += d;
            if (v->desc_max < v->desc_scroll)
                v->desc_scroll = v->desc_max;
        }
    }
}

/* shows card `id` (by card id) in the current panel */
void dd_info_select(int id)
{
    DdInfoView *v = dd_info_cur();

    if (id != v->card || v->uid)
        dd_info_set_card(v, id);
}

/* shows the field card `uid` in the current panel */
void dd_info_select_uid(int uid)
{
    DdInfoView *v = dd_info_cur();

    if (v->uid != uid)
        dd_info_set_uid(v, uid);
}

/* opens the pair with card `id` in panel `which` and the current card in the other */
void dd_info_show_card(int which, int id, int keep)
{
    DdInfo *in = &dd_info;
    DdInfoView cur = *dd_info_cur();

    if (keep == 0) {
        in->pinned = 1;
    } else {
        if (!in->peek && in->pinned)
            memcpy(in->save, in->view, sizeof(in->view));
        in->peek = 1;
    }
    dd_info_set_card(&in->view[which], id);
    in->view[which].on = 1;
    if (cur.uid > 0) {
        dd_info_set_uid(&in->view[1 - which], cur.uid);
        in->view[1 - which].on = 0;
    } else {
        dd_info_set_card(&in->view[1 - which], cur.card);
        in->view[1 - which].on = 0;
    }
    in->target = 3;
}

/* same with the field card `uid` */
void dd_info_show_uid(int which, int uid, int keep)
{
    DdInfo *in = &dd_info;
    DdInfoView cur = *dd_info_cur();

    if (keep == 0) {
        in->pinned = 1;
    } else {
        in->peek = 1;
        if (in->pinned)
            memcpy(in->save, in->view, sizeof(in->view));
    }
    dd_info_set_uid(&in->view[which], uid);
    in->view[which].on = 1;
    if (cur.uid > 0) {
        dd_info_set_uid(&in->view[1 - which], cur.uid);
        in->view[1 - which].on = 0;
    } else {
        dd_info_set_card(&in->view[1 - which], cur.card);
        in->view[1 - which].on = 0;
    }
    in->target = 3;
}

#ifdef NON_MATCHING /* o teste de pinned sai como bnez + b com atrasos vazios no original; nenhuma forma testada reproduziu */
/* closes the pair opened by the player (peek == 0) or by browsing (peek != 0) */
void dd_info_hide(int peek)
{
    DdInfo *in = &dd_info;

    if (peek && !in->peek)
        return;
    if (!peek && !in->pinned)
        return;
    in->view[0].on = 0;
    in->view[1].on = 0;
    if (peek == 0) {
        in->pinned = 0;
    } else {
        in->peek = 0;
        if (in->pinned)
            memcpy(in->view, in->save, sizeof(in->view));
    }
    if (!dd_info_dual())
        in->target = 0;
}
#endif

void dd_info_disable(int off)
{
    DdInfo *in = &dd_info;

    dd_info.off = off;
    if (dd_info_dual())
        in->target = 3;
    else
        in->target = 0;
}

#ifdef NON_MATCHING /* a ordem das cargas/gravações da cópia de si[] para st difere (7 palavras) */
/* current values of the card of panel `v` on the field */
void dd_info_stats(DdInfoView *v)
{
    DdCardStats st;
    int si[11];
    int uid = v->uid;
    int mask;
    int id;
    int zone;
    int loc;
    int r;

    memset(&st, 0, sizeof(st));
    mask = 0;
    if (uid > 0) {
        id = dv_list_id(uid);
        loc = dv_field_locate(uid);
        if (loc < 0) {
            zone = -1;
            if ((r = dv_hand_find_uid(0, uid)) >= 0)
                zone = 0;
            else if ((r = dv_hand_find_uid(1, uid)) >= 0)
                zone = 1;
            if (zone >= 0 && r >= 0) {
                st.level = dv_list_card_level(zone, uid);
                mask = 0x40;
            }
        } else {
            uid = loc & 0xFF;
            zone = (unsigned char)((loc & 0xFFFF) >> 8);
            if (zone >= 0 && zone < 5) {
                dv_field_stats(uid, zone, si);
                st.type = si[1];
                st.level = si[4];
                st.def = si[6];
                st.kind = si[2];
                st.atk = si[5];
                v->cur.atk = si[7];
                v->cur.def = si[8];
                mask = 0x7C;
                if (v->cur.attr == 13 || v->cur.attr == 14) {
                    st.attr = 0;
                    st.attr_icon = 0;
                    mask |= 3;
                }
            }
            if (dv_zone_faceup(uid, zone, 0) && zone >= 0 && zone < 11) {
                st.extra[1] = dv_slot_value_1909(uid, zone);
                if (card_is_in_group_1909(id))
                    mask |= 0x200;
                st.extra[0] = dv_slot_e68a0(uid, zone);
                if (card_get_group_value_1983(id))
                    mask |= 0x100;
                st.extra[2] = dv_slot_link_1a0c(uid, zone);
                if (card_is_in_group_1a0c(id))
                    mask |= 0x400;
                if ((st.extra[3] = dv_slot_counter_1a90(uid, zone)) > 0)
                    mask |= 0x800;
                if ((st.extra[4] = dv_slot_counter_1439(uid, zone)) > 0)
                    mask |= 0x1000;
                if ((st.extra[5] = dv_slot_counter_1513(uid, zone)) > 0)
                    mask |= 0x2000;
                switch (id) {
                case 0x1357:
                case 0x1358:
                case 0x14CC:
                case 0x14D1:
                case 0x161C:
                    mask |= 0x4000;
                    st.mask = dv_slot_link(uid, zone);
                    break;
                case 0x171F:
                case 0x145B:
                    mask |= 0x8000;
                    st.mask = dv_slot_link(uid, zone);
                    break;
                case 0x19C2:
                    mask |= 0x10000;
                    st.mask = dv_slot_link(uid, zone);
                    break;
                case 0x1368:
                    mask |= 0x20000;
                    st.mask = dv_slot_link(uid, zone);
                    break;
                case 0x1A74:
                    mask |= 0x20000;
                    st.mask = dv_field_card(uid, zone);
                    break;
                }
            }
        }
    }
    v->cmp = st;
    v->show_mask = mask;
}
#endif

/* panel that shows the selected card: the second one while the pair is open */
DdInfoView *dd_info_cur(void)
{
    DdInfoView *v = dd_info.view;

    if (!dd_info_dual())
        return v;
    if (v->on)
        v++;
    return v;
}

void dd_info_set_card(DdInfoView *v, int id)
{
    if (id <= 0)
        id = -1;
    v->uid = 0;
    v->card = id;
    v->id = id;
    v->variant = inv_get_hi(id);
    v->vram = 0x160000;
    if (id == -1)
        return;
    v->cur.attr = card_get_attribute(id);
    v->cur.attr_icon = card_get_attribute_icon(id);
    v->cur.atk = card_get_atk_q(id) == 0xFFFF ? -1 : card_get_atk_q(id);
    v->cur.def = card_get_def_q(id) == 0xFFFF ? -1 : card_get_def_q(id);
    v->cur.type = card_get_type(id);
    v->cur.kind = card_get_kind(id);
    v->cur.level = card_get_level(id);
    v->cur.prop5 = card_get_prop5_hi(id);
    v->show_mask = 0;
    v->desc_scroll = 0;
    v->desc_max = 0;
}

void dd_info_set_uid(DdInfoView *v, int uid)
{
    v->uid = uid;
    v->id = v->card = dv_list_id(uid);
    v->variant = inv_get_hi(v->card);
    v->vram = 0x160000;
    v->cur.attr = card_get_attribute(v->card);
    v->cur.attr_icon = card_get_attribute_icon(v->card);
    v->cur.atk = card_get_atk_q(v->card) == 0xFFFF ? -1 : card_get_atk_q(v->card);
    v->cur.def = card_get_def_q(v->card) == 0xFFFF ? -1 : card_get_def_q(v->card);
    v->cur.type = card_get_type(v->card);
    v->cur.kind = card_get_kind(v->card);
    v->cur.level = card_get_level(v->card);
    v->cur.prop5 = card_get_prop5_hi(v->card);
    v->show_mask = 0;
    v->desc_scroll = 0;
    v->desc_max = 0;
    dd_info_stats(v);
}

/* 1 while the pair of panels is open */
int dd_info_dual(void)
{
    DdInfo *in = &dd_info;

    if (dd_info.off)
        return 0;
    if (in->pinned || in->peek)
        return 1;
    return 0;
}
