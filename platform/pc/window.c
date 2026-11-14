/* Window, GL context and keyboard (port step 5a, docs/port_survey.md).

   X11 + GLX with the system's 32-bit libX11/libGL: there is no 32-bit SDL on the build machine (lib32-sdl2 is not
   in the repositories; lib32-sdl3 is not installed). Only this file knows about X11; swap it for SDL when a 32-bit
   SDL is available (same five functions).
   Env: TAGFORCE_HEADLESS=1 keeps the window unmapped (GL still renders, frame dumps work); without a display there
   is no GL at all and the GE only follows the lists. TAGFORCE_SCALE (default 2) is the window size per PSP pixel.

   The GL context is current on one thread at a time; the game draws from more than one thread (one runs at a time,
   kernel.c), so a thread that blocks gives the context up (host_gfx_release, called by the scheduler) and the next
   GL user takes it (host_gfx_bind). */
#include <X11/Xlib.h>
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
static int mapped, scale = 2;
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
void host_pump(void)
{
    XEvent ev;

    if (state <= 0)
        return;
    while (XPending(dpy)) {
        XNextEvent(dpy, &ev);
        if (ev.type == KeyPress)
            buttons |= key_bit(XLookupKeysym(&ev.xkey, 0));
        else if (ev.type == KeyRelease)
            buttons &= ~key_bit(XLookupKeysym(&ev.xkey, 0));
        else if (ev.type == FocusOut)
            buttons = 0;
        else if (ev.type == ClientMessage && (Atom)ev.xclient.data.l[0] == wm_delete)
            exit(0);
    }
}

/* TAGFORCE_PAD="frame:mask[:frames],...": hold the PSP buttons mask (hex) from that presented frame on, for
   frames (default 6); for runs without a keyboard (headless, tests) */
static unsigned int scripted(void)
{
    extern unsigned int frame_no;   /* ge.c */
    static const char *pad;
    const char *p;
    unsigned int b = 0;

    if (!pad && !(pad = getenv("TAGFORCE_PAD")))
        pad = "";
    for (p = pad; *p;) {
        char *e;
        unsigned int f = strtoul(p, &e, 0), m = 0, n = 6;
        if (*e == ':')
            m = strtoul(e + 1, &e, 16);
        if (*e == ':')
            n = strtoul(e + 1, &e, 0);
        if (frame_no >= f && frame_no < f + n)
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

/* shows the 480x272 top-left of the bound read framebuffer (GL rows = PSP rows), integer scaled */
void host_present(void)
{
    if (state <= 0 || !mapped)
        return;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, 480 * scale, 272 * scale);
    glBlitFramebuffer(0, 0, 480, 272, 0, 272 * scale, 480 * scale, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glXSwapBuffers(dpy, win);
    host_pump();
}
