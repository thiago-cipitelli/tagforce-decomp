// @range 0x0881494C-0x088158B4
// @rodata 0x0882AF94-0x0882AFD8
/* Free duel over ad hoc: the player entries exchanged with the other machines. */
#include "libtitle.h"

unsigned short *tfd_partner_by_type();
unsigned short *wcsncpy(unsigned short *dst, const unsigned short *src, int n);
int peer_self_get(void);
void local_record_set(TFdPacket *p);
TFdPacket *peer_get(int i);
int peer_count_get(void);
int peer_self_byte1(void);
int tfd_get_seat(int i);
void tpt_set_name(short i, int kind, int type2, int src, int deck, void *name);
void tpt_set_all(short i, int kind, int type2, int src, int deck, void *name, void *deckname);
unsigned short *sysstate_get_name(void);
int peers_all_match(int a, int b, int c);
int peer_self_byte2(void);
int peer0_byte2(void);
void tfd_import_populate(TFdPacket *p);
int tfd_all_ready(TFdPacket *p);
void tfd_chr_copy_draw(RecipeData *dst, TFdDeck *src);
void netsync_transfer_setup(void *buf, void *src, int size);
int netsync_transfer(void);

void tfd_pt_send_packet(TFdPacket *pk, short who, short type, short type2, short kind, short deck, short side)
{
    TDuelCfg *d = &title.duel;
    TFdList *l = &d->list;
    TFdDeck *peer = &l->deck[d->u001];
    TFdDeck *own = &l->deck[d->u004];
    unsigned short *name;
    unsigned short otype;
    unsigned short ou76;
    unsigned short odk;
    unsigned short odeck;

    memset(pk, 0, sizeof(TFdPacket));
    pk->kind = 2;
    pk->ver = 2;
    pk->b2 = 0;
    if (who == peer_self_get()) {
        otype = own->type;
        ou76 = own->u76;
        odk = own->dk;
        odeck = own->deck;
        name = tfd_partner_by_type(type, kind, deck);
        if (name != 0) {
            wcsncpy(pk->name, name, 0x18);
            memset(&pk->name[0x17], 0, 2);
        }
        wcsncpy(&pk->w38, (unsigned short *)&side, 1);
        wcsncpy(&pk->w3a, &otype, 1);
        wcsncpy(&pk->w3c, &ou76, 1);
        wcsncpy(&pk->w3e, &odk, 1);
        wcsncpy(&pk->w40, &odeck, 1);
        wcsncpy(pk->name2, own->deckname, 0x18);
        memset(&pk->name2[0x17], 0, 2);
    } else {
        name = tfd_partner_by_type((short)peer->type, peer->dk, (short)peer->deck);
        wcsncpy(&pk->w38, (unsigned short *)&side, 1);
        if (name != 0) {
            wcsncpy(pk->name, name, 0x18);
            memset(&pk->name[0x17], 0, 2);
        }
        if (kind != 1 && (name = tfd_partner_by_type(type, kind, deck)) != 0) {
            wcsncpy(pk->name2, name, 0x18);
            memset(&pk->name2[0x17], 0, 2);
        }
        wcsncpy(&pk->w3a, (unsigned short *)&type, 1);
        wcsncpy(&pk->w3c, (unsigned short *)&type2, 1);
        wcsncpy(&pk->w3e, (unsigned short *)&kind, 1);
        wcsncpy(&pk->w40, (unsigned short *)&deck, 1);
    }
    local_record_set(pk);
}

