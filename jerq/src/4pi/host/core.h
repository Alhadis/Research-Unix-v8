#ifndef CORE_H
#define CORE_H
#ifndef UNIV_H
#include "univ.h"
#endif

enum Behavs {
	BREAKED		= 1,
	ERRORED		= 0,
	HALTED		= 2,
	ACTIVE		= 3,
	PENDING		= 4,
	STMT_STEPPED	= 5,
	INST_STEPPED	= 6,
};
char *BehavsName(Behavs);

>pri
union UCslfd {			/* lsb lo */
	 double	dbl;
	 float	flt;
	 long	lng;
	 short	sht;
	 char	chr;
};

#include <CC/sys/types.h>
#include <CC/sys/stat.h>
>
class Context {
PUBLIC(Context,U_CONTEXT)
	char	*error;
		Context()	{ error = "context save error"; }
virtual	void	restore();
};

#define PEEKFAIL ((Cslfd*)-1)
class Core : public PadRcv{
		friend TermCore; friend HostCore; friend KernCore; friend SnetCore;
		friend KernMaster; friend HostMaster; friend SnetMaster;
>pub
	char	pub_filler[68];
>pri
	struct stat	corestat;
	struct stat	stabstat;
	char	*behavs_problem;
>
	int	corefd;
	int	stabfd;
	int	_online;
	Process	*_process;
	SymTab	*_symtab;
virtual	char	*read(long,char*,int);
virtual	char	*write(long,char*,int);
	int	corefstat();
	int	stabfstat();
PUBLIC(Core,U_CORE)
		Core(Process*, Master*);
	Process *process();
	SymTab	*symtab();
	char	*procpath();
	char	*stabpath();

virtual	Context	*newContext();
virtual	char	*liftbpt(Trap*);
virtual	char	*laybpt(Trap*);
virtual	int	REG_PC();
virtual	int	REG_FP();
virtual	int	REG_SP();
virtual	int	REG_AP();
virtual	long	saved(Frame*,int,int=0);
virtual	Behavs	behavs();
virtual	Asm	*newAsm();
virtual	Cslfd	*peek(long,Cslfd* =PEEKFAIL);
virtual	Cslfd	*peekcode(long);
virtual	CallStk	*callstack();
virtual	Frame	frameabove(long);
virtual	char	*blockmove(long,long,long);
virtual	char	*special(char*,long);
virtual	char	*destroy();
virtual	char	*eventname();
virtual	char	*open();
virtual	char	*peekstring(long,char* = 0);
virtual	char	*poke(long,long,int);
virtual	char	*pokedbl(long,double,int);
virtual	char	*problem();
virtual	char	*readcontrol();
virtual	char	*regname(int);
virtual	char	*reopen(char*,char*);
virtual	char	*resources();
virtual	char	*run();
virtual	char	*step(long=0,long=0);
virtual	char	*stepprolog();
virtual	char	*stop();
virtual	char	*specialop(char*);
virtual	int	event();
virtual	int	online();
virtual	long	regloc(int,int=0);
virtual	void	close();
virtual	long	apforcall(int);
virtual	char	*docall(long,int);
virtual	long	returnregloc();

	char	*regpoke(int r,long l);
	long	regpeek(int r);
virtual	long	fp();
virtual	long	sp();
virtual	long	pc();
virtual	long	ap();
};

#endif
