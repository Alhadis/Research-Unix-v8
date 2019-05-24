#define	BUFSIZ	4096
#define	_NFILE	120
# ifndef FILE
extern	struct	_iobuf {
	int	_cnt;
	unsigned char	*_ptr;
	unsigned char	*_base;
	short	_flag;
	char	_file;
} _iob[_NFILE];
# endif

#define	_IOREAD	01
#define	_IOWRT	02
#define	_IONBF	04
#define	_IOMYBUF	010
#define	_IOEOF	020
#define	_IOERR	040
#define	_IOSTRG	0100
#define	_IOLBF	0200
#define	_IORW	0400
#define	NULL	0
#define	FILE	struct _iobuf
#define	EOF	(-1)

#define	stdin	(&_iob[0])
#define	stdout	(&_iob[1])
#define	stderr	(&_iob[2])
#define	getc(p)		(--(p)->_cnt>=0? (int)*(p)->_ptr++:_filbuf(p))
#define	getchar()	getc(stdin)
#define putc(x,p) (--(p)->_cnt>=0? ((int)(*(p)->_ptr++=(x))):_flsbuf((x),p))
#define	putchar(x)	putc(x,stdout)
#define	feof(p)		(((p)->_flag&_IOEOF)!=0)
#define	ferror(p)	(((p)->_flag&_IOERR)!=0)
#define	fileno(p)	((p)->_file)

FILE	*fopen();
FILE	*fdopen();
FILE	*freopen();
long	ftell();
char	*fgets();

#define	P_tmpdir	"/tmp/"
#define	L_tmpnam	(sizeof(P_tmpdir) + 15)
