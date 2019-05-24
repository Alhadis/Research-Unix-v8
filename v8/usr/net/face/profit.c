#include "stdio.h"
#include "signal.h"
struct rec {
	long len;
	struct rec *next;
	char *fname;
	long cnt[1];
} *proFptr = (struct rec *)-1;	/* end of list marker */
profit()
{	int i;
	FILE *fd;
	struct rec *x = proFptr;
	fd = fopen("/etc/net/prof.out", "w");
	while(x != (struct rec *)-1) {
		fprintf(fd, "%s\n", x->fname);
		for(i = 3; i < x->len; i++)
			fprintf(fd, "%d\n", x->cnt[i-3]);
		x = x->next;
	}
	fflush(fd);
	fclose(fd);
	signal(SIGTERM, profit);
}
