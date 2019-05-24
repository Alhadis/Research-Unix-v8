#include "snetcore.h"
#include "process.pri"
#include "srcdir.h"
#include "expr.pub"
#include "master.pub"
#include "bpts.pub"
#include "frame.pri"
#include "memory.pub"
#include "asm.pub"
#include "symtab.pub"
#include "symbol.h"
#include "srctext.pub"
#include "format.pub"
#include <ctype.h>
SRCFILE("snetproc.c")

Index SnetProcess.carte()
{
	Menu m( "open process", (Action)&SnetProcess::open );
	trace( "%d.carte()", this ); OK(ZIndex);
	return m.index();
}

void SnetProcess.open()
{
	Menu m;
	char *error;

	trace( "%d.open(%d)", this );	VOK;
	Process::openpad();
	if( core ) return;
	insert(ERRORKEY, "Checking process and symbol table...");
	core = (Core*) new SnetCore(this, (SnetMaster*)master);
	if( error = core->open() ){
		delete core;
		core = 0;
		m.last( "open process", (Action)&open, 0 );
		pad->menu( m );
		insert(ERRORKEY, error);
		return;
	}
	insert(ERRORKEY, core->symtab()->warn());
	globals = new Globals(core);
	_asm = core->newAsm();
	m.last( "go",   (Action)&go,  );
	m.last( "stop", (Action)&stop );
	m.last( "src text", (Action)&srcfiles );
	m.last( "Globals",   (Action)&openglobals );
	m.last( "RawMemory", (Action)&openmemory  );
	m.last( "Assembler", (Action)&openasm     );
	pad->menu(m);
	_bpts = new Bpts(core);
	_bpts->lay();
	pad->makecurrent();
	docycle();
}
