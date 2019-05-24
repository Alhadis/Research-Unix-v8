#
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"

char	line[LINSIZ];
int	infile;
char	*lp;
char	peekc,lastc = EOR;
int	eof;

/* input routines */

eol(c)
char	c;
{
	return(c==EOR || c==';');
}

int
rdc()
{
	do {
		readchar();
	} while (lastc==SP || lastc==TB);
	return(lastc);
}

reread()
{
	peekc = lastc;
}

clrinp()
{

	lp = 0;
	peekc = 0;
}

int
readchar()
{
	if (eof)
		lastc=0;
	else if (peekc) {
		lastc = peekc;
		peekc = 0;
	}
	else {
		if (lp==0) {
			lp=line;
			do {
				eof = read(infile,lp,1)==0;
				if (mkfault)
					error(0);
			} while (eof==0 && *lp++!=EOR);
			*lp=0;
			lp=line;
		}
		if ((lastc = *lp) != 0)
			lp++;
	}
	return(lastc);
}

nextchar()
{
	if (eol(rdc())) {
		reread();
		return(0);
	}
	return(lastc);
}

quotchar()
{
	if (readchar()=='\\')
		return(readchar());
	else if (lastc=='\'')
		return(0);
	else
		return(lastc);
}

getformat(deformat)
char *deformat;
{
	register char *fptr;
	register BOOL	quote;

	fptr=deformat;
	quote=FALSE;
	while ((quote ? readchar()!=EOR : !eol(readchar())))
		if ((*fptr++ = lastc)=='"')
			quote = ~quote;
	lp--;
	if (fptr!=deformat)
		*fptr = '\0';
}
