#include "termcore.h"
#include "master.pri"
#include "format.pri"
#include <CC/signal.h>
SRCFILE("main.c")

void ErrExit(char *e)
{
	fprintf(stderr, "%s\n", e);
	exit(1);
}

#define SIZE 4096
void Copy(int from, int to)
{
	char buf[SIZE];

	for( ;; ) write(to, buf, read(from, buf, SIZE));
}

void ReadWrite(char *dev)
{
	Remote *r = new Remote(dev);
	if( fork() )
		Copy( r->fd, 1 );
	else
		Copy( 0, r->fd );
}
void KillGroup(int minutes)
{
	sleep(minutes*60);
	::killpg(0,SIGKILL);
}

Remote *Share(char *comment = "")
{
	Remote *r = new Remote("/dev/tty");
	CoreVersion(r, comment);
	r->share();
	return r;
}

void Spawn(char *cmd)
{
	Remote *r = Share();
	r->pktstart(C_SPER);
	r->sendlong(getpid());
	r->sendstring(cmd);
	r->pktflush();
	KillGroup(10);
}

void Mail(char *to, char *cmd )
{
	FILE *fp, *Popen(char*,char*);
	if( fp = Popen(sf("/bin/mail %s", to), "w") )
		fprintf( fp, "%s\n", cmd );
	if( !fp || Pclose(fp)==-1 )
		ErrExit( "cannot send mail" );
	Share( sf("3pi: waiting for %s...", to) );
	KillGroup(24*60);
}

char *SYS()
{
	char *sysfile = "/etc/whoami";
	static char sys[32];
	if( !sys[0] ){
		FILE *fp = fopen(sysfile, "r");
		if( !fp || !fgets(sys, sizeof sys -1, fp) )
			ErrExit( "cannot determine host name" );
		fclose(fp);
		sys[strlen(sys)-1] = '\0';
	}
	return sys;
}

char *ttyname(int), *getenv(char*), *Getwd();
char *CD;
char *COREHOST;
char *COREDEV;
char *MUXTERM = "/usr/jerq/lib/muxterm";
char *UNIX = "/unix";
char *SNETSYMS = 0;
char *PADSTERM;
char *PIHOST = "/usr/jerq/bin/3pi";
char *PITERM = "32ld /usr/jerq/mbin/pi.m";
char *DEVKMEM = "/dev/kmemr";
char *TAPTO = 0;
char *HOME = 0;

void LoadTerm()
{
	if( PadsInit(PADSTERM) )
		ErrExit("cannot load terminal");
}

Remote *NewRemote()
{
	if( COREHOST ){
		printf("connecting to %s %s...", COREHOST, COREDEV);
		fflush(stdout);
		char *server = sf( "%s -r %s", PIHOST, COREDEV );
		int fd = tdkexec(COREHOST, server);
		if( fd<0 ){
			extern char *dkerror;
			ErrExit(dkerror ? dkerror : "tdkexec failed");
		}
		return new Remote(fd);
	} else
		return new Remote(COREDEV);
}

void mainbatch(char **av)
{
	char *core = "core", *aout = "a.out";
	if( *av ) core = *av++;
	if( *av ) aout = *av++;
	new BatchMaster(core, aout);
	exit(0);
}

void mainpi(char **av)
{
	if( !strcmp(av[0],"-t") )
		mainbatch(av+1);
	LoadTerm();
	TapTo = TAPTO;
	NewHelp();
	NewWd();
	new HostMaster();
	PadsServe();
}

#include <sgtty.h>
void mainspi()
{
	Remote *r = NewRemote();
	ioctl(r->fd, TIOCNXCL);
	LoadTerm();
	NewHelp();
	NewWd();
	new SnetMaster(r);
	PadsServe();
}
	
void main3pi(char **av)
{
	Remote *r;
	if( COREDEV ){
		r = NewRemote();
		LoadTerm();
		NewHelp();
		NewWd();
		new TermMaster(r);
		PadsServe();
	}
	char *programmer = 0;
	while( *av ){
		if( eqstr(*av, "-r") && *++av )
			ReadWrite(*av);
		else if( eqstr(*av, "-p") && *++av )
			programmer = *av++;
		else
			ErrExit("use: 3pi [-p programmer]" );
	}
	COREDEV = ttyname(0);
	if( !COREDEV )
		ErrExit( "cannot determine tty name" );
	if( system(PITERM) )
		ErrExit( "cannot load terminal agent" );
	Bls spawn;
	if( PADSTERM )
		spawn.af( "PADSTERM=\"%s\" ", PADSTERM );
	spawn.af( "PIHOST=%s COREDEV=%s MUXTERM=%s UNIX=%s DEVKMEM=%s %s",
			PIHOST, COREDEV, MUXTERM, UNIX, DEVKMEM, PIHOST );
	if( programmer )
		Mail(programmer, sf("COREHOST=%s %s", SYS(), spawn.text) );
	Spawn( sf("%s %s", CD, spawn.text) );
}

void main(int, char **av)
{
	char *e;
	if( e = getenv("COREHOST") ) COREHOST = e;
	if( e = getenv("COREDEV") ) COREDEV = e;
	if( e = getenv("MUXTERM") ) MUXTERM = e;
	if( e = getenv("PADSTERM") ) PADSTERM = e;
	if( e = getenv("PIHOST") ) PIHOST = e;
	if( e = getenv("PITERM") ) PITERM = e;
	if( e = getenv("UNIX") ) UNIX = e;
	if( e = getenv("DEVKMEM") ) DEVKMEM = e;
	if( e = getenv("HOME") ) HOME = e;
	if( e = getenv("TAPTO") ) TAPTO = e;
	if( e = getenv("SNETSYMS") ) SNETSYMS = e;
	CD = sf( "builtin cd %s;", Getwd() );
	if( HOME ) TAPTO = sf("%s/.pilog", HOME);

	char *base = basename(*av++);
	if( !strcmp(base,"3pi") )
		main3pi(av);
	else if( !strcmp(base,"spi") )
		mainspi();
	else
		mainpi(av);
}
