
/*
 * Output segment
 */
	.org	1024
seg1:
charsw:
/*	br	o_isbs		/* 010 - BS */
/*	br	o_isht		/* 011 - HT */
/*	br	o_isnl		/* 012 - NL */
/*	br	o_isvt		/* 013 - VT */
/*	br	o_isff		/* 014 - FF */
/*	br	o_iscr		/* 015 - CR */
/*	br	o_sendit	/* 016 - SO */
/*	br	o_sendit	/* 017 - SI */

out:
/*
 * This is equivalent to xmit routine
 */
	CHK(B1_ENT,1)		/* check point */
/*
 * Registers are used as follows:
 *	r15 - flag
 *	r14 - free queue pointer
 *	r13 - bit 7-0 of XADDR
 *	r12 - bit 15-8 of XADDR
 *	r11 - bit 17-16 of XADDR
 *	r9 - chan#
 *	r8/r10 - LTE address
 *	r7 - blklen, pklen
 *	r6,r5 - XLEN
 *	r4 - C_S
 * 	r3 - C_R
 *	r2 - blklen
 *
 */

/*
 * outq is a linked list for all output active channels
 */
	CHK(B1_ERO,1)		/* check point */
	mov	outq,mar
	mov	%outq,%mar
	mov	mem,r2|mar++	/* outq1 = outq */
	mov	r2,mem
o_robin:
/*
 * while (outq1) (pointer in r2)
 */
	brz	o_bye
	QA(r2)
	mov	mem,r2|mar++
	mov	mem,r9
	mov	outq1,mar
	mov	%outq1,%mar
	mov	r2,mem
	GLTE(r9)		/* set up r8/r10 */
/*
 * if (C_S ^ C_R)&DKBMASK), then the chan is blocked
 */
	mov	C_S,brg
	add	brg,r10,mar
	mov	mem,r4|mar++
	xor	mem,r4,brg
	mov	brg,r0
	mov	DKBMASK,brg
	and	brg,r0
	dec	r0,-
	brz	o_endrobin
	mov	mem,r3|mar++
	mov	mem,r2			/* r2 = C_A */
/*
 * r15 is a flag:
 *	bit 0 - C_S == C_R
 *	bit 1 - mlen == 0
 *	bit 4 - send XCNTL
 */
	mov	C_XCNTL,brg
	add	brg,r10,mar
	mov	mem,r0
	dec	r0,-
	brz	1f
	mov	(1<<4),brg
	br	2f
1:	mov	0,brg
2:	mov	brg,r15
/*
 * mlen' (in r1/r2) = (S-A-1)&07 * DKBLOCK
 */
	mov	r2,brg
	addn	brg,r4,brg		/* brg=C_S-C_A-1 */
	mov	brg,r5
	mov	WINDOW,brg
	and	brg,r5
	mov	DKBLOCK,brg
	mov	brg,r6
	TIMES(r5,r6,r2,r1)
/*
 * if (mlen'<XLEN) {mlen=XLEN-mlen'; goto o_while;}
 * else if (mlen'>XLEN) { goto o_endrobin; }
 * else if (mlen'=XLEN) { goto o_cntl; }
 *
 * mlen (r6/5) = XLEN (r6/5) - mlen' (r1/r2)
 */
	mov	C_XLEN,brg
	add	brg,r10,mar
	mov	mem,r6|mar++
	mov	mem,r5|mar++
	mov	r1,brg
	sub	brg,r6
	mov	r2,brg
	subc	brg,r5,r5|brg
	brc	1f
/*
 * XLEN < mlen': no data to go out
 */
	br	o_endrobin
1:
/*
 * see if mlen==0
 */
	or	brg,r6,brg
	mov	brg,r0
	dec	r0,-
	brz	o_cntl		/* send XCNTL, if any */
/*
 * ptr = C_XADDR+mlen'
 */
	add	mem,r1,brg|mar++
	mov	brg,r13
	addc	mem,r2,brg|mar++
	mov	brg,r12
	mov	mem,r11
	adc	r11
o_while:
/*
 * blklen (in r2) = min{mlen, DKBLOCK}
 * mlen -= blklen
 */
	mov	DKBLOCK+1,brg
	sub	brg,r6
	mov	0,brg
	subc	brg,r5
	brc	o_chunk
/*
 * mlen <= DKBLOCK: blklen = mlen; mlen = 0;
 */
	mov	DKBLOCK+1,brg
	add	brg,r6,brg
	mov	brg,r2
	mov	brg,r7
	mov	0,brg
	mov	brg,r6
	mov	brg,r5
	mov	(1<<1),brg
	or	brg,r15
	br	o_blk
o_chunk:
/*
 * mlen > DKBLOCK: blklen = DKBLOCK; mlen -= DKBLOCK;
 */
	inc	r6
	adc	r5
	mov	DKBLOCK,brg
	mov	brg,r2
	mov	brg,r7
