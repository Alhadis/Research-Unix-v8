struct frame {
	MLONG		fp;
	MLONG		pc;
	MLONG		retaddr;
	MLONG		function;
	MWORD		regmask;
	MLONG		regbase;
	int		regdelta;
	char		fname[16];
	struct ramnl	*sline;
	struct ramnl	*so;
	struct ramnl	*bfun;
	struct frame	*caller;
	struct frame	*callee;
	int		visible;
	int		noregs;
};

struct frame *train;

struct frame sentinel;

#define REGS_CONTEXT 1
#define LINE_CONTEXT 2
#define AUTO_CONTEXT 4
#define FULL_CONTEXT (AUTO_CONTEXT|REGS_CONTEXT|LINE_CONTEXT)

#define VERBOSE_CHAIN 1
#define MPX_CHAIN  2
#define FULL_CHAIN 4
