// @range 0x08813C84-0x088156A4
// @asm 0x08815058-0x08815438
/* Story dialogs: the message window and the bust-ups of the speakers (two at most, placed on the
   left/centre/right slots) driven through the libfield commands 0xE (open), 0xF (animation),
   0x10 (close), 0x13 (message) and 0x14 (choice), and the expansion of the message text:
   $Mnnnn card names, 《Cnnn?v》/《Mnnn?v》 character names and 《PLAYER》 the player name. */
#include "libstory.h"

int st_id_to_chr(int id);
int st_id_to_rec(int id);
int st_chr_costume(int chr);
unsigned short *st_chr_text_a(int chr);
unsigned short *stt_get_a(int i);
unsigned short *stt_get_b(int i);
void st_fld_wait_idle(void);
unsigned short *sysstate_get_name(void);
unsigned short *card_get_name(int id);
unsigned short *chara_get_name(int i, int j);
int wcslen(const unsigned short *s);
int wcschr_index(const unsigned short *s, unsigned short c);
unsigned short *wcscpy(unsigned short *d, const unsigned short *s);
unsigned short *wcsncpy(unsigned short *d, const unsigned short *s, unsigned int n);
unsigned short *wcscat(unsigned short *d, const unsigned short *s);
unsigned short *wcsncat(unsigned short *d, const unsigned short *s, unsigned int n);

/* the remap tables of the text tables: (id, variant, text index) triples ending with 0, 0, 0 */
unsigned short *stt_count_a();
unsigned short *stt_count_b();

unsigned short *stext_expand(unsigned short *dst, unsigned short *src, int n);
int stalk_bu_add(StalkList *out, int id, int chara, int anim, int pos);
int stalk_bu_add2(StalkList *out, int id1, int chara1, int anim1, int id2, int chara2, int anim2);
int stalk_bu_speak(StalkList *out, int id, int chara, int anim, int pos);
void stalk_bu_close_all(void);
void stext_cards(unsigned short *text);
void stext_names(unsigned short *text);
void stext_player(unsigned short *dst);
void stext_replace(unsigned short *text, unsigned short *key, unsigned short *val);

void stalk_init(void)
{
    memset(&stalk, 0, sizeof(StalkState));
}

void stalk_term(void)
{
    memset(&stalk, 0, sizeof(StalkState));
}

/* a line of character id: its bust-up comes in (or speaks) with anim, then the message window;
   no_speaker leaves the window without a speaking slot */
void stalk_say(int id, int anim, int no_speaker, int pos, unsigned short *text)
{
    StalkState *t = &stalk;
    int chr;
    int chara;
    StalkList open;
    int r1;
    StalkMsg msg;
    int r2;
    StalkBuAnim a;
    StalkList list;
    int r3;

    chr = st_id_to_chr(id);
    chara = st_chr_costume(chr);
    if (chara <= 0) {
        stext_expand(t->text, text, 0x100);
        msg.text = t->text;
        msg.title = st_chr_text_a(chr);
        msg.speaker = -1;
    } else {
        if (stalk_bu_speak(&open, id, chara, anim, pos)) {
            if (fld_cmd_start(0xE, &open) == 0)
                return;
            while (!fld_cmd_poll(0xE, &r1))
                sys_frame_wait();
        } else {
            a.index = t->talk_pos;
            list.items = &a;
            list.n = 1;
            a.anim = anim;
            if (fld_cmd_start(0xF, &list) == 0)
                return;
            while (!fld_cmd_poll(0xF, &r3))
                sys_frame_wait();
        }
        if (no_speaker)
            no_speaker = -1;
        else
            no_speaker = t->talk_pos;
        stext_expand(t->text, text, 0x100);
        msg.text = t->text;
        msg.title = st_chr_text_a(chr);
        msg.speaker = no_speaker;
    }
    if (fld_cmd_start(0x13, &msg)) {
        while (!fld_cmd_poll(0x13, &r2))
            sys_frame_wait();
    }
}

/* bust-up of character id comes in at slot pos */
void stalk_bu_enter(int id, int anim, int pos)
{
    int chara;
    StalkList list;
    int r;

    chara = st_chr_costume(st_id_to_chr(id));
    if (chara > 0 && stalk_bu_add(&list, id, chara, anim, pos) && fld_cmd_start(0xE, &list)) {
        while (!fld_cmd_poll(0xE, &r))
            sys_frame_wait();
    }
}

