#
#  bitblt(sm,r,dm,p,fc)
#  Bitmap *sm,*dm;
#  Rectangle r;
#  Point p;
#  int fc;
#
#  by John F. Reiser  summer 1982
#
#  Depending on the case at hand, generate very good code and execute it.
#

		# offsets in a Point
	set x,0
	set y,2
		# offsets in a Rectangle
	set origin,0
	set corner,4
		# offsets in a Bitmap
	set base,0
	set width,4
	set rect,6
		# parameter offsets from %fp
	set sm,8
	set r,12
	set dm,20
	set p,24
	set fc,28

	set NREG,11

	global bitblt
bitblt:
	movm.l &0x3f3e,-(%sp)		# save C registers
	movm.l NREG*4-4+sm(%sp),&0x001f
		# d1=r.o.x,,r.o.y; d2=r.c.x,,r.c.y; d4=p.x,,p.y;
	mov.l %d0,%a4	# sm
	mov.l %d3,%a5	# dm
	mov.w NREG*4-4+fc(%sp),%a6	# a6.w == fc
	movm.l rect(%a4),&0x9	# d0=sm.o.x,,sm.o.y; d3=sm.c.x,,sm.c.y;
	movm.l rect(%a5),&0x60	# d5=dm.o.x,,dm.o.y; d6=dm.c.x,,dm.c.y;

	lea.l $L50(%pc),%a0
L5:
		# clip r.y to sm.y
	mov.w %d0,%d7	# sm.o.y
	sub.w %d1,%d7	# - r.o.y
	ble.b L10
	mov.w %d0,%d1	# r.o.y = sm.o.y; /* r.o.y was above sm.rect */
	add.w %d7,%d4	# p.y parallels r.o.y
L10:
	cmp.w %d2,%d3	# r.c.y : sm.c.y
	ble.b L20
	mov.w %d3,%d2	# r.c.y = sm.c.y; /* bottom of r was below sm.rect */
L20:
		# clip (r.y at p.y) to dm.y
	mov.w %d5,%d7	# dm.o.y
	sub.w %d4,%d7	# -p.y
	ble.b L30
	mov.w %d5,%d4	# p.y = dm.o.y; /* p.y was above dm.rect */
	add.w %d7,%d1	# r.o.y parallels p.y
L30:
	mov.w %d1,%d7	# r.o.y
	add.w %d6,%d7	# + dm.c.y
	sub.w %d4,%d7	# - p.y  /* == max y that dm.rect allows in r */
	cmp.w %d2,%d7	# r.c.y : limit
	ble.b L40
	mov.w %d7,%d2	# r.c.y = limit
L40:
	mov.w %d2,%d7	# r.c.y
	sub.w %d1,%d7	# - r.o.y
	sub.w &1,%d7	# /* == h-1  in bits */
	blt.b ret
	jmp (%a0)

retgen:
	lea.l gensiz(%sp),%sp
ret8:
	add.l &8,%sp
ret:
	movm.l (%sp)+,&0x7cfc
	rts

L50:
		# mirror in pi/4 and reuse same code to clip x
	swap.w %d0; swap.w %d1; swap.w %d2; swap.w %d3
	swap.w %d4; swap.w %d5; swap.w %d6; swap.w %d7
	lea.l $L55(%pc),%a0
	br.b L5

L55:
	mov.l %d1,%a1
	mov.l %d4,%d6
#
#  So far
#	%d7 == h-1,,w-1
#	%d6 == p.y,,p.x
#	%a6.w == fc
#	%a5 == dm
#	%a4 == sm
#	%a1 == r.o.y,,r.o.x
#
#  Compute masks, and width in words
#
	mov.w %d6,%d0		# p.x  /* left endpoint of dst */
	mov.w %d7,%d1		# w-1
	add.w %d6,%d1		# right endpoint

	mov.l &-1,%d3
	mov.l &15,%d2
	and.w %d0,%d2
	lsr.w %d2,%d3		# mask1
	mov.l &-1,%d5
	mov.l &15,%d2
	and.w %d1,%d2
	add.w &1,%d2
	lsr.w %d2,%d5
	not.w %d5		# mask2
	swap.w %d5
	mov.w %d3,%d5		# mask2,,mask1

	asr.w &4,%d0
	asr.w &4,%d1
	sub.w %d0,%d1
	sub.w &1,%d1		# inner-loop width in words

	mov.l &0,%d4		# assume LtoR
	mov.w width(%a5),%d3
	add.w %d3,%d3
	mov.w width(%a4),%d2
	add.w %d2,%d2
