// @range 0x08812728-0x088132FC
// @data 0x0881D7E8-0x0881D9FC
/* Story text packs: script/st0N_t01_brain.ehp (scripts, steps, decks, schedules of each character and
   the tables of the field, items, cards, mails, lessons and mornings) and its per-language pack
   (st0N_t01_<lang>.ehp, the messages); the per-character and per-event message files loaded on demand. */
#include "libstory.h"

typedef struct StMsg {                 /* 0x08AC935C */
    void *ehp;                         /* +0x00 brain.ehp */
    void *ehp_lang;                    /* +0x04 <lang>.ehp */
    int lang;                          /* +0x08 index in st_lang_suffix */
    int route;                         /* +0x0C */
} StMsg;

extern StMsg st_msg;

void *ehp_find(void *pack, const char *name, int *size);
int st_remap_812400(int id);
void *st_file_load(const char *path, int heap);
void *st_ehp_load(void *ehp, const char *name, int heap);
void st_res_free(void *p, int heap);
void st_scr_set_g(int i, void *p, void *s, void *c, void *g);
void st_scr_set_step(int i, void *p);
void st_scr_set_deck(int i, void *p);
void st_scr_set_dh(int i, void *p);
void st_scr_set_dn(int i, void *p);
void st_scr_set_sche(int i, void *p);
void st_scr_set_es(int i, void *p);
void st_scr_set_ei(int i, void *p);
void st_scr_set_el(int i, void *p);
void *st_scr_get_es(int i);
void *st_scr_get_ei(int i);
void *st_scr_get_el(int i);
void stt_set_c(void *l, void *i);
void stt_set_a(void *tag, void *l, void *i);
void stt_set_b(void *tag, void *l, void *i);
void stt_set_branch(void *con, void *pos);
void sev_set_table(void *p);
void st_pickup_set_table(void *p);
void st_mail_set_esm(void *p);
void st_mail_set_text(void *l, void *i);
void st_lesson_set_plan(void *p);
void st_lesson_set_esm(void *p);
void st_lesson_set_text(void *l, void *i);
void st_morning_set_plan(void *p);
void st_morning_set_esm(void *p);
void st_morning_set_text(void *l, void *i);
void st_iso_set_es(void *p);
void st_iso_set_ei(void *p);
void st_iso_set_el(void *p);
void *st_iso_get_es(void);
void *st_iso_get_ei(void);
void *st_iso_get_el(void);

void st_msg_unload(int kind);
void st_script_dir(char *buf, int route);
void *st_ehp_find_num(void *ehp, const char *prefix, int i, const char *a, const char *b, const char *ext);
void st_make_name(char *buf, const char *prefix, int i, const char *a, const char *b, const char *ext);
void *st_ehp_find_lang(const char *name);

char *st_lang_suffix[] = { "j", "e", "g", "f", "i", "s" };

void st_init_812728(void)
{
    memset(&st_msg, 0, sizeof(StMsg));
}

void st_term_81273c(void)
{
    st_msg_unload(0);
    st_msg_unload(1);
    st_msg_unload(2);
    memset(&st_msg, 0, sizeof(StMsg));
}

/* loads the brain pack and the language pack of a route */
int st_msg_open(int route)
{
    StMsg *m = &st_msg;
    char path[32];
    char lang_path[32];

    memset(path, 0, 0x20);
    memset(lang_path, 0, 0x20);
    st_msg.lang = st_remap_812400(cfg_get_language());
    st_script_dir(path, route);
    strcpy(lang_path, path);
    strcat(path, "brain.ehp");
    strcat(lang_path, st_lang_suffix[st_msg.lang]);
    strcat(lang_path, ".ehp");
    if ((st_msg.ehp = st_file_load(path, 0)) == 0)
        return 0;
    return (m->ehp_lang = st_file_load(lang_path, 0)) != 0;
}

