#
# device-independent boot block
# uses the rom device driver
# reads an already-de-headered, small file with predetermined name
# into the beginning of memory
# and executes it
#

	.set	HIADDR,0x70000	# high address at which we run

	.set	REGMASK,0x6e	# registers to save for second boot
	.set	SR1,0		# offsets in save area
	.set	SR3,2*4
	.set	DRIVER,4*4	# where device driver addr gets saved

	.set	BSIZE, 512	# size of a disk block
	.set	ISIZE, 64	# size of an i-node
	.set	INOPB, BSIZE/ISIZE
	.set	ROOTINO, 2	# i-number of the root
	.set	ILBLK, 2	# first block of i-list
	.set	NDIREC, 10	# number of direct blocks
	.set	I_ADDR, 12	# offset to addresses in inode
	.set	DIRSIZ, 14	# chars in filename

	.set	TXCS,34		# console device csr
	.set	TXDB,35		# console device data buffer
	.set	TRDY_B,7	# bit number of ready bit in TXCS

#
# main code
# relocate ourselves to high memory
# read the root directory, and search it for
# the desired file
# read that file in
#

beg:
	nop;nop;nop;nop		# space required by dec bootstrap
	nop;nop;nop;nop
	nop;nop;nop;nop
start:
	movl	$HIADDR,sp	# set up new stack
	pushr	$REGMASK	# save important boot registers
	movl	sp,r11		# remember where they are
	movc3	$end,beg,*$HIADDR	# copy us up
	jmp	*$HIADDR + strel	# and jump up there
strel:
#
# get the root i-node
#
	movl	$ROOTINO,r0
	bsbb	iget
#### check is directory?
	subl2	$NDIREC*4,sp		# make room for block pointers
	movl	sp,r10
	bsbw	addrcpy

#
# search the root directory
#

	clrl	r12			# init block pointer
dirblk:
	movl	r12,r0
	clrl	r5			# use beginning of mem as buffer
	bsbw	lread
	blbs	r0,dirgot
	 movab	notfound,r0
	 bsbw	putstr
direof:	 brb	direof

notfound: .asciz "no boot\r\n"

dirgot:
	clrl	r9
dirent:
	cmpl	r9,$FSBSIZE		# done with this block?
	blss	dirok			# no
	incl	r12			# yes, get next one
	brb	dirblk
dirok:
	tstw	(r9)			# empty entry?
	beql	dirno			# yes, skip it
	cmpc3	$DIRSIZ,2(r9),bootname	# desired name?
	beql	diryes			# yep
dirno:
	addl2	$DIRSIZ+2,r9
	brb	dirent

#
# found the file
# fetch its i-node and read it in
#

diryes:
	movzwl	(r9),r0			# i-number
	bsbb	iget
##### check i-node?
	bsbb	addrcpy
	clrl	r12
rdloop:
	movl	r12,r0
	mull3	$FSBSIZE,r12,r5
	bsbb	lread
	blbc	r0,done
	incl	r12
	brb	rdloop

#
# read it in
# reset registers and start it
#

done:
	movl	r11,sp
	popr	$REGMASK
	movl	r5,r11		# put boot flags in stupid bky place
	movzbl	$DEVNUM,r10		# more berkeley bullshit - 7 == uda50
	jmp	*$2		# skip register mask & start it

#
# fetch an i-node
# r0 has the i-number
# on exit, r0 points to the i-node in core
# address 0 is used as a buffer
#

iget:
	decl	r0			# i-numbers are 1 based
	clrl	r1			# make a quadword
	ediv	$INOPB,r0,r8,-(sp)	# r8 == block; (sp) == offset
	addl2	$ILBLK*FSBSIZE/BSIZE,r8
	clrl	r5			# into address 0
	bsbb	bread
	mull3	(sp)+,$ISIZE,r0		# point to i-node
	rsb

#
# copy/convert block numbers
# out of an i-node
# r10 points to where we want them
# r0 points to the i-node
# for now, only the direct blocks
#
# r0 is destroyed
#

addrcpy:
	movl	r10,r1			# make a volatile copy
	clrl	r2			# counter
	addl2	$I_ADDR,r0		# point to addresses
ad0:
	movw	(r0)+,(r1)+
	movb	(r0)+,(r1)+
	clrb	(r1)+
	aoblss	$NDIREC,r2,ad0
	rsb

#
# read a (filesystem) block from a file
# (filesystem) block number in r0
# buffer address in r5
# r10 points to the addresses
# returns status in r0:
# low bit set if read a block
# clear if error or block doesn't exist
#

lread:
	cmpl	r0,$NDIREC
	blss	lr0
lerr:
	clrl	r0
	rsb
lr0:
	movl	(r10)[r0],r8
	beql	lerr
	mull3	$FSBSIZE/BSIZE,r8,-(sp)	# fs block to disk block
	pushl	$FSBSIZE/BSIZE	# count of disk blocks within fs block
lrlp:
	movl	4(sp),r8
	bsbb	bread
	addl2	$BSIZE,r5
	incl	4(sp)
	sobgtr	(sp),lrlp
	addl2	$8,sp
	rsb

#
# read a block from the disk
# using the rom subroutine
# block number in r8;
# buffer address in r5
# no return if it failed
#

bread:
	movq	SR1(r11),r1	# device address stuff
	movl	SR3(r11),r3	# unit number
	pushl	r5		# duplicate address for driver routine
	jsb	*DRIVER(r11)	# and call driver
	blbc	r0,berr
	movl	(sp)+,r5	# fixup stack
	rsb

berr:
	movab	diskerr,r0
	bsbb	putstr
berr0:	brb	berr0

diskerr: .asciz "disk error\r\n"

#
# print a null-terminated string on the console
# string address in r0
#

putstr:
	movb	(r0)+,r1
	bneq	ps0
	 rsb
ps0:
	mfpr	$TXCS,r2
	bbc	$TRDY_B,r2,ps0
	mtpr	r1,$TXDB
	brb	putstr

#
# miscellaneous stuff
#

bootname:
	.byte	'b,'o,'o,'t,0,0,0,0,0,0,0,0,0,0

end:
