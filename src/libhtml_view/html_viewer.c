// @range 0x08804508-0x088047F4
/* sceUtilityHtmlViewer session: parameters (download folder = the game's save folder, start page by
   region, 6 MB working memory in .bss) and the per-frame state machine net init -> viewer -> net term. */
typedef struct {
    unsigned int size;
    int language;
    int buttonSwap;
    int graphicsThread;
    int accessThread;
    int fontThread;
    int soundThread;
    int result;
    int reserved[4];
    void *memaddr;
    unsigned int memsize;
    int unknown1;
    int unknown2;
    char *initialurl;
    unsigned int numtabs;
    unsigned int interfacemode;
    unsigned int options;
    char *dldirname;
    char *dlfilename;
    char *uldirname;
    char *ulfilename;
    unsigned int cookiemode;
    unsigned int unknown3;
    char *homeurl;
    unsigned int textsize;
    unsigned int connectmode;
    unsigned int disconnectmode;
    unsigned int memused;
    int unknown4[10];
} SceUtilityHtmlViewerParam;

extern void *memset(void *, int, unsigned int);
extern int lang_to_dl_index(void);
extern short region_pick_value(int i);
extern void html_url_build(char *out);
extern void save_path_build(char *out);
extern int sceUtilityHtmlViewerInitStart(SceUtilityHtmlViewerParam *p);
extern int sceUtilityHtmlViewerGetStatus(void);
extern int sceUtilityHtmlViewerUpdate(int visible);
extern int sceUtilityHtmlViewerShutdownStart(void);
extern void sceGuSync(int mode, int what);
extern int html_net_init(void);
extern void html_net_term(void);

extern int html_state;
extern SceUtilityHtmlViewerParam html_param;
extern char html_url[0x200];
extern char html_savedir[0x84];
extern void *html_mem_ptr;
extern char html_mem[];

int html_viewer_start(void *mem)
{
    SceUtilityHtmlViewerParam *const p = &html_param;

    memset(p, 0, sizeof *p);
    p->size = sizeof *p;
    p->language = lang_to_dl_index();
    if ((unsigned short)region_pick_value(0) == 0x2000) {
        p->buttonSwap = 0;
    } else {
        p->buttonSwap = 1;
    }
    p->soundThread = 16;
    p->graphicsThread = 17;
    p->fontThread = 18;
    p->accessThread = 19;
    p->memsize = 0x600000;
    p->memaddr = mem;
    html_url_build(html_url);
    p->initialurl = html_url;
    p->numtabs = 3;
    p->interfacemode = 2;
    p->options = 0x2BA;
    save_path_build(html_savedir);
    p->dldirname = html_savedir;
    p->cookiemode = 3;
    p->unknown3 = 0x200;
    p->textsize = 1;
    p->connectmode = 1;
    p->disconnectmode = 1;
    p->homeurl = html_url;
    p->dlfilename = 0;
    p->ulfilename = 0;
    p->uldirname = 0;
    p->memused = 0;
    return sceUtilityHtmlViewerInitStart(p);
}

int html_viewer_update(void)
{
    SceUtilityHtmlViewerParam *p = &html_param;

    sceGuSync(0, 0);
    switch (sceUtilityHtmlViewerGetStatus()) {
    case 1:
    case 4:
        break;
    case 2:
        sceUtilityHtmlViewerUpdate(2);
        break;
    case 3:
        sceUtilityHtmlViewerShutdownStart();
        break;
    case 0:
        return p->result == 0 ? 1 : 2;
    }
    return 0;
}

void html_init(void)
{
    html_state = 1;
    html_mem_ptr = html_mem;
}

int html_step(void)
{
    switch (html_state) {
    case 1:
        if (html_net_init() < 0) {
            html_state = 3;
            break;
        }
        if (html_viewer_start(html_mem_ptr) < 0) {
            html_state = 3;
            break;
        }
        html_state++;
        break;
    case 2:
        if (html_viewer_update() != 0) {
            html_state++;
        }
        break;
    case 3:
        html_net_term();
        html_state++;
        break;
    case 4:
        return 1;
    }
    return 0;
}
