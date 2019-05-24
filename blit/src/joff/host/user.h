#include "stab.h"

#define BYTEDOT   1
#define	WORDDOT   2
#define	LONGDOT   3
#define	INSTDOT   4
#define	STRUCTDOT 5
#define	ENUMDOT   6
#define	STRINGDOT 7

#define PLUSONE  8
#define MINUSONE 9
#define INDIRECT 10
#define XREF	 11
#define DECIMAL	 12
#define HEX	 13
#define OCTAL	 14
#define ASCII	 15
#define SNARF	 16
#define DOTOPS	 16


int dotty;
MLONG dot;
dotfmt;
radix;
MLONG snarfdot;
snarfdotty;
int dotstrlen;

struct ramnl dotstab[2];

char userline[128];
int lexindex;
