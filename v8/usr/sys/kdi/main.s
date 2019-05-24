/*
 * This version of kmc-cpu interface uses two circular buffers.
 * Cmdbuf is used to pass command to kmc;
 * statbuf is used to report status.
 * Csr4~5 are the head/tail of the cmdbuf, and csr6~7 for statbuf.
 *
 * When csr0 is 0, kmc is idle and does nothing.
 * The cpu will set csr to 1 and pass cmdbuf/statbuf addresses
 * in csr4~7.
 * Then the kmc will set csr0 to 2 and stay at this state forever.
 *
 *
 * Permanently assigned registers:
 *
 * Channel dependent assigned registers:
 *
 *	r14 - free kmc queue pointer
 *	r9 - channel #
 *	r8 - group number currently being serviced
 *	r10 - address of current line-table entry
 */

#define	C_RLEN		0	/* read length */
#define	C_RADDR		2	/* read address */
#define	C_RULEN		5	/* uncheck input in RADDR */
#define	C_XLEN		7	/* write length */
#define	C_XADDR		9	/* write address */
#define	C_RQ		12	/* un-ack input: in host memory */
#define	C_RBLEN		13	/* length */
#define	C_RB		15	/* un-check input: waiting for trailer */
#define	C_S		16	/* next seq# to be sent */
#define	C_R		17	/* last seq# echoed */
#define	C_A		18	/* last seq# acked */
#define	C_C		19	/* last seq# received */
#define	C_TA0		20	/* trailer: BOT/BOTM/BOTS/SOI */
#define	C_TA1		21	/* len of trailer data */
#define	C_TA2		22	/* 1st byte of trailer/SOI data */
#define	C_TA3		23	/* 2nd byte of trailer/SOI data */
#define	C_RSEQ		24	/* seq# for this block */
#define	C_XCNTL		25	/* cntl char following the data */
#define	C_X		26	/* state of the chan */
#define	C_X1		27	/* state of the chan (ovf from C_X) */
#define	C_Y		28	/* input state */
#define	C_ITIME		29	/* timer for last char input */
#define	C_INITIM	30	/* initial value for ITIME */
 
/*
 * define bits in C_X
 * bits 0-2 are exclusive
 */
#define	XIDLE	(1<<0)	/* chan is uninitialized */
#define	XACT	(1<<1)	/* chan is initialized */
#define	XINIT	(1<<2)	/* chan is in the init stage */
/*
 * define bits in C_X1 (overflowed from C_X)
 */
#define	XNIL	(1<<0)	/* send trailer only */
#define	XREJ	(1<<3)	/* hasn't rcv ECHO since last REJ */
#define	XBLK	(1<<4)	/* set if block mode, else char mode */
#define	XENQ	(1<<5)	/* has sent ENQ and not rcv ECHO yet */
#define	XACK	(1<<6)	/* C_S has been incremented since ENQ/CHECK */
#define	XBOTM	(1<<7)	/* send BOTM (instead of BOT) */
/*
 * define bits in C_Y
 */
#define	YTIMACT	(1<<0)	/* timer active */
#define	YEXPIRE	(1<<1)	/* timer expires */
#define	YBLOCK	(1<<5)	/* rcv return on block boundary */
#define	YTIME	(1<<6)	/* rcv return on time expires */
/*
 * Define useful constants
 */
#define	NIL	0377
#define	NULL	0377
#define	NUL	0377
#define	ZERO	0
/*
 * Datakit and protocol related constant
 */
#define	DKCHUNK		16	/* packet size - don't change */
#define	CHANMODS	96	/* # of channels per module */
#define	NGRP		CHANMODS/8-1 /* # of bytes for bit map */
#define	BUFSIZ		10	/* size of cmdbuf, statbuf, etc */
#define	MAXBADPK	64	/* bad packet received before complaining */
#define	DKBMASK		03	/* seq # is  0 to 3 */
#define	WINDOW		07	/* max window size */
#define	DKBLOCK		28	/* block size before trailer */
#define	BSIZE		64	/* size (9-bits) of each host buffer */
#define	BSIZE2		BSIZE*2	/* size in bytes */
/*
 * protocol cntl patterns
 */
#define	I_SEQ		0010	/* 8 sequence numbers to end trailers */
#define	I_ECHO		0020	/* 8 echoes, data given to host */
#define	I_REJ		0030	/* 8 rejections, transmission error */
#define	I_ACK		0040	/* first of 8 acks, correct reception */
#define	I_BOT		0050	/* normal beginning of trailer */
#define	I_BOTM		0051	/* trailer with more data to follow */
#define	I_BOTS		0052	/* seq update algorithm on this trailer */
#define	I_SOI		0053	/* start of interrupt trailer */
#define	I_EOI		0054	/* end of interrupt trailer */
#define	I_ENQ		0055	/* xmitter request flow/error status */
#define	I_CHECK		0056	/* xmitter request error status */
#define	I_INITREQ	0057	/* request initialization */
#define	I_INIT0		0060	/* disable trailer processing */
#define	I_INIT1		0061	/* enable trailer processing */
#define	I_AINIT		0062	/* response to INIT0/INIT1 */
/*
 * command type
 */
#define	KC_INIT		1	/* init: 0,0,0,0 */
#define	KC_SEND		2	/* send: len, cntl, mode, addr */
#define	KC_RCVB		3	/* rcv: len, time, mode, addr */
#define	KC_CLOSE	4	/* close: 0, 0, 0, 0 */
#define	KC_XINIT	5	/* re-init xmitter: 0, 0, 0, 0 */
#define	KC_CMD		6	/* cmd to kmc: cmd, 0, 0, 0 */
#define	KC_FLAG		7	/* i/oflag: iflag, oflaghi, oflaglo, 0 */
#define	KC_SOI		8	/* send express: (byte2<<8)|byte1, 0, 0, 0 */
#define	KC_TDK		9	/* send to tdk: ctl, 0, 0, 0 */
/*
 * report type
 */
#define	KS_SEND		20	/* send: 0, 0, 0, 0 */
#define	KS_RDB		21	/* rcv: residue len, cntl, mode, 0 */
#define	KS_EOI		22	/* rcv express: (byte2<<8)|byte1, 0, 0, 0 */
#define	KS_CNTL		23	/* rcv tdk cntl: 0, cntl, 0, 0 */
#define	KS_ERR		24	/* error: code, 0, 0, 0 */
/*
 * sub command bits for KC_CMD
 */
#define	OFLUSH	(1<<1)	/* Flush output */
#define	OSPND	(1<<2)	/* Suspend output */
#define	ORSME	(1<<3)	/* Resume output */
/*
 * KC_RCVB mode
 */
#define	CBLOCK	(1<<5)	/* return on block boundary */
#define	CTIME	(1<<6)	/* return on time expires */
/*
 * KS_RDB mode
 */
#define	SFULL	(1<<0)	/* buffer full */
#define	SCNTL	(1<<1)	/* cntl char rcv */
#define	SABORT	(1<<3)	/* rcv aborted */
#define	SBLOCK	(1<<5)	/* block boundary */
#define	STIME	(1<<6)	/* timer expires */
/* 
 * define bit pattern 
 */
#define	BIT0	(1<<0)
#define	BIT1	(1<<1)
#define	BIT2	(1<<2)
#define	BIT3	(1<<3)
#define	BIT4	(1<<4)
#define	BIT5	(1<<5)
#define	BIT6	(1<<6)
#define	BIT7	(1<<7)

#define	CLT0		0
#define	CLT1		010
#define	CLK1MS		16
#define	R_TIMER		6	/* multiple of 50 us (300us here) */
/*
 * head/tail of circular buffers
 */
#define	HC		csr4	/* head of cmdbuf */
#define	TC		csr5	/* tail of cmdbuf */
#define	HS		csr6	/* head of statbuf */
#undef	TS
#define	TS		csr7	/* tail of statbuf */
/*
 * Define bits in npr
 */
#define NRQ (1<<0)
#define OUT (1<<4)
#define BYTE (1<<7)
/*
 * Define bits in nprx
 */
#define NEM (1<<0)
#define ACLO (1<<1)
#define PCLK (1<<4)
#define VEC4 (1<<6)
#define BRQ (1<<7)
/*
 * Define check point code
 */
