# long multiply

	global	lmul%%

lmul%%:
	link	%fp,&0
	mov.w	&014,-(%sp)
	movm.l	&030000,-(%sp)
	mov.w	%d0,%d2
	mov.w	%d0,%d1
	ext.l	%d1
	swap.w	%d1
	swap.w	%d0
	sub.w	%d0,%d1
	mov.w	10(%fp),%d0
	mov.w	%d0,%d3
	ext.l	%d3
	swap.w	%d3
	sub.w	8(%fp),%d3
	muls.w	%d0,%d1
	muls.w	%d2,%d3
	add.w	%d1,%d3
	muls.w	%d2,%d0
	swap.w	%d0
	sub.w	%d3,%d0
	swap.w	%d0
	movm.l	(%sp)+,&014
	unlk	%fp
	rts
