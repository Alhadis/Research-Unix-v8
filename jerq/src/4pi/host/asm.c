#include "asm.pri"
#include "vaxoptab.h"
#include "mac32optab.h"
#include "core.pub"
#include "format.pub"
#include "parse.h"
#include "expr.pub"
#include "frame.pub"
#include "process.pub"
#include "symtab.pub"
#include "symbol.h"
#include "bpts.pub"
SRCFILE("asm.c")

char *Instr.arg(int)			{ return "<arg>";	}
char *Instr.mnemonic()			{ return "<mnemonic>";	}
int Instr.argtype(int)			{ return 0;		}
int Instr.nargs()			{ return 0;		}

VaxInstr.VaxInstr(Asm*a,long l):(a,l)	{ tab = 0; display(); }
Mac32Instr.Mac32Instr(Asm*a,long l):(a,l) { display(); }
M68kInstr.M68kInstr(Asm*a,long l):(a,l) { display(); }

char *Asm.literaldelimiter()		{ return "<literal>"; }
Instr *Asm.newInstr(long l)		{ return (l,0); }

char *VaxAsm.literaldelimiter()		{ return "$"; }
Instr *VaxAsm.newInstr(long a)		{ return (Instr*) new VaxInstr(this,a); }
VaxAsm.VaxAsm(Core *c):(c)		{ }

char *Mac32Asm.literaldelimiter()	{ return "&"; }
Instr *Mac32Asm.newInstr(long a)	{ return (Instr*) new Mac32Instr(this,a); }
Instr *M68kAsm.newInstr(long a)		{ return (Instr*) new M68kInstr(this,a); }

Mac32Asm.Mac32Asm(Core *c):(c)		{ }
M68kAsm.M68kAsm(Core *c):(c)		{ }

Asm.Asm(Core *c)
{
	trace( "%d.Asm(%d)", this, c );		VOK;
	core = c;
	fmt = F_SYMBOLIC|F_HEX;
}

void Asm.userclose()
{
	trace( "%d.userclose()", this );	VOK;
	delete pad;
	pad = 0;
	for( ; instrset; instrset = instrset->sib )
		delete instrset;
}

void Asm.banner()
{
	trace( "%d.banner()", this );	VOK;
	if( pad ){
		pad->banner("Assembler: %s", core->procpath());
		pad->name("Asm %s", basename(core->procpath()));
	}
}

void Asm.open(long a)
{
	trace( "%d.open(%d)", this, a );	VOK;
	if( !pad ){
		Menu m;
		pad = new Pad( (PadRcv*) this );
		banner();
		m.last( "display pc",     (Action)&displaypc	 );
		if( core->online() ){
			m.first( "go",		  (Action)&go		 );
			m.last( "step  1 instr ", (Action)&instrstep,  1 );
			m.last( "step  4 instrs", (Action)&instrstep,  4 );
			m.last( "step 16 instrs", (Action)&instrstep, 16 );
			m.last( "step 64 instrs", (Action)&instrstep, 64 );
			m.last( "step over call", (Action)&stepover	 );
		}
		pad->menu(m);
	}
	pad->makecurrent();
	if( a ) newInstr(a);
}

void Asm.go()
{
	trace( "%d.go()", this ); VOK;
	core->process()->go();
}

void Asm.displaypc()
{
	trace( "%d.displaypc()", this );	VOK;
	open(core->pc());
}

char *Asm.help()
{
	trace( "%d.help()", this );
	return ".=<expr> {display instruction at address}";
}

char *Asm.kbd(char *s)
{
	Parse y(G_DOTEQ_CONEX,0);
	Expr *e;
	Bls error;

	trace( "%d.kbd(%s)", this, s );		OK("kbd");
	if( !(e = (Expr*)y.parse(s)) )
		return sf("%s: %s", y.error, s);
	e->evaltext(core->process()->globals, error);
	if( e->evalerr )
		return sf("%s: %s", s, error.text);
	newInstr(e->val.lng);
	return 0;
}

char *Instr.literal(long f)			/* is Format right? */
{
	static char t[128];

	trace( "%d.literal(0x%X) 0x%X %g", this, f, m.lng, m.flt ); OK("literal");
	sprintf( t, "%s%s", _asm->literaldelimiter(),
			Format(f&~F_SYMBOLIC).f(m.lng,m.dbl) );
	return t;
}

char *Instr.symbolic(char *prefix)		/* is Format right? */
{
	static char t[128];

	trace( "%d.symbolic(%s) 0x%X", this, prefix, m.lng ); OK("symbolic");
	strcpy(t, prefix);
	strcat(t, Format(fmt, _asm->core->symtab()).f(m.lng));
	return t;
}

