# count subroutine called during profiling
#
	global	mcount
	comm	countbase,4

mcount:
	mov.l	(%a0),%a1
	beq 	init
incr:
	add.l	&1,(%a1)
return:
	rts
init:
	mov.l	countbase,%a1
	beq 	return
	add.l	&8,countbase
	mov.l	(%sp),(%a1)+
	mov.l	%a1,(%a0)
	br  	incr
