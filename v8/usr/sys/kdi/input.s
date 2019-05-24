
/*
 * Input Segment
 */
	.org	2048
seg2:
in:
	CHK(B2_ENT,2)		/* check point */
	SENDCMD(D_READ)		/* for dr11-c version */

i_csrtst:
/*
 * While there is data in the receiver fifo
 * copy it out
 * While (dkcsr & DKRDONE) {
 */
	READDATA(i_disp)
/*
 * Scan for the 1st word of a packet where a channel #
 * in the low byte and DKMARK in the hi byte
 * If not the packet marker, just throw it away and repeat
 */
	mov	ILO,r9		/* read chan # */
	mov	IHI,brg		/* read hi byte and strobe */
	br1	i_mark
	DELAY(3)
	br	i_csrtst
 
i_mk4:
/*
 * a data in odl left over from last chan# (r9)
 */
	CALL(copy1,s.copy1)
	br	i_mk7
i_mk5:
/*
 * for char mode, check for timeout
 */
	CALL(cktimer,s.cktimer)
	br	i_mk8
i_mk6:
/*
 * see a DKMARK again
 * r13 and r12 can't be destroyed elsewhere e.g. 'rcv'
 */
	mov	r1,brg
	addn	brg,r9,-
	brz	i_lp0
	mov	r12,brg
	br0	i_mk4		/* copy the left-over byte in odl */
i_mk7:
/*
 * if the previous channel was in char mode and timeout mode,
 * then do timeout processing
 */
	asl	r13,brg
	br7	i_mk5		/* char mode */
i_mk8:
	mov	r1,brg
	mov	brg,r9
i_mark:
/*
 * We found the 1st word, the channel # is in r9
 * If the channel # is 377 (all 1's), that means
 * probably power failed or module fell out of bin
 */
	dec	r9,-
	brz	i_incomp
	mov	CHANMODS,brg
	sub	brg,r9,-	/* chan# - CHANMODS */
	brc	i_incomp
	GLTE(r9)		/* set up r8/r10 */
/*
 * r13: a global flag in input.s
 * bit 7 - RBLEN > 0 (or C_RB != NULL)
 * bit 6 - char mode
 * bit 4 - C_TA0 != 0
 * bit 1 - RLEN == 0
 * bit 0 - RADDR is odd
 * r12: a global flag in input.s
 * bit 0 - odl has the 1st byte of a pair
 * r15: report mode
 */
	mov	0,brg
	mov	brg,r13
	mov	brg,r12
	mov	C_RB,brg
	add	brg,r10,mar
	mov	mem,brg
	brz	1f
	mov	(1<<7),brg
	or	brg,r13
1:
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	XBLK,brg
	orn	brg,r0,-
	brz	1f
	mov	(1<<6),brg
	or	brg,r13
1:
	mov	C_TA0,brg
	add	brg,r10,mar
	mov	mem,r0
	dec	r0,-
	brz	1f
	mov	(1<<4),brg
	or	brg,r13
1:
	mov	C_RLEN,brg
	add	brg,r10,mar
	mov	mem,r0|mar++
	or	mem,r0,r0|mar++
	dec	r0,-
	brc	1f
	mov	(1<<1),brg
	or	brg,r13
1:
	mov	mem,brg
	br0	1f
	br	i_loop
1:
	mov	(1<<0),brg
	or	brg,r13
i_lp0:
/*
 * add possible delay for null byte
 */
	DELAY(0)
	D422(1)
i_loop:
/*
 * main loop to read the rest of the packet
 * see if a control envelop or data envelop
 */
	D422(0)
	READDATA(i_id6)
	mov	ILO,r1
	mov	IHI,brg
	br1	i_mk6		/* see DKMARK again */
	br0	i_data
/*
 * A good control envelop is detected for chan # (in r9)
 * control data in r1
 * if 1 <= r1 <= 010, interface module cntl
 * if 010 <= r1 <= 077, protocol cntl
 * if 0100 <= r1 <= 0177, user cntl
 * if r1 >= 0200, supervision cntl
 */
	dec	r1,-
	brz	i_lp0		/* null byte */
	CHK(B2_CNTL,2)		/* check point */
/*
 * if r12.0 call copy1, adjust r13
 */
	mov	r12,brg
	br0	i_lp4
