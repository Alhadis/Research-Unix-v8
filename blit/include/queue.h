#define	QUEUE_H
#define	NCHARS	512
#define NULL ((char *)0)
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
struct clist queues[3];
struct cbuf *freelist;

#define	RCVQUEUE	queues[0]	/* chars from host */
#define	KBDQUEUE	queues[1]	/* chars from keyboard */
#define	OUTQUEUE	queues[2]	/* chars to send to host */
