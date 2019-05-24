/*
 * iostat
 */
#include <nlist.h>
#include <sys/dk.h>
#include "/usr/sys/h/param.h"
/* this is a botch, fix sys/buf.h */
#define KERNEL
#include "/usr/sys/h/buf.h"

struct nlist nl[] = {
	{ "_dk_busy" },
#define	X_DK_BUSY	0
	{ "_dk_time" },
#define	X_DK_TIME	1
	{ "_dk_xfer" },
#define	X_DK_XFER	2
	{ "_dk_wds" },
#define	X_DK_WDS	3
	{ "_tk_nin" },
#define	X_TK_NIN	4
	{ "_tk_nout" },
#define	X_TK_NOUT	5
	{ "_dk_seek" },
#define	X_DK_SEEK	6
	{ "_cp_time" },
#define	X_CP_TIME	7
	{ "_dk_mspw" },
#define	X_DK_MSPW	8
	{ "_io_info" },
#define X_DK_INFO	9
	{ "_io_wait" },
#define X_IO_WAIT	10
	{ 0 },
};
struct
{
	int	dk_busy;
	long	cp_time[CPUSTATES];
	long	dk_time[DK_NDRIVE];
	long	dk_wds[DK_NDRIVE];
	long	dk_seek[DK_NDRIVE];
	long	dk_xfer[DK_NDRIVE];
	float	dk_mspw[DK_NDRIVE];
	struct	{
		int nbuf;
		long nread, nreada, ncache, nwrite;
		long whyread[W_LAST+1], whycache[W_LAST+1];
	} io_info;
	long	io_wait;
	long	tk_nin;
	long	tk_nout;
} s, s1;

int	mf;
double	etime;

