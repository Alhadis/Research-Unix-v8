/*
 * egrep -- print lines containing (or not containing) a regular expression
 *
 *	status returns:
 *		0 - ok, and some matches
 *		1 - ok, but no matches
 *		2 - some error; matches irrelevant
 */
%token CHAR DOT CCL NCCL OR CAT STAR PLUS QUEST
%left OR
%left CHAR DOT CCL NCCL '('
%left CAT
%left STAR PLUS QUEST

%{
#include <stdio.h>
#include <ctype.h>

#define BLKSIZE 512	/* size of reported disk blocks */
#define MAXLIN 1000
#define MAXPOS 10000
#define NCHARS 128
#define NSTATES 128
#define FINAL -1
#define LEFT '\177'	/* serves as ^ */
#define RIGHT '\n'	/* serves as record separator and as $ */
char gotofn[NSTATES][NCHARS];
int state[NSTATES];
char out[NSTATES];
int line  = 1;
int name[MAXLIN];
int left[MAXLIN];
int right[MAXLIN];
int parent[MAXLIN];
int foll[MAXLIN];
int positions[MAXPOS];
char chars[MAXLIN];
int nxtpos = 0;
int inxtpos;
int nxtchar = 0;
int tmpstat[MAXLIN];
int initstat[MAXLIN];
int istat;
int nstate = 1;
int xstate;
int count;
int icount;
char *input;

char reinit = 0;

long	lnum;
int	bflag;
int	cflag;
int	fflag;
int	lflag;
int	nflag;
int	hflag	= 1;
int	iflag;
int	sflag;
int	vflag;
int	nfile;
long	blkno;
long	tln;
int	nsucc;
int	badbotch;

int	f;
FILE	*expfile;
%}

%%
s:	t
		={ unary(FINAL, $1);
		  line--;
		}
	;
t:	b r
		={ $$ = node(CAT, $1, $2); }
	| OR b r OR
		={ $$ = node(CAT, $2, $3); }
	| OR b r
		={ $$ = node(CAT, $2, $3); }
	| b r OR
		={ $$ = node(CAT, $1, $2); }
	;
b:
		={ $$ = enter(DOT);
		   $$ = unary(STAR, $$); }
	;
r:	CHAR
		={ $$ = enter($1); }
	| DOT
		={ $$ = enter(DOT); }
	| CCL
		={ $$ = cclenter(CCL); }
	| NCCL
		={ $$ = cclenter(NCCL); }
	;

r:	r OR r
		={ $$ = node(OR, $1, $3); }
	| r r %prec CAT
		={ $$ = node(CAT, $1, $2); }
	| r STAR
		={ $$ = unary(STAR, $1); }
	| r PLUS
		={ $$ = unary(PLUS, $1); }
	| r QUEST
		={ $$ = unary(QUEST, $1); }
	| '(' r ')'
		={ $$ = $2; }
	| error 
	;

%%
yyerror(s) {
	fprintf(stderr, "egrep: %s\n", s);
	exit(2);
}

yylex() {
	extern int yylval;
	int cclcnt, x;
	register char c, d;
	switch(c = nextch()) {
		case '^': c = LEFT;
			goto defchar;
		case '$': c = RIGHT;
			goto defchar;
		case '|': return (OR);
		case '*': return (STAR);
		case '+': return (PLUS);
		case '?': return (QUEST);
		case '(': return (c);
		case ')': return (c);
		case '.': return (DOT);
		case '\0': return (0);
		case RIGHT: return (OR);
		case '[': 
			x = CCL;
			cclcnt = 0;
			count = nxtchar++;
			if ((c = nextch()) == '^') {
				x = NCCL;
				c = nextch();
			}
			do {
				if (c == '\0') synerror();
				if (c == '-' && cclcnt > 0 && chars[nxtchar-1] != 0) {
					if ((d = nextch()) != 0) {
						c = chars[nxtchar-1];
						while (c < d) {
							if (nxtchar >= MAXLIN) overflo();
							chars[nxtchar++] = ++c;
							cclcnt++;
						}
						continue;
					}
				}
				if (nxtchar >= MAXLIN) overflo();
				chars[nxtchar++] = c;
				cclcnt++;
			} while ((c = nextch()) != ']');
			chars[count] = cclcnt;
			return (x);
		case '\\':
			if ((c = nextch()) == '\0') synerror();
		defchar:
		default: yylval = c; return (CHAR);
	}
}
nextch() {
	register char c;
	if (fflag) {
		if ((c = getc(expfile)) == EOF) return(0);
	}
	else c = *input++;
	return(iflag? tolower(c): c);
}

