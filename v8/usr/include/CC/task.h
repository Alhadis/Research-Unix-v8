printf(char* ...);

#define SIZE		750
#define MODE		DEDICATED

#define UNTOUCHED	052525

class object;
class sched;
class task;
class qhead;
class qtail;

extern sched* run_chain;		/* list of ready to run scheds */
extern task* task_chain;		/* list of all tasks */

extern int task_error(int, object*);
extern void print_error(int);

#define E_OLINK		1
#define E_ONEXT		2
#define E_GETEMPTY	3
#define E_PUTOBJ	4
#define E_PUTFULL	5
#define E_BACKOBJ	6
#define E_BACKFULL	7
#define E_SETCLOCK	8
#define E_CLOCKIDLE	9
#define E_RESTERM	10
#define E_RESRUN	11
#define E_NEGTIME	12
#define E_RESOBJ	13
#define E_HISTO		14
#define E_STACK 	15
#define E_STORE		16
#define E_TASKMODE	17
#define E_TASKDEL	18
#define E_TASKPRE	19
#define E_TIMERDEL	20
#define E_SCHTIME	21
#define E_SCHOBJ	22
#define E_QDEL		23
#define E_RESULT	24
#define E_WAIT		25
#define MAXERR	E_WAIT

extern char _hwm;
#define hwm()	(_hwm=1)

class olink
/*	the building block for chains of task pointers */
{
friend object;
	olink*	l_next;
	task*	l_task;
		olink(task* t, olink* l) { l_task=t; l_next=l; };
};

class oqueue
{
friend qhead;
friend qtail;
	int	q_max;
	int	q_count;
	object*	q_ptr;
	qhead*	q_head;
	qtail*	q_tail;

		oqueue(int m)	{ q_max=m; q_count=0; q_head=0; q_tail=0; };
		~oqueue()	{ (q_count)?task_error(E_QDEL,0):0; };
	void	print(int);
};

	/*	header file for the task system		*/

#define TIMER		1
#define TASK		2
#define QHEAD		4
#define QTAIL		5

#define IDLE		1
#define RUNNING		2
#define TERMINATED	4

#define DEDICATED	1
#define SHARED		2

#define CHAIN		1
#define VERBOSE		2
#define STACK		4

extern task* clock_task;
extern task* thistask;
extern long clock;


typedef int (*PFIO)(int,object*);
typedef void (*PFV)();

extern PFIO error_fct;
extern PFV exit_fct;

#define EMODE		1
#define WMODE		2
#define ZMODE		3

class object
{
	olink*	o_link;
public:
	object*	o_next;
	int	o_type;

		object(int t = 0)		{ o_type=t; o_link=0; o_next=0; }
		~object();
	void	remember(task* t)	{ o_link = new olink(t,o_link); }
	void	forget(task*);
	void	alert();

	void	print(int);
};

class sched : public object
{
friend timer;
friend task;
friend object;
	void	schedule();
	void	insert(int, object*);
	void	remove();

	long	s_time;		/* holds result after cancel() */
	int	s_state;
public:
	void	print(int);

	long	rdtime()	{ return s_time; };
	int	rdstate()	{ return s_state; };

	void	cancel(int);
	int	result();
};

struct timer : public sched
{
		timer(int);
		~timer();
	void	reset(int);
	void	print(int);
};

class task : public sched
{
friend sched;
		task(char* =0, int =0, int =0);
		~task();

	void	save();
	void	restore();
	int	curr_hwm();

	int*	t_framep;	/* WARNING: t_framep
				   is manipulated as an offset
				   by restore()
				*/
	int*	t_basep;
	int	t_size;		/* holds hwm after cancel() */
	int*	t_savearea;
	int	t_trap;
	class team*	t_team;

	int	t_mode;
	int	t_stacksize;

	object*	t_alert;
public:
	task*	t_next;
	char*	t_name;

	int	waitvec(object**);
	int	waitlist(object* ...);
	void	wait(object* ob)	{ (void) waitlist(ob,0); };

	void	delay(int);
	int	preempt();
	void	sleep();
	void	resultis(int);
	void	cancel(int);

	void	print(int);
};



class qhead : public object
{
friend qtail;
		qhead(int = WMODE, int = 10000);
		~qhead();

	int	qh_mode;
	oqueue*	qh_queue;
public:
	object*	get();
	int	putback(object*);

	int	rdcount()	{ return qh_queue->q_count; }
	int	rdmax()		{ return qh_queue->q_max; }
	int	rdmode()	{ return qh_mode; }
	qtail*	tail();

	qhead*	cut();
	void	splice(qtail*);

	void	setmode(int m)	{ qh_mode = m; };
	void	setmax(int m)	{ qh_queue->q_max = m; };

	void	print(int);
};

class qtail : public object
{
friend qhead;
		qtail(int = WMODE, int = 10000);
		~qtail();

	int	qt_mode;
	oqueue*	qt_queue;
public:
	int	put(object*);

	int	rdspace()	{ return qt_queue->q_max - qt_queue->q_count; };
	int	rdmax()		{ return qt_queue->q_max; };
	int	rdmode()	{ return qt_mode; };

	qtail*	cut();
	void 	splice(qhead*);

	qhead*	head();

	void	setmode(int m)	{ qt_mode = m; };
	void	setmax(int m)	{ qt_queue->q_max = m; };

	void	print(int);
};

struct histogram
/*
	"nbin" bins covering the range [l:r[ uniformly
	nbin*binsize == r-l
*/
{
	int	l, r;
	int	binsize;
	int	nbin;
	int*	h;
	long	sum;
	long	sqsum;
		histogram(int=16, int=0, int=16);

	void	add(int);
	void	print();
};

/*	the result of randint() is always >= 0	*/

#define DRAW (randx = randx*1103515245 + 12345)
#define ABS(x)	(x&0x7fffffff)

#ifdef pdp11
#define MASK(x)	((x>>16)&077777)
#define MAX 32768.0
#endif

#ifdef vax
#define MASK(x) ABS(x)
#define MAX 2147483648.0
#endif

class randint
/*	uniform distribution in the interval [0,MAX] */
{
	long	randx;
public:
		randint(long s = 0)	{ randx=s; }
	void	seed(long s)	{ randx=s; }
	int	draw()		{ return MASK(DRAW); }
	float	fdraw()		{ return ABS(DRAW)/MAX; };
};

class urand : public randint
/*	uniform distribution in the interval [low,high]	*/
{
public:
	int	low, high;
		urand(int l, int h)	{ low=l; high=h; }
	int	draw() { return int(low + (high-low) * (0+randint::draw()/MAX)); }
};

extern double log(double);

class erand : public randint
/*	exponential distribution random number generator */
{
public:
	int	mean;
		erand(int m) { mean=m; };
	int	draw() { return int( (-mean * log( (double) (MAX-randint::draw())/MAX) + .5)); };
};
