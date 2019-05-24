#
#	Loader, 'jmp load'
#

#
#	cp	%a2
#	error	%a4
#	c	%d0
#	size	%d3
#	state	%d5
#	tcrc	%d6
#	ack	%d7
#
set	PTYP,		0xc0
set	ACKON,		0x80
set	ACKOFF,		0xc0
set	NOCRC,		0x40
#define	SEQMASK		((~PTYP)&0xff)
#define	SEQMOD		(SEQMASK+1)

	data	1
	lcomm	packet,136	# incoming packet buffer
	text
	global	load
load:
	clr.l	%a4
nextpkt:
	mov.l	&0,%d5
continue:
	jsr	getc
	tst.w	%d0
	blt	badcrc
	mov.w	6(%pc,%d5.w),%d1
	jmp	2(%pc,%d1.w)
L%46:
	short	L%29-L%46
	short	L%31-L%46
	short	L%34-L%46
	short	L%36-L%46
	short	L%38-L%46
	short	badcrc-L%46
L%29:	# case 0:
	mov.l	&packet,%a2
	mov.b	%d0,(%a2)+
	mov.w	%d0,%d6
	and.w	&PTYP,%d6
	cmp.w	%d6,&ACKON
	bne.b	L%30
	clr.l	%a4
	mov.l	&1,%d7
	br.b	L%53
L%30:
	cmp.w	%d6,&ACKOFF
	bne.b	L%32
	mov.l	&0,%d7
L%53:
	mov.l	&0,%d6
stateplusplus:
	add.w	&2,%d5
	bra	break
L%32:
	cmp.w	%d6,&NOCRC
	bne.b	continue
	mov.l	&-1,%d7
	bra.b	L%53
L%31:	# case 1:
	mov.w	%d0,%d3
	sub.w	&4,%d3
	blt.b	nextpkt
	cmp.w	%d0,&132
	bgt.b	nextpkt
	mov.w	%d0,%d2
	mov.b	%d0,(%a2)+
	bra.b	stateplusplus
L%34:	# case 2:
	mov.b	%d0,(%a2)+
	sub.w	&1,%d2
	bgt.b	break
L%35:
	tst.w	%d7
	blt.b	L%39
	bra.b	stateplusplus
L%36:	# case 3:
	cmp.b	%d0,%d6
	beq.b	L%37
	add.w	&2,%d5
L%37:
	add.w	&2,%d5
	br	continue
L%38:	# case 4:
	asr.w	&8,%d6
	cmp.b	%d0,%d6
	bne.b	badcrc
L%39:
	mov.b	packet,%d0
	mov.l	packet+2,%a3
	tst.w	%d3
	bne.b	L%40
	mov.l	%a4,%d1
	bne	nextpkt
	tst.w	%d7
	ble.b	L%52
	jsr	putc
	jsr	putc
	jsr	putc
L%52:
	jmp	(%a3)
L%40:
	tst.w	%d7
	ble.b	L%51
	jsr	putc
L%51:
	mov.l	&packet+6,%a2
L%43:
	mov.b	(%a2)+,(%a3)+
L%42:
	sub.w	&1,%d3
	bne.b	L%43
	br	nextpkt
badcrc:
	mov.l	&1,%a4
	br	nextpkt
break:
	eor.b	%d6,%d0		# byte-at-time crc calculation
	mov.l	&15,%d1	
	and.b	%d0,%d1	
	add.l	%d1,%d1	
	mov.l	&crc16t_3,%a1
	mov.w	0(%a1,%d1.l),%d1
	lsr.b	&3,%d0	
	and.w	&30,%d0	
	mov.l	&crc16t_3+32,%a0
	mov.w	0(%a0,%d0.w),%d4
	eor.w	%d1,%d4	
	lsr.w	&8,%d6	
	eor.w	%d4,%d6	
	bra	continue
#
#	CRC-16 32 word look-up table
#
	data	1
	even
	global	crc16t_3
crc16t_3:
	short	0
	short	49345
	short	49537
	short	320
	short	49921
	short	960
	short	640
	short	49729
	short	50689
	short	1728
	short	1920
	short	51009
	short	1280
	short	50625
	short	50305
	short	1088
	short	0
	short	52225
	short	55297
	short	5120
	short	61441
	short	15360
	short	10240
	short	58369
	short	40961
	short	27648
	short	30720
	short	46081
	short	20480
	short	39937
	short	34817
	short	17408
#
#	This version is a byte-at-a-time calculation
#
#	newcrc = bcrc(oldcrc, byte);
#
#	text
#	even
#	global	bcrc
#bcrc:
#	link	%fp,&bcrcF
#	movm.l	&bcrcM,bcrcS(%fp)
#	mov.w	8(%fp),%d2	# old crc
#	mov.w	10(%fp),%d3	# new byte
#	eor.b	%d2,%d3
#	mov.l	&15,%d0
#	and.b	%d3,%d0
#	add.l	%d0,%d0
#	mov.l	&crc16t_3,%a1
#	mov.w	0(%a1,%d0.l),%d0
#	lsr.b	&3,%d3
#	and.w	&30,%d3
#	mov.l	&crc16t_3+32,%a0
#	mov.w	0(%a0,%d3.w),%d1
#	eor.w	%d0,%d1
#	lsr.w	&8,%d2
#	eor.w	%d1,%d2
#	mov.w	%d2,%d0
#	movm.l	bcrcS(%fp),&bcrcM
#	unlk	%fp
#	rts
#	set	bcrcS,-16
#	set	bcrcF,-22
#	set	bcrcM,02034
#
#	This version calculates the crc for a whole packet.
#	The calculated crc is compared with the one found at the tail;
#	- returns !0 for not equal, 0 for equal.
#	The correct crc is stored at the tail.
#
#	error = crc(&packet, length);
#
	text
	even
	global	crc
crc:
	link	%fp,&crcF
	movm.l	&crcM,crcS(%fp)
	mov.l	8(%fp),%a2	# packet address
	clr.l	%d2		# calculated crc
	mov.w	12(%fp),%d4	# length
	ble	crc%140
crc%170:
	mov.b	(%a2)+,%d3
	eor.b	%d2,%d3
	mov.l	&15,%d0
	and.b	%d3,%d0
	add.l	%d0,%d0
	mov.l	&crc16t_3,%a1
	mov.w	0(%a1,%d0.l),%d0
	lsr.b	&3,%d3
	and.w	&30,%d3
	mov.l	&crc16t_3+32,%a0
	mov.w	0(%a0,%d3.w),%d1
	eor.w	%d0,%d1
	lsr.w	&8,%d2
	eor.w	%d1,%d2
	sub.w	&1,%d4
	bgt	crc%170
crc%140:
	cmp.b	%d2,(%a2)
	beq	crc%180
	add.w	&1,%d4
crc%180:
	mov.b	%d2,(%a2)+
	lsr.w	&8,%d2
	cmp.b	%d2,(%a2)
	beq	crc%190
	add.w	&1,%d4
crc%190:
	mov.b	%d2,(%a2)+
	mov.w	%d4,%d0
	movm.l	crcS(%fp),&crcM
	unlk	%fp
	rts
	set	crcS,-16
	set	crcF,-22
	set	crcM,02034
