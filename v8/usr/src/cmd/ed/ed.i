










































typedef int	(*SIG_TYP)();
SIG_TYP signal();








































typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;



typedef	struct	_physadr { int r[1]; } *physadr;
typedef	long	daddr_t;
typedef	char *	caddr_t;
typedef	u_short ino_t;
typedef	long	swblk_t;
typedef	long	size_t;
typedef	long	time_t;
typedef	long	label_t[14];
typedef	short	dev_t;
typedef	long	off_t;
typedef long	portid_t;

























































































































































































typedef struct	fd_set { unsigned long fds_bits[	(128		+	sizeof(int)	*	8		-1)/(	sizeof(int)	*	8		)]; } fd_set;













struct sgttyb {
	char	sg_ispeed;		
	char	sg_ospeed;		
	char	sg_erase;		
	char	sg_kill;		
	short	sg_flags;		
};




struct tchars {
	char	t_intrc;	
	char	t_quitc;	
	char	t_startc;	
	char	t_stopc;	
	char	t_eofc;		
	char	t_brkc;		
};




struct	insld {
	short	ld;
	short	level;
};




struct	passfd {
	union  {
		struct	file *fp;
		int	fd;
	} f;
	short	uid;
	short	gid;
	short	nice;
	short	fill;
};

























































































































struct ltchars {
	char	t_suspc;	
	char	t_dsuspc;	
	char	t_rprntc;	
	char	t_flushc;	
	char	t_werasc;	
	char	t_lnextc;	
};





struct luchars {
	char	t_undoc;	
	char	t_urotc;	
};




















































































































































typedef int jmp_buf[10];





















char	Q[]	= "";
char	T[]	= "TMP";



int	peekc;
int	lastc;
char	savedfile[128];
char	file[128];
char	linebuf[512];
char	rhsbuf[512/2];
char	expbuf[256+4];
int	circfl;
int	*zero;
int	*dot;
int	*dol;
int	given;
int	*addr1;
int	*addr2;
char	genbuf[512];
long	count;
char	*nextip;
char	*linebp;
int	ninbuf;
int	io;
int	pflag;
long	lseek();
int	(*oldhup)();
int	(*oldquit)();
int	vflag	= 1;
int	xflag;
int	xtflag;
int	kflag;
int	oflag;
char	key[9 + 1];
char	crbuf[512];
char	perm[768];
char	tperm[768];
int	listf;
int	listn;
int	col;
char	*globp;
int	tfile	= -1;
int	tline;
char	*tfname;
char	*loc1;
char	*loc2;
char	*locs;
char	ibuff[512];
int	iblock	= -1;
char	obuff[512];
int	oblock	= -1;
int	ichanged;
int	nleft;
char	WRERR[]	= "WRITE ERROR";
int	names[26];
int	anymarks;
char	*braslist[5];
char	*braelist[5];
int	nbra;
int	subnewa;
int	subolda;
int	fchange;
int	wrapp;
int	bpagesize = 20;
unsigned nlall = 128;

int	*address();
char	*getline();
char	*getblock();
char	*place();
char	*mktemp();
char	*malloc();
char	*realloc();
jmp_buf	savej;

main(argc, argv)
char **argv;
{
	register char *p1, *p2;
	extern int onintr(), quit(), onhup();
	int (*oldintr)();

	oldquit = signal(3	, 	(int (*)())1);
	oldhup = signal(1	, 	(int (*)())1);
	oldintr = signal(2	, 	(int (*)())1);
	if ((int)signal(15	, 	(int (*)())1) == 0)
		signal(15	, quit);
	argv++;
	while (argc > 1 && **argv=='-') {
		switch((*argv)[1]) {

		case '\0':
			vflag = 0;
			break;

		case 'q':
			signal(3	, 	(int (*)())0);
			vflag = 1;
			break;

		case 'X':
			xflag = 1;
			break;

		case 'o':
			oflag = 1;
			vflag = 0;
			break;
		}
		argv++;
		argc--;
	}
	if(xflag){
		getkey();
		kflag = crinit(key, perm);
	}

	if (oflag) {
		p1 = "/dev/stdout";
		p2 = savedfile;
		while (*p2++ = *p1++)
			;
		globp = "a";
	} else if (argc>1) {
		p1 = *argv;
		p2 = savedfile;
		while (*p2++ = *p1++)
			if (p2 >= &savedfile[sizeof(savedfile)])
				p2--;
		globp = "r";
	}
	zero = (int *)malloc(nlall*sizeof(int));
	tfname = mktemp("/tmp/eXXXXX");
	init();
	if (((int)oldintr&01) == 0)
		signal(2	, onintr);
	if (((int)oldhup&01) == 0)
		signal(1	, onhup);
	setjmp(savej);
	commands();
	quit();
}

