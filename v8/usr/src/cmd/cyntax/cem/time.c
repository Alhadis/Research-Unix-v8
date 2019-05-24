#include	<time.h>
#include	"cem.h"

/*
 *	Return a sstring representation of a time.  If it is
 *	the same year as now leave out the year.  If it is the
 *	same day and we aren't being pedantic leave out the date.
 */
char	*
stime(t)
long	t;
{
	register struct tm	*tp;
	register char		*p;
	static struct tm	this;
	static long		now;
	extern char		*ctime();
	extern long		time();
	extern struct tm	*localtime();

	if (now == 0)
	{
		now = time((long *)0);
		this = *localtime(&now);
	}

	tp = localtime(&t);
	p = ctime(&t);

	if (tp->tm_year == this.tm_year)
		p[19] = '\0';
	else
		p[24] = '\0';

	if
	(
		!pedantic
		&&
		tp->tm_mon == this.tm_mon
		&&
		tp->tm_mday == this.tm_mday
		&&
		tp->tm_year == this.tm_year
	)
		return p + 11;
	else
		return p;
}
