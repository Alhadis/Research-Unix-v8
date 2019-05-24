#include "termcore.h"
#include "hostcore.h"
#include "process.pri"
#include "srcdir.h"
#include "expr.pub"
#include "master.pub"
#include "bpts.pri"
#include "frame.pri"
#include "memory.pub"
#include "asm.pub"
#include "symtab.pub"
#include "symbol.h"
#include "srctext.pub"
#include "format.pub"
SRCFILE("process.c")

Process.Process(Process *sib, char *p, char *s, char *c)
{
	trace( "%d.Process(%d,%s,%s,%s)",this,sib,p,s,c); VOK;
	sibling = sib;
	procpath = p ? sf("%s",p) : 0;
	stabpath = s ? sf("%s",s) : 0;
	comment  = c ? sf("%s",c) : 0;
	stoprequest = 0;
	cycles = 0;
	_prefix = 0;
	srcdir = 0;
	parent = 0;
	isdead = 0;
}

void Process.currentstmt()
{
	trace( "%d.currentstmt()", this ); VOK;

	if( !callstk ){
		insert(ERRORKEY, "cannot show source (there is no callstack yet)");
		return;
	}
	for( long l = 0; l < callstk->size; ++l ){
		Frame *f = frame(l);
		if( !f->func ) continue;
		Stmt *stmt = f->func->stmt(f->pc);
/*		Stmt *stmt = (Stmt*) core->symtab()->loctosym(U_STMT, f->pc); */
		if( stmt ){
			stmt->select();
			return;
		}
	}
	insert(ERRORKEY, "cannot show source for any frame on callstack");
	pad->makecurrent();
}

Index Process.carte() { return ZIndex; }

void Process.closeframes()
{
	trace( "%d.closeframes()", this );	VOK;
	if( callstk )
		for( long l = callstk->size-1; l>=0; --l ){
			if( callstk->fpf[l].frame ){
			 	callstk->fpf[l].frame->hostclose();
				delete callstk->fpf[l].frame;
				callstk->fpf[l].frame = 0;
			}
		}
}

void Process.userclose()
{
	trace( "%d.userclose()", this );	VOK;
	if( _bpts ){
		_bpts->hostclose();
		delete _bpts;
		_bpts = 0;
	}
	if( srcdir ){
		srcdir->hostclose();
		delete srcdir;
		srcdir = 0;
	}
	if( memory ){
		memory->userclose();
		delete memory;
		memory = 0;
	}
	if( _asm ){
		_asm->userclose();
		delete _asm;
		_asm = 0;
	}
	closeframes();
	if( core ){
		core->close();
		delete core;
		core = 0;
	}
	if( globals ){
		globals->hostclose();
		delete globals;
		globals = 0;
	}
	if( pad ){
		delete pad;
		pad = 0;
	}
	isdead = 1;
	master->insert(this);
	master->makeproc(procpath, stabpath, comment);
}

Bpts *Process.bpts() { return _bpts; }

SymTab	*Process.symtab() { return core->symtab(); }

void Process.banner()
{
	trace( "%d.banner()", this );	VOK;
	if( pad ){
		pad->name(procpath);				/* basename ? */
		pad->banner("Process: %s", procpath);
	}
}

void Process.openpad()
{
	trace( "%d.openpad(%d)", this );	VOK;
	if( !pad ){
		pad = new Pad( (PadRcv*) this );
		pad->options(TRUNCATE|NO_TILDE);
		banner();
		pad->clear();			/* from a previous attempt! */
		pad->makecurrent();
	}
	pad->makecurrent();
}

void Process.openglobals(Menu *m)
{
	trace( "%d.openglobals()", this );	VOK;
	if( !globals ) globals = new Globals(core);
	globals->open();
	if( m) globals->addvars(m);
}

void Process.srcfiles()
{
	trace( "%d.srcfiles()", this );	VOK;
	if( !srcdir ) srcdir = new SrcDir( this );
	srcdir->open();
}

void Process.merge()
{
	trace( "%d.merge()",this,); VOK;
	CallStk *old = callstk;
	callstk = core->callstack();
	if( !old ) return;
	long c = callstk->size-1, i;
	for( long o = old->size-1; o>=0; --o ){
		Frame *of;
		if( of = old->fpf[o].frame ){
			for( i = c; i>=0; --i )
				if( callstk->fpf[i].func == old->fpf[o].func )
					break;
			if( i >= 0 ){
				Frame nf = callstk->frame(i);
				of->ap = nf.ap;
				of->fp = nf.fp;
				of->pc = nf.pc;
				of->regbase = nf.regbase;
				of->regsave = nf.regsave;
				if( of->pad ){
					of->banner();
					of->changes();
				}
				callstk->fpf[i].frame = of;
				of->level = i;
				c = i-1;
			} else {
				of->hostclose();
				delete of;
			}
		}
	}
	delete old;
}

void Process.mergeback(long)	/* arg never used - goes away */
{
	trace( "%d.mergeback()", this );	VOK;
	
	if( core->behavs() == ACTIVE ){
		stop();
		if( core->behavs() == ACTIVE ){
			insert(ERRORKEY, "process not stopped");
			return;
		}
	}
	merge();
	pad->lines(ERRORKEY);
	pad->lines(callstk->size+ERRORKEY);
	linereq(BEHAVSKEY);
}