char *Instr.local(UDisc d, long a)		/* VAX */
{
	Func *f;
	Block *b;
	static char t[300];
	int found = 0;

	trace( "%d.local(0x%X,%d)", this, d, a ); OK( "Instr.local" );
	if( !(f = (Func*)_asm->core->symtab()->loctosym(U_FUNC, addr)) ) return 0;
	if( !(b = f->blk(addr)) ) return 0;
	strcpy( t, "" );
	{
		BlkVars bv(b);
		Var *v;

		while( v = bv.gen() )
			if( v->disc() == d && v->range.lo == a ){
				if( found ) strcat( t, "=" );
				strcat( t, v->text() );
				found = 1;
				t[260] = '\0';
			}
	}
	return found ? t : 0;
}

char *Instr.regarg( char *lay, long f )		/* is Format right? */
{
	static char t[128];
	char *o, *r, *id;

	trace("%d.regarg(%s,0x%X) %d 0x%X", this, lay, f, reg, m.lng); OK("regarg");
	r = _asm->core->regname(reg);
	o = Format(f&~F_SYMBOLIC).f(m.lng);
	if( f&F_SYMBOLIC ){
		if( reg == _asm->core->REG_AP() ){
			if( id = local(U_ARG, m.lng) )
				o = id;
		} else if( reg == _asm->core->REG_FP() ){
			if( id = local(U_AUT, m.lng) )
				o = id;
		} else {
			if( id = local(U_REG, reg) )
				r = sf( "%s=%s", id, r );
		}
	}
	sprintf( t, lay, o, r );
	return t;
}

char *VaxInstr.arg(int i)
{
	long modereg, mode, type;

	trace( "%d.Arg(0x%X)", this, argtype ); OK("Instr.arg");
	type = argtype(i)&7;
	if( argtype(i)&ACCB )
		modereg = 0xAF + (type<<5);			/* branch disp */
	else
		modereg = (unsigned char) _asm->core->peekcode(next++)->chr;
	mode = modereg>>4;
	reg = modereg&0xF;
	m = *_asm->core->peekcode(next);
	trace( "argtype=0x%X mode=0x%X reg=0x%X", argtype(i), mode, reg );
	switch( modereg ){
	case 0x8F:						/* immed  */
	    	switch(type){
		case TYPB: next += 1; return literal( F_MASKEXT8|fmt );
        	case TYPW: next += 2; return literal( F_MASKEXT16|fmt );
    		case TYPL: next += 4; return literal( fmt );
 		case TYPQ: next += 8; return literal( F_DBLHEX );
 		case TYPF: next += 4; return literal( F_FLOAT );
 		case TYPD: next += 8; return literal( F_DOUBLE );
		default:	      return sf( "<0x8F:0x%X>", type );
		}
	case 0x9F: next+=4;		return symbolic("*");
	case 0xAF: m = m.chr+(next+=1); return symbolic("" );
	case 0xBF: m = m.chr+(next+=1); return symbolic("*");
	case 0xCF: m = m.sht+(next+=2); return symbolic("" );
	case 0xDF: m = m.sht+(next+=2); return symbolic("*");
	case 0xEF: m = m.lng+(next+=4); return symbolic("" );
	case 0xFF: m = m.lng+(next+=4); return symbolic("*");
	}
	switch( mode ){
	case 0:
	case 1:
	case 2:
	case 3:
		if( type==TYPF || type==TYPD ) {
			m = (modereg<<4)|0x4000; return literal( F_FLOAT );
		} else {
			m = modereg; return literal( fmt );
		}
	case 4:	char *ix = sf(
			regarg( "%s[%s]",	fmt&F_SYMBOLIC|F_NONE ) );
		return sf( "%s%s", arg(i), ix );
	case 5:   return regarg( "%s%s",	fmt&F_SYMBOLIC|F_NONE );
	case 6:   return regarg( "%s(%s)",	fmt&F_SYMBOLIC|F_NONE );
	case 7:   return regarg( "%s-(%s)",	fmt&F_SYMBOLIC|F_NONE );
	case 8:   return regarg( "%s(%s)+",	fmt&F_SYMBOLIC|F_NONE );
	case 9:   return regarg( "%s*(%s)+",	fmt&F_SYMBOLIC|F_NONE );
	case 0xA: next+=1; m = m.chr; return regarg( "%s(%s)",	fmt|F_MASKEXT8	);
	case 0xB: next+=1; m = m.chr; return regarg( "*%s(%s)",	fmt|F_MASKEXT8	);
	case 0xC: next+=2; m = m.sht; return regarg( "%s(%s)",	fmt|F_MASKEXT16	);
	case 0xD: next+=2; m = m.sht; return regarg( "*%s(%s)",	fmt|F_MASKEXT16	);
	case 0xE: next+=4;	      return regarg( "%s(%s)",	fmt		);
	case 0xF: next+=4;	      return regarg( "*%s(%s)",	fmt		);
	}
	return sf( "modereg=0x%X", modereg );
} 

