static	char *sccsid = "@(#)sh.glob.c 4.2 3/11/81";
#include "sh.h"

/*
 * C Shell
 */

int	globcnt;

char	*globchars =	"`{[*?";

char	*gpath, *gpathp, *lastgpathp;
int	globbed;
bool	noglob;
bool	nonomatch;
char	*entp;
char	**sortbas;

char **
glob(v)
	register char **v;
{
	char agpath[BUFSIZ];
	char *agargv[GAVSIZ];

	gpath = agpath; gpathp = gpath; *gpathp = 0;
	lastgpathp = &gpath[sizeof agpath - 2];
	ginit(agargv); globcnt = 0;
#ifdef GDEBUG
	printf("glob entered: "); blkpr(v); printf("\n");
#endif
	noglob = adrof("noglob") != 0;
	nonomatch = adrof("nonomatch") != 0;
	globcnt = noglob | nonomatch;
	while (*v)
		collect(*v++);
#ifdef GDEBUG
	printf("glob done, globcnt=%d, gflag=%d: ", globcnt, gflag); blkpr(gargv); printf("\n");
#endif
	if (globcnt == 0 && (gflag&1)) {
		blkfree(gargv), gargv = 0;
		return (0);
	} else
		return (gargv = copyblk(gargv));
}

ginit(agargv)
	char **agargv;
{

	agargv[0] = 0; gargv = agargv; sortbas = agargv; gargc = 0;
	gnleft = NCARGS - 4;
}

collect(as)
	register char *as;
{
	register int i;

	if (any('`', as)) {
#ifdef GDEBUG
		printf("doing backp of %s\n", as);
#endif
		dobackp(as, 0);
#ifdef GDEBUG
		printf("backp done, acollect'ing\n");
#endif
		for (i = 0; i < pargc; i++)
			if (noglob)
				Gcat(pargv[i], "");
			else
				acollect(pargv[i]);
		if (pargv)
			blkfree(pargv), pargv = 0;
#ifdef GDEBUG
		printf("acollect done\n");
#endif
	} else if (noglob || eq(as, "{") || eq(as, "{}")) {
		Gcat(as, "");
		sort();
	} else
		acollect(as);
}

acollect(as)
	register char *as;
{
	register int ogargc = gargc;

	gpathp = gpath; *gpathp = 0; globbed = 0;
	expand(as);
	if (gargc == ogargc) {
		if (nonomatch) {
			Gcat(as, "");
			sort();
		}
	} else
		sort();
}

sort()
{
	register char **p1, **p2, *c;
	char **Gvp = &gargv[gargc];

	p1 = sortbas;
	while (p1 < Gvp-1) {
		p2 = p1;
		while (++p2 < Gvp)
			if (strcmp(*p1, *p2) > 0)
				c = *p1, *p1 = *p2, *p2 = c;
		p1++;
	}
	sortbas = Gvp;
}

expand(as)
	char *as;
{
	register char *cs;
	register char *sgpathp, *oldcs;
	struct stat stb;

	sgpathp = gpathp;
	cs = as;
	if (*cs == '~' && gpathp == gpath) {
		addpath('~');
		for (cs++; letter(*cs) || digit(*cs) || *cs == '-';)
			addpath(*cs++);
		if (!*cs || *cs == '/') {
			if (gpathp != gpath + 1) {
				*gpathp = 0;
				if (gethdir(gpath + 1))
					error("Unknown user: %s", gpath + 1);
				strcpy(gpath, gpath + 1);
			} else
				strcpy(gpath, value("home"));
			gpathp = strend(gpath);
		}
	}
	while (!any(*cs, globchars)) {
		if (*cs == 0) {
			if (!globbed)
				Gcat(gpath, "");
			else if (stat(gpath, &stb) >= 0) {
				Gcat(gpath, "");
				globcnt++;
			}
			goto endit;
		}
		addpath(*cs++);
	}
	oldcs = cs;
	while (cs > as && *cs != '/')
		cs--, gpathp--;
	if (*cs == '/')
		cs++, gpathp++;
	*gpathp = 0;
	if (*oldcs == '{') {
		execbrc(cs, NOSTR);
		return;
	}
	matchdir(cs);
endit:
	gpathp = sgpathp;
	*gpathp = 0;
}

