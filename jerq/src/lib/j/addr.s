	.text
	.def	addr;	.val	addr;	.scl	2;	.type	0144;	.endef
	.globl	addr
addr:
	MOVW	0(%ap),%r2		# get pointer to bitmap
	ARSH3	&5,4(%ap),%r0		# p.x >> 5
	ARSH3	&5,8(%r2),%r1		# dm->rect.origin.x >> 5
	SUBW2	%r1,%r0			# (p.x >> 5) - (dm->rect.origin.x >> 5)
	SUBH3	10(%r2),6(%ap),%r1	# p.y - dm->rect.origin.y -> r1
	MULW2	{uword}4(%r2),%r1	# above * dm->width
	ADDW2	%r1,%r0			# add word offsets for x and y
	ALSW3	&2,%r0,%r0		# mult by 4 to get offset in bytes -> r0
	ADDW2	0(%r2),%r0		# add Bitmap.base to offset -> r0
	RET	
	.def	addr;	.val	.;	.scl	-1;	.endef
