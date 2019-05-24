#define SCNRECST	(('S'<<8)|1)
#define SCNREC	(('S'<<8)|2)
#define SCNSENDST	(('S'<<8)|3)
#define SCNSEND	(('S'<<8)|4)
#define SCNSBMOD	(('S'<<8)|5)
#define SCNBPMOD	(('S'<<8)|6)
#define SCNHACK		(('S'<<8)|7)

#define SCN_SBMASK	0160
#define SCN_BPMASK	014
/*	request commands	*/
#define SCNRS_S1	0x01
#define SCNRS_S2	0x02
#define SCNRS_S3	0x03
#define SCNRS_P	0x04
#define SCNRS_S	0x05

/*	action commands	*/
#define SCN_SCAN	0x08
#define SCN_HOLD	0x09
#define SCN_HOME	0x0a
#define SCN_RUNC	0x0b
#define SCN_WAIT	0x0c
#define SCN_RESET	0x0d

/*	select commands	*/
#define SCN_FNT	0x10
#define SCN_FDT	0x11
#define SCN_FLT	0x12
#define SCN_ROM	0x13
#define SCN_RAM	0x14
#define SCN_GRAY	0x15
#define SCN_TEST	0x16
#define SCN_DEFAULT	0x17
#define SCN_ANT	0x18
#define SCN_ADT	0x19
#define SCN_ALT	0x1a
#define SCN_ATT	0x1b

/*	enable commands	*/
#define SCN_EN_PRE	0x1d
#define SCN_DIS_PRE	0x1e

/*	download commands	*/
#define SCN_CONT	0x23
#define SCN_LMEM	0x24
#define SCN_LLENG	0x25
#define SCN_RMEM	0x26
#define SCN_RLENG	0x27
#define SCN_RETM	0x28
#define SCN_DIAG	0x29

/*	replies from scanner	*/
#define SCN_NEG	0x30
#define SCN_STATE0	0x40
#define SCN_STATE1	0x41
#define SCN_STATE2	0x42
#define SCN_STATE3	0x43
#define SCN_STATE4	0x44
#define SCN_STATE5	0x45
#define SCN_STATE6	0x46
#define SCN_REC	0x48

/* sb modes */
#define SCN_MOD_BW	0140
#define SCN_MOD_0	0
#define SCN_MOD_1	020
#define SCN_MOD_2	040
#define SCN_MOD_3	060
#define SCN_MOD_4	0100
#define SCN_MOD_5	0120
#define SCN_ARM		02

#define SCN_SREADY 0200

/* receive status bits */
#define SCN_REC_DONE	0200
#define SCN_REC_IE	0100

/* send status bits */
#define SCN_SEND_DONE	0200
#define SCN_SEND_IE	0100


