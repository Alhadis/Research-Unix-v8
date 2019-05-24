/*
 * stream tracer
 */

#include "trc.h"
#if NTRC > 0
#include "../h/param.h"
#include "../h/stream.h"
#include "../h/ioctl.h"
#include "../h/conf.h"
#include "../h/strace.h"


extern int	nulldev();

struct trc	trc[NTRC];

int	trcclose(), 	/* queue close routine */
	trcopen(), 	/* queue open routine */
	trcput();	/* queue put routine */

static struct qinit	rinit = { trcput, nulldev, trcopen, trcclose, 300, 0 },
			winit = { trcput, nulldev, trcopen, trcclose, 300, 200 };
struct streamtab	trcinfo = { &rinit, &winit };


trcclose (q)
	register struct queue	*q;
{
	register struct trc	*tr = (struct trc *) q->ptr;

	trcupdate (q);
	trcupdate (WR (q));
	tr->tr_state = 0;
	printf ("%s: close\n", tr->tr_name);
	return;
}


/*
 * I/O control operations.
 * Process request and return nonzero iff request is for us.
 */

trcioctl (q, b)
	register struct queue	*q;
	register struct block	*b;
{
	register struct trc	*tr = (struct trc *) q->ptr;
	register struct trcioc	*ioc = (struct trcioc *) b->rptr;

	switch (ioc->command) {

	/*
	 * get name
	 */
	case TRCGNAME:
		bcopy (tr->tr_name, ioc->arg.name, TR_NAME-1);
		printf ("%s: get name %d\n", tr->tr_name, tr-trc);
		b->wptr = b->rptr + sizeof (int) + TR_NAME - 1;
		break;

	/*
	 * set name
	 */
	case TRCSNAME:
		bcopy (ioc->arg.name, tr->tr_name, TR_NAME-1);
		tr->tr_name[TR_NAME-1] = '\0';
		printf ("%s: set name %d\n", tr->tr_name, tr-trc);
		b->wptr = b->rptr;
		break;

	/*
	 * get block type trace mask
	 */
	case TRCGMASK:
		ioc->arg.mask = tr->tr_mask;
		printf ("%s: get mask 0x%x\n", tr->tr_name, tr->tr_mask);
		b->wptr = b->rptr + 2 * sizeof (int);
		break;

	/*
	 * set block type trace mask
	 */
	case TRCSMASK:
		tr->tr_mask = ioc->arg.mask;
		printf ("%s: set mask 0x%x\n", tr->tr_name, tr->tr_mask);
		b->wptr = b->rptr;
		break;

	default:
		return (0);
	}

	b->type = M_IOCACK;
	qreply (q, b);
	return (1);
}


/*ARGSUSED*/

int
trcopen (q, dev)
	register struct queue	*q;
	int			dev;
{
	register struct trc	*tr;

	if (q->ptr)
		return (1);		/* already attached */

	for (tr = trc; tr->tr_state & TR_USE; tr++)
		if (tr >= &trc[NTRC-1])
			return (0);

	WR (q)->ptr = q->ptr = (caddr_t) tr;

	tr->tr_state = TR_USE;
	tr->tr_mask = ~0;		/* trace all block types */
	bcopy ("trc", tr->tr_name, 4);
	trcupdate (q);
	trcupdate (WR (q));
	printf ("trc: open %d\n", tr-trc);

	return (1);
}


trcprb (b)
	register struct block	*b;
{
	u_char		buf[1+4+4*64];	/* 64 is max # bytes in a block */
	register u_char	*nl, *s, *t, *u;
	register int	c;

	for (s = b->rptr, t = b->wptr, nl = u = buf; s < t; s++) {
		if ((c = *s) & 0x80) {
			*u++ = 'M';
			*u++ = '-';
			c &= ~0x80;
		}
		if (c == 0x7f) {
			*u++ = '^';
			c = '?';
		}
		else	if (c < ' ') {
				*u++ = '^';
				c += '@';
		}
		*u++ = c;
		if (u - nl > 64)
			*(nl = u++) = '\n';
	}

	*u = '\0';
	printf ("%s", buf);
	return;
}


