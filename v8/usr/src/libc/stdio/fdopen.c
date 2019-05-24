/* @(#)fdopen.c	4.2 (Berkeley) 3/9/81 */
/*
 * Unix routine to do an "fopen" on file descriptor
 * The mode has to be repeated because you can't query its
 * status
 */

#include	<stdio.h>
#include	<errno.h>

FILE *
fdopen(fd, mode)
register char *mode;
{
	extern int errno;
	register FILE *iop;
	extern FILE *_lastbuf;

	for (iop = _iob; iop->_flag&(_IOREAD|_IOWRT|_IORW); iop++)
		if (iop >= _lastbuf)
			return(NULL);
	iop->_cnt = 0;
	iop->_file = fd;
	if (*mode != 'r') {
		iop->_flag |= _IOWRT;
		if (*mode == 'a')
			lseek(fd, 0L, 2);
	} else
		iop->_flag |= _IOREAD;
	if (mode[1] == '+') {
		iop->_flag &= ~(_IOREAD|_IOWRT);
		iop->_flag |= _IORW;
	}
	return(iop);
}
