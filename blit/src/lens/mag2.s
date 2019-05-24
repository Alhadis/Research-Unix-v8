data	1
comm	Jdisplay,4
comm	Drect,8
text
L%34:
stabs	"mag2.c",0x15,0,L%34
stabs	"magby2",0x12,8,magby2
global	magby2
magby2:
link	%fp,&F%1
movm.l	&M%1,S%1(%fp)
mov.l	8(%fp),%a4
mov.l	12(%fp),%a2
mov.w	16(%fp),%d2
mov.w	18(%fp),%d3
mov.w	20(%fp),%d4
mov.l	24(%fp),%a3
mov.w	22(%fp),%d5
sub.w	&1,%d5
sub.w	&1,%d4
lsr.w	&1,%d4
scc	%d7
clr.l	%d0
L%38:
mov.w	%d4,%d6
tst.b	%d7
bne.b	L%40

L%43:
clr.w	%d0
mov.b	-(%a4),%d0
add.w	%d0,%d0
mov.w	0(%a3,%d0.w),-(%a2)
L%40:
clr.w	%d0
mov.b	-(%a4),%d0
add.w	%d0,%d0
mov.w	0(%a3,%d0.w),-(%a2)
L%42:
dbf	%d6,L%43
L%41:
add.w	%d2,%a4
add.w	%d3,%a2
L%37:
dbf	%d5,L%38
L%36:
stabs	"i",0x4,04,5
stabs	"j",0x4,04,6
stabs	"to",0x4,044,10
stabs	"hcount",0x4,04,4
stabs	"odd",0x4,04,7
stabs	"fjump",0x4,04,2
stabs	"tjump",0x4,04,3
stabs	"bits",0x4,044,11
stabs	"from",0x4,054,12
L%35:
movm.l	S%1(%fp),&M%1
unlk	%fp
rts
stabs	"from",0x6,054,8
stabs	"to",0x6,044,12
stabs	"fjump",0x6,04,16
stabs	"tjump",0x6,04,18
stabs	"hcount",0x6,04,20
stabs	"vcount",0x6,04,22
stabs	"bits",0x6,044,24
stabs	"magby2",0x19,26,L%35
set	S%1,-36
set	T%1,-36
set	F%1,-40
set	M%1,0x1cfc
data	1
