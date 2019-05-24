	data	1
	text
	global	almul%
almul%:
	link	%fp,&F%1
	mov.w	&M%1,-2(%fp)
	movm.l	&M%1,S%1(%fp)
#	line 4, file "muldiv%.c"
	mov.l	12(%fp),(%sp)
	mov.l	8(%fp),%a0
	mov.l	(%a0),-(%sp)
	jsr	lmul%
	add.l	&4,%sp
	mov.l	8(%fp),%a1
	mov.l	%d0,(%a1)
L%12:
	movm.l	S%1(%fp),&M%1
	unlk	%fp
	rts
	set	S%1,-2
	set	T%1,-2
	set	F%1,-6
	set	M%1,00
	data	1
	text
	global	aldiv%
aldiv%:
	link	%fp,&F%2
	mov.w	&M%2,-2(%fp)
	movm.l	&M%2,S%2(%fp)
#	line 10, file "muldiv%.c"
	mov.l	12(%fp),(%sp)
	mov.l	8(%fp),%a0
	mov.l	(%a0),-(%sp)
	jsr	ldiv%
	add.l	&4,%sp
	mov.l	8(%fp),%a1
	mov.l	%d0,(%a1)
L%14:
	movm.l	S%2(%fp),&M%2
	unlk	%fp
	rts
	set	S%2,-2
	set	T%2,-2
	set	F%2,-6
	set	M%2,00
	data	1
	text
	global	alrem%
alrem%:
	link	%fp,&F%3
	mov.w	&M%3,-2(%fp)
	movm.l	&M%3,S%3(%fp)
#	line 16, file "muldiv%.c"
	mov.l	12(%fp),(%sp)
	mov.l	8(%fp),%a0
	mov.l	(%a0),-(%sp)
	jsr	lrem%
	add.l	&4,%sp
	mov.l	8(%fp),%a1
	mov.l	%d0,(%a1)
L%16:
	movm.l	S%3(%fp),&M%3
	unlk	%fp
	rts
	set	S%3,-2
	set	T%3,-2
	set	F%3,-6
	set	M%3,00
	data	1
	text
	global	uldiv%
uldiv%:
	link	%fp,&F%4
	mov.w	&M%4,-2(%fp)
	movm.l	&M%4,S%4(%fp)
#	line 23, file "muldiv%.c"
	mov.l	12(%fp),(%sp)
	mov.l	8(%fp),-(%sp)
	jsr	ldiv%
	add.l	&4,%sp
	br	L%18
L%18:
	movm.l	S%4(%fp),&M%4
	unlk	%fp
	rts
	set	S%4,-2
	set	T%4,-2
	set	F%4,-6
	set	M%4,00
	data	1
	text
	global	ulrem%
ulrem%:
	link	%fp,&F%5
	mov.w	&M%5,-2(%fp)
	movm.l	&M%5,S%5(%fp)
#	line 30, file "muldiv%.c"
	mov.l	12(%fp),(%sp)
	mov.l	8(%fp),-(%sp)
	jsr	lrem%
	add.l	&4,%sp
	br	L%20
L%20:
	movm.l	S%5(%fp),&M%5
	unlk	%fp
	rts
	set	S%5,-2
	set	T%5,-2
	set	F%5,-6
	set	M%5,00
	data	1
