static	char sccsid[] = "@(#)cc.c 4.1 10/1/80";
/*
 * cc - front end for C compiler
 */
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <dir.h>

char	*cpp = "/lib/cpp";
char	*ccom = "/usr/blit/lib/ccom";
char	*as = "/usr/blit/bin/mas";
char	*c2 = "/usr/blit/lib/mc2";
char	*ld = "/usr/blit/bin/mld";
char	*crt0 = "/usr/blit/lib/notsolow.o";

char	tmp0[30];		/* big enough for /tmp/ctm%05.5d */
char	*tmp1, *tmp2, *tmp3, *tmp4, *tmp5;
char	*outfile;
char	*savestr(), *strspl(), *setsuf();
int	idexit();
char	**av, **clist, **llist, **plist;
int	cflag, eflag, gflag, pflag, sflag, wflag, Rflag, exflag, proflag, oflag;
int	jflag, mflag=1;	/* Used for jerq. Rob Pike (read comment as you will) */
	/* Note: default is to compile for mpx(1) */
int	exfail;
char	*chpass;
char	*npassname;
extern	int	optind;
extern	int	opterr;
extern	char	*optarg;
extern	int	optopt;

int	nc, nl, np, nxo, na;

#define	cunlink(x)	if (x) unlink(x)

main(argc, argv)
	char **argv;
{
	char *t;
	char *assource;
	int i, j, c;

	/* ld currently adds upto 5 args; 10 is room to spare */
	/* I upped it anyway -- rob */
	av = (char **)calloc(argc+20, sizeof (char **));
	clist = (char **)calloc(argc+20, sizeof (char **));
	llist = (char **)calloc(argc+20, sizeof (char **));
	plist = (char **)calloc(argc+20, sizeof (char **));
	opterr = 0;
	while (optind<argc) switch (c = getopt(argc, argv, "jmSo:ROXPgwEPcD:I:U:C:t:B:l:")) {
	case 'S':
		sflag++;
		cflag++;
		continue;
	case 'j':
		if(mflag==0)
			error("only use one of -j and -m", "");
		else{
			jflag++;
			mflag=0;
			crt0 = "/usr/blit/lib/l.o";
		}
		continue;
	case 'm':
		if(jflag)
			error("only use one of -j and -m", "");
		else{
			crt0="/usr/blit/lib/crt0.o";
			mflag=0;
		}
		continue;
	case 'l':
		llist[nl++] = strspl("-l", optarg);
		continue;
	case 'o':
		outfile = optarg;
		switch (getsuf(outfile)) {

		case 'c':
		case 'o':
			error("-o would overwrite %s", outfile);
			exit(8);
		}
		continue;
	case 'R':
		Rflag++;
		continue;
	case 'X':
		ccom="/usr/scj/mcc/comp";
		continue;
	case 'O':
		oflag++;
		continue;
	case 'p':
		proflag++;
		continue;
	case 'g':
		gflag++;
		continue;
	case 'w':
		wflag++;
		continue;
	case 'E':
		exflag++;
		cflag++;
		continue;
	case 'P':
		pflag++;
	case 'c':
		cflag++;
		continue;
	case 'D':
	case 'I':
	case 'U':
	case 'C':
		plist[np] = strspl("-X", optarg);
		plist[np++][1] = c;
		continue;
	case 't':
		if (chpass)
			error("-t overwrites earlier option", 0);
		chpass = optarg;
		if (chpass[0]==0)
			chpass = "012p";
		continue;
	case 'B':
		if (npassname)
			error("-B overwrites earlier option", 0);
		npassname = optarg;
		if (npassname[0]==0)
			npassname = "/usr/scj/mcc";
		continue;
	case '?':
		t = strspl("-", "x");
		t[1] = optopt;
		llist[nl++] = t;
		continue;

	case EOF:
		t = argv[optind];
		optind++;
		c = getsuf(t);
		if (c=='c' || c=='s' || exflag) {
			clist[nc++] = t;
			t = setsuf(t, 'o');
		}
		if (nodup(llist, t)) {
			llist[nl++] = t;
			if (getsuf(t)=='o')
				nxo++;
		}
	}
	if (npassname && chpass ==0)
		chpass = "012p";
	if (chpass && npassname==0)
		npassname = "/usr/new";
	if (chpass)
	for (t=chpass; *t; t++) {
		switch (*t) {

		case '0':
			ccom = strspl(npassname, "ccom");
			continue;
		case '2':
			fprintf(stderr, "mcc: no optimizer pass in mcc\n");
			continue;
		case 'p':
			cpp = strspl(npassname, "cpp");
			continue;
		}
	}
	if (proflag)
		crt0 = "/lib/mcrt0.o";
	if (nc==0)
		goto nocom;
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, idexit);
	if (signal(SIGTERM, SIG_IGN) != SIG_IGN)
		signal(SIGTERM, idexit);
	if (pflag==0)
		sprintf(tmp0, "/tmp/ctm%05.5d", getpid());
	tmp1 = strspl(tmp0, "1");
	tmp2 = strspl(tmp0, "2");
	tmp3 = strspl(tmp0, "3");
	if (pflag==0)
		tmp4 = strspl(tmp0, "4");
	if (oflag)
		tmp5 = strspl(tmp0, "5");
	for (i=0; i<nc; i++) {
		if (nc > 1) {
			printf("%s:\n", clist[i]);
			fflush(stdout);
		}
		if (getsuf(clist[i]) == 's') {
			assource = clist[i];
			goto assemble;
		} else
			assource = tmp3;
		if (pflag)
			tmp4 = setsuf(clist[i], 'i');
		na=0;
		av[na++] = "cpp";
		av[na++] = "-Uvax";
		av[na++] = "-Dmc68000";
		if(mflag)	/* rob -- define MPX */
			av[na++] = "-DMPX";
		for (j = 0; j < np; j++)
			av[na++] = plist[j];
		av[na++] = "-I/usr/blit/include";
		av[na++] = clist[i];
		av[na++] = exflag ? "-" : tmp4;
		av[na++] = 0;
		switch(callsys(cpp, av)){
		case 0:
			break;
#define	CLASS 27
		case CLASS:
			if(callsys("/lib/cpre", av)==0)
				break;
			/* fall through */
		default:
			exfail++;
			eflag++;
		}
		if (pflag || exfail) {
			cflag++;
			continue;
		}
		if (sflag)
			assource = tmp3 = setsuf(clist[i], 's');
		av[0] = "mccom";
		if(gflag){
			av[0] = "ccom"; av[2] = tmp4; av[3] = oflag?tmp5:tmp3;
			na = 4;
			av[1] = "-g";
		}else{
			av[0] = "ccom";
			av[1] = tmp4; av[2] = oflag?tmp5:tmp3; na = 3;
		}
		if (proflag)
			av[na++] = "-XP";
		if (wflag)
			av[na++] = "-w";
		av[na] = 0;
		if (callsys(ccom, av)) {
			cflag++;
			eflag++;
			continue;
		}
		if (oflag) {
			av[0] = "mc2"; av[1] = tmp5; av[2] = tmp3; av[3] = 0;
			if (callsys(c2, av)) {
				unlink(tmp3);
				tmp3 = assource = tmp5;
			} else
				unlink(tmp5);
		}
		if (sflag)
			continue;
	assemble:
		cunlink(tmp1); cunlink(tmp2); cunlink(tmp4);
		av[0] = "mas"; av[1] = "-o"; av[2] = setsuf(clist[i], 'o');
		na = 3;
		if (Rflag)
			av[na++] = "-R";
		av[na++] = assource;
		av[na] = 0;
		if (callsys(as, av) > 1) {
			cflag++;
			eflag++;
			continue;
		}
	}
