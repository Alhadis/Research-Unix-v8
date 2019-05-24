	.data
	.set	defont, 0x0000ae08
	.globl	defont
	.set	reboot, 0x00001808
	.globl	reboot
	.set	checkbram, 0x0000346c
	.globl	checkbram
	.set	setbram, 0x000034e4
	.globl	setbram
	.comm	mouse,8
	.text
	.align	4
	.globl	_start
_start:
	movh	&0,5242880
	movb	&2,2097215
	call	&0,qinit
	movzbw	6291458,%r0
	movzbw	baud_speeds(%r0),%r0
	pushw	%r0
	call	&1,aciainit
	call	&0,binit
	call	&0,kbdinit
	call	&0,cursinit
	pushw	&end
	cmpb	6293126,&0	# VALMAXADDR
	je	.L85
	pushw	&end+262144
	jmp	.L86
.L85:
	pushw	&end+50000
.L86:
	call	&2,allocinit
	call	&0,gcinit
	call	&0,spl0
	call	&0,main
	jmp	reboot

	.text
	.globl	msvid_int
msvid_int:
	SAVE	%r8
	MOVB	0x200013,%r5
	BITB	&0x40,%r5
	BEB	mous_chk
	BITB	&0x04,%r5	# update only at 60 hz. rate
	BNEB	mous_chk	# if fall through, have	a video	interrupt
	call	&0,auto1		# calls	cursor update in cursor.c
	call	&0,clockroutine
	INCW	ticks0
mous_chk:
	BITB	&0xb0,%r5
	BEB	msvid_exit
				# if fall through, have	a mouse	interrupt
	call	&0,auto4		# call buttons handler in buttons.c
msvid_exit:
	RESTORE	%r8
	RETPS

	.text
	.globl	key_int
key_int:
	call	&0,auto2
	RETPS

	.text
	.globl	host_int
host_int:
	pushw	&queues
	andw3	0x20000c,&255,%r0
	pushw	%r0
	call	&2,qputc
	RETPS

	.text
	.globl	out_int
out_int:
	call	&0,aciatrint
	RETPS

	.data
	.globl	ticks0
	.bss	ticks0,4,4

	.text
	.globl	realtime
realtime: movw	ticks0,%r0
	RET


	.globl	spl0
spl0:	movw	%psw,%r0
	ANDW2	&0xfffe1fff,%psw	# turn off all priority bits
	TSTW	%r0
	RET

	.globl	spl1
	.globl	spl4
spl1:
spl4:	movw	%psw,%r0
	INSFW	&4,&13,&14,%psw
	TSTW	%r0
	RET

	.globl	spl5
	.globl	spl6
	.globl	spl7
spl5:
spl6:
spl7:	movw	%psw,%r0
	ORW2	&0x1e000,%psw
	TSTW	%r0
	RET
	.globl	splx
splx:
	movw	%psw,%r0
	LRSW3	&13,0(%ap),%r1
	INSFW	&4,&13,%r1,%psw
	TSTW	%r0
	RET

	.globl	excep_norm
excep_norm:
	ORW2	&0x1e000,%psw	# IPL up to 15
	TSTW	%r0		# let psw bits settle 
	pushw	&.normmsg
	call	&1, .death

	.globl	excep_stack
excep_stack:
	ORW2	&0x1e000,%psw	# IPL up to 15
	TSTW	%r0		# let psw bits settle 
	pushw	&.stackmsg
	call	&1, .death

	.globl	excep_int
	.globl	excep_proc
excep_int:
excep_proc:
	ORW2	&0x1e000,%psw	# IPL up to 15
	TSTW	%r0		# let psw bits settle 
	pushw	&.procmsg
	call	&1, .death
	.data
.normmsg:
	.byte	116,157,162,155,141,154,040,105,170,143,145,160,164,151,157,156,0
.procmsg:
	.byte	120,162,157,143,145,163,163,040,105,170,143,145,160,164,151,157,156,0
.stackmsg:
	.byte	123,164,141,143,153,040,105,170,143,145,160,164,151,157,156,0
	.text
	.globl	.death
.death:
	save	&0
	pushw	&display
	pushw	&0		# Rect(0, 0,
	pushw	&32768050	# 500, 50)
	pushw	&2
	call	&4,rectf
	pushw	&defont
	pushw	0(%ap)
	pushw	&display
	pushw	&1638435	# Pt(15, 35)
	pushw	&3
	call	&5,string
.hang:
	jmp	.hang

#	hacks
	.globl	ringbell
	.globl	test32
	.globl	pfkey
	.globl	curse
	.globl	pt
ringbell:
test32:
pfkey:
curse:
pt:
	RET

	.data
	.comm	blocked,4
	.comm	ublocked,4
	.comm	cur,4