synerror() {
	fprintf(stderr, "egrep: syntax error\n");
	exit(2);
}

enter(x) int x; {
	if(line >= MAXLIN) overflo();
	name[line] = x;
	left[line] = 0;
	right[line] = 0;
	return(line++);
}

cclenter(x) int x; {
	register linno;
	linno = enter(x);
	right[linno] = count;
	return (linno);
}

node(x, l, r) {
	if(line >= MAXLIN) overflo();
	name[line] = x;
	left[line] = l;
	right[line] = r;
	parent[l] = line;
	parent[r] = line;
	return(line++);
}

unary(x, d) {
	if(line >= MAXLIN) overflo();
	name[line] = x;
	left[line] = d;
	right[line] = 0;
	parent[d] = line;
	return(line++);
}
overflo() {
	fprintf(stderr, "egrep: regular expression too long\n");
	exit(2);
}

cfoll(v) {
	register i;
	if (left[v] == 0) {
		count = 0;
		for (i=1; i<=line; i++) tmpstat[i] = 0;
		follow(v);
		add(foll, v);
	}
	else if (right[v] == 0) cfoll(left[v]);
	else {
		cfoll(left[v]);
		cfoll(right[v]);
	}
}
cgotofn() {
	register i;
	count = 0;
	inxtpos = nxtpos;
	for (i=3; i<=line; i++) tmpstat[i] = 0;
	if (cstate(line-1)==0) {
		tmpstat[line] = 1;
		count++;
		out[1] = 1;
	}
	for (i=3; i<=line; i++) initstat[i] = tmpstat[i];
	count--;		/*leave out position 1 */
	icount = count;
	tmpstat[1] = 0;
	add(state, 1);
	istat = nxtst(1,LEFT);
}

nxtst(s,c)
char c;
{
	register i, num, k;
	int pos, curpos, number, newpos;
	num = positions[state[s]];
	count = icount;
	for (i=3; i<=line; i++) tmpstat[i] = initstat[i];
	pos = state[s] + 1;
	for (i=0; i<num; i++) {
		curpos = positions[pos];
		if ((k = name[curpos]) >= 0)
			if (
				(k == c)
				| (k == DOT && c != LEFT && c != RIGHT)
				| (k == CCL && member(c, right[curpos], 1))
				| (k == NCCL && member(c, right[curpos], 0) && c != LEFT && c != RIGHT)
			) {
				number = positions[foll[curpos]];
				newpos = foll[curpos] + 1;
				for (k=0; k<number; k++) {
					if (tmpstat[positions[newpos]] != 1) {
						tmpstat[positions[newpos]] = 1;
						count++;
					}
					newpos++;
				}
			}
		pos++;
	}
	if (notin(nstate)) {
		if (++nstate >= NSTATES) {
			for (i=1; i<NSTATES; i++)
				out[i] = 0;
			for (i=1; i<NSTATES; i++)
				for (k=0; k<NCHARS; k++)
					gotofn[i][k] = 0;
			nstate = 1;
			nxtpos = inxtpos;
			reinit = 1;
			add(state, nstate);
			if (tmpstat[line] == 1) out[nstate] = 1;
			return nstate;
		}
		add(state, nstate);
		if (tmpstat[line] == 1) out[nstate] = 1;
		gotofn[s][c] = nstate;
		return nstate;
	}
	else {
		gotofn[s][c] = xstate;
		return xstate;
	}
}


