// @range 0x08805034-0x088056E0
// @data 0x0880F954-0x0880FA1C
/* Resources of a lesson: tutorial/tutorial_<l>.ehp read synchronously into the duel heap and
   mounted; per-lesson files by name (%02d_message_res.bin, %02d_duel_command.bin, field setups),
   bounded string copies and the $-variables of the message text ($0 player name, $1..$4 ATK). */
#include "libtutorial.h"

extern char tut_strbuf[0x100];   /* 0x08810110 */
extern int tut_load_done;        /* 0x08810210 */
extern void *tut_msgs;           /* 0x08810218 message table of the lesson */
extern void *tut_heap;           /* 0x0881021C the duel's heap (libduel_draw) */

void *dd_heap_get(void);
unsigned short tut_watch_get(void);
unsigned short *sysstate_get_name(void);
int card_get_atk_q(int id);
int wsprintf(unsigned short *out, const unsigned short *fmt, ...);

int str_copy(char *d, int n, const char *s);
int str_append(char *d, int n, const char *s);
int wstr_copy(unsigned short *d, int n, const unsigned short *s);
int wstr_append(unsigned short *d, int n, const unsigned short *s);
void *tut_alloc(int n);
void tut_free_to(void *p);
void tut_heap_init(void);
void tut_heap_term(void);
int tut_load_cb(void *arg, int n);

static const char *tut_lang[6] = { "j", "e", "g", "f", "i", "s" };

/* tutorial/<name>, read in one go (waits frame by frame) and mounted */
void *tut_pack_load(const char *name)
{
    char path[0x100];
    int size;
    void *buf;

    str_copy(path, 0x100, "tutorial/");
    str_append(path, 0x100, name);
    size = ehfile_get_size(3, path);
    buf = tut_alloc(size);
    tut_load_done = 0;
    ehfile_read_async_alias(3, path, buf, size, tut_load_cb, 0);
    while (!tut_load_done)
        sys_frame_wait();
    ehp_mount(0, buf);
    return buf;
}

char *tut_sprintf(const char *fmt, int arg)
{
    sprintf(tut_strbuf, fmt, arg);
    return tut_strbuf;
}

void tut_res_load(int lesson)
{
    char path[0x100];
    int lang = cfg_get_language();

    str_copy(path, 0x100, "tutorial_");
    str_append(path, 0x100, tut_lang[lang]);
    str_append(path, 0x100, ".ehp");
    tut_pack = tut_pack_load(path);
    tut_msgs = tut_find(tut_sprintf("%02d_message_res.bin", lesson), 0);
}

void tut_res_free(void)
{
    tut_free_to(tut_pack);
    tut_pack = 0;
}

void *tut_find(const char *name, int *size)
{
    return ehp_find(tut_pack, name, size);
}

void tut_release(void *p)
{
}

unsigned short *tut_msg_get(int id)
{
    return table_get_entry(tut_msgs, id);
}

void *tut_duel_commands(int lesson)
{
    return tut_find(tut_sprintf("%02d_duel_command.bin", lesson), 0);
}

char *tut_field_set(int lesson)
{
    return tut_find(tut_sprintf("%02d_field_set.txt", lesson), 0);
}

char *tut_field_reset_file(int n)
{
    return tut_find(tut_sprintf("field_reset_%02d.txt", n), 0);
}

int tut_load_cb(void *arg, int n)
{
    tut_load_done = 1;
    return n > 0;
}

void tut_heap_init_thunk(void)
{
    tut_heap_init();
}

void tut_heap_term_thunk(void)
{
    tut_heap_term();
}

/* copy at most n - 1 characters; returns the length written */
int str_copy(char *d, int n, const char *s)
{
    int i = 0;

    while (*s != 0) {
        *d = *s;
        i++;
        d++;
        s++;
        if (i >= n) {
            d[-1] = 0;
            return i - 1;
        }
    }
    *d = 0;
    return i;
}

/* append to the string in d (buffer of n); -1 if d is already full */
int str_append(char *d, int n, const char *s)
{
    int i = 0;

    while (*d != 0) {
        i++;
        d++;
        if (i >= n)
            return -1;
    }
    while (*s != 0) {
        *d = *s;
        i++;
        d++;
        s++;
        if (i >= n) {
            d[-1] = 0;
            return i - 1;
        }
    }
    *d = 0;
    return i;
}

int wstr_copy(unsigned short *d, int n, const unsigned short *s)
{
    int i = 0;

    while (*s != 0) {
        *d = *s;
        i++;
        d++;
        s++;
        if (i >= n) {
            d[-1] = 0;
            return i - 1;
        }
    }
    *d = 0;
    return i;
}

int wstr_append(unsigned short *d, int n, const unsigned short *s)
{
    int i = 0;

    while (*d != 0) {
        i++;
        d++;
        if (i >= n)
            return -1;
    }
    while (*s != 0) {
        *d = *s;
        i++;
        d++;
        s++;
        if (i >= n) {
            d[-1] = 0;
            return i - 1;
        }
    }
    *d = 0;
    return i;
}

/* $0 = player name (in color 7); $1..$4 = ATK of the watched card, halved from $2, +200 from $3,
   +500 more for $4 */
void tut_text_var(int kind, int unused, unsigned short *out)
{
    unsigned short card = tut_watch_get();

    switch (kind) {
    case 0: {
        unsigned short fmt[9] = { '$', 'C', '7', '%', 's', '$', 'C', '0', 0 };

        wsprintf(out, fmt, sysstate_get_name());
        break;
    }
    default:
        if (card != 0) {
            unsigned short fmt[3] = { '%', 'd', 0 };
            int atk = card_get_atk_q(card);

            if (kind >= 2)
                atk /= 2;
            if (kind >= 3)
                atk += 200;
            if (kind == 4)
                atk += 500;
            wsprintf(out, fmt, atk);
        }
    }
}

/* copy src to dst (n characters) replacing $0..$9 by tut_text_var */
void tut_text_expand(unsigned short *src, unsigned short *dst, int n)
{
    unsigned short buf[0x100];
    unsigned short *s = src;
    unsigned short *d = dst;
    unsigned short *end = dst + n;
    unsigned short c;

    while (*s != 0) {
        c = *s;
        if (c == '$') {
            s++;
            c = *s;
            if (c == '$') {
                *d = c;
                d++;
                s += 2;
            } else if (c >= '0' && c <= '9') {
                tut_text_var(c - '0', 0, buf);
                *d = 0;
                d = dst + wstr_append(dst, n, buf);
                s++;
            } else {
                *d = '$';
                d++;
            }
        } else {
            *d = c;
            d++;
            s++;
        }
        if (d >= end) {
            d--;
            break;
        }
    }
    *d = 0;
}

void tut_heap_init(void)
{
    tut_heap = dd_heap_get();
}

void tut_heap_term(void)
{
    tut_heap = 0;
}

void *tut_alloc(int n)
{
    return heap_alloc(tut_heap, n);
}

void tut_free_to(void *p)
{
    heap_set_level(tut_heap, p);
}

void *tut_heap_get(void)
{
    return tut_heap;
}