void tfd_pt_recv_packet(void)
{
    TDuelCfg *d = &title.duel;
    TFdList *l = &d->list;
    Title *t = &title;
    short type;
    short type2;
    short kind;
    short deck;
    short side;
    int i;
    int skip;
    TFdPacket *pk;
    TFdDeck *dk;
    int seat;
    unsigned short *name;

    type = 0;
    type2 = 0;
    kind = 0;
    deck = 0;
    side = 0;
    skip = 0;
    if (d->opts.tag == 1) {
        switch (t->step) {
        case 11:
            if (t->step2 == 0)
                skip = 1;
            break;
        case 14:
            skip = 1;
            break;
        case 15:
            if (peer_self_byte1() != 2)
                skip = 1;
            break;
        default:
            if (t->step < 11)
                skip = 1;
            break;
        }
    }
    for (i = 0; i < peer_count_get(); i++) {
        if (!skip || (skip && i != peer_self_get())) {
            pk = peer_get(i);
            dk = &l->deck[i];
            if (pk->kind == 2) {
                wcsncpy((unsigned short *)&side, &pk->w38, 1);
                dk->side = side;
                dk->type = 1;
                dk->u76 = i;
                seat = tfd_get_seat(i);
                wcsncpy(dk->deckname, pk->name, 0x18);
                tpt_set_all(seat, 1, i, 3, -1, dk->u04, dk->deckname);
                wcsncpy((unsigned short *)&type, &pk->w3a, 1);
                if (type != 0 && i < 2) {
                    dk = &l->deck[i + 2];
                    name = 0;
                    wcsncpy((unsigned short *)&type2, &pk->w3c, 1);
                    wcsncpy((unsigned short *)&kind, &pk->w3e, 1);
                    wcsncpy((unsigned short *)&deck, &pk->w40, 1);
                    if (kind == 0 || kind == 2) {
                        wcsncpy(dk->deckname, pk->name2, 0x18);
                        name = dk->deckname;
                    }
                    dk->side = (side + 2) % 4;
                    dk->type = type;
                    dk->u76 = type2;
                    dk->dk = kind;
                    dk->deck = deck;
                    seat = (seat + 2) % 4;
                    if (kind == 0 || kind == 2)
                        kind = 3;
                    if (kind == 3)
                        tpt_set_all(seat, type, type2, kind, deck, 0, name);
                    else
                        tpt_set_name(seat, type, type2, kind, deck, 0);
                }
            }
        }
    }
}