#define	B0_KCSO	(1<<0)	/* enter KC_SOI cmd */
#define	B0_KCCM	(1<<1)	/* enter KC_CMD cmd */
#define	B0_KCFG	(1<<2)	/* enter KC_FLAG cmd */
#define	B0_KC1	(1<<3)	/* enter KC_ONE cmd */
#define	B0_KCSD	(1<<4)	/* enter KC_SEND cmd */
#define	B0_KCRB	(1<<5)	/* enter KC_RCVB cmd */
#define	B0_KCIT	(1<<6)	/* enter KC_INIT cmd */
#define	B0_KCCS	(1<<7)	/* enter KC_CLOSE cmd */

#define	B1_ENT	(1<<0)	/* enter output seg */
#define	B1_CNTL	(1<<1)	/* send XCNTL */
#define	B1_ERO	(1<<2)	/* enter robin */
#define	B1_SPK	(1<<4)	/* issue D_XPACK */
#define	B1_ENRO	(1<<5)	/* endrobin */
#define	B1_EXIT	(1<<7)	/* exit output */

#define	B2_ENT	(1<<0)	/* enter input seq */
#define	B2_CNTL	(1<<1)	/* control packet */
#define	B2_SUP	(1<<2)	/* rcv sup cntl char */
#define	B2_NWWT	(1<<3)	/* rcv not well form trailer */
#define	B2_REP	(1<<4)	/* KS_RDB interrupt */
#define	B2_DATA	(1<<6)	/* data packet */
#define	B2_EXIT	(1<<7)	/* exit input seg */

#define	B3_SEQ	(1<<0)	/* rcv SEQ */
#define	B3_ECHO	(1<<1)	/* rcv ECHO */
#define	B3_ACK	(1<<2)	/* rcv ACK */
#define	B3_REJ	(1<<3)	/* rcv REJ */
#define	B3_CHK	(1<<4)	/* rcv CHK */
#define	B3_ENQ	(1<<5)	/* rcv ENQ */

#define	B4_EOI	(1<<0)	/* rcv EOI */
#define	B4_BOT	(1<<1)	/* rcv BOT */
#define	B4_IN1	(1<<2)	/* rcv INIT1 */
#define	B4_IN0	(1<<3)	/* rcv INIT0 */
#define	B4_AIN	(1<<4)	/* rcv AINIT*/
#define	B4_IREQ	(1<<5)	/* rcv INITREQ */

#define	B5_ENT	(1<<0)	/* enter rcv */
#define	B5_PLEN	(1<<1)	/* plen >= RLEN */
#define	B5_ODD	(1<<2)	/* r_odd */
#define	B5_REP	(1<<3)	/* report READ comp */
#define	B5_MOVE	(1<<4)	/* partial buffer */
#define	B5_AJAX	(1<<5)	/* enter clean() */
#define	B5_EXIT	(1<<6)	/* exit rcv */
#define	B5_TIME	(1<<7)	/* timer expires */

#define	B6_POST	(1<<0)	/* output postprocessing */
#define	B6_NL	(1<<1)	/* NL */
#define	B6_CR	(1<<2)	/* CR */
#define	B6_TAB	(1<<3)	/* TAB */
#define	B6_BS	(1<<4)	/* back space */
#define	B6_VT	(1<<5)	/* VT */
#define	B6_FF	(1<<6)	/* FF */

#define	B7_IFL	(1<<0)	/* flush input */
#define	B7_OFL	(1<<1)	/* flush output */
#define	B7_OSP	(1<<2)	/* suspend output */
#define	B7_ORE	(1<<3)	/* resume output */
#define	B7_IUB	(1<<4)	/* unblock input */
#define	B7_IBL	(1<<5)	/* block input */
#define	B7_BRK	(1<<6)	/* send break */

/*
 * Define error codes
 */
#define	E_SW		00	/* dispatcher switch */
#define	E_BUS		01	/* Unibus error */
#define	E_IPANIC	02	/* input routine panic */
#define	E_CMD		03	/* command unknown */
#define	E_NOQB		04	/* run out of queue or buffer */
#define	E_DUP		05	/* duplicate SEND */
#define	E_ODKOVF	06	/* output routine panic */
#define	E_UMETA		07	/* un-recognized cntl char */
#define	E_SYS1		041	/* system error 1 */
#define	E_SYS2		042	/* system error 2 */
 
#ifdef	DR11C
/*
 * specific declarations for dr11-c
 */
#define	DKRDONE	(1<<7)	/* 15-th bit */
#define	DKTDONE	(1<<7)	/* 7-th bit */
#define	DKCOM	(3<<0)	/* 01 bits */
#define	DKTENAB	(1<<6)	/* 6-th bit */
#define	DKRENAB	(1<<5)	/* 5-th bit */
#define DKDATA	(1<<0)	/* 8-th bit */
#define	DKMARK	(1<<1)	/* 9-th bit */
#define	ILO	idl
#define	IHI	idh
#define	OLO	odl
#define	OHI	odh
#define	D_OSEQ	0
#define	D_READ	1
#define	D_WRITE	2
#define	D_XPACK	3
#define	D_WHEAD	(1<<1)		/* 9-th bit */
#define	D_WDATA	(1<<0)		/* data byte */
#define	D_WCNTL	0		/* control byte */
#endif
#ifdef	KDI
/*
 * specific declarations for dki board
 */
#define	ILO	lur0
#define	IHI	lur5
#define	OLO	lur0
#define	OHI	lur1
#define	DKFRDY	(1<<0)	/* data from dk ready */
#define	TRQRDY	(1<<1)	/* seq# rdy: xmit done */
#define	D_OSEQ	(0<<6)
#define	D_RESET	(0<<6)
#define	D_READ	(1<<6)
#define	D_WHEAD	0202
#define	D_WDATA	0201
#define	D_WCNTL	0200
#define	D_XPACK	0301
#endif

/*
 * MACRO definition
 */
 
/*
 * Define the "BUSWAIT" macro
 * not test NMEM is dangereous if memory does not exist, but fast
 */
#define	BUSWAIT\
5:	mov	npr,brg;\
	br0	5b		/* loop until ~NRQ */

/*
 * Macros to get and release queue entries
 * r14 points to the list of available queue entries
 */
#define	FREEQ(X)\
	mov	03,brg;\
	and	brg,X,%mar;\
	mov	~03,brg;\
	and	brg,X,mar;\
	mov	r14,mem;\
	mov	X,brg;\
	mov	brg,r14

#define	GETQ(X,Y)\
	mov	r14,brg;\
	brz	Y;\
	mov	brg,X;\
	mov	03,brg;\
	and	brg,X,%mar;\
	mov	~03,brg;\
	and	brg,X,mar;\
	mov	mem,r14
 
/*
 * QA(REG) : QA converts a queue pointer to mar & %mar form.
 *	REG is a scratch pad register and has the format:
 *	bits 7-2 from REG + 00 forms the offset within a page
 *	bits 1-0 is the page number
 *	Queues are 4-byte long, so it always fall in 4-byte boundary
 *	Queues can only be located in the first 4 pages
 */
#define	QA(REG)\
	mov	~03,brg;\
	and	brg,REG,mar;\
	mov	03,brg;\
	and	brg,REG,%mar

/*
 * GEBI(X,ADDR) : get a free buffer index
 *	'bmap' to 'bmap'+16 indicate 128 buffers, each 128 bytes.
 *	A '1' in i-th bit postion indicates i-th buffer is buzy
 *	GEBI finds the first free buffer and returns its number
 *	in register X. If no buffer is free, jump to ADDR
 *	Turn on i-th bit
 *	X can't be r0
 *	using r0, X, brg, mar, %mar
 */
#define GEBI(X,ADDR)\
	mov	0,brg;\
	mov	brg,X;\
	mov	1,brg;\
	mov	brg,r0;\
	mov	bmap,mar;\
	mov	%bmap,%mar;\
5:	mov	mem,brg;\
	brz	8f;\
6:	br0	7f;\
	br	9f;\
7:	inc	X;\
	asl	r0;\
	mov	0,brg>>;\
	br	6b;\
8:	mov	8,brg|mar++;\
	add	brg,X,X|brg;\
	br7	ADDR;\
	br	5b;\
9:	or	mem,r0,mem