commands()
{
	int getfile(), gettty();
	register *a1, c;
	register int temp;
	char lastsep;

	for (;;) {
	if (pflag) {
		pflag = 0;
		addr1 = addr2 = dot;
		print();
	}
	c = '\n';
	for (addr1 = 0;;) {
		lastsep = c;
		a1 = address();
		c = getchr();
		if (c!=',' && c!=';')
			break;
		if (lastsep==',')
			error(Q);
		if (a1==0) {
			a1 = zero+1;
			if (a1>dol)
				a1--;
		}
		addr1 = a1;
		if (c==';')
			dot = a1;
	}
	if (lastsep!='\n' && a1==0)
		a1 = dol;
	if ((addr2=a1)==0) {
		given = 0;
		addr2 = dot;	
	}
	else
		given = 1;
	if (addr1==0)
		addr1 = addr2;
	switch(c) {

	case 'a':
		add(0);
		continue;

	case 'b':
		nonzero();
		browse();
		continue;

	case 'c':
		nonzero();
		newline();
		rdelete(addr1, addr2);
		append(gettty, addr1-1);
		continue;

	case 'd':
		nonzero();
		newline();
		rdelete(addr1, addr2);
		continue;

	case 'E':
		fchange = 0;
		c = 'e';
	case 'e':
		setnoaddr();
		if (vflag && fchange) {
			fchange = 0;
			error(Q);
		}
		filename(c);
		init();
		addr2 = zero;
		goto caseread;

	case 'f':
		setnoaddr();
		filename(c);
		puts(savedfile);
		continue;

	case 'g':
		global(1);
		continue;

	case 'i':
		add(-1);
		continue;


	case 'j':
		if (!given)
			addr2++;
		newline();
		join();
		continue;

	case 'k':
		nonzero();
		if ((c = getchr()) < 'a' || c > 'z')
			error(Q);
		newline();
		names[c-'a'] = *addr2 & ~01;
		anymarks |= 01;
		continue;

	case 'm':
		move(0);
		continue;

	case 'n':
		listn++;
		newline();
		print();
		continue;

	case '\n':
		if (a1==0) {
			a1 = dot+1;
			addr2 = a1;
			addr1 = a1;
		}
		if (lastsep==';')
			addr1 = a1;
		print();
		continue;

	case 'l':
		listf++;
	case 'p':
	case 'P':
		newline();
		print();
		continue;

	case 'Q':
		fchange = 0;
	case 'q':
		setnoaddr();
		newline();
		quit();

	case 'r':
		filename(c);
	caseread:
		if ((io = open(file, 0)) < 0) {
			lastc = '\n';
			error(file);
		}
		setwide();
		squeeze(0);
		ninbuf = 0;
		c = zero != dol;
		append(getfile, addr2);
		exfile();
		fchange = c;
		continue;

	case 's':
		nonzero();
		substitute(globp!=0);
		continue;

	case 't':
		move(1);
		continue;

	case 'u':
		nonzero();
		newline();
		if ((*addr2&~01) != subnewa)
			error(Q);
		*addr2 = subolda;
		dot = addr2;
		continue;

	case 'v':
		global(0);
		continue;

	case 'W':
		wrapp++;
	case 'w':
		setwide();
		squeeze(dol>zero);
		if ((temp = getchr()) != 'q' && temp != 'Q') {
			peekc = temp;
			temp = 0;
		}
		filename(c);
		if(!wrapp ||
		  ((io = open(file,1)) == -1) ||
		  ((lseek(io, 0L, 2)) == -1))
			if ((io = creat(file, 0666)) < 0)
				error(file);
		wrapp = 0;
		if (dol > zero)
			putfile();
		exfile();
		if (addr1<=zero+1 && addr2==dol)
			fchange = 0;
		if (temp == 'Q')
			fchange = 0;
		if (temp)
			quit();
		continue;

	case 'X':
		setnoaddr();
		newline();
		xflag = 1;
		puts("Entering encrypting mode!");
		getkey();
		kflag = crinit(key, perm);
		continue;


	case '=':
		setwide();
		squeeze(0);
		newline();
		count = addr2 - zero;
		putd();
		putchr('\n');
		continue;

	case '!':
		callunix();
		continue;

	case -1:
		return;

	}
	error(Q);
	}
}

print(){
	register int *a1;
	nonzero();
	a1 = addr1;
	do {
		if (listn) {
			count = a1-zero;
			putd();
			putchr('\t');
		}
		puts(getline(*a1++));
	} while (a1 <= addr2);
	dot = addr2;
	listf = 0;
	listn = 0;
	pflag = 0;
}

