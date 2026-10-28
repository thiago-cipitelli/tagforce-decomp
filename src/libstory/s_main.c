// @range 0x08804000-0x08804270
/* Scene 2 (story): module entry, the scene thread (init of every subsystem, the story loop run once
   per frame until it ends, then the term of every subsystem and the return to the caller scene). */
#include "libstory.h"

void story_main(void);
void story_exit(void);
int story_draw_cb(void);
void story_exit_cb(void);
void story_init_all(void);
void story_term_all(void);
int story_start(void);
void story_update(void);
int story_done(void);
void story_leave(void);
int story_draw(void);

extern unsigned char story_scene_data[];

void st_res_init(void);
void st_init_812728(void);
void st_init_80c9e4(void);
void st_init_816890(void);
void st_pl_init(void);
void st_chr_init(void);
void st_init_8132fc(void);
void scal_init(void);
void st_init_806a18(void);
void st_init_810ae4(void);
void sev_init(void);
void st_pickup_init(void);
void stalk_init(void);
void st_init_815b54(void);
void st_init_8165f8(void);
void st_init_8171f4(void);
void st_init_810d24(void);
void st_init_817500(void);
void st_init_8177c4(void);
void st_init_817930(void);
void st_term_810af8(void);
void st_term_806a48(void);
void scal_term(void);
void st_term_813310(void);
void st_chr_term(void);
void st_pl_term(void);
void sev_term(void);
void st_pickup_term(void);
void stalk_term(void);
void st_term_815b68(void);
void st_term_80c9f8(void);
void st_term_8168a4(void);
void st_term_81660c(void);
void st_term_817208(void);
void st_term_810d38(void);
void st_term_817514(void);
void st_term_8177d8(void);
void st_term_817944(void);
void st_term_81273c(void);
void st_res_term(void);

int module_entry(int arg)
{
    if (arg == 0)
        ehscript_register((int (*)(void))story_main, (int (*)(void))story_exit, story_scene_data);
    return 0;
}

void story_main(void)
{
    sceKernelChangeCurrentThreadAttr(0, 0x4000);
    rand_seed_from_rtc();
    story_init_all();
    sys_frame_wait();
    sys_state_set_callbacks(2, story_draw_cb, (int (*)(void))story_exit_cb);
    if (story_start()) {
        do {
            story_update();
            sys_frame_wait();
        } while (!story_done());
    }
    sys_state_set_callbacks(0, 0, 0);
    gfx_overlay_enable(1);
    sys_frame_wait();
    story_term_all();
    story_leave();
}

void story_init_all(void)
{
    st_res_init();
    st_init_812728();
    st_init_80c9e4();
    st_init_816890();
    st_pl_init();
    st_chr_init();
    st_init_8132fc();
    scal_init();
    st_init_806a18();
    st_init_810ae4();
    sev_init();
    st_pickup_init();
    stalk_init();
    st_init_815b54();
    st_init_8165f8();
    st_init_8171f4();
    st_init_810d24();
    st_init_817500();
    st_init_8177c4();
    st_init_817930();
}

void story_term_all(void)
{
    st_term_810af8();
    st_term_806a48();
    scal_term();
    st_term_813310();
    st_chr_term();
    st_pl_term();
    sev_term();
    st_pickup_term();
    stalk_term();
    st_term_815b68();
    st_term_80c9f8();
    st_term_8168a4();
    st_term_81660c();
    st_term_817208();
    st_term_810d38();
    st_term_817514();
    st_term_8177d8();
    st_term_817944();
    st_term_81273c();
    st_res_term();
}

void story_exit(void)
{
}

int story_draw_cb(void)
{
    return story_draw();
}

void story_exit_cb(void)
{
}
