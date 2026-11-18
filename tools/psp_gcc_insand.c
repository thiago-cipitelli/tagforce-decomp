/* psp_gcc_insand.c: LD_PRELOAD hook for the cc1 of psp-gcc 1.7.1 (i386 Linux) that gives it the field-clearing
   AND of the later psp-gcc revision the game's SDK was built with (docs/build.md, "Trilha GCC", rodada 37).

   That revision recognizes `x & M`, M = ones with a single run of zeros and not an andi immediate, as one insn
   `ins x,$0,pos,n` whose source is tied to its destination ("0" constraint): combine forms it (from li+and, from
   (x >> k) << k, ...), the register allocator sees one insn and reload adds `move rd,rs` when rs stays live.
   cc1 1.7.1 has no such pattern (it emits ext / li+and / sra+sll). This hook adds it without rebuilding cc1:
   recog() is redirected to my_recog(), which returns the code of a donor insn for that RTL, and the donor's
   insn_data entry (rotrdi3: TARGET_64BIT only, never used here, same operand layout set/op/op) gets the new
   operands, constraints and output.  Only active with -mext.

   Second change (rodada 39): the later revision hoists more loop invariants. loop.c moves an invariant when
   threshold * savings * lifetime >= insns in the loop, threshold = (has_call ? 1 : 2) * (1 + n_non_fixed_regs);
   cc1 1.7.1 has n_non_fixed_regs = 64, the game's compiler behaves as if it were 100..256 (192 = 64 + the 128 VFPU
   registers, if that revision made them allocatable). Hoisted constants that do not get a hard register are
   rematerialized by reload in its round-robin spill register, which is the -1 in a0 instead of v0 of _strtod_r and
   __svfscanf_r. rest_of_compilation() is wrapped to set n_non_fixed_regs (PSP_GCC_NNF=n overrides, 64 = cc1 1.7.1).

   Third change (rodada 41): in store_expr, when the target has a queued post-increment (`*p++ = ...`), the
   increments queued while expanding the value are emitted right after it, before the store (the target's own
   entries stay queued). `*p++ = *fp->_p++` then stores fp->_p before the byte, as __svfscanf_r of the original.
   The call to expand_expr in that branch (after gen_reg_rtx) is redirected to rhs_expand(). PSP_GCC_RHSQ=0 turns it off.

   Fourth change (rodada 42): GCSE's jump bypassing (bypass_block) does not redirect a predecessor edge whose source
   block has several predecessors and holds only the constant set and its jump (`retval = 0; goto test` shared by
   the `return 0`s of an inlined function). ccc_encode_sjis keeps the 0/1 result of the inlined sceCccIsValidJIS in v0
   and tests it, as the original; the bypasses of ccc_encode_utf16, ccc_decode_sjis, _vfprintf_r and __svfscanf_r
   (source blocks with one predecessor, or with more work) stay. The edges are taken out of the block's predecessor
   list for the call and put back in their order. PSP_GCC_BYPASS=0 turns it off.

   Built by tools/setup_toolchain.sh (addresses from nm of cc1 as -DADDR_*), used by tools/psp_gcc.sh.  */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct rtx_def *rtx;
struct rtx_def { unsigned short code; unsigned char mode; unsigned char flags; void *fld[1]; };
#define XEXP(x, i) ((rtx)(x)->fld[i])
#define INTVAL(x) ((int)(intptr_t)(x)->fld[0])
enum { C_CONST_INT = 64, C_SET = 57, C_AND = 93, VOIDmode = 0, SImode = 4 };   /* GCC 3.3 rtl.def/machmode.def */

struct insn_operand_data { int (*pred)(rtx, int); const char *constraint; unsigned short mode; char strict_low, elim; };
struct insn_data { const char *name; const void *output; void *genfun; const struct insn_operand_data *operand;
                   char n_operands, n_dups, n_alternatives, output_format; };

#define A(x) ((void *)(uintptr_t)(x))
static int (*register_operand)(rtx, int) = A(ADDR_register_operand);
static struct insn_data *insn_data = A(ADDR_insn_data);
static volatile int *target_flags = A(ADDR_target_flags);
static int (*recog_orig)(rtx, rtx, int *);
#define MASK_EXT 0x20000000
#define DONOR 219   /* CODE_FOR_rotrdi3 in cc1 1.7.1 */

