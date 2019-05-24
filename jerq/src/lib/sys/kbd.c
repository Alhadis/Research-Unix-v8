#include <jerq.h>
#include <queue.h>
#include <kbd.h>
#include <setup.h>

int	kbdrepeat;
int	rptcount;
int	kbdstatus;

/*
 * All characters with high bits set are used to index this table to yield
 * the generated character. This makes TAB==^I, etc, and removes the silly
 * mappings like ^@==0xba instead of NUL.  However, ^, is still NUL.
 * For now, shifted keys are the same as unshifted, if mapped in this table;
 * for example shift RETURN is RETURN.  This may change....
 * Characters that 'cannot happen' are mapped to 'X'==0x58, to ferret out bugs.
 * Break is well known as 0x80, disconnect as 0x81.
 * This map is done at interrupt level time, so is always applied.
 */
unsigned char defkeymap[]={
/*80*/	0x58,	0x58,	0x58,	0x58,	0x58,	0x58,	0x58,	0x58,
	0x58,	0x58,	0x58,	0x58,	0x58,	0x58,	0x8e,	0x81,
/*90*/	0x90,	0x91,	0x92,	0x93,	0x94,	0x95,	0x96,	0x97,
	0x98,	0x99,	0x9a,	0x9b,	0x58,	0x58,	0x58,	0x58,
/*A0*/	0x58,	0xa1,	0xa2,	0xa3,	0xa4,	0xa5,	0xa6,	0xa7,
	0x58,	0x58,	0x58,	0x58,	0x58,	0x58,	0xae,	0x80,
/*B0*/	0xb0,	0xb1,	0xb2,	0xb3,	0xb4,	0xb5,	0x0a,	0xb7,
	0xb8,	0xb9,	0x00,	0xbb,	0x1e,	0xbd,	0xbe,	0x1f,
/*C0*/	0xc0,	0xc1,	0xc2,	0xc3,	0xc4,	0x58,	0xc6,	0x0d,
	0xc8,	0xc9,	0xca,	0xcb,	0xcc,	0xcd,	0xce,	0xcf,
/*D0*/	0x09,	0x08,	0xd2,	0xd3,	0xd4,	0xd5,	0xd6,	0xd7,
	0x58,	0x58,	0x58,	0x58,	0x58,	0x58,	0x7f,	0x58,
/*E0*/	0x58,	0x58,	0xe2,	0x1b,	0x58,	0xe5,	0x58,	0x0d,
	0xe8,	0xe9,	0xea,	0xeb,	0xec,	0xed,	0xee,	0xef,
/*F0*/	0x09,	0x08,	0xb2,	0x1b,	0x58,	0xf5,	0x0a,	0x58,
	0x58,	0x58,	0x58,	0x58,	0x58,	0x58,	0x7f,	0x58,
};

kbdchar()
{
	return qgetc(&KBDQUEUE);
}
kbdinit()
{
	/* init	the keyboard */
	DUART->b_cmnd=RESET_RECV|DIS_TX|DIS_RX;
	DUART->b_cmnd=RESET_TRANS;
	DUART->b_cmnd=RESET_ERR;
	DUART->b_cmnd=RESET_MR;
	DUART->mr1_2b=CHAR_ERR|PAR_ENB|EVN_PAR|CBITS8;
	DUART->mr1_2b=NRML_MOD|ONEP000SB;
	DUART->b_sr_csr=BD4800BPS;
	DUART->b_cmnd=RESET_MR|ENB_TX|ENB_RX;
	DUART->scc_ropbc=0x08; /* set output pins for kbd tx port*/
	/* turn chirps on/off depending on BRAM */
	if(VALKEYTONE)
		kbdstatus=0;	/* no chirp */
	else
		kbdstatus=TTY_CHIRP;	/* chirp, chirp */
	DUART->b_data=kbdstatus|0x02; /* request status */
}
auto2(){
	register unsigned s, c;
	s=DUART->b_sr_csr;
	c=DUART->b_data;
	if(s&(FRM_ERR|OVR_RUN)) 
		return;
	if(s&PAR_ERR){	/* control word: caps lock or repeat */
		checkbram();
		VALCAPS=(c&0x04)? 0 : 1; /* set the caps lock flag */
		setbram();
		if(c&0x10)	/* turn repeat off */
			kbdrepeat=rptcount=0;
		else		/* the next character is to be repeated */
			kbdrepeat=RPTON;
		/*
		 * Don't actually set the repeat bit until the character
		 * after the control code
		 */
	}else{		/* ordinary character */
		rptcount=0;	/* new char so restart repeat timer */
		if(c&0x80)
			c=defkeymap[c&0x7f];
		qputc(&KBDQUEUE, (int)c);
		if((kbdrepeat&RPTMASK) == RPTON){
			kbdrepeat=RPTHAVECHR|RPTON;
			kbdrepeat|=c;
		}
	}
}
kbdrpt(){
	qputc(&KBDQUEUE, kbdrepeat&0xff);
}
