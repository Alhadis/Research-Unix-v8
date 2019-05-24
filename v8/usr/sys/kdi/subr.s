
/*
 * subroutine
 */
	.org	3072
seg3:
r_err_bus:
/*
 * Unibus request fails to complete within 20 usec
 * currently is not tested
 */
	mov	%err_bus,brg
	mov	brg,pcr
	jmp	err_bus

rcv:
/*
 * copy packets from C_RQ to C_RADDR (from host to host for XBLK)
 * copy from C_RB to C_RADDR for ~XBLK
 * input : r9 is the chan #, r8/r10 point to LTE
 * Registers are used as follows:
 *	r15 - repmode
 *	r13(LSB), r12, r11 - ptr2 (RADDR)
 *	r9 - chan #
 *	r8/r10 - LTE address
 *	r7 - RQ
 *	r6(LSB), r5, r4(MSB) - stage 2 buffer
 *	r3 - plen
 *	r2 - len
 */
	CHK(B5_ENT,5)
/*
	mov	repsave,mar
	mov	%repsave,%mar
	mov	r12,mem
 */
	mov	0,brg
	mov	brg,r15
	mov	repctl,mar
	mov	%repctl,%mar
	mov	0,mem		/* flag to indicate no CNTL rcv yet */
/*
 * set STIME if timer expires
 */
	mov	r8,%mar
	mov	C_Y,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	YEXPIRE,brg
	orn	brg,r0,-
	brz	1f
	br	2f
1:	mov	STIME,brg
	or	brg,r15
2:
/*
 * put RADDR into r13, r12, r11
 * put RQ/RB into r7
 * RQ and RADDR will be restored with the new value after the while loop
 */
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	XBLK,brg
	orn	brg,r0,-
	brz	1f
	mov	C_RB,brg
	br	2f
1:	/* block mode */
	mov	C_RQ,brg
2:	add	brg,r10,mar
	mov	mem,r7
	brz	r_ex2
	mov	C_RADDR,brg
	add	brg,r10,mar
	mov	mem,r13|mar++
	mov	mem,r12|mar++
	mov	mem,r11
r_while:
/*
 * while ((C_RLEN>0) && (C_RQ!=NULL))
 */
	mov	r7,r7
	brz	r_endwhile
	mov	C_RLEN,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	mem,r2|mar++
	mov	mem,r1
	or	mem,r0
	dec	r0,-
	brz	r_endwhile
/*
 * len222 = len = min(plen,C_RLEN);
 */
	QA(r7)
	mov	mem,mem|mar++
	mov	mem,r3|mar++		/* plen */
	mov	mem,idl|mar++		/* *pdata */
	dec	r3,-			/* test for rej packet */
	brz	r_rq
/*
 * r2,r1 -= len222
 */
	mov	r8,%mar
	mov	C_RLEN,brg
	add	brg,r10,mar
	mov	r3,brg
	sub	brg,r2
	mov	0,brg
	subc	brg,r1
	brc	1f
/*
 * case: plen > C_RLEN
 * len = C_RLEN (r2)
 * C_RLEN = 0
 * plen (r3) = plen - len
 */
	mov	mem,r2
	mov	0,mem|mar++
	mov	0,mem
	CHK(B5_PLEN,5)
	mov	r2,brg
	sub	brg,r3
	br	2f
1:
/*
 * case: plen <= C_RLEN
 * C_RLEN is now in r2, r1
 * len = plen (r3)
 * plen = 0;
 */
	mov	r2,mem|mar++
	mov	r1,mem
	mov	r3,brg
	mov	brg,r2
	sub	brg,r3		/* plen = 0 */
2:
/*
 * idl is the buffer index
 * let r6-r4 points to pdata
 */
	mov	idl,brg
	mov	brg,r1
	BITA(r1,r6,r5,r4)
/*
 * while (len--) *ptr2++ = *ptr1++;
 * test 'r13' for even or odd first
 */
	mov	r13,brg
	br0	r_odd
	br	r_even
r_odd:
/*
 * copy one byte
 */
	CHK(B5_ODD,5)
	GETTWO(r6,r5,r4,r_err_bus)
	dec	r2
	mov	idh,brg
	br0	1f
	br	r_cnt1		/* cntl is rcv */
1:
	mov	idl,brg
	mov	brg,odl
	mov	brg,odh
	PUTONE(r13,r12,r11,r_err_bus)
	dec	r2,-
	brz	r_done
r_even:
/*
 * copy two bytes at a time
 */
	mov	2,brg
	sub	brg,r2,-	/* len - 2 */
	brc	1f
	brz	r_odd
	br	r_done