/* two bust-ups come in at once (left and right) */
void stalk_bu_enter2(int id1, int anim1, int id2, int anim2)
{
    int chr1;
    int chr2;
    int chara1;
    int chara2;
    StalkList list;
    int r;

    chr1 = st_id_to_chr(id1);
    chr2 = st_id_to_chr(id2);
    chara1 = st_chr_costume(chr1);
    chara2 = st_chr_costume(chr2);
    if (chara1 > 0 && chara2 > 0 && stalk_bu_add2(&list, id1, chara1, anim1, id2, chara2, anim2)
        && fld_cmd_start(0xE, &list)) {
        while (!fld_cmd_poll(0xE, &r))
            sys_frame_wait();
    }
}

/* bust-up of character id goes out (no wait) */
void stalk_bu_hide(int id)
{
    StalkState *t = &stalk;
    int k = -1;
    int i;
    StalkList list;
    int pos[2];

    for (i = 0; i < 2; i++) {
        if (t->chr[i] == id) {
            k = i;
            break;
        }
    }
    if (k >= 0) {
        pos[0] = t->bu[k].index;
        list.items = pos;
        list.n = 1;
        if (fld_cmd_start(0x10, &list)) {
            t->bu[k].chara = 0;
            t->bu[k].index = 0;
            t->bu[k].anim = 0;
            t->chr[k] = 0;
        }
    }
}

void stalk_bu_hide_all(void)
{
    stalk_bu_close_all();
}

/* every bust-up goes out (no wait) */
void stalk_bu_close_all(void)
{
    StalkState *t = &stalk;
    int i;
    int n = 0;
    StalkList list;
    int pos[2];

    for (i = 0; i < 2; i++) {
        if (t->chr[i] != 0)
            n++;
    }
    switch (n) {
    case 0:
        return;
    case 1:
        list.n = 1;
        pos[0] = t->bu[0].index;
        list.items = pos;
        break;
    case 2:
        list.n = 2;
        pos[0] = t->bu[0].index;
        pos[1] = t->bu[1].index;
        list.items = pos;
        break;
    }
    if (fld_cmd_start(0x10, &list)) {
        t->bu[0].chara = 0;
        t->bu[0].index = 0;
        t->bu[0].anim = 0;
        t->chr[0] = 0;
        t->bu[1].chara = 0;
        t->bu[1].index = 0;
        t->bu[1].anim = 0;
        t->chr[1] = 0;
        t->talk_pos = -1;
    }
}

void stalk_bu_wait(void)
{
    int r;

    while (!fld_cmd_poll(0x10, &r))
        sys_frame_wait();
}

/* message window with text, titled with the name of character id (0: no title) */
void stalk_show(unsigned short *text, int id)
{
    StalkState *t = &stalk;
    unsigned short *title;
    StalkMsg msg;
    int r;

    if (id != 0)
        title = st_chr_text_a(st_id_to_chr(id));
    else
        title = 0;
    stext_expand(t->text, text, 0x100);
    msg.speaker = -1;
    msg.text = t->text;
    msg.title = title;
    if (fld_cmd_start(0x13, &msg)) {
        while (!fld_cmd_poll(0x13, &r))
            sys_frame_wait();
    }
}

int stalk_msg_close(void)
{
    StalkMsg msg;

    msg.speaker = -1;
    msg.text = 0;
    msg.title = 0;
    return fld_cmd_start(0x13, &msg) != 0;
}

void stalk_msg_wait(void)
{
    int r;

    while (!fld_cmd_poll(0x13, &r))
        sys_frame_wait();
}

/* end of a dialog: window and bust-ups out, field idle */
void st_ev_clear(void)
{
    if (stalk_msg_close())
        stalk_msg_wait();
    stalk_bu_wait();
    st_fld_wait_idle();
}

/* fixed dialogs 0x100..0x102; 0x101 asks a yes/no question (1 for the first answer) */
int st_ev_run(int ev)
{
    StalkList list;
    int res;
    unsigned short *items[2];

    switch (ev) {
    case 0x100:
        stalk_show(stext_get(6), 0);
        break;
    case 0x101:
        stalk_show(stext_get(3), 0);
        items[0] = stext_get(4);
        items[1] = stext_get(5);
        list.items = items;
        list.n = 2;
        if (fld_cmd_start(0x14, &list) == 0)
            return 0;
        while (!fld_cmd_poll(0x14, &res))
            sys_frame_wait();
        switch (res) {
        case 0:
            return 1;
        case 1:
            return 0;
        }
        break;
    case 0x102:
        stalk_show(stext_get(2), 0);
        break;
    }
    return 0;
}

