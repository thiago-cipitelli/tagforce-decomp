/* GE display-list interpreter on OpenGL 3.3 (port step 5a, docs/port_survey.md).

   sceGeListEnQueue(Head) queues a list; the queue runs at once, in the caller's thread, up to the stall address of
   the head list (UpdateStallAddr runs it further) or its END, then the next list. Callbacks (FINISH, SIGNAL) run
   after the interpreter returns, as the interrupt would. Every command lands in reg[op]; matrices, CLUT, vertex and
   index addresses, transfers and draws are acted on.
   Draws: vertices are decoded and transformed on the CPU (morph, skinning, world/view/projection, lighting,
   texgen, viewport) into GL clip space and drawn as triangle lists (strips/fans/sprites expanded; flat shading
   copies the provoking color); one shader does the texture function, color doubling, specular, fog, alpha test.
   Draw targets: one FBO (RGBA8 + depth/stencil) per (EDRAM offset, width); a texture whose address falls inside
   one samples it. GL rows = PSP rows (row 0 on top of the PSP screen is GL row 0); host_present flips.
   Fragment alpha is never written outside clear mode: on the PSP the alpha channel of the frame buffer is the
   stencil value.
   Addresses: list words keep 24 bits plus 4 (BASE) or 8 (TBW, CBW...) high bits; host pointers are 32-bit, so a
   28-bit address takes the top nibble of the current list pointer when bits 24-27 agree (stack lists), else 0.
   Not done (logged once when used): curves (BEZIER/SPLINE), color test, logic op, depth-range clamp, dithering,
   16-bit frame buffers as textures/transfers (an FBO is RGBA8 whatever the PSP format), mipmaps (level 0 only).
   Debugging: TAGFORCE_GE_TRACE=<frame> logs every draw of that frame, TAGFORCE_GE_SKIP=<n> leaves draw n of it
   out, TAGFORCE_GE_SKIP_VT=<hex> every draw with that vertex type. */
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

int dprintf(int fd, const char *fmt, ...);
int host_gfx_init(void);
void host_gfx_bind(void);
void host_present(void);

/* --- EDRAM: 2 MB, aligned so that (pointer & 0x1FFFFF) is the offset, as with the PSP's 0x04000000; the other
   2 MB are slack for texture reads past the end --- */

static u8 edram[0x400000] __attribute__((aligned(0x400000)));

void *sceGeEdramGetAddr(void) { return edram; }
unsigned int sceGeEdramGetSize(void) { return 0x200000; }
int sceGeEdramSetAddrTranslation(int width) { return 0; }

static int in_edram(u32 a) { return (a & ~0x3FFFFFu) == (u32)edram; }

/* --- state --- */

static u32 reg[256];
static float bone[96], world[12], view[12], proj[16], tgen[12], morphw[8];
static int bonei, worldi, viewi, proji, tgeni;
static u8 *vaddr, *iaddr;
static u32 offset_addr, cur_pc;
static u8 clut[1024];
static int gl_on;                       /* 1: draw with GL; 0: only follow the lists */
unsigned int frame_no;                  /* presented frames (window.c: scripted pad) */
static unsigned int trace_frame = ~0u;  /* TAGFORCE_GE_TRACE */
static int draw_no, skip_draw = -1;
static u32 skip_vt;     /* TAGFORCE_GE_SKIP: draw of the traced frame left out */

static float f24(u32 d)
{
    union { u32 i; float f; } u;

    u.i = d << 8;
    return u.f;
}

static void log_once(int key, const char *what)
{
    static u8 seen[512];

    if (!seen[key & 511]) {
        seen[key & 511] = 1;
        dprintf(2, "ge: %s\n", what);
    }
}

/* host pointer of a GE address (24 + 4 or 8 bits) */
static void *host_addr(u32 a)
{
    if ((a & 0xFF000000) == 0x04000000 || (a & 0xFF000000) == 0x44000000)   /* PSP VRAM */
        return edram + (a & 0x1FFFFF);
    if (!(a & 0xF0000000) && (cur_pc & 0xF0000000) && !((cur_pc ^ a) & 0x0F000000))
        a |= cur_pc & 0xF0000000;
    return (void *)a;
}

static u32 rel(u32 d)
{
    return (((reg[0x10] & 0x0F0000) << 8 | d) + offset_addr) & 0x0FFFFFFF;
}

/* --- GL objects --- */

typedef struct {
    u32 fbp, fbw;
    int w, h, maxy;
    GLuint fbo, tex, rb;
} Fbo;

#define NFBO 32
static Fbo fbos[NFBO];
static int nfbo;
static GLuint prog, vao, vbo;
static GLint u_tex_on, u_func, u_tcc, u_dbl, u_env, u_fog_on, u_fogc, u_atest, u_aref, u_amask, u_uvxf;

static const char *vs_src =
    "#version 330 core\n"
    "layout(location=0) in vec4 a_pos; layout(location=1) in vec4 a_c0; layout(location=2) in vec3 a_c1;\n"
    "layout(location=3) in vec3 a_uv; layout(location=4) in float a_fog;\n"
    "out vec4 v_c0; out vec3 v_c1; out vec3 v_uv; out float v_fog;\n"
    "void main() { gl_Position = a_pos; v_c0 = a_c0; v_c1 = a_c1; v_uv = a_uv; v_fog = a_fog; }\n";

static const char *fs_src =
    "#version 330 core\n"
    "in vec4 v_c0; in vec3 v_c1; in vec3 v_uv; in float v_fog;\n"
    "uniform sampler2D u_tex; uniform int u_tex_on, u_func, u_tcc, u_dbl, u_fog_on, u_atest, u_aref, u_amask;\n"
    "uniform vec3 u_env, u_fogc; uniform vec4 u_uvxf;\n"
    "out vec4 o;\n"
    "bool cmp(int f, int a, int b) {\n"
    "  if (f == 0) return false; if (f == 1) return true; if (f == 2) return a == b; if (f == 3) return a != b;\n"
    "  if (f == 4) return a < b; if (f == 5) return a <= b; if (f == 6) return a > b; return a >= b; }\n"
    "void main() {\n"
    "  vec4 c = v_c0;\n"
    "  if (u_tex_on == 1) {\n"
    "    vec4 t = texture(u_tex, v_uv.xy / v_uv.z * u_uvxf.xy + u_uvxf.zw);\n"
    "    float ta = u_tcc == 1 ? t.a : 1.0;\n"
    "    if (u_func == 0) c = vec4(c.rgb * t.rgb, c.a * ta);\n"
    "    else if (u_func == 1) c.rgb = u_tcc == 1 ? mix(c.rgb, t.rgb, t.a) : t.rgb;\n"
    "    else if (u_func == 2) c = vec4(mix(c.rgb, u_env, t.rgb), c.a * ta);\n"
    "    else if (u_func == 3) c = vec4(t.rgb, u_tcc == 1 ? t.a : c.a);\n"
    "    else c = vec4(c.rgb + t.rgb, c.a * ta);\n"
    "    if (u_dbl == 1) c.rgb *= 2.0;\n"
    "  }\n"
    "  c.rgb += v_c1;\n"
    "  if (u_fog_on == 1) c.rgb = mix(u_fogc, c.rgb, clamp(v_fog, 0.0, 1.0));\n"
    "  c = clamp(c, 0.0, 1.0);\n"
    "  if (u_atest >= 0 && !cmp(u_atest, int(c.a * 255.0 + 0.5) & u_amask, u_aref & u_amask)) discard;\n"
    "  o = c;\n"
    "}\n";

