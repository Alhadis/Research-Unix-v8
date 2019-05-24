/*
 * Header for object code improver
 */

#include <stdio.h>

#ifndef	CHECK
#define	CHECK(x)
#endif

#define	JBR	1
#define	CBR	2
#define	JMP	3
#define	LABEL	4
#define	DLABEL	5
#define	EROU	7
#define	JSW	9
#define	MOV	10
#define	CLR	11
#define	COM	12
#define	INC	13
#define	DEC	14
#define	NEG	15
#define	TST	16
#define	ASR	17
#define	ASL	18
#define	SXT	19
#define	CMP	20
#define	ADD	21
#define	SUB	22
#define	BIT	23
#define	AND	24
#define	OR	25
#define	MUL	26
#define	DIV	27
#define	ASH	28
#define	XOR	29
#define	TEXT	30
#define	DATA	31
#define	BSS	32
#define	EVEN	33
#define	MOVF	34
#define	MOVOF	35
#define	MOVFO	36
#define	ADDF	37
#define	SUBF	38
#define	DIVF	39
#define	MULF	40
#define	CLRF	41
#define	CMPF	42
#define	NEGF	43
#define	TSTF	44
#define	CFCC	45
#define	SOB	46
#define	JSR	47
#define BTST	48
#define SHORT	49
#define LSL 	50
#define LSR	51
#define SET	52
#define MOVM	53
#define LEA	54
#define	END	55
#define	PCRELO	56
#define	PCRELOX	57
#define	STABS	60

#define	JEQ	0
#define	JNE	1
#define	JLE	2
#define	JGE	3
#define	JLT	4
#define	JGT	5
#define	JLO	6
#define	JHI	7
#define	JLOS	8
#define	JHIS	9

#define	BYTE	100
#define WORD	101
#define LONG	102
#define	LSIZE	512

struct node {
	char	op;
	char	subop;
	struct	node	*forw;
	struct	node	*back;
	struct	node	*ref;
	int	labno;
	char	*code;
	int	refc;
};

extern struct optab {
	char	*opstring;
	int	opcode;
} optab[];

char	line[LSIZE];
struct	node	first;
char	*curlp;
int	nbrbr;
int	nsaddr;
int	redunm;
int	iaftbr;
int	njp1;
int	nrlab;
int	nxjump;
int	ncmot;
int	nrevbr;
int	loopiv;
int	nredunj;
int	nskip;
int	ncomj;
int	nsob;
int	nrtst;
int	nlit;

int	nchange;
extern int	isn;
int	debug;
extern int	lastseg;
extern char 	*lastnseg;
char	*lasta;
char	*lastr;
char	*alasta;
char	*alastr;
char	*firstr;
extern char	revbr[];
char	regs[80][40];
char	conloc[20];
char	conval[20];
char	ccloc[20];

#define	RT1	36
#define	RT2	37
#define RT3	76
#define RT4	77
#define	AREG	8
#define	NREG	16
#define	LABHS	127
#define	OPHS	57

struct optab *ophash[OPHS];
extern struct	node *nonlab();
extern char	*copy();
extern char	*sbrk();
extern char	*findcon();
extern struct	node *insertl();
extern struct	node *codemove();
extern char	*sbrk();
extern char	*alloc();