/* splits text at the line feeds into at most 5 expanded lines */
void stext_split(unsigned short *text, unsigned short **lines, int *count)
{
    unsigned short line[64];
    unsigned short buf[64];
    int pos;
    int len;
    int n;

    *count = 0;
    pos = 0;
    if (wcslen(text) < 0x146) {
        n = 0;
        do {
            memset(line, 0, sizeof(line));
            len = wcschr_index(&text[pos], '\n');
            if (len <= 0)
                return;
            wcsncpy(line, &text[pos], len);
            stext_expand(buf, line, 0x40);
            wcscpy(*lines, buf);
            n++;
            pos += len + 1;
            *count = n;
            lines++;
        } while (n < 5);
    }
}

/* copies src to dst (n characters) and expands the tags; 0 when the result does not fit */
unsigned short *stext_expand(unsigned short *dst, unsigned short *src, int n)
{
    unsigned short player[32];
    unsigned short tag[32];
    unsigned short *end;

    memset(player, 0, sizeof(player));
    memset(tag, 0, sizeof(tag));
    dst[n - 1] = 0;
    end = dst + n - 1;
    wcscpy(dst, src);
    if (*end != 0)
        return 0;
    stext_cards(dst);
    if (*end != 0)
        return 0;
    stext_names(dst);
    if (*end != 0)
        return 0;
    stext_player(player);
    tag[0] = 0x300A;
    tag[1] = 'P';
    tag[2] = 'L';
    tag[3] = 'A';
    tag[4] = 'Y';
    tag[5] = 'E';
    tag[6] = 'R';
    tag[7] = 0x300B;
    tag[8] = 0;
    stext_replace(dst, tag, player);
    return *end != 0 ? 0 : dst;
}

/* bust-up of character id comes in at slot pos: alone, it takes the slot; with one already shown
   both move to the sides (id at the right unless pos is 50001); with two, it replaces the one at
   slot pos (or the other); 0 when it is already there */
int stalk_bu_add(StalkList *out, int id, int chara, int anim, int pos)
{
    StalkState *t = &stalk;
    int n = 0;
    int i;
    int k;
    StalkList list1;
    int r1;
    StalkList list2;
    int r2;

    for (i = 0; i < 2; i++) {
        if (t->chr[i] == id && t->bu[i].index == pos)
            return 0;
        if (t->chr[i] != 0)
            n++;
    }
    t->talk_pos = -1;
    switch (n) {
    case 0:
        t->chr[0] = id;
        t->bu[0].chara = chara;
        t->bu[0].index = pos;
        t->bu[0].anim = anim;
        out->items = &t->bu[0];
        out->n = 1;
        break;
    case 1:
        list1.n = 1;
        list1.items = &t->bu[0].index;
        if (fld_cmd_start(0x10, &list1) == 0)
            return 0;
        while (!fld_cmd_poll(0x10, &r1))
            sys_frame_wait();
        t->chr[1] = id;
        t->bu[1].chara = chara;
        t->bu[1].anim = anim;
        if (pos == 50001) {
            t->bu[0].index = 0;
            t->bu[1].index = 2;
        } else {
            t->bu[1].index = 0;
            t->bu[0].index = 2;
        }
        out->items = &t->bu[0];
        out->n = 2;
        break;
    case 2:
        k = t->bu[0].index != pos;
        list2.n = 1;
        list2.items = &t->bu[k].index;
        if (fld_cmd_start(0x10, &list2) == 0)
            return 0;
        while (!fld_cmd_poll(0x10, &r2))
            sys_frame_wait();
        t->chr[k] = id;
        t->bu[k].chara = chara;
        t->bu[k].index = pos;
        t->bu[k].anim = anim;
        out->items = &t->bu[k];
        out->n = 1;
        break;
    }
    return 1;
}

/* two bust-ups come in at the left and right slots: the ones already there are skipped */
int stalk_bu_add2(StalkList *out, int id1, int chara1, int anim1, int id2, int chara2, int anim2)
{
    StalkState *t = &stalk;
    int id[2];
    int anim[2];
    int pos[2];
    int add[2];
    int i;
    int j;
    int n;

    id[0] = id1;
    id[1] = id2;
    anim[0] = anim1;
    anim[1] = anim2;
    pos[0] = 0;
    pos[1] = 2;
    add[0] = 1;
    add[1] = 1;
    n = 2;
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            if (t->chr[j] == id[i] && t->bu[j].index == pos[i]) {
                add[i] = 0;
                n--;
            }
        }
    }
    switch (n) {
    case 0:
        return 0;
    case 1:
        for (i = 0; i < 2; i++) {
            if (add[i]) {
                stalk_bu_hide(id[i]);
                stalk_bu_enter(id[i], anim[i], pos[i]);
            }
        }
        return 1;
    case 2:
        stalk_bu_close_all();
        t->chr[0] = id[0];
        t->bu[0].anim = anim[0];
        t->bu[0].chara = chara1;
        t->bu[1].chara = chara2;
        t->bu[0].index = 0;
        t->chr[1] = id[1];
        t->bu[1].anim = anim[1];
        t->bu[1].index = 2;
        out->items = &t->bu[0];
        out->n = 2;
        return 1;
    }
}

