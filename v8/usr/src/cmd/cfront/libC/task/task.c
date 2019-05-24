
#include "task.h"

/*	macros giving the addresses of the stack frame pointer
	and the program counter of the caller of the current function
	given the first local variable

	TOP points to the top of the current stack frame
	given the last local variable
*/

#ifdef pdp11

#define FP()		(&_that+4)
#define OLD_FP(fp)	(*fp)
#define TOP(var9)	(&var9)

#endif
#ifdef vax

#define FP(p)		((int*)(&p+1))
#define OLD_AP(fp)	(*(fp+2))
#define OLD_FP(fp)	(*(fp+3))
#define TOP(p)		top(&p)
extern int * top(...);

#endif

#define SETTRAP()	t_trap = *(t_basep-t_stacksize+1)
#define CHECKTRAP()	if (t_trap != *(t_basep-t_stacksize+1)) task_error(E_STACK,0)


int _hwm;

class team
{
friend task;
	int	no_of_tasks;
	task*	got_stack;
	int*	stack;
	team(task*, int = 0);
	~team() { delete stack; }
};
team.team(task* t, int stacksize) {
	no_of_tasks = 1;
	got_stack = t;
	if (stacksize) {
		stack = new int[stacksize];
		while (stack == 0) task_error(E_STORE,0);
	}
}


void usemainstack()
/* fudge to allow simple stack overflow check */
{
	register v[SIZE+100];

	if (_hwm)
		for (register i=0;i<SIZE+100;i++) v[i] = UNTOUCHED;
	else
		v[0] = 0;
}

void copy_stack(register* f, register c, register* t)
/*
	copy c words down from f to t
	do NOT attempt to copy "copy_stack"'s own stackframe
*/
{
	while (c--) { *t-- = *f--;}
}

void task.swap_stack(int* p, int* ta, int* de, int* pa, int* ap)
{
	int x = pa-TOP(x)+1;	/* size of active stack */
	copy_stack(pa,x,p);
	x = pa-p;		/* distance from old stack to new */
	t_framep = ta-x;	/* fp on new frame */
				/* now doctor the new frame */
#ifdef vax
	OLD_AP(t_framep) = int(ap-x);
#endif
	OLD_FP(t_framep) = int(de-x);
	restore();
}

task.task(char* name, int mode, int stacksize) : (TASK)
/*
	executed in the task creating a new task - thistask.
	1:	put thistask at head of scheduler queue,
	2:	create new task
	3:	transfer execution to new task
	derived::derived can never return - its return link is destroyed

	if thistask==0 then we are executing on main()'s stack and
	should turn it into the "main" task
*/
{
	int* p;
	int* ta_fp = (int*)FP(p);
	int* de_fp = (int*)OLD_FP(ta_fp);
#ifdef vax
	int* de_ap = (int*)OLD_AP(ta_fp);
#endif
	int* pa_fp = (int*)OLD_FP(de_fp);
	int x;

	t_name = name;
	t_mode = (mode) ? mode : DEDICATED;
	t_stacksize = (stacksize) ? stacksize : SIZE;
	t_size = 0;		/* avoid stack copy at initial restore */
	t_alert = 0;
	s_state = RUNNING;
	t_next = task_chain;
	task_chain = this;
	th = this;	/* fudged return value -- "returned" from swap */

	switch ((int)thistask) {
	case 0:
		/* initialize task system by creating "main" task */
		thistask = (task*) 1;
		thistask = new task("main");
		break;
	case 1:
		/*	create "main" task	*/
		usemainstack();		/* ensure that store is allocated */
		t_basep = (int*)OLD_FP(pa_fp);	/* fudge, what if main
					   	   is already deeply nested
						*/
		t_team = new team(this);	/* don't allocate stack */
		t_team->no_of_tasks = 2;   	/* never deallocate */
		return;
	}
	thistask->th = this;	/* return pointer to "child" */
	thistask->t_framep = de_fp;
	thistask->insert(0,this);

	switch (t_mode) {
	case DEDICATED:
		t_team = new team(this,t_stacksize);
		t_basep = t_team->stack + t_stacksize - 1;
		if (_hwm) for (x=0; x<t_stacksize; x++) p[x] = UNTOUCHED;
		thistask = this;
		swap_stack(t_basep,ta_fp,de_fp,pa_fp,de_ap);
	case SHARED:
		thistask->t_mode = SHARED; /* you cannot share on your own */
		t_basep = pa_fp;
		t_team = thistask->t_team;
		t_team->no_of_tasks++;
		t_framep = ta_fp;
		if (mode==0 && stacksize==0)
			t_stacksize = thistask->t_stacksize - (thistask->t_basep - t_basep);
		thistask = this;
		return;
	default:
		task_error(E_TASKMODE,this);
	}
}

void task.save()
/*
	save task's state so that ``restore'' can resume it later
	by returning from the function which called "save"
		- typically the scheduler
*/
{
	int* x;
	register* p = (int*)FP(x);

	t_framep = (int*)OLD_FP(p);

	CHECKTRAP();

	if (t_mode == SHARED) {
		register int sz;
		t_size = sz = t_basep - p + 1;
		p = new int[sz];
		while (p == 0) task_error(E_STORE,0);
		t_savearea = &p[sz-1];
		copy_stack(t_basep,sz,t_savearea);
	};
}

