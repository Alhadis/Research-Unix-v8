#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#define	N	16
#define	C	20
#define NF	10

/*
 * Memory administration
 *
 * Using a lot of memory is great when sorting a lot of data.
 * Using a megabyte to sort the output of `who' loses big.
 * MAXMEM, MINMEM and DEFMEM define the absolute maximum,
 * minimum and default memory requirements.  Administrators
 * can override any or all of these via defines at compile time.
 * Users can override the amount allocated (within the limits
 * of MAXMEM and MINMEM) on the command line.
 */

#ifndef	MAXMEM
#define	MAXMEM	1048576	/* Megabyte maximum */
#endif

#ifndef	MINMEM
#define	MINMEM	16384	/* 16K minimum */
#endif

#ifndef	DEFMEM
#define	DEFMEM	131072	/* Same as old sort */
#endif


#define ASC 	0
#define NUM	1
#define MON	2

#define	blank(c) ((c)==' ' || (c)=='\t')
FILE	*is, *os;
char	*dirtry[] = {"/usr/tmp", "/tmp", NULL};
char	**dirs;
char	file1[30];
char	*file = file1;
char	*filep;
int	nfiles;
int	*lspace;
unsigned alloc,tryfor;
char bufin[BUFSIZ], bufout[BUFSIZ];	/* Use setbuf's to avoid malloc calls.
					** Malloc seems to develope heartburn
					** when brk returns storage.
					*/
int	maxrec;
int	cmp(), cmpa();
int	(*compare)() = cmpa;
char	*eol();
int	term();
int 	mflg;
int	mphase;
int	nway;
int	cflg;
int	uflg;
char	*outfil;
int unsafeout;	/*kludge to assure -m -o works*/
char	tabchar;
int 	eargc;
char	**eargv;

char zero[256];

char	fold[256] = {
	0200,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0212,0213,0214,0215,0216,0217,
	0220,0221,0222,0223,0224,0225,0226,0227,
	0230,0231,0232,0233,0234,0235,0236,0237,
	0240,0241,0242,0243,0244,0245,0246,0247,
	0250,0251,0252,0253,0254,0255,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0275,0276,0277,
	0300,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0312,0313,0314,0315,0316,0317,
	0320,0321,0322,0323,0324,0325,0326,0327,
	0330,0331,0332,0333,0334,0335,0336,0337,
	0340,0341,0342,0343,0344,0345,0346,0347,
	0350,0351,0352,0353,0354,0355,0356,0357,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0372,0373,0374,0375,0376,0377,
	0000,0001,0002,0003,0004,0005,0006,0007,
	0010,0011,0012,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0024,0025,0026,0027,
	0030,0031,0032,0033,0034,0035,0036,0037,
	0040,0041,0042,0043,0044,0045,0046,0047,
	0050,0051,0052,0053,0054,0055,0056,0057,
	0060,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0072,0073,0074,0075,0076,0077,
	0100,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0112,0113,0114,0115,0116,0117,
	0120,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0132,0133,0134,0134,0136,0137,
	0140,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0112,0113,0114,0115,0116,0117,
	0120,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0132,0173,0174,0175,0176,0177
};
char nofold[256] = {
	0200,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0212,0213,0214,0215,0216,0217,
	0220,0221,0222,0223,0224,0225,0226,0227,
	0230,0231,0232,0233,0234,0235,0236,0237,
	0240,0241,0242,0243,0244,0245,0246,0247,
	0250,0251,0252,0253,0254,0255,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0275,0276,0277,
	0300,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0312,0313,0314,0315,0316,0317,
	0320,0321,0322,0323,0324,0325,0326,0327,
	0330,0331,0332,0333,0334,0335,0336,0337,
	0340,0341,0342,0343,0344,0345,0346,0347,
	0350,0351,0352,0353,0354,0355,0356,0357,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0372,0373,0374,0375,0376,0377,
	0000,0001,0002,0003,0004,0005,0006,0007,
	0010,0011,0012,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0024,0025,0026,0027,
	0030,0031,0032,0033,0034,0035,0036,0037,
	0040,0041,0042,0043,0044,0045,0046,0047,
	0050,0051,0052,0053,0054,0055,0056,0057,
	0060,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0072,0073,0074,0075,0076,0077,
	0100,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0112,0113,0114,0115,0116,0117,
	0120,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0132,0133,0134,0135,0136,0137,
	0140,0141,0142,0143,0144,0145,0146,0147,
	0150,0151,0152,0153,0154,0155,0156,0157,
	0160,0161,0162,0163,0164,0165,0166,0167,
	0170,0171,0172,0173,0174,0175,0176,0177
};

