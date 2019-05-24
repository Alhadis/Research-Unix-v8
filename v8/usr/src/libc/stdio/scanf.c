/* @(#)scanf.c	4.1 (Berkeley) 12/21/80 */
#include	<stdio.h>

scanf(fmt, args)
char *fmt;
{
	return(_doscan(stdin, fmt, &args));
}

fscanf(iop, fmt, args)
FILE *iop;
char *fmt;
{
	return(_doscan(iop, fmt, &args));
}

sscanf(str, fmt, args)
register char *str;
char *fmt;
{
	FILE _strbuf;

	_strbuf._flag = _IOREAD|_IOSTRG;
	_strbuf._ptr = _strbuf._base = (unsigned char *) str;
	_strbuf._cnt = 0;
	while (*str++)
		_strbuf._cnt++;
	return(_doscan(&_strbuf, fmt, &args));
}
