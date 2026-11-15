/* What the newlib libm sources of BOOT (libm_*.c, compiled by the SDK's GCC; docs/build.md, "Trilha GCC") take from
   newlib 1.13.0's fdlibm.h, math.h and errno.h, reduced to the parts they use, plus the names those functions have
   in symbols/BOOT.txt (the plain names belong to the VFPU versions, see vfpu_math.c). Private to src/BOOT. */
#ifndef TF_FDLIBM_H
#define TF_FDLIBM_H

typedef int __int32_t;
typedef unsigned int __uint32_t;

/* names in BOOT */
#define atanf libm_atanf
#define asinf libm_asinf
#define copysignf libm_copysignf
#define cosf libm_cosf
#define fabsf libm_fabsf
#define floorf libm_floorf
#define isnanf libm_isnanf
#define scalbnf libm_scalbnf
#define sinf msl_sinf
#define sqrtf libm_sqrtf
#ifdef __mips__
#define __fdlib_version D_08873880     /* _LIB_VERSION, in BOOT's .data (1 = _XOPEN_) */
#endif

#define __P(p) p

/* fdlibm.h: float with denormals and infinities */
#define FLT_UWORD_IS_FINITE(x) ((x)<0x7f800000L)
#define FLT_UWORD_IS_NAN(x) ((x)>0x7f800000L)
#define FLT_UWORD_IS_INFINITE(x) ((x)==0x7f800000L)
#define FLT_UWORD_MAX 0x7f7fffff
#define FLT_UWORD_EXP_MAX 0x43000000
#define FLT_UWORD_LOG_MAX 0x42b17217
#define FLT_UWORD_LOG_2MAX 0x42b2d4fc
#define HUGE ((float)3.40282346638528860e+38)
#define FLT_UWORD_HALF_MAX (FLT_UWORD_MAX-(1L<<23))
#define FLT_LARGEST_EXP (FLT_UWORD_MAX>>23)
#define FLT_UWORD_IS_ZERO(x) ((x)==0)
#define FLT_UWORD_IS_SUBNORMAL(x) ((x)<0x00800000L)
#define FLT_UWORD_MIN 0x00000001
#define FLT_UWORD_EXP_MIN 0x43160000
#define FLT_UWORD_LOG_MIN 0x42cff1b5
#define FLT_SMALLEST_EXP -22

typedef union
{
  float value;
  __uint32_t word;
} ieee_float_shape_type;

/* Get a 32 bit int from a float.  */
#define GET_FLOAT_WORD(i,d)					\
do {								\
  ieee_float_shape_type gf_u;					\
  gf_u.value = (d);						\
  (i) = gf_u.word;						\
} while (0)

/* Set a float from a 32 bit int.  */
#define SET_FLOAT_WORD(d,i)					\
do {								\
  ieee_float_shape_type sf_u;					\
  sf_u.word = (i);						\
  (d) = sf_u.value;						\
} while (0)

/* math.h */
struct exception
{
  int type;
  char *name;
  double arg1;
  double arg2;
  double retval;
  int err;
};
extern int matherr(struct exception *e);
#define DOMAIN 1
#define SING 2
#define OVERFLOW 3
#define UNDERFLOW 4
#define TLOSS 5
#define PLOSS 6

enum __fdlibm_version
{
  __fdlibm_ieee = -1,
  __fdlibm_svid,
  __fdlibm_xopen,
  __fdlibm_posix
};
#define _LIB_VERSION_TYPE enum __fdlibm_version
#define _LIB_VERSION __fdlib_version
extern const _LIB_VERSION_TYPE _LIB_VERSION;
#define _IEEE_  __fdlibm_ieee
#define _SVID_  __fdlibm_svid
#define _XOPEN_ __fdlibm_xopen
#define _POSIX_ __fdlibm_posix

/* errno.h */
extern int *__errno(void);
#define errno (*__errno())
#define EDOM 33
#define ERANGE 34

extern float atanf __P((float));
extern float asinf __P((float));
extern float atan2f __P((float, float));
extern float copysignf __P((float, float));
extern float cosf __P((float));
extern float fabsf __P((float));
extern float floorf __P((float));
extern int isnanf __P((float));
extern float scalbnf __P((float, int));
extern float sinf __P((float));
extern float sqrtf __P((float));

/* ieee style elementary float functions */
extern float __ieee754_sqrtf __P((float));
extern float __ieee754_asinf __P((float));
extern float __ieee754_atan2f __P((float,float));
extern __int32_t __ieee754_rem_pio2f __P((float,float*));

/* float versions of fdlibm kernel functions */
extern float __kernel_sinf __P((float,float,int));
extern float __kernel_cosf __P((float,float));
extern int   __kernel_rem_pio2f __P((float*,float*,int,int,int,const __int32_t*));

#endif