i_lp2:
	mov	r1,brg
	br7	i_tdkcl
	mov	0277,brg	/* 6-th bit is not set */
	or	brg,r1,brg
	brz	i_ucntl
/*
 * this is a protocol cntl char
 * r5 = r1 & WINDOW
 * r0 = r1 & 0370
 */
	mov	0370,brg
	and	brg,r1,brg
	mov	brg,r0
	mov	WINDOW,brg
	and	brg,r1,brg
	mov	brg,r5
/*
 * switch (r0) {
 *	case .....
 */
	mov	I_SEQ,brg
	addn	brg,r0,-
	brz	i_seq
	mov	I_ECHO,brg
	addn	brg,r0,-
	brz	i_echo
	mov	I_ACK,brg
	addn	brg,r0,-
	brz	i_ack
	mov	I_BOT,brg
	addn	brg,r0,-
	brz	i_050
	mov	I_REJ,brg
	addn	brg,r0,-
	brz	i_rej
	mov	I_AINIT,brg
	addn	brg,r1,-
	brz	i_ainit
	mov	I_INIT1,brg
	addn	brg,r1,-
	brz	i_init
	mov	I_INIT0,brg
	addn	brg,r1,-
	brz	i_init
i_umeta:
/*
 * un-recognized meta char
 */
/*
	ERROR(E_UMETA)
 */
	br	i_loop
i_tdkcl:
/*
 * code overflow to output.s
 */
	mov	%ii_tdkcl,brg
	mov	brg,pcr
	jmp	ii_tdkcl

i_ucntl:
/*
 * rcv a user cntl char
 * if (r12.0) then copy odl to raddr first
 * set idh to 0 and goto id9
 */
	CHK1(B2_SUP,2)
	mov	0,brg
	mov	brg,idh			/* indicate cntl */
	br	i_id9			/* copy to C_RB */

i_lp4:
	CALL(copy1,s.copy1)
	br	i_lp2

i_050:
	mov	sw050,brg
	br	(add,brg,r5),%sw050
sw050:
	br	i_bot
	br	i_botm
	br	i_bots
	br	i_soi
	br	i_eoi
	br	i_enq
	br	i_check
	br	i_initreq
i_init:
/*
 * code overflow to output.s
 */
	mov	%ii_init,brg
	mov	brg,pcr
	jmp	ii_init
i_ainit:
/*
 * code overflow to output.s
 */
	mov	%ii_ainit,brg
	mov	brg,pcr
	jmp	ii_ainit
i_initreq:
/*
 * code overflow to output.s
 */
	mov	%ii_req,brg
	mov	brg,pcr
	jmp	ii_req
i_enq:
/*
 * code overflow to output.s
 */
	mov	%ii_enq,brg
	mov	brg,pcr
	jmp	ii_enq
i_check:
/*
 * code overflow to output.s
 */
	mov	%ii_check,brg
	mov	brg,pcr
	jmp	ii_check

i_bot:
i_botm:
i_bots:
i_soi:
/*
 * TA0 = r1; TA1 = 0;
 */
	CHK(B4_BOT,4)
	mov	r8,%mar
	mov	C_TA0,brg
	add	brg,r10,mar
	mov	r1,mem|mar++
	mov	0,mem
	mov	(1<<4),brg
	or	brg,r13
	br	i_loop
i_eoi:
/*
 * code overflow to output.s
 */
	mov	%ii_eoi,brg
	mov	brg,pcr
	jmp	ii_eoi

i_rej:
/*
 * if XENQ is set, then treat this REJ as ECHO
 * otherwise ignore this REJ
 */
	mov	r8,%mar
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	~XENQ,brg
	or	brg,r0,-
	brz	i_ec0		/* XENQ is set, no ECHO was rcv */
	br	i_loop		/* ignore this REJ */
i_echo:
/*
 * R = r5;
 * if ((S-R-1)&07 < (S-A-1)&07) {
 *	len1 = r5 - A;
 *	X &= ~XREJ; A = r5;
 *	if (len1==0) goto i_rej1;
 *	len2 = (len1)*DKBLOCK;
 *	......
 * }
 */
	CHK(B3_ECHO,3)
	mov	r8,%mar
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	~XENQ,brg
i_ec0:
	and	brg,r0,mem	/* reset XENQ */
	mov	C_S,brg
	add	brg,r10,mar
	mov	mem,r3
	mov	mem,r4|mar++		/* r4, r3 = S */
	mov	r5,mem			/* R = r5 */
	addn	mem,r4,r4|mar++		/* r4 = S-R-1 */
	addn	mem,r3			/* r3 = S-A-1 */
	mov	WINDOW,brg
	and	brg,r4
	and	brg,r3,brg
	sub	brg,r4,-
	brc	i_rej1			/* (S-R-1) >= (S-A-1) */