static GLuint shader(GLenum type, const char *src)
{
    GLuint s = glCreateShader(type);
    GLint ok;
    char log[1024];

    glShaderSource(s, 1, &src, 0);
    glCompileShader(s);
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        glGetShaderInfoLog(s, sizeof log, 0, log);
        dprintf(2, "ge: shader: %s\n", log);
        abort();
    }
    return s;
}

static void gl_setup(void)
{
    static int tried;

    if (tried)
        return;
    tried = 1;
    if (getenv("TAGFORCE_GE_TRACE"))
        trace_frame = atoi(getenv("TAGFORCE_GE_TRACE"));
    if (getenv("TAGFORCE_GE_SKIP_VT"))
        skip_vt = strtoul(getenv("TAGFORCE_GE_SKIP_VT"), 0, 16);
    if (getenv("TAGFORCE_GE_SKIP"))
        skip_draw = atoi(getenv("TAGFORCE_GE_SKIP"));
    if (!host_gfx_init())
        return;
    prog = glCreateProgram();
    glAttachShader(prog, shader(GL_VERTEX_SHADER, vs_src));
    glAttachShader(prog, shader(GL_FRAGMENT_SHADER, fs_src));
    glLinkProgram(prog);
    glUseProgram(prog);
#define U(n) n = glGetUniformLocation(prog, #n)
    U(u_tex_on); U(u_func); U(u_tcc); U(u_dbl); U(u_env); U(u_fog_on); U(u_fogc); U(u_atest); U(u_aref);
    U(u_amask); U(u_uvxf);
#undef U
    glUniform1i(glGetUniformLocation(prog, "u_tex"), 0);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 60, (void *)0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 60, (void *)16);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 60, (void *)32);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 60, (void *)44);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, 60, (void *)56);
    glEnable(GL_DEPTH_CLAMP);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    gl_on = 1;
}

