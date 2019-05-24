#include "univ.h"
#include "dtype.pri"
#include "symbol.h"
#include "symtab.pub"
#include "srctext.pub"
#include "core.pub"
#include "process.pub"
#include "bpts.pub"
#include "phrase.pub"
#include "parse.h"
#include "format.pub"
SRCFILE("symbol.c")

int Symbol.disc()	{ trace( "%d.disc()", this ); return U_ERROR; }
int Var.disc()	{ return _disc; }

int Symbol.ok()
{
	if( !this ) return 0;
	switch( disc() ){
		case U_ARG:
		case U_AUT:
		case U_BLOCK:
		case U_FST:
		case U_FUNC:
		case U_GLB:
		case U_MOT:
		case U_REG:
		case U_SOURCE:
		case U_STA:
		case U_STMT:
		case U_UTYPE:
			return 1;
	}
	return 0;
}

int Var.ok()
{
	if( !this ) return 0;
	switch( disc() ){
		case U_ARG:
		case U_AUT:
		case U_FST:
		case U_GLB:
		case U_MOT:
		case U_STA:
			return 1;
	}
	return 0;
}


Symbol.Symbol( Symbol *up, Symbol *left, char *t )
{					
	trace( "%d.Symbol(%d ^%d <%d %s)", this, up, left, t );
	_text = t;
	parent = up;
	if( left ) left->rsib = this;
	this->lsib = left;
}

Stmt.Stmt(SymTab *stab, Block *up, Stmt *left ):(up,left,"<stmt>")
{
	if(stab) stab->enter(this);
	process = stab ? stab->core()->process(): 0;
	condition = 0;
	condtext = 0;
}

Block.Block(SymTab *stab, Symbol *up, Block *left, char *t ):(up,left,t) {}

BlkVars.BlkVars(Block *i)	{ VOK; b = i; v = 0; }

Var *BlkVars.gen()
{
	trace( "%d.gen() %d %d", this, v, b );	OK(0);
	if(v) v = (Var*)v->rsib;
	while( !v && b && b->disc()==U_BLOCK ){
		v = b->var;
		b = (Block*)b->parent;
	}
	trace( "%s", v->dump() );
	return v;
}

Source.Source(SymTab *stab, Source *left, char *t, long c):(0,left,t)
{
	symtab = stab;
	blk = new Block( stab, this, 0, sf("%s.sta_blk",t) );
	srctext = new SrcText(this,c);
}

UType.UType(SymTab *stab, long b, long s, char *id):(0,0,id)
{
	trace( "%d.UType(%d,%d,%d,%s)", this, stab, b, s, id );
	begin = b;
	size = s;
	symtab = stab;
	canspecial = stab->core()->specialop(_text);
	if( stab ) stab->enter(this);
}

Var.Var(SymTab *stab, Block *up, Var *left, UDisc d, char* id):(up,left,id)
{
	if( (_disc = d)<=TOSYM && stab ) stab->enter( this );
}

Func.Func(SymTab *stab, Source *up, Func *left, long i, char* id):(up,left,id)
{
	begin = i;
	if( stab ) stab->enter( this );
}

char *Symbol.dump()
{
	static char t[128];

	if( !this ) return "0";
	sprintf(t,"%d %s %s %d %d",this,DiscName(disc()),_text,range.lo,range.hi);
	return t;
}

void Func.gather()
{
	Source *src = source();

	trace("%d.gather() %s", this, dump() ); VOK;
	if( _blk = src->symtab->gatherfunc(this) )
		_blk->parent = src->blk;
	else
		_blk = src->blk;
}

void UType.gather()
{
	trace("%d.gather() %s ",this, dump() ); VOK;
	mem = symtab->gatherutype(this);
	if( !mem ) return;			/* empty struct - what to do ? */
	TypMems g(this);
	Var *v;
	if( v = g.gen() ) v->pick = 1;
	if( v = g.gen() ) v->pick = 1;
}

Source *Symbol.source()
{
	trace( "%d.source() %s", this, dump() );
	return !this ? 0 : disc() == U_SOURCE ? (Source*)this : parent->source();
}

char *Symbol.text(long) { OK("Symbol.text"); return _text; }

char *Source.text(long) { OK("Source.text"); return basename(_text); }

Stmt *Func.stmt(long pc)
{
	Stmt *s, *r;

	trace( "%d.stmt(%d)", this, pc ); OK(0);
	for( s = blk()->stmt; s; s = r ){
		r = (Stmt*)s->rsib;
		if( !r || r->range.lo>pc ) break;
	}
	return s && s->range.lo<=pc ? s : 0;
}

TypMems.TypMems(UType *i)	{ ut = i; v = 0; }

Var *TypMems.gen()
{
	trace( "%d.gen()", this ); OK(0);
	if( ut ){
		if( !ut->mem ) ut->gather();
		v = ut->mem;
		ut = 0;
	} else if( v )
		v = (Var*)v->rsib;
	trace( "%s", v->dump() );
	return v;
}

Block *Func.blk()
{
	OK(0);
	if(!_blk) gather();
	return _blk;
}

Block *Func.blk(long pc)
{
	Stmt *s;

	trace( "%d.blk(%d)", this, pc ); OK(0);
	if( !pc || !(s = stmt(pc)) || !s->parent ) return blk();
	return (Block*) s->parent;
}

Var *Func.argument( int a )
{
	trace( "%d.argument(%d)", this, a ); OK(0);
	BlkVars bv(blk());
	Var *v;
	int i = 0;
	while( v = bv.gen() )
		if( v->disc()==U_ARG && ++i==a ) return v;
	return 0;
}

char *Func.text(long)
{
	return sf( "%s()", this ? _text : "?" );
}