int *
address()
{
	register int sign, *a;
	int opcnt, nextopand, *b;
	register int c;

	nextopand = -1;
	sign = 1;
	opcnt = 0;
	a = dot;
	do {
		do c = getchr(); while (c==' ' || c=='\t');
		if ('0'<=c && c<='9') {
			peekc = c;
			if (!opcnt)
				a = zero;
			a += sign*getnum();
		} else switch (c) {
		case '$':
			a = dol;
			
		case '.':
			if (opcnt)
				error(Q);
			break;
		case '\'':
			c = getchr();
			if (opcnt || c<'a' || 'z'<c)
				error(Q);
			a = zero;
			do a++; while (a<=dol && names[c-'a']!=(*a&~01));
			break;
		case '?':
			sign = -sign;
			
		case '/':
			compile(c);
			b = a;
			for (;;) {
				a += sign;
				if (a<zero)
					a = dol;
				if (a>dol)
					a = zero;
				if (execute(a))
					break;
				if (a==b)
					error(Q);
			}
			break;
		default:
			if (nextopand == opcnt) {
				a += sign;
				if (a<zero || dol<a)
					continue;       
			}
			if (c!='+' && c!='-' && c!='^') {
				peekc = c;
				if (opcnt==0)
					a = 0;
				return (a);
			}
			sign = 1;
			if (c!='+')
				sign = -sign;
			nextopand = ++opcnt;
			continue;
		}
		sign = 1;
		opcnt++;
	} while (zero<=a && a<=dol);
	error(Q);
	
}
int getnum()
{
	register int r, c;

	r = 0;
	while ((c=getchr())>='0' && c<='9')
		r = r*10 + c - '0';
	peekc = c;
	return (r);
}

setwide()
{
	if (!given) {
		addr1 = zero + (dol>zero);
		addr2 = dol;
	}
}

setnoaddr()
{
	if (given)
		error(Q);
}

nonzero()
{
	squeeze(1);
}

squeeze(i)
int i;
{
	if (addr1<zero+i || addr2>dol || addr1>addr2)
		error(Q);
}

newline()
{
	register c;

	if ((c = getchr()) == '\n' || c == -1)
		return;
	if (c=='p' || c=='l' || c=='n') {
		pflag++;
		if (c=='l')
			listf++;
		else if (c=='n')
			listn++;
		if ((c=getchr())=='\n')
			return;
	}
	error(Q);
}

filename(comm)
{
	register char *p1, *p2;
	register c;

	count = 0;
	c = getchr();
	if (c=='\n' || c==-1) {
		p1 = savedfile;
		if (*p1==0 && comm!='f')
			error(Q);
		p2 = file;
		while (*p2++ = *p1++)
			;
		return;
	}
	if (c!=' ')
		error(Q);
	while ((c = getchr()) == ' ')
		;
	if (c=='\n')
		error(Q);
	p1 = file;
	do {
		if (p1 >= &file[sizeof(file)-1] || c==' ' || c==-1)
			error(Q);
		*p1++ = c;
	} while ((c = getchr()) != '\n');
	*p1++ = 0;
	if (savedfile[0]==0 || comm=='e' || comm=='f') {
		p1 = savedfile;
		p2 = file;
		while (*p1++ = *p2++)
			;
	}
}

exfile()
{
	close(io);
	io = -1;
	if (vflag) {
		putd();
		putchr('\n');
	}
}

onintr()
{
	signal(2	, onintr);
	putchr('\n');
	lastc = '\n';
	error(Q);
}

onhup()
{
	signal(2	, 	(int (*)())1);
	signal(1	, 	(int (*)())1);
	if (dol > zero) {
		addr1 = zero+1;
		addr2 = dol;
		io = creat("ed.hup", 0666);
		if (io > 0)
			putfile();
	}
	fchange = 0;
	quit();
}

error(s)
char *s;
{
	register c;

	wrapp = 0;
	listf = 0;
	listn = 0;
	putchr('?');
	puts(s);
	count = 0;
	lseek(0, (long)0, 2);
	pflag = 0;
	if (globp)
		lastc = '\n';
	globp = 0;
	peekc = lastc;
	if(lastc)
		while ((c = getchr()) != '\n' && c != -1)
			;
	if (io > 0) {
		close(io);
		io = -1;
	}
	longjmp(savej, 1);
}

getchr()
{
	char c;
	if (lastc=peekc) {
		peekc = 0;
		return(lastc);
	}
	if (globp) {
		if ((lastc = *globp++) != 0)
			return(lastc);
		globp = 0;
		return(-1);
	}
	if (read(0, &c, 1) <= 0)
		return(lastc = -1);
	lastc = c&0177;
	return(lastc);
}

gettty()
{
	register int rc;

	if (rc = gety())
		return(rc);
	if (linebuf[0]=='.' && linebuf[1]==0)
		return(-1);
	return(0);
}

gety()
{
	register int c;
	register char *gf;
	register char *p;

	p = linebuf;
	gf = globp;
	while ((c = getchr()) != '\n') {
		if (c==-1) {
			if (gf)
				peekc = c;
			return(c);
		}
		if ((c &= 0177) == 0)
			continue;
		*p++ = c;
		if (p >= &linebuf[512-2])
			error(Q);
	}

	*p++ = 0;
	return(0);
}

