#include "stdio.h"
#include "ctype.h"
typedef unsigned long ul;
typedef struct {
	char *fname;
	int len;	/* how many counts have been seen */
	int quot;	/* how much has been allocated for cnt */
	unsigned long *cnt;
} stab;
stab *tab;
int ntab, ltab;
unsigned long N, B, L, V;	/* for summary */
double A, U;
FILE *fd, *sfd, *cfd;
extern FILE *popen();
char fname[512] = "/";	/* not checked for overflow */
char buf[256];
char curfunc[256];
extern char *malloc();
char flg[128];
unsigned long val;

main(argc, argv)
char **argv;
{	int i, j;
	for(i = 1; i < argc; i++)
		if(argv[i][0] != '-')
			fprintf(stderr, "unknown arg %s\n", argv[i]);
		else for(j = 1; argv[i][j]; j++)
			switch(argv[i][j]) {
			default:
				fprintf(stderr, "unk option %c\n", argv[i][j]);
				break;
			case 'b':	/* each bb */
				flg['b']++;
				break;
			case 'a':	/* intermediate everything */
				flg['a']++;
				break;
			case 'i':	/* count machine instrs */
				flg['i']++;
				break;
			case 'f':	/* counts by function */
				flg['f']++;
				flg['s']++;
				break;
			case 'p':	/* print, in addition */
				flg['p']++;
				break;
			case 's':	/* bb summary */
				flg['s']++;
				break;
			case 'c':	/* compress prof.out */
				flg['c']++;
				break;
			}
	if(argc <= 1)
		flg['p']++;
	if((fd = fopen("prof.out", "r")) == 0) {
		perror("prof.out");
		exit(1);
	}
	readall();
	fclose(fd);
	if(flg['f']) {
		for(i = 0; i < ntab; i++)
			fsum(tab + i);
	}
	if(flg['s']) {
		for(i = 0; i < ntab; i++)
			summary(tab + i);
		if(ntab > 1)
			printf("%.0fie %ui %uin %.0fbbe %ubb %ubbne total\n",
				A, N, B, U, L, V);
	}
	if(flg['c']) {
		fd = fopen("prof.out", "w");
		if(fd == 0) {
			perror("rewriting prof.out");
			exit(1);
		}
		for(i = 0; i < ntab; i++) {
			fprintf(fd, "%s\n", tab[i].fname);
			for(j = 0; j < tab[i].len; j++)
				fprintf(fd, "%u\n", tab[i].cnt[j]);
		}
		fclose(fd);
	}
	if(flg['p'] || flg['i'] || flg['a'] || flg['b'])
		giantprint();
	exit(0);
}

readall()
{	int c, i, index;
	stab *curtab = 0;
sawnl:
	if((c = getc(fd)) == EOF)
		return;
	if(c == '\n')
		goto sawnl;
	if(c == '/') {
		fscanf(fd, "%s", fname+1);
		for(i = 0; i < ntab; i++)
			if(strcmp(fname, tab[i].fname) == 0)
				break;
		if(i >= ntab) {	/* new file */
			if(ltab == 0) {
				tab = (stab *)malloc(20 * sizeof(stab));
				ltab = 20;
			}
			else if(ntab >= ltab)
				tab = (stab *)realloc((char *)tab,
					(ltab += 20) * sizeof(stab));
			tab[ntab].fname = malloc(sizeof(fname) + 1);
			strcpy(tab[ntab].fname, fname);
			tab[ntab].len = tab[ntab].quot = 0;
			ntab++;
		}
		curtab = tab + i;
		index = 0;
	}
	else if(c < '0' || c > '9') {
		fprintf(stderr, "prof.out has weird format\n");
		abort();
	}
	else {
		ungetc(c, fd);
		fscanf(fd, "%d", &val);
		if(curtab->len <= index) {
			if(curtab->quot == 0) {
				curtab->cnt = (ul *)malloc(100*sizeof(long));
				curtab->quot = 100;
				for(i = 0; i < 100; i++)
					curtab->cnt[i] = 0;
			}
			else if(curtab->len >= curtab->quot) {
				curtab->cnt = (ul *)realloc(curtab->cnt,
					(curtab->quot += 200) * sizeof(long));
				for(i = curtab->quot-200; i < curtab->quot; i++)
					curtab->cnt[i] = 0;
			}
			curtab->len++;
		}
		curtab->cnt[index++] += val;
	}
	goto sawnl;
}

