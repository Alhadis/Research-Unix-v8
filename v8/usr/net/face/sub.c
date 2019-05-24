#include "fserv.h"
#include "errno.h"
#include "neta.h"

extern char *realloc();
/* temporary static alloc of devtab */

struct tdev devtab[NDEV];
struct stat rootstat;
int ndev, dev;
extern int alive;
char *cmdnames[] = {"0?", "stat", "wrt", "read", "free", "trunc", "updat",
	"get", "nami", "put", "10?"};
char *buf, *nbuf;
int buflen, nbuflen;

#define NF	100
/* temporary static alloc of netftab */
netf netftab[NF];
int nnetf;

#define MASK	0x0

ourdev(n)
{	int i;
	for(i = 0; i < ndev; i++)
		if(devtab[i].his == n)
			return(devtab[i].ours);
	return(-1);
}

hisdev(n)
{	int i;
	for(i = 0; i < ndev; i++)
		if(devtab[i].ours == n)
			return(devtab[i].his);
	return(-1);
}

newdev(n)
{	struct tdev *p;
	p = devtab + ndev++;
	p->ours = n;
	p->his = dev++;
	debug1("\tnewdev %d %d", p->ours, p->his);
}
netf *
gettag(n)
long n;
{	int i;
	for(i = 0; i < nnetf; i++)
		if(netftab[i].tag == n)
			return(netftab + i);
	errno = ENOENT;
	return(0);
}

netf *
getnetf(d, i)	/* either root of netfs, or in the table */
{	int j;
	netf *p;

	if(i == 0) {
		errno = ENOENT;
		return(0);
	}
	for(j = 0; j < nnetf; j++)
		if(netftab[j].ino == i && netftab[j].dev == d)
			return(netftab + j);
	debug2("\tgetnetf(%d,%d)", d, i);
	for(p = netftab, j = 0; j < nnetf; j++, p++) {
		debug2("\t%d %d %d %s", p->tag, p->dev, p->ino, p->name?p->name:"");
	}
	return(0);
}

netf *
newnetf(s)
char *s;
{	int i;
	netf *p;
	struct stat stb;

	if(statbyname(s, &stb) < 0)
		return(0);
	if(p = getnetf(hisdev(stb.st_dev), stb.st_ino)) {
		p->statb = stb;
		return(p);
	}
	for(i = 0; i < nnetf; i++)
		if(netftab[i].ino == 0)
			goto found;
	i = nnetf++;
	if(nnetf > NF) {
		errno = ENOMEM;
		return(0);
	}
found:
	alive++;
	p = netftab + i;
	p->name = malloc(strlen(s) + 1);
	strcpy(p->name, s);
	p->statb = stb;
	p->dev = hisdev(p->statb.st_dev);
	if(p->dev == -1) {
		newdev(p->statb.st_dev);
		p->dev = hisdev(p->statb.st_dev);
	}
	p->ino = p->statb.st_ino;
	p->tag = (p->dev << 16) ^ p->ino ^ MASK;
	debug2("\tnew %d %d %s", p->dev, p->ino, p->name);
	debug2("\tnew 0%o %d %d", p->statb.st_mode, p->statb.st_uid, p->statb.st_gid);
	debug2("\tnew %d", nnetf);
	return(p);
}

clrnetf(n)
{	netf *p;
	p = gettag(n);
	if(p == 0) {
		debug("\tclr net got 0 (x%x)", n);
/*
		dumpstate();
		leave(8);	/* shut it down */
		return;
	}
	if(p->ino == 2 && p->dev == devtab[0].his)
		return;		/* hold on to the root */
	if(p->tag != n) {
		debug("\tclr weird %d %d", n, p->tag);
		errno = EIO;
		dumpstate();
		return;
	}
	debug1("\tclear %d %s", p->tag, p->name);
	p->tag = p->dev = p->ino = 0;
	if(p->name)
		free(p->name);
	p->name = 0;
	alive--;
}

isroot(p)
netf *p;
{
	debug2("\tisroot(dev=%d ino=%d), ourde=%d", p->dev, p->ino, ourdev(p->dev));
	return(p->ino == rootstat.st_ino && ourdev(p->dev) == rootstat.st_dev);
}

prcmd(x)
struct senda *x;
{
	debug1("got %s %d %d %d", cmdnames[x->cmd], major(x->dev), minor(x->dev), x->ino);
}

getbuf(n)
{
	if(n < buflen)
		return;
	if(buflen == 0)
		buf = malloc(buflen = n);
	else
		buf = realloc(buf, buflen = n);
	if(buf)
		return;
	debug("getbuf failed");
	leave(6);
}

getnbuf(n)
long n;
{
	if(n < nbuflen)
		return;
	if(nbuflen == 0)
		nbuf = malloc(nbuflen = n);
	else
		nbuf = realloc(nbuf, nbuflen = n);
	if(nbuf)
		return;
	debug("nbuf failed");
	leave(7);
}
/* the current name is a full path name, buf contains another component */
fixnbuf(name, count, buf, flag)
char *name, *buf;
{	int i, n;
	for(i = 0; i < count && buf[i]; i++)
		;
	count = i;
	n = strlen(name);
	getnbuf(n + count + 2);
	strcpy(nbuf, name);
	if(count > 2 || flag == NLINK || flag == NDEL) {
ok:
		nbuf[n++] = '/';
		for(i = 0; i < count; i++)
			nbuf[n++] = *buf++;
		nbuf[n] = 0;
		return;
	}
	if(count == 0)
		return;
	if(count == 1)
		if(buf[0] == '.')
			return;	/* do we know that name is a directory? */
		else
			goto ok;
	if(buf[0] != '.' || buf[1] != '.')
		goto ok;
	while(n > 0 && nbuf[n] != '/')
		n--;
	nbuf[n] = 0;
	if(nbuf[0] != '/')
		strcpy(nbuf, "/..");
}

