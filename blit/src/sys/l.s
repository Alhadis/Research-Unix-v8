	text
global	spl0
global	spl1
global	spl4
global	spl5
global	spl7
global	splx
	mov.l	&t%0,0x60	# level 0 autovector
	mov.l	&t%1,0x64	# level 1 autovector
	mov.l	&t%2,0x68	# level 2 autovector
	mov.l	&t%3,0x6c	# level 3 autovector
	mov.l	&t%4,0x70	# level 4 autovector
	mov.l	&t%5,0x74	# level 5 autovector
	jsr	jinit
	jsr	main
	jmp	reboot
t%0:
	rte
t%1:
	movm.l	&0xC0C0,-(%sp)
	mov.w	&0, 384*1024+070	# turn the interrupt off
	jsr	auto1
	mov.l	mouse, mouse+4		# BUG FIX FOR OPTIMIZED CURSOR.C IN ROM!!
	add.l	&1,ticks0
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
	bne	t%5a
	jsr	aciatrint
	bra	t%5b
t%5a:
	mov.w	&0, %d0
	mov.b	PIO2, %d0
	mov.w	%d0, -(%sp)
	mov.l	&queues, -(%sp)
	jsr	qputc	# qputc(%d0, &RCVQUEUE)
	add.l	&6, %sp
t%5b:
	movm.l	(%sp)+, &0x0303
	rte

#	set	ACIASTAT, (384*1024)+011
#	set	ACIADATA, (384*1024)+013
#	set	A_TDREBIT, 0x01		# bit number of TDRE in ACIA
#	data
#	global	dmabuf
#	global	dmainp
#	global	dmaoutp
#	global	ndmachars
#	global	dmacount
#dmabuf:	space	1024
#dmainp:	long	dmabuf
#dmaoutp:long	dmabuf
#ndmachars:
#	short	0
#dmacount:
#	short	1024
#frozen:	short	0
#	text
#t%5:
#	movm.l	&0x8080, -(%sp)		# save %a0, %d0
#	mov.l	dmainp, %a0
#	mov.b	ACIADATA, (%a0)+	# read byte, clear interrupt
#	mov.l	%a0, dmainp
#	sub.w	&1, dmacount		# number of bytes left to read
#	bgt	t1%5
#	bsr	dmadone
#t1%5:
#	add.w	&1, ndmachars		# number of chars in dma buffer
#	cmp.w	ndmachars, &400
#	blt	t2%5			# no need to freeze queue
#	tst.w	frozen			# already frozen?
#	bne	t2%5			# yup
#	bsr	freeze			# nope
#t2%5:
#	or.w	&1, proctab+8		# setrun(&demuxp)
#	movm.l	(%sp)+, &0x0101
#	rte
#freeze:
#	mov.w	&1, frozen
#	mov.w	&0x2000, %sr		# spl0() for the loop
#freeze1:
#	mov.b	ACIASTAT, %d0
#	btst	&A_TDREBIT, %d0
#	beq	freeze1
#	mov.b	&0x13, ACIADATA		# send ^S
#	rts
#thaw:
#	mov.w	&0, frozen
#	mov.b	ACIASTAT, %d1
#	btst	&A_TDREBIT, %d1
#	beq	thaw
#	mov.b	&0x11, ACIADATA		# send ^Q
#	rts
#dmadone:
#	# %a0 already contains dmainp
#	cmp.l	%a0, &dmabuf+1024
#	blo	dmadon1
#	mov.l	&dmabuf, %a0		# wrap to beginning of buffer
#	mov.l	%a0, dmainp
#	bra	dmadon2
#dmadon1:
#	cmp.l	%a0, dmaoutp
#	blo	dmadon2
#	# set count to # chars to end of buffer; out pointer is behind in ptr.
#	mov.l	&dmabuf+1024, %d0
#	bra	dmadon3
#dmadon2:
#	# set count to number of characters from here to out pointer
#	mov.l	dmaoutp, %d0
#dmadon3:
#	sub.l	%a0, %d0
#	mov.w	%d0, dmacount
#	rts
#	global	getdma
#getdma:
#	mov.w	%sr, %d1
#	mov.w	&0x2500, %sr		# spl5()
#	tst.w	ndmachars
#	bgt	getdma1
#	mov.w	&-1, %d0
#	bra	getdma3
#getdma1:
#	mov.l	dmaoutp, %a0
#	mov.b	(%a0)+, %d0		# next char
#	cmp.l	%a0, &dmabuf+1024	# off end of buffer?
#	blo	getdma2
#	mov.l	&dmabuf, %a0		# yes; point back to beginning
#getdma2:
#	mov.l	%a0, dmaoutp		# update pointer
#	sub.w	&1, ndmachars
#	bgt	getdma3
#	mov.l	&dmabuf, dmainp		# buffer empty, reset
#	mov.l	&dmabuf, dmaoutp
#	mov.w	&1024, dmacount
#	# for speed, we can do this in the clock interrupt routine.
#	tst.w	frozen
#	beq	getdma3			# queue not frozen
#	mov.w	%d1, %sr
#	bsr	thaw			# queue frozen; open the gates
#	rts
#getdma3:
#	mov.w	%d1, %sr		# splx()
#	rts
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
global	reboot
reboot:
	mov.w	&0, 384*1024+050	# disable bus error
	mov.w	&0x2700, %sr		# spl7()
	mov.l	256*1024+4, %a0
	jmp	(%a0)
data
ticks0:
	long	0
text
global realtime
realtime:
	mov.l	ticks0,%d0
	rts