static Fbo *fbo_get(u32 fbp, u32 fbw)
{
    Fbo *f;
    int i;

    for (i = 0; i < nfbo; i++)
        if (fbos[i].fbp == fbp && fbos[i].fbw == fbw)
            return &fbos[i];
    if (nfbo == NFBO) {
        log_once(1, "out of frame buffers");
        return &fbos[0];
    }
    f = &fbos[nfbo++];
    f->fbp = fbp;
    f->fbw = fbw;
    f->w = fbw;
    f->h = 512;
    glGenTextures(1, &f->tex);
    glBindTexture(GL_TEXTURE_2D, f->tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, f->w, f->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glGenRenderbuffers(1, &f->rb);
    glBindRenderbuffer(GL_RENDERBUFFER, f->rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, f->w, f->h);
    glGenFramebuffers(1, &f->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, f->fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, f->tex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, f->rb);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(1, 1, 1, 1);
    glDepthMask(1);
    glStencilMask(0xFF);
    glClearColor(0, 0, 0, 0);
    glClearDepth(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    return f;
}

/* the frame buffer whose memory holds EDRAM offset off (32-bit pixels); 0 if none */
static Fbo *fbo_at(u32 off, int *x, int *y)
{
    Fbo *b = 0;
    int i;

    for (i = 0; i < nfbo; i++) {
        Fbo *f = &fbos[i];
        if (off >= f->fbp && off < f->fbp + f->fbw * 4 * (f->maxy + 1) && (!b || f->fbp > b->fbp))
            b = f;
    }
    if (b) {
        u32 d = off - b->fbp;
        *y = d / (b->fbw * 4);
        *x = d % (b->fbw * 4) / 4;
    }
    return b;
}

/* --- textures --- */

typedef struct {
    u32 addr, fmt, tw, th, bufw, swz, clutkey, hash;
    unsigned int checked, used;
    GLuint id;
} Tex;

#define NTEX 1024
static Tex texs[NTEX];
static u32 *texbuf;
static u8 *swzbuf;

static u32 hash(const u8 *p, int n)
{
    const u32 *w = (const u32 *)p;
    u32 h = 2166136261u;
    int i;

    for (i = 0; i < n / 4; i++)
        h = (h ^ w[i]) * 16777619u;
    return h;
}

static u32 c16(u32 c, int fmt)
{
    u32 r, g, b, a;

    switch (fmt) {
    case 0:   /* 5650 */
        r = c & 31; g = c >> 5 & 63; b = c >> 11 & 31;
        return (r << 3 | r >> 2) | (g << 2 | g >> 4) << 8 | (b << 3 | b >> 2) << 16 | 0xFF000000u;
    case 1:   /* 5551 */
        r = c & 31; g = c >> 5 & 31; b = c >> 10 & 31; a = c >> 15;
        return (r << 3 | r >> 2) | (g << 3 | g >> 2) << 8 | (b << 3 | b >> 2) << 16 | (a ? 0xFF000000u : 0);
    default:  /* 4444 */
        r = c & 15; g = c >> 4 & 15; b = c >> 8 & 15; a = c >> 12;
        return r * 17 | g * 17 << 8 | b * 17 << 16 | a * 17 << 24;
    }
}

static u32 clut_color(u32 i)
{
    u32 m = reg[0xC5];

    i = ((i >> (m >> 2 & 31)) & (m >> 8 & 0xFF)) | (m >> 16 & 31) << 4;
    if ((m & 3) == 3)
        return ((u32 *)clut)[i & 255];
    return c16(((u16 *)clut)[i & 511], m & 3);
}

static const int fmt_bits[] = { 16, 16, 16, 32, 4, 8, 16, 32 };

/* PSP DXT blocks (after PPSSPP's TextureDecoder): the color part is 4 row bytes of 2-bit indices then two 5650
   colors; DXT3 adds 4 rows of 4-bit alpha after it, DXT5 48 bits of 3-bit alpha indices (low 32 first) then the
   two alpha values. DXT3/5 always use the 4-color mode. */
static void dxt_decode(const u8 *src, u32 fmt, u32 tw, u32 th, u32 rowbytes)
{
    u32 bx, by, x, y, bs = fmt == 8 ? 8 : 16;

    for (by = 0; by < th / 4 + (th < 4); by++)
        for (bx = 0; bx < tw / 4 + (tw < 4); bx++) {
            const u8 *b = src + by * rowbytes + bx * bs;
            u32 c1 = b[4] | b[5] << 8, c2 = b[6] | b[7] << 8, col[4], i;
            u32 ca = c16(c1, 0), cb = c16(c2, 0);
            /* DXT colors are RGB565 with red in the HIGH bits (PPSSPP's DecodeColors, tools/gim.py c565), the
               opposite of the GE's own 5650: every DXT texture came out with red and blue swapped (blue card backs) */
            ca = (ca & 0xFF00FF00u) | (ca >> 16 & 0xFF) | (ca & 0xFF) << 16;
            cb = (cb & 0xFF00FF00u) | (cb >> 16 & 0xFF) | (cb & 0xFF) << 16;
            unsigned long long ad = 0;
            u32 al[8];
            col[0] = ca;
            col[1] = cb;
            if (c1 > c2 || fmt != 8) {
                col[2] = col[3] = 0xFF000000u;
                for (i = 0; i < 24; i += 8) {
                    u32 p = ca >> i & 0xFF, q = cb >> i & 0xFF;
                    col[2] |= (2 * p + q) / 3 << i;
                    col[3] |= (p + 2 * q) / 3 << i;
                }
            } else {
                col[2] = 0xFF000000u;
                for (i = 0; i < 24; i += 8)
                    col[2] |= ((ca >> i & 0xFF) + (cb >> i & 0xFF)) / 2 << i;
                col[3] = 0;
            }
            if (fmt == 10) {
                u32 a1 = b[14], a2 = b[15];
                ad = (unsigned long long)(b[12] | b[13] << 8) << 32 | (u32)(b[8] | b[9] << 8 | b[10] << 16 | b[11] << 24);
                al[0] = a1;
                al[1] = a2;
                for (i = 2; i < 8; i++)
                    al[i] = a1 > a2 ? ((8 - i) * a1 + (i - 1) * a2) / 7
                          : i < 6 ? ((6 - i) * a1 + (i - 1) * a2) / 5 : i == 6 ? 0 : 255;
            }
            for (y = 0; y < 4 && by * 4 + y < th; y++)
                for (x = 0; x < 4 && bx * 4 + x < tw; x++) {
                    u32 c = col[b[y] >> x * 2 & 3];
                    if (fmt == 9)
                        c = (c & 0xFFFFFF) | ((b[8 + y * 2] | b[9 + y * 2] << 8) >> x * 4 & 15) * 17 << 24;
                    else if (fmt == 10)
                        c = (c & 0xFFFFFF) | al[ad >> (y * 4 + x) * 3 & 7] << 24;
                    texbuf[(by * 4 + y) * tw + bx * 4 + x] = c;
                }
        }
}

/* GL texture for the current TEXADDR0..; *xf: scale and offset of the normalized coordinates */
static GLuint tex_get(float *xf)
{
    u32 a = (reg[0xA0] & 0xFFFFF0) | (reg[0xA8] & 0xFF0000) << 8;
    u32 bufw = reg[0xA8] & 0xFFFF, fmt = reg[0xC3] & 15, swz = reg[0xC2] & 1;
    u32 tw = 1 << (reg[0xB8] & 15), th = 1 << (reg[0xB8] >> 8 & 15);
    u32 clutkey = 0, h, rowbytes, bytes;
    const u8 *src;
    Tex *t = 0, *lru = &texs[0];
    u32 x, y;
    int i;

    src = host_addr(a);
    xf[0] = xf[1] = 1;
    xf[2] = xf[3] = 0;
    if (in_edram((u32)src)) {
        int fx, fy;
        Fbo *f = fbo_at((u32)src & 0x1FFFFF, &fx, &fy);
        if (f) {
            xf[0] = (float)tw / f->w;
            xf[1] = (float)th / f->h;
            xf[2] = (float)fx / f->w;
            xf[3] = (float)fy / f->h;
            if (fmt != 3)
                log_once(2, "frame buffer sampled with a non-8888 format");
            return f->tex;
        }
    }
    if (fmt > 10) {
        log_once(3, "texture format > 10");
        return 0;
    }
    if (bufw < 1)
        bufw = tw;
    if (fmt >= 8)   /* DXT: 4x4 blocks of 8 (DXT1) or 16 bytes, bufw / 4 blocks per row */
        rowbytes = bufw / 4 * (fmt == 8 ? 8 : 16), bytes = rowbytes * ((th + 3) / 4);
    else
        rowbytes = bufw * fmt_bits[fmt] / 8, bytes = rowbytes * th;
    if (fmt >= 4)
        clutkey = hash(clut, fmt == 4 ? 64 : 1024) ^ reg[0xC5] * 31;
    for (i = 0; i < NTEX; i++) {
        Tex *e = &texs[i];
        if (e->id && e->addr == (u32)src && e->fmt == fmt && e->tw == tw && e->th == th && e->bufw == bufw &&
            e->swz == swz && e->clutkey == clutkey) {
            t = e;
            break;
        }
        if (e->used < lru->used)
            lru = e;
    }
    if (t && t->checked == frame_no) {
        t->used = frame_no;
        return t->id;
    }
    h = hash(src, bytes);
    if (t && t->hash == h) {
        t->checked = t->used = frame_no;
        return t->id;
    }
    if (!t) {
        t = lru;
        if (!t->id)
            glGenTextures(1, &t->id);
        t->addr = (u32)src;
        t->fmt = fmt;
        t->tw = tw;
        t->th = th;
        t->bufw = bufw;
        t->swz = swz;
        t->clutkey = clutkey;
    }
    t->hash = h;
    t->checked = t->used = frame_no;
    if (!texbuf) {
        texbuf = malloc(1024 * 1024 * 4);
        swzbuf = malloc(1024 * 1024 * 4);
    }
    if (swz) {   /* 16-byte x 8-row blocks, row by row */
        const u8 *s = src;
        u32 bx, by, l, bw = rowbytes / 16;
        for (by = 0; by < th / 8 + (th < 8); by++)
            for (bx = 0; bx < bw; bx++)
                for (l = 0; l < 8; l++, s += 16)
                    memcpy(swzbuf + (by * 8 + l) * rowbytes + bx * 16, s, 16);
        src = swzbuf;
    }
    if (fmt >= 8) {
        dxt_decode(src, fmt, tw, th, rowbytes);
        goto upload;
    }
    for (y = 0; y < th; y++) {
        u32 *d = texbuf + y * tw;
        const u8 *r = src + y * rowbytes;
        for (x = 0; x < tw; x++) {
            switch (fmt) {
            case 0: case 1: case 2: d[x] = c16(((u16 *)r)[x], fmt); break;
            case 3: d[x] = ((u32 *)r)[x]; break;
            case 4: d[x] = clut_color(r[x >> 1] >> (x & 1) * 4 & 15); break;
            case 5: d[x] = clut_color(r[x]); break;
            case 6: d[x] = clut_color(((u16 *)r)[x]); break;
            case 7: d[x] = clut_color(((u32 *)r)[x]); break;
            }
        }
    }
upload:
    glBindTexture(GL_TEXTURE_2D, t->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, texbuf);
    return t->id;
}

/* --- vertices --- */

typedef struct { float pos[4], c0[4], c1[3], uv[3], fog; } GV;   /* what the shader gets (60 bytes) */
typedef struct { float w[8], uv[2], col[4], nrm[3], pos[3]; } Vtx;

typedef struct { int wt, nw, tt, ct, nt, pt, ow, ot, oc, on, op, size; } Fmt;

static Fmt vfmt(u32 vt)
{
    static const int sz[4] = { 0, 1, 2, 4 };
    Fmt f;
    int off = 0, al = 1;

#define FIELD(o, s, n) do { if (s) { off = (off + (s) - 1) & ~((s) - 1); o = off; off += (s) * (n); if ((s) > al) al = (s); } } while (0)
    f.wt = vt >> 9 & 3;
    f.nw = (vt >> 14 & 7) + 1;
    f.tt = vt & 3;
    f.ct = vt >> 2 & 7;
    f.nt = vt >> 5 & 3;
    f.pt = vt >> 7 & 3;
    f.ow = f.ot = f.oc = f.on = f.op = 0;
    FIELD(f.ow, sz[f.wt], f.nw);
    FIELD(f.ot, sz[f.tt], 2);
    FIELD(f.oc, f.ct == 7 ? 4 : f.ct >= 4 ? 2 : 0, 1);
    FIELD(f.on, sz[f.nt], 3);
    FIELD(f.op, sz[f.pt], 3);
#undef FIELD
    f.size = (off + al - 1) & ~(al - 1);
    return f;
}

static float comp(const u8 *p, int type, int i, float s8, float s16)
{
    switch (type) {
    case 1: return ((signed char *)p)[i] * s8;
    case 2: return ((short *)p)[i] * s16;
    default: return ((float *)p)[i];
    }
}

static void decode(const u8 *p, const Fmt *f, int through, Vtx *v)
{
    int i;

    for (i = 0; i < 8; i++)
        v->w[i] = 0;
    if (f->wt)
        for (i = 0; i < f->nw; i++)
            v->w[i] = f->wt == 1 ? p[f->ow + i] / 128.0f : f->wt == 2 ? ((u16 *)(p + f->ow))[i] / 32768.0f
                                                                      : ((float *)(p + f->ow))[i];
    v->uv[0] = v->uv[1] = 0;
    if (f->tt)
        for (i = 0; i < 2; i++)
            v->uv[i] = f->tt == 1 ? p[f->ot + i] * (through ? 1 : 1 / 128.0f)
                     : f->tt == 2 ? ((u16 *)(p + f->ot))[i] * (through ? 1 : 1 / 32768.0f)
                                  : ((float *)(p + f->ot))[i];
    if (f->ct >= 4) {
        u32 c = f->ct == 7 ? *(u32 *)(p + f->oc) : c16(*(u16 *)(p + f->oc), f->ct - 4);
        for (i = 0; i < 4; i++)
            v->col[i] = (c >> i * 8 & 0xFF) / 255.0f;
    }
    v->nrm[0] = v->nrm[1] = v->nrm[2] = 0;
    if (f->nt)
        for (i = 0; i < 3; i++)
            v->nrm[i] = comp(p + f->on, f->nt, i, 1 / 128.0f, 1 / 32768.0f);
    v->pos[0] = v->pos[1] = v->pos[2] = 0;
    if (through && f->pt == 1) {
        v->pos[0] = ((signed char *)(p + f->op))[0];
        v->pos[1] = ((signed char *)(p + f->op))[1];
        v->pos[2] = p[f->op + 2];
    } else if (through && f->pt == 2) {
        v->pos[0] = ((short *)(p + f->op))[0];
        v->pos[1] = ((short *)(p + f->op))[1];
        v->pos[2] = ((u16 *)(p + f->op))[2];
    } else if (f->pt) {
        for (i = 0; i < 3; i++)
            v->pos[i] = comp(p + f->op, f->pt, i, 1 / 128.0f, 1 / 32768.0f);
    }
}

/* vertex i with morphing */
static void fetch(const u8 *p, const Fmt *f, int nmorph, int through, Vtx *v)
{
    Vtx m;
    int k, i;

    if (nmorph == 1) {
        decode(p, f, through, v);
        return;
    }
    memset(v, 0, sizeof *v);
    for (k = 0; k < nmorph; k++) {
        float w = morphw[k];
        decode(p + k * f->size, f, through, &m);
        for (i = 0; i < 8; i++) v->w[i] += m.w[i] * w;
        for (i = 0; i < 2; i++) v->uv[i] += m.uv[i] * w;
        for (i = 0; i < 4; i++) v->col[i] += m.col[i] * w;
        for (i = 0; i < 3; i++) v->nrm[i] += m.nrm[i] * w, v->pos[i] += m.pos[i] * w;
    }
}

static void mul43(const float *m, const float *in, float w, float *out)
{
    int j;

    for (j = 0; j < 3; j++)
        out[j] = in[0] * m[j] + in[1] * m[3 + j] + in[2] * m[6 + j] + w * m[9 + j];
}

static void rgb(u32 c, float *o)
{
    o[0] = (c & 0xFF) / 255.0f;
    o[1] = (c >> 8 & 0xFF) / 255.0f;
    o[2] = (c >> 16 & 0xFF) / 255.0f;
}

static float dot3(const float *a, const float *b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }

static void norm3(float *v)
{
    float l = sqrtf(dot3(v, v));

    if (l > 0)
        v[0] /= l, v[1] /= l, v[2] /= l;
}

/* PSP lighting in world space (after PPSSPP's software transform) */
static void light(const float *wp, const float *wn, const float *vcol, int has_col, float *c0, float *c1)
{
    u32 mu = reg[0x53];
    float amb[4], dif[3], spe[3], gamb[3], emi[3], n[3];
    float spec_pow = f24(reg[0x5B]);
    int l, i;

    if ((mu & 1) && has_col)
        memcpy(amb, vcol, 16);
    else
        rgb(reg[0x55], amb), amb[3] = (reg[0x58] & 0xFF) / 255.0f;
    if ((mu & 2) && has_col) memcpy(dif, vcol, 12); else rgb(reg[0x56], dif);
    if ((mu & 4) && has_col) memcpy(spe, vcol, 12); else rgb(reg[0x57], spe);
    rgb(reg[0x5C], gamb);
    rgb(reg[0x54], emi);
    for (i = 0; i < 3; i++) {
        c0[i] = emi[i] + gamb[i] * amb[i];
        c1[i] = 0;
        n[i] = (reg[0x51] & 1) ? -wn[i] : wn[i];
    }
    c0[3] = (reg[0x5D] & 0xFF) / 255.0f * amb[3];
    norm3(n);
    for (l = 0; l < 4; l++) {
        u32 lt = reg[0x5F + l];
        int type = lt >> 8 & 3, kind = lt & 3;
        float lp[3], to[3], d = 1, scale = 1, la[3], ld[3], ls[3], dt;
        if (!(reg[0x18 + l] & 1))
            continue;
        for (i = 0; i < 3; i++)
            lp[i] = f24(reg[0x63 + l * 3 + i]);
        if (type == 0) {
            memcpy(to, lp, 12);
            norm3(to);
        } else {
            for (i = 0; i < 3; i++)
                to[i] = lp[i] - wp[i];
            d = sqrtf(dot3(to, to));
            norm3(to);
            scale = f24(reg[0x7B + l * 3]) + f24(reg[0x7C + l * 3]) * d + f24(reg[0x7D + l * 3]) * d * d;
            scale = scale > 0 ? 1 / scale : 1;
            if (scale > 1)
                scale = 1;
            if (type == 2) {
                float dir[3], a;
                for (i = 0; i < 3; i++)
                    dir[i] = f24(reg[0x6F + l * 3 + i]);
                norm3(dir);
                a = -dot3(dir, to);
                scale = a >= f24(reg[0x8B + l]) ? scale * powf(a, f24(reg[0x87 + l])) : 0;
            }
        }
        rgb(reg[0x8F + l * 3], la);
        rgb(reg[0x90 + l * 3], ld);
        rgb(reg[0x91 + l * 3], ls);
        dt = dot3(to, n);
        if (dt < 0)
            dt = 0;
        if (kind == 2)
            dt = powf(dt, spec_pow);
        for (i = 0; i < 3; i++)
            c0[i] += (la[i] * amb[i] + ld[i] * dif[i] * dt) * scale;
        if (kind == 1) {
            float hv[3] = { to[0], to[1], to[2] + 1 };
            norm3(hv);
            dt = dot3(hv, n);
            if (dt > 0) {
                dt = powf(dt, spec_pow);
                for (i = 0; i < 3; i++)
                    c1[i] += ls[i] * spe[i] * dt * scale;
            }
        }
    }
    if (!(reg[0x5E] & 1))
        for (i = 0; i < 3; i++)
            c0[i] += c1[i], c1[i] = 0;
}

/* environment-map coordinate from light l: (N.L + 1) / 2, L toward the light (from the vertex unless directional),
   the half vector with the view direction when the light does specular */
static float shade_coord(int l, const float *wp, const float *n)
{
    float L[3];
    int i;

    for (i = 0; i < 3; i++)
        L[i] = f24(reg[0x63 + l * 3 + i]) - ((reg[0x5F + l] >> 8 & 3) ? wp[i] : 0);
    norm3(L);
    if ((reg[0x5F + l] & 3) == 1) {
        float v[3] = { view[2], view[5], view[8] };
        norm3(v);
        for (i = 0; i < 3; i++)
            L[i] += v[i];
        norm3(L);
    }
    return (dot3(L, n) + 1) / 2;
}

static GV *gv;
static int gv_cap;

/* PRIM: decode, transform, expand to GL triangles/lines/points */
static void prim(u32 d)
{
    static const GLenum mode[] = { GL_POINTS, GL_LINES, GL_LINES, GL_TRIANGLES, GL_TRIANGLES, GL_TRIANGLES,
                                   GL_TRIANGLES, GL_TRIANGLES };
    u32 vt = reg[0x12];
    int count = d & 0xFFFF, type = d >> 16 & 7, through = vt >> 23 & 1, it = vt >> 11 & 3;
    int nmorph = (vt >> 18 & 7) + 1, i, n = 0, nv;
    Fmt f = vfmt(vt);
    GV *in;
    float fw, fh, xs, ys, zs, xc, yc, zc, ox, oy, tw, th;
    Fbo *fb;
    u32 fbp, fbw;
    int clear = reg[0xD3] & 1, tex_on, lit;

    if (!count)
        return;
    if (!gl_on)
        goto advance;
    if (trace_frame == frame_no) {   /* TAGFORCE_GE_TRACE=<frame>: one line per draw of that frame */
        dprintf(2, "draw %d: prim %d n %d vt %06X fb %06X/%d tex %d fmt %d %08X tmap %X light %d blend %d:%03X "
                "z %d:%d func %03X clear %X cull %d:%d\n", draw_no, type, count, vt, reg[0x9C], reg[0x9D] & 0x7FC,
                reg[0x1E] & 1, reg[0xC3], (reg[0xA0] & 0xFFFFF0) | (reg[0xA8] & 0xFF0000) << 8, reg[0xC0],
                reg[0x17] & 1, reg[0x21] & 1, reg[0xDF], reg[0x23] & 1, reg[0xDE], reg[0xC9], reg[0xD3],
                reg[0x1D] & 1, reg[0x9B]);
    }
    if (trace_frame == frame_no && draw_no++ == skip_draw)
        goto advance;
    if (skip_vt && vt == skip_vt)   /* TAGFORCE_GE_SKIP_VT: leave out every draw with this vertex type */
        goto advance;
    fbp = ((reg[0x9C] & 0xFFFFF0) | (reg[0x9D] & 0xFF0000) << 8) & 0x1FFFF0;
    fbw = reg[0x9D] & 0x7FC;
    if (!fbw)
        goto advance;
    fb = fbo_get(fbp, fbw);
    fw = fb->w;
    fh = fb->h;
    xs = f24(reg[0x42]); ys = f24(reg[0x43]); zs = f24(reg[0x44]);
    xc = f24(reg[0x45]); yc = f24(reg[0x46]); zc = f24(reg[0x47]);
    ox = (reg[0x4C] & 0xFFFF) / 16.0f;
    oy = (reg[0x4D] & 0xFFFF) / 16.0f;
    tw = 1 << (reg[0xB8] & 15);
    th = 1 << (reg[0xB8] >> 8 & 15);
    tex_on = (reg[0x1E] & 1) && !clear;
    lit = (reg[0x17] & 1) && !through;
    if (gv_cap < count * 3 + 6) {
        gv_cap = count * 3 + 6;
        gv = realloc(gv, gv_cap * 2 * sizeof *gv);
    }
    in = gv + gv_cap;   /* second half: one GV per input vertex */
    for (i = 0; i < count; i++) {
        int idx = it == 0 ? i : it == 1 ? iaddr[i] : it == 2 ? ((u16 *)iaddr)[i] : ((u32 *)iaddr)[i];
        Vtx v;
        GV *o = &in[i];
        int k;

        if (f.ct < 4) {   /* no vertex color: material ambient */
            rgb(reg[0x55], v.col);
            v.col[3] = (reg[0x58] & 0xFF) / 255.0f;
        }
        fetch(vaddr + idx * f.size * nmorph, &f, nmorph, through, &v);
        o->c1[0] = o->c1[1] = o->c1[2] = 0;
        o->fog = 1;
        if (through) {
            o->pos[0] = v.pos[0] * 2 / fw - 1;
            o->pos[1] = v.pos[1] * 2 / fh - 1;
            o->pos[2] = v.pos[2] * 2 / 65535 - 1;
            o->pos[3] = 1;
            o->uv[0] = v.uv[0] / tw;
            o->uv[1] = v.uv[1] / th;
            o->uv[2] = 1;
            memcpy(o->c0, v.col, 16);
        } else {
            float p[3], nn[3], wp[3], wn[3], vp[3], c[4];
            if (f.wt) {   /* skinning */
                float t[3];
                p[0] = p[1] = p[2] = nn[0] = nn[1] = nn[2] = 0;
                for (k = 0; k < f.nw; k++) {
                    mul43(bone + k * 12, v.pos, 1, t);
                    p[0] += t[0] * v.w[k]; p[1] += t[1] * v.w[k]; p[2] += t[2] * v.w[k];
                    mul43(bone + k * 12, v.nrm, 0, t);
                    nn[0] += t[0] * v.w[k]; nn[1] += t[1] * v.w[k]; nn[2] += t[2] * v.w[k];
                }
            } else {
                memcpy(p, v.pos, 12);
                memcpy(nn, v.nrm, 12);
            }
            mul43(world, p, 1, wp);
            mul43(world, nn, 0, wn);
            mul43(view, wp, 1, vp);
            for (k = 0; k < 4; k++)
                c[k] = vp[0] * proj[k] + vp[1] * proj[4 + k] + vp[2] * proj[8 + k] + proj[12 + k];
            o->pos[0] = (c[0] * xs + c[3] * (xc - ox)) * 2 / fw - c[3];
            o->pos[1] = (c[1] * ys + c[3] * (yc - oy)) * 2 / fh - c[3];
            o->pos[2] = (c[2] * zs + c[3] * zc) * 2 / 65535 - c[3];
            o->pos[3] = c[3];
            if (lit)
                light(wp, wn, v.col, f.ct >= 4, o->c0, o->c1);
            else
                memcpy(o->c0, v.col, 16);
            o->fog = (vp[2] + f24(reg[0xCD])) * f24(reg[0xCE]);
            switch (reg[0xC0] & 3) {
            case 0:
                o->uv[0] = v.uv[0] * f24(reg[0x48]) + f24(reg[0x4A]);
                o->uv[1] = v.uv[1] * f24(reg[0x49]) + f24(reg[0x4B]);
                o->uv[2] = 1;
                break;
            case 1: {
                float s[3], t[3];
                switch (reg[0xC0] >> 8 & 3) {
                case 0: memcpy(s, v.pos, 12); break;
                case 1: s[0] = v.uv[0]; s[1] = v.uv[1]; s[2] = 0; break;
                case 2: memcpy(s, v.nrm, 12); norm3(s); break;
                default: memcpy(s, v.nrm, 12); break;
                }
                mul43(tgen, s, 1, t);
                memcpy(o->uv, t, 12);
                break;
            }
            default: {   /* shade mapping (PPSSPP's PSPShadeMapCoord) */
                float n2[3];
                memcpy(n2, wn, 12);
                norm3(n2);
                o->uv[0] = shade_coord(reg[0xC1] & 3, wp, n2);
                o->uv[1] = shade_coord(reg[0xC1] >> 8 & 3, wp, n2);
                o->uv[2] = 1;
                break;
            }
            }
        }
        if (trace_frame == frame_no && (i == 0 || i == count - 1 || count <= 8))   /* vertices: pixels and texels */
            dprintf(2, "  v%d: xy %.1f %.1f uv %.1f %.1f (tex %dx%d bufw %d)\n", i,
                    (o->pos[0] / o->pos[3] + 1) * fw / 2, (o->pos[1] / o->pos[3] + 1) * fh / 2,
                    o->uv[0] * tw, o->uv[1] * th, (int)tw, (int)th, (int)(reg[0xA8] & 0xFFFF));
    }
    /* expand */
    switch (type) {
    case 0: case 1:
        memcpy(gv, in, count * sizeof *gv);
        n = count;
        break;
    case 2:
        for (i = 1; i < count; i++)
            gv[n++] = in[i - 1], gv[n++] = in[i];
        break;
    case 3: case 4: case 5:
        nv = type == 3 ? count / 3 : count - 2;
        for (i = 0; i < nv; i++) {
            GV *a = type == 3 ? &in[i * 3] : type == 4 ? &in[i] : &in[0];
            GV *b = type == 3 ? &in[i * 3 + 1] : &in[i + 1], *c = type == 3 ? &in[i * 3 + 2] : &in[i + 2];
            if (type == 4 && (i & 1)) {   /* odd strip triangles: same winding as the even ones */
                GV *t = a;
                a = b;
                b = t;
            }
            gv[n++] = *a;
            gv[n++] = *b;
            gv[n++] = *c;
            if (!(reg[0x50] & 1)) {   /* flat: the last vertex's colors */
                memcpy(gv[n - 3].c0, c->c0, 28);
                memcpy(gv[n - 2].c0, c->c0, 28);
            }
        }
        break;
    case 6:
        for (i = 0; i + 1 < count; i += 2) {
            GV a = in[i], b = in[i + 1], q[4];
            float ax = a.pos[0] / a.pos[3], ay = a.pos[1] / a.pos[3];
            float bx = b.pos[0] / b.pos[3], by = b.pos[1] / b.pos[3], bz = b.pos[2] / b.pos[3];
            float au = a.uv[0] / a.uv[2], av = a.uv[1] / a.uv[2], bu = b.uv[0] / b.uv[2], bv = b.uv[1] / b.uv[2];
            int k;
            for (k = 0; k < 4; k++) {
                q[k] = b;
                q[k].pos[0] = k & 1 ? bx : ax;
                q[k].pos[1] = k & 2 ? by : ay;
                q[k].pos[2] = bz;
                q[k].pos[3] = 1;
                q[k].uv[0] = k & 1 ? bu : au;
                q[k].uv[1] = k & 2 ? bv : av;
                q[k].uv[2] = 1;
            }
            /* a sprite flipped in exactly one axis is drawn rotated: the two off-diagonal corners trade texture
               coordinates (PPSSPP's RotateUV). The card frame keeps its side borders lying down in the texture
               and draws them this way; without it they came out as stretched blocks */
            if ((ax < bx) != (ay < by) && ax != bx && ay != by) {
                float t0 = q[1].uv[0], t1 = q[1].uv[1];
                q[1].uv[0] = q[2].uv[0]; q[1].uv[1] = q[2].uv[1];
                q[2].uv[0] = t0; q[2].uv[1] = t1;
            }
            gv[n++] = q[0]; gv[n++] = q[1]; gv[n++] = q[2];
            gv[n++] = q[1]; gv[n++] = q[3]; gv[n++] = q[2];
        }
        break;
    default:
        log_once(4, "primitive type 7");
        break;
    }
    if (!n)
        goto advance;
    {   /* GL state */
        static const GLenum func[] = { GL_NEVER, GL_ALWAYS, GL_EQUAL, GL_NOTEQUAL, GL_LESS, GL_LEQUAL, GL_GREATER,
                                       GL_GEQUAL };
        static const GLenum sop[] = { GL_KEEP, GL_ZERO, GL_REPLACE, GL_INVERT, GL_INCR, GL_DECR, GL_KEEP, GL_KEEP };
        u32 s1 = reg[0xD4], s2 = reg[0xD5];
        int sx1 = s1 & 0x3FF, sy1 = s1 >> 10 & 0x3FF, sx2 = s2 & 0x3FF, sy2 = s2 >> 10 & 0x3FF;

        glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);
        glViewport(0, 0, fb->w, fb->h);
        glEnable(GL_SCISSOR_TEST);
        if (sx2 < sx1 || sy2 < sy1)
            goto advance;
        glScissor(sx1, sy1, sx2 - sx1 + 1, sy2 - sy1 + 1);
        if (sy2 > fb->maxy)
            fb->maxy = sy2 < fb->h ? sy2 : fb->h - 1;
        if (clear) {
            int m = reg[0xD3] >> 8;
            glDisable(GL_BLEND);
            glColorMask(m & 1, m & 1, m & 1, (m & 2) != 0);
            if (m & 4) {
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_ALWAYS);
                glDepthMask(1);
            } else {
                glDisable(GL_DEPTH_TEST);
            }
            if (m & 2) {
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(GL_ALWAYS, (int)(gv[n - 1].c0[3] * 255 + 0.5f), 0xFF);
                glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
                glStencilMask(0xFF);
            } else {
                glDisable(GL_STENCIL_TEST);
            }
            glUniform1i(u_atest, -1);
            glUniform1i(u_fog_on, 0);
        } else {
            u32 mask = reg[0xE8];
            if (reg[0x21] & 1) {
                static const GLenum sf[] = { GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA,
                                             GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
                                             GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA,
                                             GL_ONE_MINUS_DST_ALPHA, GL_CONSTANT_COLOR };
                static const GLenum df[] = { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA,
                                             GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA,
                                             GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA,
                                             GL_ONE_MINUS_DST_ALPHA, GL_CONSTANT_COLOR };
                static const GLenum eq[] = { GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT, GL_MIN, GL_MAX,
                                             GL_FUNC_ADD, GL_FUNC_ADD, GL_FUNC_ADD };
                u32 b = reg[0xDF], a = b & 15, c = b >> 4 & 15;
                GLenum ga = sf[a > 10 ? 10 : a], gb = df[c > 10 ? 10 : c];
                float fc[3];
                if (a == 10 && (reg[0xE0] & 0xFFFFFF) == 0xFFFFFF) ga = GL_ONE;
                if (a == 10 && !(reg[0xE0] & 0xFFFFFF)) ga = GL_ZERO;
                if (c == 10 && (reg[0xE1] & 0xFFFFFF) == 0xFFFFFF) gb = GL_ONE;
                if (c == 10 && !(reg[0xE1] & 0xFFFFFF)) gb = GL_ZERO;
                rgb(ga == GL_CONSTANT_COLOR ? reg[0xE0] : reg[0xE1], fc);
                if (ga == GL_CONSTANT_COLOR && gb == GL_CONSTANT_COLOR && (reg[0xE0] ^ reg[0xE1]) & 0xFFFFFF)
                    log_once(5, "blend with two different fixed colors");
                glBlendColor(fc[0], fc[1], fc[2], 1);
                glEnable(GL_BLEND);
                glBlendFunc(ga, gb);
                glBlendEquation(eq[b >> 8 & 7]);
            } else {
                glDisable(GL_BLEND);
            }
            glColorMask((mask & 0xFF) != 0xFF, (mask & 0xFF00) != 0xFF00, (mask & 0xFF0000) != 0xFF0000, 0);
            if (reg[0x23] & 1) {
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(func[reg[0xDE] & 7]);
                glDepthMask(!(reg[0xE7] & 1));
            } else {
                glDisable(GL_DEPTH_TEST);
            }
            if (reg[0x24] & 1) {
                u32 st = reg[0xDC], so = reg[0xDD];
                glEnable(GL_STENCIL_TEST);
                glStencilFunc(func[st & 7], st >> 8 & 0xFF, st >> 16 & 0xFF);
                glStencilOp(sop[so & 7], sop[so >> 8 & 7], sop[so >> 16 & 7]);
                glStencilMask(0xFF);
            } else {
                glDisable(GL_STENCIL_TEST);
            }
            if (reg[0x22] & 1) {
                glUniform1i(u_atest, reg[0xDB] & 7);
                glUniform1i(u_aref, reg[0xDB] >> 8 & 0xFF);
                glUniform1i(u_amask, reg[0xDB] >> 16 & 0xFF);
            } else {
                glUniform1i(u_atest, -1);
            }
            glUniform1i(u_fog_on, (reg[0x1F] & 1) && !through);
            if (reg[0x1F] & 1) {
                float fc[3];
                rgb(reg[0xCF], fc);
                glUniform3fv(u_fogc, 1, fc);
            }
            if (reg[0x27] & 1)
                log_once(6, "color test (ignored)");
            if (reg[0x28] & 1)
                log_once(7, "logic op (ignored)");
        }
        glUniform1i(u_tex_on, 0);
        if (tex_on) {
            float xf[4];
            GLuint t = tex_get(xf);
            if (t) {
                float env[3];
                int lin = reg[0xC6] & 1, wrap_u = reg[0xC7] & 1, wrap_v = reg[0xC7] >> 8 & 1;
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, t);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, lin ? GL_LINEAR : GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (reg[0xC6] >> 8 & 1) ? GL_LINEAR : GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_u ? GL_CLAMP_TO_EDGE : GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_v ? GL_CLAMP_TO_EDGE : GL_REPEAT);
                glUniform1i(u_tex_on, 1);
                glUniform1i(u_func, reg[0xC9] & 7);
                glUniform1i(u_tcc, reg[0xC9] >> 8 & 1);
                glUniform1i(u_dbl, reg[0xC9] >> 16 & 1);
                rgb(reg[0xCA], env);
                glUniform3fv(u_env, 1, env);
                glUniform4fv(u_uvxf, 1, xf);
            }
        }
        if ((reg[0x1D] & 1) && !clear && type >= 3 && type <= 5) {
            glEnable(GL_CULL_FACE);
            /* with GL row 0 = PSP row 0 (the image is upside down for GL): CULL 0 -> GL_FRONT. Checked on
               the character models: the other way round the eyes and the mouth come out black. */
            glCullFace(reg[0x9B] & 1 ? GL_BACK : GL_FRONT);
        } else {
            glDisable(GL_CULL_FACE);
        }
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, n * sizeof *gv, gv, GL_STREAM_DRAW);
        glDrawArrays(mode[type], 0, n);
    }