nocom:
	if (cflag==0 && nl!=0) {
		i = 0;
		av[0] = "ld"; na=1;
		if(!mflag){
			av[na++] = "-b";
			av[na++] = "256";
		}
		av[na++] = crt0;
		if (outfile) {
			av[na++] = "-o";
			av[na++] = outfile;
		}
		if(mflag){
			av[na++] = "-R";
			av[na++] = "-M";
			av[na++]= "-d";
		}
		while (i < nl)
			av[na++] = llist[i++];
		if(jflag)
			av[na++] = "/usr/blit/lib/libsys.a";
		if(mflag)
			av[na++] = "/usr/blit/lib/libmj.a";
		av[na++] = "/usr/blit/lib/libj.a";
		av[na++] = "/usr/blit/lib/libc.a";
		av[na++] = 0;
		eflag |= callsys(ld, av);
		if (nc==1 && nxo==1 && eflag==0)
			unlink(setsuf(clist[0], 'o'));
	}
	dexit();
}

idexit()
{

	eflag = 100;
	dexit();
}

dexit()
{

	if (!pflag) {
		cunlink(tmp1);
		cunlink(tmp2);
		if (sflag==0)
			cunlink(tmp3);
		cunlink(tmp4);
		cunlink(tmp5);
	}
	exit(eflag);
}