i_gack:
	mov	mem,r2			/* r2 = old A */
	mov	r5,mem			/* A = r5 */
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	~XREJ,brg
	and	brg,r0,mem		/* X &= ~XREJ */
/*
 * if XLEN==0 and XCNTL==0, xmit is inactive, discard this ECHO/REJ
 */
	mov	C_XLEN,brg
	add	brg,r10,mar
	mov	mem,r0|mar++
	or	mem,r0
	dec	r0,-
	brc	1f
	mov	C_XCNTL,brg
	add	brg,r10,mar
	mov	mem,r0
	dec	r0,-
	brz	i_xnil			/* check if trailer only mode */
1:
	mov	r2,brg
	sub	brg,r1,brg
	mov	brg,r2			/* r2 = r1 - old A */
	mov	WINDOW,brg
	and	brg,r2			/* r2 = len1 */
	dec	r2,-
	brz	i_rej1
	mov	DKBLOCK,brg
	mov	brg,r3
	TIMES(r2,r3,r7,r6)		/* r7/6 (len2) = r2 * r3 */
/*
 * if (len2>XLEN) { XLEN=0; report; }
 * else if (len2=XLEN) { XLEN=0; report if ~XCNTL; }
 * else if (len2<XLEN) { XLEN -= len2; XADDR =+ len2; }
 */
	mov	C_XLEN,brg
	add	brg,r10,mar
	mov	mem,r3
	mov	r6,brg
	sub	brg,r3,mem
	mov	mem,r0|mar++
	mov	mem,r4
	mov	r7,brg
	subc	brg,r4,mem
	brc	i_ec2		/* XLEN >= len2 */
i_ec1:
/*
 * This chan has done the transmission
 * remove chan# from 'outq' and report
 */
	mov	C_XLEN,brg
	add	brg,r10,mar
	mov	0,mem|mar++
	mov	0,mem
	mov	C_XCNTL,brg
	add	brg,r10,mar
	mov	0,mem
	mov	%repinfo,%mar
	mov	repinfo,mar
	mov	KS_SEND,mem
	CALL(report,s.report)
/*
 * remove chan# from 'outq'
 * r2: running ptr of outq, r3: next ptr in outq, r4: ptr of outq1
 */
	mov	outq,mar
	mov	%outq,%mar
	mov	mem,r2
0:
	brz	6f
	QA(r2)
	mov	mem,r3
	mov	NIL,mem|mar++
	addn	mem,r9,-
	brz	5f
	mov	outq1,mar
	mov	%outq1,%mar
1:
	mov	mem,r4
	brz	2f
	QA(r4)
	br	1b
2:	/* put r2 at the end of outq1 */
	mov	r2,mem
3:	/* put r3 to r2 */
	mov	r3,brg
	mov	brg,r2
	br	0b
5:	/* delete this chan# */
	FREEQ(r2)
	br	3b
6:	/* restore outq from outq1 */
	mov	outq1,mar
	mov	%outq1,%mar
	mov	mem,brg
	mov	outq,mar
	mov	%outq,%mar
	mov	brg,mem|mar++
	mov	NIL,mem
	mov	r8,%mar
	br	i_rej1
i_ec2:
/*
 * len2 <= XLEN
 */
	or	mem,r0,r0|mar++
	dec	r0,-
	brc	i_ec3		/* len2 < XLEN */
/*
 * len2 = XLEN: check if XCNTL is user cntl
 */
	mov	C_XCNTL,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	0300,brg
	and	brg,r0
	mov	0100,brg
	addn	brg,r0,-
	brz	i_rej1		/* don't report */
	br	i_ec1
i_ec3:
/*
 * len2 < XLEN
 * C_XADDR += len2;
 */
	mov	0,r0
	add	mem,r6,mem|mar++
	addc	mem,r7,mem|mar++
	addc	mem,r0,mem
