#include <stdio.h>
#include <signal.h>
#include "trace.h"
#include "trace.d"

#if 0
#define debug(s1, s2, s3, s4, s5)	printf(s1, s2, s3, s4, s5)
#else
#define debug(s1, s2, s3, s4, s5)	
#endif

 struct TBL       *tbl;
 struct LBT	  *lbt;
 struct MBOX      *mbox;
 struct MNAME     *fullname;

 struct REVPOL **expr;
 struct PROCSTACK **procstack;

 struct VARPARS	  *procpars;
 struct TBLPARS	  *tblpars;

 struct LOCVARS   *tblvars;

 struct TBLPARS   *tablpars;

 struct QUEUE     **starter, **head, **tail;
 struct QUEUE     *s_first, *s_last;

/*
 * reftasks : mapping of logic reftask id to table number
 * processes: mapping of logic process id to table number
 * basics   : initial process table
 * mask     : masks off processes from the state information
 */

 int *state, *reftasks, *processes, *basics;
 int *qsize, *globvars, *inits, *xob;

 /*
  * the state-set of an assertion table is mapped
  * onto the global variables (to simplify state checking)
  * the first global that is an element from the state set
  * is given by integer `abase'
  */

 int abase = -1;		/* base of assertion table state set */
 int ebase = -1;		/* base of error table state set */
 int assertbl = -1;		/* table used for assertion checking */
 int errortbl = -1;		/* table used for assertion checking */

 char mask[MAXPROC];		/* is set before nrprocs is read */

 int QMAX;			/* max qsize per queue  */

 int maxcol = 0;
 int nrtbl = 0;
 int nrqs = 0;
 int nrrefs = 0;
 int nrprocs = 0;
 int nrinit = 0;		/* length initial string */
 int nrvars = 0;
 int nexpr = 0;
 int nrmesgs = 0;
 int msgbase = -1;
 int maxlevel = -1;
 int maxreached = -1;

 char qoverride = 0;
 char noshortcut = 0;		/* disable timeout heuristics */
 char prbyq = 0;		/* controls output format   */
 char blast = 0;		/* very quick and very dirty mode     */
 char qandirty = 0;		/* quick and dirty mode     */
 char lockplus = 0;		/* report both buffer locks and loops    */
 char firstlock = 0;		/* stop at first buffer lock found       */
 char maxxed = 0;		/* bound on search depth    */
 char prefix = 0;		/* print all prefixes too */
 char timedd = 0;		/* verbose mode */
 char completed = 0;		/* not interrupted */
 char ignvars = 0;		/* variable values ignored in state checking */
 char ignques = 0;		/* queue states are ignored idem */
 char sensible = 0;		/* default mode: sensible partial search */
 char muststore = 0;		/* must perform loop check and store state */

 double zapper = (double) 30720;	/* 30 x 1024 */

 long zapped = 0;
 long locksf = 0;
 long loopsf = 0;
 long normf = 0;
 long callno = 0;

 int *effnrstates;	/* effective nr of table states per process */
 int aperiod = 120;

 char * topofmem;
 char * sbrk();

 FILE *mb;
 char *Smalloc();

 extern double iseen, ireseen;

onalarm()
{ struct {
	long u;
	long p;
	long chld_usrt;
	long chld_syst;
  } tim;

  float t;
	times(&tim);
	t = (float) (tim.u  + tim.p)/ (float) 60.0;

	if (++callno%10 == 1)
	{	fprintf(stderr, " seconds  depth states");
		fprintf(stderr, "  zapped terms loops locks  memory\n");
	}
	fprintf(stderr, "%8.2f %6d", t, maxreached);
	fprintf(stderr, " %6g %7ld", iseen+ireseen, zapped);
	fprintf(stderr, " %5ld", normf);
	fprintf(stderr, " %5ld %5ld %7u\n", loopsf, locksf, sbrk(0) - topofmem);
	signal(SIGALRM, onalarm);
	alarm(aperiod);

	return;
}