o_blk:
/*
 * now wants to send a block with len in r2
 * pklen (r1) = min{r2, DKCHUNK}
 * blklen -= pklen
 */
	mov	DKCHUNK,brg
	sub	brg,r2
	brc	1f
/*
 * r2 < DKCHUNK: r1 = r2; r2 = 0;
 */
	add	brg,r2,brg
	mov	brg,r1
	dec	r1,-
	brz	o_tail		/* go send trailer */
	mov	0,brg
	mov	brg,r2
	br	2f
1:
/*
 * r2 >= DKCHUNK: r1 = DKCHUNK; r2 -= DKCHUNK;
 */
	mov	brg,r1
2:
o_mark:
/*
 * Now send the DKMARK with chan#
 */
	SENDHEAD(r9)
/*
 * output the rest of the data
 * if r13 is odd, get one byte first
 */
	mov	r13,brg
	br0	o_getone
o_gettwo:
/*
 * Get another two bytes of data for this channel
 */
	SGETTWO(r13,r12,r11,o_err_bus)
/*
 * Send two bytes out to CIM
 * if pklen is zero, then issue send-packet command
 * otherwise, go back to get another two bytes
 * Now odh was set to DKDATA
 */
	dec	r1
	BUSWAIT
	SENDDATA(D_WDATA,idl)
	dec	r1		/* pklen-- */
	brz	o_sodd
o_sendhi:
	D422(3)
	SENDDATA(D_WDATA,idh)	/* send the hi byte */
	dec	r1,-		/* pklen-- */
	brz	o_sendpack
	br	o_gettwo
o_getone:
/*
 * get 2 bytes, throw away the low byte
 * send the hi byte
 */
	dec	r13
	mov	0,brg
	subc	brg,r12
	subc	brg,r11		/* back up to even address */
	dec	r1
	GETTWO(r13,r12,r11,o_err_bus)
	br	o_sendhi
o_sodd:
/*
 * hi byte has not been sent yet: dec addr by 1
 */
	mov	0,brg
	dec	r13
	subc	brg,r12
	subc	brg,r11
o_sendpack:
/*
 * if r2 (running blklen) > 0, issue XPACK and goto o_blk
 */
	dec	r2,-
	brz	o_pack2
/*
 * Having loaded a whole packet, now issue the
 * transmit-packet command, i.e. dkcsr=D_XPACK
 */
	CHK(B1_SPK,1)		/* check point */
	D422(2)
	SENDPACK(r1)
	br	o_blk		/* go back to send another packet */
o_pack2:
/*
 * if r7-1 (fixed blklen) & 014 == 014, issue XPACK and send trailer
 *	in the next packet
 * else don't issue XPACK and send trailer in this packet
 */
	dec	r7,brg
	mov	brg,r0
	mov	014,brg
	orn	brg,r0
	brz	1f
	br	o_ta1
1:
	SENDPACK(r1)
	D422(4)
o_tail:
/*
 * just sent a block, now send the trailer
 * MARK+chan# BOT blklen 0 SEQ+C_S
 */
	SENDHEAD(r9)
o_ta1:
	D422(2)
	mov	r15,brg
	br1	o_ta3
o_ta1.5:
	mov	I_BOTM,brg
o_ta2:
	mov	brg,r0
	SENDDATA(D_WCNTL,r0)
	D422(4)
	SENDDATA(D_WDATA,r7)
o_ta2.5:
	D422(4)
	SENDDATA(D_WDATA,ZERO)
	D422(3)
	mov	I_SEQ,brg
	or	brg,r4,brg
	mov	brg,r0
	SENDDATA(D_WCNTL,r0)
	D422(4)
	SENDPACK(r1)
/*
 * C_S++ and set XACK
 */
	inc	r4		/* C_S + 1 */
	mov	WINDOW,brg
	and	brg,r4
	mov	r8,%mar
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	XACK,brg
	or	brg,r0,mem
/*
 * r2 - pklen
 * Now r6/r5 is the real mlen
 * while (mlen && C_S != C_R) {
 */
	mov	r3,brg
	xor	brg,r4,brg
	mov	brg,r0
	mov	DKBMASK,brg
	and	brg,r0
	dec	r0,-
	brz	o_end1
	mov	r15,brg
	br1	o_ta5
	br	o_while
o_ta3:
/*
 * reach end of this xmit buffer
 */
	mov	r15,brg
	br4	o_ta5
o_ta4:
	mov	r8,%mar
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	~XBOTM,brg
	or	brg,r0,-
	brz	1f
	mov	I_BOT,brg
	br	o_ta2
1:
	mov	I_BOTM,brg
	br	o_ta2