advance:
    if (it)
        iaddr += count << (it - 1);
    else
        vaddr += count * f.size * nmorph;
}

/* --- block transfer (sceGuCopyImage) --- */

static void transfer(u32 d)
{
    u8 *src = host_addr((reg[0xB2] & 0xFFFFF0) | (reg[0xB3] & 0xFF0000) << 8);
    u8 *dst = host_addr((reg[0xB4] & 0xFFFFF0) | (reg[0xB5] & 0xFF0000) << 8);
    int sw = reg[0xB3] & 0x7FF, dw = reg[0xB5] & 0x7FF, bpp = d & 1 ? 4 : 2;
    int sx = reg[0xEB] & 0x3FF, sy = reg[0xEB] >> 10 & 0x3FF, dx = reg[0xEC] & 0x3FF, dy = reg[0xEC] >> 10 & 0x3FF;
    int w = (reg[0xEE] & 0x3FF) + 1, h = (reg[0xEE] >> 10 & 0x3FF) + 1, y, fx, fy;
    Fbo *f;

    if (gl_on && in_edram((u32)src) && (f = fbo_at(((u32)src + (sy * sw + sx) * bpp) & 0x1FFFFF, &fx, &fy))) {
        if (bpp != 4) {
            log_once(8, "16-bit transfer from a frame buffer");
        } else {   /* frame buffer memory <- FBO, whole rows */
            glBindFramebuffer(GL_FRAMEBUFFER, f->fbo);
            glReadPixels(0, fy, f->w, h, GL_RGBA, GL_UNSIGNED_BYTE, edram + f->fbp + fy * f->fbw * 4);
        }
    }
    for (y = 0; y < h; y++)
        memmove(dst + ((dy + y) * dw + dx) * bpp, src + ((sy + y) * sw + sx) * bpp, w * bpp);
    if (gl_on && in_edram((u32)dst) && (f = fbo_at(((u32)dst + (dy * dw + dx) * bpp) & 0x1FFFFF, &fx, &fy))) {
        if (bpp != 4) {
            log_once(9, "16-bit transfer to a frame buffer");
        } else {   /* FBO <- frame buffer memory, whole rows */
            glBindTexture(GL_TEXTURE_2D, f->tex);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, f->fbw);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, fy, f->w, h, GL_RGBA, GL_UNSIGNED_BYTE,
                            edram + f->fbp + fy * f->fbw * 4);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        }
    }
}

/* --- lists --- */

typedef struct { void (*signal)(int, void *); void *signal_arg; void (*finish)(int, void *); void *finish_arg; } GeCb;
typedef struct {
    int used, cbid, sp;
    u32 *pc, *stall;
    u32 *stack[32];
    u32 stack_off[32];
} GeList;

static GeCb cbs[16];
static GeList lists[16];
static int queue[16], nq;
static struct { void (*fn)(int, void *); int id; void *arg; } pend[64];
static int npend;

static void callback(int cbid, int finish, int id)
{
    GeCb *c;

    if (cbid < 0 || cbid >= 16 || npend == 64)
        return;
    c = &cbs[cbid];
    if (finish ? !c->finish : !c->signal)
        return;
    pend[npend].fn = finish ? c->finish : c->signal;
    pend[npend].arg = finish ? c->finish_arg : c->signal_arg;
    pend[npend].id = id;
    npend++;
}

static void jump(GeList *l, u32 a, int call)
{
    if (call) {
        if (l->sp == 32) {
            log_once(10, "call stack overflow");
            return;
        }
        l->stack_off[l->sp] = offset_addr;
        l->stack[l->sp++] = l->pc;
    }
    l->pc = host_addr(a);
}

static void ret(GeList *l)
{
    if (!l->sp) {
        log_once(11, "RET with an empty stack");
        return;
    }
    l->sp--;
    l->pc = l->stack[l->sp];
    offset_addr = l->stack_off[l->sp];
}

