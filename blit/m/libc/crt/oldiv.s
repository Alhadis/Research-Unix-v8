	global	ldiv%%
	global	lrem%%
	global	uldiv%%
	global	ulrem%%
lrem%%:
ulrem%%:
	mov.l %d2,-(%sp)
	mov.l %d3,-(%sp)
	bsr.b ldiv0
	mov.l &2,%d3		# sign of remainder depends only on sign of dividend
	bsr.b ldiv1
	neg.l %d1
	mov.l %d1,%d0
return:
	mov.l (%sp)+,%d3
	mov.l (%sp)+,%d2
	rts
ldiv%%:
uldiv%%:
	mov.l %d2,-(%sp)
	mov.l %d3,-(%sp)
	bsr.b ldiv0
	bsr.b ldiv1
	neg.l %d0
	bra.b return
ldiv0:		# setup divisor
	mov.l &2,%d3		# default is positive result
	mov.l 16(%sp),%d2
	bpl.b ldiv0r
	neg.l %d2
	mov.l &0,%d3		# negative result
ldiv0r:
	rts
ldiv1:		# setup dividend and go
	mov.l %d0,%d1
	bpl.b ldiv1g
	neg.l %d1
	bchg &1,%d3		# result changes sign
ldiv1g:
	add.l %d3,(%sp)		# a bit of microprogramming!
	cmp.l %d2,&65535
	bhi.b ldivhard
	swap.w %d1
	mov.l &0,%d0
	mov.w %d1,%d0
	divu.w %d2,%d0
	swap.w %d0
	mov.w %d0,%d1
	swap.w %d1
	divu.w %d2,%d1
	mov.w %d1,%d0		# %d0=quo
	clr.w %d1
	swap.w %d1		# %d1=rem
	rts

# The divisor is known to be >= 2^16 so only 16 cycles are needed.
ldivhard:
	mov.l %d1,%d0
	clr.w %d1
	swap.w %d1
	swap.w %d0
	clr.w %d0
		# %d1 now             0,,hi(dividend)
		# %d0 now  lo(dividend),,0
		#              this zero ^ shifts left 1 bit per cycle,
		#              becoming top half of quotient
		#
	mov.l &16-1,%d3		# dbr counts down to -1
ldiv10:
	add.l %d0,%d0		# add is 2 cycles faster than shift or rotate
	addx.l %d1,%d1
	cmp.l %d2,%d1
	bhi.b ldiv11
	sub.l %d2,%d1
	add.w &1,%d0		# bottom bit changes from 0 to 1 (no carry)
ldiv11:
	dbr %d3,ldiv10
	rts