1:
	GETTWO(r6,r5,r4,r_err_bus)
	dec	r2
	mov	idh,brg
	br0	1f
	br	r_cnt1		/* cntl is rcv */
1:
	mov	idl,brg
	mov	brg,odl
	GETTWO(r6,r5,r4,r_err_bus)
	dec	r2
	mov	idh,brg
	br0	1f
	br	r_cnt0
1:
	mov	idl,brg
	mov	brg,odh
	PUTTWO(r13,r12,r11,r_err_bus)
	br	r_even
r_done:
/*
 * this is the end of copy
 * if (plen==0) ?
 */
	dec	r3,-
	brz	r_rq
/*
 * plen != 0, move data to the front of the buffer
 * while (plen--) *ptr2++ = *ptr1++
 * ptr2 in r13-11 (pdata[0]), ptr1 in r6-r4 (pdata[plen])
 */
	CHK(B5_MOVE,5)
	mov	tmp,mar
	mov	%tmp,%mar
	mov	r13,mem|mar++
	mov	r12,mem|mar++
	mov	r11,mem
	QA(r7)
	mov	mem,mem|mar++
	mov	r3,mem			/* store plen */
	mov	mem,idl|mar++		/* idl = plen */
	mov	mem,r3			/* points to pdata[0] */
	BITA(r3,r13,r12,r11)
	mov	idl,brg
	mov	brg,r3
r_move:
/*
 * while (plen--) ....
 */
	dec	r3,-
	brz	r_ret
	GETTWO(r6,r5,r4,r_err_bus)
	mov	idl,brg
	mov	brg,odl
	mov	idh,brg
	mov	brg,odh
	PUTTWO(r13,r12,r11,r_err_bus)
	dec	r3		/* move 2-bytes: 1 9-bit envelop */
	br	r_move
r_ret:
/*
 * restore r13, r12, r11
 */
	mov	tmp,mar
	mov	%tmp,%mar
	mov	mem,r13|mar++
	mov	mem,r12|mar++
	mov	mem,r11
/*
 * now test if cntl was rcv
 */
	mov	repctl,mar
	mov	%repctl,%mar
	mov	mem,r0
	dec	r0,-
	brc	r_cnt3		/* rcv cntl flag set */
	br	r_rp2		/* plen>0, then RLEN must be 0 */
r_rq:
/*
 * RQ = Pnext;
 * release buffer and queue
 * send echo or rej if pseq!=0
 */
	mov	r7,brg
	mov	brg,r5			/* r5 = old RQ */
	QA(r5)
	mov	mem,r7|mar++		/* r7 = new RQ */
	mov	0,mem|mar++		/* plen = 0 */
	mov	mem,r6			/* r6 = buf pointer */
	mov	NIL,mem|mar++
	mov	mem,r1			/* pseq */
	mov	mem,r2
	mov	0,mem
	FREEQ(r5)
	IBEG(r6)
	dec	r1,-
	brz	r_rq2
/*
 * if pseq&0370 == I_SEQ: pseq += I_ECHO - I_SEQ; set SBLOCK
 */
	mov	~WINDOW,brg
	and	brg,r2
	mov	I_SEQ,brg
	xor	brg,r2
	dec	r2,-
	brc	r_rq1
	mov	WINDOW,brg
	and	brg,r1
	mov	I_ECHO,brg
	or	brg,r1		/* r1 = ECHO + seq */
/*
 * if YBLOCK is set, set SBLOCK
 */
	mov	r8,%mar
	mov	C_Y,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	YBLOCK,brg
	orn	brg,r0,-
	brz	1f
	br	r_rq1
1:
	mov	SBLOCK,brg
	or	brg,r15		/* reach end of a block */
r_rq1:
	CALL(send,s.send)
	mov	r8,%mar
	mov	C_C,brg
	add	brg,r10,mar
	mov	r1,mem
r_rq2:
	mov	repctl,mar
	mov	%repctl,%mar
	mov	mem,r0
	dec	r0,-
	brc	r_cnt3
r_report:
/*
 * if C_RLEN==0 report(KS_RDB,r9)
 */
	mov	r8,%mar
	mov	C_RLEN,brg
	add	brg,r10,mar
	mov	mem,r0|mar++
	or	mem,r0
	dec	r0,-
	brz	r_rp2
	mov	SBLOCK,brg
	orn	brg,r15,-
	brz	r_rp3
	br	r_while
r_rp2:
	mov	SFULL,brg
	or	brg,r15			/* RLEN reaches 0 */