/* M = ones with a single run of zeros [pos, pos + len), not an andi immediate */
static int ins_mask(rtx op, int *pos, int *len)
{
    unsigned int z;
    int p = 0, l = 0;

    if (op->code != C_CONST_INT || (unsigned int)INTVAL(op) <= 0xFFFF)
        return 0;
    z = ~(unsigned int)INTVAL(op);
    while (!(z & 1)) { z >>= 1; p++; }
    while (z & 1) { z >>= 1; l++; }
    *pos = p; *len = l;
    return z == 0;
}

static const char *ins_output(rtx *operands, rtx insn)
{
    static char buf[32];
    int p, l;

    (void)insn;
    if (!ins_mask(operands[2], &p, &l))
        abort();
    snprintf(buf, sizeof buf, "ins\t%%0,$0,%d,%d", p, l);
    return buf;
}

static int my_recog(rtx x, rtx insn, int *pnum_clobbers)
{
    int p, l;

    if ((*target_flags & MASK_EXT) && x->code == C_SET && x->mode == VOIDmode) {
        rtx src = XEXP(x, 1);
        if (src->code == C_AND && src->mode == SImode && ins_mask(XEXP(src, 1), &p, &l)
            && register_operand(XEXP(x, 0), SImode) && register_operand(XEXP(src, 0), SImode))
            return DONOR;
    }
    return recog_orig(x, insn, pnum_clobbers);
}

static struct insn_operand_data ops[3] = {
    { 0, "=d", SImode, 0, 0 }, { 0, "0", SImode, 0, 0 }, { 0, "", SImode, 0, 0 } };

static int *n_non_fixed_regs = A(ADDR_n_non_fixed_regs);
static void (*roc_orig)(void *);
static int nnf = 192;

static void my_rest_of_compilation(void *decl)
{
    *n_non_fixed_regs = nnf;
    roc_orig(decl);
}

static void *(*expand_expr_f)(void *, void *, int, int) = A(ADDR_expand_expr);
static void (*emit_queue_f)(void) = A(ADDR_emit_queue);
static void ***cfun_p = A(ADDR_cfun);
#define PENDING (*(rtx *)((char *)(*cfun_p)[2] + 24))        /* cfun->expr->x_pending_chain */
#define QNEXT(q) (*(rtx *)((char *)(q) + 4 + 4 * 4))          /* QUEUED_NEXT: XEXP (q, 4) */

static void *rhs_expand(void *exp, void *target, int mode, int modifier)
{
    rtx saved = PENDING, r = expand_expr_f(exp, target, mode, modifier), q;

    if (PENDING != saved) {
        for (q = PENDING; q && QNEXT(q) != saved; q = QNEXT(q))
            ;
        if (q) {                /* only the entries added by the value */
            QNEXT(q) = 0;
            emit_queue_f();
            PENDING = saved;
        } else                  /* the value flushed the older ones itself */
            emit_queue_f();
    }
    return r;
}

static void writable(void *a, size_t n)
{
    long pg = sysconf(_SC_PAGESIZE);
    uintptr_t s = (uintptr_t)a & ~(pg - 1), e = ((uintptr_t)a + n + pg - 1) & ~(pg - 1);

    if (mprotect((void *)s, e - s, PROT_READ | PROT_WRITE | PROT_EXEC)) {
        perror("psp_gcc_insand: mprotect");
        exit(1);
    }
}

/* jump from function R to FN; returns a trampoline that calls the original R. R must start with
   push %ebp; mov %esp,%ebp; push %edi; push %esi; push %ebx: 6 bytes that move as they are */