int tfd_comm_machine(void)
{
    Title *t = &title;
    TDuelCfg *d = &title.duel;
    TFdList *l = &d->list;
    TDuelOpts *o = &d->opts;
    TFdPacket *pk = &l->pkt;
    TFdDeck *e;
    int i;
    int n;

    pad_get();
    d->u001 = peer_self_get();
    title.duel.list.n = peer_count_get();
    switch (d->u005) {
    case 0:
        memset(pk, 0, sizeof(TFdPacket));
        pk->kind = 1;
        pk->ver = 1;
        pk->b2 = 0;
        wcsncpy(pk->name, sysstate_get_name(), 15);
        local_record_set(pk);
        d->u005++;
        break;
    case 1:
        if (peers_all_match(1, 4, 0xFF)) {
            tfd_import_populate(pk);
            pk->ver = 2;
            local_record_set(pk);
            d->u005++;
        }
        break;
    case 2:
        if (o->kind == 0) {
            if (!o->tag) {
                if (peer_self_get() == 0) {
                    if (peers_all_match(1, 2, 0xFF)) {
                        pk->kind = 2;
                        pk->ver = 1;
                        local_record_set(pk);
                        d->u005 = 3;
                    }
                } else if (peer_get(0)->kind == 2) {
                    pk->kind = 2;
                    pk->ver = 1;
                    local_record_set(pk);
                    d->u005 = 3;
                }
            } else {
                if (peer_self_get() == 0) {
                    if (peers_all_match(1, 2, 0xFF)) {
                        pk->kind = 2;
                        pk->ver = 1;
                        local_record_set(pk);
                        d->u005 = 4;
                    }
                } else if (peer_get(0)->kind == 2) {
                    pk->kind = 2;
                    pk->ver = 1;
                    local_record_set(pk);
                    d->u005 = 4;
                }
            }
        }
        break;
    case 3:
        tfd_pt_send_packet(pk, d->u001, l->deck[d->u001].type, l->deck[d->u001].u76, l->deck[d->u001].dk,
                           (short)l->deck[d->u001].deck, l->deck[d->u001].side);
        d->u005 = 4;
        break;
    case 4:
        tfd_pt_recv_packet();
        tfd_all_ready(pk);
        if (t->step == 14) {
            tfd_pt_send_packet(pk, d->u001, l->deck[d->u001].type, l->deck[d->u001].u76, l->deck[d->u001].dk,
                               (short)l->deck[d->u001].deck, l->deck[d->u001].side);
            t->step = 15;
        } else if (t->step == 15) {
            if (peer_self_byte1() == 2) {
                pk->kind = 2;
                pk->ver = 1;
                local_record_set(pk);
                t->step = 16;
            }
        } else if (t->step == 0x2B || t->step == 0x11) {
            tfd_get_seat(d->u000);
            tfd_pt_send_packet(pk, d->u000, l->sel.type, l->sel.u02, l->sel.dk, (short)l->sel.deck,
                               l->deck[d->u001].side);
            if (t->step == 0x2B)
                t->step = 0x2C;
            else
                t->step = 0x12;
        } else if (t->step == 0x2C || t->step == 0x12) {
            if (peer_self_byte1() == 2) {
                tfd_get_seat(d->u000);
                e = &l->deck[d->u000];
                e->type = l->sel.type;
                e->u76 = l->sel.u02;
                e->dk = l->sel.dk;
                e->deck = l->sel.deck;
                pk->kind = 2;
                pk->ver = 1;
                local_record_set(pk);
                if (t->step == 0x2C)
                    t->step = 0x2D;
                else
                    t->step = 0x13;
            }
        } else if (t->step == 0x2F) {
            tfd_get_seat(d->u000);
            tfd_pt_send_packet(pk, d->u000, l->sel.type, l->sel.u02, l->sel.dk, (short)l->sel.deck,
                               l->deck[d->u001].side);
            t->step = 0x30;
        } else if (t->step == 0x30) {
            if (peer_self_byte1() == 2) {
                tfd_get_seat(d->u000);
                e = &l->deck[d->u000];
                e->type = l->sel.type;
                e->u76 = l->sel.u02;
                e->dk = l->sel.dk;
                e->deck = l->sel.deck;
                pk->kind = 2;
                pk->ver = 1;
                local_record_set(pk);
                t->step = 0x31;
            }
        } else if (t->step == 0x16) {
            pk->ver = 2;
            pk->b2 = 1;
            local_record_set(pk);
            t->step = 0x17;
        } else if (t->step == 0x17) {
            pk->ver = 2;
            pk->b2 = 1;
            local_record_set(pk);
            if (peer_self_byte1() == 2 && peer_self_byte2() == 1) {
                t->step = 0x18;
                d->u005 = 5;
            }
        }
        break;
    case 5:
        pk->ver = 2;
        pk->b2 = 1;
        local_record_set(pk);
        tfd_pt_recv_packet();
        if (tfd_all_ready(pk)) {
            d->u002 = d->u001;
            d->u005 = 7;
        }
        break;
    case 6:
        if (peer_self_get() == 0) {
            if (peers_all_match(2, 2, 2)) {
                pk->b2 = 3;
                local_record_set(pk);
                d->u002 = d->u001;
                d->u005++;
            }
        } else if (peer0_byte2() == 3) {
            pk->b2 = 3;
            local_record_set(pk);
            d->u002 = d->u001;
            d->u005++;
        }
        break;
    case 7:
        e = &l->deck[d->u002];
        l->mark = heap_alloc(t->heap, 0x300);
        l->buf = heap_alloc(t->heap, 0xC00);
        tfd_chr_copy_draw(&d->send[0], e);
        if (o->tag && d->u004b4 == 1)
            tfd_chr_copy_draw(&d->send[1], &l->deck[d->u004]);
        netsync_transfer_setup(l->buf, d->send, 0x300);
        d->u005++;
        break;
    case 8:
        if (netsync_transfer()) {
            for (i = 0; i < peer_count_get(); i++)
                memcpy(&l->recv[i], l->buf + i * 0x300, sizeof(RecipeData));
            for (i = peer_count_get(); i < 4; i++)
                memcpy(&l->recv[i], l->buf + ((i - 2) * 0x300 + sizeof(RecipeData)), sizeof(RecipeData));
            heap_set_level(t->heap, l->mark);
            heap_set_level(t->heap, l->buf);
            d->u005 = 15;
        }
        break;
    case 9:
        pk->kind = 2;
        pk->ver = 2;
        pk->b2 = 12;
        local_record_set(pk);
        if (peers_all_match(2, 2, 12))
            d->u005 = 10;
        break;
    case 10:
        if (o->tag) {
            if (d->u004b4 == 1) {
                d->u002 = d->u004;
                d->u005 = 11;
            } else {
                pk->ver = 1;
                pk->kind = 7;
                pk->b2 = 0;
                local_record_set(pk);
                d->u005 = 12;
            }
        } else {
            d->u005 = 15;
        }
        break;
    case 11:
        l->mark = heap_alloc(t->heap, 0x180);
        l->buf = heap_alloc(t->heap, 0x600);
        d->u005++;
        break;
    case 12:
        if (netsync_transfer()) {
            for (n = peer_count_get(); n < 4; n++)
                memcpy(&l->recv[n], l->buf + (n - 2) * 0x180, sizeof(RecipeData));
            heap_set_level(t->heap, l->mark);
            heap_set_level(t->heap, l->buf);
            d->u005 = 13;
        }
        break;
    case 13:
        pk->kind = 2;
        pk->ver = 2;
        pk->b2 = 14;
        local_record_set(pk);
        if (peers_all_match(2, 2, 14))
            d->u005 = 15;
        break;
    case 14:
    case 15:
    case 16:
        break;
    }
    return 0;
}
