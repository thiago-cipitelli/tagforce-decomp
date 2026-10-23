// @range 0x08805948-0x08805A00
// @asm 0x08805948-0x08805A00
/* Analog stick scrolling of the card text: the stick's vertical deflection past the dead zone
   moves a clamped scroll position. */
#include "liblimitlist.h"

#ifdef NON_MATCHING /* o original deixa vazios os atrasos do `b` e do `beqz` e lê `max` antes de `pos`; ll_scroll_pad depende de ll_scroll_add ser static compilada antes (mantém o PadState em $v0 através da chamada) */
static void ll_scroll_add(LlScroll *s, int d)
{
    unsigned int max = s->max;
    unsigned int v;

    if (d >= 0 || s->pos >= (unsigned int)-d) {
        if (max == 0)
            v = 0;
        else {
            v = s->pos + d;
            if (v > max)
                v = max;
        }
    } else {
        v = 0;
    }
    s->pos = v;
}

void ll_scroll_pad(LlScroll *s)
{
    PadState *pad = pad_get();

    if (s) {
        if (pad->b.analog_y > 0x9E)
            ll_scroll_add(s, (pad->b.analog_y - 0x9E) * 4);
        if (pad->b.analog_y < 0x62)
            ll_scroll_add(s, (pad->b.analog_y - 0x62) * 4);
    }
}
#endif
