/*
 * Prototype toggle in bootstrap code for ht type tapes.
 * If on anything but a 780 with a tape at slave 1 of mba 1
 * this will have to be repaired by patching mba and ht.
 */
	movl	mba,r10
	mull3	ht,$0x80,r11
	addl3	r11,r10,r11
	addl2	$0x400,r11
	movl	$1,4(r10)
	movl	$9,(r11)
	cvtwl	$012300,0x24(r11)
	clrl	12(r10)
	movl	$0x80000000,0x800(r10)
	cvtwl	$-512,16(r10)
	movl	$0x39,(r11)
	halt
	.align	2
mba:	.long	0x20012000
ht:	.long	0
