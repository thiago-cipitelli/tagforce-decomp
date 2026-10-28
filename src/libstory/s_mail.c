// @range 0x088132FC-0x088138AC
// @asm 0x088134BC-0x088136E8
// @asm 0x088136E8-0x08813874
// @asm 0x08813874-0x08813894
/* Mails of the PDA: the received list in the save (100 records of 6 bytes: mail id, new/read flags,
   sender, date), the mail table of brain.ehp (MailESM) and its texts (MailEL/MailEI), the view list
   built for the PDA and the attachments of an opened mail. */
#include "libstory.h"

typedef struct StMail {                /* received mail, 6 bytes */
    unsigned short id : 10;            /* +0x00 mail id, 0 ends the list */
    unsigned short isnew : 1;          /* +0x01 bit 2 */
    unsigned short read : 1;           /* +0x01 bit 3 */
    unsigned short : 4;
    unsigned short from : 10;          /* +0x02 sender (st_id_to_rec) */
    unsigned short month : 4;          /* +0x03 bits 2..5, calendar month */
    unsigned short : 2;
    unsigned char day : 6;             /* +0x04 */
    unsigned char : 2;
    unsigned char pad;
} StMail;

typedef struct StMailView {            /* line of the PDA mail list, 12 bytes */
    StMail m;                          /* +0x00 */
    short pad;                         /* +0x06 */
    unsigned short *subject;           /* +0x08 */
} StMailView;

typedef struct StMailAttach {          /* attachment of an opened mail, 8 bytes */
    unsigned char kind;                /* +0x00 */
    unsigned char id;                  /* +0x01 */
    unsigned short x02;                /* +0x02 */
    unsigned short *text;              /* +0x04 */
} StMailAttach;

typedef struct StMailTables {          /* 0x08ACB020 */
    unsigned short *esm;               /* +0x00 MailESM.bin: records of 5 halfwords from the mail id */
    unsigned short *base;              /* +0x04 MailEL: texts */
    int *index;                        /* +0x08 MailEI: offset (in halfwords) of each text */
} StMailTables;

extern StMailAttach st_mail_attach[256];        /* 0x08AC936C */
extern unsigned short st_mail_attach_text[32][64]; /* 0x08AC9B6C */
extern StMailView st_mail_view[100];            /* 0x08ACAB6C */
extern StMail *st_mail_list;                    /* 0x08ACB01C in the save */
extern StMailTables st_mail_tbl;                /* 0x08ACB020 */

int st_id_to_rec(int id);
int st_remap_812334(int id);
int scal_get_cal_month(void);
int scal_get_day(void);
int scal_daynum_cal(ScalDate *d);
void st_pl_x2c_inc(void);
void st_pl_x2c_dec(void);
unsigned short *st_text_at(int i, int *index, unsigned short *base);
unsigned short *st_wcs_expand(unsigned short *dst, unsigned short *src, int n);
unsigned short *st_mail_find(int id);
int *st_mail_index(void);
unsigned short *st_mail_base(void);

void st_init_8132fc(void)
{
    memset(&st_mail_tbl, 0, sizeof(StMailTables));
}

void st_term_813310(void)
{
    memset(&st_mail_tbl, 0, sizeof(StMailTables));
}

void st_mail_set_list(StMail *list)
{
    st_mail_list = list;
}

void st_mail_clear(int i)
{
    memset(&st_mail_list[i], 0, sizeof(StMail));
}

void st_mail_set_esm(unsigned short *esm)
{
    st_mail_tbl.esm = esm;
}

void st_mail_set_text(unsigned short *base, int *index)
{
    st_mail_tbl.base = base;
    st_mail_tbl.index = index;
}