main(argc, argv)
char *argv[];
{
	extern char *ctime();
	register  i;
	int iter;
	double f1, f2;
	long t;
	int tohdr = 1;

	nlist("/vmunix", nl);
	if(nl[X_DK_BUSY].n_type == 0) {
		printf("dk_busy not found in /vmunix namelist\n");
		exit(1);
	}
	mf = open("/dev/kmem", 0);
	if(mf < 0) {
		printf("cannot open /dev/kmem\n");
		exit(1);
	}
	iter = 0;
	while (argc>1&&argv[1][0]=='-') {
		argc--;
		argv++;
	}
	lseek(mf, (long)nl[X_DK_MSPW].n_value, 0);
	read(mf, s.dk_mspw, sizeof s.dk_mspw);
	if(argc > 2)
		iter = atoi(argv[2]);
loop:
	if (--tohdr == 0) {
		for (i = 0; i < DK_NDRIVE; i++)
			if (s.dk_mspw[i] != 0.0)
				printf("           D%d ", i);
		printf("         CPU\n");
		for (i = 0; i < DK_NDRIVE; i++)
			if (s.dk_mspw[i] != 0.0)
				printf(" sps tps msps ");
		printf(" us ni sy id wt\trd\trda\tch\twrt\n");
		tohdr = 9;
	}
	lseek(mf, (long)nl[X_DK_BUSY].n_value, 0);
 	read(mf, &s.dk_busy, sizeof s.dk_busy);
 	lseek(mf, (long)nl[X_DK_TIME].n_value, 0);
 	read(mf, s.dk_time, sizeof s.dk_time);
 	lseek(mf, (long)nl[X_DK_XFER].n_value, 0);
 	read(mf, s.dk_xfer, sizeof s.dk_xfer);
 	lseek(mf, (long)nl[X_DK_WDS].n_value, 0);
 	read(mf, s.dk_wds, sizeof s.dk_wds);
 	lseek(mf, (long)nl[X_TK_NIN].n_value, 0);
 	read(mf, &s.tk_nin, sizeof s.tk_nin);
 	lseek(mf, (long)nl[X_TK_NOUT].n_value, 0);
 	read(mf, &s.tk_nout, sizeof s.tk_nout);
	lseek(mf, (long)nl[X_DK_SEEK].n_value, 0);
	read(mf, s.dk_seek, sizeof s.dk_seek);
	lseek(mf, (long)nl[X_CP_TIME].n_value, 0);
	read(mf, s.cp_time, sizeof s.cp_time);
	lseek(mf, (long)nl[X_DK_MSPW].n_value, 0);
	read(mf, s.dk_mspw, sizeof s.dk_mspw);
	lseek(mf, (long)nl[X_DK_INFO].n_value, 0);
	read(mf, (char *)&s.io_info, sizeof s.io_info);
	lseek(mf, (long)nl[X_IO_WAIT].n_value, 0);
	read(mf, (char *)&s.io_wait, sizeof s.io_wait);
#define sv(x)	t = s.x; s.x -= s1.x; s1.x = t;
	for (i = 0; i < DK_NDRIVE; i++) {
		sv(dk_xfer[i]); sv(dk_seek[i]); sv(dk_wds[i]); sv(dk_time[i]);
	}
	sv(tk_nin);
	sv(tk_nout);
	sv(io_info.nread);
	sv(io_info.nreada);
	sv(io_info.ncache);
	sv(io_info.nwrite);
	sv(io_wait);
	for(i = 0; i <= W_LAST; i++) {
		sv(io_info.whyread[i]);
		sv(io_info.whycache[i]);
	}
	etime = 0;
	for(i=0; i<CPUSTATES; i++) {
		sv(cp_time[i]);
		etime += s.cp_time[i];
	}
	if (etime == 0.0)
		etime = 1.0;
	etime /= 60.0;
	for (i=0; i<DK_NDRIVE; i++)
		if (s.dk_mspw[i] != 0.0)
			stats(i);
	for (i=0; i<CPUSTATES; i++)
		stat1(i);
	printf("%3.0f", s.io_wait*100./60./etime);
	printf("\t%d\t%d\t%d\t%d", s.io_info.nread, s.io_info.nreada,
		s.io_info.ncache, s.io_info.nwrite);
	printf("\n");
#define	W_FREE	1
#define W_INODE	2
#define W_INDIR 3
#define W_SUPER 4
#define W_DATA	5
#define W_EXEC	6
#define W_BADDR 7
#define W_DIREC	8
#define W_LAST	8
#define xpr(n,i) printf("%ld,%ld:%s ",s.io_info.whyread[i], s.io_info.whycache[i],n);
	xpr("f", 1); xpr("ino", 2); xpr("ind", 3); xpr("s", 4);
	xpr("d", 5); xpr("x", 6); xpr("v", 7); xpr("dir", 8);
	printf("\n");
contin:
	--iter;
	if(iter)
	if(argc > 1) {
		sleep(atoi(argv[1]));
		goto loop;
	}
}

stats(dn)
{
	register i;
	double atime, words, xtime, itime;

	if (s.dk_mspw[dn] == 0.0) {
		printf("%4.0f%4.0f%5.1f ", 0.0, 0.0, 0.0);
		return;
	}
	atime = s.dk_time[dn];
	atime /= 60.0;
	words = s.dk_wds[dn]*32.0;	/* number of words transferred */
	xtime = s.dk_mspw[dn]*words;	/* transfer time */
	itime = atime - xtime;		/* time not transferring */
/*
	printf("\ndn %d, words %8.2f, atime %6.2f, xtime %6.2f, itime %6.2f\n",
	    dn, words, atime, xtime, itime);
*/
	if (xtime < 0)
		itime += xtime, xtime = 0;
	if (itime < 0)
		xtime += itime, itime = 0;
	printf("%4.0f", s.dk_seek[dn]/etime);
	printf("%4.0f", s.dk_xfer[dn]/etime);
	printf("%5.1f ",
	    s.dk_seek[dn] ? itime*1000./s.dk_seek[dn] : 0.0);
/*
	printf("%4.1f",
	    s.dk_xfer[dn] ? xtime*1000./s.dk_xfer[dn] : 0.0);
*/
}

stat1(o)
{
	register i;
	double time;

	time = 0;
	for(i=0; i<CPUSTATES; i++)
		time += s.cp_time[i];
	if (time == 0.0)
		time = 1.0;
	printf("%3.0f", 100*s.cp_time[o]/time);
}
