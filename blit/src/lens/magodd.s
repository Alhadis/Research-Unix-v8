data	1
comm	Jdisplay,4
comm	Drect,8
text
L%34:
stabs	"magodd.c",0x15,0,L%34
stabs	"magbyodd",0x12,8,magbyodd
global	magbyodd
magbyodd:
link	%fp,&F%1
movm.l	&M%1,S%1(%fp)
mov.l	8(%fp),%a4
mov.l	12(%fp),%a2
mov.w	16(%fp),%d2
mov.l	24(%fp),%a3
mov.w	30(%fp),%d3
mov.l	%a3,%a0
add.w	28(%fp),%a0
mov.l	%a0,-4+S%1(%fp)
mov.w	%d3,%d0
sub.w	&1,%d0
mov.w	%d0,%d6
asr.w	&1,%d6
add.w	%d3,%a3
mov.w	22(%fp),%d0
sub.w	&1,%d0
mov.w	%d0,-6+S%1(%fp)
sub.w	&1,20(%fp)
mov.w	%d3,%d0
sub.w	28(%fp),%d0
mov.w	%d0,-8+S%1(%fp)
mov.b	29(%fp),%d0
and.b	&1,%d0
mov.b	%d0,%d7
sub.w	&1,28(%fp)
asr.w	&1,28(%fp)
sub.w	-8+S%1(%fp),%a2
mov.w	-8+S%1(%fp),%d0
sub.w	%d0,18(%fp)
L%38:
clr.l	%d1
mov.b	-(%a4),%d1
mulu.w	%d3,%d1
mov.l	-4+S%1(%fp),%a0
lea.l	0(%a0,%d1.l),%a5
mov.w	28(%fp),%d4
tst.b	%d7
bne.b	L%40

L%43:
mov.b	-(%a5),-(%a2)
L%40:
mov.b	-(%a5),-(%a2)
L%42:
dbf	%d4,L%43
L%41:
mov.w	20(%fp),%d4
br	L%44
L%20001:clr.l	%d1
mov.b	-(%a4),%d1
mulu.w	%d3,%d1
lea.l	0(%a3,%d1.l),%a5
mov.w	%d6,%d5
br.b	L%46
L%49:
mov.b	-(%a5),-(%a2)
L%46:
mov.b	-(%a5),-(%a2)
L%48:
dbf	%d5,L%49
L%47:
L%44:
dbf	%d4,L%20001

add.w	%d2,%a4
add.w	18(%fp),%a2
L%37:
sub.w	&1,-6+S%1(%fp)
cmp.w	-6+S%1(%fp),&-1
bne	L%38
L%36:
stabs	"i",0x5,04,6
stabs	"j",0x4,04,4
stabs	"k",0x4,04,5
stabs	"to",0x4,042,10
stabs	"bitsourc",0x4,042,13
stabs	"excess",0x5,04,8
stabs	"odd",0x4,02,7
stabs	"fjump",0x4,04,2
stabs	"bats",0x5,042,4
stabs	"bits",0x4,042,11
stabs	"from",0x4,054,12
stabs	"wcnt",0x4,04,6
stabs	"size",0x4,04,3
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
stabs	"size",0x6,04,30
stabs	"magbyodd",0x19,51,L%35
set	S%1,-40
set	T%1,-48
set	F%1,-52
set	M%1,0x3cfc
data	1