#
#  So far
#	%d7 == h-1,,w-1  in bits
#	%d6 == p.y,,p.x
#	%d5 == mask2,,mask1
#	%d4 == 0  (LtoR)
#	%d3.w == dm width in bytes
#	%d2.w == sm width in bytes
#	%d1.w == inner-loop width in words
#	%a6.w == fc
#	%a5 == dm
#	%a4 == sm
#	%a1 == r.o.y,,r.o.x
#
#  If necessary, compensate for overlap of source and destination
#
	cmp.l %a4,%a5
	bne.b L80		# overlap not possible
	mov.l %d6,%d0	# p.y,,p.x
	mov.w %a1,%d0	# p.y,,r.o.x
	cmp.l %a1,%d0	# r.o.y : p.y
	bge.b L60	# if (r.o.y < p.y)
	mov.l %d7,%d0	# h-1,,w-1
	clr.w %d0		# h-1,,0
	add.l %d0,%a1	# r.o.y += h-1;
	add.l %d0,%d6	# p.y += h-1;
	neg.w %d3		# wdst = -wdst;
	neg.w %d2		# wsrc = -wsrc;
L60:
	cmp.w %d7,&16
	blt.b L70		# l<->r swap not needed for narrow
	cmp.w %d6,%a1	# p.x : r.o.x
	ble.b L70	# if (r.o.x < p.x)
	mov.l %a1,%d0
	add.w %d7,%d0
	mov.l %d0,%a1	# r.o.x += w-1;
	add.w %d7,%d6	# p.x += w-1;
	mov.l &-1,%d4	# RtoL
	swap.w %d5		# masks in other order
L70:
L80:
#
#  Locate actual starting points
#
	mov.l %d6,%d0	# p.y,,p.x
	swap.w %d0
	mov.l %d0,-(%sp)	# p
	mov.l %a5,-(%sp)	# dm

	mov.l &15,%d0
	lea.l $L82(%pc),%a0	# assume narrow
	cmp.w %d7,%d0		# w-1 : 15
	ble.b L81		# guessed correctly
	lea.l $L85(%pc),%a0	# wide
L81:
	mov.l %a0,-(%sp)	# on return, go directly to wide/narrow code
	add.w %a6,%a6; add.w %a6,%a6	# with 4*fc

	mov.w %d1,%d7		# h-1 in bits,,inner width in words
	and.l %d0,%d6		# 0,,bit offset of p.x
	mov.l %a1,%d1		# r.o.y,,r.o.x
	and.w %d1,%d0		# bit offset of r.o.x
	sub.w %d0,%d6		# BO(p.x) - BO(r.o.x) /* amount of right rotation */
	swap.w %d1		# r.o.x,,r.o.y
	mov.l %d1,-(%sp)	# r.o
	mov.l %a4,-(%sp)	# sm
	lea.l addr,%a3
	jsr (%a3)
	mov.l %a0,%a2		# src = addr(sm,r.origin);
	add.l &8,%sp
	jmp (%a3)		# %a0 = addr(dm,p);
L82:
	mov.l &0,%d4
	mov.w %d5,%d4	# 0,,mask1
	swap.w %d5		# mask1,,mask2  (proper long mask; maybe 16 bits too wide)
	and.w %d5,%d4	# check for overlap of mask1 and mask2
	beq.b L83		# no overlap ==> %d5 already correct
	mov.l %d4,%d5	# overlap ==> reduce %d5 by 16 bits
	swap.w %d5		# and put it in the proper half
L83:
	swap.w %d7		# ,,height-1
	lea.l $nrwtab(%pc,%a6.w),%a6	# -> optab
	tst.w %d6		# amount of right rotation
	bge.b L84
	neg.w %d6
	add.l &2,%a6
L84:
	add.w (%a6),%a6
	jmp (%a6)

nrwtab:
	short opMnwr-nrwtab- 0, opMnwl-nrwtab- 2
	short opSnwr-nrwtab- 4, opSnwl-nrwtab- 6
	short opCnwr-nrwtab- 8, opCnwl-nrwtab-10
	short opXnwr-nrwtab-12, opXnwl-nrwtab-14

