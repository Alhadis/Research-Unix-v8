#include "../h/param.h"
#include "../h/systm.h"
/*
 * Return the time according to the chaos TIME protocol, in a long.
 * No byte shuffling need be done here, just time conversion.
 */
ch_time(tp)
register long *tp;
{
	*tp = time;
	*tp += 60L*60*24*((1970-1900)*365L + 1970/4 - 1900/4);
}
