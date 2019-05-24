/*
 * split -- break up a file on specified boundaries
 *
 *	status returns:
 *		0 - ok, and some matches
 *		1 - ok, but no matches
 *		2 - some error
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>

#define	CBRA	1
#define	CCHR	2
#define	CDOT	4
#define	CCL	6
#define	NCCL	8
#define	CDOL	10
#define	CEOF	11
#define	CKET	12
#define	CBACK	18

#define	STAR	01

#define	LBSIZE	512
#define	ESIZE	256
#define	NBRA	9

char	bracket[NBRA];
char	numbra;
char	nbra;
char	linebuf[LBSIZE+1];
char	filebuf[LBSIZE+1];
char	ybuf[ESIZE];
char	*fflag="x";
char	*sflag="";
unsigned	nfile;
unsigned	nline;
int	nflag=1000;
int	xflag;
int	yflag;
int	retcode = 0;
int	circf;
int	nsucc;
char	*braslist[NBRA];
char	*braelist[NBRA];
char	bittab[] = {
	1,
	2,
	4,
	8,
	16,
	32,
	64,
	128
};
struct exp{
	char expbuf[ESIZE];
	struct exp *next;
}*firstexp, *lastexp, *expp, *malloc();

main(argc, argv)
char **argv;
{
	while (--argc > 0 && (++argv)[0][0]=='-')
		switch (argv[0][1]) {
		case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9': 
			nflag=atoi(&argv[0][1]);
			if(nflag<=0)
				errexit("split: invalid numeric interval %s\n", argv[0]);
			continue;

		case 'f':
			--argc; ++argv;
			if(argc<=0)
				errexit("split: too few args for -f\n", (char *)NULL);
			fflag= *argv;
			if(fflag[0]=='\0')
				errexit("split: null file name specified for -f\n", (char *)NULL);
			continue;

		case 's':
			--argc; ++argv;
			if(argc<=0)
				errexit("split: too few args for -s\n", (char *)NULL);
			sflag= *argv;
			continue;

		case 'x':
			xflag++;
			continue;

		case 'y':
			yflag++;
			continue;

		case 'e':
			--argc;
			++argv;
			if(*argv==0 || **argv=='\0')
				errexit("split: null expression for -e\n", (char *)NULL);
			if (yflag) {
				register char *p, *s;
				for (s = ybuf, p = *argv; *p; ) {
					if (*p == '\\') {
						*s++ = *p++;
						if (*p)
							*s++ = *p++;
					} else if (*p == '[') {
						while (*p != '\0' && *p != ']')
							*s++ = *p++;
					} else if (islower(*p)) {
						*s++ = '[';
						*s++ = toupper(*p);
						*s++ = *p++;
						*s++ = ']';
					} else
						*s++ = *p++;
					if (s >= ybuf+ESIZE-5)
						errexit("split: argument too long\n", (char *)NULL);
				}
				*s = '\0';
				*argv = ybuf;
			}
			compile(*argv);
			nflag=0;
			continue;

		default:
			errexit("split: unknown flag\n", (char *)NULL);
			continue;
		}
	if(nflag)
		succeed(1);	/* Create first file */
	if (argc<=0)
		execute((char *)NULL);
	else
		execute(*argv);
	return (retcode != 0 ? retcode : nsucc == 0);
}

compile(astr)
char *astr;
{
	register c;
	register char *ep, *sp;
	char *cstart;
	char *lastep;
	char *bracketp;
	int cclcnt;
	int closed;
	char neg;

	expp=malloc(sizeof *expp);
	if(expp==NULL)
		errexit("split: too many expressions; can't malloc\n", (char *)NULL);
	if(firstexp==0) {
		firstexp=expp;
		lastexp=expp;
	} else {
		lastexp->next = expp;
		lastexp = expp;
	}
	expp->next=0;
	ep = expp->expbuf;
	sp = astr;
	lastep = 0;
	bracketp = bracket;
	closed = numbra = 0;
	if (*sp == '^') {
		circf++;
		sp++;
	}
	for (;;) {
		if (ep >= &expp->expbuf[ESIZE])
			goto cerror;
		if ((c = *sp++) != '*')
			lastep = ep;
		switch (c) {

		case '\0':
			*ep++ = CEOF;
			if(expp==firstexp)
				nbra=numbra;
			else if(nbra!=numbra)
				errexit("split: inconsistent parentheses in expression %s\n", astr);
			return;

		case '.':
			*ep++ = CDOT;
			continue;

		case '*':
			if (lastep==0 || *lastep==CBRA || *lastep==CKET)
				goto defchar;
			*lastep |= STAR;
			continue;

		case '$':
			if (*sp != '\0')
				goto defchar;
			*ep++ = CDOL;
			continue;

		case '[':
			if(&ep[17] >= &expp->expbuf[ESIZE])
				goto cerror;
			*ep++ = CCL;
			neg = 0;
			if((c = *sp++) == '^') {
				neg = 1;
				c = *sp++;
			}
			cstart = sp;
			do {
				if (c=='\0')
					goto cerror;
				if (c=='-' && sp>cstart && *sp!=']') {
					for (c = sp[-2]; c<*sp; c++)
						ep[c>>3] |= bittab[c&07];
					sp++;
				}
				ep[c>>3] |= bittab[c&07];
			} while((c = *sp++) != ']');
			if(neg) {
				for(cclcnt = 0; cclcnt < 16; cclcnt++)
					ep[cclcnt] ^= -1;
				ep[0] &= 0376;
			}

			ep += 16;

			continue;

		case '\\':
			if((c = *sp++) == '(') {
				if(numbra >= NBRA) {
					goto cerror;
				}
				*bracketp++ = numbra;
				*ep++ = CBRA;
				*ep++ = numbra++;
				continue;
			}
			if(c == ')') {
				if(bracketp <= bracket) {
					goto cerror;
				}
				*ep++ = CKET;
				*ep++ = *--bracketp;
				closed++;
				continue;
			}

			if(c >= '1' && c <= '9') {
				if((c -= '1') >= closed)
					goto cerror;
				*ep++ = CBACK;
				*ep++ = c;
				continue;
			}

		defchar:
		default:
			*ep++ = CCHR;
			*ep++ = c;
		}
	}
    cerror:
	errexit("split: RE error\n", (char *)NULL);
}

