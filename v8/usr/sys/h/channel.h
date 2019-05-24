#ifndef NUMLCH
#define NUMLCH 32
#define MAXMUX 32
#endif

struct chdat {
	int lmachno;	/* source "machine number" */
	int machno;	/* remote machine number */
	int flags;
	int lchnum;	/* source logical channel number */
	int dlchnum;	/* destination logical channel number */
	int pdid;	/* physical destination did for dest another host */
	int snunit;	/* specifies which snet this channel goes on */
	int ckticks;
	short xchan;
	struct chdat *link;
	struct bufinfo {
		short *buf;
		int len;
		int done;
	} input, output;
} ;

/* used by NIOCHSTATUS ioctl call */
struct chinfo {
	int numlch;
	struct chdat *chanaddr;
};

/* flags */
#define SENTRDY	0x01	/* we have sent an RDY */
#define SENTRNR	0x02	/* we have sent an RNR */
#define SENTDATA	0x04	/* we have sent a DATA */
#define MULTIPLEX	0x08	/* this channel is multiplexed */
#define RETRY	0x10	/* timeout - resend message */
