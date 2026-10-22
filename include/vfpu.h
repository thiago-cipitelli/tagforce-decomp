#ifndef VFPU_H
#define VFPU_H
/* Sony's libvfpu (sceVfpu*) and the FPU trig routines next to it, linked into BOOT and exported to the
   gmodules through libehsys; the SDK names come from the export NIDs. Matrices are column-major (x, y,
   z, w are the columns, as the VFPU loads them). Float parameters come last, as in the SDK; the EABI
   passes them in $f12/$f13 whatever their position. */

typedef struct ScePspFVector2 {
    float x, y;
} ScePspFVector2;

typedef struct ScePspFVector3 {
    float x, y, z;
} ScePspFVector3;

typedef struct ScePspFVector4 {
    float x, y, z, w;
} ScePspFVector4;

typedef struct ScePspIVector2 {
    int x, y;
} ScePspIVector2;

typedef struct ScePspIVector3 {
    int x, y, z;
} ScePspIVector3;

typedef struct ScePspIVector4 {
    int x, y, z, w;
} ScePspIVector4;

typedef struct ScePspFMatrix2 {
    ScePspFVector2 x, y;
} ScePspFMatrix2;

typedef struct ScePspFMatrix3 {
    ScePspFVector3 x, y, z;
} ScePspFMatrix3;

typedef struct ScePspFMatrix4 {
    ScePspFVector4 x, y, z, w;
} ScePspFMatrix4;

int isinf_sign(float x);
float wrap_angle(float x);
float sinf(float x);
float cosf(float x);
float atanf(float x);
float asinf(float x);
float acosf(float x);

ScePspFVector2 *vec2_sign_float(ScePspFVector2 *out, const ScePspFVector2 *v);
ScePspIVector2 *vec2_sign_int(ScePspIVector2 *out, const ScePspFVector2 *v);
ScePspFVector2 *vec2_set(ScePspFVector2 *out, float x, float y);
ScePspFVector2 *vec2_copy(ScePspFVector2 *out, const ScePspFVector2 *v);
ScePspFVector2 *vec2_zero(ScePspFVector2 *out);
ScePspFVector2 *vec2_neg_zero(ScePspFVector2 *out);
ScePspIVector2 *vec2_ceil(ScePspIVector2 *out, const ScePspFVector2 *v);
ScePspIVector2 *vec2_trunc(ScePspIVector2 *out, const ScePspFVector2 *v);
ScePspIVector2 *vec2_round(ScePspIVector2 *out, const ScePspFVector2 *v);
ScePspIVector2 *vec2_floor(ScePspIVector2 *out, const ScePspFVector2 *v);
ScePspFVector2 *vec2_from_ivec(ScePspFVector2 *out, const ScePspIVector2 *v);
ScePspFVector2 *vec2_add(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b);
ScePspFVector2 *vec2_sub(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b);
ScePspFVector2 *vec2_mul(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b);
ScePspFVector2 *vec2_div(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b);
ScePspFVector2 *vec2_neg(ScePspFVector2 *out, const ScePspFVector2 *v);
ScePspFVector2 *vec2_abs(ScePspFVector2 *out, const ScePspFVector2 *v);
ScePspFVector2 *vec2_lerp(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b, float t);
ScePspFVector2 *vec2_scale(ScePspFVector2 *out, const ScePspFVector2 *v, float s);
ScePspFVector2 *vec2_hermite(ScePspFVector2 *out, const ScePspFVector2 *p0, const ScePspFVector2 *v0,
                             const ScePspFVector2 *p1, const ScePspFVector2 *v1, float t);
ScePspFVector2 *vec2_clamp(ScePspFVector2 *out, const ScePspFVector2 *v, float lo, float hi);
ScePspFVector2 *vec2_max(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b);
ScePspFVector2 *vec2_min(ScePspFVector2 *out, const ScePspFVector2 *a, const ScePspFVector2 *b);
float vec2_dot(const ScePspFVector2 *a, const ScePspFVector2 *b);
float vec2_funnel(const ScePspFVector2 *v);
float vec2_average(const ScePspFVector2 *v);
int vec2_is_equal(const ScePspFVector2 *a, const ScePspFVector2 *b);
int vec2_is_zero(const ScePspFVector2 *v);
ScePspFVector2 *vec2_normalize(ScePspFVector2 *out, const ScePspFVector2 *v);
float vec2_length(const ScePspFVector2 *v);
float vec2_distance(const ScePspFVector2 *a, const ScePspFVector2 *b);
ScePspFVector2 *vec2_faceforward(ScePspFVector2 *out, const ScePspFVector2 *n, const ScePspFVector2 *i,
                                 const ScePspFVector2 *nref);