postlude()
{ struct {
	long u;
	long p;
	long chld_usrt;
	long chld_syst;
  } tim;
  extern double COUNT;

  float u, s;

	fflush(stdout);

	if (!completed)
		fprintf(stderr, "trace: interrupted\n");

	if (timedd)
	{	times(&tim);
		u = (float) tim.u / (float) 60.0;
		s = (float) tim.p / (float) 60.0;

		fprintf(stderr, "\ttime: %.2fs u + ", u);
		fprintf(stderr, "%.2fs sys = %.2fs\n\n", s, u+s);
		fprintf(stderr, "\t%g states, %ld zapped, ", iseen+ireseen, zapped);
		fprintf(stderr, "\t%g edges traversed\n", COUNT);

		fprintf(stderr, "\tsearch depth reached: %d; ", maxreached);
		fprintf(stderr, "memory used: %u\n", sbrk(0) - topofmem );

		fprintf(stderr, "\tfound: %ld loops,", loopsf);
		fprintf(stderr, " %ld locks, and", locksf);
		fprintf(stderr, " %ld terminating executions\n", normf);
	}
	exit(completed == 0);
}

wisdom()
{
	if (sensible)
	{	timedd = prbyq = firstlock = qandirty= 1;
		fprintf(stderr, "default search: ");
		fprintf(stderr, "-vxjfqm %d %d\n", QMAX, maxlevel);
	} else
	{	fprintf(stderr, "%s", (blast) ? "blastsearch, " : "");
		fprintf(stderr, "%s", (!blast && qandirty) ? "quicksearch, " : "");
		fprintf(stderr, "%s", (!qandirty) ? "fullsearch, " : "");
		fprintf(stderr, "depth bound %d\n", maxlevel);
	}
}

main(argc, argv)
	int argc; char **argv;
{ char c;
  int j;
  int i = 1;
  int base = 3;

	for (j = 0; j < MAXPROC; j++)
		mask[j] = 0;

	if (argc > 1 && argv[1][0] == '-')
	{	while ((c = argv[1][i++]) != '\0')
			switch (c) {
			case 'a': prefix = 1;	break;
			case 'b': blast = qandirty = 1;	break;
			case 'c': if (argc >= base)
				  {	sscanf(argv[base-1], "%d", &j);
					base++;
				  } else
				  	usage("missing argument for `c' flag");
				  prbyq = firstlock = lockplus = timedd = 1;
				  switch (j) {
					case 0: blast = qandirty = 1;
						break;
					case 1: maxxed = 3;	/* fall through */
					case 2: qandirty = 1;
						break;
					case 3: maxxed = 2; break; /* 1{ x effnr */
					case 4: maxxed = 3;	   /* 2 x effnr  */
					case 5: break;		   /* 8 x effnr  */
					default: usage("unknown validation class");
				  }
				  break;
			case 'f': prbyq = 1;	break;
			case 'F': prbyq = 2;	break;
			case 'i': ignvars = 1;	break;
			case 'j': firstlock = 1; break;
			case 'k': if (argc >= base)
				  {	sscanf(argv[base-1], "%d", &j);
					zapper = (double) (j * 1024);
					base++;
				  } else
				  	usage("missing argument for `k' flag");
				  break;
			case 'L': if (argc >= base)
				  {	sscanf(argv[base-1], "%d", &j);
					zapper = (double) ((j > 0) ? j : 1);
					base++;
				  } else
				  	usage("missing argument for `k' flag");
				  break;
			case 'l': lockplus = 1;	break;
			case 'm': maxxed = 1;
				  if (argc >= base)
				  {	sscanf(argv[base-1], "%d", &maxlevel);
					base++;
				  } else
				  	usage("missing argument for `m' flag");
				  break;
			case 'n': noshortcut = 1; break;
			case 'q': qoverride = 1;
				  if (argc >= base)
				  {	sscanf(argv[base-1], "%d", &QMAX);
					base++;
				  } else
				  	usage("missing argument for `q' flag");
				  break;
			case 'r': if (argc >= base)
				  {	sscanf(argv[base-1], "%d", &j);
					base++;
				  } else
				  	usage("missing argument for `r' flag");
				  signal(SIGALRM, postlude);
				  aperiod = 0;
				  alarm(j*60);
				  break;
			case 'R': if (argc >= base)
				  {	sscanf(argv[base-1], "%d", &j);
					base++;
				  } else
				  	usage("missing argument for `R' flag");
				  aperiod = j*60;
				  break;
			case 's': setup(); showtables(); exit(0);
				  break;
			case 't': if (argc >= base)
				  {	sscanf(argv[base-1], "%d", &j);
					if (j >= 0 && j < MAXPROC)
						mask[j] = 1;
					else
						usage("illegal table number");
					base++;
				  } else
				  	usage("missing argument for `t' flag");
				  break;
			case 'v': timedd = 1;	break;
			case 'x': qandirty = 1; break;
			case 'y': ignques = 1;	break;
			case 'z': sensible = 1; break;
			default : usage("unknown option");
			}
	} else
		sensible = 1;

	if (sensible && !qoverride)
	{	qoverride = 1;
		QMAX = 2;
	}
	setup();

	init();
	topofmem = sbrk(0);

	if (maxxed != 1)
	{
		if (sensible && maxxed != 0)
			whoops("sorry, cannot combine options 'z' and 'c'");

		for (j = maxlevel = 0; j < nrprocs; j++)
			maxlevel += effnrstates[processes[j]];

		switch (maxxed) {
			case 3: maxlevel *= 2; break;
			case 2: maxlevel = (3*maxlevel)/2; break;
			case 0: if (sensible)
					maxlevel *= 2;
				else
					maxlevel *= 8;	/* avoid pathetic cases */
				break;
			default: whoops("cannot happen - main");
		}
		maxxed = 1;
	}
	inilookup();	/* requires maxlevel to be set first */
	if (aperiod > 0)
	{	signal(SIGALRM, onalarm);
		alarm(aperiod);
	} signal(SIGINT, postlude);

	wisdom();
	muststore = 1;
	FSE(0);
	completed = 1;
	postlude();
}

