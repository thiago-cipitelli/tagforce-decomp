/* Allegrex-specific intrinsics used by the original code (mwccpsp builtins). The PC build gets plain C. */
#ifndef ALLEGREX_H
#define ALLEGREX_H
#ifdef __MWERKS__
#define ALLEGREX_MAX(a, b) __builtin_allegrex_max(a, b)
#define ALLEGREX_MIN(a, b) __builtin_allegrex_min(a, b)
#define ALLEGREX_INS(dst, src, pos, size) __builtin_allegrex_ins(dst, src, pos, size)
#define ALLEGREX_EXT(src, pos, size) __builtin_allegrex_ext(src, pos, size)
#else
#define ALLEGREX_MAX(a, b) ((a) > (b) ? (a) : (b))
#define ALLEGREX_MIN(a, b) ((a) < (b) ? (a) : (b))
#define ALLEGREX_INS(dst, src, pos, size) (((dst) & ~((((1u << (size)) - 1u)) << (pos))) | (((src) & ((1u << (size)) - 1u)) << (pos)))
#define ALLEGREX_EXT(src, pos, size) (((src) >> (pos)) & ((1u << (size)) - 1u))
#endif
#endif
