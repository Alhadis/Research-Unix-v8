#
#	%a2		pointer to text
#	%a3		pointer to control header
#
global	spl0
global	spl1
global	spl4
global	spl5
global	spl7
global	splx
	set	M, 0	# memory requirements
	set	S, 1	# seek
	set	D, 2	# data
	set	E, 3	# end of message

	set	CNTL, 156*1024-8	# Control header
	set	JUNK, CNTL+4		# a longword

text
long	CNTL
long	reboot
data
jmptab:
	long	caseM
	long	caseS
	long	caseD
	long	caseE
text
	jmp	START	# 010 is a known place
	# The display bitmap in rom at last!
display:
	long	156*1024	# base
	short	50		# width
	short	0
	short	0
	short	800
	short	1024		# rect
	long	0		# _null
	# graphics in rom at last!
	long	addr
	long	alloc
	long	gcalloc
	long	add
	long	sub
	long	inset
	long	div
	long	mul
	long	rsubp
	long	raddp
	long	eqpt
	long	eqrect
	long	balloc
	long	bitblt
	long	texture
	long	rectf
	long	jrectf
	long	segment
	long	jmoveto
	long	jlineto
	long	jmove
	long	jline
	long	jsegment
	long	point
	long	menuhit
	long	ptinrect
	long	rectXrect
	long	rectclip
	long	screenswap
	long	string
	long	strwidth
	long	layerop
	long	newlayer
	long	dellayer
	long	lbitblt
	long	lrectf
	long	ltexture
	long	lsegment
	long	lblt
	long	upfront
global box
global	jgrey
box: jgrey: reboot:
	mov.l	&0, %a0
	mov.l	&64*1024, %d0
clear:
	clr.l	(%a0)+
	sub.l	&1, %d0
	bne.b	clear

	mov.l	&t%0,0x60	# level 0 autovector
	mov.l	&t%1,0x64	# level 1 autovector
	mov.l	&t%2,0x68	# level 2 autovector
	mov.l	&t%3,0x6c	# level 3 autovector
	mov.l	&t%4,0x70	# level 4 autovector
	mov.l	&t%5,0x74	# level 5 autovector
	jmp	main
exit:	bra.b	exit
t%0:
	rte
t%1:
	movm.l	&0xC0C0,-(%sp)
	mov.w	&0, 384*1024+070	# turn the interrupt off
	jsr	auto1
	movm.l	(%sp)+,&0x0303
	rte
t%2:
	movm.l	&0xC0C0,-(%sp)
	jsr	auto2
	movm.l	(%sp)+,&0x0303
	rte
t%3:
	movm.l	&0xC0C0,-(%sp)
	jsr	auto3
	movm.l	(%sp)+,&0x0303
	rte
t%4:
	movm.l	&0xC0C0,-(%sp)
	jsr	auto4
	movm.l	(%sp)+,&0x0303
	rte
	set	PIO2,	(384*1024+013)	# ACIA data reg
	set	STAT2,	(384*1024+011)	# ACIA status reg
	set	RDRFBIT,	0	# bit num of RDRF in STAT2
t%5:
	movm.l	&0xC0C0,-(%sp)
	mov.b	STAT2, %d0
	btst	&RDRFBIT, %d0
	bne	t%5a
	jsr	aciatrint
	bra	t%5b
t%5a:
	mov.w	&0, %d0
	mov.b	PIO2, %d0
	mov.w	%d0, -(%sp)
	mov.l	&queues, -(%sp)
	jsr	qputc	# qputc(%d0, HOSTQUEUE)
	add.l	&6, %sp
t%5b:
	movm.l	(%sp)+, &0x0303
	rte

spl0:
	mov.w	%sr,%d0
	mov.w	&0x2000, %sr
	rts
spl1:
	mov.w	%sr, %d0
	mov.w	&0x2100, %sr
	rts
spl4:
	mov.w	%sr, %d0
	mov.w	&0x2400, %sr
	rts