/* hands every table of the two packs to its subsystem */
int st_msg_load_all(void)
{
    StMsg *m = &st_msg;
    void *gs;
    void *gc;
    int i;
    void *gp;

    if (st_msg.ehp == 0)
        return 0;
    if (m->ehp_lang == 0)
        return 0;
    for (i = 0; i < 150; i++) {
        st_scr_set_step(i, st_ehp_find_num(m->ehp, "Scr", i, 0, "Step", ".bin"));
        st_scr_set_deck(i, st_ehp_find_num(m->ehp, "Scr", i, 0, "Deck", ".bin"));
        st_scr_set_dh(i, st_ehp_find_num(m->ehp, "Scr", i, 0, "DH", ".bin"));
        st_scr_set_dn(i, st_ehp_find_num(m->ehp, "Scr", i, 0, "DN", ".bin"));
        st_scr_set_sche(i, st_ehp_find_num(m->ehp, "Scr", i, 0, "Sche", ".bin"));
        gp = st_ehp_find_num(m->ehp, "Scr", i, "G", "P", ".bin");
        gs = st_ehp_find_num(m->ehp, "Scr", i, "G", "S", ".bin");
        gc = st_ehp_find_num(m->ehp, "Scr", i, "G", "C", ".bin");
        st_scr_set_g(i, gp, gs, gc, st_ehp_find_num(m->ehp, "Scr", i, "G", "G", ".bin"));
    }
    stt_set_c(st_ehp_find_lang("SysMsgL"), st_ehp_find_lang("SysMsgI"));
    stt_set_a(ehp_find(m->ehp, "ImpChrMsgScrTag.bin", 0), st_ehp_find_lang("ImpChrMsgL"),
        st_ehp_find_lang("ImpChrMsgI"));
    stt_set_b(ehp_find(m->ehp, "ImpMapMsgScrTag.bin", 0), st_ehp_find_lang("ImpMapMsgL"),
        st_ehp_find_lang("ImpMapMsgI"));
    stt_set_branch(ehp_find(m->ehp, "FieldCon.bin", 0), ehp_find(m->ehp, "FieldPos.bin", 0));
    sev_set_table(ehp_find(m->ehp, "ItemCon.bin", 0));
    st_pickup_set_table(ehp_find(m->ehp, "CardCon.bin", 0));
    st_mail_set_esm(ehp_find(m->ehp, "MailESM.bin", 0));
    st_mail_set_text(st_ehp_find_lang("MailEL"), st_ehp_find_lang("MailEI"));
    st_lesson_set_plan(ehp_find(m->ehp, "LessonLesPlan.bin", 0));
    st_lesson_set_esm(ehp_find(m->ehp, "LessonESM.bin", 0));
    st_lesson_set_text(st_ehp_find_lang("LessonEL"), st_ehp_find_lang("LessonEI"));
    st_morning_set_plan(ehp_find(m->ehp, "MorningMorPlan.bin", 0));
    st_morning_set_esm(ehp_find(m->ehp, "MorningESM.bin", 0));
    st_morning_set_text(st_ehp_find_lang("MorningEL"), st_ehp_find_lang("MorningEI"));
    return 1;
}

/* message files of a character: ScrNNNESM, ScrNNNEI<lang>, ScrNNNEL<lang> (.bin.gz) */
void st_msg_load_chr(int i)
{
    StMsg *m = &st_msg;
    char name[32];

    if (st_msg.ehp != 0 && m->ehp_lang != 0) {
        st_make_name(name, "Scr", i, "ES", "M", ".bin.gz");
        st_scr_set_es(i, st_ehp_load(m->ehp, name, 1));
        st_make_name(name, "Scr", i, "EI", st_lang_suffix[m->lang], ".bin.gz");
        st_scr_set_ei(i, st_ehp_load(m->ehp_lang, name, 1));
        st_make_name(name, "Scr", i, "EL", st_lang_suffix[m->lang], ".bin.gz");
        st_scr_set_el(i, st_ehp_load(m->ehp_lang, name, 1));
    }
}