/*
 * IBEG(X) : return buffer index X to pool (i.e. turn off bit in 'bmap')
 *	X can't be r0
 *	using r0, X, brg, mar, %mar
 */
#define	IBEG(X)\
	mov	X,brg;\
	brz	9f;\
	mov	bmap0,mar;\
	mov	%bmap0,%mar;\
6:	mov	8,brg|mar++;\
	sub	brg,X;\
	brc	6b;\
	add	brg,X;\
	mov	1,brg;\
	mov	brg,r0;\
7:	dec	X;\
	brz	8f;\
	asl	r0,r0|brg;\
	br	7b;\
8:	mov	0,r0;\
	addn	brg,r0;\
	and	mem,r0,mem;\
9:

/*
 * BITA(I,X,Y,Z): buffer index (in I) to address (X:LSB, Y, Z:MSB)
 */
#define	BITA(I,X,Y,Z)\
	mov	baddr,mar;\
	mov	%baddr,%mar;\
	mov	mem,X|mar++;\
	mov	mem,Y|mar++;\
	mov	mem,Z;\
8:	dec	I;\
	brz	9f;\
	mov	BSIZE2,brg;\
	add	brg,X;\
	adc	Y;\
	adc	Z;\
	br	8b;\
9:

/*
 * Subroutine CALL and RETURN macros
 */
#define	CALL(X,SAVE)\
	mov	%SAVE,%mar;\
	mov	SAVE,mar;\
	mov	%9f,mem|mar++;\
	mov	9f,mem;\
	mov	%X,brg;\
	mov	brg,pcr;\
	jmp	X;\
9:

#define	RETURN(SAVE)	\
	mov	%SAVE,%mar;\
	mov	SAVE,mar;\
	mov	mem,pcr|mar++;\
	jmp	(mem)

/*
 * ERROR(X) --- store error code in brg, jump to errlog
 */
#define	ERROR(X) \
	mov	%errlog,brg;\
	mov	brg,pcr;\
	mov	X,brg;\
	jmp	errlog
 
/*
 * Macro BRB(X,Y,Z): if bit Y is on in X, then branch to Z
 *	X is a scratch pat register
 * Macro BRP(X,Y,Z): if bit Y is on in X, then branch to Z
 *	X may not be a scratch pat register
 */
#define	BRB(X,Y,Z)	\
	mov	~Y,brg;\
	or	brg,X,-;\
	brz	Z
#define	BRP(X,Y,Z)	\
	mov	X,r0;\
	mov	~Y,brg;\
	or	brg,r0,-;\
	brz	Z
 
/*
 * CHK(X,Y): set flag X on Y-th check word
 */
#ifdef CHECK
#define	CHK(X,Y)\
	mov	Y,brg;\
	mov	brg,r0;\
	mov	check,brg;\
	add	brg,r0,mar;\
	mov	%check,%mar;\
	mov	X,brg;\
	mov	brg,r0;\
	or	mem,r0,mem
#else
#define	CHK(X,Y)
#endif
 
/*
 * CHK1(X,Y): set flag X on Y-th check word
 * same as CHK except restore page register (r8)
 */
#ifdef CHECK
#define	CHK1(X,Y)\
	CHK(X,Y);\
	mov	r8,%mar
#else
#define	CHK1(X,Y)
#endif

/*
 * GLTE(CHAN) : CHAN is the channel #
 *	LTE size is 32 bytes for each channel
 *	When exit, r8(%mar), r10(mar) = addr of LTE
 *	Using r8, r10, brg, mar, mem
 */
#define	GLTE(CHAN)\
	mov	CHAN,brg;\
	mov	brg,brg>>;\
	mov	brg,brg>>;\
	mov	brg,brg>>;\
	mov	brg,r8;\
	mov	0340,brg;\
	and	brg,r8,brg;\
	mov	brg,r10|mar;\
	mov	037,brg;\
	and	brg,r8;\
	mov	%dzst,brg;\
	add	brg,r8,r8|%mar
 
/*
 * INCA(X,Y,Z,AMOUNT) - inc the content of registers X, Y, Z
 *	(Z most significant) by AMOUNT
 *	X, Y, Z are scratch pad registers
 *	Using X, Y, Z, brg
 */
#define	INCA(X,Y,Z,AMOUNT)\
	mov	AMOUNT,brg;\
	add	brg,X;\
	adc	Y;\
	adc	Z

/*
 * SPUTTWO(Z,ERR) - put two bytes from odl & odh to cpu
 *	cpu addresses are in oal (bits 7-0), oah (bits 15-8),
 *	and Z (bits 17-16)
 *	If error, go to ERR.
 *	When exit, cpu addresses are NOT incremented by 2
 *	Does NOT wait for UNIBUS to complete
 *	Using r0, r1, brg, npr, oal, oah, odl, odh, mar, mem, X, Y, Z.
 */
#define	SPUTTWO(Z,ERR)\
	asl	Z,brg;\
	mov	brg,r0;\
	asl	r0;\
	mov	nprx,r1;\
	mov	~(BRQ|ACLO|(3<<2)),brg;\
	and	brg,r1,brg;\
	or	brg,r0,nprx	/* set up bits 17-16 */;\
	mov	NRQ|OUT,brg;\
	mov	brg,npr		/* issue unibus request */;\
	BUSWAIT
 
/*
 * PUTTWO(X,Y,Z,ERR) - put two bytes from odl & odh to cpu
 *	cpu addresses are stored in X (bits 7-0)
 *		Y (bits 15-8) and Z (bits 17-16)
 *	If error, go to ERR.
 *	When exit, cpu addresses are incremented by 2
 *	Using r0, r1, brg, npr, oal, oah, odl, odh, mar, mem, X, Y, Z.
 */
#define	PUTTWO(X,Y,Z,ERR)\
	mov	X,brg;\
	mov	brg,oal;\
	mov	Y,brg;\
	mov	brg,oah;\
	SPUTTWO(Z,ERR);\
	INCA(X,Y,Z,2);\
	BUSWAIT
 
/*
 * SPUTONE(Z,ERR) - put one bytes from odl/odh to cpu
 *	cpu addresses are in oal (bits 7-0), oah (bits 15-8),
 *	and Z (bits 17-16)
 *	If error, go to ERR.
 *	When exit, cpu addresses are NOT incremented by 1
 *	Does NOT wait for UNIBUS to complete
 *	Using r0, r1, brg, npr, oal, oah, odl, odh, mar, mem, Z.
 */
#define	SPUTONE(Z,ERR)\
	asl	Z,brg;\
	mov	brg,r0;\
	asl	r0;\
	mov	nprx,r1;\
	mov	~(BRQ|ACLO|(3<<2)),brg;\
	and	brg,r1,brg;\
	or	brg,r0,nprx	/* set up bits 17-16 */;\
	mov	NRQ|OUT|BYTE,brg;\
	mov	brg,npr		/* issue unibus request */;\
	BUSWAIT
 
/*
 * PUTONE(X,Y,Z,ERR) - put one bytes from odl/odh to cpu
 *	cpu addresses are stored in X (bits 7-0)
 *		Y (bits 15-8) and Z (bits 17-16)
 *	If error, go to ERR.
 *	When exit, cpu addresses are incremented by 1
 *	Using r0, r1, brg, npr, oal, oah, odl, odh, mar, mem, X, Y, Z.
 */
#define	PUTONE(X,Y,Z,ERR)\
	mov	X,brg;\
	mov	brg,oal;\
	mov	Y,brg;\
	mov	brg,oah;\
	SPUTONE(Z,ERR);\
	INCA(X,Y,Z,1);\
	BUSWAIT
 
/*
 * SGETTWO(X,Y,Z,ERR) - get two bytes into idl & idh from cpu
 *	cpu addresses are stored in X (bits 7-0)
 *		Y (bits 15-8) and Z (bits 17-16)
 *	If error, go to ERR.
 *	Does NOT wait for UNIBUS to complete
 *	When exit, cpu addresses are incremented by 2
 *	Using r0, brg, npr, ial, iah, idl, idh, mar, mem, X, Y, Z.
 */
#define	SGETTWO(X,Y,Z,ERR)\
	mov	X,brg;\
	mov	brg,ial;\
	mov	Y,brg;\
	mov	brg,iah;\
	asl	Z,brg;\
	mov	brg,r0;\
	asl	r0;\
	mov	NRQ,brg;\
	or	brg,r0,npr;\
	INCA(X,Y,Z,2);\
	BUSWAIT
 
