/*
 *
 *	UNIX debugger
 *
 */
#include "defs.h"
#include "regs.h"
#include <a.out.h>

MSG	LONGFIL;
MSG	NOTOPEN;
MSG	BADMOD;

int	infile;
int	outfile;
int	maxpos;
int	radix;

BKPT *bkpthead;

char	lastc;

char *signals[] = {
	"",
	"hangup",
	"interrupt",
	"quit",
	"illegal instruction",
	"trace/BPT",
	"IOT",
	"EMT",
	"floating exception",
	"killed",
	"bus error",
	"memory fault",
	"bad system call",
	"broken pipe",
	"alarm call",
	"terminated",
	"signal 16",
	"stopped",
	"stop (tty)",
	"continue (signal)",
	"child termination",
	"stop (tty input)",
	"stop (tty output)",
	"input available (signal)",
	"cpu timelimit",
	"file sizelimit",
	"signal 26",
	"signal 27",
	"signal 28",
	"signal 29",
	"signal 30",
	"signal 31",
};

/* general printing routines ($) */

printtrace(modif)
{
	int	i;
	register BKPT *bk;
	char	*comptr;
	register struct nlist *sp;
	int	stack;

	if (cntflg==0)
		cntval = -1;
	switch (modif) {

	case '<':
		if (cntval == 0) {
			while (readchar() != EOR)
				;
			reread();
			break;
		}
		if (rdc() == '<')
			stack = 1;
		else {
			stack = 0;
			reread();
		}
		/* fall through */
	case '>':
		{
			char	file[64];
			char	Ifile[128];
			extern char	*Ipath;
			int	index;

			index=0;
			if (modif=='<')
				iclose(stack, 0);
			else
				oclose();
			if (rdc()!=EOR) {
				do {
					file[index++]=lastc;
					if (index>=63)
						error(LONGFIL);
				} while (readchar()!=EOR);
				file[index]=0;
				if (modif=='<') {
					if (Ipath) {
						strcpy(Ifile, Ipath);
						strcat(Ifile, "/");
						strcat(Ifile, file);
					}
					infile=open(file,0);
					if (infile<0 && (infile=open(Ifile,0))<0) {
						infile=STDIN;
						error(NOTOPEN);
					} else {
						if (cntflg)
							var[9] = cntval;
						else
							var[9] = 1;
					}
				} else {
					outfile=open(file,1);
					if (outfile<0)
						outfile=creat(file,0644);
					else
						lseek(outfile,0L,2);
				}

			} else {
				if (modif == '<')
					iclose(-1, 0);
			}
			reread();
		}
		break;

	case 'p':
		kmproc();
		break;

	case 'k':
		kmsys();
		break;

	case 'd':
		if (adrflg) {
			if (adrval != 0
			&&  (adrval<2 || adrval>16))
				error("radix should be between 2 and 16, or 0");
			radix = adrval;
			if (radix)
				printf("radix=%d base ten",radix);
			else
				printf("radix=magic");
		}
		break;

	case 'q':
	case 'Q':
		done();

	case 'w':
	case 'W':
		maxpos=(adrflg?adrval:MAXPOS);
		break;

	case 's':
	case 'S':
		maxoff=(adrflg?adrval:MAXOFF);
		break;

	case 'v':
	case 'V':
		prints("variables\n");
		for (i=0;i<NVARS;i++) {
			if (var[i]) {
				printc((i<=9 ? '0' : 'a'-10) + i);
				printf(" = %R\n", var[i]);
			}
		}
		break;

	case 'm':
	case 'M':
		printmap("? map", symmap);
		printmap("/ map", cormap);
		break;

	case 0:
	case '?':
		if (pid)
			printf("pcs id = %d\n",pid);
		else
			prints("no process\n");
		sigprint();
		flushbuf();

	case 'r':
	case 'R':
		printregs(modif);
		return;

	case 'c':
	case 'C':
		ctrace(modif);
		break;

		/*print externals*/
	case 'e':
	case 'E':
		for (sp = symtab; sp < esymtab; sp++) {
			if (sp->n_type==(N_DATA|N_EXT) || sp->n_type==(N_BSS|N_EXT))
				printf("%s:%12t%R\n", sp->n_un.n_name,
					ltow(lget(sp->n_value,CORF|DATASP)));
		}
		break;

		/*print breakpoints*/
	case 'b':
	case 'B':
		printf("breakpoints\ncount%8tbkpt%24tcommand\n");
		for (bk=bkpthead; bk; bk=bk->nxtbkpt)
			if (bk->flag) {
				printf("%-8.8d",bk->count);
				psymoff((WORD)bk->loc,INSTSP,"%24t");
				comptr=bk->comm;
				while (*comptr)
					printc(*comptr++);
			}
		break;

	default:
		error(BADMOD);
	}

}

printmap(s,mp)
char *s;
register MAP *mp;
{
	char *maptype();

	if (mp == symmap)
		printf("%s%12t`%s'\n", s, fsym < 0 ? "-" : symfil);
	else if (mp == cormap)
		printf("%s%12t`%s'\n", s, fcor < 0 ? "-" : corfil);
	else
		printf("%s\n", s);
	for (; mp->flag & MPINUSE; mp++)
		printf("%-8s b = %-16Re = %-16Rf = %-16R\n",
		    maptype(mp->sp), (WORD)mp->b, (WORD)mp->e, (WORD)mp->f);
}

char *
maptype(sp)
int sp;
{

	switch (sp & SPTYPE) {
	case INSTSP:
		return ("text");
	case DATASP:
		return ("data");
	case UBLKSP:
		return ("user");
	default:
		return ("nonsense");
	}
}

printpc()
{
	TLONG w;

	dot = (ADDR)rtow(rget(PC));
	psymoff((WORD)dot, INSTSP, "?%16t");
	w = lget(dot, SYMF|INSTSP);
	chkerr();
	printins(SYMF|INSTSP, w);
	printc(EOR);
}

char	*illinames[] = {
	"reserved addressing fault",
	"privileged instruction fault",
	"reserved operand fault"
};
char	*fpenames[] = {
	0,
	"integer overflow trap",
	"integer divide by zero trap",
	"floating overflow trap",
	"floating/decimal divide by zero trap",
	"floating underflow trap",
	"decimal overflow trap",
	"subscript out of range trap",
	"floating overflow fault",
	"floating divide by zero fault",
	"floating undeflow fault"
};

sigprint()
{
	if ((signo>=0) && (signo<sizeof signals/sizeof signals[0]))
		prints(signals[signo]);
	switch (signo) {

	case SIGFPE:
		if ((sigcode > 0
		&&  sigcode < sizeof fpenames / sizeof fpenames[0]))
			printf(" (%s)", fpenames[sigcode]);
		break;

	case SIGILL:
		if ((sigcode >= 0
		&&  sigcode < sizeof illinames / sizeof illinames[0]))
			printf(" (%s)", illinames[sigcode]);
		break;
	}
}