opMnwr:
	mov.l (%a2),%d0
	mov.l (%a0),%d1
	ror.l %d6,%d0
	eor.l %d1,%d0
	and.l %d5,%d0
	eor.l %d1,%d0
	mov.l %d0,(%a0)
	add.w %d2,%a2
	add.w %d3,%a0
	dbr %d7,opMnwr
	br ret8

opMnwl:
	mov.l (%a2),%d0
	mov.l (%a0),%d1
	rol.l %d6,%d0
	eor.l %d1,%d0
	and.l %d5,%d0
	eor.l %d1,%d0
	mov.l %d0,(%a0)
	add.w %d2,%a2
	add.w %d3,%a0
	dbr %d7,opMnwl
	br ret8

opSnwr:
	mov.l (%a2),%d0
	ror.l %d6,%d0
	and.l %d5,%d0
	or.l %d0,(%a0)
	add.w %d2,%a2
	add.w %d3,%a0
	dbr %d7,opSnwr
	br ret8

opSnwl:
	mov.l (%a2),%d0
	rol.l %d6,%d0
	and.l %d5,%d0
	or.l %d0,(%a0)
	add.w %d2,%a2
	add.w %d3,%a0
	dbr %d7,opSnwl
	br ret8

opCnwr:
	mov.l (%a2),%d0
	ror.l %d6,%d0
	and.l %d5,%d0
	not.l %d0
	and.l %d0,(%a0)
	add.w %d2,%a2
	add.w %d3,%a0
	dbr %d7,opCnwr
	br ret8

opCnwl:
	mov.l (%a2),%d0
	rol.l %d6,%d0
	and.l %d5,%d0
	not.l %d0
	and.l %d0,(%a0)
	add.w %d2,%a2
	add.w %d3,%a0
	dbr %d7,opCnwl
	br ret8

opXnwr:
	mov.l (%a2),%d0
	ror.l %d6,%d0
	and.l %d5,%d0
	eor.l %d0,(%a0)
	add.w %d2,%a2
	add.w %d3,%a0
	dbr %d7,opXnwr
	br ret8

opXnwl:
	mov.l (%a2),%d0
	rol.l %d6,%d0
	and.l %d5,%d0
	eor.l %d0,(%a0)
	add.w %d2,%a2
	add.w %d3,%a0
	dbr %d7,opXnwl
	br ret8

	set DBR,0x51c8
	set MOVLI,0x2000+074	# mov.l &...,
	set MOVWI,0x3000+074	# mov.w &...,
	set ADDWI,0x0640	# add.w &...,

	set FDFRAG,16	# first destination is a fragment
	set LDFRAG,17	# last destination is a fragment
	set NSHF1,18
	set FD2D,19	# first destination should store 2 words
	set LD2D,20	# last destination should store 2 words
	set FSTORE,21
	set DST1L,24	# dst inner count is 0
	set SRC1L,25	# Nsrc is 2

	set gensiz,80

widtab:
	mov.w %d0,(%a0)+; short 0
	or.w %d0,(%a0)+; short 0
	and.w %d0,(%a0)+; not.w %d0
	eor.w %d0,(%a0)+; short 0

#
#  So far
#	%d7 == h-1 (bits),,w (words)
#	%d6 == 0,,rotate count
#	%d5 == mask2,,mask1
#	%d4 == -RtoL
#	%d3.w == wdst (bytes)
#	%d2.w == wsrc (bytes)
#	%a6.w == 4*fc
#	%a2 -> src
#	%a0 -> dst
#
L85:
	lea.l $widtab(%pc,%a6.w),%a6
	tst.w %d4; bpl.b L300; bset &31,%d6
L300:
	mov.w %d7,%d0		# inner word count
	bne.b L304; bset &DST1L,%d6
L304:
	add.w &1,%d0		# Nsrc = 1+Ninner
	mov.w %d0,%a1		#   + ...
	add.w &1,%d0		# Ndst = 1+Ninner+1
	add.w %d0,%d0		# magnitude of dst addressing side effects
	tst.l %d6; bpl.b L310
	neg.w %d0; add.l &2,%a0		# RtoL