i_rej1:
/*
 * if REJ && (X&XREJ==0)
 */
	mov	~I_REJ,brg
	or	brg,r1,brg
	brz	1f
	br	i_loop
1:
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	~XREJ,brg
	or	brg,r0,-
	brz	i_loop		/* if REJ had been rcv, ignore this REJ */
i_grej:
/*
 * rcv a REJ: S=(r5+1)&WINDOW, X |= XREJ;
 */
	CHK1(B3_REJ,3)
	inc	r5
	mov	C_S,brg
	add	brg,r10,mar
	mov	WINDOW,brg
	and	brg,r5,mem
	mov	C_X1,brg
	add	brg,r10,mar
	mov	XREJ,brg
	mov	brg,r0
	or	mem,r0,mem
/*
 * if XNIL, call strailer, else loop
 */
	mov	~XNIL,brg
	or	brg,r0,-
	brz	1f
	br	i_loop
1:
	CALL(strailer,s.strailer)
	br	i_loop
i_xnil:
/*
 * if XNIL, report completion, else loop
 */
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	XNIL,brg
	orn	brg,r0,-
	brz	1f
	br	i_loop			/* rcv suspicious ECHO */
1:	xor	brg,r0,mem		/* reset XNIL bit */
	br	i_ec1			/* report KS_SEND */

i_ack:
/*
 * if (r5!=A) goto i_gack;
 * if ((X&XREJ) == 0)) goto i_grej;
 * break;
 */
	CHK(B3_ACK,3)
	mov	r8,%mar
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r4
	mov	~XACK,brg
	and	brg,r4,mem
	mov	C_A,brg
	add	brg,r10,mar
	addn	mem,r5,-
	brz	1f
	br	i_gack
1:
	mov	(XREJ|XACK),brg
	and	brg,r4
	dec	r4,-
	brz	i_grej
	br	i_loop

i_seq:
/*
 * pseq = ECHO + r5;
 * check if char or block mode processing
 * r13.6 == 0 block mode, else char mode
 */
	CHK(B3_SEQ,3)
	mov	0,brg
	mov	brg,r15		/* report mode */
	mov	r8,%mar
	asl	r13,brg		/* bit 6 of r13 is char mode */
	br7	i_seq2
/*
 * BLOCK mode:
 * check if trailer is well formed and len is correct:
 */
	mov	C_TA0,brg
	add	brg,r10,mar
	mov	mem,r7|mar++
	mov	mem,r6|mar++
	dec	r7,-
	brz	i_nwwt
	mov	I_SOI,brg
	addn	brg,r7,-
	brz	i_nwwt
	mov	2,brg
	addn	brg,r6,-
	brz	1f
	br	i_nwwt
1:
	mov	mem,r0|mar++	/* TA2 */
	mov	mem,r6		/* TA3 */
	mov	C_RBLEN,brg
	add	brg,r10,mar
	mov	mem,r3|mar++
	mov	mem,r2
	mov	C_RULEN,brg
	add	brg,r10,mar
	add	mem,r3,brg|mar++
	addc	mem,r2
	sub	brg,r0
	mov	r2,brg
	subc	brg,r6,brg
	or	brg,r0
	dec	r0,-
	brc	i_nwwt
	mov	C_RSEQ,brg
	add	brg,r10,mar
	mov	mem,r0
	inc	r0
	mov	WINDOW,brg
	and	brg,r0,brg
	addn	brg,r5,-
	brz	i_sq2.1
	br	i_nwwt
i_seq2:
/*
 * has rcv a good block
 * C_RSEQ = r5;
 */
	mov	C_RSEQ,brg
	add	brg,r10,mar
i_sq2.1:
	mov	r5,mem
/*
 * if (RB==NIL & RQ==NIL & (RLEN!=0 | RULEN!=0)), echo the seq# now
 */
	mov	C_RQ,brg
	add	brg,r10,mar
	mov	mem,r2
	mov	C_RB,brg
	add	brg,r10,mar
	and	mem,r2
	brz	1f
	br	i_sq2.3
1:	mov	C_RLEN,brg
	add	brg,r10,mar
	mov	mem,r2|mar++
	or	mem,r2
	mov	C_RULEN,brg
	add	brg,r10,mar
	or	mem,r2,r2|mar++
	or	mem,r2
	dec	r2,-
	brz	i_sq2.2
	mov	I_ECHO,brg
	or	brg,r5,brg
	mov	brg,r1
	CALL(send,s.send)
	mov	r8,%mar
	mov	C_C,brg
	add	brg,r10,mar
	mov	r1,mem
	asl	r13,brg
	br7	i_loop			/* go if char mode */