spl5:
	mov.w	%sr, %d0
	mov.w	&0x2500, %sr
	rts
spl7:
	mov.w	%sr, %d0
	mov.w	&0x2700, %sr
	rts
splx:
	mov.w	4(%sp), %sr
	rts

START:
#	clear memory
	mov.w	&0, (384*1024)+030		# so we can watch
	mov.w	&0, (384*1024)+040
	mov.l	&64*1024, %d0
	mov.l	&0, %a0
clearloop:
	clr.l	(%a0)+
	sub.l	&1, %d0
	bne.b	clearloop
#
	mov.l	&CNTL, %a3
	mov.l	%a3, %sp
	jsr	aciainit
loop:
	jsr	getheader
	clr.l	%d0
	clr.l	%d1
	mov.b	0(%a3), %d0		# control byte, M etc.
	mov.w	2(%a3), %d1		# length of data
	blt.b	NAK
	cmp.b	%d0, &E
	bgt.b	NAK
	# In range; do the job
	lsl.l	&2, %d0
	mov.l	&jmptab, %a0
	mov.l	0(%a0,%d0.l),%a0
	jmp	(%a0)

caseM:
	tst.w	%d1
	beq.b	loop
	jsr	getc
	sub.w	&1, %d1
	bra.b	caseM

caseS:
	jsr	getl
	mov.l	%d0, %a2		# memory address
	bra.b	loop

caseD:
	tst.w	%d1	# a little insurance
	blt	NAK
	cmp.w	%d1, &128
	bgt.b	NAK
caseD1:
	tst.l	%d1
	beq.b	loop
	jsr	getc
	mov.b	%d0, (%a2)+
	sub.w	&1, %d1
	bra.b	caseD1

caseE:
	mov.l	&156*1024, %sp
	mov.l	&0x100, %a0
	jmp	(%a0)
#
#	sets for ACIA
#
	set	ACIASTAT, (384*1024)+011
	set	ACIADATA, (384*1024)+013
	set	A_CDIV16, 0x1		# divide speed by 16
	set	A_RESET, 0x3		# reset acia
	set	A_S1NB8, 0x14		# 1 stop bit, no parity, 8 bits
	set	A_RDRF, 0x01		# receive data register full
	set	A_RDRFBIT, 0x00		# bit number of RDRF
	set	A_TDRE, 0x02		# transmitter data buffer empty
	set	A_TDREBIT, 0x01		# bit number of TDRE
#
#	ACIAinit: wiggle reset and make the ACIA happy
#
aciainit:
	mov.b	&A_RESET, ACIASTAT
	mov.b	&A_S1NB8+A_CDIV16, ACIASTAT
	rts
#
#	Putc: pump out a character to the host.
#char in %d0; unchanged
#%d1 trashed
#
putc:
	mov.b	ACIASTAT, %d1
	btst	&A_TDREBIT, %d1
	beq.b	putc
	mov.b	%d0, ACIADATA
	rts
#
#	Getc: receive character from host.
#result is in %d0
#DEBUG: return 0xFF if error bit
getc:
	mov.b	ACIASTAT, %d0
#	btst	&A_RDRFBIT, %d0
	and.b	&0x71, %d0
	beq.b	getc
	and.b	&0x70, %d0		# error?
	beq.b	OK
	mov.b	&0xFF, %d0
	mov.b	ACIADATA, %d1		# fix error?
	rts
OK:
	mov.b	ACIADATA, %d0
	rts

getl:
	mov.l	&JUNK, %a0		# after control header
	mov.l	%a0, %a1		# save it away
	jsr	getc
	mov.b	%d0, (%a0)+
	jsr	getc
	mov.b	%d0, (%a0)+
	jsr	getc
	mov.b	%d0, (%a0)+
	jsr	getc
	mov.b	%d0, (%a0)+
	mov.l	(%a1), %d0
	rts

getheader:
	jsr	getl
	mov.l	%d0, (%a3)
	rts

NAK:
	bra.b	NAK
