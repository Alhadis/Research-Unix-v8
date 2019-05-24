#ifndef BPTS_H
#define BPTS_H
#ifndef UNIV_H
#include "univ.h"
#endif

>pri
class Core;

enum LiftLay { LIFT, LAY };

class Trap : PadRcv { friend Bpts; friend TermCore; friend HostCore;
	long	key;
	char	saved;
class	Stmt	*stmt;
	char	*error;
	Trap	*sib;
	char	*liftorlay(LiftLay,Core*);
PUBLIC(Trap,U_TRAP)
		Trap(Stmt*, Trap *);
};
>
class Bpts : public PadRcv {
>pub
	char	pub_filler[16];
>pri
	friend	HostProcess; friend Process;
	Pad	*pad;
	Core	*core;
	Trap	*trap;
	int	layed;
	Trap	*istrap(Stmt*);
	void	select(Trap*);
	void	clearall();
	void	refresh();
>
	void	liftparents(Bpts *);
PUBLIC(Bpts,U_BPTS)
		Bpts(class Core *);
	void	lift();
	void	lay();
	void	set(class Stmt*);
	void	clr(Stmt*);
	int	isbpt(Stmt*);
	int	isasmbpt(long);
	Stmt	*bptstmt(long);
	void	hostclose();
	void	banner();
};
#endif
