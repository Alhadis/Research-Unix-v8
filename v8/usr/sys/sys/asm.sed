/calls/{
s/calls	$0,_spl0/mfpr	$18,r0\
	mtpr	$0,$18/
s/calls	$0,_spl4/mfpr	$18,r0\
	mtpr	$0x14,$18/
s/calls	r[0-9]*,_spl4/mfpr	$18,r0\
	mtpr	$0x14,$18/
s/calls	$0,_spl5/mfpr	$18,r0\
	mtpr	$0x15,$18/
s/calls	r[0-9]*,_spl5/mfpr	$18,r0\
	mtpr	$0x15,$18/
s/calls	$0,_spl6/mfpr	$18,r0\
	mtpr	$0x18,$18/
s/calls	r[0-9]*,_spl6/mfpr	$18,r0\
	mtpr	$0x18,$18/
s/calls	$0,_spl7/mfpr	$18,r0\
	mtpr	$0x1f,$18/
s/calls	$1,_splx/mfpr	$18,r0\
	mtpr	(sp)+,$18/
s/calls	$1,_mfpr/mfpr	(sp)+,r0/
s/calls	$2,_mtpr/mtpr	4(sp),(sp)\
	addl2	$8,sp/
s/calls	$1,_resume/ashl	$9,(sp)+,r0 \
	movpsl	-(sp) \
	jsb	_Resume/
s/calls	$3,_bcopy/movc3	8(sp),*(sp),*4(sp)\
	addl2	$12,sp/
s/calls	$3,_strncmp/cmpc3	8(sp),*(sp),*4(sp)\
	addl2	$12,sp/
s/calls	$3,_copyin/jsb	_Copyin\
	addl2	$12,sp/
s/calls	$3,_copyout/jsb	_Copyout\
	addl2	$12,sp/
s/calls	$1,_fubyte/movl	(sp)+,r0 \
	jsb	_Fubyte/
s/calls	$1,_fuibyte/movl (sp)+,r0 \
	jsb	_Fubyte/
s/calls	$1,_fuword/movl (sp)+,r0 \
	jsb	_Fuword/
s/calls	$1,_fuiword/movl (sp)+,r0 \
	jsb	_Fuword/
s/calls	$2,_subyte/movl	(sp)+,r0 \
	movl	(sp)+,r1 \
	jsb	_Subyte/
s/calls	$2,_suibyte/movl (sp)+,r0 \
	movl	(sp)+,r1 \
	jsb	_Subyte/
s/calls	$2,_suword/movl (sp)+,r0 \
	movl	(sp)+,r1 \
	jsb	_Suword/
s/calls	$2,_suiword/movl (sp)+,r0 \
	movl	(sp)+,r1 \
	jsb	_Suword/
s/calls	$1,_setrq/movl	(sp)+,r0 \
	jsb	_Setrq/
s/calls	$1,_remrq/movl	(sp)+,r0 \
	jsb	_Remrq/
s/calls	$0,_swtch/movpsl	-(sp)\
	jsb	_Swtch/
s/calls	$1,_setjmp/movl	(sp)+,r0 \
	jsb	_Setjmp/
s/calls	$1,_longjmp/movl	(sp)+,r0 \
	jsb	_Longjmp/
s/calls	$1,_ffs/ffs	$0,$32,(sp)+,r0 \
	bneq	1f \
	mnegl	$1,r0 \
1: \
	incl	r0/
s/calls	$2,_insque/insque	*(sp)+,*(sp)+/
s/calls	$1,_remque/remque	*(sp)+,r0/
s/calls	$2,_bzero/movc5	$0,(r0),$0,4(sp),*(sp)\
	addl2	$8,sp/
s/calls	$1,_htons/rotl	$8,(sp),r0\
	movb	1(sp),r0\
	movzwl	r0,r0\
	addl2	$4,sp/
s/calls	$1,_ntohs/rotl	$8,(sp),r0\
	movb	1(sp),r0\
	movzwl	r0,r0\
	addl2	$4,sp/
s/calls	$1,_htonl/rotl	$-8,(sp),r0\
	insv	r0,$16,$8,r0\
	movb	3(sp),r0\
	addl2	$4,sp/
s/calls	$1,_ntohl/rotl	$-8,(sp),r0\
	insv	r0,$16,$8,r0\
	movb	3(sp),r0\
	addl2	$4,sp/
}