ScePspFVector2 *vec2_reflect(ScePspFVector2 *out, const ScePspFVector2 *v, const ScePspFVector2 *n);
ScePspFVector2 *vec2_refract(ScePspFVector2 *out, const ScePspFVector2 *v, const ScePspFVector2 *n, float eta);

ScePspFVector3 *vec3_set(ScePspFVector3 *out, float x, float y, float z);
ScePspFVector3 *vec3_copy(ScePspFVector3 *out, const ScePspFVector3 *v);
ScePspFVector3 *vec3_zero(ScePspFVector3 *out);
ScePspFVector3 *vec3_neg_zero(ScePspFVector3 *out);
ScePspIVector3 *vec3_ceil(ScePspIVector3 *out, const ScePspFVector3 *v);
ScePspIVector3 *vec3_trunc(ScePspIVector3 *out, const ScePspFVector3 *v);
ScePspIVector3 *vec3_round(ScePspIVector3 *out, const ScePspFVector3 *v);
ScePspIVector3 *vec3_floor(ScePspIVector3 *out, const ScePspFVector3 *v);
ScePspFVector3 *vec3_from_ivec(ScePspFVector3 *out, const ScePspIVector3 *v);
ScePspFVector3 *vec3_add(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b);
ScePspFVector3 *vec3_sub(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b);
ScePspFVector3 *vec3_mul(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b);
ScePspFVector3 *vec3_div(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b);
ScePspFVector3 *vec3_neg(ScePspFVector3 *out, const ScePspFVector3 *v);
ScePspFVector3 *vec3_abs(ScePspFVector3 *out, const ScePspFVector3 *v);
ScePspFVector3 *vec3_lerp(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b, float t);
ScePspFVector3 *vec3_hermite(ScePspFVector3 *out, const ScePspFVector3 *p0, const ScePspFVector3 *v0,
                             const ScePspFVector3 *p1, const ScePspFVector3 *v1, float t);
ScePspFVector3 *vec3_scale(ScePspFVector3 *out, const ScePspFVector3 *v, float s);
ScePspFVector3 *vec3_clamp(ScePspFVector3 *out, const ScePspFVector3 *v, float lo, float hi);
ScePspFVector3 *vec3_max(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b);
ScePspFVector3 *vec3_min(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b);
float vec3_dot(const ScePspFVector3 *a, const ScePspFVector3 *b);
ScePspFVector3 *vec3_cross_nw(ScePspFVector3 *out, const ScePspFVector3 *a, const ScePspFVector3 *b);
float vec3_funnel(const ScePspFVector3 *v);
float vec3_average(const ScePspFVector3 *v);
int vec3_is_equal(const ScePspFVector3 *a, const ScePspFVector3 *b);
int vec3_is_zero(const ScePspFVector3 *v);
ScePspFVector3 *vec3_sign_float(ScePspFVector3 *out, const ScePspFVector3 *v);
ScePspIVector3 *vec3_sign_int(ScePspIVector3 *out, const ScePspFVector3 *v);
ScePspFVector3 *vec3_normalize(ScePspFVector3 *out, const ScePspFVector3 *v);
float vec3_length(const ScePspFVector3 *v);
float vec3_distance(const ScePspFVector3 *a, const ScePspFVector3 *b);
ScePspFVector3 *vec3_faceforward(ScePspFVector3 *out, const ScePspFVector3 *n, const ScePspFVector3 *i,
                                 const ScePspFVector3 *nref);
ScePspFVector3 *vec3_reflect(ScePspFVector3 *out, const ScePspFVector3 *v, const ScePspFVector3 *n);
ScePspFVector3 *vec3_refract(ScePspFVector3 *out, const ScePspFVector3 *v, const ScePspFVector3 *n, float eta);