/*
 * GETTWO(X,Y,Z,ERR) - get two bytes into idl & idh from cpu
 *	cpu addresses are stored in X (bits 7-0)
 *		Y (bits 15-8) and Z (bits 17-16)
 *	If error, go to ERR.
 *	When exit, cpu addresses are incremented by 2
 *	Using r0, brg, npr, ial, iah, idl, idh, mar, mem, X, Y, Z.
 */
#define	GETTWO(X,Y,Z,ERR)\
	SGETTWO(X,Y,Z,ERR);\
	BUSWAIT
 
/*
 * MOD1(X,Y,Z) : Z = (X==Y)? 0 : X
 * X, Y, Z are scratch pad registers
 * X, Y won't be changed when returned
 * Using brg
 */
#define	MOD1(X,Y,Z)\
	mov	X,brg;\
	addn	brg,Y,-;\
	brz	8f;\
	br	9f;\
8:	mov	0,brg;\
9:	mov	brg,Z

/*
 * MOD0(X,Y,Z) - Z = (X<0)? (X+Y) : (X)
 * X, Y, Z are scratch pad registers
 * X, Y won't be changed when returned
 * Using brg
 */
#define	MOD0(X,Y,Z)\
	mov	X,brg;\
	br7	8f;\
	br	9f;\
8:	add	brg,Y,brg;\
9:	mov	brg,Z

/*
 * TIMES(W,X,Y,Z) - Y(hibyte)Z(low) = W * X
 * W, X, Y, Z are scratch pad registers
 * W, X, Y, Z can't be the same
 * X, Y, Z can't be r0
 * X, W won't be changed when returned
 * Using brg, r0
 */
#define	TIMES(W,X,Y,Z)\
	mov	0,brg;\
	mov	brg,Y;\
	mov	brg,Z;\
	mov	W,brg;\
	mov	brg,r0;\
	mov	X,brg;\
8:	dec	r0;\
	brz	9f;\
	add	brg,Z;\
	adc	Y;\
	br	8b;\
9:

/*
 * BIT(X,Y) - bit pattern - Y = (1 << (x))
 * X, Y are scratch pat registers and can't be the same
 * usually 0 <= X <= 7
 */
#define	BIT(X,Y)\
	mov	1,brg;\
	mov	brg,Y;\
8:	dec	X;\
	brz	9f;\
	asl	Y;\
	br	8b;\
9:

/*
 * SENDDATA(HI,LO) : send HI to lur1 and LO to lur0
 *	using brg, lur0, lur1, lur2 (if KDI)
 *	using brg, odl, odh, oal, oah, mem, mar, npr, nprx, r0 (DR11C)
 */
#define	SENDDATA(HI,LO)	\
	mov	HI,brg;\
	mov	brg,OHI;\
	mov	LO,brg;\
	mov	brg,OLO;\
	SENDD

#ifdef	KDI
#ifdef CPM422
/*
 *	cpm422 can't send an envelop within 2us
 *	using brg, r0
 */
#define D422(X)	DELAY(X)
#else
#define	D422(X)
#endif
/*
 * DELAY(X):  some time
 *	delay 400*(X+1) ns
 *	using r0, brg
 */
#define	DELAY(X) \
	mov	X,brg;\
	mov	brg,r0;\
9:	dec	r0;\
	brc	9b

/*
 * READDATA(LOC) : read data in lur0 (lo byte) and lur5 (hi byte)
 *	if data is not ready (i.e. DKFRDY (bit 0) is not set), goto LOC.
 *	using lur0, lur1, lur5, lur6, brg
 */
#define	READDATA(LOC)	\
	mov	lur6,brg;\
	br0	8f;\
	br	LOC;\
8:

/*
 * SENDD : OLO and OHI have been loaded, just trigger lur2
 */
#define	SENDD\
	mov	0,brg;\
	mov	brg,lur2	/* strobe */

/*
 * SENDHEAD(CHAN): send DKMARK with channel #
 *	using brg, lur0, lur1, lur2
 */
#define	SENDHEAD(CHAN)\
	SENDDATA(D_WHEAD,CHAN)

/*
 * SENDPACK(SEQ) : send seq# to seq fifo
 *	SEQ is in a scratch pad register
 *	using brg, lur1, lur2, SEQ
* using also lur6 to check that the transmitter becomes
* ready again.  We busy wait until it does.
* We the read the sequence number so that the busy
* wait loop will be valid the next time
 */
#define	SENDPACK(SEQ)	\
	mov	D_XPACK,brg;\
	mov	brg,lur1;\
	SENDD;\
1:	mov	lur6,brg;\
	br1	1f;\
	br	1b;\
1:	mov	D_OSEQ,brg;\
	mov	brg,lur1;\
	mov	lur1,brg	/*read strobe*/

/*
 * SENDCMD(CMD)
 */
#define	SENDCMD(CMD)\
	mov	CMD,brg;\
	mov	brg,lur1;\
	D422(3)

#else
/*
 * DR11-C specific macros
 */
/*
 * DELAY(X): DR11C is slow enough and don't need delay
 */
#define DELAY(X)

/*
 * READCMD : read csr word in the DR-11C
 * Input: csraddr is the csr unibus address
 * Output: idl (low-byte) and idh (hi-byte) of the csr
 * Using ial, iah, mar, mem, r0, brg, npr
 */
#define	READCMD\
	mov	%csraddr,%mar;\
	mov	csraddr,mar;\
	READDR
 
/*
 * READDATA : read dki word of the DR-11C
 * Input: dkiaddr is the dki unibus address
 * Output: idl (low-byte) and idh (hi-byte) of the dki
 * Using ial, iah, mar, mem, r0, brg, npr
 */
#define	READDATA(LOC)\
	READCMD;\
	mov	idh,brg;\
	br7	9f;\
	br	LOC;\
9:;\
	mov	%dkiaddr,%mar;\
	mov	dkiaddr,mar;\
	READDR
 
/*
 * READDR : read a word whose unibus addr is pointed by mar
 */
#define	READDR\
	mov	mem,ial|mar++;\
	mov	mem,iah|mar++;\
	mov	mem,r0;\
	asl	r0;\
	asl	r0;\
	mov	NRQ,brg;\
	or	brg,r0,npr;\
	BUSWAIT
 
/*
/*
 * WRITEDR : write odl and odh to unibus addr pointed by mar
 */
#define	WRITEDR\
	mov	nprx,r0;\
	mov	~(BRQ|ACLO|(3<<2)),brg;\
	and	brg,r0,brg;\
	mov	mem,oal|mar++;\
	mov	mem,oah|mar++;\
	mov	mem,r0;\
	asl	r0;\
	asl	r0;\
	or	brg,r0,nprx;\
	mov	OUT|NRQ,brg;\
	mov	brg,npr;\
	BUSWAIT
 
/*
 * SENDCMD : Issue a DR-11C command by outputing csr
 * Using odl, odh, oal, oah, mar, mem, brg, npr, nprx, r0
 */
#define	SENDCMD(CMD)\
	mov	CMD,brg;\
	mov	brg,odl;\
	mov	0,brg;\
	mov	brg,odh;\
	mov	%csraddr,%mar;\
	mov	csraddr,mar;\
	WRITEDR

/*
 * SENDD: OHI, OLO have been loaded
 * Using odl, odh, oal, oah, mar, mem, brg, npr, nprx, r0
 */
#define	SENDD\
	mov	%dkoaddr,%mar;\
	mov	dkoaddr,mar;\
	WRITEDR

/*
 * SENDHEAD(CHAN) : send DKMARK with channel #
 *	channel # can't be in r0
 *	using odl, odh, oal, oah, mar, %mar, mem, brg, npr, nprx, r0
 */
#define	SENDHEAD(CHAN) \
	SENDCMD(D_WRITE);\
	SENDDATA(D_WHEAD,CHAN)

/*
 * SENDPACK(SEQ) : send seq# to seq fifo
 *	SEQ can't be in r0
 *	using odl, odh, oal, oah, mar, %mar, mem, brg, npr, nprx, r0
 */
#define	SENDPACK(SEQ) \
	SENDCMD(D_XPACK);\
	SENDDATA(SEQ,ZERO)

#endif

