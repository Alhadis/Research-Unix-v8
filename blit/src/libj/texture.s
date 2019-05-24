	# texture(bm, rec, tx, fc)	
	# Bitmap *bm;
	# Rectangle rec;
	# Texture tx;

	#  John F. Reiser  summer 1982

	set bm,8
		set base,0
		set width,4
		set rect,6
	set rec,12
		set origin,0
			set x,0
			set y,2
		set corner,4
	set tx,20
	set fc,24


 # bits %d0
 # bits1 %d1
 # ndx %d2
 # mask1 %d3
 # mask2 %d4
 # iw %d5
 # icnt %d6
 # ocnt %d7
 # p %a0
 # frag %a1
 # tp %a2
 # dP %a3
 # func %a4

	set M%1,0x1cfc		# omits %d[01], %a[01567]
	set S%1,-9*4		# nine registers
	set F%1,-9*4

	global texture
texture:
	link %fp,&F%1
	movm.l &M%1,S%1(%fp)
	movm.l bm(%fp),&0x04c1	# %d0=bm; %d6=r.o.x,,r.o.y; %d7=r.c.x,,r.c.y; %a2=tx;
	mov.l %d0,%a3		# bm
	movm.l rect(%a3),&0x3	# %d0=bm.o.x,,bm.o.y; %d1=bm.c.x,,bm.c.y
	cmp.w %d0,%d6	# bm.o.y : r.o.y
	ble.b L10
	mov.w %d0,%d6	# r.o.y = bm.o.y; /* r.o.y was above bm.rect */
L10:
	cmp.w %d7,%d1	# r.c.y : bm.c.y
	ble.b L15
	mov.w %d1,%d7	# r.c.y = bm.c.y; /* r.c.y was below bm.rect */
L15:
	mov.l &0xf,%d2
	and.w %d6,%d2		# word number in texture
	add.w %d2,%d2		# byte offset in lookup table
	sub.w %d6,%d7		# height of rectangle
	ble.b L30
	swap.w %d0		# bm.o.y,,bm.o.x
	swap.w %d1		# bm.c.y,,bm.c.x
	swap.w %d6		# r.o.y,,r.o.x
	swap.w %d7		#    h,,r.c.x
	cmp.w %d0,%d6	# bm.o.x : r.o.x
	ble.b L20
	mov.w %d0,%d6	# r.o.x = bm.o.x; /* r.o.x was left of bm.rect */
L20:
	cmp.w %d7,%d1	# r.c.x : bm.c.x
	ble.b L25
	mov.w %d1,%d7	# r.c.x = bm.c.x; /* r.c.x was right of bm.rect */
L25:
	mov.w %d7,%d1
	sub.w %d6,%d1		# width in bits
L30:
	ble ret
		# set %a0 -> first word of bits
	mov.l %d1,%d4		# save over call
	mov.l %d6,%d0		# r.o.y,,r.o.x
	swap.w %d0
	mov.l %d0,-(%sp)
	mov.l %a3,-(%sp)
	jsr addr
	add.l &8,%sp
	mov.l %d4,%d1

	mov.l &0xf,%d0
	and.w %d7,%d0		# position of rightmost bit
	mov.l &-1,%d4
	lsr.w %d0,%d4
L40:
	not.w %d4		# mask for bits in rightmost word
	beq.b L40		# adjust if full word
	mov.l &0xf,%d0
	and.w %d6,%d0		# position of leftmost bit
	mov.l &-1,%d3
	lsr.w %d0,%d3		# mask for bits in leftmost word

	sub.w &1,%d7
	asr.w &4,%d7		# word number of rightmost bit
	asr.w &4,%d6		# word number of leftmost bit
	sub.w %d6,%d7
	bgt.b L60
	and.w %d4,%d3	# all in one word
	mov.w fc(%fp),%d1
	mov.b $skptab(%pc,%d1.w),%d0
	ext.w %d0	# skip inner loop and fragmentation on right
	br.b L80
skptab:
	byte f_movE-f_mov4, f_bisE-f_bis4, f_bicE-f_bic4, f_xorE-f_xor4
L60:
	sub.w &1,%d7		# inner width in words
	mov.w &3,%d0
	and.w %d7,%d0
	sub.w &4,%d0
	neg.w %d0
	add.w %d0,%d0		# fragmentation offset
L80:
	mov.l &0,%d1
	mov.w width(%a3),%d1
	sub.w %d7,%d1		# compensate for (%an)+ displacement
	sub.w &1,%d1		# first word has (%an)+ also
	add.l %d1,%d1		# byte displacement
	mov.l %d1,%a3
	lsr.w &2,%d7		# nchunks
	mov.w %d7,%d5
	swap.w %d7		# height
	mov.w fc(%fp),%d1
	asl.w &2,%d1
	lea.l $ftab(%pc,%d1.w),%a4
	mov.l %a4,%a1
	add.w (%a1)+,%a4
	add.w (%a1),%a1
	add.w %d0,%a1
	sub.w &1,%d7
	sub.l %a3,%a0
get:
	add.l %a3,%a0
	mov.w 0(%a2,%d2.w),%d0
	add.w &2,%d2; and.w &0x1e,%d2
	mov.w %d0,%d1; and.w %d3,%d1
	mov.w %d5,%d6
	jmp (%a4)
ftab:
	short f_mov-ftab-0,  f_mov4-ftab-2
	short f_bis-ftab-4,  f_bis4-ftab-6
	short f_bic-ftab-8,  f_bic4-ftab-10
	short f_xor-ftab-12, f_xor4-ftab-14

f_xor:
	eor.w %d1,(%a0)+
	jmp (%a1)
f_xor4:
	eor.w %d0,(%a0)+; eor.w %d0,(%a0)+; eor.w %d0,(%a0)+; eor.w %d0,(%a0)+
	dbr %d6,f_xor4
	and.w %d4,%d0; eor.w %d0,(%a0)
f_xorE:
	dbr %d7,get
	br.b ret

f_mov:
	mov.w (%a0),%d1
	eor.w %d0,%d1; and.w %d3,%d1; eor.w %d1,(%a0)+
	jmp (%a1)
f_mov4:
	mov.w %d0,(%a0)+; mov.w %d0,(%a0)+; mov.w %d0,(%a0)+; mov.w %d0,(%a0)+
	dbr %d6,f_mov4
	mov.w (%a0),%d1
	eor.w %d0,%d1; and.w %d4,%d1; eor.w %d1,(%a0)
f_movE:
	dbr %d7,get
ret:
	movm.l S%1(%fp),&M%1
	unlk %fp
	rts

f_bic:
	not.w %d3; not.w %d4
	add.l &6,%a4
	not.w %d0; mov.w %d0,%d1
	or.w %d3,%d1; and.w %d1,(%a0)+
	jmp (%a1)
f_bic4:
	and.w %d0,(%a0)+; and.w %d0,(%a0)+; and.w %d0,(%a0)+; and.w %d0,(%a0)+
	dbr %d6,f_bic4
	or.w %d4,%d0; and.w %d0,(%a0)
f_bicE:
	dbr %d7,get
	br.b ret

f_bis:
	or.w %d1,(%a0)+
	jmp (%a1)
f_bis4:
	or.w %d0,(%a0)+; or.w %d0,(%a0)+; or.w %d0,(%a0)+; or.w %d0,(%a0)+
	dbr %d6,f_bis4
	and.w %d4,%d0; or.w %d0,(%a0)
f_bisE:
	dbr %d7,get
	br.b ret
