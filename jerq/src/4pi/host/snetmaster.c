#include "snetcore.h"
#include "master.pri"
#include "process.pri"
#include "symtab.pri"
SRCFILE("termmaster.c")

Process	*SnetMaster.domakeproc(char *proc, char *stab, char* comment)
{
	return (Process*) new SnetProcess(child, proc, stab, comment);
}

extern char *SNETSYMS;

SnetMaster.SnetMaster(Remote *r )
{
	Menu m;

	trace( "%d.SnetMaster()", this );	VOK;
	pad = new Pad( (PadRcv*) this );		/* this code cannot */
	pad->options(TRUNCATE|SORTED);			/* be in base ctor */
	pad->name( "Spi" );
	pad->banner( "Spi:" );
	m.last( "host pi", (Action)&pi );
	pad->menu(m);
	pad->makecurrent();
	if( SNETSYMS ){
		Process *p = (Process*) new SnetProcess(0, 0, SNETSYMS);
		core = new SnetCore(p, this);
		pad->insert(1, FLUSHLINE, "Checking %s symbols ...", SNETSYMS );
		char *error = core->open();
		if( error )
			pad->insert(1, "%s: %s", SNETSYMS, error);
		else
			pad->insert(1, "%s", core->symtab()->warn() );
	}
	remote = r;
	CoreVersion(remote);
	remote->pktstart(C_SPEE);
	remote->sendlong(getpid());
	remote->pktflush();
	SnetProcess *p = makeP(1);
	p->open();
}

SnetProcess *SnetMaster.makeP(long P, char *stab)
{
	trace( "%d.makeP(%d,%s)", this, P, stab );	OK(0);
	return (SnetProcess*) makeproc( sf("Proc%d",P), stab, 0 );
}

char *SnetMaster.kbd(char *s)
{
	return "kbd";
}

char *SnetMaster.help()
{
	return "help";
}

void SnetMaster.pi()
{
	trace( "%d.pi()", this );	VOK;
	if( !hostmaster ) hostmaster = new HostMaster;
	hostmaster->pad->makecurrent();
}
