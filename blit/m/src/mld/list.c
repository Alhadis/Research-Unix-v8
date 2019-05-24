#include "list.h"
#define NULL 0
extern char *realloc(), *malloc();
list xmustuse = {0, 0, 0, 50};
list initlist = {0, 0, 0, 50};
list	*mustuse = &xmustuse;
lfree(xlist) list *xlist;
{
	if(xlist->lnext > 0)
		free((char *)xlist->ldata);
}
append(xlist, p) list *xlist; char *p;
{	char **u;
	if(xlist->lnext + 1 >= xlist->lcnt) {
		unsigned int u;
		u = (xlist->lcnt + xlist->lincr)*sizeof(char *);
		if(xlist->lcnt)
			xlist->ldata = (char **)realloc((char *)xlist->ldata, u);
		else xlist->ldata = (char **)malloc(u);
		if(xlist->ldata == NULL) fatal("no list space\n");
		xlist->lcnt += xlist->lincr;
		xlist->lincr *= 2;
	}
	u = xlist->ldata + xlist->lnext++;
	*u = p;
}
eachlist(l, f) list *l; int (*f)();
{	int i;
	for(i = 0; i < l->lnext; i++)
		(*f)(l->ldata[i]);
};
