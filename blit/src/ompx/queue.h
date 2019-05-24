#define	NCHARS	256
#define	NULL	0
#define	STOPPED	1

struct cbuf {
	struct	cbuf *next;
	short	word;
};
struct clist {
	struct	cbuf *c_tail;
	struct	cbuf *c_head;
	short	c_cc;
	short	state;
};


struct cbuf cbufs[NCHARS];
struct cbuf *freelist;