static const char known[] =   /* opcodes kept in reg[] or acted on */
    "\x00\x01\x02\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0e\x0f\x10\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d"
    "\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x28\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x36\x37\x38\x3a\x3b\x3c"
    "\x3d\x3e\x3f\x40\x41\x42\x43\x44\x45\x46\x47\x48\x49\x4a\x4b\x4c\x4d\x50\x51\x53\x54\x55\x56\x57\x58\x5b\x5c"
    "\x5d\x5e\x5f\x60\x61\x62\x63\x64\x65\x66\x67\x68\x69\x6a\x6b\x6c\x6d\x6e\x6f\x70\x71\x72\x73\x74\x75\x76\x77"
    "\x78\x79\x7a\x7b\x7c\x7d\x7e\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92"
    "\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad"
    "\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca"
    "\xcb\xcc\xcd\xce\xcf\xd0\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6"
    "\xe7\xe8\xe9\xea\xeb\xec\xee\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xff";

/* runs l until its stall address (0) or its end (1) */
static int exec(GeList *l)
{
    u32 prev = 0;

    for (;;) {
        u32 w, op, d;

        if (l->stall && l->pc == l->stall)
            return 0;
        cur_pc = (u32)l->pc;
        w = *l->pc++;
        op = w >> 24;
        d = w & 0xFFFFFF;
        switch (op) {
        case 0x01: vaddr = host_addr(rel(d)); break;
        case 0x02: iaddr = host_addr(rel(d)); break;
        case 0x04: prim(d); break;
        case 0x05: case 0x06: log_once(12, "BEZIER/SPLINE (skipped)"); break;
        case 0x08: jump(l, rel(d & 0xFFFFFC), 0); break;
        case 0x09: break;   /* BJUMP: the bounding box always passes */
        case 0x0A: jump(l, rel(d & 0xFFFFFC), 1); break;
        case 0x0B: ret(l); break;
        case 0x0C:   /* END */
            if (prev >> 24 == 0x0F) {
                callback(l->cbid, 1, prev & 0xFFFF);
                return 1;
            }
            if (prev >> 24 == 0x0E) {
                u32 b = prev >> 16 & 0xFF, a = (prev & 0xFFFF) << 16 | (d & 0xFFFF);
                switch (b) {
                case 0x01: case 0x02: case 0x03: callback(l->cbid, 0, prev & 0xFFFF); break;
                case 0x08: break;
                case 0x10: jump(l, a, 0); break;
                case 0x11: jump(l, a, 1); break;
                case 0x12: ret(l); break;
                default: log_once(13, "unknown SIGNAL behavior"); break;
                }
                break;
            }
            return 1;
        case 0x13: offset_addr = d << 8; break;
        case 0x14: offset_addr = cur_pc & 0x0FFFFFFF; break;
        case 0x2A: bonei = d & 0x7F; break;
        case 0x2B: if (bonei < 96) bone[bonei++] = f24(d); break;
        case 0x2C: case 0x2D: case 0x2E: case 0x2F: case 0x30: case 0x31: case 0x32: case 0x33:
            morphw[op - 0x2C] = f24(d);
            break;
        case 0x3A: worldi = d & 15; break;
        case 0x3B: if (worldi < 12) world[worldi++] = f24(d); break;
        case 0x3C: viewi = d & 15; break;
        case 0x3D: if (viewi < 12) view[viewi++] = f24(d); break;
        case 0x3E: proji = d & 15; break;
        case 0x3F: if (proji < 16) proj[proji++] = f24(d); break;
        case 0x40: tgeni = d & 15; break;
        case 0x41: if (tgeni < 12) tgen[tgeni++] = f24(d); break;
        case 0xC4: {
            u32 n = (d & 0x3F) * 32;
            memcpy(clut, host_addr((reg[0xB0] & 0xFFFFF0) | (reg[0xB1] & 0xFF0000) << 8), n);
            break;
        }
        case 0xEA: transfer(d); break;
        default:
            if (!memchr(known, (int)op, sizeof known - 1)) {
                char s[32];
                snprintf(s, sizeof s, "unknown command 0x%02X", op);
                log_once(0x100 | op, s);
            }
            break;
        }
        reg[op] = d;
        prev = w;
    }
}