/*
 * Data definitions
 */
	.data
/*
 * Global data structures
 */
	.org	0
/*
 * check point code
 */
check:	.byte	0,0,0,0,0,0,0
/*
 * 1 ms timer
 * 16 times 50-us timer expires ==> set this timer
 */
clk1ms:		.byte	0
/*
 * 5 sec timer : 4096 * 1ms
 * Every time clk1ms expires (1ms gone by), clk5s gets decremented
 */
clk5s:		.byte	0,0
/*
 * temp storage
 */
tmp:	.byte	0,0,0,0,0,0,0,0,0,0,0,0
/*
 * return code: code, lenlo, lenhi, cntl, mode
 */
repinfo:	.byte	0
replen:		.byte	0,0
repctl:		.byte	0
repmode:	.byte	0
/*
 * report seq#
 */
kseq:		.byte	0
/*
 * # of bad packets received so far
 */
badpack:	.byte	0
/*
 * temp storage for output routine
 */
blklen:		.byte	0
/*
 * a flag indicates a cntl char was rcv (used in subr.s)
 */
icntl:		.byte	0
/*
 * input state: bit7 - RBLEN>0; bit0 - 1st byte of a pair is now in odl
 */
iflag:		.byte	0
/*
 * chan#
 */
chan:		.byte	0
/*
 * seq#
 */
pseq:		.byte	0
/*
 * save r13/r12/r1/... when calling 'rcv'
 */
rcvsave:	.byte	0,0,0,0
/*
 * save area for r12 in report
 */
repsave:	.byte	0,0,0,0
/*
 * buffer map: 16 locations indicate 16*8 (128) buffers, each 128 bytes
 */
bmap0:		.byte	0
bmap:		.org	.+16
/*
 * timer map: for max 96 channels
 */
timemap:	.org	.+12
/*
 * bit pattern
 */
bitpatt:	.byte	BIT0,BIT1,BIT2,BIT3,BIT4,BIT5,BIT6,BIT7
/*
 * starting addresses of host buffer
 */
baddr:		.byte	0,0,0
/*
 * Addresses for cmdbuf, tail of cmdbuf, statbuf, head of statbuf,
 */
cbaddr:		.byte	0,0,0
cbtail:		.byte	0,0,0
sbaddr:		.byte	0,0,0
sbhead:		.byte	0,0,0
/*
 * CALL/RETURN return address
 */
s.send:		.byte	0,0
s.rcv:		.byte	0,0
s.report:	.byte	0,0
s.clean:	.byte	0,0
s.copy1:	.byte	0,0
s.cktimer:	.byte	0,0
s.strailer:	.byte	0,0
/*
 * dr11c specific
 */
#ifdef DR11C
/*
 * every time PCLK expires, r_timer gets decremented.
 * if r_timer reaches -1, then read CSR to check R/TDONE
 */
r_timer:	.byte	0
/*
 * UNIBUS addresses for the DR11C
 * Bits 7-0 in 1st byte, 15-8 in 2nd, 17-16 in 3rd (right-adjust)
 */
csraddr:	.byte	0,0,0
dkoaddr:	.byte	0,0,0
dkiaddr:	.byte	0,0,0
#endif
/*
 * output active queue
 */
outq:		.byte	NIL
outq1:		.byte	NIL		/* work area */
 
/*
 * Queue entries
 */
inqueue:
	.org	4*256
/*
 * LTE tables from page 4 to 15, each entry is 32 bytes
 */
dzst:
/*
 * Instruction text starts here
 */
	.text
/*
 * Segment 0
 *
 * This is the main segment
 */
	.org	0
seg0:
init:
/*
 * Set csr0 to idle state (i.e. 0)
 */
	mov	0,brg
	mov	brg,csr0
/*
 * initialize all global variables
 */
	mov	brg,mar
	mov	brg,%mar
	mov	inqueue-1,brg
	mov	brg,r0
init1:
	mov	0,mem|mar++
	dec	r0
	brc	init1
/*
 * initialize special variables (~0)
 */
	mov	outq,mar
	mov	%outq,%mar
	mov	NIL,mem|mar++
	mov	NIL,mem|mar++
	mov	%bitpatt,%mar
	mov	bitpatt,mar
	mov	7,r7
	mov	1,r1
1:
	mov	r1,mem|mar++
	dec	r7
	brz	2f
	asl	r1
	br	1b
2:
/*
 * Initialize the free-buffer list
 * page 0-7 is for global data and queues
 * page 8-15 for LTE entries
 */
#define	NLTE	(CHANMODS+7)/8
#define	NPQ	4
#define	NQE	((NPQ*256-((inqueue+3)&~3))/4)
/*
 * Initialize the list of available queue entries
 */
	mov	NIL,brg
	mov	brg,r14
	mov	(inqueue+3)&~3,brg
	mov	brg,r0
	mov	NQE-2,brg
	mov	brg,r1
init4:
	FREEQ(r0)
	mov	4,brg		/* queue is 4-byte long */
	add	brg,r0
	adc	r0
	dec	r1
	brc	init4
/*
 * Initialize LTE queue pointers
 */
	mov	CHANMODS-1,brg
	mov	brg,r9
init6:
/*
 * reset all LTE values: (can be done globally)
 */
	GLTE(r9)
	mov	31,brg			/* 32: size of LTE */
	mov	brg,r0
1:
	mov	0,mem|mar++
	dec	r0
	brc	1b
/*
 * initialized C_X to idle and RQ/RB to nil
 * initailize whatever else is needed
 */
	mov	r8,%mar
	mov	C_X,brg
	add	brg,r10,mar
	mov	XIDLE,mem
	mov	C_RQ,brg
	add	brg,r10,mar
	mov	NIL,mem
	mov	C_RB,brg
	add	brg,r10,mar
	mov	NIL,mem
	dec	r9
	brc	init6
/*
 * Go to disp
 */
	br	disp
/*
 * Dispatcher loop--keep looking for something to do
 */
disp:
/*
 * If csr0 == 0, does nothing and loop.
 * If csr0 == 1, read addresses from csr2~7.
 * If csr0 == 2, normal operation (i.e. take command from
 *	cmdbuff and put report to statbuf).
 */
	mov	csr0,brg
	mov	3,r3
	and	brg,r3,brg
	br1	disp2		/* normal operation */
	br0	disp1		/* direct command */
	mov	brg,r0
	dec	r0,-
	brz	disp		/* idle */
	ERROR(E_SW)
disp1:
/*
 * Get init information from structure pointed by csr2~4
 * struct init {
 *	paddr_t	*cmdaddr;
 *	paddr_t	*stataddr;
 *	paddr_t	*bufaddr
 *	paddr_t	*csraddr
 * };
 */
	mov	csr2,r13
	mov	csr3,r12
	mov	csr4,r11
	GETTWO(r13,r12,r11,err_bus)
	mov	idl,r5
	GETTWO(r13,r12,r11,err_bus)
	mov	cbaddr,mar
	mov	%cbaddr,%mar
	mov	idl,mem|mar++		/* cbaddr */
	mov	idh,mem|mar++
	mov	r5,mem|mar++
	mov	idl,mem|mar++		/* cbtail */
	mov	idh,mem|mar++
	mov	r5,mem|mar++
	GETTWO(r13,r12,r11,err_bus)
	mov	idl,r5
	GETTWO(r13,r12,r11,err_bus)
	mov	sbaddr,mar
	mov	%sbaddr,%mar
	mov	idl,mem|mar++		/* sbaddr */
	mov	idh,mem|mar++
	mov	r5,mem|mar++
	mov	idl,mem|mar++		/* sbhead */
	mov	idh,mem|mar++
	mov	r5,mem|mar++
	GETTWO(r13,r12,r11,err_bus)
	mov	idl,r5
	GETTWO(r13,r12,r11,err_bus)
	mov	baddr,mar
	mov	%baddr,%mar
	mov	idl,mem|mar++		/* bufaddr */
	mov	idh,mem|mar++
	mov	r5,mem|mar++
#ifdef KDI
/*
 * Issue D_RESET command and set dko to 0 to clear all fifo's
 */
	mov	D_RESET,brg
	mov	brg,lur1
	mov	brg,lur2