L310:
	sub.w %d0,%d3		# compensate dst for autoincrement

	mov.w %d5,%d4		# mask1
	swap.w %d5		# mask2

	cmp.w %d4,&-1;            beq.b L320; bset &FDFRAG,%d6
L320:

	cmp.w %d5,&-1; seq.b %d1; beq.b L330; bset &LDFRAG,%d6
L330:

	tst.w %d6; bne.b L360	# not NOSHIFT
	add.w &1,%a1		# Nsrc = 1+Ninner+1
	mov.l %d6,%d0; swap.w %d0; ext.w %d0	# 0,,flag bits
	asr.w &1,%d7; roxl.w &1,%d0	# account for inner words odd
	mov.b $nstab(%pc,%d0.w),%d0
	bpl.b L340; add.w &1,%d7
L340:
	add.b %d0,%d0
	bpl.b L350; sub.w &1,%d7
L350:
	swap.w %d0; eor.l %d0,%d6	# the bits
	btst &DST1L,%d6; bne.b L355
	btst &FD2D,%d6; beq.b L410
L355:
	ext.l %d4; bmi.b L410; swap.w %d4; not.w %d4	# NOSHIFT mask1 .l
	br.b L410		# NOSHIFT mask2 .l
nstab:
	byte 0x82,0x80,0x04,0x80	# 0x80: +1 inner;  0x40: -1 inner
	byte 0x02,0x00,0x44,0x00	# 0x04: FD2D;      0x02: NSHF1 no first word
L360:
	ext.w %d1; sub.w %d1,%d7	# extend inner loop

	mov.l &0xf,%d0		# 0  1     7  8  9     e  f
	add.w &8,%d6		# 8  9     f  0  1     6  7
	and.w %d0,%d6
	sub.w &8,%d6		# 0  1     7 -8 -7    -2 -1  X=C= sign
	mov.w %d6,%d1; bge.b L367	#                    X unchanged
	neg.w %d1   		#             8  7     2  1  X=C= 1
L367:
	roxl.w &1,%d1		# 0  2     e 11  f     5  3
	and.w %d0,%d1		# 0  2     e  1  f     5  3
	lsl.w &8,%d1		# magic position
	short ADDWI+001
	  ror.l &8,%d0
	mov.w %d1,%a3		# the rotate instruction

	mov.l &0,%d1; not.w %d1		# 0,,-1
	ror.l %d6,%d1		# where the bits are after a rotate

	mov.w %d1,%d0; and.w %d4,%d0; beq.b L370	# 1 src word covers dst frag
	not.w %d1;     and.w %d4,%d1; beq.b L370
	add.w &1,%a1; br.b L390		# fragment needs another src word
L370:
	sub.w &1,%d7		# .l takes an inner word
	bset &FD2D,%d6
	ext.l %d4; bmi.b L390
	swap.w %d4; not.w %d4	# mask1 .l
L390:

	swap.w %d1

	mov.w %d1,%d0; and.w %d5,%d0; beq.b L400	# 1 src word covers dst frag
	not.w %d1;     and.w %d5,%d1; beq.b L400
	add.w &1,%a1; br.b L420		# fragment needs another src word
L400:
	dbr %d7,L405		# .l takes an inner word
	clr.w %d7; br.b L420	# nothing there to take
L405:
L410:
	bset &LD2D,%d6
	ext.l %d5; bmi.b L420
	swap.w %d5; not.w %d5	# mask2 .l
L420:

	tst.w NREG*4-4+fc+8(%sp); bne.b L430; bset &FSTORE,%d6
L430:
	mov.w %a1,%d0		# Nsrc
	add.w %d0,%d0		# magnitude of src addressing side effects
	tst.l %d6; bpl.b L431
	neg.w %d0; add.l &2,%a2		# RtoL
L431:
	sub.w %d0,%d2		# compensate src for autoincrement

	lea.l -gensiz(%sp),%sp
	mov.l %sp,%a5
	swap.w %d3
	swap.w %d2

	cmp.w %a1,&2; bgt L445
	short MOVWI+00000
	  mov.l (%a2)+,%d0
	tst.l %d6; bpl.b L432; add.w &010,%d0	# RtoL
