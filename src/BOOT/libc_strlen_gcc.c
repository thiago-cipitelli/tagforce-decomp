// @cc gcc -nopeep
/* strlen of the Sony SDK libc: newlib 1.12.0's libc/machine/mips/strlen.c, which for 32-bit MIPS is a top-level asm
   statement (the function itself in assembly), here with its own section, type and size so the link places it like the
   other GCC objects. No function for cc1, so no peephole (-nopeep). Its `@gcc` range is in libc_string.c, whose plain C
   the PC port uses. */

#ifdef __mips__
__asm__ (
	"	.section .text.strlen,\"ax\",@progbits\n"
	"	.set	noreorder\n"
	"	.set	nomacro\n"
	"	.globl	strlen\n"
	"	.ent	strlen\n"
	"	.type	strlen,@function\n"
	"strlen:\n"
	"	addiu	$2,$4,1\n"
	"\n"
	"1:	lbu	$3,0($4)\n"
	"	bnez	$3,1b\n"
	"	addiu	$4,$4,1\n"
	"\n"
	"	jr	$31\n"
	"	subu	$2,$4,$2\n"
	"	.end	strlen\n"
	"	.size	strlen,.-strlen\n"
	"	.set	macro\n"
	"	.set	reorder\n"
	"	.previous\n");
#endif
