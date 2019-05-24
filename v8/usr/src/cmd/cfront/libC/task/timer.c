#include "task.h"

timer.timer(int d) : (TIMER)
{
	s_state = IDLE;
	insert(d,this);
}

timer.~timer()
{
	if (s_state != TERMINATED) task_error(E_TIMERDEL,this);
}

void timer.reset(int d)
{
	remove();
	insert(d,this);
}

void timer.print(int)
{ 
	long tt = s_time;
	printf("timer %ld == clock+%ld\n",tt,tt-clock);
}
