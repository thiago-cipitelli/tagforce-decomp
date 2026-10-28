// @range 0x088177C4-0x08817930
/* The bust-up portraits the story shows on the field: load requests (at most 9 at a time, field
   command 12) and the unload of all of them (field command 13). */
#include "libstory.h"

extern int st_bu_count;                /* 0x08ACBB1C portraits loaded */

void st_init_8177c4(void)
{
    memset(&st_bu_count, 0, sizeof(st_bu_count));
}

void st_term_8177d8(void)
{
    memset(&st_bu_count, 0, sizeof(st_bu_count));
}

/* asks the field to load the n portraits of `list`; returns how many were requested */
int st_costume_add(int *list, int n)
{
    int *count = &st_bu_count;
    struct {
        int *list;
        int n;
    } arg;

    if (n == 0)
        return 0;
    if (*count + n > 9)
        n = 9 - *count;
    arg.list = list;
    arg.n = n;
    if (fld_cmd_start(12, &arg) == 0)
        return 0;
    *count += n;
    return n;
}

/* waits for the pending loads, then unloads every portrait */
void st_fld_wait_idle(void)
{
    int *count = &st_bu_count;
    int r0;
    int arg;
    int r1;

    while (!fld_cmd_poll(12, &r0))
        sys_frame_wait();
    if (fld_cmd_start(13, &arg)) {
        *count = 0;
        while (!fld_cmd_poll(13, &r1))
            sys_frame_wait();
    }
}
