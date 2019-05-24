#include "mcc.h"
#include "symtab.pri"
#include "symbol.h"
#include "dtype.pri"
SRCFILE("mccsymtab.c")

DType MccSymTab.chain(int t, nlist *n)
{
	DType d;
	return d;
}

DType MccSymTab.gatherdtype(nlist *n)
{
	DType d;
	return d;
}

MccSymTab.MccSymTab(Core* c, int fd, SymTab *i, long r):(c, fd, i, r)
{
	hdr = new exec;
}

MccSymTab.~MccSymTab() { delete hdr; }

char *MccSymTab.gethdr()
{
	trace( "%d.getheader()", this );	OK( "MccSymTab.gethdr" );
	return 0;
}

Block *MccSymTab.gatherfunc(Func *func)
{
	return 0;
}

void MccSymTab.gathervar( nlist *n, Var **v, Block *b, UDisc d )
{
}

char *MccSymTab.gettbl()
{
	return 0;
}

Source *MccSymTab.tree()
{
	return 0;
}

Var *MccSymTab.gatherutype(UType *u)
{
	return 0;
}

nlist *MccSymTab.nlistvector(long start, long size )
{
	return 0;
}

