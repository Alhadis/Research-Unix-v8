#include	<tty.h>

#define	NPROC	8
#define	STKSIZ	2048
#define	CBSIZE	64	/* NPROC*CBSIZE <= queue.h/NCHARS */

#ifndef	QUEUE_H
struct cbuf {	/* duplicated from queue.h for simplicity */
	struct	cbuf *next;
	short	word;
};
struct clist {
	struct	cbuf *c_tail;
	struct	cbuf *c_head;
	short	c_cc;
	short	state;
};
#endif

typedef struct Proc{
	long		*sp;		/* stack pointer (really frame pointer) */
	int		(*fcn)();	/* starting address */
	int		state;		/* state bits; see below */
	Layer		*layer;		/* screen area+obscured parts */
	Rectangle 	rect;		/* rectangle on screen */
	struct clist	kbdqueue;	/* chars waiting for this process */
	long		traploc;	/* pc of fault */
	short		traptype;	/* type of fault; see Pl.s */
	int		nticks;		/* ticks pending on ALARM */
	Point		curpt;		/* for the jline etc. crowd */
	Texture		*cursor;	/* local cursor */
	short		inhibited;	/* local cursor inhibited? */
	short		nchars;		/* number of chars in input buffer */
	unsigned char	cbuf[CBSIZE*3];	/* circular buffer */
	unsigned char	*cbufpin;	/* next char to write in buffer */
	unsigned char	*cbufpout;	/* next char to remove from buffer */
	struct ttychars ttychars;	/* chars for tty driver emulation */
	char		stack[STKSIZ];	/* not big enough... */
}Proc;

#define	P	(*((struct Proc **)0406))

#ifndef	MPX_H
struct Proc *newproc();
struct Proc proctab[NPROC];
#endif

#ifdef	MPXTERM
/*
 * Data at start of user program
 */
struct udata{
	short	bra_b_start;	/* branch over data */
	Rectangle Drect;
	struct Mouse mouse;
	Layer	*Jdisplayp;
	char	**argv;
	int	argc;
};
/* states */
#define	RUN		1	/* ready to be scheduled */
#define	BUSY		2	/* active */
#define	BLOCKED		4	/* blocked by user with ^S */
#define	USER		8	/* a user-68ld'd process */
#define	KBDLOCAL	16	/* has requested the KBD */
#define	MOUSELOCAL	32	/* has requested the MOUSE */
#define	GOTMOUSE	64	/* currently owns MOUSE */
#define	WAKEUP		128	/* tell CONTROL to issue setrun(p) */
#define	MOVED		256	/* layer got moved */
#define	UNBLOCKED	512	/* Has been unblocked */
#define	ZOMBIE		1024	/* proc died horribly; waiting for debugger */
#define	RESHAPED	2048	/* layer got reshaped */
#define	ZOMBOOT		4096	/* put in ZOMBIE state after booting */
#define	ALARMREQD	8192	/* has requested an alarm */	

#define	setrun(p)	((p)->state|=RUN)
#define	setnorun(p)	((p)->state&=~RUN)

#else
#define	RESHAPED	2048	/* layer got reshaped */
#define	MOVED		256	/* layer got moved */
#endif	MPXTERM
