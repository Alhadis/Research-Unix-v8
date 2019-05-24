#
/*
 *
 *	UNIX debugger
 *
 */

#include "defs.h"
#include <a.out.h>

MSG	BADMOD;
MSG	ADWRAP;

char	lastc, peekc;

scanform(icount,ifp,itype,ptype)
WORD	icount;
char	*ifp;
{
	register char	*fp;
	register char	c;
	register int	fcount;
	ADDR	savdot;

	while (icount) {
		fp=ifp;
		savdot=dot;
		/*now loop over format*/
		while (*fp && errflg==0) {
			if (!isdigit(*fp))
				fcount = 1;
			else {
				fcount = 0;
				while (isdigit(c = *fp++)) {
					fcount *= 10;
					fcount += c-'0';
				}
				fp--;
			}
			if (*fp==0)
				break;
			fp=exform(fcount,fp,itype,ptype);
		}
		dotinc=dot-savdot;
		dot=savdot;
		if (errflg) {
			if (icount<0) {
				errflg=0;
				break;
			}
			else
				error(errflg);
		}
		if (--icount)
			dot=inkdot(dotinc);
		if (mkfault)
			error(NULL);
	}
}

char *
exform(fcount,ifp,itype,ptype)
int fcount;
char *ifp;
{
	/* execute single format item `fcount' times
	 * sets `dotinc' and moves `dot'
	 * returns address of next format item
	 */
	register WORD	w;
	ADDR	savdot;
	register char	*fp;
	register char	c, modifier;
	union {
		TFLOAT s;
		TDOUBLE d;
	} fl;

	while (fcount > 0) {
		fp = ifp;
		c = *fp;
		modifier = *fp++;
		if (charpos()==0 && modifier!='a')
			printf("%16m");
		switch(modifier) {

		case SP:
		case TB:
			break;

		case 't':
		case 'T':
			printf("%T", fcount);
			return(fp);

		case 'r':
		case 'R':
			printf("%M", fcount);
			return(fp);

		case 'a':
			psymoff((WORD)dot, ptype, itype & SYMF ?"?%16t":"/%16t");
			dotinc = 0;
			break;

		case 'p':
			w = atow(aget(dot, itype));
			if (errflg)
				return (fp);
			if (mkfault)
				return (0);
			psymoff(w, ptype, "%16t");
			dotinc = sizeof(TADDR);
			break;

		case 'u':
		case 'w':
		case 'd':
		case 'x':
		case 'o':
		case 'q':
			w = stow(sget(dot, itype));
			if (errflg)
				return (fp);
			if (mkfault)
				return (0);
			dotinc = sizeof(TSHORT);
			if (c == 'u')
				printf("%-8U", w);
			else if (c == 'w')
				printf("%-8R", w);
			else if (c == 'd')
				printf("%-8D", w);
			else if (c == 'x')
				printf("%-8X", w);
			else if (c == 'o')
				printf("%-8O", w);
			else if (c == 'q')
				printf("%-8Q", w);
			break;

		case 'U':
		case 'W':
		case 'D':
		case 'X':
		case 'O':
		case 'Q':
			w = ltow(lget(dot, itype));
			if (errflg)
				return (fp);
			if (mkfault)
				return (0);
			dotinc = sizeof(TLONG);
			if (c == 'U')
				printf("%-16U", w);
			else if (c == 'W')
				printf("%-16R", w);
			else if (c == 'D')
				printf("%-16D", w);
			else if (c == 'X')
				printf("%-16X", w);
			else if (c == 'O')
				printf("%-16O", w);
			else if (c == 'Q')
				printf("%-16Q", w);
			break;

		case 'b':
		case 'B':
		case 'c':
		case 'C':
			w = ctow(cget(dot, itype));
			if (errflg)
				return (fp);
			if (mkfault)
				return (0);
			if (modifier == 'C')
				printesc((char)w);
			else if (modifier == 'B' || modifier == 'b')
				printf("%-8O", w);
			else
				printc((char)w);
			dotinc = sizeof(TCHAR);
			break;

		case 's':
		case 'S':
			savdot=dot;
			dotinc=sizeof(TCHAR);
			while ((w=ctow(cget(dot,itype))) && errflg==0) {
				dot=inkdot((WORD)sizeof(TCHAR));
				if (modifier == 'S')
					printesc((char)w);
				else
					printc((char)w);
				endline();
			}
			dotinc=(dot-savdot+1) * sizeof(TCHAR);
			dot=savdot;
			break;

		case 'Y':
			printf("%-24Y", (long)ltow(lget(dot, itype)));
			break;

		case 'i':
			printins(itype,lget(dot, itype));
			printc(EOR);
			break;

		case 'f':
			if (fget(dot, itype, (char *)&fl.s, sizeof(TFLOAT)) == 0)
				return (fp);
			if (mkfault)
				return (0);
			dotinc = sizeof(TFLOAT);
			fpout('f', (char *)&fl.s);
			break;

		case 'F':
			if (fget(dot, itype, (char *)&fl.d, sizeof(TDOUBLE)) == 0)
				return (fp);
			if (mkfault)
				return (0);
			dotinc = sizeof(TDOUBLE);
			fpout('F', (char *)&fl.d);
			break;

		case 'n':
		case 'N':
			printc('\n');
			dotinc=0;
			break;

		case '"':
			dotinc=0;
			while (*fp != '"' && *fp)
				printc(*fp++);
			if (*fp)
				fp++;
			break;

		case '^':
			dot=inkdot(-dotinc*fcount);
			return(fp);

		case '+':
			dot=inkdot((WORD)fcount);
			return(fp);

		case '-':
			dot=inkdot(-(WORD)fcount);
			return(fp);

		default:
			error(BADMOD);
		}
		if ((itype & SPTYPE) != NOSP)
			dot=inkdot(dotinc);
		fcount--;
		endline();
	}

	return(fp);
}

printesc(c)
{
	c &= STRIP;
	if (c == 0177)
		printf("^?");
	else if (c < SP)
		printf("^%c", c + '@');
	else
		printc(c);
}

ADDR
inkdot(incr)
WORD incr;
{
	ADDR	newdot;

	newdot=dot+incr;
	if ((incr >= 0 && newdot < dot)
	||  (incr < 0 && newdot > dot))
		error(ADWRAP); 						
	return(newdot);
}
