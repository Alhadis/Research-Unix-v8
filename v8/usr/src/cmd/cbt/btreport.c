#include "stdio.h"
#include "cbt.h"
#include "sys/types.h"
#include "sys/stat.h"
extern bfile *bopen();

long ndcnt[MXHT + 1];
long frcnt, reccnt, reclen;
bfile *bt;

main(argc, argv)
char **argv;
{	int i, j;
	for(i = 1; i < argc; i++) {
		doarg(argv[i]);
		for(j = 0; j <= MXHT; j++)
			ndcnt[j] = 0;
		frcnt = reccnt = reclen = 0;
		if(i+1 < argc)
			putchar('\n');
	}
	exit(0);
}

doarg(s)
char *s;
{	struct stat statbuf;
	long x;
	int i;
	bt = bopen(s, 0);
	if(bt == NULL) {
		i = strlen(s);
		if(s[i-2] == '.') {
			s[i-2] = 0;
			if(s[i-1] == 'F')
				return;
			if(s[i-1] == 'T')
				bt = bopen(s, 0);
		}
		if(bt == NULL) {
			perror(s);
			return;
		}
	}
	fstat(bt->tfd, &statbuf);
	printf("%s.T %ld bytes", s, statbuf.st_size);
	if(bt->dfd > 0 && fstat(bt->dfd, &statbuf) == 0)
		printf(", %s.F %ld bytes", s, statbuf.st_size);
	putchar('\n');
	donode(0);
	for(x = i = 0; i <= MXHT; i++)
		x += ndcnt[i] * NDSZ;
	printf("%ld bytes used in tree\n", x);
	for(i = 0; i <= bt->height; i++)
		printf(" %ld nodes at level %d", ndcnt[i], i);
	printf("\n%ld bytes free\n", frcnt);
	printf("%ld records totalling %ld bytes\n", reccnt, reclen);
	bclose(bt);
}

/* this routine believes the tree is well-formed */
donode(n)
{	char buf[NDSZ];
	hdr *b = (hdr *)buf;
	int i;
	(void) lseek(bt->tfd, (long)NDSZ * n, 0);
	(void) read(bt->tfd, buf, NDSZ);
	ndcnt[b->hlev]++;
	frcnt += nfree(b);
	if(b->hlev)
		for(i = 0; i <= b->kcnt; i++)
			donode(*ndadr(b, i));
	else
		for(i = 0; i < b->kcnt; i++) {
			reccnt++;
			if(!(b->htype & INDEX))
				reclen += lfadr(b, i)->llen;
		}
}
static struct D { struct D *a; char *b;} VER = {&VER,"\n81/4/26:btreport.c\n"};
/*1010101001010110*/
