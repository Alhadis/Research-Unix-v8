/* M68000 .l .w .b for VAX host */

typedef long	MLONG;
typedef short	MWORD;
typedef char	MBYTE;

#include <stdio.h>
#include <ctype.h>
#include "../protocol.h"

MLONG peekword(), peekbyte(), peeklong(), longfromjerq();
void  pokeword(), pokebyte(), pokelong(), jerqdo(), addr_desc();
char  *jerqkbd(), *peeknstr();
unsigned char ofetch();
float f68ktovax();

char *fmtstring(), *fmtint(), *fmtchar(), *fmtbyte(), *fmtuns(),
	*display(), *doh(), *fmtdot(), *fmtxref(), *graphopnames();

char *calloc(), *talloc(), *fmt(), *fmtreturn();

struct ramnl *lookup(), *visible(), *fileline();
char *sofile(), *soline(), *basename(), *srcline();

char *strcpy(), *strcat(), *strncpy(), *strncat(), *strncmp();

struct frame *framechain();

char *readuser();

MLONG locate();

char *mcctype();

#define assert(e) { if( !(e) ) { wrap(__LINE__); } }

int clean, oflag, hatebs;

#define flush() fflush( stdout )

setbpt(), clrbpt();
int quitc(), newc(), breakptmenu(), goc(), haltc(), stmtstepc();
int localsc(), stkframec(), tracebackc(), globalsc();


typedef struct Menu{
	char	**item;		/* string array, ending with 0 */
} Menu;