cstate(v) {
	register b;
	if (left[v] == 0) {
		if (tmpstat[v] != 1) {
			tmpstat[v] = 1;
			count++;
		}
		return(1);
	}
	else if (right[v] == 0) {
		if (cstate(left[v]) == 0) return (0);
		else if (name[v] == PLUS) return (1);
		else return (0);
	}
	else if (name[v] == CAT) {
		if (cstate(left[v]) == 0 && cstate(right[v]) == 0) return (0);
		else return (1);
	}
	else { /* name[v] == OR */
		b = cstate(right[v]);
		if (cstate(left[v]) == 0 || b == 0) return (0);
		else return (1);
	}
}


member(symb, set, torf) {
	register i, num, pos;
	num = chars[set];
	pos = set + 1;
	for (i=0; i<num; i++)
		if (symb == chars[pos++]) return (torf);
	return (!torf);
}

notin(n) {
	register i, j, pos;
	for (i=1; i<=n; i++) {
		if (positions[state[i]] == count) {
			pos = state[i] + 1;
			for (j=0; j < count; j++)
				if (tmpstat[positions[pos++]] != 1) goto nxt;
			xstate = i;
			return (0);
		}
		nxt: ;
	}
	return (1);
}

add(array, n) int *array; {
	register i;
	if (nxtpos + count >= MAXPOS) overflo();
	array[n] = nxtpos;
	positions[nxtpos++] = count;
	for (i=3; i <= line; i++) {
		if (tmpstat[i] == 1) {
			positions[nxtpos++] = i;
		}
	}
}

follow(v) int v; {
	int p;
	if (v == line) return;
	p = parent[v];
	switch(name[p]) {
		case STAR:
		case PLUS:	cstate(v);
				follow(p);
				return;

		case OR:
		case QUEST:	follow(p);
				return;

		case CAT:	if (v == left[p]) {
					if (cstate(right[p]) == 0) {
						follow(p);
						return;
					}
				}
				else follow(p);
				return;
		case FINAL:	if (tmpstat[line] != 1) {
					tmpstat[line] = 1;
					count++;
				}
				return;
	}
}


main(argc, argv)
char **argv;
{
	while (--argc > 0 && (++argv)[0][0]=='-')
		switch (argv[0][1]) {

		case 's':
			sflag++;
			continue;

		case 'h':
			hflag = 0;
			continue;

		case 'i':
			iflag++;
			continue;

		case 'b':
			bflag++;
			continue;

		case 'c':
			cflag++;
			continue;

		case 'e':
			argc--;
			argv++;
			goto cut;

		case 'f':
			fflag++;
			continue;

		case 'l':
			lflag++;
			continue;

		case 'n':
			nflag++;
			continue;

		case 'v':
			vflag++;
			continue;

		default:
			fprintf(stderr, "egrep: unknown flag\n");
			continue;
		}
cut:
	if (argc<=0)
		exit(2);
	if (fflag) {
		if ((expfile = fopen(*argv, "r")) == NULL) {
			fprintf(stderr, "egrep: can't open %s\n", *argv);
			exit(2);
		}
	}
	else input = *argv;
	argc--;
	argv++;

	yyparse();

	cfoll(line-1);
	cgotofn();
	nfile = argc;
	if (argc<=0) {
		if (lflag) exit(1);
		execute(0);
	}
	else while (--argc >= 0) {
		execute(*argv);
		argv++;
	}
	exit(badbotch ? 2 : nsucc==0);
}

