#include "queue.h"
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
	register struct cbuf *p;
	p = freelist;
	if (p==NULL)
		return(0);
	freelist = p->next;
	p->next = NULL;
	p->word = w;
	if (q->c_cc == 0) {
		q->c_head = q->c_tail = p;
		return(++q->c_cc);
	}
	q->c_tail->next = p;
	q->c_tail = p;
	return(++q->c_cc);
}

qgetc(q)
	register struct clist *q;
{
	register struct cbuf *p;
	if ((p=q->c_head)==NULL)
		return(-1);
	if (--q->c_cc == 0)
		q->c_head = q->c_tail = NULL; else
		q->c_head = p->next;
	p->next = freelist;
	freelist = p;
	return(p->word);
}

struct cbuf *
qtail(p)
	register struct cbuf *p;
{
	register struct cbuf *q;
	q=p;
	while((q=p->next)!=NULL){
		if(q->next==NULL)
			return(p);
		p=q;
	}
	return(p);	/* One word in list */
}

/*
 * Take from end; sort of unputc
 */
qtakec(q)
	register struct clist *q;
{
	register struct cbuf *p;
	if ((p=q->c_tail) == NULL)
		return(-1);
	if (--q->c_cc == 0)
		q->c_tail = q->c_head = NULL;
	else{
		q->c_tail = qtail(q->c_head);
		q->c_tail->next = NULL;
		}
	p->next = freelist;
	freelist = p;
	return(p->word);
}

qclear(q)
	register struct clist *q;
{
	if(q->c_cc == 0)
		return;
	q->c_cc = 0;
	q->c_tail->next = freelist;
	freelist = q->c_head;
	q->c_head = NULL;
	q->c_tail = NULL;
}
