#include "mfile2.h"
#include "setjmp.h"
extern jmp_buf back;
extern char *regnames[], *frameptr, *argptr;
#ifdef M32
#define REGMASK 0x3
#define REGVAR	3
#define UP ('A' - 'a')
#define VAX 0
#else if VAX
#define REGMASK 0x3f
#define REGVAR	6
#define M32 0
#endif
/* these would be 3 and 3 respectively on the mac32, 3f and 6 on the vax */
/* registers 0..REGVAR-1 are scratch */
typedef struct {
	unsigned char ans;
	unsigned char regmask;
	unsigned short flag;
} ret;
#define CC	1
#define VALUE	2
#define TOSTACK	4
#define ASADDR	8
#define SCRATCH	16
#define ICON0	(VAX? 32: 0)
#define ICON1	(VAX? 64: 0)
#define ISREG	128
#define DESTISLEFT	256
#define CANINDIR	512
#define FAILX	1024
#define INDEX	(VAX? 2048: 0)
#define FAIL0	4096
#define FAIL (FAILX|FAIL0)
#define USED	(VAX?8192:0)
#define NOGOOD	(FAIL|USED)
#define AUTO	(16384)	/* ans is auto incr/decr */
/* as param; as return
 * CC just need the condition codes; condition codes are valid
 * VALUE need the value (for incr/decr.  as opposed to just the side effect)
 * TOSTACK put the result on the stack
 * ASADDR as an address, for various weird environments (calls, extzv, ...)
 * SCRATCH ; the result is in a scratch place
 * ICON0 ; the result is the constant 0
 * ICON1 ; the result is the constant 1
 * ISREG ; the result is in a register
 * DESTISLEFT put the result in the left operand (asg ops)
 * CANINDIR ; the result can be indirected through
 * FAIL ; there is no result, for we ran out of registers
 * INDEX ; some size-dependent addressing mode was used, so watch it
 * FAIL0 ; we ran out of register 0
 * USED plan to use result; don't re-use, as in 3+(*p++=*q++), register p, q
/* ASADDR is never used with a dest, and is always VALUE|ASADDR */
ret doit(), allocreg(), specialreg(), indir(), tostack(), indirit(), simpler(),
	alloctmp(), checksize();

#define LEFT 1
#define RIGHT 2
char *genjmp();

#define BUF 64	/* overflow unchecked */
char bufs[256][BUF];
char *buf, *bufend;
char prbuf[10240];
char *prptr;
#define done(s, n, rm) s.ans = (buf - bufs[0])/BUF; buf += BUF; s.flag = n; s.regmask = rm; if(buf >= bufend) cerror("buf ov"); return(s)
#define str(s)	bufs[s.ans]

NODE *copytree(), *gimmenode(), *convnodol();
extern int ntree, regvar, Pflag, bbcount;
