#include <sys/types.h>
#include <sys/dir.h>
/*
 * char *spname(name)
 *	char name[];
 *
 * returns pointer to correctly spelled name,
 * or 0 if no reasonable name is found;
 * uses a static buffer to store correct name,
 * so copy it if you want to call the routine again.
 */
char *
spname(name)
	register char *name;
{
	register char *p, *q, *new;
	register d, nd, dir;
	static char newname[80], guess[DIRSIZ+1], best[DIRSIZ+1];
	static struct{
		ino_t ino;
		char name[DIRSIZ+1];
	} nbuf;

	new = newname;
	nbuf.name[DIRSIZ] = '\0';
	for(;;){
		while(*name == '/')
			*new++ = *name++;
		*new = '\0';
		if(*name == '\0')
			return(newname);
		p = guess;
		while(*name!='/' && *name!='\0'){
			if(p != guess+DIRSIZ)
				*p++ = *name;
			name++;
		}
		*p = '\0';
		if((dir=open(newname,0)) < 0)
			return((char *)0);
		d = 3;
		while(read(dir, &nbuf, sizeof (struct direct)) == sizeof (struct direct))
			if(nbuf.ino){
				nd=SPdist(nbuf.name, guess);
				if(nd<=d && nd!=3) {	/* <= to avoid "." */
					p = best;
					q = nbuf.name;
					do; while(*p++ = *q++);
					d = nd;
					if(d == 0)
						break;
				}
			}
		close(dir);
		if(d == 3)
			return(0);
		p = best;
		do; while(*new++ = *p++);
		--new;
	}
}
/*
 * very rough spelling metric
 * 0 if the strings are identical
 * 1 if two chars are interchanged
 * 2 if one char wrong, added or deleted
 * 3 otherwise
 */
SPdist(s, t)
	register char *s, *t;
{
	while(*s++ == *t)
		if(*t++ == '\0')
			return(0);
	if(*--s){
		if(*t){
			if(s[1] && t[1] && *s==t[1] && *t==s[1] && SPeq(s+2,t+2))
				return(1);
			if(SPeq(s+1, t+1))
				return(2);
		}
		if(SPeq(s+1, t))
			return(2);
	}
	if(*t && SPeq(s, t+1))
		return(2);
	return(3);
}
SPeq(s, t)
	register char *s, *t;
{
	while(*s++ == *t)
		if(*t++ == '\0')
			return(1);
	return(0);
}