ScePspFVector4 *vec4_set(ScePspFVector4 *out, float x, float y, float z, float w);
ScePspFVector4 *vec4_set_xyz(ScePspFVector4 *out, float x, float y, float z);
ScePspFVector4 *vec4_copy(ScePspFVector4 *out, const ScePspFVector4 *v);
ScePspFVector4 *vec4_zero(ScePspFVector4 *out);
ScePspFVector4 *vec4_neg_zero(ScePspFVector4 *out);
ScePspIVector4 *vec4_ceil(ScePspIVector4 *out, const ScePspFVector4 *v);
ScePspIVector4 *vec4_trunc(ScePspIVector4 *out, const ScePspFVector4 *v);
ScePspIVector4 *vec4_round(ScePspIVector4 *out, const ScePspFVector4 *v);
ScePspIVector4 *vec4_floor(ScePspIVector4 *out, const ScePspFVector4 *v);
ScePspFVector4 *vec4_from_ivec(ScePspFVector4 *out, const ScePspIVector4 *v);
ScePspFVector4 *vec4_add(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
ScePspFVector4 *vec3_add_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
ScePspFVector4 *vec4_sub(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
ScePspFVector4 *vec3_sub_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
ScePspFVector4 *vec4_mul(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
ScePspFVector4 *vec3_mul_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
ScePspFVector4 *vec4_div(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
ScePspFVector4 *vec3_div_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
ScePspFVector4 *vec4_neg(ScePspFVector4 *out, const ScePspFVector4 *v);
ScePspFVector4 *vec4_abs(ScePspFVector4 *out, const ScePspFVector4 *v);
ScePspFVector4 *vec4_lerp(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b, float t);
ScePspFVector4 *vec3_lerp_w(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b, float t);
ScePspFVector4 *vec4_hermite(ScePspFVector4 *out, const ScePspFVector4 *p0, const ScePspFVector4 *v0,
                             const ScePspFVector4 *p1, const ScePspFVector4 *v1, float t);
ScePspFVector4 *vec4_hermite_xyz(ScePspFVector4 *out, const ScePspFVector4 *p0, const ScePspFVector4 *v0,
                                 const ScePspFVector4 *p1, const ScePspFVector4 *v1, float t);
ScePspFVector4 *vec4_scale(ScePspFVector4 *out, const ScePspFVector4 *v, float s);
ScePspFVector4 *vec3_scale_w(ScePspFVector4 *out, const ScePspFVector4 *v, float s);
ScePspFVector4 *vec4_clamp(ScePspFVector4 *out, const ScePspFVector4 *v, float lo, float hi);
ScePspFVector4 *vec4_clamp_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, float lo, float hi);
ScePspFVector4 *vec4_max(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
ScePspFVector4 *vec4_min(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
float vec4_dot(const ScePspFVector4 *a, const ScePspFVector4 *b);
float vec3_dot2(const ScePspFVector4 *a, const ScePspFVector4 *b);
ScePspFVector4 *vec3_cross(ScePspFVector4 *out, const ScePspFVector4 *a, const ScePspFVector4 *b);
float vec4_funnel(const ScePspFVector4 *v);
float vec4_average(const ScePspFVector4 *v);
int vec4_is_equal(const ScePspFVector4 *a, const ScePspFVector4 *b);
int vec4_is_zero(const ScePspFVector4 *v);
ScePspFVector4 *vec4_sign_float(ScePspFVector4 *out, const ScePspFVector4 *v);
ScePspIVector4 *vec4_sign_int(ScePspIVector4 *out, const ScePspFVector4 *v);
ScePspFVector4 *vec4_normalize(ScePspFVector4 *out, const ScePspFVector4 *v);
ScePspFVector4 *vec3_normalize2(ScePspFVector4 *out, const ScePspFVector4 *v);
ScePspFVector4 *vec4_normalize_phase(ScePspFVector4 *out, const ScePspFVector4 *v);
float vec4_length_xyz(const ScePspFVector4 *v);
float vec4_distance_xyz(const ScePspFVector4 *a, const ScePspFVector4 *b);
ScePspFVector4 *vec4_faceforward_xyz(ScePspFVector4 *out, const ScePspFVector4 *n, const ScePspFVector4 *i,
                                     const ScePspFVector4 *nref);
ScePspFVector4 *vec4_reflect_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, const ScePspFVector4 *n);
ScePspFVector4 *vec4_refract_xyz(ScePspFVector4 *out, const ScePspFVector4 *v, const ScePspFVector4 *n, float eta);

ScePspFMatrix2 *mat2_identity(ScePspFMatrix2 *out);
ScePspFMatrix2 *mat2_zero(ScePspFMatrix2 *out);
ScePspFMatrix2 *mat2_copy(ScePspFMatrix2 *out, const ScePspFMatrix2 *m);
ScePspFVector2 *mat2_transform(ScePspFVector2 *out, const ScePspFMatrix2 *m, const ScePspFVector2 *v);
ScePspFMatrix2 *mat2_mul(ScePspFMatrix2 *out, const ScePspFMatrix2 *a, const ScePspFMatrix2 *b);
ScePspFMatrix2 *mat2_scale(ScePspFMatrix2 *out, const ScePspFMatrix2 *m, float s);
ScePspFMatrix2 *mat2_transpose(ScePspFMatrix2 *out, const ScePspFMatrix2 *m);
ScePspFMatrix2 *mat2_rot_z(ScePspFMatrix2 *out, const ScePspFMatrix2 *m, float angle);
int mat2_is_identity(const ScePspFMatrix2 *m);
float mat2_trace(const ScePspFMatrix2 *m);
float mat2_determinant(const ScePspFMatrix2 *m);
ScePspFMatrix2 *mat2_adjoint(ScePspFMatrix2 *out, const ScePspFMatrix2 *m);

ScePspFMatrix3 *mat3_identity(ScePspFMatrix3 *out);
ScePspFMatrix3 *mat3_zero(ScePspFMatrix3 *out);
ScePspFMatrix3 *mat3_copy(ScePspFMatrix3 *out, const ScePspFMatrix3 *m);
ScePspFVector3 *mat3_transform_vec3(ScePspFVector3 *out, const ScePspFMatrix3 *m, const ScePspFVector3 *v);
ScePspFMatrix3 *mat3_mul(ScePspFMatrix3 *out, const ScePspFMatrix3 *a, const ScePspFMatrix3 *b);
ScePspFMatrix3 *mat3_scale(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float s);
ScePspFMatrix3 *mat3_transpose(ScePspFMatrix3 *out, const ScePspFMatrix3 *m);
ScePspFMatrix3 *mat3_rot_z(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float angle);
ScePspFMatrix3 *mat3_rot_y(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float angle);
ScePspFMatrix3 *mat3_rot_x(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, float angle);
ScePspFMatrix3 *mat3_from_euler(ScePspFMatrix3 *out, const ScePspFMatrix3 *m, const ScePspFVector3 *angles);
int mat3_is_identity(const ScePspFMatrix3 *m);
float mat3_trace(const ScePspFMatrix3 *m);
float mat3_determinant(const ScePspFMatrix3 *m);
ScePspFMatrix3 *mat3_adjoint(ScePspFMatrix3 *out, const ScePspFMatrix3 *m);
ScePspFMatrix3 *mat3_inverse(ScePspFMatrix3 *out, float *det, const ScePspFMatrix3 *m);
ScePspFMatrix3 *mat3_normalize(ScePspFMatrix3 *out, const ScePspFMatrix3 *m);

ScePspFMatrix4 *mat4_identity(ScePspFMatrix4 *out);
ScePspFMatrix4 *mat4_zero(ScePspFMatrix4 *out);
ScePspFMatrix4 *mat4_copy(ScePspFMatrix4 *out, const ScePspFMatrix4 *m);
ScePspFMatrix4 *mat4_set_transfer(ScePspFMatrix4 *m, const ScePspFVector4 *v);
ScePspFVector4 *mat4_get_transfer(ScePspFVector4 *out, const ScePspFMatrix4 *m);
ScePspFVector4 *mat4_transform_vec4(ScePspFVector4 *out, const ScePspFMatrix4 *m, const ScePspFVector4 *v);
ScePspFVector4 *mat4_transform_xyz(ScePspFVector4 *out, const ScePspFMatrix4 *m, const ScePspFVector4 *v);

#endif
