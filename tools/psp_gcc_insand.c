/* psp_gcc_insand.c: LD_PRELOAD hook for the cc1 of psp-gcc 1.7.1 (i386 Linux) that gives it the field-clearing
   AND of the later psp-gcc revision the game's SDK was built with (docs/build.md, "Trilha GCC", rodada 37).

   That revision recognizes `x & M`, M = ones with a single run of zeros and not an andi immediate, as one insn
   `ins x,$0,pos,n` whose source is tied to its destination ("0" constraint): combine forms it (from li+and, from
   (x >> k) << k, ...), the register allocator sees one insn and reload adds `move rd,rs` when rs stays live.
   cc1 1.7.1 has no such pattern (it emits ext / li+and / sra+sll). This hook adds it without rebuilding cc1:
   recog() is redirected to my_recog(), which returns the code of a donor insn for that RTL, and the donor's
   insn_data entry (rotrdi3: TARGET_64BIT only, never used here, same operand layout set/op/op) gets the new
   operands, constraints and output.  Only active with -mext.

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

static void writable(void *a, size_t n)
{
    long pg = sysconf(_SC_PAGESIZE);
    uintptr_t s = (uintptr_t)a & ~(pg - 1), e = ((uintptr_t)a + n + pg - 1) & ~(pg - 1);

    if (mprotect((void *)s, e - s, PROT_READ | PROT_WRITE | PROT_EXEC)) {
        perror("psp_gcc_insand: mprotect");
        exit(1);
    }
}

__attribute__((constructor)) static void init(void)
{
    /* recog starts with push %ebp; mov %esp,%ebp; push %edi; push %esi; push %ebx: 6 bytes that move as they are */
    static const unsigned char prologue[6] = { 0x55, 0x89, 0xE5, 0x57, 0x56, 0x53 };
    unsigned char *r = A(ADDR_recog), *t;

    if (memcmp(r, prologue, 6) || strcmp(insn_data[DONOR].name, "rotrdi3")) {
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
    t = mmap(0, 16, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    memcpy(t, r, 6);                       /* trampoline: the prologue, then jmp recog+6 */
    t[6] = 0xE9;
    *(int32_t *)(t + 7) = (int32_t)((uintptr_t)(r + 6) - (uintptr_t)(t + 11));
    recog_orig = (void *)t;
    writable(r, 5);
    r[0] = 0xE9;                           /* recog: jmp my_recog */
    *(int32_t *)(r + 1) = (int32_t)((uintptr_t)my_recog - (uintptr_t)(r + 5));
}
