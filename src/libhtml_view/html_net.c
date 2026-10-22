// @range 0x08804000-0x0880439C
/* Scene entry of the "web site" screen and bring-up/tear-down of the network stack the PSP HTML
   viewer needs: net modules, sceNet/Inet/Resolver/Apctl, then SSL, HTTP, HTTPS and the cookie store. */
#include "psp.h"
#include "ehsys.h"

void scene_pop_to(int scene, int arg);
int sceNetInit(int poolsize, int calloutprio, int calloutstack, int netintrprio, int netintrstack);
int sceNetTerm(void);
int sceNetInetInit(void);
int sceNetInetTerm(void);
int sceNetResolverInit(void);
int sceNetResolverTerm(void);
int sceNetApctlInit(int stacksize, int prio);
int sceNetApctlTerm(void);
int sceSslInit(int poolsize);
int sceSslEnd(void);
int sceHttpInit(int poolsize);
int sceHttpEnd(void);
int sceHttpsInit(int a, int b, int c, int d);
int sceHttpsEnd(void);
int sceHttpsLoadDefaultCert(int a, int b);
int sceHttpLoadSystemCookie(void);
int sceHttpSaveSystemCookie(void);

void html_init(void);
int html_step(void);
void html_http_end(void);
void html_net_modules_unload(void);
void html_net_term(void);

extern char html_scene_data[];

void html_main(void);
void html_exit(void);

int module_entry(void)
{
    ehscript_register((int (*)(void))html_main, (int (*)(void))html_exit, html_scene_data);
    return 0;
}

void html_main(void)
{
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    html_init();
    for (;;) {
        if (html_step() != 0) {
            break;
        }
        sys_frame_wait();
    }
    scene_pop_to(14, 0);
}

void html_exit(void)
{
}

int html_net_modules_load(void)
{
    int r;

    r = sceUtilityLoadNetModule(1);
    if (r < 0) goto fail;
    r = sceUtilityLoadNetModule(3);
    if (r < 0) goto fail;
    r = sceUtilityLoadNetModule(4);
    if (r < 0) goto fail;
    r = sceUtilityLoadNetModule(5);
    if (r < 0) goto fail;
    r = sceUtilityLoadNetModule(6);
    if (r < 0) goto fail;
    r = sceUtilityLoadNetModule(7);
    if (r < 0) goto fail;
    return r;
fail:
    html_net_modules_unload();
    return r;
}

void html_net_modules_unload(void)
{
    sceUtilityUnloadNetModule(7);
    sceUtilityUnloadNetModule(6);
    sceUtilityUnloadNetModule(5);
    sceUtilityUnloadNetModule(4);
    sceUtilityUnloadNetModule(3);
    sceUtilityUnloadNetModule(1);
}

int html_http_init(void)
{
    int r;

    r = sceSslInit(0x28000);
    if (r < 0) {
        return r;
    }
    r = sceHttpInit(0x28000);
    if (r < 0) {
        html_http_end();
        return r;
    }
    r = sceHttpsInit(0, 0, 0, 0);
    if (r < 0) {
        html_http_end();
        return r;
    }
    r = sceHttpsLoadDefaultCert(0, 0);
    if (r < 0) {
        html_http_end();
        return r;
    }
    r = sceHttpLoadSystemCookie();
    if (r < 0) {
        html_http_end();
        return r;
    }
    return r;
}

void html_http_end(void)
{
    sceHttpSaveSystemCookie();
    sceHttpsEnd();
    sceHttpEnd();
    sceSslEnd();
}

int html_net_init(void)
{
    int r;

    r = html_net_modules_load();
    if (r < 0) {
        return r;
    }
    r = sceNetInit(0x20000, 42, 0, 42, 0);
    if (r < 0) {
        return r;
    }
    r = sceNetInetInit();
    if (r < 0) {
        html_net_term();
        return r;
    }
    r = sceNetResolverInit();
    if (r < 0) {
        html_net_term();
        return r;
    }
    r = sceNetApctlInit(0x1800, 48);
    if (r < 0) {
        html_net_term();
        return r;
    }
    r = html_http_init();
    if (r < 0) {
        html_net_term();
        return r;
    }
    return r;
}

void html_net_term(void)
{
    html_http_end();
    sceNetApctlTerm();
    sceNetResolverTerm();
    sceNetInetTerm();
    sceNetTerm();
    html_net_modules_unload();
}
