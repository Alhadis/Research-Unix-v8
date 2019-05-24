#include <stdio.h>
#include <nlist.h>
#include <core.h>
#include <sys/param.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <ctype.h>
#include <sgtty.h>


struct nlist nl[] ={
	{"_intrtime"},
	{"_cp_time"},
	{"_avenrun"},
	{ "" },
};

struct t {
	long	cp_time[5];
} ocp, ncp;
double runqueue, oldrunqueue;
long	oin, nin, otime, ntime;

/*	"user",
	"nice",
	"sys",
	"queue",
	"idle",
*/

char *sys = "/unix";
char *core = "/dev/kmem";
char *newmail();
int mem;
char vec[]="01234\n";

main(argc, argv)
	char *argv[];
{
	register i;
	long l;
	int init=1;
	int clicks=0;
	int clocks=0;
	int ticks=5;
	struct sgttyb sttybuf;

	if(argc>1){
		ticks= -atoi(argv[1]);
		if(ticks<2)
			ticks=2;
	}
	nlist(sys, nl);

	mem = open(core, 0);
	if (mem<0) {
		printf("can't open %s\n", core);
		exit(1);
	}
	mailinit();
	if(!boot("/usr/dud/mbin/vismon.m"))
		exit(1);
	ioctl(0, TIOCGETP, &sttybuf);
	sttybuf.sg_flags|=RAW;
	sttybuf.sg_flags&=~ECHO;
	ioctl(0, TIOCSETP, &sttybuf);
	dotime();
	doload();
	lseek(mem, (long)nl[1].n_value, 0);
	read(mem, &ocp, sizeof(ocp));
	for (;;) {
		sleep(ticks);
		lseek(mem, (long)nl[1].n_value, 0);
		read(mem, &ncp, sizeof(ncp));
		l = ncp.cp_time[3];
		ncp.cp_time[3] = ncp.cp_time[4];
		ncp.cp_time[4] = l;
		for (i=0; i<5; i++) {
			vec[i] = ncp.cp_time[i] - ocp.cp_time[i];
			ocp.cp_time[i] = ncp.cp_time[i];
		}
		if(init)
			init=0;
		else
			write(1, vec, 6);
		if(clicks++ >= 60/ticks){
			dotime();
			doload();	/* must be immediately after dotime() */
			clicks = 0;
		}
		if(clocks++ >= 15/ticks){
			char *p;
			if(p=newmail())
				sendmail(p);
			clocks = 0;
		}
	}
}
dotime(){
	char *p, *ctime();
	long l;
	time(&l);
	p = ctime(&l);
	p[16] = 'T';
	write(1, &p[11], 6);
}
doload(){
	char buf[16];
	double fabs();
	lseek(mem, (long)nl[2].n_value, 0);
	read(mem, &runqueue, sizeof(runqueue));
	sprintf(buf, " %.2f %c%.2f\n", runqueue, "-+"[runqueue>oldrunqueue],
			fabs(runqueue-oldrunqueue));
	write(1, buf, strlen(buf));
	oldrunqueue = runqueue;
}
boot(s)
	char *s;
{
	if(system("/usr/dud/bin/32ld", "32ld", s))
		return(0);
	return(1);
}
system(s, t, u)
char *s, *t, *u;
{
	int status, pid, l;

	if ((pid = fork()) == 0) {
		execl(s, t, u, 0);
		_exit(127);
	}
	while ((l = wait(&status)) != pid && l != -1)
		;
	if (l == -1)
		status = -1;
	return(status);
}
/*
 * Stuff for mail
 */
#include <sys/types.h>
#include <sys/stat.h>

char mailfile[100], line[200];
long lastsize;

mailinit(){
	char *getlogin(), *getenv();
	char *n, *p, *index();
	struct stat buf;
	n=getenv("MAIL");
	if(n==0){
		n=getlogin();
		if(n==0){
			printf("vismon: can't determine login name\n");
			exit(1);
		}
		sprintf(mailfile, "/usr/spool/mail/%s", n);
	}else
		strcpy(mailfile, n);
	n = mailfile;
	while ((p = index(n,'/')) != NULL)
		n = ++p;
	if (n != mailfile) {
		*(--n) = '\0';
		chdir(mailfile);
		strcpy(mailfile,++n);
	}
	lastsize = (stat(mailfile, &buf) < 0)? 0 : buf.st_size;
}

char *
newmail(){
	struct stat buf;
	FILE *f;
	char *n, *p, *q, *index();
	if(stat(mailfile, &buf)<0)
		return 0;
	if(buf.st_size<=lastsize){
		lastsize = buf.st_size;
		return 0;
	}
	f = fopen(mailfile, "r");
	fseek (f, lastsize, 0);
	lastsize = buf.st_size;
	while (fgets(line, sizeof line, f) != NULL)
		if (strncmp(line, "From ", 5) == 0) {
			for(n=line+5; *n!=' ' && *n && *n!='\n'; n++)
				;
			*n=0;
			p=line+5;
			while (q=index(p,'!'))
				p=q+1;
			fclose(f);
			return p;
		}
	fclose(f);
	return "somebody?\n";

}

#define ICONPATH "/usr/jerq/icon/face/"
char iconfile[100]=ICONPATH;

sendmail(p)
	char *p;
{
	FILE *icon;
	struct stat statbuf;
	char buf[6], hexbuf[4];
	int i;
	buf[5] = 'M';
	write(1, buf, 6);
	strncpy(iconfile+strlen(ICONPATH), p, 100-strlen(ICONPATH));
	if (stat(iconfile, &statbuf)<0)
		strncpy(iconfile+strlen(ICONPATH), "unknown", 100-strlen(ICONPATH));
	if ((icon = fopen(iconfile, "r")) == NULL) {
		write(1, "\n", 1);
		return;
	}
	for (i=0; i<64; i++) {
		while (getc(icon)!='x')
			;
		hexbuf[0]=getc(icon); hexbuf[1]=getc(icon); 
		hexbuf[2]=getc(icon); hexbuf[3]=getc(icon); 
		write(1, hexbuf, 4);
	}
	write(1, "\n", 1);
	fclose(icon);
/*
	if(strlen(p) > 5)
		write(1, p+5, strlen(p)-5);
*/
}