matchdir(pattern)
	char *pattern;
{
	struct stat stb;
	struct direct dirbuf[BUFSIZ / sizeof (struct direct)];
	char d_name[DIRSIZ+1];
	register int dirf, cnt;

	dirf = open(gpath, 0);
	if (dirf < 0) {
		if (globbed)
			return;
		goto patherr;
	}
	if (fstat(dirf, &stb) < 0)
		goto patherr;
	if (!isdir(stb)) {
		errno = ENOTDIR;
		goto patherr;
	}
	while ((cnt = read(dirf, (char *) dirbuf, sizeof dirbuf)) >= sizeof dirbuf[0]) {
		register struct direct *ep = dirbuf;

		for (cnt /= sizeof (struct direct); cnt > 0; cnt--, ep++) {
			if (ep->d_ino == 0)
				continue;
			copdent(d_name, ep->d_name);
			if (match(d_name, pattern)) {
				Gcat(gpath, d_name);
				globcnt++;
			}
		}
	}
	close(dirf);
	return;

patherr:
	Perror(gpath);
}

copdent(to, from)
	register char *to, *from;
{
	register int cnt = DIRSIZ;

	do
		*to++ = *from++;
	while (--cnt);
	*to = 0;
}

execbrc(p, s)
	char *p, *s;
{
	char restbuf[BUFSIZ + 2];
	register char *pe, *pm, *pl;
	int brclev = 0;
	char *lm, savec, *sgpathp;

	for (lm = restbuf; *p != '{'; *lm++ = *p++)
		continue;
	for (pe = ++p; *pe; pe++)
	switch (*pe) {

	case '{':
		brclev++;
		continue;

	case '}':
		if (brclev == 0)
			goto pend;
		brclev--;
		continue;

	case '[':
		for (pe++; *pe && *pe != ']'; pe++)
			continue;
		if (!*pe)
			error("Missing ]");
		continue;
	}
pend:
	if (brclev || !*pe)
		error("Missing }");
	for (pl = pm = p; pm <= pe; pm++)
	switch (*pm & (QUOTE|TRIM)) {

	case '{':
		brclev++;
		continue;

	case '}':
		if (brclev) {
			brclev--;
			continue;
		}
		goto doit;

	case ','|QUOTE:
	case ',':
		if (brclev)
			continue;
doit:
		savec = *pm;
		*pm = 0;
		strcpy(lm, pl);
		strcat(restbuf, pe + 1);
		*pm = savec;
		if (s == 0) {
			sgpathp = gpathp;
			expand(restbuf);
			gpathp = sgpathp;
			*gpathp = 0;
		} else if (amatch(s, restbuf))
			return (1);
		sort();
		pl = pm + 1;
		continue;

	case '[':
		for (pm++; *pm && *pm != ']'; pm++)
			continue;
		if (!*pm)
			error("Missing ]");
		continue;
	}
	return (0);
}

match(s, p)
	char *s, *p;
{
	register int c;
	register char *sentp;
	char sglobbed = globbed;

	if (*s == '.' && *p != '.')
		return (0);
	sentp = entp;
	entp = s;
	c = amatch(s, p);
	entp = sentp;
	globbed = sglobbed;
	return (c);
}

amatch(s, p)
	register char *s, *p;
{
	register int scc;
	int ok, lc;
	char *sgpathp;
	struct stat stb;
	int c, cc;

	globbed = 1;
	for (;;) {
		scc = *s++ & TRIM;
		switch (c = *p++) {

		case '{':
			return (execbrc(p - 1, s - 1));

		case '[':
			ok = 0;
			lc = 077777;
			while (cc = *p++) {
				if (cc == ']') {
					if (ok)
						break;
					return (0);
				}
				if (cc == '-') {
					if (lc <= scc && scc <= *p++)
						ok++;
				} else
					if (scc == (lc = cc))
						ok++;
			}
			if (cc == 0)
				error("Missing ]");
			continue;

		case '*':
			if (!*p)
				return (1);
			if (*p == '/') {
				p++;
				goto slash;
			}
			for (s--; *s; s++)
				if (amatch(s, p))
					return (1);
			return (0);

		case 0:
			return (scc == 0);

		default:
			if (c != scc)
				return (0);
			continue;

		case '?':
			if (scc == 0)
				return (0);
			continue;

		case '/':
			if (scc)
				return (0);
slash:
			s = entp;
			sgpathp = gpathp;
			while (*s)
				addpath(*s++);
			addpath('/');
			if (stat(gpath, &stb) == 0 && isdir(stb))
				if (*p == 0) {
					Gcat(gpath, "");
					globcnt++;
				} else
					expand(p);
			gpathp = sgpathp;
			*gpathp = 0;
			return (0);
		}
	}
}

