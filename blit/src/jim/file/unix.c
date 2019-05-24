#include "file.h"
#include "msgs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

char errfile[64];

#define	CMDSIZE	128
char	unixcmd[CMDSIZE];
extern	tempfile;

Unix(f, type, wholefile)
	File *f;
{
	register pid, rpid;
	int (*onbpipe)();
	int retcode;
	char unixbuf[512];
	int	pipe1[2];
	int	pipe2[2];
	long	nbytes=0;
	long	posn, ntosend;
	if(errfile[0]==0){
		if(getenv("HOME"))
			strcpy(errfile, getenv("HOME"));
		else
			strcpy(errfile, "/tmp");
		strcat(errfile, "/jim.err");
	}
	if(wholefile){
		posn=0;
		ntosend=length(f);
	}else{
		posn=f->selloc;
		ntosend=f->nsel;
	}
	Fputblock(f);
	if(type != '!'){
		if(pipe(pipe1) == -1)
			error("couldn't make pipe", (char *)0);
	}
	if(type == '|')	/* align temp files for both sides of pipe */
		splitblock(f, Fseek(f, posn));
	unlink(errfile);
	if ((pid = fork()) == 0) {
		int fd=creat(errfile, 0666);
		if(fd<0)
			fd=creat("/dev/null", 0666);
		close(2);
		dup(fd);
		close(fd);
		if(type=='<' || type=='|'){
			close(1);
			dup(pipe1[1]);
		}else if(type == '>'){
			close(0);
			dup(pipe1[0]);
			close(1);
			dup(2);
		}
		if (type != '!') {
			close(pipe1[0]);
			close(pipe1[1]);
		}
		if(type == '|'){
			if(pipe(pipe2) == -1)
				exit(1);
			if((pid=fork()) == 0){
				/*
				 * It's ok if we get SIGPIPE here
				 */
				close(pipe2[0]);
				exit(Fwritepart(f, posn, ntosend, pipe2[1]));
			}
			if(pid == -1){
				write(2, "Can't fork\n?!", 13);
				exit(1);
			}
			close(0);
			dup(pipe2[0]);
			close(pipe2[0]);
			close(pipe2[1]);
		}
		close(tempfile);
		if(type == '<')
			close(0);	/* so it won't read from jim.m */
		execl("/bin/sh", "sh", "-c", unixcmd, 0);
		exit(-1);
	}
	if(pid == -1)
		error("Can't fork", (char *)0);
	if(type=='<' || type=='|') {
		close(pipe1[1]);
		nbytes=Fread(f, posn+ntosend, (char *)0, FALSE, pipe1[0]);
		close(pipe1[0]);
	} else if(type == '>') {
		onbpipe = signal(SIGPIPE, 1);
		close(pipe1[0]);
		Fwritepart(f, posn, ntosend, pipe1[1]);
		close(pipe1[1]);
		signal(SIGPIPE, onbpipe);
	}
	do; while ((rpid = wait(&retcode)) != pid && rpid != -1);
	retcode = (retcode>>8)&0377;
	if(type == '|' || type=='<'){
		if(retcode == 0){
			Fdeltext(f, posn, ntosend);
			if(wholefile)
				f->selloc=f->origin=0;
			f->nsel=nbytes;
		} else
			dprintf("status!=0; original not deleted.\n", (char *)0);
	}
	checkerrs();
	if(nbytes)
		modified(f);
	if(type=='<' || type=='|')
		refresh(f);
}
refresh(f)
	register File *f;
{
	register x=f->nsel>32000? 32000 : f->nsel;
	send(f-file, O_SELECT, (int)(f->selloc-f->origin), 2, data2(x));
	send(f-file, O_MOVE, -1, 2, data2(32767));
	tellseek(f);
}
checkerrs(){
	struct stat statb;
	char buf[64];
	register f;
	register char *p;
	if(stat(errfile, &statb)==0 && statb.st_size){
		if((f=open(errfile, 0)) != -1){
			if(read(f, buf, sizeof buf-1)>0){
				for(p=buf; p<&buf[sizeof buf-1]; p++)
					if(*p=='\n')
						break;
				*p=0;
				dprintf("%s", buf);
				if(p-buf > statb.st_size)
					dprintf("...");
			}
			close(f);
		}
		dprintf("\n");
	} else
		unlink(errfile);
}
