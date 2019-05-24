#include "stdio.h"
#include "fserv.h"
#include "sys/neta.h"
extern int host;
/* keep all the perm space here, to redo on file change */

typedef struct {
	unsigned short cuid;
	short cgid;
	unsigned short huid;
	short hgid;
} pe;
typedef struct {
	char *name;	/* eg mh/astro/seki */
	short ix;	/* its index > 0 */
	short cnt;	/* number of entries */
	pe *tab;	/* the entries */
	pe *inv;	/* host->client index */
} ps;
ps *prm;
int nprm, prmlen;
short *ptemp;
int ptnum, ptlen;
long lastlooked;

perminit()
{	FILE *fd;
	int i;
	fd = fopen("/usr/net/people", "r");
	if(fd == NULL) {
		fd = fopen("/etc/net/people", "r");
		if(fd == NULL) {
			perror("people");
			exit(1);
		}
	}
	for(i = 0; i < nprm; i++) {
		if(prm[i].name)
			free(prm[i].name);
		if(prm[i].tab)
			free((char *)prm[i].tab);
		if(prm[i].inv)
			free((char *)prm[i].inv);
	}
	nprm = 0;
	ptnum = 0;
	pparse(fd);
	newhost(0);
	fclose(fd);
	lastlooked = time(0);
	for(i = 0; i < nprm; i++)
		psort(prm + i);
}

permredo()
{	struct stat stb;
	int i;
	if(stat("/usr/net/people", &stb) == -1 || stb.st_mtime <= lastlooked)
		return;
	for(i = 0; i < nprm; i++) {
		free(prm[i].name);
		free(prm[i].tab);
		free(prm[i].inv);
	}
	nprm = 0;
	ptnum = 0;
	perminit();
}

newhost(s)
char *s;
{	ps *p;
	short *x;
	int i;
	if(nprm > 0) {	/* clean up after last one */
		p = prm + nprm - 1;
		p->tab = (pe *)malloc(ptnum * sizeof(short));
		x = (short *)p->tab;
		for(i = 0; i < ptnum; i++)
			*x++ = ptemp[i];
		p->cnt = ptnum / 4;	/* 4 = #shorts / pe */
		p->inv = (pe *)malloc(ptnum * sizeof(short));
		ptnum = 0;
	}
	if(s == 0)
		return;
	if(nprm >= prmlen) {
		if(nprm == 0) {
			prm = (ps *)malloc(5 * sizeof(ps));
			prmlen = 5;
		}
		else {
			prmlen *= 2;
			prm = (ps *)realloc((char *)prm, prmlen * sizeof(ps));
		}
	}
	p = prm + nprm++;
	p->name = malloc(strlen(s) + 1);
	strcpy(p->name, s);
	p->ix = nprm;
}

doclient(uid, gid)
{
	if(ptnum + 1 >= ptlen) {
		if(ptnum == 0) {
			ptemp = (short *)malloc(20 * sizeof(short));
			ptlen = 20;
		}
		else {
			ptlen *= 2;
			ptemp = (short *)realloc((char *)ptemp, ptlen * sizeof(short));
		}
	}
	ptemp[ptnum++] = uid;
	ptemp[ptnum++] = gid;
}

dohost(uid, gid)
{
	doclient(uid, gid);
}

ccmp(a, b)
pe *a, *b;
{
	if(a->cuid != b->cuid)
		return(a->cuid - b->cuid);
	else
		return(a->cgid - b->cgid);
}

hcmp(a, b)
pe *a, *b;
{
	if(a->huid != b->huid)
		return(a->huid - b->huid);
	else
		return(a->hgid - b->hgid);
}
psort(p)
ps *p;
{	int i;
	pe *x, *y;
	qsort((char *)p->tab, p->cnt, sizeof(pe), ccmp);
	for(i = 0; i < p->cnt; i++)
		p->inv[i] = p->tab[i];
	qsort((char *)p->inv, p->cnt, sizeof(pe), hcmp);
}

isfriend(s)
char *s;
{	int i;
	for(i = 0; i < nprm; i++)
		if(strcmp(prm[i].name, s) == 0)
			return(prm[i].ix);
	return(0);
}

pe *
hsearch(h, u, g)
{	pe *x;
	int lo, hi, j;
	h--;		/* change from ix to loc in array */
	debug4("hsearch(%s,%d,%d)", prm[h].name, u, g);
	lo = 0;
	hi = prm[h].cnt;
	x = prm[h].inv + (j = (lo + hi)/2);
	while(j > lo) {
		if(x->huid > u)
			hi = j;
		else
			lo = j;
		j = (lo + hi) / 2;
		x = prm[h].inv + j;
	}
	if(x->huid != u)
		return(0);
	for(; x >= prm[h].inv && x->huid == u; x--)
		if(x->hgid == g)
			return(x);
	if(x->huid != u)
		x++;
	return(x);
}
			

hostuid(s)
struct stat *s;
{	pe *x;
	x = hsearch(host, s->st_uid, s->st_gid);
	if(x)
		return(x->cuid);
	else
		return(-1);
}

hostgid(s)
struct stat *s;
{	pe *x;
	x = hsearch(host, s->st_uid, s->st_gid);
	if(x)
		return(x->cgid);
	else
		return(-1);
}

pe *
csearch(h, u, g)
{	pe *x;
	int lo, hi, j;
	h--;		/* change from ix to loc in array */
	debug4("csearch(%s,%d,%d)", prm[h].name, u, g);
	lo = 0;
	hi = prm[h].cnt;
	x = prm[h].tab + (j = (lo + hi)/2);
	while(j > lo) {
		if(x->cuid > u)
			hi = j;
		else
			lo = j;
		j = (lo + hi) / 2;
		x = prm[h].tab + j;
	}
	if(x->cuid != u)
		return(0);
	for(; x >= prm[h].tab && x->cuid == u; x--)
		if(x->cgid == g)
			return(x);
	if(x->cuid != u)
		x++;
	return(x);
}

myuid(u, g)
{	pe *x;
	x = csearch(host, u, g);
	if(x == 0)
		return(-1);
	else
		return(x->huid);
}

mygid(u, g)
{	pe *x;
	x = csearch(host, u, g);
	if(x == 0)
		return(-1);
	else
		return(x->hgid);
}

isowner(x, sbuf)
struct senda *x;
struct stat *sbuf;
{
	/* local file is owned by user */
	if((sbuf->st_mode & S_IFMT) == S_IFCHR || (sbuf->st_mode & S_IFMT) == S_IFBLK)
		return(0);
	if(sbuf->st_uid != -1 && myuid(x->uid, x->gid) == sbuf->st_uid)
		return(1);
	if(x->uid == 0)		/* cheat, not the whole story */
		return(1);
	return(0);
}
noaccess(x, sbuf, how)
struct senda *x;
struct stat *sbuf;
{	int n;
	n = myuid(x->uid, x->gid);
	if(n == -1 && x->uid == 0) {
		if(how == S_IEXEC)
			goto other;
		if(x->flags == NCREAT && (sbuf->st_mode & S_IFMT) == S_IFDIR)
			return(0);
		if(x->flags == NDEL && (sbuf->st_mode & S_IFMT) == S_IFDIR)
			return(0);
		if(x->flags == NLINK)
			return(0);
	}		
	if(n == -1)
		return(1);
	if((n == sbuf->st_uid) && (sbuf->st_mode & how))
		return(0);
	if((mygid(x->uid, x->gid) == sbuf->st_gid) && (sbuf->st_mode & (how >> 3)))
		return(0);
other:
	if(sbuf->st_mode & (how >> 6))
		return(0);
	return(1);
}