static void *patch(unsigned char *r, void *fn)
{
    static const unsigned char prologue[6] = { 0x55, 0x89, 0xE5, 0x57, 0x56, 0x53 };
    unsigned char *t;

    if (memcmp(r, prologue, 6)) {
        fprintf(stderr, "psp_gcc_insand: not the cc1 of psp-gcc 1.7.1\n");
        exit(1);
    }
    t = mmap(0, 16, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    memcpy(t, r, 6);                       /* trampoline: the prologue, then jmp r+6 */
    t[6] = 0xE9;
    *(int32_t *)(t + 7) = (int32_t)((uintptr_t)(r + 6) - (uintptr_t)(t + 11));
    writable(r, 5);
    r[0] = 0xE9;                           /* r: jmp fn */
    *(int32_t *)(r + 1) = (int32_t)((uintptr_t)fn - (uintptr_t)(r + 5));
    return t;
}

/* basic_block_def / edge_def of GCC 3.3 (basic-block.h): bb->head 0, end 4, pred 16, index 44; edge->pred_next 0,
   src 8 */
#define BB_HEAD(b) (*(rtx *)(b))
#define BB_END(b) (*(rtx *)((char *)(b) + 4))
#define BB_PRED(b) (*(char **)((char *)(b) + 16))
#define E_PNEXT(e) (*(char **)(e))
#define E_SRC(e) (*(char **)((char *)(e) + 8))
#define NEXT_INSN(i) ((rtx)(i)->fld[2])
enum { C_INSN = 42, C_JUMP_INSN = 43, C_CALL_INSN = 44 };

static int (*bypass_block_orig)(void *, rtx, rtx);

static int set_and_jump_join(char *b)
{
    rtx i;
    int n = 0, np = 0;
    char *e;

    for (e = BB_PRED(b); e; e = E_PNEXT(e))
        np++;
    if (np < 2)
        return 0;
    for (i = BB_HEAD(b); ; i = NEXT_INSN(i)) {
        if (i->code == C_INSN || i->code == C_JUMP_INSN || i->code == C_CALL_INSN)
            n++;
        if (i == BB_END(b))
            break;
    }
    return n <= 2;
}

static int my_bypass_block(void *bb, rtx setcc, rtx jump)
{
    char *all[256], *keep[256], **pp, *e;
    char hidden[256];
    int n = 0, nh = 0, k = 0, i, j, r;

    for (e = BB_PRED(bb); e && n < 256; e = E_PNEXT(e)) {
        hidden[n] = set_and_jump_join(E_SRC(e));
        nh += hidden[n];
        all[n++] = e;
    }
    if (e || !nh)
        return bypass_block_orig(bb, setcc, jump);
    for (pp = &BB_PRED(bb), i = 0; i < n; i++)
        if (!hidden[i]) {
            *pp = all[i];
            pp = &E_PNEXT(all[i]);
        }
    *pp = 0;
    r = bypass_block_orig(bb, setcc, jump);
    /* bypassing only removes edges from this list: put the hidden ones back, everything in the old order */
    for (e = BB_PRED(bb); e; e = E_PNEXT(e))
        keep[k++] = e;
    for (pp = &BB_PRED(bb), i = 0; i < n; i++) {
        int in = hidden[i];

        for (j = 0; !in && j < k; j++)
            in = keep[j] == all[i];
        if (in) {
            *pp = all[i];
            pp = &E_PNEXT(all[i]);
        }
    }
    *pp = 0;
    return r;
}

__attribute__((constructor)) static void init(void)
{
    const char *e = getenv("PSP_GCC_NNF");

    if (e)
        nnf = atoi(e);
    roc_orig = patch(A(ADDR_rest_of_compilation), my_rest_of_compilation);
    if (strcmp(insn_data[DONOR].name, "rotrdi3")) {
        fprintf(stderr, "psp_gcc_insand: not the cc1 of psp-gcc 1.7.1\n");
        exit(1);
    }
    ops[0].pred = register_operand;
    ops[1].pred = register_operand;
    ops[2].pred = A(ADDR_const_int_operand);
    writable(&insn_data[DONOR], sizeof insn_data[DONOR]);
    insn_data[DONOR].name = "andsi3_ins";
    insn_data[DONOR].output = (void *)ins_output;
    insn_data[DONOR].operand = ops;
    insn_data[DONOR].n_operands = 3;
    insn_data[DONOR].n_dups = 0;
    insn_data[DONOR].n_alternatives = 1;
    insn_data[DONOR].output_format = 3;   /* INSN_OUTPUT_FORMAT_FUNCTION */
    recog_orig = patch(A(ADDR_recog), my_recog);
    e = getenv("PSP_GCC_RHSQ");
    if (!e || atoi(e)) {
        /* store_expr, queued_subexp_p (target) branch: call expand_expr (exp, gen_reg_rtx (...), ...) */
        unsigned char *c = A(ADDR_store_expr + 0x4AE);
        if (c[0] != 0xE8 || (uintptr_t)(c + 5) + *(int32_t *)(c + 1) != (uintptr_t)expand_expr_f) {
            fprintf(stderr, "psp_gcc_insand: not the cc1 of psp-gcc 1.7.1 (store_expr)\n");
            exit(1);
        }
        writable(c, 5);
        *(int32_t *)(c + 1) = (int32_t)((uintptr_t)rhs_expand - (uintptr_t)(c + 5));
    }
    e = getenv("PSP_GCC_BYPASS");
    if (!e || atoi(e))
        bypass_block_orig = patch(A(ADDR_bypass_block), my_bypass_block);
}