L432:
	mov.w %d0,(%a5)+
	mov.l &0,%d1; mov.w &-0x1000,%d2; mov.w &0100,%d3
	lea.l $L438(%pc),%a1
	mov.l &-1,%d0		# prepare bits to decide on "swap"
	tst.w %d6; bpl.b L432d; neg.w %d6
	lsl.l %d6,%d0; br.b L432e
L432d:
	lsr.l %d6,%d0
L432e:
	btst &DST1L,%d6; beq.b L434
	bset &FD2D,%d6; bne.b L432a
	ext.l %d4; bmi.b L432a; swap.w %d4; not.w %d4	# mask1 .l
L432a:
	bset &LD2D,%d6; bne.b L432b
	ext.l %d5; bmi.b L432b; swap.w %d5; not.w %d5	# mask2 .l
L432b:
	and.l %d5,%d4; mov.l %d4,%d5	# single .l does it all
	add.l &1,%d4; beq L730		# all 32 bits
	sub.l &1,%d4		# need an "and"
	and.l %d5,%d0
	cmp.l %d5,%d0
	beq.b L432c
	short MOVWI+05300
	  swap.w %d0
L432c:
	tst.w %d6; bne L690	# and a rotate
	br.b L437		# NOSHIFT
L434:
	mov.w %a3,(%a5)+	# the rotate instr
	short MOVWI+05300
	  mov.l %d0,%d1		# copy after rotate
	and.l %d4,%d0
	cmp.l %d4,%d0
	seq.b %d0; neg.b %d0; ext.w %d0
	short ADDWI+000
	  swap.w %d0
	mov.w %d0,(%a5)+
	lea.l $L436(%pc),%a1
	br.b L437
L436:
	  and.w %d4,%d0
	mov.w &01001,%d1; clr.w %d2; clr.w %d3
	lea.l $L438(%pc),%a1
L437:
	br L700
L438:
	  and.w %d5,%d0
	br L545
L445:
#
#  During compilation
#	%d7 == h-1,,w
#	%d6 == flags,,rotate count
#	%d5 == mask2
#	%d4 == mask1
#	%d3 == dst_dW,,bits for xxx.[wl]
#	%d2 == src_dW,,bits for mov.[wl]
#	%d1.w == parity
#	%a6 -> optab
#	%a5 -> next generated instruction
#	%a4 -> top of inner loop
#	%a3.w == rotate instruction
#	%a2 -> src
#	%a1 -> fragment "and" instruction
#	%a0 -> dst
#
	tst.w %d6; bne.b L480	# not NOSHIFT ==> always need first word
	btst &NSHF1,%d6; bne.b L485	# interplay of NOSHIFT, odd, FDFRAG
L480:
	mov.l &1,%d1
	and.w %d7,%d1		# parity of inner word count
	lsl.w &2,%d1		# even ==> frag in %d0, odd ==> frag in %d1
	bsr genwid		# generate for first word
	  and.w %d4,%d0
L485:
	cmp.w %d7,&2; ble.b L490	# inner dbr always falls through
	btst &FSTORE,%d6; beq.b L490	# no conflict "mov field" vs. %d6
	short MOVWI+05300		# init inner count
	  mov.w %a4,%d6
L490:
	mov.l %a5,%a4		# top of inner loop
	asr.w &1,%d7		# check inner word count
	blt.b L540		# single .l does it all
	bcc.b L500		# even
	beq.b L520		# 1
	short MOVWI+05300
	  br.b L500		# jump into middle of inner loop
	add.l &1,%a4		# remember to fixup "br.b"
	add.w &1,%d7		# middle entry ==> no dbr offset
L500:
	beq.b L530		# no inner words at all
	mov.l &4,%d1		# use %d1 in
	bsr.b genwid		# even half of inner loop
	  short 0
L510:
	mov.w %a4,%d0; neg.w %d0
	bclr &0,%d0; beq.b L520
	add.w %a5,%d0; mov.b %d0,(%a4)+		# fixup "br.b" into middle
L520:
	mov.l &0,%d1		# use %d0 in
	bsr.b genwid		# odd half of inner loop
	  short 0
	sub.w &1,%d7		# offset for inner dbr loop
	ble.b L530		# dbr always falls through
	mov.w &DBR+6,(%a5)+
	sub.l %a5,%a4; mov.w %a4,(%a5)+	# dbr displacement
L530:

	btst &LDFRAG,%d6; beq.b L540	# omit "and" for full last word
	mov.l &4,%d1
	bsr.b genwid
	  and.w %d5,%d0
