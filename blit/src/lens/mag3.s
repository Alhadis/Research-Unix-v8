data	1
comm	Jdisplay,4
comm	Drect,8
text
L%34:
stabs	"mag3.c",0x15,0,L%34
stabs	"magby3",0x12,8,magby3
global	magby3
magby3:
link	%fp,&F%1
movm.l	&M%1,S%1(%fp)
mov.l	8(%fp),%a4
mov.l	12(%fp),%a2
mov.w	16(%fp),%d2
mov.w	18(%fp),%d3
mov.w	20(%fp),%d4
mov.l	24(%fp),%a3
mov.w	28(%fp),%d5
lea.l	0(%a3,%d5.w),%a0
mov.l	%a0,-4+S%1(%fp)
add.l	&3,%a3
mov.w	22(%fp),%d0
sub.w	&1,%d0
mov.w	%d0,%d6
sub.w	&1,%d4
mov.l	&3,%d0
sub.w	%d5,%d0
mov.w	%d0,-6+S%1(%fp)
sub.w	&1,%d5
sub.w	%d0,%a2
sub.w	%d0,%d3
L%38:
clr.w	%d0
clr.l	%d1
mov.b	-(%a4),%d0
mov.w	%d0,%d1
add.w	%d0,%d0
add.w	%d0,%d1
mov.l	-4+S%1(%fp),%a0
lea.l	0(%a0,%d1.l),%a5
mov.w	%d5,%d7
mov.b	-(%a5),-(%a2)
sub.w	&1,%d7
bmi.b	L%39
mov.b	-(%a5),-(%a2)
sub.w	&1,%d7
bmi.b	L%40
mov.b	-(%a5),-(%a2)
L%40:
L%39:
mov.w	%d4,%d7
br.b	L%41
L%20001:clr.w	%d0
clr.l	%d1
mov.b	-(%a4),%d0
mov.w	%d0,%d1
add.w	%d0,%d0
add.w	%d0,%d1
lea.l	0(%a3,%d1.l),%a5
mov.b	-(%a5),-(%a2)
mov.b	-(%a5),-(%a2)
mov.b	-(%a5),-(%a2)
L%41:
dbf	%d7,L%20001

add.w	%d2,%a4
add.w	%d3,%a2
L%37:
dbf	%d6,L%38
L%36:
stabs	"i",0x4,04,6
stabs	"j",0x4,04,7
stabs	"to",0x4,042,10
stabs	"bitsourc",0x4,042,13
stabs	"hcount",0x4,04,4
stabs	"excess",0x5,04,6
stabs	"fjump",0x4,04,2
stabs	"valid",0x4,04,5
stabs	"tjump",0x4,04,3
stabs	"bats",0x5,042,4
stabs	"bits",0x4,042,11
stabs	"from",0x4,054,12
L%35:
movm.l	S%1(%fp),&M%1
unlk	%fp
rts
stabs	"from",0x6,054,8
stabs	"to",0x6,042,12
stabs	"fjump",0x6,04,16
stabs	"tjump",0x6,04,18
stabs	"hcount",0x6,04,20
stabs	"vcount",0x6,04,22
stabs	"bits",0x6,042,24
stabs	"valid",0x6,04,28
stabs	"magby3",0x19,40,L%35
set	S%1,-40
set	T%1,-46
set	F%1,-50
set	M%1,0x3cfc
data	1