char	nonprint[256] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1
};

char	dict[256] = {
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1
};

struct	field {
	char *code;
	char *ignore;
	int fcmp;
	int rflg;
	int bflg[2];
	int m[2];
	int n[2];
}	fields[NF];
struct field proto = {
	nofold+128,
	zero+128,
	ASC,
	1,
	0,0,
	0,-1,
	0,0
};
int	nfields;
int 	error = 1;
char	*setfil();

main(argc, argv)
char **argv;
{
	register a;
	char *arg;
	struct field *p, *q;
	int i;
	long incr;
	char *sbrk();
	char *sp;

	copyproto();
	eargv = argv;
	tryfor = DEFMEM;
	while (--argc > 0) {
		if(**++argv == '-') for(arg = *argv;;) {
			switch(*++arg) {
			case '\0':
				if(arg[-1] == '-')
					eargv[eargc++] = "-";
				break;

			case 'o':
				if(--argc > 0)
					outfil = *++argv;
				continue;

			case 'T':
				if (--argc > 0)
					dirtry[0] = *++argv;
				continue;

			default:
				field(++*argv,nfields>0);
				break;
			}
			break;
		} else if (**argv == '+') {
			if(++nfields>=NF) {
				diag("too many keys","");
				exit(1);
			}
			copyproto();
			field(++*argv,0);
		} else
			eargv[eargc++] = *argv;
	}
	q = &fields[0];
	for(a=1; a<=nfields; a++) {
		p = &fields[a];
		if(p->code != proto.code) continue;
		if(p->ignore != proto.ignore) continue;
		if(p->fcmp != proto.fcmp) continue;
		if(p->rflg != proto.rflg) continue;
		if(p->bflg[0] != proto.bflg[0]) continue;
		if(p->bflg[1] != proto.bflg[1]) continue;
		p->code = q->code;
		p->ignore = q->ignore;
		p->fcmp = q->fcmp;
		p->rflg = q->rflg;
		p->bflg[0] = p->bflg[1] = q->bflg[0];
	}
	if(eargc == 0)
		eargv[eargc++] = "-";
	if(cflg && eargc>1) {
		diag("can check only 1 file","");
		exit(1);
	}

	safeoutfil();

	sp = sbrk(0);
	lspace = (int *) sp;
	if (!mflg && !cflg)  {
		if (tryfor < MINMEM) tryfor = MINMEM;
		else if (tryfor > MAXMEM) tryfor = MAXMEM;
		for (incr=tryfor; (sp + incr) <= sp; incr >>= 1);
		do {
			if ((long)alloc+incr <= tryfor && brk(sp+incr) == 0) {
				sp += incr;
				alloc += incr;
			}
		} while ( ( incr >>= 1 ) >= 512L );
		alloc -= 512;	/* for recursion? */
		if ( brk((char *) lspace + alloc) != 0) {
			diag("allocation error before sort", "");
			exit(1);
		}
	}

	a = -1;
	for(dirs=dirtry; *dirs; dirs++) {
		(void) sprintf(filep=file1, "%s/stm%.5uaa", *dirs, getpid());
		while (*filep)
			filep++;
		filep -= 2;
		if ( (a=creat(file, 0600)) >=0)
			break;
	}
	if(a < 0) {
		diag("can't locate temp","");
		exit(1);
	}
	(void) close(a);
	(void) unlink(file);
	if (signal(SIGHUP, SIG_IGN) != SIG_IGN)
		(void) signal(SIGHUP, term);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		(void) signal(SIGINT, term);
	(void) signal(SIGPIPE,term);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		(void) signal(SIGTERM,term);
	nfiles = eargc;
	if(!mflg && !cflg) {
		sort();
		if (ferror(stdin))
			rderror("stdin");
		(void) fclose(stdin);
	}

	mphase = 1;
	if (maxrec == 0)  maxrec = 512;
	alloc = (N + 1) * maxrec + N * BUFSIZ;
	for (nway = N; nway >= 2; --nway) {
		if (brk((char *)lspace + alloc) == 0) break;
		alloc -= maxrec + BUFSIZ;
	}
	if (nway < 2) {
		diag("allocation error before merge", "");
		term();
	}

	if (cflg)   checksort();

	a = mflg || cflg ? 0 : eargc;
	if ((i = nfiles - a) > nway) {	/* Do leftovers early */
		if ((i %= (nway - 1)) == 0)
			i = nway - 1;
		if (i != 1)  {
			newfile();
			(void) setbuf(os, bufout);
			merge(a,a+i);
			a += i;
		}
	}
	for(; a+nway<nfiles || unsafeout&&a<eargc; a=i) {
		i = a+nway;
		if(i>=nfiles)
			i = nfiles;
		newfile();
		(void) setbuf(os, bufout);
		merge(a, i);
	}
	if(a != nfiles) {
		oldfile();
		(void) setbuf(os, bufout);
		merge(a, nfiles);
	}
	error = 0;
	term();
}