/*
 * if TA0 (r7) != BOTM, interrupt with SBLOCK
 */
	mov	I_BOTM,brg
	addn	brg,r7,-
	brz	i_sq2.8
/*
 * see if YBLOCK is set
 */
	mov	C_Y,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	YBLOCK,brg
	orn	brg,r0,-
	brz	1f
	br	i_sq2.8
1:
/*
 * interrupt RCV completion with SBLOCK
 */
	mov	SBLOCK,brg
	or	brg,r15
	br	i_sq2.8
i_sq2.2:
	mov	C_RB,brg
	add	brg,r10,mar
i_sq2.3:
/*
 * put seq# at the end of C_RB
 * mar/%mar now points to C_RB
 */
	mov	mem,r2
	brz	i_sq9.5
i_sq2.4:
/*
 * block mode if r13.6==0
 * at least part of the data is in stage-1 (RB: r2) buffer
 * if TA0 (r7) != BOTM, pseq = I_SEQ + r5
 */
	mov	mem,r3
1:
	QA(r3)
	mov	mem,r3|mar++
	brz	2f
	br	1b
2:
	mov	mem,mem|mar++		/* plen */
	mov	mem,mem|mar++		/* *pdata */
	asl	r13,brg
	br7	i_sq9.3			/* char mode */
/*
 * block mode
 */
	mov	I_BOTM,brg
	addn	brg,r7,-
	brz	1f
	mov	I_SEQ,mem
	br	2f
1:	mov	I_ECHO,mem
2:	or	mem,r5,mem		/* pseq */
/*
 * put this queue (r2) to end of C_RQ
 */
	mov	r8,%mar
	mov	C_RQ,brg
	add	brg,r10,mar
1:
	mov	mem,r0
	brz	2f
	QA(r0)
	br	1b
2:
	mov	r2,mem
	mov	r8,%mar
i_sq2.8:
/*
 * if RULEN==0, no data have been copied to RADDR
 */
	mov	C_RULEN,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	0,mem|mar++
	or	mem,r0
	mov	0,mem
	dec	r0,-
	brz	i_seq3
/*
 * if RLEN==0, interrupt RCV completion
 */
	mov	C_RLEN,brg
	add	brg,r10,mar
	mov	mem,r0|mar++
	or	mem,r0
	dec	r0,-
	brc	i_seq3
	mov	SFULL,brg
	or	brg,r15
i_seq3:
	dec	r15,-
	brz	i_seq6
/*
 * report RCV completion
 */
	CHK1(B2_REP,2)
	mov	C_RLEN,brg
	add	brg,r10,mar
	mov	mem,r2
	mov	0,mem|mar++
	mov	mem,r3
	mov	0,mem
	mov	%repinfo,%mar
	mov	repinfo,mar
	mov	KS_RDB,mem|mar++
	mov	r2,mem|mar++
	mov	r3,mem|mar++
	mov	mem,mem|mar++		/* cntl char */
	mov	r15,mem			/* mode */
	CALL(report,s.report)
i_seq5:
	mov	r8,%mar
i_seq6:
/*
 * if C_RQ, call 'rcv' (ignore char mode here)
 */
	mov	C_RQ,brg	/* block mode */
	add	brg,r10,mar
	mov	mem,brg
	brz	i_ebmode
	mov	%rcvsave,%mar
	mov	rcvsave,mar
	mov	r13,mem|mar++
	mov	r12,mem
	CALL(rcv,s.rcv)
	mov	rcvsave,mar
	mov	%rcvsave,%mar
	mov	mem,r13|mar++
	mov	mem,r12
i_ebmode:
/*
 * TA0, TA1 = 0;
 * C_RB = NIL;
 * RBLEN == 0;
 * only RLEN may be 0 to turn on the bit in r13
 */
	mov	(1<<1),brg
	and	brg,r13
	mov	r8,%mar
	mov	C_TA0,brg
	add	brg,r10,mar
	mov	0,mem|mar++
	mov	0,mem
	mov	C_RBLEN,brg
	add	brg,r10,mar
	mov	0,mem|mar++
	mov	0,mem|mar++
	mov	NIL,mem
	br	i_loop
