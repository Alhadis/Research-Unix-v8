	text
	global	addr
addr:
	mov.l 4(%sp),%a1	# get pointer to Bitmap
	mov.l (%a1)+,%a0	# base
	mov.l &0xf,%d1		# no borrows for subtraction of bottom 4 bits
	or.l (%a1)+,%d1 	# width,,origin.x
	sub.w 8(%sp),%d1	# origin.x - p.x
	asr.w &4,%d1	# -x offset in words
	add.w %d1,%d1	# double to get bytes
	sub.w %d1,%a0	# add to base
	mov.w 10(%sp),%d0	# p.y
	sub.w (%a1),%d0 	# p.y - origin.y
	swap.w %d1	# width
	muls.w %d1,%d0
	add.l %d0,%d0		# double to get byte address
	add.l %d0,%a0		# add to base
	rts