sort()
{
	register char *cp;
	register c;
	register char **lp;
	char *keep, *ekeep, *ep, **tp;
	int done, i;
	char *f;

	/*
	** Records are read in from the front of the buffer area.
	** Pointers to the records are allocated from the back of the buffer.
	** If a partially read record exhausts the buffer, it is saved and
	** then copied to the start of the buffer for processing with the
	** next coreload.
	*/
	done = 0;
	keep = 0;
	i = 0;
	c = EOF;
	ep = (char *) lspace + alloc;
	do {
		lp = (char **)ep - 1;
		cp = (char *) lspace;
		if (keep != 0) { /* part of record from previous coreload */
			*lp-- = cp;
			for(; keep < ekeep; *cp++ = *keep++);
		}
		while (cp < (char *) lp)  {
			if (keep == 0)  *lp-- = cp;
			while (cp + 1 < (char *) lp && c != '\n')  {
				if(c != EOF) {
					*cp++ = c;
					c = getc(is);
					continue;
				} else if(is) {
					if(ferror(is))
						rderror(f);
					(void) fclose(is);
				}
				if(i < eargc) {
					if((f = setfil(i++)) == 0)
						is = stdin;
					else if((is = fopen(f, "r")) == NULL)
						cant(f);
					(void) setbuf(is, bufin);
					c = getc(is);
				} else
					break;
			}
			if (c == '\n')  {
				*cp++ = '\n';
				keep = 0;
			}
			else if (c != EOF) { /* save partial record */
				lp++;
				if (keep != 0) {
					diag("whopper record won't fit", "");
					term();
				}
				keep = *lp;
				ekeep = cp;
				break;
			}
			if(c == EOF) {
				if(ferror(is))
					rderror(f);
				done++;
				lp++;
				break;
			}
			c = getc(is);
		}
		lp++;
		qsort(lp, (char **) ep);
		if(done == 0 || nfiles != eargc)
			newfile();
		else
			oldfile();
		(void) setbuf(os, bufout);
		tp = (char **) ep;
		while(tp > lp)  {
			cp = *--tp;
			if(*cp) {
				do
				putc(*cp, os);
				while(*cp++ != '\n');
			}
		}
		if (ferror(os))
			wterror("sorting");
		(void) fclose(os);
	} while(done == 0);
}

char *heap[N];
int lowkid = 0;	/*
		** Index of lesser subtree of heap, if known.
		** Useful if records are mostly ordered, with long runs of
		** records being written from the same input file before
		** the heap is really restructured.
		*/


#	define EXC(a, b) t = a; a = b; b = t