setup()
{
	if ((mb = fopen("pret.out", "r")) == NULL)
	{	fprintf(stderr, "no file `pret.out'\n");
		exit(1);
	}

	getglobals();
	gettables();
	getexprs();

	fclose(mb);
}

getglobals()
{ register int i, j;
  int a, b, c, x, y, z;

	a = fscanf(mb, "%d reftasks (assert %d/%d)\n", &x, &assertbl, &errortbl);
	if (a != 3)
		badinput("reftasks");
	if (fscanf(mb, "%d processes\n", &y) != 1)
		badinput("processes");
	if (fscanf(mb, "%d queues:\n", &z) != 1)
		badinput("queues");

	debug("%d procs, %d functions, %d queues, assert %d\n", y, x, z, assertbl);

	nrtbl = x + y;
	nrqs = z;
	alloc1(x, y, z);

	getmesnames();

	for (i = 0; i < z; i++)
	{	if (fscanf(mb, "%s\t%d/%d/%d: ", mbox[i].qname, &b, &a, &c) != 4)
			badinput("queue sorts");

		alloc2(i, c, a, b);

		for (j = 0; j < c; j++)
		{	fscanf(mb, "%d,", &b);
			xob[b] = i;
		}

	}

	if (fscanf(mb, "%d inits:\n", &nrinit) != 1)
		badinput("queue inits");

	alloc3(nrinit);
	for (i = 0; i < nrinit; i++)
		fscanf(mb, "%d,", &(inits[i]));

	if (fscanf(mb, "%d g-variables: ", &nrvars) != 1)
		badinput("g-variables");

	alloc4(nrvars);

	for (i = 0; i < nrvars; i++)
		if (fscanf(mb, "%d,", &a) != 1)
			badinput("g-var-inits");
		else
		{	if ((b = determine(a)) == 1 || b == 2)
				badinput("g-var bad init");
			else
				globvars[i] = (short) a;
		}
}

