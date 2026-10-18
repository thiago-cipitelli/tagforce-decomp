/* EHF command handlers: shared types (see src/BOOT/ehf_cmd.c). */
#ifndef EHFCMD_H
#define EHFCMD_H

typedef struct Vec4 {
    float x, y, z, w;
} __attribute__((aligned(16))) Vec4;

typedef struct Mat4 {
    float m[16];
} __attribute__((aligned(16))) Mat4;

typedef struct EhfNode {
    int frame;
    int unk4;
    float time;                        /* +0x08 */
} EhfNode;

typedef union EhfArg {
    int i;
    float f;
    void *p;
} EhfArg;

typedef struct EhfCmd {
    unsigned char pad0[3];
    unsigned char argc;                /* +0x03 */
    unsigned char pad4[0xC];
    union {
        int i[1];
        float f[1];
        void *p[1];
    } a;                               /* +0x10 */
} EhfCmd;


typedef struct DrawSphere {
    Vec4 pos;
    float radius;                      /* +0x10 */
    float fovy;                        /* +0x14 */
    float near;                        /* +0x18 */
    float far;                         /* +0x1C */
} DrawSphere;

typedef struct EhfMtxState {           /* instance state of command 0x10003 */
    Mat4 m;
    Mat4 *ref;                         /* +0x40 */
    int kind;                          /* +0x44 0 none, 1 multiply m, 2 multiply *ref, 3 load m */
    int id;                            /* +0x48 */
} EhfMtxState;


int ehf_cmd_args(int *state, EhfCmd *cmd, int first, void **out, char *fmt);
unsigned int *dl_select_layer(int layer, unsigned int *size);
int dl_set_list(unsigned int *list);
unsigned int *postfx_get_param(void);
void postfx_set_param(unsigned int *p);
float msl_sinf(float x);
void mtx_push(void);
void mtx_pop(void);
void mtx_identity(void);
void mtx_translate(Vec4 *v);
void mtx_rotate_xyz(Vec4 *v);
void mtx_mult(Mat4 *m);
void mtx_load(Mat4 *m);
void mtx_get34(Mat4 *m);
void mtx_set_mode(int mode);
void mtx_get_proj(float *fovy, float *aspect, float *near, float *far);
void mtx_get_view_dir(Vec4 *out);
int draw_sphere_visible(DrawSphere *s);
void mat4_identity(Mat4 *m);
void mat4_rotate_xyz(Mat4 *out, Mat4 *m, Vec4 *v);
void mat4_transform_vec3(Mat4 *out, Mat4 *m, Vec4 *v);
void mat4_transform_vec4(Vec4 *out, Mat4 *m, Vec4 *v);
void mat4_transpose(Mat4 *out, Mat4 *m);
void quat_to_mat4(Mat4 *out, Vec4 *q);
void vec3_normalize2(Vec4 *out, Vec4 *v);
void vec3_cross(Vec4 *out, Vec4 *a, Vec4 *b);
void tgm_get_bone_matrix(void *model, Mat4 *out, int bone);

#define FRAME16(n) ((int)*(unsigned short *)&(n)->frame)

#endif
