# ldiv	- long signed division
# lrem	- long signed remainder
# uldiv	- long unsigned division
# ulrem - long unsigned remainder

# enter with d0 == dividend; 4(sp) == divisor

# return with result in d0.
# d1, a0 & a1 are blasted.

	text
	global	ldiv%%
	global	lrem%%
	global	uldiv%%
	global	ulrem%%

get_args:			# set up arguments as unsigned arguments
	mov.l	%d2,%a0		# save d2 in a0
	mov.l	8(%sp),%d2	# get divisor in d2
	bpl.b	get_arg1
	neg.l	%d2		# take absolute value
get_arg1:
	mov.l	%d0,%d1		# get dividend in d1
	bpl.b	get_arg2
	neg.l	%d1		# take absolute value
get_arg2:
	rts

lrem%%:				# sign of remainder depends only on dividend
	bsr.b	get_args
	tst.l	%d0
	bpl.b	lrem1
	bsr.b	uldiv1		# do unsigned divison
	neg.l	%d1		# negative dividend means negative remainder
	br.b	lrem2
lrem1:
	bsr.b	uldiv1		# do unsigned divison
lrem2:
	mov.l	%d1,%d0		# result is remainder
	rts

ulrem%%:
	mov.l	%d2,%a0		# save d2 in a0
	mov.l	4(%sp),%d2	# pick up divisor
	bsr.b	uldiv0		# do unsigned division
	mov.l	%d1,%d0		# result is remainder
	rts

ldiv%%:				# quotient is negative if input signs differ
	bsr.b	get_args
	eor.l	%d0,4(%sp)	# blast divisor with signs-different flag
	bsr.b	uldiv1		# do unsigned division
	tst.b	4(%sp)
	bpl.b	ldiv1
	neg.l	%d0		# negative quotient if signs different
ldiv1:
	rts

uldiv%%:
	mov.l	%d2,%a0		# save d2 in a0
	mov.l	4(%sp),%d2	# pick up divisor
uldiv0:
	mov.l	%d0,%d1		# pick up dividend
uldiv1:
	cmp.l %d2,&65535
	bhi.b uldiv2
	swap.w %d1		# can use hardware divide
	mov.l &0,%d0
	mov.w %d1,%d0
	divu.w %d2,%d0
	swap.w %d0
	mov.w %d0,%d1
	swap.w %d1
	divu.w %d2,%d1
	mov.w %d1,%d0		# d0 = unsigned quotient
	clr.w %d1
	swap.w %d1		# d1 = unsigned quotient
	mov.l	%a0,%d2		# restore d2
	rts

# The divisor is known to be >= 2^16 so only 16 cycles are needed.
uldiv2:
	mov.l	%d1,%d0
	clr.w	%d1
	swap.w	%d1
	swap.w	%d0
	clr.w	%d0

		# %d1 now             0,,hi(dividend)
		# %d0 now  lo(dividend),,0
		#              this zero ^ shifts left 1 bit per cycle,
		#              becoming top half of quotient

	mov.l	%d3,%a1		# save d3 in a1 across loop
	mov.l	&16-1,%d3	# dbr counts down to -1
uldiv3:
	add.l	%d0,%d0		# add is 2 cycles faster than shift or rotate
	addx.l	%d1,%d1
	cmp.l	%d2,%d1
	bhi.b	uldiv4
	sub.l	%d2,%d1
	add.w	&1,%d0		# bottom bit changes from 0 to 1 (no carry)
uldiv4:
	dbr	%d3,uldiv3
	mov.l	%a1,%d3		# restore d3
	mov.l	%a0,%d2		# restore d2
	rts