getmesnames()
{ register int i;
	if (fscanf(mb, "%d messages, base %d:\n", &nrmesgs, &msgbase) != 2)
		badinput("messages");

	alloc45(nrmesgs);

	for (i = 0; i < nrmesgs; i++)
		if (fscanf(mb, "%s ", fullname[i].mname) != 1)
			badinput("mesg");
}

gettables()
{ register int i, j;
  int a, b, c, d;
  char name[32];

	processes[0] = -1;
	for (i = 0; i < nrtbl; i++)
	{	if ((j = fscanf(mb, "%s %d:%d/%d:", name, &a, &b, &c)) != 4)
		{	printf("matched %d: %s %d\n", j, name, a);
			badinput("table header");
		}

		debug("table %d: %s (%d) ", i, name, a, 0);
		debug("r/c: %d/%d\n", b, c, 0, 0);

		if ((tbl[i].nrrows = b) == 0)
			badinput("empty table");
		tbl[i].nrcols = c;

		if (strcmp(name, "REF") == 0)
		{	reftasks[a] = i;
			nrrefs++;
		} else
		{	processes[a] = basics[a] = i;
			nrprocs++;
			effnrstates[i] = b;
		}

		if (b > 0)
		{	alloc5(i);
			for (j = 0; j < c; j++)
			{	if (fscanf(mb, "%d(%d),", &b, &d) != 2)
					badinput("column header");

				debug("%d(%d),\n", b, d, 0, 0);

				tbl[i].coltyp[j] = d;
				tbl[i].colmap[j] = b;
				if (d != FCT && d != SPN && d != CND)
					tbl[i].colorg[j] = whichq(b);
				else
					tbl[i].colorg[j] = -1;
			}
			getrows(i);
			getspecials(i);
			getcalls(i);
			getparams(a, i);
			getlocvars(a, i);
}	}	}

getrows(nn)
{ int a, b, c;
  int n = nn;

	for (;;)
	{	if (fscanf(mb, "%d/%d (%d) ", &a, &b, &c) != 3)
			badinput("row");

		if (a == 0 && b == 0 && c == 0)
			break;


		debug("row %d, col %d, size %d:\n", a, b, c, 0);

		if (c == 0)
			continue;

		tbl[n].deadrow[a] = 0;
		getentries(n, a, b, c);
	}
}

getentries(nn, m, p, q)
{ register int i;
  int x, y;
  int n = nn;

		tbl[n].ptr[m][p].nrpils = (short) q;

		alloc6(n, m, p, q);

		for (i = 0; i < q; i++)
		{	if (fscanf(mb, "[%d,%d] ", &x, &y) != 2)
				badinput("table entry");
			tbl[n].ptr[m][p].one[i].transf   = (short) x;
			tbl[n].ptr[m][p].one[i].valtrans = (short) y;

			debug("\t[%d,%d] \n", x, y, 0, 0);
		}
}

getspecials(in)
{ register int i;
  int n, m;
  char stri[64];
	if (fscanf(mb, "ENDSTATES %d: ", &n) != 1)
		badinput("endstates");

	for (i = 0; i < n; i++)
	{	if (fscanf(mb, "%d,", &m) != 1 || m < 0 || m >= tbl[in].nrrows)
			badinput("endstate");
		tbl[in].endrow[m] = 1;
	}
	if ((m = fscanf(mb, "%s %d: ", stri, &n)) != 2 ||
		strcmp(stri, "BADSTATES") != 0)
	{	printf("read %s %d\n", stri, n);
		badinput("badstates");
	}

	for (i = 0; i < n; i++)
	{	if (fscanf(mb, "%d,", &m) != 1 || m < 0 || m >= tbl[in].nrrows)
			badinput("badstate");
		tbl[in].badrow[m] = 1;
	}

	if ((m = fscanf(mb, "%s %d: ", stri, &n)) != 2 ||
		strcmp(stri, "LABSTATES") != 0)
	{	printf("read %s %d\n", stri, n);
		badinput("labstates");
	}

	for (i = 0; i < n; i++)
	{	if (fscanf(mb, "%d,", &m) != 1 || m < 0 || m >= tbl[in].nrrows)
			badinput("labstate");
		tbl[in].labrow[m] = 1;
	}
}

