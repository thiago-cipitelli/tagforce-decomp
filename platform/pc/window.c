/* Window, GL context and keyboard (port step 5a, docs/port_survey.md).

   X11 + GLX with the system's 32-bit libX11/libGL: there is no 32-bit SDL on the build machine (lib32-sdl2 is not
   in the repositories; lib32-sdl3 is not installed). Only this file knows about X11; swap it for SDL when a 32-bit
   SDL is available (same five functions).
   Env: TAGFORCE_HEADLESS=1 keeps the window unmapped (GL still renders, frame dumps work); without a display there
   is no GL at all and the GE only follows the lists. TAGFORCE_SCALE (default 2) is the window size per PSP pixel.

   The GL context is current on one thread at a time; the game draws from more than one thread (one runs at a time,
   kernel.c), so a thread that blocks gives the context up (host_gfx_release, called by the scheduler) and the next
   GL user takes it (host_gfx_bind). */
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#include <GL/glx.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

int dprintf(int fd, const char *fmt, ...);

static Display *dpy;
static Window win;
static GLXContext ctx;
static Atom wm_delete;
static int state;                     /* 0 not tried, 1 GL ready, -1 no GL */
static int mapped, scale = 2, win_w, win_h;   /* win_*: the size the window manager gave us (a tiling WM ignores ours) */
static pthread_t owner;
static int owned;
static unsigned int buttons;

typedef GLXContext (*CreateCtx)(Display *, GLXFBConfig, GLXContext, Bool, const int *);

/* 1 when GL works; the context is then current on the calling thread */
int host_gfx_init(void)
{
    static const int fbattr[] = { GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
                                  GLX_DOUBLEBUFFER, True, GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, None };
    static const int ctxattr[] = { GLX_CONTEXT_MAJOR_VERSION_ARB, 3, GLX_CONTEXT_MINOR_VERSION_ARB, 3,
                                   GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB, None };
    GLXFBConfig *fc;
    XVisualInfo *vi;
    XSetWindowAttributes swa;
    CreateCtx create;
    const char *e;
    int n;

    if (state)
        return state > 0;
    state = -1;
    XInitThreads();
    if (!(dpy = XOpenDisplay(0))) {
        dprintf(2, "window: no display, GE without GL\n");
        return 0;
    }
    if ((e = getenv("TAGFORCE_SCALE")) && atoi(e) > 0)
        scale = atoi(e);
    fc = glXChooseFBConfig(dpy, DefaultScreen(dpy), fbattr, &n);
    create = (CreateCtx)glXGetProcAddressARB((const GLubyte *)"glXCreateContextAttribsARB");
    if (!fc || !n || !create) {
        dprintf(2, "window: no GLX config\n");
        return 0;
    }
    vi = glXGetVisualFromFBConfig(dpy, fc[0]);
    memset(&swa, 0, sizeof swa);
    swa.colormap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
    swa.event_mask = KeyPressMask | KeyReleaseMask | StructureNotifyMask | FocusChangeMask;
    win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0, 480 * scale, 272 * scale, 0, vi->depth, InputOutput,
                        vi->visual, CWColormap | CWEventMask, &swa);
    XStoreName(dpy, win, "Yu-Gi-Oh! GX Tag Force");
    wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, win, &wm_delete, 1);
    XkbSetDetectableAutoRepeat(dpy, True, 0);
    if (!(e = getenv("TAGFORCE_HEADLESS")) || strcmp(e, "1")) {
        XMapWindow(dpy, win);
        mapped = 1;
    }
    if (!(ctx = create(dpy, fc[0], 0, True, ctxattr))) {
        dprintf(2, "window: no GL 3.3 core context\n");
        return 0;
    }
    XFree(fc);
    XFree(vi);
    glXMakeCurrent(dpy, win, ctx);
    owner = pthread_self();
    owned = 1;
    {   /* the game paces itself on the vblank clock (kernel.c): no vsync wait in the swap */
        void (*swap_interval)(Display *, GLXDrawable, int) =
            (void (*)(Display *, GLXDrawable, int))glXGetProcAddressARB((const GLubyte *)"glXSwapIntervalEXT");
        if (swap_interval)
            swap_interval(dpy, win, 0);
    }
    dprintf(2, "window: %s, %s%s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER), mapped ? "" : " (headless)");
    state = 1;
    return 1;
}

void host_gfx_bind(void)
{
    if (state > 0 && !(owned && pthread_equal(owner, pthread_self()))) {
        glXMakeCurrent(dpy, win, ctx);
        owner = pthread_self();
        owned = 1;
    }
}

/* the scheduler calls this when a thread gives up the CPU */
void host_gfx_release(void)
{
    if (owned && pthread_equal(owner, pthread_self())) {
        glXMakeCurrent(dpy, None, 0);
        owned = 0;
    }
}

static unsigned int key_bit(KeySym k)
{
    switch (k) {
    case XK_BackSpace: return 0x0001;   /* select */
    case XK_Return: return 0x0008;      /* start */
    case XK_Up: return 0x0010;
    case XK_Right: return 0x0020;
    case XK_Down: return 0x0040;
    case XK_Left: return 0x0080;
    case XK_q: return 0x0100;           /* L */
    case XK_w: return 0x0200;           /* R */
    case XK_s: return 0x1000;           /* triangle */
    case XK_x: return 0x2000;           /* circle */
    case XK_z: return 0x4000;           /* cross */
    case XK_a: return 0x8000;           /* square */
    }
    return 0;
}

/* window events: keys to PSP buttons, close = exit */
/* text entry for the on-screen keyboard (utility.c): while it is on, the keyboard types into text[] instead of
   pressing buttons, and the window title shows what was typed (the PSP keyboard itself is not drawn) */