static int running;

static void run(void)
{
    if (running)
        return;
    running = 1;
    gl_setup();
    if (gl_on)
        host_gfx_bind();
    while (nq) {
        GeList *l = &lists[queue[0]];
        if (!exec(l))
            break;
        l->used = 0;
        nq--;
        memmove(queue, queue + 1, nq * sizeof *queue);
    }
    running = 0;
    while (npend) {   /* the callbacks, after the interpreter (they may queue more lists) */
        void (*fn)(int, void *) = pend[0].fn;
        void *arg = pend[0].arg;
        int id = pend[0].id;
        npend--;
        memmove(pend, pend + 1, npend * sizeof *pend);
        fn(id, arg);
    }
}

static int enqueue(const void *list, void *stall, int cbid, int head)
{
    int i;

    for (i = 0; i < 16 && lists[i].used; i++)
        ;
    if (i == 16 || nq == 16) {
        log_once(14, "list queue full");
        return 0x80000021;
    }
    lists[i].used = 1;
    lists[i].pc = (u32 *)list;
    lists[i].stall = stall;
    lists[i].cbid = cbid;
    lists[i].sp = 0;
    if (head) {
        memmove(queue + 1, queue, nq * sizeof *queue);
        queue[0] = i;
    } else {
        queue[nq] = i;
    }
    nq++;
    run();
    return i;
}

