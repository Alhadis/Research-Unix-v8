# C library-- nap

# error = nap(nticks)

	.set	nap,64+41
.globl	_nap
.globl  cerror

_nap:
	.word	0x0000
	chmk	$nap
	bcc 	noerror
	jmp 	cerror
noerror:
	clrl	r0
	ret