static char text[64];
static int text_on, text_max, text_done;   /* text_done: 1 Enter, -1 Esc */

static void text_title(void)
{
    char t[160];

    snprintf(t, sizeof t, "Type the name: %s_   (Enter = OK, Esc = cancel)", text);
    XStoreName(dpy, win, text_on ? t : "Yu-Gi-Oh! GX Tag Force");
    XFlush(dpy);
}

/* starts typing over `init`, at most max characters; 0 when there is no window to type into */
int host_text_start(const char *init, int max)
{
    if (state <= 0 || !mapped)
        return 0;
    snprintf(text, sizeof text, "%s", init);
    text_max = max < (int)sizeof text - 1 ? max : (int)sizeof text - 1;
    text[text_max] = 0;
    text_on = 1;
    text_done = buttons = 0;
    text_title();
    return 1;
}

/* 0 still typing; 1 confirmed (out gets the text); -1 cancelled */
int host_text_poll(char *out, int n)
{
    if (text_on)
        return 0;
    snprintf(out, n, "%s", text);
    return text_done;
}

static void text_key(XKeyEvent *e)
{
    char c[8];
    KeySym k;
    int n = XLookupString(e, c, sizeof c, &k, 0), len = strlen(text);

    if (k == XK_Return || k == XK_KP_Enter || k == XK_Escape) {
        text_done = k == XK_Escape ? -1 : 1;
        text_on = 0;
    } else if (k == XK_BackSpace) {
        if (len)
            text[len - 1] = 0;
    } else if (n == 1 && c[0] >= 0x20 && c[0] < 0x7F && len < text_max) {   /* the TTF renders ASCII */
        text[len] = c[0];
        text[len + 1] = 0;
    }
    text_title();
}

void host_pump(void)
{
    XEvent ev;

    if (state <= 0)
        return;
    while (XPending(dpy)) {
        XNextEvent(dpy, &ev);
        if (ev.type == KeyPress && text_on)
            text_key(&ev.xkey);
        else if (ev.type == KeyPress)
            buttons |= key_bit(XLookupKeysym(&ev.xkey, 0));
        else if (ev.type == KeyRelease)
            buttons &= ~key_bit(XLookupKeysym(&ev.xkey, 0));
        else if (ev.type == FocusOut)
            buttons = 0;
        else if (ev.type == ConfigureNotify)
            win_w = ev.xconfigure.width, win_h = ev.xconfigure.height;
        else if (ev.type == ClientMessage && (Atom)ev.xclient.data.l[0] == wm_delete)
            exit(0);
    }
}

/* TAGFORCE_PAD="poll:mask[:polls],...": hold the PSP buttons mask (hex) from that pad poll on, for polls
   (default 1); for runs without a keyboard (headless, tests). Step 6: the unit is the game's pad reads
   (sceCtrlPeek/ReadBufferPositive, host_pad_polls in kernel.c), not presented frames: pad_update runs once per main
   loop pass, so a one-poll Down moves exactly once (with frames a 6-frame press moved twice or not at all) */
static unsigned int scripted(void)
{
    extern unsigned int host_pad_polls;   /* kernel.c */
    static const char *pad;
    const char *p;
    unsigned int b = 0;

    if (!pad && !(pad = getenv("TAGFORCE_PAD")))
        pad = "";
    {   /* TAGFORCE_PADFILE=path: the mask (hex) is read from that file on every poll, to drive a headless run live */
        static const char *pf;
        FILE *f;

        if (!pf && !(pf = getenv("TAGFORCE_PADFILE")))
            pf = "";
        if (*pf && (f = fopen(pf, "r"))) {
            if (fscanf(f, "%x", &b) != 1)
                b = 0;
            fclose(f);
        }
    }
    for (p = pad; *p;) {
        char *e;
        unsigned int f = strtoul(p, &e, 0), m = 0, n = 1;
        if (*e == ':')
            m = strtoul(e + 1, &e, 16);
        if (*e == ':')
            n = strtoul(e + 1, &e, 0);
        if (host_pad_polls >= f && host_pad_polls < f + n)
            b |= m;
        p = *e == ',' ? e + 1 : e + strlen(e);
    }
    return b;
}

unsigned int host_buttons(void)
{
    host_pump();
    return buttons | scripted();
}

/* shows the 480x272 top-left of the bound read framebuffer (GL rows = PSP rows), fitted to the window's real size
   with the 30:17 aspect kept and black bars around */
void host_present(void)
{
    if (state <= 0 || !mapped)
        return;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    {
        int ww = win_w > 0 ? win_w : 480 * scale, wh = win_h > 0 ? win_h : 272 * scale;
        int w = ww * 272 <= wh * 480 ? ww : wh * 480 / 272, h = w * 272 / 480;
        int x = (ww - w) / 2, y = (wh - h) / 2;
        GLboolean sc = glIsEnabled(GL_SCISSOR_TEST);

        glDisable(GL_SCISSOR_TEST);   /* the clear and the blit both obey the game's scissor */
        glViewport(0, 0, ww, wh);
        glColorMask(1, 1, 1, 1);      /* ge.c sets the mask again on every draw */
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glBlitFramebuffer(0, 0, 480, 272, x, y + h, x + w, y, GL_COLOR_BUFFER_BIT,
                          w % 480 || h % 272 ? GL_LINEAR : GL_NEAREST);
        if (sc)
            glEnable(GL_SCISSOR_TEST);
    }
    glXSwapBuffers(dpy, win);
    host_pump();
}
