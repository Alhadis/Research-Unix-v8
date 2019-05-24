LL0:
	.data
	.comm	_devsw,0
	.comm	_b,16384
	.comm	_blknos,16
	.comm	_iob,16864
	.comm	_cpu,4
	.comm	_mbaddr,4
	.comm	_mbaact,4
	.comm	_umaddr,4
	.comm	_ubaddr,4
	.data
	.align	1
	.globl	_tmstd
_tmstd:
	.long	0xf550
	.text
	.align	1
	.globl	_tmopen
_tmopen:
	.word	L23
	jbr 	L25
L26:
	movl	4(ap),r11
	pushl	$14
	pushl	r11
	calls	$2,_tmstrategy
	movl	96(r11),r10
L28:
	movl	r10,r0
	decl	r10
	tstl	r0
	jeql	L29
	clrl	116(r11)
	pushl	$8
	pushl	r11
	calls	$2,_tmstrategy
	jbr 	L28
L29:
	ret
	.set	L23,0xc00
L25:
	jbr 	L26
	.data
	.text
	.align	1
	.globl	_tmclose
_tmclose:
	.word	L31
	jbr 	L33
L34:
	movl	4(ap),r11
	pushl	$14
	pushl	r11
	calls	$2,_tmstrategy
	ret
	.set	L31,0x800
L33:
	jbr 	L34
	.data
	.text
	.align	1
	.globl	_tmstrategy
_tmstrategy:
	.word	L35
	jbr 	L37
L38:
	movl	4(ap),r11
	ashl	$-3,92(r11),r0
	ashl	$2,r0,r0
	addl3	r0,_umaddr,r0
	movzwl	_tmstd,r1
	bicl2	$-8192,r1
	addl3	r1,(r0),r0
	movl	r0,r7
	movl	92(r11),r9
	clrl	r8
L39:
	pushl	r7
	calls	$1,_tmquiet
	ashl	$8,r9,r0
	movl	r0,r10
	pushl	$1
	pushl	r11
	ca