getparams(pr, tb)
{ int  n, m;
  char stri[64];
	if (fscanf(mb, "%s %d/%d", stri, &n, &m) != 3)
	{	printf("read %s %d/%d\n", stri, n, m);
		badinput("parameters");
	}

	tblpars[tb].nrms = (short) n;		/* required sizes  */
	tblpars[tb].nrvs = (short) m;

	if (processes[pr] == tb)
		alloc8(pr, 2*n, 2*m);		/* avalaible sizes */

	debug("longest callist proc/ref %d(%d) (m/v): %d/%d\n", pr, tb, n, m);
}

getlocvars(p, tb)
{ register int i;
  int n, m;
	if (fscanf(mb, "%d l-variables: ", &n) != 1)
		badinput("l-variables");

	if (processes[p] != tb)
	{	tblvars[tb].nrlvars = (short) n;	/* required size */
		if (n > 0)
			tblvars[tb].lvarvals = (short *)
				Smalloc(n * sizeof(short));
		for (i = 0; i < n; i++)
			if (fscanf(mb, "%d,", &m) != 1)
				badinput("l-vars");
			else
				tblvars[tb].lvarvals[i] = (short) m;
	} else
	{	tablpars[p].nrlvars = (short) (2*n);	/* available size */
		procpars[p].nrlvars = (short) n;	/* actually used  */
		if (n > 0)
			procpars[p].lvarvals = (short *)
				Emalloc((2*n) * sizeof(short));
		for (i = 0; i < n; i++)
			if (fscanf(mb, "%d,", &m) != 1)
				badinput("lvar-inits");
			else
				procpars[p].lvarvals[i] = (short) m;
	}
}

getcalls(in)
{ register int i, j;
  int k, n, m, a, b, N;
  char stri[64];
	if (fscanf(mb, "%s %d", stri, &n) != 2)		/* FCTS */
	{	printf("read %s %d\n", stri, n);
		badinput("function calls");
	}
	alloc9(in, n);
	for (i = 0, N = n; i < N; i++)
	{	if (fscanf(mb, "%d-%d/%d: ", &n, &m, &k) != 3)
			badinput("fct call");

		alloc10(in, i, n, m, k);

		effnrstates[in] += tbl[reftasks[n]].nrrows;

		for (j = m+k, a = b = 0; j > 0; j--)
		{	if (fscanf(mb, "%d/%d", &n, &m) != 2)
				badinput("fct call entry");
			if (n == 0)
				tbl[in].calls[i].ms[a++] = (short) m;
			else
				tbl[in].calls[i].vs[b++] = (short) m;
	}	}
}

getexprs()
{ int i, j, a, b, c;
  char name[32];

	if (fscanf(mb, "%s %d\n", name, &nexpr) != 2)
		badinput("nexpr");
	if (strcmp(name, "EXPR") != 0)
		badinput("expressions");

	expr = (struct REVPOL **)
		Smalloc(nexpr * sizeof(struct REVPOL *));

	for (i = 0; i < nexpr; i++)
	{	fscanf(mb, "%d: ", &a);

		expr[i] = (struct REVPOL *)
			Smalloc(a * sizeof(struct REVPOL));

		for (j = 0; j < a; j++)
		{	fscanf(mb, "%d/%d: %d\n", &b, &c);
			expr[i][j].toktyp = b;
			expr[i][j].tokval = c;
	}	}
}