#else
/*
 * Record the unibus address (csr) for this DR-11
 * addr(dko) = addr(csr) + 2
 * addr(dki) = addr(csr) + 4
 */
	GETTWO(r13,r12,r11,err_bus)
	mov	idl,r5
	GETTWO(r13,r12,r11,err_bus)
	mov	idl,r7
	mov	idh,r6
	mov	3,brg
	mov	brg,r5		/* io page */
	mov	%csraddr,%mar
	mov	csraddr,mar
	mov	r7,mem|mar++
	mov	r6,mem|mar++
	mov	r5,mem|mar++
/*
 * Now mar points to dkoaddr
 */
	mov	2,brg
	add	brg,r7,mem|mar++
	adc	r6,mem|mar++
	adc	r5,mem|mar++
/*
 * Now mar points to dkiaddr
 */
	mov	4,brg
	add	brg,r7,mem|mar++
	adc	r6,mem|mar++
	adc	r5,mem
/*
 * Issue D_OSEQ command and set dko to 0 to clear all fifo's
 */
	mov	D_OSEQ,brg
	mov	brg,odl
	mov	0,brg
	mov	brg,odh
	SENDCMD(D_OSEQ)
	SENDDATA(ZERO,ZERO)
#endif
/*
 * Now set csr0 to 2 (i.e. normal operation)
 * Set head/tail of all circular buffers to 0
 */
	mov	0,brg
	mov	brg,csr2
	mov	brg,csr3
	mov	brg,csr4
	mov	brg,csr5
	mov	brg,csr6
	mov	brg,csr7
	mov	2,brg
	mov	brg,csr0
	br	disp

disp2:
/*
 * This is normal operation loop
 */
	mov	csr0,brg
	br0	disp2
/*
 * assume kmc is fast enough so that cmdbuf never overflows
 * If HC==TC, then no command available and loop
 */
	mov	HC,brg		/* head of cmdbuf */
	mov	TC,r0		/* tail of cmdbuf */
	addn	brg,r0,-
	brz	disp3
/*
 * Read the command pointed by TC
 * Put k_type in r1, k_chan in r9, k_len in r7-r6,
 * k_ctl/time in r5, k_mode in r4, k_addr in r3-r2-idl-idh
 */
	mov	cbtail,mar
	mov	%cbtail,%mar
	mov	mem,r13|mar++
	mov	mem,r12|mar++
	mov	mem,r11|mar++
	GETTWO(r13,r12,r11,err_bus)
	mov	idl,r1
	GETTWO(r13,r12,r11,err_bus)
	mov	idl,r9
	GETTWO(r13,r12,r11,err_bus)
	mov	idl,r7
	mov	idh,r6
	GETTWO(r13,r12,r11,err_bus)
	mov	idl,r5
	mov	idh,r4
	GETTWO(r13,r12,r11,err_bus)
	mov	idl,r3
	mov	idh,r2
	GETTWO(r13,r12,r11,err_bus)
/*
 * Now modify TC and cbtail.
 * If TC==BUFSIZ-1, then TC=0 and cbtail=cbaddr
 */
	mov	TC,r0
	mov	BUFSIZ-1,brg
	sub	brg,r0,-	/* TC-(BUFSIZ-1) */
	brc	1f
	inc	r0,TC
	br	2f
1:	mov	0,brg		/* reset TC */
	mov	brg,TC
	mov	cbaddr,mar
	mov	%cbaddr,%mar
	mov	mem,r13|mar++
	mov	mem,r12|mar++
	mov	mem,r11
2:
/*
 * Restore cbtail
 */
	mov	cbtail,mar
	mov	%cbtail,%mar
	mov	r13,mem|mar++
	mov	r12,mem|mar++
	mov	r11,mem|mar++
/*
 * Decode the command
 * switch (k_type) {
 * case	KC_SEND:	goto kc_send;
 * case	KC_RCVB:	goto kc_rcvb;
 * case	KC_SOI:		goto kc_soi;
 * case KC_CMD:		goto kc_cmd;
 * case KC_FLAG:	goto kc_flag;
 * case KC_TDK:		goto kc_tdk;
 * case KC_XINIT:	goto kc_xinit;
 * case KC_CLOSE:	goto kc_close;
 * case KC_INIT:	goto kc_init;
 * }
 */
	GLTE(r9)		/* set up r8/r10 */
	mov	KC_SEND,brg
	addn	brg,r1,-
	brz	kc_send
	mov	KC_RCVB,brg
	addn	brg,r1,-
	brz	kc_rcvb
	mov	KC_SOI,brg
	addn	brg,r1,-
	brz	kc_soi
	mov	KC_CMD,brg
	addn	brg,r1,-
	brz	kc_cmd
	mov	KC_FLAG,brg
	addn	brg,r1,-
	brz	kc_flag
	mov	KC_TDK,brg
	addn	brg,r1,-
	brz	kc_tdk
	mov	KC_XINIT,brg
	addn	brg,r1,-
	brz	kc_xinit
	mov	KC_CLOSE,brg
	addn	brg,r1,-
	brz	kc_close
	mov	KC_INIT,brg
	addn	brg,r1,-
	brz	kc_init
	ERROR(E_CMD)
	br	disp3

kc_init:
/*
 * Process a KC_INIT command
 *	r5 is either D_CALL or D_IDL1
 *	C_X = XINIT; goto kc_reset
 */
	CHK1(B0_KCIT,0)
	mov	C_X,brg
	add	brg,r10,mar
	mov	XINIT,mem
	br	kc_reset		/* expect to rcv D_TALK from ty-4 */

kc_xinit:
/*
 * Process a KC_XINIT command
 *	C_X = XINIT; C_XLEN = 0; send(INIT1);
 */
	CHK1(B0_KCIT,0)
	mov	C_X,brg
	add	brg,r10,mar
	mov	XINIT,mem|mar++
/*
	mov	0,mem
 */
	mov	C_XLEN,brg
	add	brg,r10,mar
	mov	0,mem|mar++
	mov	0,mem
	mov	C_XCNTL,brg
	add	brg,r10,mar
	mov	0,mem
	mov	I_INIT1,brg
	mov	brg,r1
	CALL(send,s.send)		/* can simply goto 'time' */
	br	disp3

kc_close:
/*
 * process KC_CLOSE command
 *	k_chan is the channel #
 *	if (C_RLEN) report(KS_RDB, chan);
 *	goto kc_reset
 */
	CHK1(B0_KCCS,0)
	mov	C_X,brg
	add	brg,r10,mar
	mov	XIDLE,mem|mar++
	mov	0,mem		/* C_X1 = 0 */
	mov	C_RLEN,brg
	add	brg,r10,mar
	mov	mem,r7|brg
	mov	brg,r0|mar++
	mov	mem,r6
	or	mem,r0
	dec	r0,-
	brz	kc_reset
/*
 * report (KS_RDB,chan) with mode SABORT
 */
	mov	repinfo,mar
	mov	%repinfo,%mar
	mov	KS_RDB,mem|mar++
	mov	r7,mem|mar++		/* residue length */
	mov	r6,mem|mar++
	mov	0,mem|mar++		/* cntl char */
	mov	SABORT,mem		/* RCV mode */
	CALL(report,s.report)
kc_reset:
/*
 * Reset RLEN, XLEN.
 * while (RQ!=NIL) free the queues and buffers
 */
	mov	r8,%mar
	mov	C_RLEN,brg
	add	brg,r10,mar
	mov	0,mem|mar++
	mov	0,mem
	mov	C_RBLEN,brg
	add	brg,r10,mar
	mov	0,mem|mar++
	mov	0,mem
	mov	C_TA0,brg
	add	brg,r10,mar
	mov	0,mem|mar++
	mov	0,mem
	mov	C_C,brg
	add	brg,r10,mar
	mov	0,mem
	mov	C_RQ,brg
	add	brg,r10,mar
	mov	mem,r2
	brz	kc_rs2
	mov	NIL,mem
	CALL(clean,s.clean)
	mov	r8,%mar
kc_rs2:
	mov	C_RB,brg
	add	brg,r10,mar
	mov	mem,r2
	brz	kc_rs4
	mov	NIL,mem
	CALL(clean,s.clean)
kc_rs4:
	mov	r8,%mar
	mov	C_XLEN,brg
	add	brg,r10,mar
	mov	0,mem|mar++
	mov	0,mem
	mov	C_XCNTL,brg
	add	brg,r10,mar
	mov	0,mem
