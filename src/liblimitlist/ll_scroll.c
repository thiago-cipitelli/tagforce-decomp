// @range 0x08805948-0x08805A00
/* Analog stick scrolling of the card text: the stick's vertical deflection past the dead zone
   moves a clamped scroll position. */
#include "liblimitlist.h"

static void ll_scroll_add(LlScroll *s, int d)
{
    unsigned int pos = s->pos;
    unsigned int max = s->max;

    if ((d < 0 && pos < (unsigned int)-d) || max == 0)
        pos = 0;
    else {
        pos += d;
        if (pos > max)
            pos = max;
    }
    s->pos = pos;
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