#define trcpr(x)	printf ("%s: %s %d bytes: %s", tr->tr_name, \
			    rw, b->wptr-b->rptr, x)

trcprint (q, b)
	register struct queue	*q;
	register struct block	*b;
{
	register struct trc	*tr = (struct trc *) q->ptr;
	register char		*rw = q->flag & QREADR ? "read" : "write";

	switch (b->type) {
	case M_ACK:
		if (tr->tr_mask & TR_ACK)
			trcpr ("M_ACK\n");
		break;
	case M_BREAK:
		if (tr->tr_mask & TR_BREAK)
			trcpr ("M_BREAK\n");
		break;
	case M_CLOSE:
		if (tr->tr_mask & TR_CLOSE)
			trcpr ("M_CLOSE\n");
		break;
	case M_CTL:
		if (tr->tr_mask & TR_CTL)
			trcpr ("M_CTL\n");
		break;
	case M_DATA:
		if (tr->tr_mask & TR_DATA)
			trcpr ("M_DATA: ``"), trcprb (b), printf ("''\n");
		break;
	case M_DELAY:
		if (tr->tr_mask & TR_DELAY)
			trcpr ("M_DELAY"), printf (": %d\n", (int) *b->rptr);
		break;
	case M_DELIM:
		if (tr->tr_mask & TR_DELIM)
			trcpr ("M_DELIM\n");
		break;
	case M_ECHO:
		if (tr->tr_mask & TR_ECHO)
			trcpr ("M_ECHO: ``"), trcprb (b), printf ("''\n");
		break;
	case M_FLUSH:
		if (tr->tr_mask & TR_FLUSH)
			trcpr ("M_FLUSH\n");
		break;
	case M_HANGUP:
		if (tr->tr_mask & TR_HANGUP)
			trcpr ("M_HANGUP\n");
		break;
	case M_IOCACK:
		if (tr->tr_mask & TR_IOCACK)
			trcpr ("M_IOCACK\n");
		break;
	case M_IOCNAK:
		if (tr->tr_mask & TR_IOCNAK)
			trcpr ("M_IOCNAK\n");
		break;
	case M_IOCTL:
		if (tr->tr_mask & TR_IOCTL) {
			register int	l = * (int *) b->rptr, h = l>>8;

			trcpr ("M_IOCTL"); 
			if (h >= ' ' && h < '\177')
				printf (": ('%c'<<8)|%d\n", h, l & 0xff);
			else	printf (": 0x%x\n", l);
		}
		break;
	case M_SIGNAL:
		if (tr->tr_mask & TR_SIGNAL)
			trcpr ("M_SIGNAL"), printf (": %d\n", (int) *b->rptr);
		break;
	case M_START:
		if (tr->tr_mask & TR_START)
			trcpr ("M_START\n");
		break;
	case M_STOP:
		if (tr->tr_mask & TR_STOP)
			trcpr ("M_STOP\n");
		break;
	default:
		trcpr ("BAD BLOCK"), printf (": type %d\n", b->type);
		break;
	}
	return;
}


/*
 * Reader/Writer queue put routine.
 */

trcput (q, b)
	register struct queue	*q;
	register struct block	*b;
{
	register struct queue	*qnext = q->next;

	if (b->type != M_IOCTL || !trcioctl (q, b)) {
		trcprint (q, b);
		(*qnext->qinfo->putp)(qnext, b);
	}
	trcupdate (q);
	return;
}


trcupdate (q)
	register struct queue	*q;
{
	register struct queue	*qnext = q->next;

	q->flag |= qnext->flag & (QFULL|QWANTR) | backq(q)->flag & QDELIM;
	qnext->flag |= q->flag & QWANTW;
	q->count = qnext->count;
	return;
}