r_rp3:
/*
 * save r7 and r13,r12, r11 here
 * report READ completion: code in r15.
 */
	CHK(B5_REP,5)
	mov	r8,%mar
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
	mov	r15,mem
	CALL(report,s.report)
	mov	r8,%mar
	br	r_ex0
r_endwhile:
/*
 * Restore RADDR from r13, r12, r11
 * Restore RQ or RB (if ~XBLK) from r7
 */
	dec	r15,-
	brc	r_rp3
	mov	r8,%mar
	mov	C_RADDR,brg
	add	brg,r10,mar
	mov	r13,mem|mar++
	mov	r12,mem|mar++
	mov	r11,mem
r_ex0:
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	XBLK,brg
	orn	brg,r0,-
	brz	1f
	mov	C_RB,brg
	br	2f
1:	/* block mode */
	mov	C_RQ,brg
2:	add	brg,r10,mar
	mov	r7,mem
r_exit:
	CHK(B5_EXIT,5)
/*
	mov	repsave,mar
	mov	%repsave,%mar
	mov	mem,r12
 */
	RETURN(s.rcv)
r_ex2:
	dec	r15,-
	brc	r_rp3
	br	r_exit
r_cnt0:
/*
 * copy 1 byte to RADDR and report completion for SCNTL
 */
	mov	odl,brg
	mov	brg,odh
	PUTONE(r13,r12,r11,r_err_bus)
r_cnt1:
/*
 * len (r2) may not be 0
 * adjust RLEN and plen
 */
	mov	r2,brg
	add	brg,r3
	mov	0,r0
	inc	r2
	mov	r8,%mar
	mov	C_RLEN,brg
	add	brg,r10,mar
	add	mem,r2,mem|mar++
	addc	mem,r0,mem
	mov	repctl,mar
	mov	%repctl,%mar
	mov	idl,mem
	br	r_done
r_cnt3:
/*
 * prepare for report
 */
	mov	SCNTL,brg
	or	brg,r15
	br	r_rp3

clean:
/*
 * r2 is a queue pointer: release all queues/buffers
 * using r0, r2, r3, brg, mar, %mar
 */
	CHK(B5_AJAX,5)
c_1:
	mov	r2,brg
	brz	c_exit
	mov	brg,r0
	QA(r0)
	mov	mem,r2|mar++	/* pnext */
	mov	0,mem|mar++	/* plen */
	mov	mem,r3
	mov	NIL,mem|mar++	/* *pdata */
	mov	0,mem		/* pseq */
	FREEQ(r0)
	IBEG(r3)
	br	c_1
c_exit:
	RETURN(s.clean)

send:
/*
 * This subroutine sends a 1-byte (in r1) meta packet on chan r9
 */
	SENDHEAD(r9)
	D422(4)
	SENDDATA(D_WCNTL,r1)
	D422(4)
	SENDPACK(r1)
	D422(4)
	SENDCMD(D_READ)
	RETURN(s.send)


report:
	mov	csr0,brg
	br0	report		/* csr0==3, loop forever */
/*
 * kin is 12 bytes: KS_SEND info 4 bytes ==> INCA(8)
 *		KS_RDB info 8 bytes ==> INCA(4)
 *		KS_EOI, KS_ERR info 6 bytes ==> INCA(6)
 * r9 - channel #
 * repinfo: code, lenlo, lenhi, cntl, mode
 * r1, r12, r13 are saved and restored when exit
 * this implementation is fast but use 70 more instructions than:
 * have a loop to put info in repinfo to odl/odh for PUTTWO
 * Modify HS and sbhead
 * Interrupt cpu
 * using r0, r1, r2, r9, r11, r12, r13, npr, mar, %mar, odl, odh, oal, oah, brg
 */
	mov	repsave,mar
	mov	%repsave,%mar
	mov	r1,mem|mar++
	mov	r12,mem|mar++
	mov	r13,mem|mar++
	mov	sbhead,mar
	mov	%sbhead,%mar
	mov	mem,r13|mar++
	mov	mem,r12|mar++
	mov	mem,r11
	mov	%repinfo,%mar
	mov	repinfo,mar
	mov	mem,odl
	mov	mem,r2		/* r2 = (repinfo) */
	mov	kseq,mar
	mov	%kseq,%mar
	mov	mem,odh		/* for debugging */
	mov	mem,r0
	inc	r0,mem
	PUTTWO(r13,r12,r11,r_err_bus)
	mov	r9,brg
	mov	brg,odl			/* chan# */
	PUTTWO(r13,r12,r11,r_err_bus)
	mov	KS_SEND,brg
	addn	brg,r2,-
	brz	rp2