summary(x)
stab *x;
{	unsigned long i, v, n, b;
	double a, u;
	for(i = u = v = 0; i < x->len; i++)
		if(x->cnt[i])
			u += x->cnt[i];
		else
			v++;
	n = strlen(x->fname);
	strcpy(buf, x->fname);
	strcpy(buf + n - 2, ".sL");
	if((fd = fopen(buf, "r")) == 0) {
		perror(buf);
		printf("%s %u bbs %.0f execs %u untouched\n",
			x->fname, x->len, u, v);
		return;
	}
	for(n = a = b = 0; ;) {
		(void) fgets(buf, sizeof(buf), fd);
		if(feof(fd))
			break;
		if(hascolon(buf))
			continue;
		n++;
		i = atoi(buf)/4 - 3;
		if(x->cnt[i])
			a += x->cnt[i];
		else
			b++;
	}
	printf("%.0fie %ui %uine %.0fbbe %ubb %ubbne %s\n", a, n, b, u, x->len,
		v, x->fname);
	fclose(fd);
	A += a; N += n; B += b; U += u; L += x->len; V += v;
}

fsum(x)
stab *x;
{	unsigned i, v, n, b, cnt;
	double a, u;
	char *p;
	strcpy(buf, x->fname);
	n = strlen(x->fname);
	strcpy(buf + n - 2, ".sL");
	if((fd = fopen(buf, "r")) == 0) {
		perror(buf);
		return;
	}
	curfunc[0] = 0;
	for(cnt = v = n = b = a = u = 0;;) {
		(void) fgets(buf, sizeof(buf), fd);
		if(feof(fd))
			break;
		if(hascolon(buf)) {
			/* 24 foo.c: 456	or
			 * 24 foo.c: _funcjunk */
			for(p = buf; *p != ':'; p++)
				;
			while(isspace(*++p))
				;
			if(isdigit(*p))
				continue;
			if(curfunc[0] != 0)
				printf("%.0fie %dcalls %ui %uine %s\n",
					a, v, n, b, curfunc);
			for(i = 0; *p && *p != '\n'; i++)
				curfunc[i] = *p++;
			curfunc[i] = 0;
			a = n = b = 0;
			i = atoi(buf)/4 - 3;
			v = x->cnt[i];
			continue;
		}
		n++;
		i = atoi(buf)/4 - 3;
		if(x->cnt[i])
			a += x->cnt[i];
		else
			b++;
	}
	if(n > 0)
		printf("%.0fie %dcalls %ui %uine %s\n", a, v, n, b, curfunc);
	fclose(fd);
}

giantprint()
{	int i, n;
	for(i = 0; i < ntab; i++) {
	/* the fname file is the source, there should be a corresponding
	 * .sL file for correlation between basic blocks and source.
	 * If fname is a .s file, the .sL file is all there is */
		n = strlen(tab[i].fname);
		if(strcmp(".s", tab[i].fname + n - 2) == 0) {
			sfile(tab + i);
			continue;
		}
		sfd = fopen(tab[i].fname, "r");
		if(sfd == 0 && !flg['a']) {
			perror(tab[i].fname);
			return;
		}
		strcpy(buf, tab[i].fname);
		strcpy(buf + n - 1, "sL");
		if((cfd = fopen(buf, "r")) == 0) {
			fprintf(stderr, "no intermediate listing file ");
			perror(buf);
			fclose(sfd);
			return;
		}
		sprintf(buf, "pr -h %s", tab[i].fname);
		if(!flg['a'])
			fd = popen(buf, "w");
		if(flg['b'])
			xlistit(tab + i);
		else
			listit(tab + i);
		if(fd)
			pclose(fd);
		if(sfd)
			fclose(sfd);
		fclose(cfd);
	}
}