char *Stmt.text(long pc)
{
	char buf[256];

	trace( "%d.text(%d)", this, pc ); OK( "Stmt.text" );
	Source *src = source();
	if( !src ) return sf( "pc=%d", range.lo );
	sprintf( buf, "%s:%d", src->text(), lineno );
	if( pc && range.lo < pc )
		strcatfmt( buf, "+%u", pc-range.lo );
	return sf( "%s", buf );
}

void Stmt.select(long svp)
{
	trace( "%d.select(%d)", this, svp );	VOK;
	Source *src = source();
	if( src ) src->srctext->select(lineno, svp);
	else asmblr();
}

char *Stmt.contextsearch(char *pat, int dir)
{
	trace( "%d.conetxtsearch(%s,%d)", this, pat?pat:"", dir); OK("search");
	Source *src = source();
	if( src )
		return src->srctext->contextsearch(lineno, pat, dir);
	return "can't search";
}

Pad *Stmt.srcpad() { return source()->srctext->pad; }

void Stmt.error(char *s)
{
	trace( "%d.error(%s)", this, s );		VOK;
	Line l;
	l.text = s;
	l.object = this;
	l.attributes |= SELECTLINE;
	l.key = lineno;
	srcpad()->insert( l );
}

char *Stmt.kbd(char *s)
{
	Parse y(G_EXPR, 0);
	trace( "%d.kbd(%s)", this, s );		OK("kbd");
	if( condition != Q_BPT ){
		switch( *s ){
			case '/': return contextsearch(s+1,  1);
			case '?': return contextsearch(s+1, -1);
		}
		process->openframe( range.lo, s );
		return 0;
	}
	Expr *newcond = (Expr*)y.parse(s);
	if( newcond ){
		condition = newcond;
		if( !condtext ) condtext = new Bls;
		condtext->clear();
		condtext->af("%s", newcond->text() );
		dobpt(1);
		select();
	} else {
		dobpt(0);
		error( sf("%s: %s", y.error, s) );
	}
	return 0;
}

char *Stmt.help()
{
	trace( "%d.help()", this );
	return condition == Q_BPT
		? "<expr> {breakpoint condition}"
		: "<expr> {eval in frame} | [/?]<string> {search}";
}

void Stmt.conditional()
{
	trace( "%d.conditional()", this );	VOK;
	if( condtext ) delete condtext;
	condtext = 0;
	condition = Q_BPT;
	select();
}

Stmt *Source.stmtafter(int l)
{
	trace( "%d.stmtafter(%d)", this, l ); OK(0);
	
	if( !linefunc ) return 0;
	while( l < linefunc->lines.lo && linefunc->lsib ){
		linefunc = (Func*) linefunc->lsib;
		trace( "%d %s ..%d]", l, linefunc->text(), linefunc->lines.hi );
		linestmt = 0;
	}
	while( l > linefunc->lines.hi && linefunc->rsib ){
		linefunc = (Func*) linefunc->rsib;
		trace( "%d %s [%d..", l, linefunc->text(), linefunc->lines.lo );
		linestmt = 0;
	}
	if( !linestmt && !(linestmt = linefunc->blk()->stmt) )
		return 0;
	while( l < linestmt->lineno && linestmt->lsib ){
		linestmt = (Stmt*) linestmt->lsib;
		trace( "%d [%d]", l, linestmt->lineno );
	}
	while( l > linestmt->lineno && linestmt->rsib ){
		linestmt = (Stmt*) linestmt->rsib;
		trace( "%d [%d]", l, linestmt->lineno );
	}
	return linestmt;
}

void Stmt.asmblr()
{
	trace( "%d.asmblr() %d", this, process );	VOK;
	if( process ) process->openasm(range.lo);
}

void Stmt.dobpt(int setorclr)
{
	trace( "%d.dobpt(%d) %d", this, setorclr, process ); VOK;
	if( condition == Q_BPT ){
		condtext = 0;
		condition = 0;
		if( setorclr )
			process->bpts()->set( this );
		else
			select();
	} else {
		if( setorclr )
			process->bpts()->set( this );
		else {
			if( condtext ) delete condtext;
			condtext = 0;
			condition = 0;
			process->bpts()->clr( this );
		}
	}
}

void Stmt.openframe()
{
	trace( "%d.openframe()", this ); VOK;
	process->openframe( range.lo );
}

Index UType.carte(Op op)
{
	Menu m;
	Var *v;
	TypMems tm(this);
	long n = 0;
	char *on = (char*) OpName(op);
	Action a = (Action)(op==O_ARROW ? &Phrase::applyarrow : &Phrase::applydot);

	trace( "%d.carte(%s)", this, on ); OK(ZIndex);
	if( canspecial )
		m.last( canspecial, (Action)&Phrase::applyunary, O_SPECIAL );
	while( v = tm.gen() ){
		Bls field( "$%s%s\240", on, v->_text );
		m.sort( field.text, a, (long) v );
		++n;
	}
	a = (Action)(op==O_ARROW ? &Phrase::allstar : &Phrase::alleval);
	m.first(sf("$%s*",on), a, (long)this);
	return m.index(n>4 ? sf("$%sid",on) : 0);
}

void UType.pickplus(int i)
{
	trace( "%d.markall(%s)", this ); VOK;
	Var *v;
	TypMems tm(this);
	while( v = tm.gen() )
		v->pick += i;
}	

Source.~Source(){}
Stmt.~Stmt()	{}
UType.~UType()	{}
Var.~Var()	{ /* on the stack in Expr */ }

Func.~Func()
{
	type.free();
}

Block.~Block()
{
	while( var ){
		var->type.free();
		delete var;
		var = (Var*)var->rsib;
	}
}