i_sq9.3:
/*
 * char mode: put I_ECHO in RB
 */
	mov	I_ECHO,brg
	or	brg,r5,mem		/* pseq */
	br	i_loop
i_sq9.5:
/*
 * C_RB was nil, now get a queue and put seq# in it
 */
	GETQ(r2,i_noqb)
	mov	NIL,mem|mar++	/* pnext */
	mov	0,mem|mar++	/* plen */
	mov	NIL,mem|mar++	/* pdata */
	mov	I_ECHO,brg
	or	brg,r5,mem	/* pseq */
	mov	r8,%mar
	mov	C_RB,brg
	add	brg,r10,mar
	mov	r2,mem
	br	i_sq2.4
i_nwwt:
/*
 * rcv a not well formed trailer
 * FREE C_RB and its buffer (*pdata)
 * if TC0==BOTS RSEQ=r1&WINDOW;
 * RSEQ is stored in r7
 */
	CHK1(B2_NWWT,2)
	mov	C_RB,brg
	add	brg,r10,mar
	mov	mem,r2
	brz	i_nw3
	mov	NIL,mem
	CALL(clean,s.clean)
	mov	r8,%mar
i_nw3:
	mov	C_TA0,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	C_RSEQ,brg
	add	brg,r10,mar
	mov	I_BOTS,brg
	addn	brg,r0,-
	brz	1f
	mov	I_REJ,brg
	br	2f
1:
	mov	r5,mem
	mov	I_ECHO,brg
2:
	mov	mem,r1
	or	brg,r1
/*
 * if (RULEN>0) {
 *	RADDR -= RULEN; RLEN += RULEN; RULEN = 0;
 * }
 */
	mov	C_RULEN,brg
	add	brg,r10,mar
	mov	mem,r7
	mov	0,mem|mar++
	mov	mem,r6|brg
	mov	0,mem
	or	brg,r7,brg
	mov	brg,r0
	dec	r0,-
	brz	i_nw6
	mov	C_RLEN,brg
	add	brg,r10,mar
	mov	mem,r4
	mov	r7,brg
	add	brg,r4,mem|mar++
	mov	mem,r4
	mov	r6,brg
	addc	brg,r4,mem|mar++
	mov	mem,r4
	mov	r7,brg
	sub	brg,r4,mem|mar++	/* RADDR */
	mov	mem,r4
	mov	r6,brg
	subc	brg,r4,mem|mar++
	mov	mem,r4
	mov	0,brg
	subc	brg,r4,mem
/*
 * turn off all bits in r13 except char mode (1<<6) bit
 * if char mode, should not receive trailer at all
 * hence, it is possible to mov brg,r13 here
 */
	mov	(1<<6),brg
	and	brg,r13			/* turn off all except bit 6 */
/*
 * if RQ is nil, send (REJ+RSEQ) : RSEQ in r1
 */
	mov	C_RQ,brg
	add	brg,r10,mar
	mov	mem,brg
	brz	1f
	br	i_nw6
1:
	mov	C_C,brg
	add	brg,r10,mar
	mov	r1,mem
	CALL(send,s.send)
	br	i_ebmode
i_nw6:
/*
 * RULEN==0 or RQ!=NIL
 * put (REJ+RSEQ) in C_RQ
 * r13 will adjusted in ebmode
 */
	GETQ(r2,i_noqb)
	mov	NIL,mem|mar++		/* pnext */
	mov	0,mem|mar++		/* plen */
	mov	NIL,mem|mar++		/* *pdata */
	mov	r1,mem			/* pseq (REJ+RSEQ) */
	mov	r8,%mar
	mov	C_RQ,brg
	add	brg,r10,mar
1:
	mov	mem,r0
	brz	2f
	QA(r0)
	br	1b
2:
	mov	r2,mem
	br	i_ebmode

i_data:
/*
 * get a data byte: may be normal data, len, or express data in SOI
 * r13:	bit 7- rblen > 0
 *	bit 6 - char mode
 *	bit 4- c_ta0 != 0
 *	bit 1- rlen == 0
 *	bit 0- raddr is odd
 * r12: bit 0- 1st byte of a pair is in odl
 *	note that if odl has the 1st byte, rlen must be >= 2
 */
	dec	r13,-
	brc	i_id4		/* special treatment */
