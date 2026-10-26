// @range 0x0881494C-0x08814F08
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