/* receives a mail (once): new, unread, sender and today's date, then tells the field */
void st_mail_receive(int id)
{
    StMail *m = st_mail_list;
    int i;
    int arg;

    for (i = 0; i < 100; i++, m++) {
        if (m->id == id)
            return;
        if (m->id == 0) {
            m->id = id;
            m->isnew = 1;
            m->read = 0;
            m->from = st_id_to_rec(st_mail_find(id)[6]);
            m->month = scal_get_cal_month();
            m->day = scal_get_day();
            st_pl_x2c_inc();
            fld_cmd_start(0x12, &arg);
            return;
        }
    }
}

int st_mail_has(int id)
{
    int i;
    StMail *m = st_mail_list;

    for (i = 0; i < 100; i++, m++) {
        if (m->id == id)
            return 1;
    }
    return 0;
}

#ifdef NON_MATCHING /* forma igual; a alocação de $s (m em s0, v em s1) e a ordem das cópias de bitfields diferem */
/* list of the dated mails for the PDA; a mail stops being new 3 days after it arrived */
StMailView *st_mail_view_list(int *count)
{
    int d;
    StMailView *v = st_mail_view;
    int i;
    int n = 0;
    StMail *m;

    int month, day;
    unsigned short *subject;
    ScalDate sent;
    ScalDate now;

    *count = 0;
    for (i = 0; i < 100; i++) {
        m = &st_mail_list[i];
        if (m->id == 0)
            break;
        month = m->month;
        if (month == 0)
            continue;
        day = m->day;
        if (day == 0)
            continue;
        if (m->isnew) {
            sent.cal_month = month;
            sent.day = day;
            now.cal_month = scal_get_cal_month();
            now.day = scal_get_day();
            d = scal_daynum_cal(&sent);
            if (scal_daynum_cal(&now) - d >= 3)
                m->isnew = 0;
        }
        subject = st_text_at(st_mail_find(m->id)[9], st_mail_index(), st_mail_base());
        v->m.id = m->id;
        v->m.isnew = m->isnew;
        v->m.read = m->read;
        v->m.from = m->from;
        v->m.month = m->month;
        n++;
        v->m.day = m->day;
        v->subject = subject;
        v++;
    }
    *count = n;
    return st_mail_view;
}
#endif

#ifdef NON_MATCHING /* o original endereça cada campo do terminador com lui/addiu próprio e guarda id em s6; alocação de $s diferente */
/* attachments of a mail (ended by a zeroed entry); marks the mail read */
StMailAttach *st_mail_open(int id)
{
    unsigned short *p = st_mail_find(id) + 10;
    int i = 0;
    StMail *m;
    int k;
    StMailAttach *a = st_mail_attach;
    unsigned short *text = st_mail_attach_text[0];

    for (;;) {
        if (*p == 0) {
            st_mail_attach[i].kind = 0;
            st_mail_attach[i].id = 0;
            st_mail_attach[i].x02 = 0;
            st_mail_attach[i].text = 0;
            break;
        }
        a->kind = st_remap_812334(p[2]);
        a->id = p[0];
        a->x02 = 0;
        k = p[4];
        st_wcs_expand(text, st_text_at(k, st_mail_index(), st_mail_base()), 0x40);
        a->text = text;
        a++;
        text += 0x40;
        i++;
        p += 5;
    }
    m = st_mail_list;
    for (i = 0; i < 100; i++, m++) {
        if (m->id == id) {
            if (!m->read) {
                st_pl_x2c_dec();
                m->read = 1;
            }
            break;
        }
    }
    return st_mail_attach;
}
#endif

#ifdef NON_MATCHING /* o original materializa &st_mail_tbl (lui/addiu) antes do lw; o mwcc sempre dobra o deslocamento */
/* record of a mail in MailESM (assumed present) */
unsigned short *st_mail_find(int id)
{
    unsigned short *p = ((StMailTables *)(char *)&st_mail_tbl)->esm;

next:
    if (*p != id) {
        p += 5;
        goto next;
    }
    return p;
}
#endif

int *st_mail_index(void)
{
    return st_mail_tbl.index;
}

unsigned short *st_mail_base(void)
{
    return st_mail_tbl.base;
}