getfile()
{
	register c;
	register char *lp, *fp;

	lp = linebuf;
	fp = nextip;
	do {
		if (--ninbuf < 0) {
			if ((ninbuf = read(io, genbuf, 512)-1) < 0)
				if (lp>linebuf) {
					puts("'\\n' appended");
					*genbuf = '\n';
				}
				else return(-1);
			fp = genbuf;
			while(fp < &genbuf[ninbuf]) {
				if (*fp++ & 0200) {
					if (kflag)
						crblock(perm, genbuf, ninbuf+1, count);
					break;
				}
			}
			fp = genbuf;
		}
		c = *fp++;
		if (c=='\0')
			continue;
		if (c&0200 || lp >= &linebuf[512]) {
			lastc = '\n';
			error(Q);
		}
		*lp++ = c;
		count++;
	} while (c != '\n');
	*--lp = 0;
	nextip = fp;
	return(0);
}

putfile()
{
	int *a1, n;
	register char *fp, *lp;
	register nib;

	nib = 512;
	fp = genbuf;
	a1 = addr1;
	do {
		lp = getline(*a1++);
		for (;;) {
			if (--nib < 0) {
				n = fp-genbuf;
				if(kflag)
					crblock(perm, genbuf, n, count-n);
				if(write(io, genbuf, n) != n) {
					puts(WRERR);
					error(Q);
				}
				nib = 511;
				fp = genbuf;
			}
			count++;
			if ((*fp++ = *lp++) == 0) {
				fp[-1] = '\n';
				break;
			}
		}
	} while (a1 <= addr2);
	n = fp-genbuf;
	if(kflag)
		crblock(perm, genbuf, n, count-n);
	if(write(io, genbuf, n) != n) {
		puts(WRERR);
		error(Q);
	}
}

append(f, a)
int *a;
int (*f)();
{
	register *a1, *a2, *rdot;
	int nline, tl;

	nline = 0;
	dot = a;
	while ((*f)() == 0) {
		if ((dol-zero)+1 >= nlall) {
			int *ozero = zero;
			nlall += 512;
			if ((zero = (int *)realloc((char *)zero, nlall*sizeof(int)))==0) {
				error("MEM?");
				onhup();
			}
			dot += zero - ozero;
			dol += zero - ozero;
		}
		tl = putline();
		nline++;
		a1 = ++dol;
		a2 = a1+1;
		rdot = ++dot;
		while (a1 > rdot)
			*--a2 = *--a1;
		*rdot = tl;
	}
	return(nline);
}

add(i)
int i;
{
	if (i && (given || dol>zero)) {
		addr1--;
		addr2--;
	}
	squeeze(0);
	newline();
	append(gettty, addr2);
}

browse()
{
	register forward, n;
	static bformat;	
	static bnum; 
	forward=1;
	if((peekc=getchr())!='\n'){
		if (peekc=='-' || peekc=='+') {
			if(peekc=='-')
				forward=0;
			getchr();
		}
		if((n=getnum())>0)
			bpagesize=n;
	}
	newline();
	if (pflag) {
		bformat=listf;
		bnum=listn;
	} else {
		listf=bformat;
		listn=bnum;
	}
	if (forward) {
		addr1=addr2;
		if((addr2+=bpagesize)>dol)
			addr2=dol;
	} else {
		if((addr1=addr2-bpagesize)<=zero)
			addr1=zero+1;
	}
	print();
}

callunix()
{
	register (*savint)(), pid, rpid;
	int retcode;

	setnoaddr();
	if ((pid = fork()) == 0) {
		signal(1	, oldhup);
		signal(3	, oldquit);
		execl("/bin/sh", "sh", "-t", 0);
		exit(0100);
	}
	savint = signal(2	, 	(int (*)())1);
	while ((rpid = wait(&retcode)) != pid && rpid != -1)
		;
	signal(2	, savint);
	if (vflag) {
		puts("!");
	}
}

quit()
{
	if (vflag && fchange && dol!=zero) {
		fchange = 0;
		error(Q);
	}
	unlink(tfname);
	exit(0);
}

rdelete(ad1, ad2)
int *ad1, *ad2;
{
	register *a1, *a2, *a3;

	a1 = ad1;
	a2 = ad2+1;
	a3 = dol;
	dol -= a2 - a1;
	do {
		*a1++ = *a2++;
	} while (a2 <= a3);
	a1 = ad1;
	if (a1 > dol)
		a1 = dol;
	dot = a1;
	fchange = 1;
}

gdelete()
{
	register *a1, *a2, *a3;

	a3 = dol;
	for (a1=zero; (*a1&01)==0; a1++)
		if (a1>=a3)
			return;
	for (a2=a1+1; a2<=a3;) {
		if (*a2&01) {
			a2++;
			dot = a1;
		} else
			*a1++ = *a2++;
	}
	dol = a1-1;
	if (dot>dol)
		dot = dol;
	fchange = 1;
}

char *
getline(tl)
{
	register char *bp, *lp;
	register nl;

	lp = linebuf;
	bp = getblock(tl, 0);
	nl = nleft;
	tl &= ~0377;
	while (*lp++ = *bp++)
		if (--nl == 0) {
			bp = getblock(tl+=0400, 0);
			nl = nleft;
		}
	return(linebuf);
}