execute(file)
char *file;
{
	register char *p1, *p2;
	register c;

	if (file) {
		if (freopen(file, "r", stdin) == NULL) {
			fprintf(stderr, "split: can't open %s\n", file);
			retcode = 2;
			return;
		}
	}
	expp=firstexp;
	for (;;) {
		p1 = linebuf;
		if(expp==firstexp){
			while ((c = getchar()) != '\n') {
				if (c == EOF)
					return;
				*p1++ = c;
				if (p1 >= &linebuf[LBSIZE-2])
					break;
			}
			*p1++ = '\0';
			p1 = linebuf;
			if(nflag){
				printf("%s\n", linebuf);
				if(++nline>=nflag){
					succeed(1);
					nline=0;
				}
				continue;
			}
		}
		p2 = expp->expbuf;
		if (circf) {
			if (advance(p1, p2))
				goto found;
			goto nfound;
		}
		/* fast check for first character */
		if (*p2==CCHR) {
			c = p2[1];
			do {
				if (*p1!=c)
					continue;
				if (advance(p1, p2))
					goto found;
			} while (*p1++);
			goto nfound;
		}
		/* regular algorithm */
		do {
			if (advance(p1, p2))
				goto found;
		} while (*p1++);
	nfound:
		if((expp=expp->next)==0){
			expp=firstexp;
			printf("%s\n", linebuf);
		}
		continue;
	found:
		succeed(xflag);
		expp=firstexp;
	}
}

advance(lp, ep)
register char *lp, *ep;
{
	register char *curlp;
	char c;
	char *bbeg;
	int ct;

	for (;;) switch (*ep++) {

	case CCHR:
		if (*ep++ == *lp++)
			continue;
		return(0);

	case CDOT:
		if (*lp++)
			continue;
		return(0);

	case CDOL:
		if (*lp==0)
			continue;
		return(0);

	case CEOF:
		return(1);

	case CCL:
		c = *lp++ & 0177;
		if(ep[c>>3] & bittab[c & 07]) {
			ep += 16;
			continue;
		}
		return(0);
	case CBRA:
		braslist[*ep++] = lp;
		continue;

	case CKET:
		braelist[*ep++] = lp;
		continue;

	case CBACK:
		bbeg = braslist[*ep];
		if (braelist[*ep]==0)
			return(0);
		ct = braelist[*ep++] - bbeg;
		if(ecmp(bbeg, lp, ct)) {
			lp += ct;
			continue;
		}
		return(0);

	case CBACK|STAR:
		bbeg = braslist[*ep];
		if (braelist[*ep]==0)
			return(0);
		ct = braelist[*ep++] - bbeg;
		curlp = lp;
		while(ecmp(bbeg, lp, ct))
			lp += ct;
		while(lp >= curlp) {
			if(advance(lp, ep))	return(1);
			lp -= ct;
		}
		return(0);


	case CDOT|STAR:
		curlp = lp;
		while (*lp++);
		goto star;

	case CCHR|STAR:
		curlp = lp;
		while (*lp++ == *ep);
		ep++;
		goto star;

	case CCL|STAR:
		curlp = lp;
		do {
			c = *lp++ & 0177;
		} while(ep[c>>3] & bittab[c & 07]);
		ep += 16;
		goto star;

	star:
		if(--lp == curlp) {
			continue;
		}

		if(*ep == CCHR) {
			c = ep[1];
			do {
				if(*lp != c)
					continue;
				if(advance(lp, ep))
					return(1);
			} while(lp-- > curlp);
			return(0);
		}

		do {
			if (advance(lp, ep))
				return(1);
		} while (lp-- > curlp);
		return(0);

	default:
		errexit("split RE botch\n", (char *)NULL);
	}
}

char *
suffix()
{
	static char s[3];
	if(nfile>26*26)
		errexit("split: too many files (max 26*26)\n", (char *)NULL);
	s[0]='a'+nfile/26;
	s[1]='a'+nfile%26;
	s[2]='\0';
	nfile++;
	return(s);
}

char *
filename()
{
	extern char *strcat(), *strcpy();
	if(numbra==0)
		return(strcat(strcpy(filebuf, fflag), suffix()));
	if(braslist[0]>=braelist[0])
		errexit("split: null file name match; line:\n%s\n", linebuf);
	(void) strncpy(filebuf, braslist[0], braelist[0]-braslist[0]);
	if(yflag)
		lowercase(filebuf);
	(void) strcpy(&filebuf[braelist[0]-braslist[0]], sflag);
	return(filebuf);
}

lowercase(s)
	register char *s;
{
	do
		if(isupper(*s))
			*s=tolower(*s);
	while(*s++);
}

succeed(xflag)
{
	long ftell();
	nsucc = 1;
	if(freopen(filename(), "w", stdout)==NULL)
		errexit("split: can't open %s\n", filebuf);
	if(!xflag)
		printf("%s\n", linebuf);
}

ecmp(a, b, count)
char	*a, *b;
{
	register cc = count;
	while(cc--)
		if(*a++ != *b++)	return(0);
	return(1);
}

errexit(s, f)
char *s, *f;
{
	fprintf(stderr, s, f);
	exit(2);
}