execute(file)
char *file;
{
	register char *p;
	register cstat;
	register c;
	register t;
	int ccount;
	char buf[2*BUFSIZ];
	char *nlp;
	if (file) {
		if ((f = open(file, 0)) < 0) {
			fprintf(stderr, "egrep: can't open %s\n", file);
			badbotch=1;
			return;
		}
	}
	else f = 0;
	ccount = 0;
	lnum = 1;
	tln = 0;
	p = buf;
	nlp = p;
	if ((ccount = read(f,p,BUFSIZ))<=0) goto done;
	blkno = ccount;
	cstat = istat;
	if (out[cstat]) goto found;
	for (;;) {
		c = *p&0377;
		if(iflag && c>='A' && c<='Z')
			c += 'a'-'A';
		if ((t = gotofn[cstat][c]) == 0)
			cstat = nxtst(cstat,c);
		else
			cstat = t;
		if (out[cstat]) {
		found:	for(;;) {
				if (*p++ == RIGHT) {
					if (vflag == 0) {
				succeed:	nsucc = 1;
						if (cflag) tln++;
						else if (sflag)
							;	/* ugh */
						else if (lflag) {
							printf("%s\n", file);
							close(f);
							return;
						}
						else {
							if (nfile > 1 && hflag) printf("%s:", file);
							if (bflag) printf("%ld:", (blkno-ccount-1)/BLKSIZE);
							if (nflag) printf("%ld:", lnum);
							if (p <= nlp) {
								while (nlp < &buf[2*BUFSIZ]) putchar(*nlp++);
								nlp = buf;
							}
							while (nlp < p) putchar(*nlp++);
						}
					}
					lnum++;
					nlp = p;
					if (reinit == 1) {
						clearg();
					}
					if ((out[(cstat=istat)]) == 0) goto brk2;
				}
				cfound:
				if (--ccount <= 0) {
					if (p <= &buf[BUFSIZ]) {
						if ((ccount = read(f, p, BUFSIZ)) <= 0) goto done;
					}
					else if (p == &buf[2*BUFSIZ]) {
						p = buf;
						if ((ccount = read(f, p, BUFSIZ)) <= 0) goto done;
					}
					else {
						if ((ccount = read(f, p, &buf[2*BUFSIZ]-p)) <= 0) goto done;
					}
					if(nlp>p && nlp<=p+ccount)
						nlp = p+ccount;
					blkno += ccount;
				}
			}
		}
		if (*p++ == RIGHT) {
			if (vflag) goto succeed;
			else {
				lnum++;
				nlp = p;
				if (reinit == 1) {
					clearg();
				}
				if (out[(cstat=istat)]) goto cfound;
			}
		}
		brk2:
		if (--ccount <= 0) {
			if (p <= &buf[BUFSIZ]) {
				if ((ccount = read(f, p, BUFSIZ)) <= 0) break;
			}
			else if (p == &buf[2*BUFSIZ]) {
				p = buf;
				if ((ccount = read(f, p, BUFSIZ)) <= 0) break;
			}
			else {
				if ((ccount = read(f, p, &buf[2*BUFSIZ] - p)) <= 0) break;
			}
			if(nlp>p && nlp<=p+ccount)
				nlp = p+ccount;
			blkno += ccount;
		}
	}
done:	close(f);
	if (cflag) {
		if (nfile > 1)
			printf("%s:", file);
		printf("%ld\n", tln);
	}
}

clearg() {
	register i, k;
			for (i=1; i<NSTATES; i++)
				out[i] = 0;
			for (i=1; i<NSTATES; i++)
				for (k=0; k<NCHARS; k++)
					gotofn[i][k] = 0;
			nstate = 1;
			nxtpos = inxtpos;
			reinit = 1;
	count = 0;
	for (i=3; i<=line; i++) tmpstat[i] = 0;
	if (cstate(line-1)==0) {
		tmpstat[line] = 1;
		count++;
		out[1] = 1;
	}
	for (i=3; i<=line; i++) initstat[i] = tmpstat[i];
	count--;		/*leave out position 1 */
	icount = count;
	tmpstat[1] = 0;
	add(state, 1);
	istat = nxtst(1,LEFT);
}