putline()
{
	register char *bp, *lp;
	register nl;
	int tl;

	fchange = 1;
	lp = linebuf;
	tl = tline;
	bp = getblock(tl, 1);
	nl = nleft;
	tl &= ~0377;
	while (*bp = *lp++) {
		if (*bp++ == '\n') {
			*--bp = 0;
			linebp = lp;
			break;
		}
		if (--nl == 0) {
			bp = getblock(tl+=0400, 1);
			nl = nleft;
		}
	}
	nl = tline;
	tline += (((lp-linebuf)+03)>>1)&077776;
	return(nl);
}

char *
getblock(atl, iof)
{
	extern read(), write();
	register bno, off;
	register char *p1, *p2;
	register int n;
	
	bno = (atl>>8)&0377;
	off = (atl<<1)&0774;
	if (bno >= 255) {
		lastc = '\n';
		error(T);
	}
	nleft = 512 - off;
	if (bno==iblock) {
		ichanged |= iof;
		return(ibuff+off);
	}
	if (bno==oblock)
		return(obuff+off);
	if (iof==0) {
		if (ichanged) {
			if(xtflag)
				crblock(tperm, ibuff, 512, (long)0);
			blkio(iblock, ibuff, write);
		}
		ichanged = 0;
		iblock = bno;
		blkio(bno, ibuff, read);
		if(xtflag)
			crblock(tperm, ibuff, 512, (long)0);
		return(ibuff+off);
	}
	if (oblock>=0) {
		if(xtflag) {
			p1 = obuff;
			p2 = crbuf;
			n = 512;
			while(n--)
				*p2++ = *p1++;
			crblock(tperm, crbuf, 512, (long)0);
			blkio(oblock, crbuf, write);
		} else
			blkio(oblock, obuff, write);
	}
	oblock = bno;
	return(obuff+off);
}

blkio(b, buf, iofcn)
char *buf;
int (*iofcn)();
{
	lseek(tfile, (long)b<<9, 0);
	if ((*iofcn)(tfile, buf, 512) != 512) {
		error(T);
	}
}

init()
{
	register *markp;

	close(tfile);
	tline = 2;
	for (markp = names; markp < &names[26]; )
		*markp++ = 0;
	subnewa = 0;
	anymarks = 0;
	iblock = -1;
	oblock = -1;
	ichanged = 0;
	close(creat(tfname, 0600));
	tfile = open(tfname, 2);
	if(xflag) {
		xtflag = 1;
		makekey(key, tperm);
	}
	dot = dol = zero;
}

global(k)
{
	register char *gp;
	register c;
	register int *a1;
	char globuf[256];

	if (globp)
		error(Q);
	setwide();
	squeeze(dol>zero);
	if ((c=getchr())=='\n')
		error(Q);
	compile(c);
	gp = globuf;
	while ((c = getchr()) != '\n') {
		if (c==-1)
			error(Q);
		if (c=='\\') {
			c = getchr();
			if (c!='\n')
				*gp++ = '\\';
		}
		*gp++ = c;
		if (gp >= &globuf[256-2])
			error(Q);
	}
	if (gp == globuf)
		*gp++ = 'p';
	*gp++ = '\n';
	*gp++ = 0;
	for (a1=zero; a1<=dol; a1++) {
		*a1 &= ~01;
		if (a1>=addr1 && a1<=addr2 && execute(a1)==k)
			*a1 |= 01;
	}
	


	if (globuf[0]=='d' && globuf[1]=='\n' && globuf[2]=='\0') {
		gdelete();
		return;
	}
	for (a1=zero; a1<=dol; a1++) {
		if (*a1 & 01) {
			*a1 &= ~01;
			dot = a1;
			globp = globuf;
			commands();
			a1 = zero;
		}
	}
}

join()
{
	register char *gp, *lp;
	register *a1;

	nonzero();
	gp = genbuf;
	for (a1=addr1; a1<=addr2; a1++) {
		lp = getline(*a1);
		while (*gp = *lp++)
			if (gp++ >= &genbuf[512-2])
				error(Q);
	}
	lp = linebuf;
	gp = genbuf;
	while (*lp++ = *gp++)
		;
	*addr1 = putline();
	if (addr1<addr2)
		rdelete(addr1+1, addr2);
	dot = addr1;
}

substitute(inglob)
{
	register *markp, *a1, nl;
	int gsubf;
	int getsub();
	int n, m;

	n = getnum();	
	gsubf = compsub();
	for (a1 = addr1; a1 <= addr2; a1++) {
		if (execute(a1)){
			int *ozero;
			m = n;
			do {
				if (--m <= 0) {
					dosub();
					if (!gsubf)
						break;
				}
			} while (execute((int *)0));
			if (m <= 0) {
				inglob |= 01;
				subnewa = putline();
				*a1 &= ~01;
				if (anymarks) {
					for (markp = names; markp < &names[26]; markp++)
						if (*markp == *a1)
							*markp = subnewa;
				}
				subolda = *a1;
				*a1 = subnewa;
				ozero = zero;
				nl = append(getsub, a1);
				nl += zero-ozero;
				a1 += nl;
				addr2 += nl;
			}
		}
	}
	if (inglob==0)
		error(Q);
}

