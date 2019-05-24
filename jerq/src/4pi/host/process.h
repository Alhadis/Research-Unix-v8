#ifndef PROCESS_H
#define PROCESS_H
#ifndef UNIV_H
#include "univ.h"
#endif
#include "format.h"

class Process : public PadRcv {
	friend TermProcess; friend HostMaster; friend Master;	
	friend HostProcess; friend SigMask; friend SrcDir;
	friend KernProcess; friend KernMaster;
	friend SnetProcess; friend BatchProcess;
	friend KernCore;

	int	isdead;
	Asm	*_asm;
	Bpts	*_bpts;
	Core	*core;
	CallStk	*callstk;
	Memory	*memory;
	Pad	*pad;
	Process	*sibling;
	Master	*master;
	Process *parent;
	SrcDir	*srcdir;
	char	*_prefix;
	Behavs	prev_behavs;
	char	stoprequest;
	char	cycles;
>pub
	char	pub_filler[256*3];
>pri
#define BEHAVSKEY 1
#define ERRORKEY 2
	Bls	bls[3];			/* error [1] behavs [2] */
	void	openbpts();
	void	mergeback(long);
	void	srcfiles();
	void	habeascorpus(Behavs,long);
	void	banner();
	int	changes();
	void	docycle();
	void	insert(long,PRINTF_TYPES);
	void	closeframes();
	void	merge();
>
	void	cycle();
virtual	Index	carte();
	void	linereq(long,Attrib=0);
PUBLIC(Process,U_PROCESS)
	Globals	*globals;
		Process(Process* =0, char* =0, char* =0, char* =0);
	char	*procpath;
	char	*stabpath;
	char	*comment;
	char	*prefix(char*);
	void	openglobals(Menu* =0);
	void	openmemory(long=0);
	void	openasm(long=0);
	Frame	*frame(long);
	SymTab	*symtab();
	Bpts	*bpts();
	void	go();
	void	stmtstep(long);
	void	stepinto();
	void	instrstep(long);
	void	stepover(long,long);
	void	openframe(long,char* =0);
	void	openpad();
	void	currentstmt();

virtual	void	userclose();
virtual	void	stop();
};
>pri
class HostProcess : public Process { friend HostCore;
	Index	carte();
	SigMask	*sigmask;
	void	opensigmask();
	void	hang();
	void	hangopen();
	void	hangtakeover();
	void	userclose();
	void	reopendump();
	void	substitute(HostProcess*);
	void	takeover();
	int	accept(Action);
	void	imprint();
	void	destroy();
	HostCore
		*hostcore()		{ return (HostCore*) core; }
public:
		HostProcess(Process*sib=0, char*p=0, char*s =0, char*c=0)
			:(sib,p,s,c) {}
	void	open(long ischild=0);
	void	stop();
};

class BatchProcess : public HostProcess {
public:
		BatchProcess(char*p, char*s):(0,p,s,0) {}
	void	open();
};

class KernProcess : public Process { friend KernCore;
	Index	carte();
	void	userclose();
	KernCore
		*kerncore()		{ return (KernCore*) core; }
	char	*kbd(char*);
	char	*help();
public:
		KernProcess(Process*sib=0, char*p=0, char*s =0, char*c=0)
			:(sib,p,s,c) {}
	void	open();
};

class TermProcess : public Process {
	Index	carte();
public:
	void	open();
		TermProcess(Process*sib=0, char*p=0, char*s =0, char*c=0)
			:(sib,p,s,c) {}
};

class SnetProcess : public Process {
	Index	carte();
public:
	void	open();
		SnetProcess(Process*sib=0, char*p=0, char*s =0, char*c=0)
			:(sib,p,s,c) {}
};
>
#endif
