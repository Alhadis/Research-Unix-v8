#include "task.h"

object.~object()
{
	if (o_link) task_error(E_OLINK,this);
	if (o_next) task_error(E_ONEXT,this);
} /* delete */

/*	note that a task can be on a chain in several places
*/

/* object.remember() ... */

void object.forget(register task* p)
/* remove all occurrences of task* p from this object's task list */
{
	register olink* ll;
	register olink* l;

	if (o_link == 0) return;

	while (o_link->l_task == p) {
		ll = o_link;
		o_link = ll->l_next;
		delete ll;
		if (o_link == 0) return;
	};

	l = o_link;
	while (ll = l->l_next) {
		if (ll->l_task == p) {
			l->l_next = ll->l_next;
			delete ll;
		}
		else l = ll;
	};
}


void object.alert()
/* prepare IDLE tasks on this object for sceduling */
{
	register olink* l;

	for (l=o_link; l; l=l->l_next) {
		register task* p = l->l_task;
		if (p->s_state == IDLE) p->insert(0,this);
	}
}

void object.print(int n)
{
	int m = n & ~CHAIN;

	switch (o_type) {
	case QHEAD:
		((qhead*) this)->print(m);
		break;
	case QTAIL:
		((qtail*) this)->print(m);
		break;
	case TASK:
		((task*) this)->print(m);
		break;
	case TIMER:
		((task*) this)->print(m);
		break;
	default:
		printf("object (o_type==%d): ",o_type);
	}

	if (n&VERBOSE) {
		olink* l;
		printf("remember_chain:\n");
		for (l=o_link; l; l=l->l_next) l->l_task->print(m);
	}

	if (n&CHAIN) {
		if (o_next) o_next->print(n);
	}
	printf("\n");
}
