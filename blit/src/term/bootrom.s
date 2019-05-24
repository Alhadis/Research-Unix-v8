#
#	ROM startup code
#
global	spl0
global	spl1
global	spl4
global	spl5
global	spl7
global	splx
global	getc
global	putc
text
#IF	ROM
# Reset vector
	long	156*1024-4
	long	reboot
#ENDIF
##IF	RAM
#	jmp	reboot
##ENDIF
	jmp	START	# 010 is a known place
#	Version string pointed to by long word
long	version
#	The display bitmap in rom at last!
global	display
display:
	long	156*1024	# base
	short	50		# width
	short	0
	short	0
	short	800
	short	1024		# rect
	long	0		# _null
	# graphics in rom at last!
	long	add
	long	addr
	long	alloc
	long	balloc
	long	bitblt
	long	dellayer
	long	div
	long	eqpt
	long	eqrect
	long	gcalloc
	long	inset
	long	jline
	long	jlineto
	long	jmove
	long	jmoveto
	long	jrectf
	long	jsegment
	long	layerop
	long	lbitblt
	long	lblt
	long	Lbox
	long	Lgrey
	long	lpoint
	long	lrectf
	long	lsegment
	long	ltexture
	long	menuhit
	long	mul
	long	newlayer
	long	point
	long	ptinrect
	long	raddp
	long	rectXrect
	long	rectclip
	long	rectf
	long	rsubp
	long	screenswap
	long	segment
	long	string
	long	strwidth
	long	sub
	long	texture
	long	upfront
	long	allocinit
reboot:
#IF	ROM
	mov.l	&8, %a0
	mov.l	&64*1024-2, %d0
clear:
	clr.l	(%a0)+
	sub.l	&1, %d0
	bne.b	clear
#ENDIF
setvectors:
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
	jsr	introutine
	movm.l	(%sp)+,&0x0303
	rte
t%2:
	movm.l	&0xC0C0,-(%sp)
	jsr	auto2
	movm.l	(%sp)+,&0x0303
	rte
t%3:
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
	bne.b	t%5a
	jsr	aciatrint
	bra.b	t%5b
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
	mov.l	&64*1024-2, %d0
	mov.l	&8, %a0
clearloop:
	clr.l	(%a0)+
	sub.l	&1, %d0
	bne.b	clearloop
#
	mov.l	&156*1024-4, %a3
	mov.l	%a3, %sp
	jsr	aciainit
	jmp	load
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
#	mov.b	&A_S1NB8+A_CDIV16, ACIASTAT
# above line temporarily patched out for 1x clocking below

	mov.b	&A_S1NB8, ACIASTAT	#1x clock is 0 field
	rts
#
#	Putc: pump out a character to the host.
#char in %d0; unchanged
#%d1 trashed
#
putcx:
putc:
	mov.b	ACIASTAT, %d1
	btst	&A_TDREBIT, %d1		# ready for next byte?
	beq.b	putcx			# no; loop
	mov.b	%d0, ACIADATA		# transmit
	rts
#
#	Getc: receive character from host.
#result is in %d0
#return -1 if error bit
getcx:
getc:
	mov.l	&0, %d0			# clear initially
	mov.b	ACIASTAT, %d0
	and.b	&0x71, %d0		# error or data?
	beq.b	getcx			# neither; loop
	and.b	&0x70, %d0		# error?
	beq.b	getcOK
	mov.b	ACIADATA, %d0		# read data to reset
	mov.w	&-1, %d0		# return -1 for error
	rts
getcOK:
	mov.b	ACIADATA, %d0
	rts
version:
	byte	'r,'o,'b,' ,'S,'e,'p,' ,'8,'2,0