Frame *Process.frame(long l)
{
	trace("%d.frame(%d)", this, l); OK(0);
	if( !callstk || l < 0 || l >= callstk->size ) return 0;
	Frame *f = callstk->fpf[l].frame;
	if( f ) return f;
	f = callstk->fpf[l].frame = new Frame(core);
	*f = callstk->frame(l);
	f->func = callstk->fpf[l].func;
	f->level = l;
	return f;
}

void Process.linereq(long l, Attrib a)
{
	trace("%d.linereq(%d,%x)", this, l, a); VOK;
	if( l <= ERRORKEY ){
		pad->insert(l, a, "%s", bls[l].text);
		return;
	}
	Frame *f = frame(l-ERRORKEY-1);
	Bls t;
	pad->insert(l, a|DONT_CUT, (PadRcv*)f, f->carte(), "%s", f->text(t));
}

void Process.insert(long l, PRINTF_ARGS)
{
	trace("%d.insert(%d)", this, l); VOK;
	if( l > ERRORKEY ) return;
	bls[l].clear();
	bls[l].af(PRINTF_COPY);
	pad->insert(l, DONT_CUT, "%s", bls[l].text);
}

void Process.openframe(long pc, char *k)
{
	trace( "%d.openframe(%d)", this, pc );	VOK;

	Func *func = (Func*) symtab()->loctosym(U_FUNC, pc);
	if( !func ){
		pad->insert(ERRORKEY, "cannot find function");
		return;
	}
	if( !callstk ){
		pad->insert(ERRORKEY, "there is no callstack yet");
		return;
	}
	long l;
	Frame *f = 0;
	for( l = 0; l < callstk->size; ++l )
		if( callstk->fpf[l].func == func ){
			f = frame(l);
			f->open();
			break;
		}
	if( !f ){
		mergeback(2);
		insert(ERRORKEY, "%s is not on callstack", func->text());
		pad->makecurrent();
		return;
	}
	for( ++l; l < callstk->size; ++l )
		if( callstk->fpf[l].func == func ){
			f->pad->insert(1, "called recursively - deepest instance");
			break;
		}
	if( f && k ) f->kbd(k);
}

void Process.openmemory(long a)
{
	trace( "%d.openmemory(%d)", this, a );	VOK;
	if( !memory ) memory = new Memory( core );
	memory->open( a );
}

void Process.openasm(long a)
{
	trace( "%d.openasm()", this );	VOK;
	if( !_asm ) _asm = core->newAsm();
	if( a!= -1 ) _asm->open( a );
}

void Process.openbpts()
{
	trace( "%d.openbpts()", this );	VOK;
	bpts()->pad->makecurrent();
}

void Process.stop()
{
	trace( "%d.stop()", this );		VOK;

	IF_LIVE( !core->online() ) return;
	insert(ERRORKEY, 0);
	switch( core->behavs() ){
	case ACTIVE:
		insert(ERRORKEY, core->stop());
		sleep(1);
		cycle();		/* will ignore next cycle from term */
		return;
	default:
		stoprequest = 1;
		habeascorpus(core->behavs(),changes());
	}
}

void Process.stmtstep(long i)
{
	char *error = 0;
	class Stmt *stmt;
	long changed = 0;

	trace( "%d.stmtstep(%d)", this, i );		VOK;
	insert(ERRORKEY, 0);
	stoprequest = 0;
	while( i>0 && !error && changed==0 ){
		insert(BEHAVSKEY, "step %d statements", i--);
		if( !(stmt = (Stmt*) symtab()->loctosym(U_STMT, core->pc())) )
			error = "cannot locate start stmt for step";
		else
			error = core->step(stmt->range.lo, stmt->range.hi);
		if( !error ) changed = changes();
	}
	insert(ERRORKEY, error);
	if( error ){
		docycle();
		return;
	}
	habeascorpus(STMT_STEPPED, changed);
}

void Process.stepinto()
{
	char *error = 0;
	class Stmt *stmt;
	long time(long*), t0 = time(0), timeout = 10, changed = 0;

	trace( "%d.stepinto()", this );		VOK;
	insert(ERRORKEY, 0);
	stoprequest = 0;
	if( !(stmt = (Stmt*) symtab()->loctosym(U_STMT, core->pc())) )
		error = "cannot locate start stmt for step into";
	while( !error && changed==0 ){
		if( core->pc() < stmt->range.lo ) break;
		if( core->pc() >= stmt->range.hi ) break;
		error = core->step();
		if( !error && time(0) > t0+timeout )
			error = sf( "step into timeout (%d secs)", timeout );
		if( !error )
			changed = changes();
	}
	if( !error )
		error = core->stepprolog();
	insert(ERRORKEY, error);
	if( error ){
		docycle();
		return;
	}
	habeascorpus(STMT_STEPPED, changed);
}