o_ta5:
/*
 * check if XCNTL to be sent
 */
	mov	r15,brg
	br4	1f
	br	o_end1		/* no XCNTL */
1:
/*
 * reset bit 4 so that next time around we will not duplicate
 */
	mov	~(1<<4),brg
	and	brg,r15
/*
 * check if blklen<DKBLOCK: send XCNTL in this block
 */
	mov	DKBLOCK,brg
	addn	brg,r7,-
	brz	o_ta1.5		/* send XCNTL in next block */
/*
 * send XCNT in this block: see if in the same chunk
 * if the last chunk size was 12-16, then start a new chunk
 */
	dec	r7,brg
	mov	brg,r0
	mov	~014,brg
	or	brg,r0,-
	brz	o_ta6		/* r7 was 16, 15, 14, or 13 */
	or	brg,r7,-
	brz	o_ta6		/* r7 was 12 */
	br	o_ta8
o_ta6:
/*
 * finish the old chunk, restart with a new chunk
 */
	SENDPACK(r1)
	D422(4)
o_ta7:
	SENDHEAD(r9)
	D422(2)
o_ta8:
	CHK(B1_CNTL,1)
	mov	r8,%mar
	mov	C_XCNTL,brg
	add	brg,r10,mar
	SENDDATA(D_WCNTL,mem)		/* send cntl */
	inc	r7
	br	o_ta4
o_cntl:
/*
 * send XCNTL, if any
 */
	mov	r15,brg
	br4	1f
	br	o_endrobin
1:
/*
 * now send the single XCNTL in a block by itself
 * code should look like:
 *	SENDHEAD(r9)
 *	SENDDATA(D_WCNTL,r1)
 *	mov	I_BOT,brg
 *	mov	brg,r1
 *	SENDDATA(D_WCNTL,r1)
 *	mov	1,r1
 *	SENDDATA(D_WDATA,r1)
 *	br	o_ta2.5
 *
 * the reason we goto o_tail is to save text space and slightly slower
 */
	mov	(1<<1),brg		/* indicate mlen == 0 */
	or	brg,r15
	mov	0,brg			/* is 0, will be 1 later */
	mov	brg,r7
	br	o_ta7
o_done:
/*
 * mlen is 0: we can send CHECK here or do nothing
	mov	I_CHECK,brg
	mov	brg,r1
	CALL(send,s.send)
 */

o_end1:
/*
 * save r4 in C_S
 */
	mov	r8,%mar
	mov	C_S,brg
	add	brg,r10,mar
	mov	r4,mem		/* restore C_S */
o_endrobin:
/*
 * BEWARE issue D_WRITE in the while loop, instead of robin loop.
 * inc r9 and go to robin loop.
 */
	CHK(B1_ENRO,1)		/* check point */
	mov	outq1,mar
	mov	%outq1,%mar
	mov	mem,r2
	br	o_robin
o_bye:
o_disp:
/*
 * return to main dispatch loop
 */
	CHK(B1_EXIT,1)		/* check point */
	mov	%disp2,brg
	mov	brg,pcr
	jmp	disp2
o_err_bus:
/*
 * Unibus request fails to complete within 20 usec
 */
	mov	%err_bus,brg
	mov	brg,pcr
	jmp	err_bus

ii_init:
/*
 * send(r9, I_AINIT);
 */
	mov	I_AINIT,brg
	mov	brg,r1
	CALL(send,s.send)		/* argument in r9, r1 */
	mov	r5,brg		/* r5=0 for init0, r5=1 for init1 */
	br0	i_init1
	br	i_init0
i_init1:
/*
 * set block mode
 * reset TA0-TA3, RSEQ, RBLEN, RULEN
 * release all queues and buffers from C_RB, C_RQ
 */
	CHK(B4_IN1,4)
	mov	r8,%mar
	mov	C_X1,brg
	add	brg,r10,mar
	mov	XBLK,brg
	mov	brg,r0
	or	mem,r0,mem		/* set block mode */
	mov	C_TA0,brg
	add	brg,r10,mar
	mov	0,mem|mar++
	mov	0,mem|mar++
	mov	0,mem|mar++
	mov	0,mem|mar++
	mov	C_RSEQ,brg
	add	brg,r10,mar
	mov	0,mem
	mov	C_RULEN,brg
	add	brg,r10,mar
	mov	0,mem|mar++
	mov	0,mem|mar++		/* RULEN = 0 */
	mov	C_RBLEN,brg
	add	brg,r10,mar
	mov	0,mem|mar++
	mov	0,mem|mar++		/* RBLEN = 0 */
	mov	mem,r2			/* r2 = C_RB */
	brz	i_11
	mov	NIL,mem
	CALL(clean,s.clean)