i_id1:
	mov	r12,brg
	br0	i_id3
/*
 * test if rlen >= 2
 */
	mov	r8,%mar
	mov	C_RLEN,brg
	add	brg,r10,mar
	mov	2,r2|brg
	mov	mem,r3
	sub	brg,r3,mem|mar++
	mov	mem,r3
	mov	0,r0|brg
	subc	brg,r3,mem|mar++	/* mar points to RADDR */
	brc	i_id2			/* rlen >= 2 */
/*
 * rlen <= 1
 */
	mov	(1<<1),brg		/* turn on no rcv pending */
	or	brg,r13
	mov	C_RLEN,brg
	add	brg,r10,mar
	add	mem,r2
	mov	r2,mem|mar++
	mov	0,mem
	dec	r2,-
	brz	i_id8			/* RLEN == 0 */
/*
 * RLEN==1: copy one byte only
 */
	br	i_id5
i_id2:
	mov	mem,oal		/* RADDR */
	add	mem,r2,mem|mar++
	mov	mem,oah
	addc	mem,r0,mem|mar++
	mov	mem,r11
	adc	r11,mem|mar++
	add	mem,r2,mem|mar++	/* RULEN */
	addc	mem,r0,mem
	mov	(1<<0),brg
	or	brg,r12
	mov	r1,brg
	mov	brg,odl
	br	i_loop
i_id3:
/*
 * 2nd byte of a pair: put r1 in odh and copy them to RADDR
 * reset r12
 */
	mov	~(1<<0),brg
	and	brg,r12		/* reset bit0 */
	mov	r1,brg
	mov	brg,odh
	SPUTTWO(r11,err_bus)
	br	i_loop
i_id4:
/*
 * r13 > 0
 */
	mov	r13,brg
	br4	i_id10		/* c_ta0 != 0 */
	br7	i_id8		/* rblen > 0 */
	br1	i_id8		/* no read pending */
	br0	i_id4.5		/* raddr is odd */
	br	i_id1		/* char mode */
i_id4.5:
/*
 * raddr is odd
 * try to copy one byte first and make raddr even
 */
	mov	~(1<<0),brg
	and	brg,r13
i_id5:
/*
 * enter here: copy r1 to raddr, since rlen, raddr, rulen were not updated,
 * now dec rlen and inc raddr and rulen
 */
	mov	r1,brg
	mov	brg,odl
	mov	brg,odh
	mov	r8,%mar
	mov	C_RLEN,brg
	add	brg,r10,mar
	mov	1,r1
	mov	mem,r2
	dec	r2,mem|mar++	/* dec rlen */
	mov	mem,r2
	mov	0,r0|brg
	mov	brg,r12		/* reset r12 */
	subc	brg,r2,mem|mar++
	mov	mem,oal		/* inc raddr */
	add	mem,r1,mem|mar++
	mov	mem,oah
	addc	mem,r0,mem|mar++
	mov	mem,r2
	addc	mem,r0,mem|mar++
/*
	mov	C_RULEN,brg
	add	brg,r10,mar
 */
	add	mem,r1,mem|mar++	/* inc rulen */
	addc	mem,r0,mem
	SPUTONE(r2,err_bus)
	br	i_loop
i_id6:
/*
 * no more data in DK rcv fifo, check if odl has the 1st byte
 */
	mov	r12,brg
	br0	1f
	br	i_id7
1:
/*
 * odl has the 1st byte of a pair: copy it to RADDR then return
 * r13 needs adjusted
 */
	CALL(copy1,s.copy1)
i_id7:
/*
 * if char mode and rulen>0, go check timer
 */
	mov	r8,%mar
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	XBLK,brg
	orn	brg,r0,-
	brz	i_disp
	CALL(cktimer,s.cktimer)
	br	i_disp
i_id8:
/*
 * rblen > 0 or no read pending
 */
	mov	1,brg
	mov	brg,idh		/* indicate data byte */
i_id9:
/*
 * copy data/cntl to C_RB
 */
	mov	(1<<7),brg
	or	brg,r13
	br	i_nd
i_id10:
/*
 * c_ta0 != 0
 */
	mov	r8,%mar
	mov	C_TA1,brg
	add	brg,r10,mar
	mov	mem,r0
	dec	r0
	brz	1f
	dec	r0
	brz	3f