void Process.instrstep(long i)
{
	char *error = 0;
	long changed = 0;

	trace( "%d.instrstep(%d)", this, i );		VOK;
	stoprequest = 0;
	insert(ERRORKEY, 0);
	while( i>0 && !error && changed==0 ){
		insert(BEHAVSKEY, "step %d instructions", i--);
		error = core->step();
		if( !error ) changed = changes();
	}
	insert(ERRORKEY, error);
	if( error ){
		docycle();
		return;
	}
	habeascorpus(INST_STEPPED,changed);
	openasm( core->pc() );
}

void Process.stepover(long lo, long hi)
{
	trace( "%d.stepover()", this );		VOK;
	stoprequest = 0;
	char *error = core->step(lo, hi);
	insert(ERRORKEY, error);
	if( error ){
		docycle();
		return;
	}
	habeascorpus(INST_STEPPED, changes());
	openasm(core->pc());
}

void Process.go()
{
	Behavs b = core->behavs();
	char *error = 0;

	trace( "%d.go() %s", this, BehavsName(b) );	VOK;
	stoprequest = 0;
	switch( b ){
	case ACTIVE:
	case ERRORED:
		habeascorpus(b,0);
		break;
	case BREAKED:
		if( !error ) error = core->step();
	case INST_STEPPED:
	case STMT_STEPPED:
	case PENDING:
	case HALTED:
		if( !error ) _bpts->lay();
		if( !error ) error = core->run();
	}
	insert(ERRORKEY, error);
	docycle();
}

void Process.cycle()
{
	trace( "%d.cycle() %d", this );		VOK;
	if( cycles <= 0 ) return;
	--cycles;
	docycle();
}

void Process.docycle()
{
	Behavs b;
	
	trace( "%d.docycle() %d", this );		VOK;
	b = core->behavs();
	switch( b ){
	case ACTIVE:
		pad->alarm(1);
		++cycles;
		break;
	case HALTED:
	case PENDING:
	case BREAKED:
		if (core->online()) _bpts->lift();
		break;
	default:
		break;
	}
	habeascorpus(b,changes());
}

void Process.habeascorpus(Behavs b, long c)
{
	char *error;
	Bls *t = &bls[BEHAVSKEY];
	Frame *f;

	trace( "%d.habeascorpus(%d, c)", this, b, c );	VOK;
	t->clear();
	if( c > 0 ) t->af( "%d %s changed ", c, c==1 ? "spy" : "spies" );
	t->af( "%s ", BehavsName(b) );
	switch( b ){
		case BREAKED:
			class Stmt *stmt = bpts()->bptstmt(core->pc());
			if( !stmt || stoprequest ) break;
			Expr *e = stmt->condition;
			if( !e || e==Q_BPT || c>0 ) break;
			delete core->callstack();	/* term needs refreshed */
			Frame cf = core->frameabove(0);
			cf.addsymbols();
			stmt->condtext->clear();
			stmt->condtext->af("[%d] ", ++stmt->hits);
			e->evaltext(&cf, *stmt->condtext);
			if( e->val.lng )
				break;
			if( !e->evalerr ){
				pad->alarm();
				++cycles;
			}
			stmt->select();
			if( e->evalerr ){
				insert(ERRORKEY, "%s",stmt->condtext->text);
				break;
			}
			error = core->step();
			if( !error ) _bpts->lay();
			if( !error ) error = core->run();
			insert(ERRORKEY, error);
			return;
		case ERRORED:
			t->af( core->problem() );
			break;
		case PENDING:
			t->af( core->eventname() );
			break;
		case ACTIVE:
			t->af( "%s ", core->resources() );	/* fall thru*/
		case INST_STEPPED:
		case STMT_STEPPED:
			long pc = core->pc();
			if( pc ){
				if( *symtab()->symaddr(pc) )
					t->af( "pc=%s", symtab()->symaddr(pc) );
				else
					t->af( "pc=%d", pc );
			}
			break;
	}
	switch( b ){
		case ACTIVE:
			pad->lines(ERRORKEY);
		case ERRORED:
			linereq(BEHAVSKEY);
			break;
		case HALTED:
		case BREAKED:
		case INST_STEPPED:
		case STMT_STEPPED:
		case PENDING:
			mergeback(2);
			if( globals ) globals->changes();
			if( b!=INST_STEPPED && (f = frame(0)) )
				f->select(b != STMT_STEPPED ? SVP : 0);
	}
}

int Process.changes()
{
	int changed = 0;
	Frame *f;

	trace( "%d.changes()", this );		OK(0);
	if( callstk )
		for( long l = 0; l < callstk->size; ++l )
			if( f = callstk->fpf[l].frame )
				changed += f->changes();
	if( globals ) changed += globals->changes();
	if( memory ) changed += memory->changes();
	return changed;
}

char *Process.prefix(char *p)
{
	trace( "%d.prefix(%s) %s", this, p, _prefix );	OK("Process.prefix");
	if( *p=='/' && strncmp(_prefix, "/n/",  3) )
		return p;
	if( p && _prefix )
		return sf( "%s/%s", _prefix, *p=='/' ? p+1 : p );
	return p;
}