L540:

	tst.w %d7; ble.b L545	# no inner loop
	btst &FSTORE,%d6; bne.b L545	# possible conflict "mov field" vs. %d6
	short MOVWI+05300		# init inner count
	  mov.w %a4,%d6
L545:
	swap.w %d3; tst.w %d3; beq.b L546	# wdst is full width of bitmap
	mov.w %d3,%a1		# dst_dW
	short MOVWI+05300
	  add.w %a1,%a0
L546:
	swap.w %d2; tst.w %d2; beq.b L547	# wsrc is full width of bitmap
	mov.w %d2,%a3		# src_dW
	short MOVWI+05300
	  add.w %a3,%a2
L547:
	mov.w &DBR+7,(%a5)+
	mov.l %sp,%a4		# top of outer loop
	cmp.b (%a4),&0x60; bne.b L548		# not br.b
	mov.b 1(%a4),%d0; ext.w %d0; lea.l 2(%a4,%d0.w),%a4	# collapse branches
L548:
	sub.l %a5,%a4; mov.w %a4,(%a5)+	# dbr displacement
	short MOVWI+05300
	  jmp (%a5)

	mov.w %d7,%a4	# init inner count
	mov.w %d7,%d6	# init inner count, 2nd case
	swap.w %d7   	# h-1
	lea.l $retgen(%pc),%a5
	jmp (%sp)

genwid:
	mov.l (%sp)+,%a1	# -> inline parameter
	mov.l $genget(%pc,%d1.w),%d0
	tst.w %d1; beq.b L550; mov.w &01001,%d1; swap.w %d1	# parity bits
L550:
	clr.w %d2; clr.w %d3	# .[wl] bits default to .w
	tst.l %d6; bpl.b L560; add.w &010,%d0	# RtoL
L560:
	tst.w %d6; bne.b L569	# not NOSHIFT
	bclr &9,%d0		# NOSHIFT always %d0
	mov.w (%a1),%d1; bne.b L564	# not inner loop
	btst &FSTORE,%d6; beq.b L562	# not "mov"
	mov.l &070,%d1; and.w %d0,%d1
	lsl.w &3,%d1; or.w %d1,%d0	# copy RtoL mode
	add.w &-0x1000,%d0		# .w ==> .l
	mov.w %d0,(%a5)+
L561:
	jmp 2(%a1)
genget:
	swap.w %d0; mov.w (%a2)+,%d0
	swap.w %d1; mov.w (%a2)+,%d1

L562:
	mov.w &-0x1000,%d2; mov.w &0100,%d3	# .w +=> .l
	add.w %d2,%d0
L563:
	mov.l &0,%d1	# NOSHIFT always %d0
	br L698		# assemble the fetch, then do the op
L564:
	lsr.w &1,%d1; bcs.b L562	# NOSHIFT always LD2D
	btst &FD2D,%d6; bne.b L562
	br.b L563		# alas, .w
L569:
	mov.w (%a1),%d1; beq.b L630	# inner loop
L570:
	lsr.w &1,%d1; bcs.b L580	# last word
	add.w &-0x1000,%d0		# force fetch .l
	mov.w %d0,(%a5)+		# the fetch .l
	short MOVLI+00000
	  mov.l %d0,%d1
	  swap.w %d0
	clr.w %d1; eor.l %d1,%d0	# parity for mov.l %d[01],%d[10]
	tst.l %d1; sne.b %d1; sub.b %d1,%d0	# parity for swap.w %d[01]
	mov.l %d0,(%a5)		# ran out of registers
	mov.l &0x4c80ec,%d0	# microcoded bits
	tst.l %d6; bpl.b L572; ror.l &1,%d0	# RtoL
L572:
	tst.w %d6; bpl.b L574; ror.l &2,%d0	# rol
L574:
	btst &FD2D,%d6; beq.b L576; ror.l &4,%d0	# first op .l
	mov.w &-0x1000,%d2; mov.w &0100,%d3	# .w +=> .l corrections
L576:
	ror.l &1,%d0; bpl.b L578	# "swap" not needed
	add.l &2,%a5
	ror.l &8,%d0; bpl.b L577	# existing "swap" parity OK
	eor.w &1,(%a5)