i_11:
	mov	r8,%mar
	mov	C_RQ,brg
	add	brg,r10,mar
	mov	mem,r2			/* r2 = C_RQ */
	brz	ii_loop
	mov	NIL,mem
	CALL(clean,s.clean)
	br	ii_loop
 
i_init0:
/*
 * set char mode
 */
	CHK(B4_IN0,4)
	mov	r8,%mar
	mov	C_X1,brg
	add	brg,r10,mar
	mov	~XBLK,brg
	mov	brg,r0
	and	mem,r0,mem		/* set char mode */
	br	ii_loop

ii_ainit:
/*
 * S = 1, R = 0, A = 0;
 * X = XACT;
 */
	CHK(B4_AIN,4)
	mov	r8,%mar
	mov	C_S,brg
	add	brg,r10,mar
	mov	1,mem|mar++		/* S = 1 */
	mov	0,mem|mar++		/* R = 0 */
	mov	0,mem			/* A = 0 */
	mov	C_X,brg
	add	brg,r10,mar
	mov	XACT,mem|mar++
	mov	mem,r0			/* C_X1 */
	mov	XNIL,brg
	orn	brg,r0,-
	brz	i_ai5			/* send trailer only */
	mov	C_XLEN,brg
	add	brg,r10,mar
	mov	mem,r0|mar++
	or	mem,r0
	mov	C_XCNTL,brg
	add	brg,r10,mar
	or	mem,r0
	dec	r0,-
	brz	ii_loop
/*
 * this channel is output active: put it in 'outq'
 */
	GETQ(r1,o_noqb)
	mov	NIL,mem|mar++
	mov	r9,mem
	mov	%outq,%mar
	mov	outq,mar
i_ai2:
	mov	mem,r2
	brz	i_ai3
	QA(r2)
	br	i_ai2
i_ai3:
	mov	r1,mem
	br	ii_loop
i_ai5:
/*
 * send trailer-only mode
 */
	CALL(strailer,s.strailer)
	br	ii_loop

ii_req:
/*
 * send I_INIT1
 * X = XINIT;
 */
	CHK(B4_IREQ,4)
	mov	I_INIT1,brg
	mov	brg,r1
	CALL(send,s.send)
	mov	r8,%mar
	mov	C_X,brg
	add	brg,r10,mar
	mov	XINIT,mem
	br	ii_loop

ii_enq:
/*
 * send C_C
 */
	CHK(B3_ENQ,3)
	mov	r8,%mar
	mov	C_C,brg
	add	brg,r10,mar
	mov	mem,r1
	CALL(send,s.send)
ii_check:
/*
 * send ACK+C_RSEQ
 * release C_RB
 */
	CHK(B3_CHK,3)
	mov	r8,%mar
	mov	C_RSEQ,brg
	add	brg,r10,mar
	mov	mem,r1
	mov	I_ACK,brg
	or	brg,r1
	CALL(send,s.send)
	mov	r8,%mar
	mov	C_RB,brg
	add	brg,r10,mar
	mov	mem,r2
	brz	ii_loop
	mov	NIL,mem
	CALL(clean,s.clean)
	mov	%i_ebmode,brg
	mov	brg,pcr
	jmp	i_ebmode

ii_eoi:
/*
 * if TA0==SOI and TA1==2, then report KS_EOI
 * TA0,TA1 = 0;
 */
	CHK(B4_EOI,4)
	mov	r8,%mar
	mov	C_TA0,brg
	add	brg,r10,mar
	mov	mem,r7
	mov	0,mem|mar++
	mov	mem,r6
	mov	0,mem|mar++
	mov	I_SOI,brg
	addn	brg,r7,-
	brz	1f
	br	ii_loop
1:
	mov	2,brg
	addn	brg,r6,-
	brz	1f
	br	ii_loop
1:
/*
 * report 2 bytes in TA2, TA3
 */
	mov	mem,r7|mar++	/* TA2 */
	mov	mem,r6		/* TA3 */
	mov	%repinfo,%mar
	mov	repinfo,mar
	mov	KS_EOI,mem|mar++
	mov	r7,mem|mar++	/* lo byte */
	mov	r6,mem|mar++	/* hi byte */
	br	ii_rpt		/* goto CALL(report,s.report) */

ii_tdkcl:
/*
 * rcv a tdk control char, report tdk cntl
 */
	mov	repinfo,mar
	mov	%repinfo,%mar
	mov	KS_CNTL,mem|mar++
	mov	0,mem|mar++
	mov	0,mem|mar++
	mov	r1,mem|mar++
ii_rpt:
	CALL(report,s.report)
ii_loop:
/*
 * return to i_loop
 */
	mov	%i_loop,brg
	mov	brg,pcr
	jmp	i_loop
o_noqb:
/*
 * run out of queues and buffers
 */
	mov	%noqb,brg
	mov	brg,pcr
	jmp	noqb

endseg1:


