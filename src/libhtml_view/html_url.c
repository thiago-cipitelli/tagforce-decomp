// @range 0x08804414-0x088044B8
// @data 0x08808788-0x08808830
/* Start page of the HTML viewer: Konami's Tag Force download page for the game's region. */
extern char *strcat(char *, const char *);
extern int region_get(void);

static const char html_url_jp[43];
static const char html_url_us[45];
static const char html_url_eu[45];

void html_url_build(char *out)
{
    switch (region_get()) {
    case 0:
        strcat(out, html_url_jp);
        break;
    case 1:
        strcat(out, html_url_us);
        break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        strcat(out, html_url_eu);
        break;
    default:
        strcat(out, html_url_us);
        break;
    }
}

static const char html_url_jp[43] = "http://www.konami.jp/gs/game/yugioh_tf/dl/";
static const char html_url_us[45] = "http://www.konami.jp/gs/game/yugioh_tf/dlus/";
static const char html_url_eu[45] = "http://www.konami.jp/gs/game/yugioh_tf/dleu/";