/*
 * TA1 (len) >=2; throw away the trailer (TA0);
 */
	mov	C_TA0,brg
	add	brg,r10,mar
	mov	0,mem|mar++
	mov	0,mem
	br	4f
1:
/*
 * len was 0: TA2=r1, len=1;;
 */
	mov	1,mem|mar++		/* TA1 = 1 */
	mov	r1,mem			/* TA2 = r1 */
	br	i_loop
3:
/*
 * len was 1: TA3=r1, len=2;
 */
	mov	2,mem|mar++		/* TA1 = 2 */
	mov	mem,mem|mar++
	mov	r1,mem			/* TA3 = r1 */
	br	i_loop
4:
	mov	~(1<<4),brg
	and	brg,r13			/* reset c_ta0 bit */
	br	i_loop

i_incomp:
i_badp:
/*
 * bad packet is received
 */
	br	i_csrtst
/*
 * OLD code for i_badp
 * Check for the # of badpacks and possibly panic.
	mov	%m_badpack,%mar
	mov	m_badpack,mar
	mov	mem,r0
	inc	r0,mem
	mov	MAXBADPK,brg
	sub	brg,r0
	ERROR(E_IPANIC)
	br	i_loop
 */
i_noqb:
/*
 * Run out of queues or input buffers
 */
	ERROR(E_NOQB)
	br	i_disp
i_disp:
/*
 * Return to main dispatch loop
 */
	mov	%disp2,brg
	mov	brg,pcr
	jmp	disp2

i_nd:
/*
 * copy data (r1) to stage-1 buffer (C_RB)
 * C_RB is a queue pointer: pnext, plen, *pdata, pseq.
 * idh - 1 if normal data, 0 if user cntl
 * r7-r5 - buffer address
 * r4 - stage-1 buffer index
 * r3 - plen
 * r2 - queue entry
 * r1 - data byte
 */
	CHK(B2_DATA,2)
	mov	r8,%mar
	mov	C_RBLEN,brg
	add	brg,r10,mar
	mov	mem,r0
	inc	r0,mem|mar++
	mov	mem,r0
	adc	r0,mem|mar++		/* RBLEN++ */
	mov	mem,r0			/* r0 = C_RB */
	brz	i_nd5
i_nd1:
	mov	r0,odl		/* save temporarily in odl */
	QA(r0)
	mov	mem,r0|mar++
	brz	i_nd2
	br	i_nd1
i_nd2:
/*
 * found end of the queue
 */
	mov	mem,r3|mar++		/* plen */
	mov	mem,r4|mar++		/* *pdata */
	mov	mem,r0			/* pseq */
	dec	r0,-
	brc	i_nd7			/* get a new queue */
	mov	BSIZE,brg
	sub	brg,r3,-
	brc	i_nd7			/* this buffer is full */
	mov	odl,r0
	QA(r0)
	mov	brg,brg|mar++		/* pnext */
	inc	r3,mem|mar++		/* plen++ */
i_nd3:
/*
 * r4 is buffer index, store 2 bytes (lo in r1 , hi in odh)
 */
	BITA(r4,r7,r6,r5)
	asl	r3,brg		/* offset is 2*plen */
	add	brg,r7,r7|brg
	mov	brg,oal
	adc	r6,r6|brg
	mov	brg,oah
	adc	r5
	mov	r1,brg
	mov	brg,odl
	mov	idh,odh
	SPUTTWO(r5,err_bus)		/* always at even boundary */
	br	i_loop
i_nd5:
/*
 * no stage-1 buffer exists yet
 */
	GETQ(r2,i_noqb)
	mov	r8,%mar
	mov	C_RB,brg
	add	brg,r10,mar
	mov	r2,mem
i_nd6:
/*
 * get a buffer and store 2 bytes at offset plen
 */
	GEBI(r4,i_noqb)
	QA(r2)
	mov	NIL,mem|mar++
	mov	1,mem|mar++		/* plen after storing */
	mov	r4,mem|mar++		/* *pdata */
	mov	0,mem			/* pseq */
	mov	mem,r3			/* plen before storing */
	br	i_nd3
i_nd7:
/*
 * get another queue and buffer to store 2 bytes
 */
	GETQ(r2,i_noqb)
	mov	odl,r0
	QA(r0)
	mov	r2,mem
	br	i_nd6

endseg2:


