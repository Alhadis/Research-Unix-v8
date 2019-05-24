#define	BUFSIZ	100
#define	_NFILE	8
# ifndef FILE
extern	struct	_iobuf {
	int	count;
	char	*cp;
	char	*base;
	char	flag;
	char	file;
} _iob[_NFILE];
# endif

#ifdef MPX
#undef exit
#undef	jinit
#define	jinit()	(Tvoid(15)(),request(RCV|SEND))
#define jputchar(x)	sendchar(x)
#define jgetchar()	(wait(RCV),rcvchar())
#endif

#define	_IOREAD	01
#define	_IOWRT	02
#define _IOTTY	04
#define	_IOEOF	010
#define	_IOERR	020
#ifndef NULL
#define	NULL	((char *)0)
#endif
#define	FILE	struct _iobuf
#define	EOF	(-1)

#define	stdin	(&_iob[0])
#define	stdout	(&_iob[1])
#define	stderr	(&_iob[2])
#define	feof(p)		(((p)->flag & _IOEOF) != 0)
#define	ferror(p)	(((p)->flag & _IOERR) != 0)
#define isatty(p)	(((p)->flag & _IOTTY) != 0)
#define	fileno(p)	((p)->file)

FILE	*fopen();
