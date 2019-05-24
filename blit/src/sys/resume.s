	text
	global	resume
	set	P, 0406			# absolute address
resume:
	link	%fp,&-46
	movm.l	&0x3CFC,-40(%fp)	# save registers
	mov.l	8(%fp),%a1		# proctab entry for new proc
	mov.l	P,%a0
	mov.l	%fp,(%a0)		# save his frame pointer
	mov.l	%a1,P			# P is now new proc
	mov.l	(%a1),%fp		# reset frame pointer
	movm.l	-40(%fp),&0x3CFC	# restore reg's and return
	unlk	%fp
	rts				# now in new proc
	data