error(s, x)
	char *s, *x;
{
	FILE *diag = exflag ? stderr : stdout;

	fprintf(diag, "mcc: ");
	fprintf(diag, s, x);
	putc('\n', diag);
	exfail++;
	cflag++;
	eflag++;
}

getsuf(as)
char as[];
{
	register int c;
	register char *s;
	register int t;

	s = as;
	c = 0;
	while (t = *s++)
		if (t=='/')
			c = 0;
		else
			c++;
	s -= 3;
	if (c <= DIRSIZ && c > 2 && *s++ == '.')
		return (*s);
	return (0);
}

char *
setsuf(as, ch)
	char *as;
{
	register char *s, *s1;

	s = s1 = savestr(as);
	while (*s)
		if (*s++ == '/')
			s1 = s;
	s[-1] = ch;
	return (s1);
}

callsys(f, v)
	char *f, **v;
{
	int t, status;

	t = fork();
	if (t == -1) {
		printf("No more processes\n");
		return (100);
	}
	if (t == 0) {
		execv(f, v);
		printf("Can't find %s\n", f);
		fflush(stdout);
		_exit(100);
	}
	while (t != wait(&status))
		;
	if ((t=(status&0377)) != 0 && t!=14) {
		if (t!=2) {
			printf("Fatal error in %s\n", f);
			eflag = 8;
		}
		dexit();
	}
	return ((status>>8) & 0377);
}

nodup(l, os)
	char **l, *os;
{
	register char *t, *s;
	register int c;

	s = os;
	if (getsuf(s) != 'o')
		return (1);
	while (t = *l++) {
		while (c = *s++)
			if (c != *t++)
				break;
		if (*t==0 && c==0)
			return (0);
		s = os;
	}
	return (1);
}

#define	NSAVETAB	1024
char	*savetab;
int	saveleft;

char *
savestr(cp)
	register char *cp;
{
	register int len;

	len = strlen(cp) + 1;
	if (len > saveleft) {
		saveleft = NSAVETAB;
		if (len > saveleft)
			saveleft = len;
		savetab = (char *)malloc(saveleft);
		if (savetab == 0) {
			fprintf(stderr, "ran out of memory (savestr)\n");
			exit(1);
		}
	}
	strncpy(savetab, cp, len);
	cp = savetab;
	savetab += len;
	saveleft -= len;
	return (cp);
}

char *
strspl(left, right)
	char *left, *right;
{
	char buf[BUFSIZ];

	strcpy(buf, left);
	strcat(buf, right);
	return (savestr(buf));
}