Gmatch(s, p)
	register char *s, *p;
{
	register int scc;
	int ok, lc;
	int c, cc;

	for (;;) {
		scc = *s++ & TRIM;
		switch (c = *p++) {

		case '[':
			ok = 0;
			lc = 077777;
			while (cc = *p++) {
				if (cc == ']') {
					if (ok)
						break;
					return (0);
				}
				if (cc == '-') {
					if (lc <= scc && scc <= *p++)
						ok++;
				} else
					if (scc == (lc = cc))
						ok++;
			}
			if (cc == 0)
				bferr("Missing ]");
			continue;

		case '*':
			if (!*p)
				return (1);
			for (s--; *s; s++)
				if (Gmatch(s, p))
					return (1);
			return (0);

		case 0:
			return (scc == 0);

		default:
			if ((c & TRIM) != scc)
				return (0);
			continue;

		case '?':
			if (scc == 0)
				return (0);
			continue;

		}
	}
}

Gcat(s1, s2)
	register char *s1, *s2;
{

	gnleft -= strlen(s1) + strlen(s2) + 1;
	if (gnleft <= 0 || ++gargc >= GAVSIZ)
		error("Arguments too long");
	gargv[gargc] = 0;
	gargv[gargc - 1] = strspl(s1, s2);
}

addpath(c)
	char c;
{

	if (gpathp >= lastgpathp)
		error("Pathname too long");
	*gpathp++ = c;
	*gpathp = 0;
}

rscan(t, f)
	register char **t;
	int (*f)();
{
	register char *p, c;

	while (p = *t++) {
		if (f == tglob)
			if (*p == '~')
				gflag |= 2;
			else if (eq(p, "{") || eq(p, "{}"))
				continue;
		while (c = *p++)
			(*f)(c);
	}
}

scan(t, f)
	register char **t;
	int (*f)();
{
	register char *p, c;

	while (p = *t++)
		while (c = *p)
			*p++ = (*f)(c);
}

tglob(c)
	register char c;
{

	if (any(c, globchars))
		gflag |= c == '{' ? 2 : 1;
	return (c);
}

trim(c)
	char c;
{

	return (c & TRIM);
}

tback(c)
	char c;
{

	if (c == '`')
		gflag = 1;
}

char *
globone(str)
	register char *str;
{
	char *gv[2];
	register char **gvp;
	register char *cp;

	gv[0] = str;
	gv[1] = 0;
	gflag = 0;
	rscan(gv, tglob);
	if (gflag) {
		gvp = glob(gv);
		if (gvp == 0) {
			setname(str);
			bferr("No match");
		}
		cp = *gvp++;
		if (cp == 0)
			cp = "";
		else if (*gvp) {
			setname(str);
			bferr("Ambiguous");
		} else
			cp = strip(cp);
/*
		if (cp == 0 || *gvp) {
			setname(str);
			bferr(cp ? "Ambiguous" : "No output");
		}
*/
		xfree((char *)gargv); gargv = 0;
	} else {
		scan(gv, trim);
		cp = savestr(gv[0]);
	}
	return (cp);
}

/*
 * Command substitute cp.  If literal, then this is
 * a substitution from a << redirection, and so we should
 * not crunch blanks and tabs, separating words only at newlines.
 */