/* message files of an event: IsoNNNESM, IsoNNNEI<lang>, IsoNNNEL<lang> (.bin.gz) */
void st_msg_load_iso(int i)
{
    StMsg *m = &st_msg;
    char name[32];

    if (st_msg.ehp != 0 && m->ehp_lang != 0) {
        st_make_name(name, "Iso", i, "ES", "M", ".bin.gz");
        st_iso_set_es(st_ehp_load(m->ehp, name, 2));
        st_make_name(name, "Iso", i, "EI", st_lang_suffix[m->lang], ".bin.gz");
        st_iso_set_ei(st_ehp_load(m->ehp_lang, name, 2));
        st_make_name(name, "Iso", i, "EL", st_lang_suffix[m->lang], ".bin.gz");
        st_iso_set_el(st_ehp_load(m->ehp_lang, name, 2));
    }
}

/* kind 0: the two packs, 1: the files of every character, 2: the event files */
void st_msg_unload(int kind)
{
    StMsg *m = &st_msg;
    void *p;
    int i;

    switch (kind) {
    default:
        if (m->ehp != 0) {
            st_res_free(m->ehp, 0);
            m->ehp = 0;
        }
        if (m->ehp_lang != 0) {
            st_res_free(m->ehp_lang, 0);
            m->ehp_lang = 0;
        }
        break;
    case 1:
        for (i = 0; i < 150; i++) {
            if ((p = st_scr_get_es(i)) != 0) {
                st_res_free(p, 1);
                st_scr_set_es(i, 0);
            }
            if ((p = st_scr_get_ei(i)) != 0) {
                st_res_free(p, 1);
                st_scr_set_ei(i, 0);
            }
            if ((p = st_scr_get_el(i)) != 0) {
                st_res_free(p, 1);
                st_scr_set_el(i, 0);
            }
        }
        break;
    case 2:
        if ((p = st_iso_get_es()) != 0) {
            st_res_free(p, 2);
            st_iso_set_es(0);
        }
        if ((p = st_iso_get_ei()) != 0) {
            st_res_free(p, 2);
            st_iso_set_ei(0);
        }
        if ((p = st_iso_get_el()) != 0) {
            st_res_free(p, 2);
            st_iso_set_el(0);
        }
        break;
    }
}

void st_script_dir(char *buf, int route)
{
    switch (route) {
    case 1:
    default:
        strcat(buf, "script/st01_t01_");
        break;
    case 2:
    case 3:
        strcat(buf, "script/st02_t01_");
        break;
    case 4:
        strcat(buf, "e3/script/st01_t01_");
        break;
    }
}

int story_route_index(void)
{
    StMsg *m = &st_msg;

    if (st_msg.route < 0 || m->route >= 2)
        return 0;
    return m->route;
}

void *st_ehp_find_num(void *ehp, const char *prefix, int i, const char *a, const char *b, const char *ext)
{
    char name[32];

    st_make_name(name, prefix, i, a, b, ext);
    return ehp_find(ehp, name, 0);
}

/* prefix + %03d (i + 1) + a + b + ext */
void st_make_name(char *buf, const char *prefix, int i, const char *a, const char *b, const char *ext)
{
    char num[16];

    memset(num, 0, 0x10);
    memset(buf, 0, 0x20);
    strcpy(buf, prefix);
    sprintf(num, "%03d", i + 1);
    strcat(buf, num);
    if (a != 0)
        strcat(buf, a);
    if (b != 0)
        strcat(buf, b);
    strcat(buf, ext);
}

/* name + <lang> + ".bin" in the language pack */
void *st_ehp_find_lang(const char *name)
{
    char buf[64];

    memset(buf, 0, 0x40);
    strcpy(buf, name);
    strcat(buf, st_lang_suffix[st_msg.lang]);
    strcat(buf, ".bin");
    return ehp_find(st_msg.ehp_lang, buf, 0);
}
