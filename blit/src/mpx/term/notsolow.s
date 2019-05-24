	bra.b	_start
global	Drect
global	mouse
global	Jdisplayp
Drect:	short 0; short 0; short 0; short 0
mouse:	short 0; short 0; short 0; short 0; short 0
Jdisplayp:	long 0
argv:	long 0	# not global
argc:	short 0	# not global
_start:
	mov.l	17 * 4 + 0406, %a0
	jsr	(%a0)
	mov.l	argv, -(%sp)
	mov.w	argc, -(%sp)
	jsr	main
	mov.l	14 * 4 + 0406, %a0
	jsr	(%a0)
