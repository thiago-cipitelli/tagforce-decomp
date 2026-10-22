// @range 0x08820ED4-0x08822F84
// @asm 0x08820ED4-0x08822F84
/* EHF command argument converter (ehf_cmd_args). A command's arguments start at word `first` of
   cmd->a with a pointer to a type string, one letter per source value, followed by the values:
     F inline float   N inline int   f pointer to float   n pointer to int            (scalars)
     V inline floats  I inline ints  v pointer to floats  i pointer to ints           (vectors)
   `fmt` has one letter per wanted value, each written through the matching out[] pointer:
     n o p q / O P Q N   1/2/3/4 ints        f g h i / G H I F   1/2/3/4 floats
     r                   the raw argument word
   A vector source fills all components at once (one type letter, one word for v/i, one word per
   component for V/I); scalar sources give one component each (one letter and one word apiece, and
   a component whose letter is not a scalar type is left untouched). Conversion between int and
   float truncates. Stops at the end of the arguments, at the end of fmt, or at an unknown letter.
   state (or a local when 0) gets {values written, type letters used, argument words used}; returns
   the first.
   The original is one generated block per destination letter (the same macro for the upper and
   lower case sets, which only differ in the component counts), 8 KB in all; it stays in asm. */
#include "psp.h"
#include "libc.h"
#include "ehfcmd.h"

#ifdef NON_MATCHING /* equivalente funcional para o port: o original é um bloco gerado por macro para cada letra de destino */
/* Component count of a destination letter (0 when unknown), and whether it is a float. */
static int ehf_arg_dest(char c, int *is_float)
{
    static const char letters[] = "nopqOPQNfghiGHIF";
    int i;

    for (i = 0; letters[i] != 0; i++) {
        if (letters[i] == c) {
            *is_float = i >= 8;
            return i % 4 + 1;
        }
    }
    return 0;
}

static void ehf_arg_store(EhfArg *dst, EhfArg v, int src_float, int dst_float)
{
    if (src_float == dst_float) {
        *dst = v;
    } else if (dst_float) {
        dst->f = (float)v.i;
    } else {
        dst->i = (int)v.f;
    }
}

int ehf_cmd_args(int *state, EhfCmd *cmd, int first, void **out, char *fmt)
{
    int local[3];
    EhfArg *a = (EhfArg *)&cmd->a.i[first];
    char *types = a->p;
    char *t = types;
    EhfArg *start = a + 1;
    EhfArg *p = start;
    EhfArg *end = (EhfArg *)&cmd->a.i[cmd->argc];
    EhfArg *dst;
    EhfArg *src;
    int n, is_float, k;

    if (state == 0) {
        state = local;
    }
    memset(state, 0, 12);
    for (; p < end && *fmt != 0; fmt++, out++, state[0]++) {
        dst = *out;
        if (*fmt == 'r') {
            *dst = *p++;
            t++;
            continue;
        }
        n = ehf_arg_dest(*fmt, &is_float);
        if (n == 0) {
            break;
        }
        switch (*t) {
        case 'F':
        case 'N':
        case 'f':
        case 'n':
            for (k = 0; k < n; k++, t++) {
                switch (*t) {
                case 'F':
                case 'N':
                    ehf_arg_store(&dst[k], *p++, *t == 'F', is_float);
                    break;
                case 'f':
                case 'n':
                    ehf_arg_store(&dst[k], *(EhfArg *)(p++)->p, *t == 'f', is_float);
                    break;
                }
            }
            break;
        case 'v':
        case 'i':
        case 'V':
        case 'I':
            src = *t == 'v' || *t == 'i' ? p->p : p;
            for (k = 0; k < n; k++) {
                ehf_arg_store(&dst[k], src[k], *t == 'v' || *t == 'V', is_float);
            }
            p += *t == 'v' || *t == 'i' ? 1 : n;
            t++;
            break;
        default:
            goto done;
        }
    }
done:
    state[1] = t - types;
    state[2] = p - start;
    return state[0];
}
#endif