char *Mac32Instr.extend(int tttt)
{
	switch( tttt ){
		case 0: return "uword";
		case 2: return "uhalf";
		case 3: return "ubyte";
		case 4: return "sword";
		case 6: return "shalf";
		case 7: return "sbyte";
	}
	return sf( "tttt=%d", tttt );
}

char *Mac32Instr.macro()
{
	switch((unsigned char) m.chr){
		case 0x09:  return "mverno";
		case 0x0D:  return "enbvjmp";
		case 0x13:  return "disvjmp";
		case 0x19:  return "movblw";
		case 0x1F:  return "strend";
		case 0x2F:  return "intack";
		case 0x35:  return "strcpy";
		case 0x3C:  return "slftst";
		case 0x45:  return "retg";
		case 0x61:  return "gate";
		case 0xAC:  return "callps";
		case 0xC8:  return "retps";
	}
	return sf( "<macro 0x%X>", (unsigned char) m.chr );
}

int Mac32Instr.argtype(int)
{
	return Mac32OpTab[opcode&0xFF].type;
}

int VaxInstr.argtype(int i)
{
	return tab->argtype[i];
}

int Mac32Instr.nargs()
{
	switch( argtype(0) ){
		case BSBB:
		case BSBH:
		case EXT:
		case JUMP1:
		case JUMP2:
		case JUMP:
		case JUMPSB:
		case MACRO:
		case NOOP16:
		case NOOP8:
		case OPRNDS1:
				return 1;
		case OPRNDS2:	return 2;
		case OPRNDS3:	return 3;
		case OPRNDS4:	return 4;

		case CALL:			/* these don't matter? */
		case AD1OP3:
		case SFPOPS2:
		case SFPOPS3:
		case DFPOPS2:
		case DFPOPS3:
		case SPRTOP0:
		case SPRTOP1:
		case SPRTOP2:
				break;
	}
	return 0;
}

char *VaxInstr.mnemonic()
{
	if( !tab ){
		for( tab = VaxOpTab; tab->opname; ++tab )
			if( tab->opcode == opcode ) break;
	}
	return tab ? tab->opname : Instr::mnemonic();
}

char *Mac32Instr.mnemonic()
{
	char *p = Mac32OpTab[opcode&0xFF].mnem, *q;
	if( !p ) return "<mac32_mnem>";
	for( q = p; *q; *q++ |= ' ' ) {}
	return p;
}

int VaxInstr.nargs()
{
	return tab->nargs;
}

char *Mac32Instr.arg(int i)
{
	unsigned char modereg, mode;

	trace( "%d.Arg(0x%X)", this ); OK("Mac32Instr.arg");
	m = *_asm->core->peekcode(next++);
	switch(argtype(i)){
	case EXT:	return literal(F_MASKEXT8|fmt);
	case NOOP8:	return "";
	case NOOP16:	next += 1; return "";
	case MACRO:	return macro();
	case JUMP1: 	m = addr+m.chr; return symbolic();
	case JUMP2: 	next += 1; m = addr+m.sht; return symbolic();
	}
	modereg = m.chr;
	mode = modereg>>4;
	reg = modereg&0xF;
	m = *_asm->core->peekcode(next);
	switch( modereg ){
/*abs*/
	case 0x7F:	next += 4; return symbolic();
	case 0xEF:	next += 4; return symbolic("*");
/*imm*/
	case 0x4F:	next += 4; return literal(fmt);
	case 0x5F:	next += 2; return literal(F_MASKEXT16|fmt);
	case 0x6F:	next += 1; return literal(F_MASKEXT8|fmt);
	}

	switch( mode ){	
/*pos*/
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:	m = modereg;        return literal(F_MASKEXT8|fmt);
/*neg*/
	case 0xF:	m = (char) modereg; return literal(F_MASKEXT8|fmt);
/*reg*/
	case 0x4:	 return regarg( "%s%s",	  fmt&F_SYMBOLIC|F_NONE );
	case 0x5: m = 0; return regarg( "%s(%s)", fmt&F_SYMBOLIC	);
/*disp*/
	case 0xC: next+=1; m = m.chr; return regarg("%s(%s)",	fmt|F_MASKEXT8 );
	case 0xD: next+=1; m = m.chr; return regarg("*%s(%s)",	fmt|F_MASKEXT8 );
	case 0xA: next+=2; m = m.sht; return regarg("%s(%s)",	fmt|F_MASKEXT16);
	case 0xB: next+=2; m = m.sht; return regarg("*%s(%s)",	fmt|F_MASKEXT16);
	case 0x8: next+=4;	      return regarg("%s(%s)",	fmt	       );
	case 0x9: next+=4;	      return regarg("*%s(%s)",	fmt	       );
/*offset*/
	case 0x6:
	case 0x7:
		  m = reg; reg = mode+3; return regarg("%s(%s)", fmt|F_MASKEXT8);
/*expanded*/
	case 0xE: char *e = extend(reg); return sf( "{%s}%s", e, arg(i) );
	}
	return sf( "modereg=0x%X", modereg );
} 

