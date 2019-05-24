#define	JERQ	No home should be without one
/*%cc p.c pad.o spname.o
 */
#include <stdio.h>
#include "pad.h"
#include <signal.h>
#ifdef	JERQ
#include <sgtty.h>
#include "/usr/jerq/include/jioctl.h"
int	ismpx=0;
#endif
#define	DEF	22	/* 3*DEF == 66, #lines per nroff page */
#define	WIDTH	79	/* some goddam terminals get confused at the margin */
#define	STDIN	(char *)0
int nprintfiles = 0;
int lineno = 0;
int pglen = DEF;
int width = WIDTH;
FILE *tty;
char buf[BUFSIZ];

main(argc, argv)
	char *argv[];
{
#ifdef	JERQ
	struct sgttyb sttybuf;
#endif
	if((tty=fopen("/dev/tty", "r")) == NULL) {
		printf("p: no /dev/tty\n");
		exit(1);
	}
	setbuf(stdout, buf);
#ifdef JERQ
	if(ioctl(1, JWINSIZE, &sttybuf) == 0){
		pglen = sttybuf.sg_ospeed-1;	/* ick */
		if(pglen<0)
			pglen = 5;	/* why not 5? */
		width = sttybuf.sg_ispeed-1;
		if(width<10)
			width=10;
		ismpx=1;
	}
#endif
	while(argc > 1) {
		--argc; argv++;
		if(*argv[0] == '-') {
			pglen = atoi(&argv[0][1]);
			if(pglen < 0)
				pglen = DEF;
		} else
			printfile(argv[0], argc==1);
	}
	if(nprintfiles == 0)
		printfile(STDIN, 1);
	return 0;
}

PAD *
spopen(file, mode)
	register char *file;
	register char *mode;
{
	register FILE *f;
	register char c;
	extern char *spname();

	if((f=fopen(file, mode)) == NULL) {
		file = spname(file);
		if(file != (char *)0){
			printf("\"p %s\"? ", file);
			fflush(stdout);
			c = getc(tty);
			if(c != 'n')
				f=fopen(file, mode);
			while(c != '\n')
				c = getc(tty);
		}
	}
	return(Pfopen(f));
}
int peek;
int cheat;
int col;
get(pad)
	PAD *pad;
{
	register c;
	if(c=peek){
		peek=0;
		if(cheat && (peek=Pgetc(pad))=='\n')
			peek=0; /* don't insert '\n' if it's coming up anyway */
		cheat=0;
	}else
		c=Pgetc(pad);
	return c;
}
put(c)
	register c;
{
	putchar(c);
	if(c=='\t')
		col=(col|7)+1;
	else if(c=='\n')
		col=0;
	else if(c=='\b' && col>0)
		col--;
	else if(c>=' ')
		col++;
	if(col>=width)
		cheat=peek='\n';
}
printfile(file, last)
	register char *file;
{
	register PAD *pad;
	register c;

	nprintfiles++;
	peek = cheat = 0;
	col = 0;
	if(file == STDIN)
		pad=Pfopen(stdin);
	else if((pad=spopen(file, "r")) == NULL) {
		/*
		 * no need to use stderr in p!
		 */
		printf("p: can't open %s\n", file);
		return;
	}
	while((c=get(pad))!=EOF || (!last && newline(pad, 1)=='-')) {
		if(c == '\n' && ++lineno==pglen) {
			if(newline(pad, 0)==EOF)
				goto Return;
		} else
			put(c);
	}
	fflush(stdout);
 Return:
	Pclose(pad);
}
newline(p, eof)
	register PAD *p;
{
	register c, i;

	lineno = 0;
	col = 0;
	fflush(stdout);
  loop:
	switch(getc(tty)){
	case '\n':		/* Continue */
#ifdef	JERQ
		if(ismpx && !eof)
			putchar('\f');
#endif
		return '\n';
	case '!':
		callunix();
		goto loop;
	case 'q':
		waitnl(1);
	case EOF:
		return EOF;
	case '-':
		for(i=1; (c=getc(tty))=='-'; i++)
			;
		ungetc(c, tty);
		peek=cheat=0;
		backpage(p, i);
		waitnl(0);
		return '-';
		/* Fall through */
	default:
		waitnl(eof);
		return '\n';
	}
}

backpage(p, pg)
	register PAD *p;
	register pg;
{
	register i;
	while(pg--)
		for(i=0; i<pglen; i++)
			if(backline(p)==EOF)
				return;
	/* Now at end of new first line; back up to beginning */
	if(backline(p)=='\n')
		(void)Pgetc(p);		/* Eat that newline */
}

backline(p)
	register PAD *p;
{
	register c;
	while((c=Pbackc(p))!='\n')
		if(c==EOF)
			return(EOF);
	return('\n');
}

waitnl(eof){
	do; while(getc(tty) != '\n');
#ifdef	JERQ
	if(ismpx && !eof)
		putchar('\f');
#endif
}

callunix()
{
	int rc, status, unixpid;
	char buf[256];
	register char *p;
	for(p=buf; (*p++=getc(tty))!='\n'; )
		;
	*--p=0;
	if( (unixpid=fork())==0 ) {
		close(0); dup(2);
		execl("/bin/sh", "sh", "-c", buf, 0);
		exit(255);
	}
	else if(unixpid == -1){
		printf("p: can't fork\n");
		return;
	}else{	signal(SIGINT, SIG_IGN); signal(SIGQUIT, SIG_IGN);
		while( (rc = wait(&status)) != unixpid && rc != -1 ) ;
		signal(SIGINT, SIG_DFL); signal(SIGQUIT, SIG_DFL);
		printf("!\n");
		fflush(stdout);
	}
}
