#include "hostcore.h"
#include "master.pri"
#include "process.pri"
#include "symtab.pri"
SRCFILE("kernmaster.c")

Process	*KernMaster.domakeproc(char *p, char *s, char* c)
{
	return (Process*) new KernProcess(child, p, s, c);
}

extern char *UNIX;

KernMaster.KernMaster(SymTab *s)
{
	trace( "%d.KernMaster()", this );	VOK;
	pad = new Pad( (PadRcv*) this );		/* this code cannot */
	pad->options(TRUNCATE|SORTED);			/* be in base ctor */
	pad->name("kpi");
	pad->banner("Kernel pi:");
	pad->makecurrent();
	core = new KernCore(0,this);
	core->_symtab = s;
	refresh();
}

void KernMaster.refresh()
{
	Process *p;

	trace( "%d.refresh()", this );	VOK;
	pad->clear();
	for( int i = 0; i < 5; ++i )
	    for( char *dir = "/tmp/dump/"; ; dir = "" ){
		char *dump = sf("%svmcore.%d", dir, i);
		char *syms = sf("%sunix.%d", dir, i);
		int qfd = open(dump,0);
		if( qfd >= 0 ){
			close(qfd);
			makeproc(dump);
			qfd = open(syms,0);
			if( qfd >= 0 ){
				close(qfd);
				makeproc(dump, syms);
			}
		}
		if( !strcmp(dir,"") ) break;
	}
	for( p = child; p; p = p->sibling )
		if( p->core ) insert(p);
}

char *KernMaster.kbd(char *s)
{
	char *corep, core[64], syms[64], star = 0;
	KernProcess *p;

	trace( "%d.kbd(%s)", this, s );		OK("kbd");
	while( *s == ' ' ) ++s;
	while( *s==' ' ) ++s;
	switch( *s ){
	case '*':
		star = 1;
		for( ++s; *s==' '; ++s ) {}
	default:
		switch( sscanf(s, "%s %s \n", corep = core, syms) ){
		case 1: syms[0] = '\0';
		case 2:	p = (KernProcess*) makeproc(corep, syms[0]?syms:0, 0);
			if( star && p ) p->open();
			break;
		default:
		return help();
		}
	}
	return 0;
}

char *KernMaster.help()
{
	trace( "%d.help()", this );
	return "[*] <path> [<tables>] {[open] system dump}";
}
