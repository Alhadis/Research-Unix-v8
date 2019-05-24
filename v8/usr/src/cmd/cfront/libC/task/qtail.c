#include "task.h"

/* construct qtail <--> oqueue */
qtail.qtail(int mode, int max) : (QTAIL)
{
	if (0 < max) {
		qt_queue = new class oqueue(max);
		qt_queue->q_tail = this;
	};
	qt_mode = mode;
}

/* destroy q if not also pointed to by a qhead */
qtail.~qtail()
{
	oqueue* q = qt_queue;

	if (q->q_head)
		q->q_tail = 0;
	else
		delete q;
}


/* insert object at rear of q (becoming new value of oqueue->q_ptr) */
int qtail.put(object* p)
{
	register oqueue* q = qt_queue;
ll:
	if (p->o_next) task_error(E_PUTOBJ,this);

	if (q->q_count < q->q_max) {
		if (q->q_count++) {
			register object* oo = q->q_ptr;
			p->o_next = oo->o_next;
			q->q_ptr = oo->o_next = p;
		}
		else {
			qhead* h = q->q_head;
			q->q_ptr = p->o_next = p;
			if (h) h->alert();
		}
		return 1;
	}

	switch (qt_mode) {
	case WMODE:
		remember(thistask);
		thistask->sleep();
		forget(thistask);
		goto ll;
	case EMODE:
		task_error(E_PUTFULL,this);
		goto ll;
	case ZMODE:
		return 0;
	}
}


/* create head for this q */
qhead* qtail.head()
{
	oqueue* q = qt_queue;
	register qhead* h = q->q_head;

	if (h == 0) {
		h = new qhead(qt_mode,0);
		q->q_head = h;
		h->qh_queue = q;
	};

	return h;
}


/* result:  ?qhead<-->? oldq<-->(new)qtail  newq<-->(this)qtail */
qtail* qtail.cut()
{
	oqueue* oldq = qt_queue;
	qtail* t = new qtail(qt_mode,oldq->q_max);
	oqueue* newq = t->qt_queue;

	t->qt_queue = oldq;
	oldq->q_tail = t;

	newq->q_tail = this;
	qt_queue = newq;

	return t;
}


/* this qtail is supposed to be downstream from the qhead h */
void qtail.splice(qhead* h)
{
	h->splice(this);
}


void qtail.print(int n)
{
	int m = qt_queue->q_max;
	int c = qt_queue->q_count;
	class qhead * h = qt_queue->q_head;

	printf("qtail (%d): mode=%d, max=%d, space=%d, head=%d\n",
		this,qt_mode,m,m-c,h);

	if (n&VERBOSE) {
		int m = n & ~(CHAIN|VERBOSE);
		if (h) {
			printf("head of queue:\n");
			h->print(m);
		}

		qt_queue->print(m);
	}
}