compsub()
{
	register seof, c;
	register char *p;

	if ((seof = getchr()) == '\n' || seof == ' ')
		error(Q);
	compile(seof);
	p = rhsbuf;
	for (;;) {
		c = getchr();
		if (c=='\\')
			c = getchr() | 0200;
		if (c=='\n') {
			if (globp && globp[0])	
				c |= 0200;
			else {
				peekc = c;
				pflag++;
				break;
			}
		}
		if (c==seof)
			break;
		*p++ = c;
		if (p >= &rhsbuf[512/2])
			error(Q);
	}
	*p++ = 0;
	if ((peekc = getchr()) == 'g') {
		peekc = 0;
		newline();
		return(1);
	}
	newline();
	return(0);
}

getsub()
{
	register char *p1, *p2;

	p1 = linebuf;
	if ((p2 = linebp) == 0)
		return(-1);
	while (*p1++ = *p2++)
		;
	linebp = 0;
	return(0);
}

dosub()
{
	register char *lp, *sp, *rp;
	int c;

	lp = linebuf;
	sp = genbuf;
	rp = rhsbuf;
	while (lp < loc1)
		*sp++ = *lp++;
	while (c = *rp++&0377) {
		if (c=='&') {
			sp = place(sp, loc1, loc2);
			continue;
		} else if (c&0200 && (c &= 0177) >='1' && c < nbra+'1') {
			sp = place(sp, braslist[c-'1'], braelist[c-'1']);
			continue;
		}
		*sp++ = c&0177;
		if (sp >= &genbuf[512])
			error(Q);
	}
	lp = loc2;
	loc2 = sp - genbuf + linebuf;
	while (*sp++ = *lp++)
		if (sp >= &genbuf[512])
			error(Q);
	lp = linebuf;
	sp = genbuf;
	while (*lp++ = *sp++)
		;
}

char *
place(sp, l1, l2)
register char *sp, *l1, *l2;
{

	while (l1 < l2) {
		*sp++ = *l1++;
		if (sp >= &genbuf[512])
			error(Q);
	}
	return(sp);
}

move(cflag)
{
	register int *adt, *ad1, *ad2;
	int getcopy();

	nonzero();
	if ((adt = address())==0)	
		error(Q);
	newline();
	if (cflag) {
		int *ozero, delta;
		ad1 = dol;
		ozero = zero;
		append(getcopy, ad1++);
		ad2 = dol;
		delta = zero - ozero;
		ad1 += delta;
		adt += delta;
	} else {
		ad2 = addr2;
		for (ad1 = addr1; ad1 <= ad2;)
			*ad1++ &= ~01;
		ad1 = addr1;
	}
	ad2++;
	if (adt<ad1) {
		dot = adt + (ad2-ad1);
		if ((++adt)==ad1)
			return;
		reverse(adt, ad1);
		reverse(ad1, ad2);
		reverse(adt, ad2);
	} else if (adt >= ad2) {
		dot = adt++;
		reverse(ad1, ad2);
		reverse(ad2, adt);
		reverse(ad1, adt);
	} else
		error(Q);
	fchange = 1;
}

reverse(a1, a2)
register int *a1, *a2;
{
	register int t;

	for (;;) {
		t = *--a2;
		if (a2 <= a1)
			return;
		*a2 = *a1;
		*a1++ = t;
	}
}

getcopy()
{
	if (addr1 > addr2)
		return(-1);
	getline(*addr1++);
	return(0);
}

