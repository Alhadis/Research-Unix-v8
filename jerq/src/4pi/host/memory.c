#include "core.pub"
#include "symtab.pub"
#include "memory.pri"
#include "parse.h"
#include "expr.pub"
#include "frame.pub"
#include "process.pub"
#include "format.pub"
SRCFILE("memory.c")

void Memory.userclose()
{
	trace( "%d.userclose(%d)", this );	VOK;
	delete pad;
	pad = 0;
	invalidate();
	for( ; cellset; cellset = cellset->sib ) delete cellset;
	current = 0;
}

void Memory.makecell(Cell *model, long addr )
{
	Cell *c;

	trace( "%d.makecell(%d,%d)", this, model, addr );	VOK;
	if( !model ){
		model = new Cell(this);
		model->fmt = F_HEX;
		model->size = 1;
	}
	for( c = cellset; c; c = c->sib )
		if( addr == c->addr ) break;
	if( !c ){
		c = new Cell(this);
		c->addr = addr;
		c->sib = cellset;
		cellset = c;
	}
	c->fmt = model->fmt;
	c->size = model->size;
	c->display();
	current = c;
}

void Memory.banner()
{
	trace( "%d.banner()", this );	VOK;
	if( pad ){
		pad->banner("Memory: %s", core->procpath());
		pad->name("Mem %s", basename(core->procpath()));
	}
}

void Memory.open(long a)
{
	trace( "%d.open(%d)", this, a );		VOK;
	if( !pad ){
		pad = new Pad( (PadRcv*) this );
		banner();
	}
	pad->makecurrent();
	if( a ) makecell(current,a);
}

char *Memory.kbd(char *s)
{
	Parse y(G_DOTEQ_CONEX,0);
	Expr *e;
	Bls error;

	trace( "%d.kbd(%s)", this, s );	OK(0);
	if( !(e = (Expr*)y.parse(s)) )
		return sf("%s: %s", y.error, s);
	e->evaltext(core->process()->globals, error);
	if( e->evalerr )
		return sf("%s: %s", s, error.text);
	makecell(current,e->val.lng);
	return 0;
}

struct FmtSize {
	long	fmt;
	long	size;
};

FmtSize FS[] = {
	F_DECIMAL,	7,
	F_SIGNED,	7,
	F_OCTAL,	7,
	F_HEX,		7,
	F_ASCII,	7,
	F_SYMBOLIC,	7,
	F_TIME,		4,
	F_FLOAT,	4,
	F_DOUBLE,	8,
	F_DBLHEX,	8,
	0,		0
};

Index Cell.carte()
{
	Menu m, s, f;

	OK(ZIndex);
	m.last( spy ? "unspy" : "spy on", (Action)&setspy, !spy );
	s.last("1 byte ", (Action)&resize, 1);
	s.last("2 bytes", (Action)&resize, 2);
	s.last("4 bytes", (Action)&resize, 4);
	s.last("8 bytes", (Action)&resize, 8);
	m.last(s.index("size"));

	for( int i = 0; FS[i].fmt; ++i )
		if( FS[i].size & size ){
			long b = FS[i].fmt;
			if( fmt&b ) b |= F_TURNOFF;
			f.last(FmtName(b), (Action)&reformat,	b);
		}
	m.last(f.index("format"));
	m.last( ".+1",		(Action)&relative,	1	);
	m.last( ".+2",		(Action)&relative,	2	);
	m.last( ".+4",		(Action)&relative,	4	);
	if( size < 8 )
		m.last( "* thru .",	(Action)&indirect,	0	);
	m.last( ".-1",		(Action)&relative,	-1	);
	m.last( ".-2",		(Action)&relative,	-2	);
	m.last( ".-4",		(Action)&relative,	-4	);

	m.last( "asmblr",	(Action)&asmblr,   	0	);

	return m.index();
}

