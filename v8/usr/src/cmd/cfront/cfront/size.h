/* @(#) size.h 1.6 3/7/85 08:45:33 */
/*	used in typ.c type.sizeof() for implementing sizeof */

extern BI_IN_WORD;
extern BI_IN_BYTE;
				/*	byte sizes */
extern SZ_CHAR;
extern AL_CHAR;

extern SZ_SHORT;
extern AL_SHORT;

extern SZ_INT;
extern AL_INT;

extern SZ_LONG;
extern AL_LONG;

extern SZ_FLOAT;
extern AL_FLOAT;

extern SZ_DOUBLE;
extern AL_DOUBLE;

extern SZ_STRUCT;	/* minimum struct size */
extern AL_STRUCT;

extern SZ_FRAME;
extern AL_FRAME;

extern SZ_WORD;

extern SZ_WPTR;
extern AL_WPTR;

extern SZ_BPTR;
extern AL_BPTR;		/*	space at top and bottom of stack frame
					(for registers, return ptr, etc.)
				*/
extern SZ_TOP;
extern SZ_BOTTOM;

extern char* LARGEST_INT;

			/* 	table sizes */
#define KTBLSIZE	123
#define GTBLSIZE	257
				/*	initial class table size */
#define CTBLSIZE	12
				/*	initial block table size */
#define TBLSIZE		20

#define BLMAX		50	/*	max block nesting */
#define TBUFSZ		48*1024	/*	(lex) input buffer size */
#define MAXFILE		127	/*	max include file nesting */

#define MAXERR		20	/* maximum number of errors before terminating */
