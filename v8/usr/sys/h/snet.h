struct oargs {
	int lmachno;
	int lchno;
	int dmachno;
	int dlchno;
};

struct voargs {
	int lmachno;
	int lchno;
	int dmachno;
	int dlchno;
	int pdid;
};

struct cargs {
	int lmachno;
	int lchno;
};

struct gpargs {
	int lmachno;
	int lch;
	char *addr;
	int len;
	struct status *status;
};

struct gmargs {
	struct pair {
		int lmachno;
		int lchno;
	} *pairp;
	char *addr;
	int len;
	struct status *status;
};

struct raargs {
	int machno;
};

struct chargs {
	int dummy;
};

struct wargs {
	int dummy;
};

struct pargs {
	int dummy;
};

struct sargs {
	int dummy;
};

#define NUMDEV 16

#undef NUMLCH
#ifdef USG5
/* warning: NUMLCH must be identically defined in /usr/include/sys/space.h */
#define NUMLCH 9
#define MAXMUX 4
#else
#define NUMLCH 32
#define MAXMUX 32
#endif

struct sninfo {
	int snnpackets, snnbadlen;
	int snnack, snnrdy, snnrnr;
	int snndata, snnnolc, snnchksum, snnrack;
	int snsack, snsrnr, sns1rdy, snsrdy, snsdata, snsnack, snlost;
	int snlock;
	int snxcnt, snrcnt;
	int sninbytes, snoutbytes;
	int snrout, sninloop, snoutfull, sncntoutfull;
	int sndevlock[NUMDEV];
	struct chdat *snlink[NUMDEV];
	int snsched, sntimeout;
	int snenqlost[NUMLCH];
};

struct snmach {
	int machno;
	int lmachno;
};

struct openwait {
	int pid;
};
#define ownext(owptr) (owptr >= &sn_openwait[NUMDEV-1] ? &sn_openwait[0] : owptr+1)
struct openinfo {
	struct openwait *owaddr, *head, *tail;
};

/*
 * Network commands
 */
#ifdef BSD42
#define NIOOPEN 	_IOW(n, 0, struct oargs)
#define NIOCLOSE	_IOW(n, 1, struct cargs)
#define NIOGET  	_IOW(n, 2, struct gpargs)
#define NIOPUT  	_IOW(n, 3, struct gpargs)
#define NIOCHECK	_IOW(n, 4, struct chargs)
#define NIOWAIT 	_IOW(n, 5, struct wargs)
#define NIOPURGE	_IOW(n, 6, struct pargs)
#define NIOSETVEC	_IOW(n, 7, struct sargs)
#define NIOGETM		_IOW(n, 8, struct gmargs)
#define NIORESET	_IOW(n, 9, struct raargs)
#define NIOABORT	_IOW(n, 10, struct raargs)
#define NIOREADSTATUS	_IO(n, 11)
#define NIOPOPEN 	_IOW(n, 12, struct voargs)
#define NIOCHSTATUS	_IO(n, 13)
#define NIOSETMACH	_IO(n, 14)
#define NIOGETMACH	_IO(n, 15)
#define NIOXOPEN 	_IOWR(n, 16, struct voargs)
#define NIOQSTATUS	_IO(n, 17)
#define NIOZEROSTAT	_IO(n, 18)
#else
#define NIOOPEN 	(('n'<<8)|0)
#define NIOCLOSE	(('n'<<8)|1)
#define NIOGET  	(('n'<<8)|2)
#define NIOPUT  	(('n'<<8)|3)
#define NIOCHECK	(('n'<<8)|4)
#define NIOWAIT 	(('n'<<8)|5)
#define NIOPURGE	(('n'<<8)|6)
#define NIOSETVEC	(('n'<<8)|7)
#define NIOGETM  	(('n'<<8)|8)
#define NIORESET	(('n'<<8)|9)
#define NIOABORT	(('n'<<8)|10)
#define NIOREADSTATUS	(('n'<<8)|11)
#define NIOPOPEN 	(('n'<<8)|12)
#define NIOCHSTATUS	(('n'<<8)|13)
#define NIOSETMACH	(('n'<<8)|14)
#define NIOGETMACH	(('n'<<8)|15)
#define NIOXOPEN 	(('n'<<8)|16)
#define NIOQSTATUS 	(('n'<<8)|17)
#define NIOZEROSTAT 	(('n'<<8)|18)
#endif