merge(a, b)
{
	FILE *tfile[N];
	char *buffer = (char *) lspace;
	register int nf;		/* number of merge files */
	register int i;
	int	j;
	register char	*cp;
	char	*t, *f;
	char	*save,*iobuf;

	save = (char *) lspace + (nway * maxrec);
	iobuf = save + maxrec;
	for (nf=0, i=a; i < b; i++)  {
		f = setfil(i);
		if (f == 0)
			tfile[nf] = stdin;
		else if ((tfile[nf] = fopen(f, "r")) == NULL)
			cant(f);
		heap[nf] = buffer + (nf * maxrec);
		(void) setbuf(tfile[nf],iobuf);
		iobuf += BUFSIZ;
		if (rline(tfile[nf], heap[nf])==0)
			nf++;
		else {
			if(ferror(tfile[nf]))
				rderror(f);
			(void) fclose(tfile[nf]);
		}
	}

	qsort(heap, (heap + nf));

	for (i=0, j=nf-1; i < j ; i++, --j ) { /* qsort is backwards */
		EXC(heap[i], heap[j]);
	}
	lowkid = 1;

	while (nf > 0)  {
		cp = heap[0];
		do
			putc(*cp, os);
		while (*cp++ != '\n');

		if (uflg) {
			t = save;
			cp = heap[0];
			do
				*t++ = *cp;
			while (*cp++ != '\n');
		}

		/* Get another record and reheap.  Bypass repeats if uflg. */

		do {
			i = (heap[0] - buffer) / maxrec;
			if (rline(tfile[i], heap[0])) {
				if (ferror(tfile[i]))
					rderror(setfil(i+a));
				(void) fclose(tfile[i]);
				if (--nf <= 0) break;
				heap[0] = heap[nf];
				if (nf < 3) lowkid = 0;
			}
			reheap(nf);
		} while (uflg && (*compare)(heap[0], save) == 0 );
	}
	for (i=a; i < b; i++) {
		if (i >= eargc)
			(void) unlink(setfil(i));
	}
	if (ferror(os))
		wterror("merging");
	(void) fclose(os);
}


reheap(ind)
register int	ind;
{
	register int i, j;
	char *t;

	--ind;	/* last legitimate index */
	for (i=0; (j = 2*i + 1) <= ind; i = j) {
		if (lowkid != 0)
			j = lowkid;
		else if ( (j < ind) && (*compare)(heap[j],heap[j+1]) <= 0 )
			j++;
		if ( (*compare)(heap[i], heap[j]) >= 0 ) {
			if (i == 0) lowkid = j;
			return;
		}
		EXC(heap[i], heap[j]);
		lowkid = 0;
	}
	return;
}
rline(iop, s)
FILE *iop;
register char *s;
{
	register char *ce;
	register int c;

	ce = s + maxrec;
	do  {
		c = getc(iop);
		if (c == EOF)
			return(1);
		if (s >= ce)
			--s;
		*s++ = c;
	}  while (c != '\n');
	return(0);
}


checksort()
{
	char *f;
	char *s[2];
	register int i, j, r;

	f = setfil(0);
	if (f == 0)
		is = stdin;
	else if ((is = fopen(f, "r")) == NULL)
		cant(f);
	(void) setbuf(is, bufin);

	i = 0;   j = 1;
	s[0] = (char *) lspace;
	s[1] = s[0] + maxrec;
	if ( rline(is, s[0]) ) {
		if (ferror(is)) {
			rderror(f);
		}
		(void) fclose(is);
		exit(0);
	}
	while ( !rline(is, s[j]) )  {
		r = (*compare)(s[i], s[j]);
		if (r < 0)
			disorder("disorder: ", s[j]);
		if (r == 0 && uflg)
			disorder("non-unique: ", s[j]);
		r = i;  i = j; j = r;
	}
	if (ferror(is))
		rderror(f);
	(void) fclose(is);
	exit(0);
}


disorder(s,t)
char *s, *t;
{
	register char *u;
	for(u=t; *u!='\n';u++) ;
	*u = 0;
	diag(s,t);
	term();
}

newfile()
{
	register char *f;

	f = setfil(nfiles);
	if((os=fopen(f, "w")) == NULL) {
		diag("can't create ",f);
		term();
	}
	nfiles++;
}

char *
setfil(i)
{

	if(i < eargc)
		if(eargv[i][0] == '-' && eargv[i][1] == '\0')
			return(0);
		else
			return(eargv[i]);
	i -= eargc;
	filep[0] = i/26 + 'a';
	filep[1] = i%26 + 'a';
	return(file);
}

oldfile()
{

	if(outfil) {
		if((os=fopen(outfil, "w")) == NULL) {
			diag("can't create ",outfil);
			term();
		}
	} else
		os = stdout;
}

safeoutfil()
{
	register int i;
	struct stat ostat,istat;

	if(!mflg||outfil==0)
		return;
	if(stat(outfil,&ostat)==-1)
		return;
	if ((i = eargc - N) < 0) i = 0;	/*-N is suff., not nec. */
	for (; i < eargc; i++) {
		if(stat(eargv[i],&istat)==-1)
			continue;
		if(ostat.st_dev==istat.st_dev&&
		   ostat.st_ino==istat.st_ino)
			unsafeout++;
	}
}

cant(f)
char *f;
{

	diag("can't open ",f);
	term();
}