/*
 * remove chan# from 'outq'
 * r2 is running pointer for outq (currently looked at)
 * r3 is the next outq entry
 * r4 is the running pointer for outq1
 */
	mov	outq,mar
	mov	%outq,%mar
	mov	mem,r2
kc_rs6:
	brz	kc_rs14
	QA(r2)
	mov	mem,r3
	mov	NIL,mem|mar++
	addn	mem,r9,-
	brz	kc_rs12			/* delete this chan# */
	mov	outq1,mar
	mov	%outq1,%mar
kc_rs8:
	mov	mem,r4
	brz	kc_rs10
	QA(r4)
	br	kc_rs8			/* try to find the end of outq1 */
kc_rs10:
/*
 * found the end of outq1, append r2 here
 */
	mov	r2,mem
kc_rs11:
/*
 * put r3 in r2 position
 */
	mov	r3,brg
	mov	brg,r2
	br	kc_rs6
kc_rs12:
	FREEQ(r2)
	br	kc_rs11
kc_rs14:
/*
 * restore outq from outq1
 */
	mov	outq1,mar
	mov	%outq1,%mar
	mov	mem,brg
	mov	outq,mar
	mov	%outq,%mar
	mov	brg,mem|mar++
	mov	NIL,mem
	br	disp3
kc_soi:
/*
 * Process a KC_SOI command
 *	send 2 bytes in r7 (lo) and r6 (hi) to chan r9
 *	bypass error/flow control mechanism
 */
	CHK1(B0_KCSO,0)
	mov	tmp,mar
	mov	%tmp,%mar
	mov	r9,mem|mar++
	mov	D_WHEAD,mem|mar++
	mov	I_SOI,mem|mar++
	mov	D_WCNTL,mem|mar++
	mov	r7,mem|mar++
	mov	D_WDATA,mem|mar++
	mov	r6,mem|mar++
	mov	D_WDATA,mem|mar++
	mov	I_EOI,mem|mar++
	mov	D_WCNTL,mem|mar++
	mov	ZERO,mem|mar++		/* seq# */
	mov	D_XPACK,mem|mar++
#ifdef DR11C
	SENDCMD(D_WRITE)		/* for dr11-c */
#endif
	mov	0,brg
	mov	brg,r5
kc_so1:
	mov	%tmp,%mar
	mov	tmp,brg
	add	brg,r5,mar
	mov	mem,OLO|mar++
	mov	mem,OHI|mar++
	SENDD
	inc	r5
	inc	r5
	mov	10,brg
	addn	brg,r5,-
	brz	kc_so3		/* issue XPACK */
	brc	disp3
	br	kc_so1
kc_so3:
	SENDCMD(D_XPACK)	/* dor dr11-c */
	br	kc_so1

kc_send:
/*
 * Process a KC_SEND command
 * The format is:
 *	k_chan is the chan #
 *	r7-r6: XLEN; r5: XCNTL; r4: mode; idl-idh-r3: XADDR;
 */
	CHK1(B0_KCSD,0)
/*
 * If this chan already in output waiting (C_XLEN!=0), error
 */
	mov	C_XLEN,brg
	add	brg,r10,mar
	mov	mem,r0|mar++
	or	mem,r0
	mov	C_XCNTL,brg
	add	brg,r10,mar
	or	mem,r0
	dec	r0,-
	brz	1f
	br	err_dup
1:
/*
 * Fill LTE info
 */
	mov	C_XLEN,brg
	add	brg,r10,mar
	mov	r7,mem|mar++	/* C_XLEN */
	mov	r6,mem|mar++
	mov	idl,mem|mar++	/* C_XADDR */
	mov	idh,mem|mar++
	mov	r3,mem|mar++
	mov	C_XCNTL,brg
	add	brg,r10,mar
	mov	r5,mem
/*
 * if mode = XBOTM (i.e. 1<<7), set this bit in C_X1;
 * otherwise reset this bit
 */
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	~XBOTM,brg
	or	brg,r4,-
	brz	1f
	and	brg,r0,mem
	br	2f
1:
	orn	brg,r0,mem
2:
/*
 * if xlen (r7,r6) and xcntl (r5) are all 0, set XNIL for sending
 * trailer only
 */
	mov	r7,brg
	or	brg,r6,brg
	or	brg,r5
	dec	r5,-
	brz	kc_sn6
/*
 * if the channel is not active (i.e. hasn't rcv ainit)
 */
	mov	C_X,brg
	add	brg,r10,mar
	mov	mem,brg
	br1	kc_sn2		/* XACT is set */
	br	disp3
kc_sn2:
/*
 * this channel is output active: put it in 'outq'
 */
	GETQ(r1,noqb)
	mov	NIL,mem|mar++
	mov	r9,mem
	mov	outq,mar
	mov	%outq,%mar
kc_sn3:
	mov	mem,r2
	brz	kc_sn5
	QA(r2)
	br	kc_sn3
kc_sn5:
	mov	r1,mem
	br	disp3
kc_sn6:
/*
 * send trailer mode is on (i.e. 0 data byte, no cntl byte)
 */
	mov	C_X,brg
	add	brg,r10,mar
	mov	mem,brg|mar++
	mov	mem,r7
	mov	XNIL,mem
	or	mem,r7,mem
	br1	kc_sn7
	br	disp3		/* hasn't rcv ainit */
kc_sn7:
	CALL(strailer,s.strailer)
	br	disp3

kc_rcvb:
/*
 * process a receive-buffer command
 * The format is:
 *	k_chan is the chan# to be input
 *	r7-r6: RLEN; r5: INITIM; r4: mode; idl-idh-r3: RADDR;
 */
	CHK1(B0_KCRB,0)
	mov	C_RLEN,brg
	add	brg,r10,mar
	mov	mem,r13
	mov	r7,mem|mar++
	mov	mem,r12
	mov	r6,mem|mar++
	mov	idl,mem|mar++		/* RADDR */
	mov	idh,mem|mar++
	mov	r3,mem
	mov	C_INITIM,brg
	add	brg,r10,mar
	mov	r5,mem
	mov	C_Y,brg
	add	brg,r10,mar
	mov	r4,mem
/*
 * If there was an unfinished read-buf, report it
 * i.e. if C_RLEN > 0
 */
	mov	r13,brg
	or	brg,r12,brg
	mov	brg,r0
	dec	r0,-
	brz	kc_rc4
/*
 * abort the old pending read
 * report whatever having been received
 */
	mov	%repinfo,%mar
	mov	repinfo,mar
	mov	KS_RDB,mem|mar++
	mov	r13,mem|mar++
	mov	r12,mem|mar++		/* residue length */
	mov	0,mem|mar++		/* cntl */
	mov	SABORT,mem		/* mode */
	CALL(report,s.report)
kc_rc4:
/*
 * if (char mode && YTIME && (RB != NULL))  set YEXPIRE
 */
	mov	r8,%mar
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	XBLK,brg
	orn	brg,r0,-
	brz	kc_rc7
	mov	C_Y,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	YTIME,brg
	orn	brg,r0,-
	brz	1f
	mov	C_RB,brg
	br	kc_rc8
1:
	mov	C_RB,brg
	add	brg,r10,mar
	mov	mem,brg
	brz	disp3
/*
 * set YEXPIRE
 */
	mov	C_Y,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	YEXPIRE,brg
	or	brg,r0,mem
kc_rc6:
/*
 * call rcv to copy RQ/RB to RADDR
 */
	CALL(rcv,s.rcv)			/* copy C_RQ to RADDR */
	br	disp3
kc_rc7:	mov	C_RQ,brg
kc_rc8:	add	brg,r10,mar
	mov	mem,brg
	brz	disp3
	br	kc_rc6

kc_cmd:
/*
 * process KC_CMD command
 *	k_chan is the chan #
 *	r7 is the subcommand
 *	only OFLUSH is implemented, others will be added later
 */
	CHK1(B0_KCCM,0)
	br	kc_rs4

kc_flag:
/*
 * process KC_FLAG command
 *	k_chan is the chan #
 *	1st two bytes (r7-r6) are iflag, next two (r5-r4) are oflag
 * TO BE DONE LATER
	CHK1(B0_KCFG,0)
 */
br disp3

