#include "univ.h"
#include "core.pub"
#include "bpts.pri"
#include "symbol.h"
#include "expr.pub"
#include "format.pub"
SRCFILE("bpts.c")

Trap.Trap(Stmt *s, Trap *t)
{
	trace( "%d.Trap( %d, %d, %d )", this, s, t );	VOK;
	key = s->range.lo;
	stmt = s;
	sib = t;
}

char *Trap.liftorlay(LiftLay lol, Core *core)
{
	trace( "%d.liftorlay(%d)", this, lol ); OK("Trap.liftorlay");
	if( !stmt ) return 0;
	if( error = lol==LIFT ? core->liftbpt(this) : core->laybpt(this) ){
		Stmt *s = stmt;
		stmt = 0;
		s->select();
	}
	return error;
}

void Bpts.banner()
{
	trace( "%d.banner()", this ); 	VOK;
	if( pad ){
		pad->banner("Breakpoints: %s", core->procpath());
		pad->name("Bpts %s", basename(core->procpath()));
	}
}

Bpts.Bpts(Core *c)
{
	Menu m;

	trace( "%d.Bpts(%d)", this, c ); 	VOK;
	core = c;
	pad = new Pad( (PadRcv*) this );
	m.last( "clear all?", (Action) &clearall );
	m.last( "clean list", (Action) &refresh );
	pad->menu(m);
	banner();
}

void Bpts.hostclose()
{
	trace( "%d.hostclose()", this ); 	VOK;
	lift();
	delete pad;
	pad = 0;
}

Trap *Bpts.istrap(Stmt *s)
{
	Trap *t;

	trace( "%d.istrap(%d)%s", this, s, s->text() );	OK(0);
	IF_LIVE(!s || !s->range.lo) return 0;
	for( t = trap; t; t = t->sib )
		if( t->stmt && t->stmt->range.lo == s->range.lo ) return t;
	return 0;
}

int Bpts.isasmbpt(long pc)
{
	Trap *t;

	trace( "%d.isasmbpt(%d)", this, pc );	OK(0);
	for( t = trap; t; t = t->sib )
		if( t->stmt && !t->stmt->parent && t->stmt->range.lo==pc ) return 1;
	return 0;
}
	

void Bpts.set(Stmt *s)
{
	Trap *t;

	trace( "%d.set(%d)%s", this, s, s->text() );	VOK;
	IF_LIVE( !s || !s->range.lo ) return;
	if( t = istrap(s) ){
		if( s != t->stmt ) t->error = sf(" same location as %s", s->text());
		select(t);
		t->stmt->select();
	} else {
		t = trap = new Trap(s, trap);
		if( layed ) t->liftorlay(LAY, core);
		select(t);
		s->select();
	}
}

void Bpts.clr(Stmt *s)
{
	Trap *t;

	trace( "%d.clr(%d)%s", this, s, s->text() );	VOK;
	IF_LIVE( !s || !s->range.lo ) return;
	if( t = istrap(s) ){
		if( layed ) t->liftorlay(LIFT, core);
		t->stmt = 0;
		select(t);
		s->select(SVP);
	} else
		pad->error( "no breakpoint at %s", s->text() );
}

int Bpts.isbpt(Stmt *s)
{
	Trap *t;

	trace( "%d.isbpt(%d) %s", this, s, s->text() );	OK(0);
	IF_LIVE(!s) return 0;
	for( t = trap; t; t = t->sib )
		if( t->stmt == s ) return 1;
	return 0;
}

Stmt *Bpts.bptstmt(long pc)
{
	Trap *t;

	trace( "%d.bptstmt(%d)", this, pc );	OK(0);
	IF_LIVE(!pc) return 0;
	for( t = trap; t; t = t->sib )
		if( t->stmt->range.lo == pc ) return t->stmt;
	return 0;
}

void Bpts.lay()
{
	Trap *t;

	trace( "%d.lay()", this );	VOK;
	if( layed ) return;
	for( t = trap; t; t = t->sib )
		if( t->liftorlay(LAY, core) ) select(t);
	layed = 1;
	
}

void Bpts.lift()
{
	Trap *t;

	trace( "%d.lift()", this );	VOK;
	if( !layed ) return;
	for( t = trap; t; t = t->sib )
		if( t->liftorlay(LIFT, core) ) select(t);
	layed = 0;
}

void Bpts.liftparents(Bpts *parent_bpts)
{
	Trap *savet;
	int savel;

	trace( "%d.liftparents(%d)", this, parent_bpts ); VOK;
	savet = trap;
	savel = layed;
	trap = parent_bpts->trap;
	layed = 1;
	lift();
	trap = savet;
	layed = savel;
}

void Bpts.select(Trap *t)
{
	Menu m;
	Bls buf;
	Attrib a = DONT_CUT;

	trace( "%d.select(%d)", this, t );	VOK;
	IF_LIVE(!t) return;
	if( t->stmt ){
		buf.af( "%s", t->stmt->text() );
		if( t->stmt->condition && t->stmt->condition!=Q_BPT )	/* ? */
			buf.af( " if(%s)", t->stmt->condition->text() );
	}
	if( t->error ){
		a |= SELECTLINE;
		buf.af( "%s", t->error );
	}
	if( t->stmt ){
		m.last( "clear bpt", (Action)&Stmt::dobpt, 0 );
		if( t->stmt->source() )
			m.last( "src  text", (Action)&Stmt::select );
		m.last( "assembler", (Action)&Stmt::asmblr );
	}
	t->error = 0;
	if( !buf.text[0] )		/* messy */
		pad->removeline( t->key );
	else
		pad->insert(t->key, a, (PadRcv*)t->stmt, m, buf.text );
}

void Bpts.clearall()
{
	Trap *t;

	trace( "%d.clearall()", this );	VOK;
	for( t = trap; t; t = t->sib )
		if( t->stmt ) clr(t->stmt);
	trap = 0; 				/* leaves garbage */
}

void Bpts.refresh()
{
	Trap *t;

	pad->clear();
	trace( "%d.refresh()", this );	VOK;
	for( t = trap; t; t = t->sib )
		if( t->stmt ) select(t);
}