diag(s,t)
char *s, *t;
{
	(void) fputs("sort: ",stderr);
	(void) fputs(s,stderr);
	(void) fputs(t,stderr);
	(void) fputs("\n",stderr);
}

term()
{
	register i;

	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGTERM, SIG_IGN);
	if(nfiles == eargc)
		nfiles++;
	for(i=eargc; i<=nfiles; i++) {	/*<= in case of interrupt*/
		(void) unlink(setfil(i));	/*with nfiles not updated*/
	}
	exit(error);
}

cmp(i, j)
char *i, *j;
{
	register char *pa, *pb;
	char *skip();
	char *code, *ignore;
	int a, b;
	int k;
	char *la, *lb;
	register int sa;
	int sb;
	char *ipa, *ipb, *jpa, *jpb;
	struct field *fp;

	for(k = nfields>0; k<=nfields; k++) {
		fp = &fields[k];
		pa = i;
		pb = j;
		if(k) {
			la = skip(pa, fp, 1);
			pa = skip(pa, fp, 0);
			lb = skip(pb, fp, 1);
			pb = skip(pb, fp, 0);
		} else {
			la = eol(pa);
			lb = eol(pb);
		}
		if(fp->fcmp==NUM) {
			sa = sb = fp->rflg;
			while(blank(*pa))
				pa++;
			while(blank(*pb))
				pb++;
			if(*pa == '-') {
				pa++;
				sa = -sa;
			}
			if(*pb == '-') {
				pb++;
				sb = -sb;
			}
			for(ipa = pa; ipa<la&&isdigit(*ipa); ipa++) ;
			for(ipb = pb; ipb<lb&&isdigit(*ipb); ipb++) ;
			jpa = ipa;
			jpb = ipb;
			a = 0;
			if(sa==sb)
				while(ipa > pa && ipb > pb)
					if(b = *--ipb - *--ipa)
						a = b;
			while(ipa > pa)
				if(*--ipa != '0')
					return(-sa);
			while(ipb > pb)
				if(*--ipb != '0')
					return(sb);
			if(a) return(a*sa);
			if(*(pa=jpa) == '.')
				pa++;
			if(*(pb=jpb) == '.')
				pb++;
			if(sa==sb)
				while(pa<la && isdigit(*pa)
				   && pb<lb && isdigit(*pb))
					if(a = *pb++ - *pa++)
						return(a*sa);
			while(pa<la && isdigit(*pa))
				if(*pa++ != '0')
					return(-sa);
			while(pb<lb && isdigit(*pb))
				if(*pb++ != '0')
					return(sb);
			continue;
		} else if(fp->fcmp==MON)  {
			sa = fp->rflg*(month(pb)-month(pa));
			if(sa)
				return(sa);
			else
				continue;
		}
		code = fp->code;
		ignore = fp->ignore;
loop: 
		while(ignore[*pa])
			pa++;
		while(ignore[*pb])
			pb++;
		if(pa>=la || *pa=='\n')
			if(pb<lb && *pb!='\n')
				return(fp->rflg);
			else continue;
		if(pb>=lb || *pb=='\n')
			return(-fp->rflg);
		if((sa = code[*pb++]-code[*pa++]) == 0)
			goto loop;
		return(sa*fp->rflg);
	}
	if(uflg)
		return(0);
	return(cmpa(i, j));
}

cmpa(pa, pb)
register char *pa, *pb;
{
	while(*pa == *pb++)
		if(*pa++ == '\n')
			return(0);
	return(
		*pa == '\n' ? fields[0].rflg:
		*--pb == '\n' ?-fields[0].rflg:
		*pb > *pa   ? fields[0].rflg:
		-fields[0].rflg
	);
}

char *
skip(pp, fp, j)
struct field *fp;
char *pp;
{
	register i;
	register char *p;

	p = pp;
	if( (i=fp->m[j]) < 0)
		return(eol(p));
	while(i-- > 0) {
		if(tabchar != 0) {
			while(*p != tabchar)
				if(*p != '\n')
					p++;
				else goto ret;
			p++;
		} else {
			while(blank(*p))
				p++;
			while(!blank(*p))
				if(*p != '\n')
					p++;
				else goto ret;
		}
	}
	if(fp->bflg[j])
		while(blank(*p))
			p++;
	i = fp->n[j];
	if(i==0 && j!=0 && fp->m[j]>0 && p[-1]==tabchar)
		p--;
	while((i-- > 0) && (*p != '\n'))
		p++;
ret:
	return(p);
}