kc_one:
/*
 * process KC_ONE command
 *	k_chan is the chan #
 *	send one byte (control byte) in r7 to ty-4
 * TO BE ADDED LATER
	CHK1(B0_KC1,0)
 */
br disp3

kc_tdk:
/*
 * process KC_TDK command
 *	k_chan is the chan #
 * TO BE ADDED LATER
 */
br disp3

kc_stime:
/*
 * process KC_STIME command
 *	k_chan is the chan #
 *	r7 is the timer value
 * TO BE ADDED LATER
 */
br disp3

disp3:
#ifdef KDI
/*
 * check input data ready and output completion
 * input ready is indicated by DKFRDY (bit 0)
 * output ready is indicated by TRQRDY (bit 1)
 */
	mov	lur6,brg
	br0	csrck2
/*
 * since we are not reading seq fifo, bit 1 is always set
	br1	csrck4
 */
#endif
	BRP(nprx,PCLK,tick)
	br	disp2

tick:
/*
 * 50-us timer expires, restart the timer
 */
	mov	nprx,r0
	mov	~(BRQ|ACLO),brg
	and	brg,r0
	mov	PCLK,brg
	or	brg,r0,nprx
#ifdef DR11C
/*
 * Dec the r_timer, and goto r_expire if r_timer reaches -1
 */
	mov	r_timer,mar
	mov	%r_timer,%mar
	mov	mem,r0
	dec	r0,mem
	brz	r_expire
	br	tick1

r_expire:
/*
 * reset r_timer
 */
	mov	R_TIMER,mem
/*
 * check input data ready, then output completion
 */
	READCMD			/* get csr in idl & idh */
/*
 * If there is an input character available then go to csrck2
 * DKRDONE is 1<<15 in dkcsr
 */
	mov	idh,brg
	br7	csrck2
/*
 * If there is an output data request then go to csrck4
 * DKTDONE is 1<<7 in dkcsr
	mov	idl,brg
	br7	csrck4
 */
#endif
tick1:
/*
 * dec the 1ms timer and return if the result is non-negative
 */
	mov	clk1ms,mar
	mov	%clk1ms,%mar
	mov	mem,r0
	dec	r0,mem
	brc	tick10
	mov	CLK1MS,mem	/* restore timer */
	mov	NGRP,brg
	mov	brg,r6
tick2:
/*
 * 1ms timer expires: dec ITIME for each input chan which on timemap
 * r6: row, r7: column, r5: map itself
 */
	mov	%timemap,%mar
	mov	timemap,brg
	add	brg,r6,mar
	mov	mem,r5|brg|mar++
	mov	7,r7
tick3:
	dec	r5,-
	brz	tick4
tick3.2:
	br7	tick5
	asl	r5,r5|brg
	dec	r7
	br	tick3.2
tick3.5:
	asl	r5,r5|brg
	dec	r7
	brc	tick3
tick4:
	dec	r6
	brc	tick2
	br	tick9
tick5:
/*
 * found this channel (r6<<2 + r7) is timer active (i.e. YTIMACT)
 */
	mov	r6,brg
	mov	brg,r9
	asl	r9
	asl	r9
	asl	r9
	mov	r7,brg
	add	brg,r9		/* r9 is the chan # */
	GLTE(r9)
	mov	C_Y,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	YTIME,brg
	orn	brg,r0,-
	brz	tick7
	br	timeerr
tick7:
/*
 * timer processing is enabled for this chan
 */
	mov	YEXPIRE,brg
	orn	brg,r0,-
	brz	timeerr
	mov	YTIMACT,brg
	orn	brg,r0,-
	brz	1f
	br	timeerr			/* timer not active */
1:
/*
 * timer is active, dec ITIME
 */
	mov	C_ITIME,brg
	add	brg,r10,mar
	mov	mem,r0
	dec	r0,mem
	brc	tick3.5		/* not reach zero yet */
/*
 * timer expires: set YEXPIRE
 */
	mov	C_Y,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	YEXPIRE,brg
	or	brg,r0,mem
/*
 * YEXPIRE is set: call 'rcv' 
 */
	mov	%rcvsave,%mar
	mov	rcvsave,mar
	mov	r5,mem|mar++
	mov	r6,mem|mar++
	mov	r7,mem|mar++
	CALL(rcv,s.rcv)
	mov	%rcvsave,%mar
	mov	rcvsave,mar
	mov	mem,r5|mar++
	mov	mem,r6|mar++
	mov	mem,r7|mar++
	CHK(B5_TIME,5)
tick8:
/*
 * reset timemap for this chan
 * the following was done in 'rcv'
	mov	%bitpatt,%mar
	mov	bitpatt,brg
	add	brg,r7,mar
	mov	mem,r0
	mov	%timemap,%mar
	mov	timemap,brg
	add	brg,r6,mar
	xor	mem,r0,mem
 */
	br	tick3.5
timeerr:
/*
 * should never happen: will be removed after being debugged
 */
mov 0222,brg
mov brg,r15
br .

tick9:
/*
 * dec 5 sec timer, goto timeout if expires
 * steals this 50-us cycle and 'time' no returns
 */
	mov	clk5s,mar
	mov	%clk5s,%mar
	mov	mem,r0
	dec	r0
	mov	r0,mem|mar++
	mov	mem,r1
	mov	0,brg
	subc	brg,r1,mem
	and	mem,r0,-
	brz	time
tick10:
/*
 * if (outq) goto csrck4;
 * This is the replacement of sending a dummy packet to chan 511
 */
	mov	outq,mar
	mov	%outq,%mar
	mov	mem,r0
	brz	disp2
csrck4:
/*
 * Output data request--jump to output segment
 */
	mov	%out,brg
	mov	brg,pcr
	jmp	out
csrck2:
/*
 * Input data available--jump to input segment
 */
	mov	%in,brg
	mov	brg,pcr
	jmp	in

time:
/*
 * timeout routine
 * for each active chan, if (S != R+1) send ENQ
 */
/*
 * reinitialize the clk5s
 */
	mov	clk5s,mar
	mov	%clk5s,%mar
	mov	CLT0,mem|mar++
	mov	CLT1,mem
	mov	CHANMODS,brg
	mov	brg,r9		/* channel # */
time1:
	dec	r9
	brz	disp2
	GLTE(r9)			/* setup r8/r10 for LTE */
	mov	C_X,brg
	add	brg,r10,mar
	mov	mem,brg
	br0	time1		/* bit 0 indicates un-initialized */
	br1	time2		/* bit 1 indicates initialized */
/*
 * INIT1 was sent and no reply; keep sending INIT1
 */
	mov	I_INIT1,brg
	br	time3
time2:
/*
 * chan is active, if (S!=R+1) send ENQ
 */
	mov	C_S,brg
	add	brg,r10,mar
	mov	mem,r1|mar++
	mov	mem,r0
	inc	r0
	mov	WINDOW,brg
	and	brg,r0,brg
	addn	brg,r1,-
	brz	time1
	mov	C_X1,brg
	add	brg,r10,mar
	mov	mem,r0
	mov	~(XACK|XREJ),brg
	and	brg,r0			/* reset XACK and XREJ */
	mov	XENQ,brg
	or	brg,r0,mem		/* set XENQ */
	mov	I_ENQ,brg
time3:
	mov	brg,r1
	CALL(send,s.send)		/* argument in r9, r1 */
	br	time1
 
err_bus:
/*
 * The Unibus transfer fails to complete within 20 usec
 * Clear NEM bit, queue the error, and go to disp
 */
	mov	nprx,r0
	mov	~(BRQ|ACLO|NEM),brg
	and	brg,r0,nprx
	ERROR(E_BUS)
	br	disp
 
err_dup:
/*
 * Two SEND command
 */
	ERROR(E_DUP)
	br	disp

noqb:
/*
 * no queue or buffer
 */
	ERROR(E_NOQB)
	br	disp

errlog:
/*
 * r9 - chan#
 * brg - error code
 * after interrupt cpu, go to init and wait for reinitialization
 */
	mov	repinfo,mar
	mov	%repinfo,%mar
	mov	KS_ERR,mem|mar++
	mov	brg,mem
	CALL(report,s.report)
	br	disp2
 
/*
 * End of segment 0
 */
endseg0:
/*
 * Pick up code from other segments
 */
#include	"output.s"
#include	"input.s"
#include	"subr.s"
