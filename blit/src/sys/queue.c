#include "queue.h"
#undef NULL
#define	NULL		((struct cbuf *)0)

/*
 * These are mostly for speed, not size.
 * Note that sr must be the second data register declared in each routine.
 * UGH ICK UGH (but fast)
*/
#define	spl7()	asm("mov.w %sr, %d3; mov.w &0x2700, %sr");
#define	splx(sr)	asm("mov.w %d3, %sr");

qinit()
{
	register struct cbuf *p, *q;

	q = NULL;
	for(p=cbufs; p< &cbufs[NCHARS]; p++)  {
		p->next = q;
		q = p;
	}
	freelist = q;
}

qputc(q, w)
	register struct clist *q;
	register w;
{
	register sr;
	register cc;
	register struct cbuf *p;
	spl7();
	p = freelist;
	if (p==NULL) {
		splx(sr);
		return(0);
	}
	freelist = p->next;
	p->next = NULL;
	p->word = w;
	if (q->c_cc == 0)
		q->c_head = q->c_tail = p;
	else {
		q->c_tail->next = p;
		q->c_tail = p;
	}
	cc= ++q->c_cc;
	splx(sr);
	return(cc);
}

qgetc(q)
	register struct clist *q;
{
	register struct cbuf *p;
	register x;
	register sr;
	spl7();
	if ((p=q->c_head)==NULL) {
		splx(sr);
		return(-1);
	}
	if (--q->c_cc == 0)
		q->c_head = q->c_tail = NULL;
	else
		q->c_head = p->next;
	p->next = freelist;
	x = p->word;
	freelist = p;
	splx(sr);
	return(x);
}

qclear(q)
	register struct clist *q;
{
	register dummy;
	register sr;
	spl7();
	if(q->c_cc == 0){
		splx(sr);
		return;
	}
	q->c_cc = 0;
	q->c_tail->next = freelist;
	freelist = q->c_head;
	q->c_head = NULL;
	q->c_tail = NULL;
	splx(sr);
}