/* bust-up of character id speaks: the slot of the one already shown, otherwise it comes in like
   stalk_bu_add (centre when alone) and becomes the speaker; 0 when already shown */
int stalk_bu_speak(StalkList *out, int id, int chara, int anim, int pos)
{
    StalkState *t = &stalk;
    int n = 0;
    int i;
    int p;
    int k;
    StalkList list1;
    int r1;
    StalkList list2;
    int r2;

    for (i = 0; i < 2; i++) {
        if (t->chr[i] == id) {
            t->talk_pos = t->bu[i].index;
            return 0;
        }
        if (t->chr[i] != 0)
            n++;
    }
    switch (n) {
    case 0:
        t->chr[0] = id;
        t->bu[0].chara = chara;
        t->bu[0].anim = anim;
        t->bu[0].index = 1;
        t->talk_pos = 1;
        out->items = &t->bu[0];
        out->n = 1;
        break;
    case 1:
        list1.n = 1;
        list1.items = &t->bu[0].index;
        if (fld_cmd_start(0x10, &list1) == 0)
            return 0;
        while (!fld_cmd_poll(0x10, &r1))
            sys_frame_wait();
        t->chr[1] = id;
        t->bu[1].chara = chara;
        t->bu[1].anim = anim;
        if (pos == 50001) {
            t->bu[0].index = 0;
            t->bu[1].index = 2;
            t->talk_pos = 2;
        } else {
            t->bu[1].index = 0;
            t->talk_pos = 0;
            t->bu[0].index = 2;
        }
        out->items = &t->bu[0];
        out->n = 2;
        break;
    case 2:
        p = pos == 50001 ? 2 : 0;
        k = t->bu[0].index != p;
        list2.n = 1;
        list2.items = &t->bu[k].index;
        if (fld_cmd_start(0x10, &list2) == 0)
            return 0;
        while (!fld_cmd_poll(0x10, &r2))
            sys_frame_wait();
        t->chr[k] = id;
        t->bu[k].chara = chara;
        t->bu[k].index = p;
        t->talk_pos = p;
        t->bu[k].anim = anim;
        out->items = &t->bu[k];
        out->n = 1;
        break;
    }
    return 1;
}

/* $Mnnnn -> name of card nnnn */
void stext_cards(unsigned short *text)
{
    unsigned short buf[256];
    unsigned short out[256];
    int len;
    int i;
    int id;
    int k;
    unsigned short c;

    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));
    wcscpy(buf, text);
    len = wcslen(buf);
    if (len > 0x100)
        return;
    for (i = 0;; i++) {
        if (buf[i] == 0) {
            wcscpy(text, buf);
            return;
        }
        if (buf[i] == '$' && buf[i + 1] == 'M') {
            memset(out, 0, sizeof(out));
            if (i > 0)
                wcsncpy(out, buf, i);
            id = 0;
            for (k = 5; k > 1; k--) {
                c = buf[i + k];
                switch (k) {
                case 5:
                    id += (unsigned short)(c - '0');
                    break;
                case 4:
                    id += (unsigned short)(c - '0') * 10;
                    break;
                case 3:
                    id += (unsigned short)(c - '0') * 100;
                    break;
                case 2:
                    id += (unsigned short)(c - '0') * 1000;
                    break;
                }
            }
            wcscat(out, card_get_name(id));
            if (buf[i + 6] != 0)
                wcsncat(out, &buf[i + 6], len - (i + 6));
            if (out[255] != 0)
                return;
            wcscpy(buf, out);
            len = wcslen(buf);
            if (len > 0x100)
                return;
            i += wcslen(card_get_name(id));
        }
    }
}