L577:
	ror.l &8,%d0; bpl.b L578	# existing order OK
	sub.l &2,%a5
	mov.l (%a5),%d0; swap.w %d0; mov.l %d0,(%a5)
	add.l &2,%a5
L578:
	add.l &2,%a5
	swap.w %d1		# junk,,parity
	br.b L690
L580:
	btst &LD2D,%d6; beq.b L630	# operator .w
	mov.w &-0x1000,%d2		# mov.w +=> mov.l
	mov.w &0100,%d3 		# xxx.w +=> xxx.l
L630:
	tst.l %d6; smi.b %d1
	eor.b %d6,%d1; bpl.b L650	# rotation in same direction as scan
	swap.w %d0		# interchange "swap" and "mov"
L650:
	mov.l %d0,(%a5)+

	swap.w %d1		# junk,,parity
	mov.w (%a1),%d0; lsr.w &1,%d0; bcs.b L660	# last word
	short MOVWI+000
	  mov.l %d0,%d1
	eor.w %d1,%d0
	mov.w %d0,(%a5)+
	br.b L690
L660:
	tst.l %d6; bmi.b L690		# RtoL
	btst &LD2D,%d6; beq.b L690	# not .l
	tst.w %d6; bpl.b L670		# ror
	sub.l &2,%a5; br.b L690		# no "swap"
L670:
	mov.w -4(%a5),(%a5)+		# extra "swap"
L690:
	mov.w %a3,%d0
	eor.b %d1,%d0
L698:
	mov.w %d0,(%a5)+	# the rotate instruction
L700:

	mov.w (%a1),%d0; beq.b L730	# inner loop
	btst &0,%d0; bne.b L705		# last word
	btst &FDFRAG,%d6; beq.b L730	# no "and"
L705:
	add.w %d3,%d0; add.w %d1,%d0; sub.b %d1,%d0	# and.[wl] %d[45],%d[01]
	btst &FSTORE,%d6; beq.b L720
		# "mov" partial word
	swap.w %d0		# save the "and"
	short MOVWI+00000	# ,%d0
	  mov.w (%a0),%d6
	add.w %d2,%d0		# mov.[wl]
	tst.l %d6; bpl.b L710; add.w &020,%d0	# RtoL; "(%a0)" ==> "-(%a0)"
L710:
	mov.w %d0,(%a5)+	# instr to fetch memory part of word
	short MOVWI+00000 	# ,%d0
	  eor.w %d6,%d0
	add.w %d3,%d0; add.b %d1,%d0	# eor.[wl] %d6,%d[01]
	swap.w %d0; mov.l %d0,(%a5)+; swap.w %d0; mov.w %d0,(%a5)+
	mov.w %d2,%d0; add.b %d1,%d0	# mov.[wl] %d[01],
	mov.l &-0100,%d1	# RtoL correction, if necessary
	br.b L770
L720:
	mov.w %d0,(%a5)+	# "and" for non-mov operators
L730:
	mov.w 2(%a6),%d0; beq.b L740	# not F_CLR
	add.w %d3,%d0; add.b %d1,%d0	# not.[wl] %d[01]
	mov.w %d0,(%a5)+
L740:
	btst &FSTORE,%d6; beq.b L790	# non-"mov"
	mov.w %d2,%d0; add.b %d1,%d0	# mov.[wl] %d[01],
	mov.l &0100,%d1 	# RtoL correction, if necessary
L770:
	add.w (%a6),%d0
	tst.l %d6; bpl.b L780
	add.w %d1,%d0 	# RtoL correction
L780:
	mov.w %d0,(%a5)+
	jmp 2(%a1)

L790:
	mov.w %d1,%d0; clr.b %d0; add.w %d3,%d0		# xxx.[wl] %d[01]
	mov.l &010,%d1		# RtoL correction, if necessary
	br.b L770

#
#  During execution
#	%d[01] == rotator
#	%d2 [reserved for texture bits]
#	%d3 [reserved for texture index]
#	%d4 == mask1
#	%d5 == mask2
#	%d6.w == inner count
#	%d7.w == outer count
#	%a0 -> dst
#	%a1 == dst_dW
#	%a2 -> src
#	%a3 == src_dW
#	%a4.w == inner count init
#	%a5 -> retgen
#	%a6 [reserved for -> texture]
#