char *
eol(p)
register char *p;
{
	while(*p != '\n') p++;
	return(p);
}

copyproto()
{
	register i;
	register int *p, *q;

	p = (int *)&proto;
	q = (int *)&fields[nfields];
	for(i=0; i<sizeof(proto)/sizeof(*p); i++)
		*q++ = *p++;
}

field(s,k)
char *s;
{
	register struct field *p;
	register d;
	p = &fields[nfields];
	d = 0;
	for(; *s!=0; s++) {
		switch(*s) {
		case '\0':
			return;

		case 'b':
			p->bflg[k]++;
			break;

		case 'd':
			p->ignore = dict+128;
			break;

		case 'f':
			p->code = fold+128;
			break;
		case 'i':
			p->ignore = nonprint+128;
			break;

		case 'c':
			cflg = 1;
			continue;

		case 'm':
			mflg = 1;
			continue;

		case 'M':
			p->fcmp = MON;
			break;

		case 'n':
			p->fcmp = NUM;
			break;
		case 't':
			tabchar = *++s;
			if(tabchar == 0) s--;
			continue;

		case 'r':
			p->rflg = -1;
			continue;
		case 'u':
			uflg = 1;
			continue;

		case 'y':
			if ( *++s ) tryfor = number(&s);
			else {
				--s;
				tryfor = MAXMEM;
			}
			continue;

		case 'z':
			if ( *++s )
				maxrec = number(&s);
			else --s;
			continue;

		case '.':
			if(p->m[k] == -1)	/* -m.n with m missing */
				p->m[k] = 0;
			d = &fields[0].n[0]-&fields[0].m[0];
			if (*++s == '\0') {
				--s;
				p->m[k+d] = 0;
				continue;
			}

		default:
			p->m[k+d] = number(&s);
		}
		compare = cmp;
	}
}

number(ppa)
char **ppa;
{
	int n;
	register char *pa;
	pa = *ppa;
	n = 0;
	while(isdigit(*pa)) {
		n = n*10 + *pa - '0';
		*ppa = pa++;
	}
	return(n);
}

#define qsexc(p,q) t= *p;*p= *q;*q=t
#define qstexc(p,q,r) t= *p;*p= *r;*r= *q;*q=t

qsort(a,l)
char **a, **l;
{
	register char **i, **j;
	char **k;
	char **lp, **hp;
	int c;
	char *t;
	unsigned n;


start:
	if((n=l-a) <= 1)
		return;

	n /= 2;
	hp = lp = a+n;
	i = a;
	j = l-1;


	for(;;) {
		if(i < lp) {
			if((c = (*compare)(*i, *lp)) == 0) {
				--lp;
				qsexc(i, lp);
				continue;
			}
			if(c < 0) {
				++i;
				continue;
			}
		}

loop:
		if(j > hp) {
			if((c = (*compare)(*hp, *j)) == 0) {
				++hp;
				qsexc(hp, j);
				goto loop;
			}
			if(c > 0) {
				if(i == lp) {
					++hp;
					qstexc(i, hp, j);
					i = ++lp;
					goto loop;
				}
				qsexc(i, j);
				--j;
				++i;
				continue;
			}
			--j;
			goto loop;
		}


		if(i == lp) {
			if(uflg&&!mphase)
				for(k=lp+1; k<=hp;) **k++ = '\0';
			if(lp-a >= l-hp) {
				qsort(hp+1, l);
				l = lp;
			} else {
				qsort(a, lp);
				a = hp+1;
			}
			goto start;
		}


		--lp;
		qstexc(j, lp, i);
		j = --hp;
	}
}

char * months[] = {
	"jan",
	"feb",
	"mar",
	"apr",
	"may",
	"jun",
	"jul",
	"aug",
	"sep",
	"oct",
	"nov",
	"dec"
};
month(s)
char *s;
{
	register char *t, *u;
	register i;
	char	*f = fold + 128;

	while(blank(*s))
		s++;
	for(i=0; i<sizeof(months)/sizeof(*months); i++) {
		for(t=s,u=months[i]; f[*t++]==f[*u++]; )
			if(*u==0)
				return(i);
	}
	return(-1);
}


rderror(s)
char *s;
{
	diag("read error on ", s == 0 ? "stdin" : s);
	term();
}

wterror(s)
char *s;
{
	diag("write error while ", s);
	term();
}