extern int rr2,rr3,rr4;
int rr2,rr3,rr4;

swap(task*);
sswap(task*);

void task.restore()
/*
	make "this" task run after suspension by returning from the frame
	denoted by "t_framep"

	the key function "swap" is written in assembly code,
	it returns from the function which "save"d the task
		- typically the scheduler

	"sswap" copies the stack back from the save area before "swap"ing
	arguments to "sswap" are passed in rr2,rr3,rr4 to avoid overwriting them
	it is equivallent to "copystack" followed by "swap".
*/
{
	register sz;

	SETTRAP();

	if ((t_mode == SHARED) && (sz=t_size)){
		register* p = t_savearea - sz + 1;
		register x = (this != t_team->got_stack);
		t_team->got_stack = this;
		delete p;
		if (x) {
			rr4 = (int) t_savearea;
			rr3 = sz;
			rr2 = (int) t_basep;
			sswap(this);
		}
		else
			swap(this);
	}
	else
		swap(this);
}

void task.cancel(int val)
/*
	TERMINATE and free stack space
*/
{
	sched::cancel(val);
	if (_hwm) t_size = curr_hwm();
	if (t_team->no_of_tasks-- == 1) delete t_team;
}

task.~task()
/*
	free stack space and remove task from task chain
*/
{
	if (s_state != TERMINATED) task_error(E_TASKDEL,this);
	if (this == task_chain)
		task_chain = t_next;
	else {
		register task* t;
		register task* tt;

		for (t=task_chain; tt=t->t_next; t=tt)  
			if (tt == this) {
				t->t_next = t_next;
				break;
			}
	}

	if (this == thistask) {
		delete (int*) thistask;	/* fudge: free(_that) */
		thistask = 0;
		schedule();
	}
}

void task.resultis(int val)
{
	cancel(val);
	if (this == thistask) schedule();
}

void task.sleep()
{
	if (s_state == RUNNING) remove();
	if (this == thistask) schedule();
}

void task.delay(int d)
{
	insert(d,this);
	if (thistask == this) schedule();
}

int task.preempt()
{
	if (s_state == RUNNING) {
		remove();
		return s_time-clock;
	}
	else {
		task_error(E_TASKPRE,this);
		return 0;
	}
}

char* state_string(int s)
{
	switch (s) {
	case IDLE:		return "IDLE";
	case TERMINATED:	return "TERMINATED";
	case RUNNING:		return "RUNNING";
	default:		return 0;
	}
}

char* mode_string(int m)
{
	switch(m) {
	case SHARED:		return "SHARED";
	case DEDICATED:		return "DEDICATED";
	default:		return 0;
	}
}

void task.print(int n)
/*
	``n'' values:	CHAIN,VERBOSE,STACK
*/
{
	char* ss = state_string(s_state);
	char* ns = (t_name) ? t_name : "";
	
	printf("task %s ",ns);
	if (this == thistask)
		printf("(is thistask):\n");
	else if (ss)
		printf("(%s):\n",ss);
	else
		printf("(state==%d CORRUPTED):\n",s_state);

	if (n&VERBOSE) {
		int res = (s_state==TERMINATED) ? (int) s_time : 0;
		char* ms = mode_string(t_mode);
		if (ms == 0) ms = "CORRUPTED";
		printf("\tthis==%d mode=%s alert=%d next=%d result=%d\n",
			this,ms,t_alert,t_next,res);
	}

	if (n&STACK) {
		printf("\tstack: ");
		if (s_state == TERMINATED) {
			if (_hwm) printf("hwm=%d",t_size);
			printf(" deleted\n");
		}
		else {
			int b = (int) t_basep;
			int x = ((this==thistask) || t_mode==DEDICATED) ? b-(int)t_framep : t_size;
			printf("max=%d current=%d",t_stacksize,x);
			if (_hwm) printf(" hwm=%d",curr_hwm());
			printf(" t_base=%d, t_frame=%d, t_size=%d\n",b,t_framep,t_size);
		}
	}

	if (n&CHAIN) {
		if (t_next) t_next->print(n);
	}
}

int task.curr_hwm()
{
	int* b = t_basep;
	int i;
	for (i=t_stacksize-1; 0<=i && *(b-i)==UNTOUCHED; i--) ;
	return i;
}

int task.waitlist(object* a)
{
	return waitvec(&a);
}

int task.waitvec(object* * v)
/*
	first determine if it is necessary to sleep(),
	return hint: who caused return
*/
{
	int i = 0;
	int r;
	object* ob;

	while (ob = v[i++]) {
		t_alert = ob;
		switch (ob->o_type) {
		case TASK:
		case TIMER:
			if (((sched*)ob)->s_state == TERMINATED) goto ex;
			break;
		case QHEAD:
			if (((qhead*)ob)->rdcount()) goto ex;
			break;
		case QTAIL:
			if (((qtail*)ob)->rdspace()) goto ex;
			break;
		}
		ob->remember(this);
	}
	if (i==2 && v[0]==(object*)thistask) task_error(E_WAIT,0);
	sleep();
ex:
	i = 0;
	while (ob = v[i++]) {
		ob->forget(this);
		if (ob == t_alert) r = i-1;
	}
	return r;
} 



