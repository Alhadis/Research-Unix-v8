#ifndef MEMORY_H
#define MEMORY_H
#ifndef UNIV_H
#include "univ.h"
#endif
>pri
class Cell : public PadRcv {
	friend	Memory;
	friend	Process;
class	Memory	*memory;
	long	addr;
	int	fmt;
	int	size;
	Cell	*sib;
	Cslfd	*spy;

	char	*kbd(char*);
	char	*help()
			{ return "$=<expr> {update cell} | .=<expr> {open cell}"; }
	void	relative(long);
	void	indirect();
	void	reformat(long);
	void	resize(int);
	void	display(char *error = 0);
	void	asmblr();
	Index	carte();
	void	setspy(long);
	int	changed();
PUBLIC(Cell,U_CELL)
		Cell(Memory *m) { memory = m; }
};
>
class Memory : public PadRcv {
>pub
	char	pub_filler[12];
>pri
	friend	Cell;
class	Pad	*pad;
	Cell	*cellset;
	Cell	*current;
	void	makecell(Cell*,long);
>
class	Core	*core;
	char	*kbd(char*);
	char	*help()	{ return ".=<expr> {show memory cell at address}"; }
PUBLIC(Memory,U_MEMORY)
		Memory(class Core* c)		{ core = c; }
	void	open(long=0);
	void	userclose();
	void	banner();
	int	changes(long verbose=0);
};
#endif