showtables()
{ register int i, j;
  int k, n, m;
  char table[2*MAXPROC][64];

	for (i = 0; i < nrprocs; i++)
	{	j = processes[i];
		sprintf(table[j], "process %d", i);
	}
	for (i = 0; i < nrrefs; i++)
	{	j = reftasks[i];
		sprintf(table[j], "reftask %d", i);
	}

	for (i = 0; i < nrtbl; i++)
	{
		printf("\n%s (table %d):\n   \t", table[i], i);
		for (k = 0; k < tbl[i].nrcols; k++)
		{	switch(tbl[i].coltyp[k]) {
				case INP:  printf("I:"); break;
				case DFL:  printf("D:"); break;
				case TMO:  printf("T:"); break;
				case OUTP: printf("O:"); break;
				case SPN:  printf("S:"); break;
				case CND:  printf("C:"); break;
				case FCT:  printf("F:"); break;
			}
			printf("%d/%d\t", tbl[i].colmap[k], tbl[i].colorg[k]);
		}

		for (j = 0; j < tbl[i].nrrows; j++)
		{	printf("\n%3d\t", j);
			for (k = 0; k < tbl[i].nrcols; k++)
			{	if (tbl[i].ptr[j][k].nrpils > 0)
				{	n = (int) tbl[i].ptr[j][k].one[0].transf;
					m = (int) tbl[i].ptr[j][k].one[0].valtrans;
					printf("%d(%d)", n, m);
				}
				if (tbl[i].ptr[j][k].nrpils > 1)
					putchar('+');
				putchar('\t');
			}
	}	}
	putchar('\n');
}

whichq(n)
{ int i;
	if (n < 0 || n >= MANY)
		return -1;

	if ((i = xob[n]) == -1)
		badinput("mbox sort incomplete");

	return i;
}

init()
{ register int i, m;
	state = (int *)
		Smalloc(nrprocs * sizeof(int));
	qsize = (int *)
		Smalloc(nrqs * sizeof(int));
	head = (struct QUEUE **)
		Smalloc(nrqs * sizeof(struct QUEUE *));
	tail = (struct QUEUE **)
		Smalloc(nrqs * sizeof(struct QUEUE *));
	starter = (struct QUEUE **)
		Smalloc(nrqs * sizeof(struct QUEUE *));


	for (i = 0; i < nrqs; i++)
	{	starter[i] = (struct QUEUE *)
			Smalloc(sizeof(struct QUEUE));
		starter[i]->next = NULL;
		qsize[i] = 0;
		head[i] = tail[i] = starter[i];
	}

	decode();
	initable();	/* do not change the order of inits */
	iniqtable();
	inihash();

	for (i = 0; i < nrprocs; i++)
	{	state[i] = 0;
		procstack[i] = NULL;

		lbt[i].mapcol = (int *)
			Smalloc(maxcol * sizeof(int));
		lbt[i].orgcol = (int *)
			Smalloc(maxcol * sizeof(int));
		fiddler(i);
	}

	s_last = NULL;

	for (i = 0; i < nrinit; i++)
	{	m = inits[i];
		if (send(m, whichq(m), NONE, NONE) == 0)
			badinput("qsize too small for initial string...");
	}
	if (assertbl != NONE)
	{	assertbl = reftasks[assertbl];

		abase = nrvars;
		nrvars += tbl[assertbl].nrrows;

		if (abase > 0)
			globvars = (int *)
				Realloc(globvars, nrvars * sizeof(int));
		else
			globvars = (int *)
				Smalloc(nrvars * sizeof(int));

		globvars[abase] = 1;
		for (i = abase+1; i < nrvars; i++)
			globvars[i] = 0;
	}
	if (errortbl != NONE)
	{	errortbl = reftasks[errortbl];

		ebase = nrvars;
		nrvars += tbl[errortbl].nrrows;

		if (ebase > 0)
			globvars = (int *)
				Realloc(globvars, nrvars * sizeof(int));
		else
			globvars = (int *)
				Smalloc(nrvars * sizeof(int));

		globvars[ebase] = 1;
		for (i = ebase+1; i < nrvars; i++)
			globvars[i] = 0;
	}
}

decode()
{ int i, j, m;

	for (i = 0; i < nrvars; i++)
	{	m = convert(globvars[i], NONE);
		globvars[i] = (short) m;
	}
	for (i = 0; i < nrprocs; i++)
	for (j = 0; j < procpars[i].nrlvars; j++)
	{	m = convert(procpars[i].lvarvals[j], i);
		procpars[i].lvarvals[j] = (short) m;
	}
}
