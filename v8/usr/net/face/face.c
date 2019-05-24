#include "face.h"
#define	SIZE	128
Dir	machine;
Inode	root={
	DIR,
	2,
};
char	facedir[]="/usr/jerq/icon/face48";	/* we chdir to here, to save nameis */
Trie	*filetrie;
struct{
	Inode	*dir;
	char	*local;
	char	*realdir;
}dirtab[]={
	0,	"48x48x1",	"",
	0,	"512x512x8",	"/t0/face/512x512x8/",
	0,	0,		0,
};
faceinit(dir)
	char *dir;
{
	char buf[128];
	register i;
	root.idir=&machine;
	initd(&machine, &root, &root);
	if(chdir(facedir)==-1)
		error("can't chdir to", facedir);
	sprintf(buf, "%s/%s", dir, "machine.tab");
	buildmachine(buf);
	for(i=0; dirtab[i].local; i++)
		attach(dirtab[i].local, dirtab[i].dir=newd(&root), &machine);
	sprintf(buf, "%s/%s", dir, "people.tab");
	buildpeople(buf);
}
buildmachine(tab)
	char *tab;
{
	char left[SIZE], right[SIZE];
	FILE *f=efopen(tab, "r");
	Inode *ip;
	for(;;)
		switch(parse(f, left, right)){
		case -1:
			fclose(f);
			return;
		case 0:
			break;
		case 1:
		default:
			error("syntax error on machine line:", left);
		case 2:
			if((ip=namei(right, &root))==0)
				attach(right, ip=newd(&root), &machine);
			attach(left, ip, &machine);
			break;
		}
}
Inode *
faceinode(s, parent)
	char *s;
	Inode *parent;
{
	register Inode *ip;
	register i;
	if((ip=(Inode *)tlookup(s, filetrie))==0){
		char buf[256];
		ip=newd(parent);
		if(filetrie==0)
			filetrie=tcreate(dupstr(s), (Trie *)ip);
		else
			tinsert(dupstr(s), (Trie *)ip, filetrie);
		for(i=0; dirtab[i].local; i++){
			sprintf(buf, "%s%s", dirtab[i].realdir, s);
			if(access(buf, 4)==0){
				attach(dirtab[i].local, newf(buf), ip->idir);
				if(namei(s, dirtab[i].dir)==0)
					attach(s, newf(buf), dirtab[i].dir->idir);
			}
		}
	}
	return ip;
}
buildpeople(tab)
	char *tab;
{
	char left[SIZE], right[SIZE];
	char where[SIZE], who[SIZE];
	register Inode *ip;
	FILE *f=efopen(tab, "r");
	for(;;)
		switch(parse(f, left, right)){
		case -1:
			fclose(f);
			return;
		case 0:
			break;
		case 1:
		default:
			error("syntax error on people line:", left);
		case 2:
			namesplit(left, where, who);
			if((ip=namei(where, &root))==0)
				attach(where, ip=newd(&root), &machine);
			attach(who, faceinode(right, ip), ip->idir);
			break;
		}
}
namesplit(name, mach, person)
	char *name, *mach, *person;
{
	char *p;
	strcpy(mach, name);
	if((p=strchr(mach, '/'))==0)
		error("expected machine/person; got", name);
	*p++=0;
	strcpy(person, p);
}
parse(f, l, r)
	FILE *f;
	register char *l, *r;
{
	register char *p;
	char buf[SIZE];
	if(fgets(buf, SIZE, f)==0)
		return -1;
	if(buf[0]=='#')
		return 0;
	if((p=strchr(buf, '='))==0){
		copytonl(l, buf);
		return 1;
	}
	*p++=0;
	strcpy(l, buf);
	copytonl(r, p);
	return 2;
}
copytonl(a, b)
	register char *a, *b;
{
	register char *p;
	if(p=strchr(b, '\n'))	/* assignment = */
		*p=0;
	strcpy(a, b);
}
equate(a, b, t)
	register char *a, *b;
	register Trie **t;
{
	register Trie *v;
	if(*t==0)
		*t=tcreate(b, v=(Trie *)dupstr(b));
	else if((v=tlookup(b, *t))==0)
		tinsert(b, v=(Trie *)dupstr(b), *t);
	tinsert(a, v, *t);
}
char *
permalloc(n)	/* knows we'll never free it */
	int n;
{
	static long *ptr;
	static nleft;
	register long *p;
	n=(n+sizeof(long)-1)/sizeof(long);
	if(n>nleft){
		ptr=(long *)emalloc((n+2048)*sizeof(long));
		nleft=n+2048;
	}
	p=ptr;
	ptr+=n;
	nleft-=n;
	return (char *)p;
}
char *
emalloc(n)
	unsigned n;
{
	register char *p=malloc(n);
	if(p==0)
		error("malloc failed", (char *)0);
	return p;
}
char *
erealloc(s, n)
	char *s;
	unsigned n;
{
	register char *p=realloc(s, n);
	if(p==0)
		error("realloc failed", (char *)0);
	return p;
}
FILE *
efopen(s, m)
	char *s, *m;
{
	FILE *f=fopen(s, m);
	if(f==0)
		error("can't open file", s);
	return f;
}
char *
dupstr(s)
	char *s;
{
	char *t=permalloc(strlen(s)+1);
	strcpy(t, s);
	return t;
}
error(s, t)
	char *s, *t;
{
	fprintf(stderr, t? "face: %s %s\n" : "face: %s\n", s, t);
	abort();
}
