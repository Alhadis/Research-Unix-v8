#include "snetcore.h"
#include "symtab.pri"
#include "process.pub"		/* needed because of stabpath */
#include "master.pri"		/* remote */
#include "frame.pri"
#include "symbol.h"
#include "asm.pri"
#include "format.pub"
#include "bpts.pri"
SRCFILE("snetcore.c")

int SnetCore.REG_FP()			{ return  9; }
int SnetCore.REG_PC()			{ return 15; }
int SnetCore.REG_SP()			{ return 12; }

char *SnetCore.step(long lo, long hi)
{
	return "step";
}

char *SnetCore.resources()
{
	return "resources";
}

long SnetCore.regloc(int r, int size)
{
	return 0;
}

long SnetCore.saved(Frame *f, int r, int size)	/* search symtab, then RAM */
{
	return 0;
}

CallStk *SnetCore.callstack()
{
	return 0;
}

char *SnetCore.rcverror(char *s)
{
	char error[256];
	return rcvstring(error)[0] ? sf("%s %s", s, error) : 0;
}

char *SnetCore.laybpt(Trap *t)
{
	return "laybpt";
}

char *SnetCore.liftbpt(Trap *t)
{
	return "liftbpt";
}

char *SnetCore.run()
{
	return "run";
}

char *SnetCore.stop()
{
	return "stop";
}

char *SnetCore.regname(int r)
{
	return sf("r%d", r);
}

char *SnetCore.eventname()
{
	return "eventname";
}

Behavs SnetCore.behavs()
{
	return ERRORED;
}

char *SnetCore.problem()
{
	return Core::problem();
}

SnetCore.SnetCore(Process *p, SnetMaster *m):(p,m)
{
	remote = m->remote;
	_online = 1;
	argarea = 0;
}

long SnetCore.rcvlong()				{ return remote->rcvlong();	}
short SnetCore.rcvshort()			{ return remote->rcvshort();	}
unsigned char SnetCore.rcvuchar()		{ return remote->rcvuchar();	}
char *SnetCore.rcvstring(char *x)		{ return remote->rcvstring(x);	}

void SnetCore.sendlong(long  x)			{ remote->sendlong(x);		}
void SnetCore.sendshort(short x)		{ remote->sendshort(x); 	}
void SnetCore.senduchar(unsigned char x)	{ remote->senduchar(x); 	}
void SnetCore.sendstring(char *x)		{ remote->sendstring(x);	}

void SnetCore.pktstart(char c)			{ remote->pktstart(c);		}
void SnetCore.pktend()				{ remote->pktend();		}
void SnetCore.pktflush()			{ remote->pktflush();		}

char *SnetCore.open()
{
	char *error;

	trace("%d.open() %d %s", this, _process, procpath()); OK("Core.open");
	P = 0;
	if( procpath() ){
		if( sscanf(procpath(), "Proc%d", &P) != 1 || !P )
			return "invalid process name - should be ProcN";
	}
	relocate = 0;
/*
	if( P ){
		if( error = readcontrol() )
			return error;
	}
	if( stabpath() )
		stabfd = ::open(stabpath(), 0);
	_symtab = (SymTab*) new MccSymTab(this, stabfd, _symtab, relocate);
	_symtab->read();
*/
	return 0;
}

long SnetCore.pc()
{
	return 0;
}

char *SnetCore.readcontrol()
{
	return "readcontrol";
}

char *SnetCore.read(long loc, char *buf, int r)
{
	trace( "%d.read(%d,%d,%d)", this, loc, buf, r ); OK("SnetCore.read");
	pktstart(C_READ);
	sendlong(loc);
	sendlong(r);
	pktflush();
	while(r-->0) *buf++ = rcvuchar();
	return rcverror("read : ");
}

char *SnetCore.write(long loc, char *buf, int w)
{
	trace( "%d.write(%d,%d,%d)", this, loc, buf, w ); OK("SnetCore.write");
	pktstart(C_WRITE);
	sendlong(loc);
	sendlong(w);
	while(w-->0) senduchar(*buf++);
	pktflush();
	return rcverror("write : ");
}

#define CYCLE 4
Cslfd *SnetCore.peek(long loc, Cslfd *fail)	/* 68000 no floats */
{
	static i;
	static Cslfd *cycle;

	trace( "%d.peek(0x%X,%d)", this, loc, fail ); OK(fail);
	if( !cycle ) cycle = new Cslfd[CYCLE];
	unsigned char raw[4];
	Cslfd c;
	if( read(loc, raw, 4) )
		return Core::peek(loc, fail);
	c.chr =  raw[0];
	c.sht = (raw[0]<<8 ) |  raw[1];
	c.lng =	(raw[0]<<24) | (raw[1]<<16) | (raw[2]<<8) | raw[3];
	c.dbl = c.flt = 0.0;
	c.flterr = "float undefined";
	cycle[(i++, i%=CYCLE)] = c;
	return cycle+i;
}

Asm *SnetCore.newAsm()	{ return new M68kAsm(this); }

char *SnetCore.poke(long loc, register long l, int n)
{
	trace( "%d.poke(0x%X,0x%X,%d)", this, loc, l, n ); OK("SnetCore.poke");
	char raw[4];
	register char *r = raw+4;
	while( r >= raw ){
		*--r = l;
		l >>= 8;
	}
	return write( loc, raw+4-n, n );
}

char *SnetCore.peekstring(long loc, char *fail)
{
	static char buf[260];
	trace( "%d.peekstring(0x%X,%s)", this, loc, fail?fail:"0" ); OK(fail);
	pktstart(C_STRING);
	sendlong(loc);
	pktflush();
	rcvstring(buf);
	char error[256];
	return !rcvstring(error)[0]
		? buf
		: fail ? fail : sf( "string error: %s", error );
}

Context* SnetCore.newContext()
{
	return 0;
}

long SnetCore.apforcall(int bytes)
{
	return 0;
}

char *SnetCore.docall(long addr, int)
{
	trace( "%d.docall(0x%X)", this, addr ); OK("docall");
	if( !addr || !argarea )
		return "SnetCore.docall() error";
	pktstart(CF_CALL);
	sendlong(addr);
	sendlong(argarea);
	pktflush();
	retloc = rcvlong();
	return rcverror();
}

long SnetCore.returnregloc()
{
	trace( "%d.returnregloc()", this ); OK(0);
	return retloc;
}