char **
dobackp(cp, literal)
	char *cp;
	bool literal;
{
	register char *lp, *rp;
	char *ep;
	char word[BUFSIZ];
	char *apargv[GAVSIZ + 2];

	if (pargv) {
		abort();
		blkfree(pargv);
	}
	pargv = apargv;
	pargv[0] = NOSTR;
	pargcp = pargs = word;
	pargc = 0;
	pnleft = BUFSIZ - 4;
	for (;;) {
		for (lp = cp; *lp != '`'; lp++) {
			if (*lp == 0) {
				if (pargcp != pargs)
					pword();
#ifdef GDEBUG
				printf("leaving dobackp\n");
#endif
				return (pargv = copyblk(pargv));
			}
			psave(*lp);
		}
		lp++;
		for (rp = lp; *rp && *rp != '`'; rp++)
			if (*rp == '\\') {
				rp++;
				if (!*rp)
					goto oops;
			}
		if (!*rp)
oops:
			error("Unmatched `");
		ep = savestr(lp);
		ep[rp - lp] = 0;
		backeval(ep, literal);
#ifdef GDEBUG
		printf("back from backeval\n");
#endif
		cp = rp + 1;
	}
}

backeval(cp, literal)
	char *cp;
	bool literal;
{
	int pvec[2];
	int quoted = (literal || (cp[0] & QUOTE)) ? QUOTE : 0;
	char ibuf[BUFSIZ];
	register int icnt = 0, c;
	register char *ip;
	bool hadnl = 0;
	char *fakecom[2];
	struct command faket;

	faket.t_dtyp = TCOM;
	faket.t_dflg = 0;
	faket.t_dlef = 0;
	faket.t_drit = 0;
	faket.t_dspr = 0;
	faket.t_dcom = fakecom;
	fakecom[0] = "` ... `";
	fakecom[1] = 0;
	/*
	 * We do the psave job to temporarily change the current job
	 * so that the following fork is considered a separate job.
	 * This is so that when backquotes are used in a
	 * builtin function that calls glob the "current job" is not corrupted.
	 * We only need one level of pushed jobs as long as we are sure to
	 * fork here.
	 */
	psavejob();
	/*
	 * It would be nicer if we could integrate this redirection more
	 * with the routines in sh.sem.c by doing a fake execute on a builtin
	 * function that was piped out.
	 */
	mypipe(pvec);
	if (pfork(&faket, -1) == 0) {
		struct wordent paraml;
		struct command *t;

		close(pvec[0]);
		dmove(pvec[1], 1);
		dmove(SHDIAG, 2);
		initdesc();
		arginp = cp;
		while (*cp)
			*cp++ &= TRIM;
		lex(&paraml);
		if (err)
			error(err);
		alias(&paraml);
		t = syntax(paraml.next, &paraml, 0);
		if (err)
			error(err);
		if (t)
			t->t_dflg |= FPAR;
		execute(t, -1);
		exitstat();
	}
	xfree(cp);
	close(pvec[1]);
	do {
		int cnt = 0;
		for (;;) {
			if (icnt == 0) {
				ip = ibuf;
				icnt = read(pvec[0], ip, BUFSIZ);
				if (icnt <= 0) {
					c = -1;
					break;
				}
			}
			if (hadnl)
				break;
			--icnt;
			c = (*ip++ & TRIM);
			if (c == 0)
				break;
			if (c == '\n') {
				/*
				 * Continue around the loop one
				 * more time, so that we can eat
				 * the last newline without terminating
				 * this word.
				 */
				hadnl = 1;
				continue;
			}
			if (!quoted && (c == ' ' || c == '\t'))
				break;
			cnt++;
			psave(c | quoted);
		}
		/*
		 * Unless at end-of-file, we will form a new word
		 * here if there were characters in the word, or in
		 * any case when we take text literally.  If
		 * we didn't make empty words here when literal was
		 * set then we would lose blank lines.
		 */
		if (c != -1 && (cnt || literal))
			pword();
		hadnl = 0;
	} while (c >= 0);
#ifdef GDEBUG
	printf("done in backeval, pvec: %d %d\n", pvec[0], pvec[1]);
	printf("also c = %c <%o>\n", c, c);
#endif
	close(pvec[0]);
	pwait();
	prestjob();
}

psave(c)
	char c;
{

	if (--pnleft <= 0)
		error("Word too long");
	*pargcp++ = c;
}

pword()
{

	psave(0);
	if (pargc == GAVSIZ)
		error("Too many words from ``");
	pargv[pargc++] = savestr(pargs);
	pargv[pargc] = NOSTR;
#ifdef GDEBUG
	printf("got word %s\n", pargv[pargc-1]);
#endif
	pargcp = pargs;
	pnleft = BUFSIZ - 4;
}