/*
 * report is KS_RDB and others
 */
	mov	%replen,%mar
	mov	replen,mar
	mov	mem,odl|mar++		/* lenlo */
	mov	mem,odh			/* lenhi */
	PUTTWO(r13,r12,r11,r_err_bus)
	mov	%repctl,%mar
	mov	repctl,mar
	mov	mem,odl|mar++
	mov	mem,odh
	PUTTWO(r13,r12,r11,r_err_bus)
	mov	KS_RDB,brg
	addn	brg,r2,-
	brz	rp3
	br	rp4
rp2:
/*
 * report is KS_SEND
 */
	mov	8,brg
	br	rpy
rp2.5:
/*
 * reset bit map for chan# in r9
 */
	mov	7,brg
	and	brg,r9,brg
	mov	brg,r0
	mov	%bitpatt,%mar
	mov	bitpatt,brg
	add	brg,r0,mar
	mov	mem,r0
	mov	r9,brg
	mov	0,brg>>
	mov	0,brg>>
	mov	0,brg>>
	mov	brg,r2
	mov	%timemap,%mar
	mov	timemap,brg
	add	brg,r2,mar
	or	mem,r0,mem		/* turn bit on  */
	xor	mem,r0,mem		/* turn bit off */
	br	rp4
rp3:
/*
 * report was KS_RDB
 * reset C_Y and RULEN
 */
	mov	r8,%mar
	mov	C_Y,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	0,mem			/* reset READ mode */
	mov	C_RULEN,brg
	add	brg,r10,mar
	mov	0,mem|mar++
	mov	0,mem
/*
 * if YTIMACT, reset bit in timemap
 */
	mov	YTIMACT,brg
	orn	brg,r0,-
	brz	rp2.5
rp4:
	mov	4,brg
rpy:
/*
 * brg: increment count
 * Now modify HS and sbhead
 * If HS==BUFSIZ-1, then HS=0 and sbhead=sbaddr
 */
	mov	brg,odl
	mov	HS,r0
	mov	BUFSIZ-1,brg
	sub	brg,r0,-	/* HS-(BUFSIZ-1) */
	brc	1f
	inc	r0,HS
	INCA(r13,r12,r11,odl)
	br	2f
1:	mov	0,brg		/* reset HS */
	mov	brg,HS
	mov	sbaddr,mar
	mov	%sbaddr,%mar
	mov	mem,r13|mar++
	mov	mem,r12|mar++
	mov	mem,r11
2:
/*
 * Restore sbhead
 */
	mov	sbhead,mar
	mov	%sbhead,%mar
	mov	r13,mem|mar++
	mov	r12,mem|mar++
	mov	r11,mem|mar++
/*
 * Send interrupt 2 to the PDP-11
 */
	mov	0200,brg
	mov	brg,csr2	/* RDYO to csr2 for kmc.c */
	mov	nprx,brg|r0
	br7	1f
	mov	BRQ|VEC4,brg
	mov	brg,nprx
1:
/*
 * restore r1, r12, r13
 */
	mov	repsave,mar
	mov	%repsave,%mar
	mov	mem,r1|mar++
	mov	mem,r12|mar++
	mov	mem,r13|mar++
	RETURN(s.report)

copy1:
/*
 * r13 is set to (1<<0) and bit 6 (block/char mode) unchanged because:
 *	rblen == 0; c_ta0 == 0; rlen>0; raddr will become odd after copy1
 * r12.0 is 1; reset r12 when exit
 *	copy odl to raddr and restore r1 before return
 *	raddr is in oal, oah, r11
 *	since rlen, raddr, rulen were changed for 2 bytes
 *		adjustments are needed here
 */
	mov	(1<<6),brg
	and	brg,r13		/* preserve block/char mode bit */
	mov	r1,brg
	mov	brg,idh		/* save r1 */
	mov	odl,brg
	mov	brg,odh
	SPUTONE(r11,r_err_bus)
	mov	r8,%mar
	mov	C_RLEN,brg
	add	brg,r10,mar
	mov	1,r1|brg
	or	brg,r13		/* set bit 0 (radd odd) */
	mov	0,r0|brg
	mov	brg,r12			/* reset r12 */
	add	mem,r1,mem|mar++	/* inc rlen */
	addc	mem,r0,mem|mar++
	mov	mem,r11
	dec	r11,mem|mar++		/* dec raddr */
	mov	mem,r11
	subc	brg,r11,mem|mar++
	mov	mem,r11
	subc	brg,r11,mem|mar++
	mov	mem,r11
	dec	r11,mem|mar++		/* dec rulen */
	mov	mem,r11
	subc	brg,r11,mem|mar++
