	data	1
	comm	mouse,2
	text
L%13:
	stabs	"/usr/dmr",0x15,0,L%13
	stabs	"/mc2bug.",0x15,0,L%13
	stabs	"c",0x15,0,L%13
	stabs	"whichbut",0x12,5,whichbut
	global	whichbut
whichbut:
	link	%fp,&F%1
	movm.l	&M%1,S%1(%fp)
	data	1
	even
L%15:
	short	0
	short	3
	short	2
	short	2
	short	1
	short	1
	short	2
	short	2
	text
	mov.w	mouse,%d0
	and.w	&7,%d0
	ext.l	%d0
	add.l	%d0,%d0
	mov.l	&L%15,%a1
	mov.w	0(%a1,%d0.l),%d0
	br	L%14
	stabs	"which",0x2,0144,L%15
L%14:
	movm.l	S%1(%fp),&M%1
	unlk	%fp
	rts
	stabs	"whichbut",0x19,8,L%14
	set	S%1,0
	set	T%1,0
	set	F%1,-4
	set	M%1,0x0000
	data	1