void Instr.dobpt(int setorclr)
{
	trace( "%d.dobpt(%d)", this, setorclr ); VOK;
	Stmt *stmt = new Stmt(0,0,0);
	stmt->range.lo = addr;
	stmt->process = _asm->core->process();
	stmt->dobpt(setorclr);
}

Instr.Instr(Asm *a, long l)
{
	trace( "%d.Instr(%d,%d)", this, a, l );	VOK;
	if( !l ) return;
	addr = l;
	_asm = a;
	sib = _asm->instrset;
	_asm->instrset = this;
	_asm->banner();					/* why here? */
	fmt = _asm->fmt;
	bpt = _asm->core->process()->bpts()->isasmbpt(addr);
}

void Instr.display()
{
	int	i;
	Bls	t;

	trace( "%d.display()", this );	VOK;
	if( !addr ) return;
	t.af( "%s%s", bpt?">>>":"", Format(fmt,_asm->core->symtab()).f(addr) );
	opcode = _asm->core->peekcode(addr)->chr;
	next = addr+1;
	if( mnemonic() ){
		t.af( ": %s ", mnemonic() );
		for( i = 0; i < nargs(); ++i )
			t.af( "%s%s", i?",":"", arg(i) );
	}
	_asm->pad->insert( addr, SELECTLINE, (PadRcv*)this, carte(), t.text );
}

void Instr.showsrc()
{
	trace( "%d.showsrc()", this );	ok();
	Stmt *stmt = (Stmt*) _asm->core->symtab()->loctosym(U_STMT, addr);
	if( stmt ) stmt->select();
}

long AF[] = { F_OCTAL, F_SIGNED, F_HEX, F_SYMBOLIC, 0 };

Index Instr.carte()
{
	Menu m, f;

	trace( "%d.carte()", this );	ok();
	if( _asm->core->online() ){
		if( bpt )
			m.last( "clr bpt", (Action)&dobpt, 0 );
		else
			m.last( "set bpt", (Action)&dobpt, 1 );
	}
	if( _asm->core->symtab()->loctosym(U_STMT, addr) )
		m.last( "src text", (Action)&showsrc, 0 );
	m.last( "open frame",	(Action)&openframe );
	m.last( "next  1",	(Action)&succ,		1 );
	m.last( "next  5",	(Action)&succ,		5 );
	m.last( "next 10",	(Action)&succ,		10 );
	for( int i = 0; AF[i]; ++i ){
		long b = AF[i];
		if( fmt&b ) b |= F_TURNOFF;
		f.last(FmtName(b), (Action)&reformat, b);
	}
	m.last(f.index("format"));
	m.last( "refresh",	(Action)&succ,		-1 );
	m.last( "raw mem",	(Action)&memory,	0 );
	return m.index();
}

void Instr.openframe()
{
	trace( "%d.openframe()", this ); VOK;
	_asm->core->process()->openframe(addr);
}

void Instr.reformat(int f)
{
	trace( "%d.reformat(0x%X) 0x%X", this, f, fmt ); VOK;
	if( f&F_TURNOFF)
		fmt &= ~f;
	else
		fmt |= f;
	if( !fmt ) fmt = F_HEX;
	_asm->fmt = fmt;
	_asm->newInstr(addr);
}

void Instr.succ(int n)		/* think about it! */
{
	trace( "%d.succ(%d)", this, n );	VOK;
	_asm->fmt = fmt;
	if( n>0 ) _asm->newInstr(next)->succ(n-1);
	else if( n<0 ) _asm->newInstr(addr);
}

void Instr.memory()
{
	trace( "%d.memory()" ); VOK;
	_asm->core->process()->openmemory(addr);
}

void Asm.instrstep(long i)
{
	trace( "%d.instrstep(%d)", this, i );	VOK;
	core->process()->instrstep(i);
}

void Asm.stepover()
{
	trace( "%d.stepover()", this );	VOK;
	core->process()->
		stepover( core->pc(), newInstr(core->pc())->next );
}
