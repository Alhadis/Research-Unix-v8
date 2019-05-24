/* idiff:  interactive diff */

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>

#define	HUGE	10000	/* large number of lines */

char	*progname;
char	*diffout = "/tmp/idiff.XXXXXX";
char	*outfile = "idiff.out";

main(argc, argv)
	int argc;
	char *argv[];
{
	FILE *fin, *fout, *f1, *f2, *efopen();
	char buf[BUFSIZ], *mktemp(), *basename(), *p;
	struct stat stbuf;
	int cleanup();

	progname = argv[0];
	if (argc != 3) {
		fprintf(stderr, "Usage: idiff file1 file2\n");
		exit(1);
	}
	f1 = efopen(argv[1], "r");
	f2 = efopen(argv[2], "r");
	fstat(fileno(f2), &stbuf);
	if (stbuf.st_mode & S_IFDIR) {
		fclose(f2);
		sprintf(buf, "%s/%s", argv[2], basename(argv[1]));
		f2 = efopen(buf, "r");
	}
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, cleanup);
	fout = efopen(outfile, "w");
	mktemp(diffout);
	sprintf(buf,"diff %s %s >%s",argv[1], argv[2], diffout);
	system(buf);
	fin = efopen(diffout, "r");
	idiff(f1, f2, fin, fout);
	unlink(diffout);
	printf("%s output in file %s\n", progname, outfile);
	exit(0);
}

cleanup()
{
	unlink(diffout);
	unlink(outfile);
	exit(1);
}

char *basename(s)	/* find last component of filename */
	char *s;
{
	static char *p;

	for (p = s+strlen(s)-1; p >= s; p--)
		if (*p == '/')
			return p+1;
	return s;
}

idiff(f1, f2, fin, fout)	/* process diffs */
	FILE *f1, *f2, *fin, *fout;
{
	char *tempfile = "idiff.XXXXXX";
	char buf[BUFSIZ], buf2[BUFSIZ], *mktemp();
	FILE *ft, *efopen();
	int cmd, n, from1, to1, from2, to2, nf1, nf2, done;

	mktemp(tempfile);
	nf1 = nf2 = 0;
	done = 0;
	while (!done && fgets(buf, sizeof buf, fin) != NULL) {
		parse(buf, &from1, &to1, &cmd, &from2, &to2);
		n = to1-from1 + to2-from2 + 1; /* #lines from diff */
		if (cmd == 'c')
			n += 2;
		else if (cmd == 'a')
			from1++;
		else if (cmd == 'd')
			from2++;
		printf("%s", buf);
		while (n-- > 0) {
			fgets(buf, sizeof buf, fin);
			printf("%s", buf);
		}
		for(;;) {
			printf("? ");
			fflush(stdout);
			fgets(buf, sizeof buf, stdin);
			switch (buf[0]) {
			case '2':
				to1 = to2 = HUGE;
				done = 1;
			case '>':
				nskip(f1, to1-nf1);
				ncopy(f2, to2-nf2, fout);
				break;
			case '1':
				to1 = to2 = HUGE;
				done = 1;
			case '<':
				nskip(f2, to2-nf2);
				ncopy(f1, to1-nf1, fout);
				break;
			case 'e':
				ncopy(f1, from1-1-nf1, fout);
				nskip(f2, from2-1-nf2);
				ft = efopen(tempfile, "w");
				ncopy(f1, to1+1-from1, ft);
				fprintf(ft, "---\n");
				ncopy(f2, to2+1-from2, ft);
				fclose(ft);
				sprintf(buf2, "ed %s", tempfile);	
				system(buf2);
				ft = efopen(tempfile, "r");
				ncopy(ft, HUGE, fout);
				fclose(ft);
				break;
			case '!':
				system(buf+1);
				printf("!\n");
				break;
			case 'd':
				nskip(f1, to1-nf1);
				nskip(f2, to2-nf2);
				break;
			default:
				printf("< > d e 1 2 !\n");
				continue;
			}
			break;
		}
		nf1 = to1;
		nf2 = to2;
	}
	ncopy(f1, HUGE, fout);	/* can fail on very long files */
	unlink(tempfile);
}

parse(s, pfrom1, pto1, pcmd, pfrom2, pto2)
	char *s;
	int *pcmd, *pfrom1, *pto1, *pfrom2, *pto2;
{
#define a2i(p) while (isdigit(*s)) p = 10*(p) + *s++ - '0'

	*pfrom1 = *pto1 = *pfrom2 = *pto2 = 0;
	a2i(*pfrom1);
	if (*s == ',') {
		s++;
		a2i(*pto1);
	} else
		*pto1 = *pfrom1;
	*pcmd = *s++;
	a2i(*pfrom2);
	if (*s == ',') {
		s++;
		a2i(*pto2);
	} else
		*pto2 = *pfrom2;
}

nskip(fin, n)	/* skip n lines of file fin */
	FILE *fin;
{
	char buf[BUFSIZ];

	while (n-- > 0)
		fgets(buf, sizeof buf, fin);
}

ncopy(fin, n, fout)	/* copy n lines from fin to fout */
	FILE *fin, *fout;
{
	char buf[BUFSIZ];

	while (n-- > 0) {
		if (fgets(buf, sizeof buf, fin) == NULL)
			return;
		fputs(buf, fout);
	}
}

FILE *efopen(file, mode)	/* fopen file, die if can't */
	char *file, *mode;
{
	FILE *fp, *fopen();
	extern char *progname;

	if ((fp = fopen(file, mode)) != NULL)
		return fp;
	fprintf(stderr, "%s: can't open file %s mode %s\n",
		progname, file, mode);
	exit(1);
}