/*
 * restore r1 from idh and return
 */
	mov	idh,brg
	mov	brg,r1
	RETURN(s.copy1)

cktimer:
/*
 * check timer status: it is called from input routine:
 * if char mode and channel # is changed or fifo is empty;
 * if rulen>0 && rlen==0, reset rulen to 0, report SFULL, and return
 * if (c_rb==nil && c_rulen==0) return;
 * if (YTIME) set YEXPIRE or turn on timer
 * call 'rcv'
 * r1 has to be preserved
 * r9: channel number
 * using: r0, ....
 */
	mov	r8,%mar
	mov	C_RULEN,brg
	add	brg,r10,mar
	mov	mem,r2|mar++
	or	mem,r2
	dec	r2,-
	brz	ct_3
/*
 * if rlen==0, report SFULL
 */
	mov	C_RLEN,brg
	add	brg,r10,mar
	mov	mem,r0|mar++
	or	mem,r0
	dec	r0,-
	brc	ct_4
	mov	%repinfo,%mar
	mov	repinfo,mar
	mov	KS_RDB,mem|mar++
	mov	0,mem|mar++		/* residue length */
	mov	0,mem|mar++
	mov	0,mem|mar++		/* cntl char */
	mov	SFULL,mem		/* mode */
	CALL(report,s.report)
	br	ct_9
ct_3:
	mov	C_RB,brg
	add	brg,r10,mar
	mov	mem,brg
	brz	ct_9
ct_4:
/*
 * check if timeout mode
 */
	mov	C_Y,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	YTIME,brg
	orn	brg,r0,-
	brz	1f
	br	ct_7
1:
	mov	YEXPIRE,brg
	orn	brg,r0,brg
	brz	ct_7		/* timer already expires */
	mov	YTIMACT,brg
	orn	brg,r0,-
	brz	ct_7		/* timer already active */
/*
 * set timer active and load initial timer value
 */
	or	brg,r0,mem	/* set YTIMACT */
	mov	C_INITIM,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	C_ITIME,brg
	add	brg,r10,mar
	dec	r0,mem
	brc	ct_10
/*
 * timer expires: INITIM is 0
 */
	mov	0,mem
	mov	C_Y,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	YEXPIRE,brg
	or	brg,r0
	mov	~YTIMACT,brg
	and	brg,r0,mem
ct_7:
/*
 * timer expires, call rcv
 * save r1 here before calling 'rcv'
 */
	mov	%rcvsave,%mar
	mov	rcvsave,mar
	mov	r13,mem|mar++
	mov	r12,mem|mar++
	mov	r1,mem|mar++
	CALL(rcv,s.rcv)
	mov	%rcvsave,%mar
	mov	rcvsave,mar
	mov	mem,r13|mar++
	mov	mem,r12|mar++
	mov	mem,r1
ct_9:
	RETURN(s.cktimer)
ct_10:
/*
 * timer not expire: turn on bitmap and set YTIMACT
 */
	mov	r9,brg
	mov	7,r7
	and	brg,r7
	mov	0,brg>>
	mov	0,brg>>
	mov	0,brg>>
	mov	brg,r0
	mov	%bitpatt,%mar
	mov	bitpatt,brg
	add	brg,r7,mar
	mov	mem,r7
	mov	%timemap,%mar
	mov	timemap,brg
	add	brg,r0,mar
	or	mem,r7,mem
	br	ct_7

strailer:
/*
 * XNIL is set, send trailer-only on this channel
 * chan# is not on 'outq'
 * r9, r8/r10 is set up
 * using r0, r1, r2, ...
 */
	mov	r8,%mar
	mov	C_S,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	I_SEQ,brg
	or	brg,r0,brg
	mov	brg,r1
	inc	r0
	mov	WINDOW,brg
	and	brg,r0,mem
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	XBOTM,brg
	orn	brg,r0,-
	brz	1f
	mov	I_BOT,brg
	br	2f
1:	mov	I_BOTM,brg
2:	mov	brg,r2
	SENDHEAD(r9)
	D422(4)
	SENDDATA(D_WCNTL,r2)
	D422(4)
	SENDDATA(D_WDATA,0)
	D422(4)
	SENDDATA(D_WDATA,0)
	D422(4)
	SENDDATA(D_WCNTL,r1)
	D422(4)
	SENDPACK(r0)
	D422(4)
	SENDCMD(D_READ)
	RETURN(s.strailer)

/*
 * end of segment 3
 */
endseg3:


