// @range 0x08820ED4-0x08822F84
/* EHF typed command arguments: converts the argument words of a command into the caller's outputs.
   The first argument word (at index `first`) points to a type string with one letter per source
   value; the following words are the values. `fmt` has one letter per output (`out[k]`):

     float  int   components  scalar sources written through
     G      O     1           an array of component pointers
     H      P     2
     I      Q     3
     F      N     4
     f      n     1           indexing the output
     g      o     2
     h      p     3
     i      q     4
     r            1 word copied raw

   Source letters: F float, N int (immediate words, one per component, each with its own type
   letter), f float *, n int * (pointer to one value, one word and letter per component); V float
   vector, I int vector (immediate, one word per component, one type letter), v float *, i int *
   (pointer to the vector, one word, one letter); an immediate vector into an N-component output
   takes N words. Ints and floats convert with (int)/(float); a scalar component with an unknown
   letter is skipped.
   An unknown destination letter, or a source letter that starts no scalar or vector, stops the
   parse. `state` (may be 0) receives the number
   of outputs converted, type letters and value words consumed; the return is the output count. */
#include "libc.h"
#include "ehfcmd.h"

/* One component from a scalar source into lvalue D of an int / float output. */
#define SCALAR_INT(D)                         \
    switch (*t) {                             \
    case 'n':                                 \
        D = *(int *)s->p;                     \
        s++;                                  \
        break;                                \
    case 'f':                                 \
        D = (int)*(float *)s->p;              \
        s++;                                  \
        break;                                \
    case 'N':                                 \
        D = s->i;                             \
        s++;                                  \
        break;                                \
    case 'F':                                 \
        D = (int)s->f;                        \
        s++;                                  \
        break;                                \
    }
#define SCALAR_FLOAT(D)                       \
    switch (*t) {                             \
    case 'n':                                 \
        D = (float)*(int *)s->p;              \
        s++;                                  \
        break;                                \
    case 'f':                                 \
        D = *(float *)s->p;                   \
        s++;                                  \
        break;                                \
    case 'N':                                 \
        D = (float)s->i;                      \
        s++;                                  \
        break;                                \
    case 'F':                                 \
        D = s->f;                             \
        s++;                                  \
        break;                                \
    }

/* Component K of a vector source into output component K. */
#define VI_I(K) ((int *)*out)[K] = s[K].i
#define VI_V(K) ((int *)*out)[K] = (int)s[K].f
#define VI_i(K) ((int *)*out)[K] = ((int *)s->p)[K]
#define VI_v(K) ((int *)*out)[K] = (int)((float *)s->p)[K]
#define VF_I(K) ((float *)*out)[K] = (float)s[K].i
#define VF_V(K) ((float *)*out)[K] = s[K].f
#define VF_i(K) ((float *)*out)[K] = (float)((int *)s->p)[K]
#define VF_v(K) ((float *)*out)[K] = ((float *)s->p)[K]

#define REP1(M) M(0)
#define REP2(M) M(0); M(1)
#define REP3(M) M(0); M(1); M(2)
#define REP4(M) M(0); M(1); M(2); M(3)

/* A whole vector source: immediate vectors take N words, pointers one. */
#define VECTOR(T, N)                          \
    switch (*t) {                             \
    case 'I':                                 \
        REP##N(V##T##_I);                     \
        s += N;                               \
        break;                                \
    case 'V':                                 \
        REP##N(V##T##_V);                     \
        s += N;                               \
        break;                                \
    case 'i':                                 \
        REP##N(V##T##_i);                     \
        s++;                                  \
        break;                                \
    case 'v':                                 \
        REP##N(V##T##_v);                     \
        s++;                                  \
        break;                                \
    default:                                  \
        goto done;                            \
    }                                         \
    t++

/* Lower-case destinations: the output components are indexed. */
#define DEST_INDEXED(SCALAR, T, CT, N)                         \
    switch (*t) {                                              \
    case 'I':                                                  \
    case 'V':                                                  \
    case 'i':                                                  \
    case 'v':                                                  \
        VECTOR(T, N);                                          \
        break;                                                 \
    case 'n':                                                  \
    case 'f':                                                  \
    case 'N':                                                  \
    case 'F':                                                  \
        for (i = 0; i < N; t++, i++) {                         \
            SCALAR(((CT *)*out)[i]);                           \
        }                                                      \
        break;                                                 \
    default:                                                   \
        goto done;                                             \
    }                                                          \
    break

/* Upper-case destinations: scalar sources go through an array of component pointers. */
#define DEST_PTRS(SCALAR, T, CT, N)                            \
    switch (*t) {                                              \
    case 'I':                                                  \
    case 'V':                                                  \
    case 'i':                                                  \
    case 'v':                                                  \
        VECTOR(T, N);                                          \
        break;                                                 \
    case 'n':                                                  \
    case 'f':                                                  \
    case 'N':                                                  \
    case 'F': {                                                \
        CT *d[4] = { (CT *)*out, (CT *)*out + 1, (CT *)*out + 2, (CT *)*out + 3 }; \
                                                               \
        for (i = 0; i < N; t++, i++) {                         \
            SCALAR(*d[i]);                                     \
        }                                                      \
        break;                                                 \
    }                                                          \
    default:                                                   \
        goto done;                                             \
    }                                                          \
    break

int ehf_cmd_args(EhfArgState *state, EhfCmd *cmd, int first, void **out, char *fmt)
{
    EhfArgState tmp;
    EhfArg *a;
    EhfArg *args;
    char *types;
    EhfArg *start;
    EhfArg *end;
    char *t;
    EhfArg *s;
    int i;

    if (state == 0) {
        state = &tmp;
    }
    memset(state, 0, sizeof(EhfArgState));
    a = (EhfArg *)&cmd->a;
    args = a + first;
    types = args[0].p;
    start = args + 1;
    end = a + cmd->argc;
    t = types;
    for (s = start; s < end; fmt++, out++, state->n++) {
        if (*fmt == 0) {
            break;
        }
        switch (*fmt) {
        case 'n':
            DEST_INDEXED(SCALAR_INT, I, int, 1);
        case 'o':
            DEST_INDEXED(SCALAR_INT, I, int, 2);
        case 'p':
            DEST_INDEXED(SCALAR_INT, I, int, 3);
        case 'q':
            DEST_INDEXED(SCALAR_INT, I, int, 4);
        case 'f':
            DEST_INDEXED(SCALAR_FLOAT, F, float, 1);
        case 'g':
            DEST_INDEXED(SCALAR_FLOAT, F, float, 2);
        case 'h':
            DEST_INDEXED(SCALAR_FLOAT, F, float, 3);
        case 'i':
            DEST_INDEXED(SCALAR_FLOAT, F, float, 4);
        case 'F':
            DEST_PTRS(SCALAR_FLOAT, F, float, 4);
        case 'G':
            DEST_PTRS(SCALAR_FLOAT, F, float, 1);
        case 'H':
            DEST_PTRS(SCALAR_FLOAT, F, float, 2);
        case 'I':
            DEST_PTRS(SCALAR_FLOAT, F, float, 3);
        case 'N':
            DEST_PTRS(SCALAR_INT, I, int, 4);
        case 'O':
            DEST_PTRS(SCALAR_INT, I, int, 1);
        case 'P':
            DEST_PTRS(SCALAR_INT, I, int, 2);
        case 'Q':
            DEST_PTRS(SCALAR_INT, I, int, 3);
        case 'r':
            *(int *)*out = s->i;
            t++;
            s++;
            break;
        default:
            goto done;
        }
    }
done:
    state->ntypes = t - types;
    state->nwords = s - start;
    return state->n;
}
