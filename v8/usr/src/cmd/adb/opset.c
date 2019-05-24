/*
 * instruction decoding
 * VAX instructions; works only on a VAX for now
 */

#include "defs.h"

/* instruction printing */

/*
 * Argument access types
 */
#define ACCA	(8<<3)	/* address only */
#define ACCR	(1<<3)	/* read */
#define ACCW	(2<<3)	/* write */
#define ACCM	(3<<3)	/* modify */
#define ACCB	(4<<3)	/* branch displacement */
#define ACCI	(5<<3)	/* XFC code */

/*
 * Argument data types
 */
#define TYPB	0	/* byte */
#define TYPW	1	/* word */
#define TYPL	2	/* long */
#define TYPQ	3	/* quad */
#define TYPF	4	/* floating */
#define TYPD	5	/* double floating */

typedef	struct optab	*OPTAB;
struct optab {
	char *iname;
	char val;
	char nargs;
	char argtype[6];
} optab[];
char *regname[];
char *fltimm[];
static ADDR	incp;
static int	type;

int ioptab[256]; /* index by opcode to optab */

mkioptab() {/* set up ioptab */
	register OPTAB p=optab;

	while (p->iname){
		ioptab[p->val&LOBYTE]=p-optab;
		p++;
	}
}

printins(idsp,ins)
register WORD ins;
{
	register short argno;	/* argument index */
	register short mode;	/* mode */
	register char **r;	/* register name */
	long	d;		/* assembled byte, word, long or float */
	long	snarf();
	register char *	ap;
	register OPTAB	ip;

	type = DATASP;
	ins &= LOBYTE;
	ip=optab+ioptab[ins];
	printf("%s%8t",ip->iname);
	incp = 1;
	ap = ip->argtype;
	for (argno=0; argno<ip->nargs; argno++,ap++) {
		var[argno] = 0x80000000;
		if (argno!=0) printc(',');
top:
		if (*ap&ACCB)
			mode = 0xAF + ((*ap&7)<<5);  /* branch displacement */
		else{
			mode = cget(inkdot((WORD)incp), idsp);
			chkerr();
			++incp;
		}
		if (mode & 0300) {/* not short literal */
			r = &regname[mode&0xF];
			mode >>= 4;
			switch ((int)mode) {
			case 4: /* [r] */
				printf("[%s]",*r);
				goto top;
			case 5: /* r */
				printf("%s",*r);
				break;
			case 6: /* (r) */
				printf("(%s)",*r);
				break;
			case 7: /* -(r) */
				printf("-(%s)",*r);
				break;
			case 9: /* *(r)+ */
				printc('*');
			case 8: /* (r)+ */
				if (r==(regname+0xF)) {
					printc('$');
					if (mode==9){	/* PC absolute, always 4 bytes*/
						d = snarf(4, idsp);
						goto disp;
					}
					switch(*ap&7){
					case TYPB:
						d = snarf(1, idsp);
						goto disp;
					case TYPW:
						d = snarf(2, idsp);
						goto disp;
					case TYPL:
						d = snarf(4, idsp);
						goto disp;
					case TYPQ:
						d = snarf(4, idsp);
						printquad(d, snarf(4, idsp));
						break;
					case TYPF:
						printfloating(snarf(4, idsp), 0L);
						break;
					case TYPD:
						d = snarf(4, idsp);
						printfloating(d, snarf(4, idsp));
						break;
					} /*end of type switch */
					/*
					 *	here only for TYPQ, TYPf, TYPD
					 *	others went to disp
					 */
				} else {	/*it's not PC immediate or abs*/
					printf("(%s)+",*r);
				}
				break;
			case 0xB:	/* byte displacement defferred*/
				printc('*');
			case 0xA:	/* byte displacement */
				d = snarf(1, idsp);
				goto disp;
			case 0xD:	/* word displacement deferred */
				printc('*');
			case 0xC:	/* word displacement */
				d = snarf(2, idsp);
				goto disp;
			case 0xF:	/* long displacement deferred */
				printc('*');
			case 0xE:	/* long displacement */
				d = snarf(4, idsp);
				goto disp;
disp:
				var[argno]=d;
				if (r==(regname+0xF) && mode>=0xA){
					/* PC offset addressing */
					var[argno] += dot+incp;
				}
				psymoff(var[argno],type,"");
				if (r != regname+0xF)
					printf("(%s)",*r);
				break;
			} /* end of the mode switch */
		} else {   /* short literal */
			var[argno]=mode;
			if((*ap&7)==TYPF || (*ap&7)==TYPD)
				printf("$%s",fltimm[mode]);
			else
				printf("$%r",mode);
		}
	}
	if (ins==0xCF || ins==0xAF || ins==0x8F) {/* CASEx instr */
		for (argno=0; argno<=var[2]; ++argno) {
			printc(EOR);
			printf("    %R:  ",argno+var[1]);
			d=sget(inkdot((WORD)incp+argno+argno),idsp);
			if (d&0x8000) d -= 0x10000;
			psymoff((WORD)(inkdot((WORD)incp)+d),type,"");
		}
		incp += var[2]+var[2]+2;
	}
	dotinc=incp;
}

/*
 *	magic values to mung an offset to a register into
 *	something that psymoff can understand.. all magic
 */
/* 0	1	2	3	4 */
static long magic_masks[5] =	{
	0,	0x80,	0x8000,	0,	0};
static long magic_compl[5] =	{
	0,	0x100,	0x10000,0,	0};

long
snarf(nbytes, idsp)
int	nbytes;
{
	register	int	byteindex;
	union Long{
		char	long_bytes[4];
		long	long_value;
	} d;

	d.long_value = 0;
	for (byteindex = 0; byteindex < nbytes; byteindex++){
		d.long_bytes[byteindex] = cget(inkdot((WORD)incp), idsp);
		chkerr();
		++incp;
	}
	if (d.long_value & magic_masks[nbytes])
		d.long_value -= magic_compl[nbytes];
	return(d.long_value);
}

printfloating(word_first, word_last)
long	word_first;
long	word_last;
{
	union	Double{
		struct {
			long	word_first;
			long	word_last;
		} composite;
		double	dvalue;
	} reconstructed;

	reconstructed.composite.word_first = word_first;
	reconstructed.composite.word_last = word_last;
	fpout('F', (char *)&reconstructed.dvalue);
}

printquad(word_first, word_last)
long	word_first;
long	word_last;
{
	union Quad {
		char	quad_bytes[8];
		long	quad_long[2];
	} reconstructed;
	int	leading_zero = 1;
	int	byteindex;
	int	nibbleindex;
	register	int	ch;

	reconstructed.quad_long[0] = word_first;
	reconstructed.quad_long[1] = word_last;
	for (byteindex = 7; byteindex >= 0; --byteindex){
		for (nibbleindex = 4; nibbleindex >= 0; nibbleindex -= 4){
			ch = (reconstructed.quad_bytes[byteindex]
			    >> nibbleindex) & 0x0F;
			if ( ! (leading_zero &= (ch == 0) ) ){
				if (ch <= 0x09)
					printc(ch + '0');
				else
					printc(ch - 0x0A + 'a');
			}
		}
	}
}
