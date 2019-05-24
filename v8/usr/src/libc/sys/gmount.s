# C library -- gmount

# error = gmount(dev, dir [,more])

	.set	gmount,49
.globl	_gmount
.globl  cerror

_gmount:
	.word	0x0000
	chmk	$gmount
	bcc 	noerror
	jmp 	cerror
noerror:
	clrl	r0
	ret