compile(aeof)
{
	register eof, c;
	register char *ep;
	char *lastep;
	char bracket[5], *bracketp;
	int cclcnt;

	ep = expbuf;
	eof = aeof;
	bracketp = bracket;
	if ((c = getchr()) == '\n') {
		peekc = c;
		c = eof;
	}
	if (c == eof) {
		if (*ep==0)
			error(Q);
		return;
	}
	circfl = 0;
	nbra = 0;
	if (c=='^') {
		c = getchr();
		circfl++;
	}
	peekc = c;
	lastep = 0;
	for (;;) {
		if (ep >= &expbuf[256])
			goto cerror;
		c = getchr();
		if (c == '\n') {
			peekc = c;
			c = eof;
		}
		if (c==eof) {
			if (bracketp != bracket)
				goto cerror;
			*ep++ = 11;
			return;
		}
		if (c!='*')
			lastep = ep;
		switch (c) {

		case '\\':
			if ((c = getchr())=='(') {
				if (nbra >= 5)
					goto cerror;
				*bracketp++ = nbra;
				*ep++ = 1;
				*ep++ = nbra++;
				continue;
			}
			if (c == ')') {
				if (bracketp <= bracket)
					goto cerror;
				*ep++ = 12;
				*ep++ = *--bracketp;
				continue;
			}
			if (c>='1' && c<'1'+5) {
				*ep++ = 14;
				*ep++ = c-'1';
				continue;
			}
			*ep++ = 2;
			if (c=='\n')
				goto cerror;
			*ep++ = c;
			continue;

		case '.':
			*ep++ = 4;
			continue;

		case '\n':
			goto cerror;

		case '*':
			if (lastep==0 || *lastep==1 || *lastep==12)
				goto defchar;
			*lastep |= 01;
			continue;

		case '$':
			if ((peekc=getchr()) != eof && peekc!='\n')
				goto defchar;
			*ep++ = 10;
			continue;

		case '[':
			*ep++ = 6;
			*ep++ = 0;
			cclcnt = 1;
			if ((c=getchr()) == '^') {
				c = getchr();
				ep[-2] = 8;
			}
			do {
				if (c=='\n')
					goto cerror;
				if (c=='-' && ep[-1]!=0) {
					if ((c=getchr())==']') {
						*ep++ = '-';
						cclcnt++;
						break;
					}
					while (ep[-1]<c) {
						*ep = ep[-1]+1;
						ep++;
						cclcnt++;
						if (ep>=&expbuf[256])
							goto cerror;
					}
				}
				*ep++ = c;
				cclcnt++;
				if (ep >= &expbuf[256])
					goto cerror;
			} while ((c = getchr()) != ']');
			lastep[1] = cclcnt;
			continue;

		defchar:
		default:
			*ep++ = 2;
			*ep++ = c;
		}
	}
   cerror:
	expbuf[0] = 0;
	nbra = 0;
	error(Q);
}

execute(addr)
int *addr;
{
	register char *p1, *p2;
	register c;

	for (c=0; c<5; c++) {
		braslist[c] = 0;
		braelist[c] = 0;
	}
	if (addr == (int *)0) {
		if (circfl)
			return(0);
		locs = p1 = loc2;
	} else {
		if (addr==zero)
			return(0);
		p1 = getline(*addr);
		locs = 0;
	}
	p2 = expbuf;
	if (circfl) {
		loc1 = p1;
		return(advance(p1, p2));
	}
	
	if (*p2==2) {
		c = p2[1];
		do {
			if (*p1!=c)
				continue;
			if (advance(p1, p2)) {
				loc1 = p1;
				return(1);
			}
		} while (*p1++);
		return(0);
	}
	
	do {
		if (advance(p1, p2)) {
			loc1 = p1;
			return(1);
		}
	} while (*p1++);
	return(0);
}

advance(lp, ep)
register char *ep, *lp;
{
	register char *curlp;
	int i;

	for (;;) switch (*ep++) {

	case 2:
		if (*ep++ == *lp++)
			continue;
		return(0);

	case 4:
		if (*lp++)
			continue;
		return(0);

	case 10:
		if (*lp==0)
			continue;
		return(0);

	case 11:
		loc2 = lp;
		return(1);

	case 6:
		if (cclass(ep, *lp++, 1)) {
			ep += *ep;
			continue;
		}
		return(0);

	case 8:
		if (cclass(ep, *lp++, 0)) {
			ep += *ep;
			continue;
		}
		return(0);

	case 1:
		braslist[*ep++] = lp;
		continue;

	case 12:
		braelist[*ep++] = lp;
		continue;

	case 14:
		if (braelist[i = *ep++]==0)
			error(Q);
		if (backref(i, lp)) {
			lp += braelist[i] - braslist[i];
			continue;
		}
		return(0);

	case 14|01:
		if (braelist[i = *ep++] == 0)
			error(Q);
		curlp = lp;
		while (backref(i, lp))
			lp += braelist[i] - braslist[i];
		while (lp >= curlp) {
			if (advance(lp, ep))
				return(1);
			lp -= braelist[i] - braslist[i];
		}
		continue;

	case 4|01:
		curlp = lp;
		while (*lp++)
			;
		goto star;

	case 2|01:
		curlp = lp;
		while (*lp++ == *ep)
			;
		ep++;
		goto star;

	case 6|01:
	case 8|01:
		curlp = lp;
		while (cclass(ep, *lp++, ep[-1]==(6|01)))
			;
		ep += *ep;
		goto star;

	star:
		do {
			lp--;
			if (lp==locs)
				break;
			if (advance(lp, ep))
				return(1);
		} while (lp > curlp);
		return(0);

	default:
		error(Q);
	}
}

backref(i, lp)
register i;
register char *lp;
{
	register char *bp;

	bp = braslist[i];
	while (*bp++ == *lp++)
		if (bp >= braelist[i])
			return(1);
	return(0);
}

cclass(set, c, af)
register char *set;
register c;
{
	register n;

	if (c==0)
		return(0);
	n = *set++;
	while (--n)
		if (*set++ == c)
			return(af);
	return(!af);
}

