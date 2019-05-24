typedef struct {
	char **ldata;	/* every pointer can be coerced to (char *) */
	short lnext, lcnt, lincr;
} list;
extern list *mustuse, initlist;
