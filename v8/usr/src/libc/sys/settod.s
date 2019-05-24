
# C library -- settod

# oldtod =  settod(newtod);

	.set	settod,64+42
.globl	_settod

_settod:
	.word	0x0000
	chmk	$settod
	bcc 	noerror
	jmp 	cerror
noerror:
	ret