putd()
{
	register r;

	r = count%10;
	count /= 10;
	if (count)
		putd();
	putchr(r + '0');
}

puts(sp)
register char *sp;
{
	col = 0;
	while (*sp)
		putchr(*sp++);
	putchr('\n');
}

char	line[70];
char	*linp	= line;

putchr(ac)
{
	register char *lp;
	register c;

	lp = linp;
	c = ac;
	if (listf) {
		if (c=='\n') {
			if (linp!=line && linp[-1]==' ') {
				*lp++ = '\\';
				*lp++ = 'n';
			}
		} else {
			if (col > (72-4-2)) {
				col = 8;
				*lp++ = '\\';
				*lp++ = '\n';
				*lp++ = '\t';
			}
			col++;
			if (c=='\b' || c=='\t' || c=='\\') {
				*lp++ = '\\';
				if (c=='\b')
					c = 'b';
				else if (c=='\t')
					c = 't';
				col++;
			} else if (c<' ' || c=='\177') {
				*lp++ = '\\';
				*lp++ =  (c>>6)    +'0';
				*lp++ = ((c>>3)&07)+'0';
				c     = ( c    &07)+'0';
				col += 3;
			}
		}
	}
	*lp++ = c;
	if(c == '\n' || lp >= &line[64]) {
		linp = line;
		write(oflag?2:1, line, lp-line);
		return;
	}
	linp = lp;
}
crblock(permp, buf, nchar, startn)
char *permp;
char *buf;
long startn;
{
	register char *p1;
	int n1;
	int n2;
	register char *t1, *t2, *t3;

	t1 = permp;
	t2 = &permp[256];
	t3 = &permp[512];

	n1 = startn&0377;
	n2 = (startn>>8)&0377;
	p1 = buf;
	while(nchar--) {
		*p1 = t2[(t3[(t1[(*p1+n1)&0377]+n2)&0377]-n2)&0377]-n1;
		n1++;
		if(n1==256){
			n1 = 0;
			n2++;
			if(n2==256) n2 = 0;
		}
		p1++;
	}
}

getkey()
{
	struct sgttyb b;
	int save;
	int (*sig)();
	register char *p;
	register c;

	sig = signal(2	, 	(int (*)())1);
	if (gtty(0, &b) == -1)
		error("Input not tty");
	save = b.sg_flags;
	b.sg_flags &= ~010;
	stty(0, &b);
	puts("Key:");
	p = key;
	while(((c=getchr()) != -1) && (c!='\n')) {
		if(p < &key[9])
			*p++ = c;
	}
	*p = 0;
	b.sg_flags = save;
	stty(0, &b);
	signal(2	, sig);
	return(key[0] != 0);
}





crinit(keyp, permp)
char	*keyp, *permp;
{
	register char *t1, *t2, *t3;
	register i;
	int ic, k, temp, pf[2];
	unsigned random;
	char buf[13];
	long seed;

	t1 = permp;
	t2 = &permp[256];
	t3 = &permp[512];
	if(*keyp == 0)
		return(0);
	strncpy(buf, keyp, 8);
	while (*keyp)
		*keyp++ = '\0';
	buf[8] = buf[0];
	buf[9] = buf[1];
	if (pipe(pf)<0)
		pf[0] = pf[1] = -1;
	if (fork()==0) {
		close(0);
		close(1);
		dup(pf[0]);
		dup(pf[1]);
		execl("/usr/lib/makekey", "-", 0);
		execl("/lib/makekey", "-", 0);
		exit(1);
	}
	write(pf[1], buf, 10);
	if (wait((int *)0)==-1 || read(pf[0], buf, 13)!=13)
		error("crypt: cannot generate key");
	close(pf[0]);
	close(pf[1]);
	seed = 123;
	for (i=0; i<13; i++)
		seed = seed*buf[i] + i;
	for(i=0;i<256;i++){
		t1[i] = i;
		t3[i] = 0;
	}
	for(i=0; i<256; i++) {
		seed = 5*seed + buf[i%13];
		random = seed % 65521;
		k = 256-1 - i;
		ic = (random&0377) % (k+1);
		random >>= 8;
		temp = t1[k];
		t1[k] = t1[ic];
		t1[ic] = temp;
		if(t3[k]!=0) continue;
		ic = (random&0377) % k;
		while(t3[ic]!=0) ic = (ic+1) % k;
		t3[k] = ic;
		t3[ic] = k;
	}
	for(i=0; i<256; i++)
		t2[t1[i]&0377] = i;
	return(1);
}

makekey(a, b)
char *a, *b;
{
	register int i;
	long t;
	char temp[9 + 1];

	for(i = 0; i < 9; i++)
		temp[i] = *a++;
	time(&t);
	t += getpid();
	for(i = 0; i < 4; i++)
		temp[i] ^= (t>>(8*i))&0377;
	crinit(temp, b);

}
