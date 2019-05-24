	data	1
	comm	Jdisplay,4
	comm	Drect,8
	text
L%34:
	stabs	"x.c",0x15,0,L%34
	stabs	"main",0x12,2,main
	global	main
main:
	link	%fp,&F%1
	movm.l	&M%1,S%1(%fp)
	mov.l	262,%a0
	add.l	&2308,%a0
	mov.l	%a0,(%sp)
	jsr	foo
L%35:
	movm.l	S%1(%fp),&M%1
	unlk	%fp
	rts
	stabs	"main",0x19,4,L%35
	set	S%1,0
	set	T%1,0
	set	F%1,-4
	set	M%1,0x0000
	data	1