#ifdef NON_MATCHING /* mesma lógica; o original mantém o deslocamento de buf[i + 7] num registrador próprio e reestende as entradas das tabelas, e a alocação de registradores não bate */
/* 《Cnnn?v》 -> name of character nnn (variant v), 《Mnnn?v》 -> text of the b table, in colour */
void stext_names(unsigned short *text)
{
    unsigned short buf[256];
    unsigned short out[256];
    unsigned short code[4];
    int mode = 0;
    int len;
    int i;
    unsigned short *p;
    int id;
    int var;
    unsigned short *name;
    unsigned short *tab;
    unsigned short *e;
    int j;
    unsigned short c;

    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));
    memset(code, 0, sizeof(code));
    code[0] = '$';
    code[1] = 'C';
    code[2] = 0;
    code[3] = 0;
    wcscpy(buf, text);
    len = wcslen(buf);
    if (len > 0x100)
        return;
    for (i = 0, p = buf;; i++, p++) {
        if (*p == 0) {
            wcscpy(text, buf);
            return;
        }
        if (*p != 0x300A)
            continue;
        if (p[1] == 'C')
            mode = 1;
        else if (p[1] == 'M')
            mode = 2;
        else if (p[1] == 'P')
            continue;
        if (mode == 0)
            return;
        memset(out, 0, sizeof(out));
        if (i > 0)
            wcsncpy(out, buf, i);
        id = (unsigned short)(p[2] - '0') * 100 + (unsigned short)(p[3] - '0') * 10 + (unsigned short)(p[4] - '0');
        var = (unsigned short)(p[6] - '0');
        switch (mode) {
        case 1:
            if (var == 1) {
                name = chara_get_name(0, st_id_to_rec(id));
            } else {
                tab = stt_count_a();
                for (j = 0, e = tab;; j++, e += 3) {
                    if (e[0] == 0 && e[1] == 0 && e[2] == 0) {
                        name = chara_get_name(0, st_id_to_rec(id));
                        break;
                    }
                    if (id == e[0] && var == e[1]) {
                        name = stt_get_a(tab[j * 3 + 2]);
                        break;
                    }
                }
            }
            code[2] = '5';
            break;
        case 2:
            tab = stt_count_b();
            for (j = 0, e = tab;; j++, e += 3) {
                if (e[0] == 0 && e[1] == 0 && e[2] == 0) {
                    name = stt_get_b(tab[j * 3 - 1]);
                    break;
                }
                if (id == e[0] && (var == e[1] || e[0] != e[3])) {
                    name = stt_get_b(tab[j * 3 + 2]);
                    break;
                }
            }
            code[2] = '6';
            break;
        }
        wcscat(out, code);
        wcscat(out, name);
        code[2] = '0';
        wcscat(out, code);
        /* ponytail: the original never advances here and spins unless the tag closes at i + 7 */
        c = buf[i + 7];
        while (c != 0) {
            if (c == 0x300B) {
                wcsncat(out, &buf[i + 8], len - (i + 8));
                break;
            }
        }
        wcscpy(buf, out);
        len = wcslen(buf);
        if (len > 0x100)
            return;
    }
}
#endif

/* the player name (at most 32 characters) */
void stext_player(unsigned short *dst)
{
    unsigned short name[32];

    memset(name, 0, sizeof(name));
    wcscpy(name, sysstate_get_name());
    if (wcslen(name) < 0x21)
        wcscpy(dst, name);
}

/* every key in text -> val in colour 7 */
void stext_replace(unsigned short *text, unsigned short *key, unsigned short *val)
{
    unsigned short buf[256];
    unsigned short out[256];
    unsigned short code[4];
    int len;
    int i;
    int hit;
    unsigned short *k;
    unsigned short *p;
    int j;

    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));
    memset(code, 0, sizeof(code));
    wcscpy(buf, text);
    len = wcslen(buf);
    if (len > 0x100)
        return;
    for (i = 0;; i++) {
        hit = 0;
        p = &buf[i];
        if (*p == 0) {
            wcscpy(text, buf);
            return;
        }
        if (key[0] != *p)
            continue;
        p++;
        k = key + 1;
        for (;;) {
            if (*k == 0)
                break;
            if (*k == *p) {
                hit = 1;
                k++;
                p++;
            } else {
                hit = 0;
                break;
            }
        }
        if (hit) {
            memset(out, 0, sizeof(out));
            if (i > 0)
                wcsncpy(out, buf, i);
            if (val != 0 && val[0] != 0) {
                code[0] = '$';
                code[1] = 'C';
                code[2] = '7';
                code[3] = 0;
                wcscat(out, code);
                wcscat(out, val);
                code[2] = '0';
                wcscat(out, code);
            }
            j = i + wcslen(key);
            if (buf[j] != 0)
                wcsncat(out, &buf[j], len - j);
            wcscpy(buf, out);
            len = wcslen(buf);
            if (len > 0x100)
                return;
            i += wcslen(val);
        }
    }
}