void Cell.display(char *error)
{
	Cslfd *m;
	Bls t;
	long afmt = fmt&~(F_ASCII|F_DOUBLE|F_FLOAT|F_DBLHEX|F_TIME);
	long cfmt = fmt&~F_SYMBOLIC;

	trace( "%d.display()", this );	VOK;
	switch( size ){
	case 8:
		if( !(cfmt &= F_DOUBLE|F_DBLHEX) )
			cfmt = F_DOUBLE;
		break;
	default:
		if( !(cfmt &= ~(F_DOUBLE|F_DBLHEX)) )
			cfmt = F_HEX;
	}
	Format a(afmt?afmt:F_HEX, memory->core->symtab());
	Format c(cfmt);
	if( spy ) t.af( ">>> " );
	t.af( "%s/%d: ", a.f(addr), size );
	switch( size ){
		case 1: c.format |= F_MASKEXT8;  break;
		case 2: c.format |= F_MASKEXT16; break;
	}
	if( m = memory->core->peek(addr,0) ){
		long val;
		switch( size ){
			case 1: c.format |= F_MASKEXT8;
				val = m->chr;
				break;
			case 2: c.format |= F_MASKEXT16;
				val = m->sht;
				break;
			case 4: val = m->lng;
			}
		t.af( "%s", c.f(val, m->dbl) );
		if( spy ) *spy = *m;
	} else
		t.af( "cannot read" );
	if( error ) t.af( " %s", error );
	Attrib attrib = ACCEPT_KBD|SELECTLINE;
	if( spy ) attrib |= DONT_CUT;
	memory->pad->insert(addr, attrib, (PadRcv*)this, carte(), "%s", t.text);
}

int Cell.changed()
{
	int changed = 0;
	Cslfd *m;

	trace( "%d.changed()" ); OK(0);
	if( !spy ) return 0;
	m = memory->core->peek(addr);
	switch( size ){
		case 1: if( m->chr != spy->chr ) changed = 1;  break;
		case 2: if( m->sht != spy->sht ) changed = 1;  break;
		case 4: if( m->lng != spy->lng ) changed = 1;  break;
		case 8: if( m->dbl != spy->dbl ) changed = 1;  break;
	}
	if( changed ) display();
	return changed;
}


void Cell.setspy(long s)
{
	trace( "%d.setspy(%d)", this, s );	VOK;
	if( !s && spy ){
		delete spy;
		spy = 0;
	} else if( s )
		spy = new Cslfd;
	display();
}

void Cell.relative(long a)
{
	trace( "%d.relative(%d)", this, a );	VOK;
	memory->makecell(this,addr+a*size);
}

void Cell.indirect()
{
	Cslfd *m;

	trace( "%d.indirect()", this );		VOK;
	if( !(m = memory->core->peek(addr,0)) )
	    display();			/* will have same problem */
	else
	    memory->makecell(this, size==1 ? (unsigned  char) m->chr
				 : size==2 ? (unsigned short) m->sht : m->lng);
}

char *Cell.kbd(char *s)	/* should eval against loader symbol frame */
{
	Parse y(G_DOLEQ_CONEX, 0);
	Expr *e;
	Bls t;

	trace( "%d.kbd(%s)", this, s );		OK("kbd");
	if( !*s ){
		relative(1);
		return 0;
	}
	if( (e = (Expr*)y.parse(s)) ){
		e->evaltext(memory->core->process()->globals, t);
		display(e->evalerr ? t.text
				   : memory->core->poke(addr,e->val.lng,size));
		return 0;
	}
	return memory->kbd(s);
}

void Cell.reformat(long f)
{
	trace( "%d.reformat(0x%X) 0x%X ", this, f, fmt ); VOK;
	fmt ^= f;
	if( f&F_TURNOFF )
		fmt &= ~f;
	else
		fmt |= f;
	display();
}

void Cell.resize(int s)
{
	trace( "%d.resize(%d) %d", this, s, size ); VOK;
	size = s;
	reformat(0);
}

void Cell.asmblr()
{
	trace( "%d.asmblr()" );	VOK;
	memory->core->process()->openasm(addr);
}

int Memory.changes(long verbose)
{
	Cell *c;
	long changes = 0, key = 0x40000000, spies = 0;	/* !!! */
	trace( "%d.changes()", this ); OK(0);

	pad->removeline(key);	
	for( c = cellset; c; c = c->sib )
		if( c->spy ){
			++spies;
			changes += c->changed();
		}
	if( verbose ){
		if( spies == 0 )
			pad->insert( key, SELECTLINE, "no spies" );
		else if( changes==0 )
			pad->insert( key, SELECTLINE, "no spies changed" );
	}
	return changes;
}	
