/*
 * unpack comet microcode and patch bits:
 * it's too much trouble to write this in C
 *
 * unpack(packed, unpacked, nwords, nbits)
 *
 * copies nwords nbits-bit things from packed
 * into nwords 32-bit things in unpacked
 */

	.globl	_unpack
_unpack:
	.word	0
	movl	8(ap),r0	# to
	clrl	r1		# bit offset
	movl	12(ap),r2	# word count
	bneq	0f
	 ret
0:	movl	16(ap),r3	# bit size
	bneq	1f
	 ret
1:
	extzv	r1,r3,*4(ap),(r0)+
	addl2	r3,r1
	sobgtr	r2,1b
	ret