int sceGeListEnQueue(const void *list, void *stall, int cbid, void *arg) { return enqueue(list, stall, cbid, 0); }
int sceGeListEnQueueHead(const void *list, void *stall, int cbid, void *arg) { return enqueue(list, stall, cbid, 1); }

int sceGeListUpdateStallAddr(int qid, void *stall)
{
    if (qid < 0 || qid >= 16 || !lists[qid].used)
        return 0;
    lists[qid].stall = stall;
    run();
    return 0;
}

int sceGeListSync(int qid, int mode) { return 0; }
int sceGeDrawSync(int mode) { return 0; }
int sceGeBreak(int mode, void *p) { return 0; }
int sceGeContinue(void) { return 0; }

int sceGeSetCallback(void *cb)
{
    int i;

    for (i = 0; i < 16 && (cbs[i].signal || cbs[i].finish); i++)
        ;
    if (i == 16)
        return 0x80000022;
    memcpy(&cbs[i], cb, sizeof cbs[i]);
    return i;
}

int sceGeUnsetCallback(int cbid)
{
    if (cbid >= 0 && cbid < 16)
        memset(&cbs[cbid], 0, sizeof cbs[cbid]);
    return 0;
}

/* --- display --- */

static void dump(Fbo *f)
{
    static const char *pat;
    static int every = -1;
    static u8 px[480 * 272 * 4];
    char name[256];
    FILE *o;
    int y, x;

    if (every < 0) {
        const char *e = getenv("TAGFORCE_DUMP_EVERY");
        pat = getenv("TAGFORCE_DUMP");
        every = e && atoi(e) > 0 ? atoi(e) : 1;
    }
    if (!pat || frame_no % every)
        return;
    snprintf(name, sizeof name, pat, frame_no);
    if (!(o = fopen(name, "wb")))
        return;
    if (f)
        glReadPixels(0, 0, 480, 272, GL_RGBA, GL_UNSIGNED_BYTE, px);
    else
        memset(px, 0, sizeof px);
    fprintf(o, "P6\n480 272\n255\n");
    for (y = 0; y < 272; y++)
        for (x = 0; x < 480; x++)
            fwrite(px + (y * 480 + x) * 4, 1, 3, o);
    fclose(o);
}

/* EDRAM offset the CPU wrote last through sceDmacMemcpy (the movie player's frames), -1 when none */
static u32 cpu_fb = (u32)-1;

int sceDmacMemcpy(void *dst, const void *src, unsigned int size)
{
    memcpy(dst, src, size);
    if (in_edram((u32)dst))
        cpu_fb = (u32)dst & 0x1FFFFF;
    return 0;
}

int sceDisplaySetFrameBuf(void *top, int bufw, int fmt, int sync)
{
    Fbo *f = 0;
    int i;

    frame_no++;
    draw_no = 0;
    gl_setup();
    if (!gl_on)
        return 0;
    host_gfx_bind();
    if (top && in_edram((u32)top) && ((u32)top & 0x1FFFFF) == cpu_fb) {   /* CPU pixels -> the FBO shown */
        cpu_fb = (u32)-1;
        if (fmt != 3) {
            log_once(15, "CPU-written frame buffer not in 8888");
        } else {
            f = fbo_get((u32)top & 0x1FFFFF, bufw);
            glBindTexture(GL_TEXTURE_2D, f->tex);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, bufw);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, f->w, 272, GL_RGBA, GL_UNSIGNED_BYTE, edram + f->fbp);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        }
    }
    if (top && in_edram((u32)top))
        for (i = 0; i < nfbo; i++)
            if (fbos[i].fbp == ((u32)top & 0x1FFFFF) && fbos[i].fbw == (u32)bufw)
                f = &fbos[i];
    glBindFramebuffer(GL_READ_FRAMEBUFFER, f ? f->fbo : 0);
    dump(f);
    if (f)
        host_present();
    return 0;
}
