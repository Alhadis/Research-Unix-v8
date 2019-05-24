#include "../protocol.h"
#include "core.pri"

void CoreVersion(Remote*,char* ="");

class SnetCore : public Core { friend SnetContext;
	long	P;
	long	P_state;
	long	P_pc;
	long	P_psw;
	long	relocate;
	Remote	*remote;
	char	*devtty;
	Frame	*cache;
	short	lo;
	short	hi;
	long	argarea;
	long	retloc;

	long	rcvlong();
	short	rcvshort();
unsigned char	rcvuchar();
	void	sendlong(long x);
	void	sendshort(short x);
	void	senduchar(unsigned char x);
	char	*rcvstring(char*);
	void	sendstring(char*);
	void	pktstart(char c);
	void	pktend();
	void	pktflush();
	char	*rcverror(char* = "");

	char	*read(long,char*,int);
	char	*write(long,char*,int);
	void	freeargarea();
public:
		SnetCore(Process*,SnetMaster*);
	Asm	*newAsm();
	Context	*newContext();
	Behavs	behavs();
	Cslfd	*peek(long,Cslfd* = 0);
	CallStk	*callstack();
	char	*eventname();
	char	*laybpt(Trap*);
	char	*liftbpt(Trap*);
	char	*open();
	char	*peekstring(long,char* =0);
	char	*poke(long,long,int);
	char	*problem();
	char	*readcontrol();
	char	*regname(int);
	char	*resources();
	char	*run();
	char	*step(long=0,long=0);
	char	*stop();
	int	REG_FP();
	int	REG_PC();
	int	REG_SP();
	long	regloc(int,int=0);
	long	pc();
	long	saved(Frame*,int,int=0);
	long	apforcall(int);
	char	*docall(long,int);
	long	returnregloc();
};

class SnetContext : public Context { friend SnetCore;
	SnetCore
		*core;
public:
		SnetContext()		{}
};