sfile(x)
stab *x;
{	int i;
	char *p;
	strcpy(buf, x->fname);
	strcat(buf, "L");
	sfd = fopen(buf, "r");
	if(sfd == 0) {
		perror(buf);
		return;
	}
	sprintf(buf, "pr -h %s", x->fname);
	fd = popen(buf, "w");
	if(fd == 0) {
		perror(buf);
		fclose(sfd);
		return;
	}
	for(;;) {
		(void) fgets(buf, sizeof(buf), sfd);
		if(feof(sfd)) {
			pclose(fd);
			fclose(sfd);
			return;
		}
		for(i = 0, p = buf; *p >= '0' && *p <= '9'; p++)
			i = 10 * i + *p - '0';
		i = i/4 - 3;
		fprintf(fd, "%u\t%s", x->cnt[i], p);
	}
}

char *
stripnum(s)
char *s;
{
	while(*s && (isdigit(*s) || isspace(*s)))
		s++;
	return(s);
}

listit(x)
stab *x;
{	int bb, lnum, done, i;
	unsigned long count;
	char *p;
	count = done = 0;
	for(;;) {
		(void) fgets(buf, sizeof(buf), cfd);
		if(feof(cfd))
			break;
		for(p = buf; *p && *p != ':' ; p++)
			;
		if(*p++ != ':') {
			count += i = x->cnt[atoi(buf)/4 - 3];
			if(flg['a'])
				printf("%d\t%s", i, stripnum(buf));
			continue;
		}
		else if(flg['a']) {
			printf("%s", stripnum(buf));
			continue;
		}
		lnum = atoi(p);	/* atoi("_...") == 0 */
		i = atoi(buf);
		for(; done < lnum - 1; done++) {
			(void) fgets(buf, sizeof(buf), sfd);
			if(flg['i'])
				fprintf(fd, "%ui\t", count);
			if(flg['p'])
				fprintf(fd, "%u\t", x->cnt[bb]);
			fprintf(fd, "%s", buf);
			count = 0;
		}
		bb = i/4 - 3;
	}
	if(flg['a'])
		return;
	(void) fgets(buf, sizeof(buf), sfd);
	if(feof(sfd))
		strcpy(buf, "unexpected eof\n");
	if(flg['i'])
		fprintf(fd, "%ui\t", count);
	if(flg['p'])
		fprintf(fd, "%u\t", x->cnt[bb]);
	fprintf(fd, "%s", buf);
}

xlistit(x)
stab *x;
{	int bb, lnum, i, j;
	char *p;
	bb = 0;
loop:
	if(feof(cfd))
		return;
	(void) fgets(buf, sizeof(buf), cfd);
	for(p = buf; *p && *p != ':'; p++)
		;
	if(*p++ != ':')
		goto loop;
	lnum = atoi(p);		/* hmm */
	if(lnum <= 0)
		goto loop;
	i = atoi(buf)/4 - 3;
	(void) fgets(buf, sizeof(buf), sfd);
	buf[strlen(buf) - 1] = 0;
	fprintf(fd, "%s", buf);
	if(bb > i)
		fprintf(fd, " %d:%u", i, x->cnt[i]);
	for(j = bb; j <= i; j++)
		fprintf(fd, " %d:%u", j, x->cnt[j]);
	putc('\n', fd);
	bb = j;
	goto loop;
}

hascolon(s)
register char *s;
{
	for(; *s && *s != ':'; s++)
		;
	if(*s == ':')
		return(1);
	else
		return(0);
}
