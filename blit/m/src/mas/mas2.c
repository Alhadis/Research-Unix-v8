#include <stdio.h>
#include "mas.h"
#include "mas.yh"
#include <signal.h>

int	curlen;
int	debugflag	= 0;
int	lineno	= 1;
int	nstabs;
char	*filename = NULL;
long	datbase;
struct	nlist *nextsym = {symtab};
struct	nlist *firstsym;
struct	exec hdr = {
	A_MAGIC1, 0, 0, 0, 0, 0, 0, 0,
};

char	*tmpn1;
char	*tmpn2;
char	*tmpn3;
char	*tmpns;
struct	exp	usedot[NLOC+NLOC];
FILE	*usefile[NLOC+NLOC];
FILE	*rusefile[NLOC+NLOC];
FILE	*stabfil;

main(argc, argv)
char **argv;
{
	int symcmp();
	register struct instab *ip;
	register struct nlist *sp;
	int c;
	long v;
	register struct nlist **hp;
	char *outfile = "a.out";
	int delexit();

	for (ip=instab; ip->opname; ip++) {
		strcpy(yytext, ip->opname);
		usrname = 0;
		hp = lookup(1);
		if ((*hp)->tag==0) {
			sp = *hp;
			sp->n_value = ip-instab;
			if (ip->size<=BWL) {
				sp->tag = INST0-256;
				if (ip->addr1)
					sp->tag = INST1-256;
				if (ip->addr2)
					sp->tag = INST2-256;
			} else
				sp->tag = ip->size - BWL;
		}
		sp->n_type++;
	}
	strcpy(yytext, "%d0");
	for (c=0; c<8; c++) {
		yytext[1] = 'd';
		yytext[2] = c+'0';
		sp = *lookup(1);
		sp->n_type = N_ABS;
		sp->n_value = c;
		yytext[1] = 'a';
		sp = *lookup(1);
		sp->n_type = N_ABS;
		sp->n_value = c+8;
	}
	strcpy(yytext, "%sp");
	sp = *lookup(1);
	sp->n_type = N_ABS;
	sp->n_value = AREGS+7;
	strcpy(yytext, "%fp");
	sp = *lookup(1);
	sp->n_type = N_ABS;
	sp->n_value = AREGS+6;
	strcpy(yytext, "%usp");
	sp = *lookup(1);
	sp->n_type = N_ABS;
	sp->n_value = SPREG;
	strcpy(yytext, "%pc");
	sp = *lookup(1);
	sp->n_type = N_ABS;
	sp->n_value = PCREG;
	strcpy(yytext, "%cc");
	sp = *lookup(1);
	sp->n_type = N_ABS;
	sp->n_value = CCREG;
	strcpy(yytext, "%sr");
	sp = *lookup(1);
	sp->n_type = N_ABS;
	sp->n_value = SRREG;
	firstsym = nextsym;
	for (c=0; c<NLOC; c++) {
		usedot[c].xtype = N_TEXT;
		usedot[c+NLOC].xtype = N_DATA;
	}
	if (signal(SIGINT, SIG_IGN)!=SIG_IGN)
		signal(SIGINT, delexit);
	tmpn1 = mktemp("/tmp/mas1aXXXXX");
	tmpfil = fopen(tmpn1, "w");
	if (tmpfil==NULL) {
		yyerror("Bad temp1file");
		delexit();
	}
	while(argc > 1)
		if(argv[1][0] == '-') {
			if(argv[1][1] == 'd') {
				debugflag = 1;
				--argc;
				++argv;
			} else if(argv[1][1]=='o' && argc>2) {
				outfile = argv[2];
				argc -= 2;
				argv += 2;
			} else {
				yyerror("Bad option argument");
				delexit();
			}
		} else {
			lineno = 1;
			if(freopen(filename=argv[1], "r", stdin) == NULL) {
				printf("as: Can't open %s\n", filename);
				delexit();
			}
			p2filename(filename);
			yyparse();
			argc--;
			argv++;
		}
	put2(0, tmpfil);
	put2(0, tmpfil);
	put2(0, tmpfil);
	put2(0, tmpfil);
	if (anyerrs)
		delexit();
	fclose(tmpfil);
	tmpfil = fopen(tmpn1, "r");
	if (tmpfil==NULL) {
		yyerror("Bad tmp1file (r)");
		delexit();
	}
	tsize = 0;
	for (c=0; c<NLOC; c++) {
		v = usedot[c].xvalue;
		usedot[c].xvalue = tsize;
		tsize += v;
	}
	datbase = round(tsize, SEGRND);
	for (c=0; c<NLOC; c++) {
		v = round(usedot[NLOC+c].xvalue, DW);
		usedot[NLOC+c].xvalue = datbase+dsize;
		dsize += v;
	}
	hdr.a_bss = dsize;
	for (sp=firstsym; sp->n_name[0]; sp++) {
		if ((sp->n_type&XTYPE)==N_UNDF)
			sp->n_type = N_EXT+N_UNDF;
		else if ((sp->n_type&XTYPE)==N_DATA)
			sp->n_value += usedot[sp->index].xvalue;
		else if ((sp->n_type&XTYPE)==N_TEXT)
			sp->n_value += usedot[sp->index].xvalue;
		else if ((sp->n_type&XTYPE)==N_BSS) {
			long bs;
			int rnd = 0;
			bs = sp->n_value;
			if (bs>=8)
				rnd = DW;
			else if (bs>=4)
				rnd = FW;
			else if (bs>=2)
				rnd = HW;
			if (rnd) {
				hdr.a_bss = round(hdr.a_bss, (long)rnd);
			}
			sp->n_value = hdr.a_bss + datbase;
			hdr.a_bss += bs;
		}
	}
	hdr.a_bss -= dsize;
	hdr.a_bss = round(hdr.a_bss, (long)DW);
	tmpn2 = mktemp("/tmp/mas2aXXXXX");
	txtfil = fopen(outfile, "w");
	if (txtfil==NULL) {
		yyerror("Cannot create %s", outfile);
		delexit();
	}
	usefile[0] = txtfil;
	tmpn3 = mktemp("/tmp/mas3aXXXXX");
	relfil = fopen(tmpn3, "w");
	if (relfil==NULL) {
		yyerror("temp file can't be opened");
		delexit();
	}
	rusefile[0] = relfil;
	hdr.a_text = tsize;
	hdr.a_data = dsize;
	hdr.a_syms = sizeof(symtab[0]) * ((nextsym-firstsym) + nstabs);
	fwrite((char *)&hdr, sizeof(hdr), 1, txtfil);
	tsize = 0;
	dsize = 0;
	lineno = 1;
	dotp = &usedot[0];
	passno = 2;
	for (sp=symtab; sp->n_name[0]; sp++)
		sp->index = 20000;
	rewind(stdin);
	tmpns = mktemp("/tmp/mas4aXXXXX");
	if ((stabfil = fopen(tmpns, "w")) == NULL) {
		yyerror("stab tmp open err");
		delexit();
	}
	yyparse();
	for (c=0; c<NLOC; c++) {
		if (usefile[c]) {
			if (c>0)
				fclose(usefile[c]);
			fclose(rusefile[c]);
		}
		if (usefile[NLOC+c]) {
			txtfil = usefile[NLOC+c];
			relfil = rusefile[NLOC+c];
			while (usedot[c+NLOC].xvalue&DW) {
				outhw(0, N_ABS, SNULL, 0);
				usedot[c+NLOC].xvalue += 2;
			}
			fclose(txtfil);
			fclose(relfil);
		}
	}
	txtfil = usefile[0];
	for (c=1; c<NLOC+NLOC; c++) {
		register ch;
		if (usefile[c]) {
			tmpn2[TMPC] = c+'a';
			relfil = fopen(tmpn2, "r");
			if (relfil==NULL) {
				yyerror("cannot reopen temp");
				continue;
			}
			while ((ch = getc(relfil))>=0)
				putc(ch, txtfil);
			fclose(relfil);
		}
	}
	for (c=0; c<NLOC+NLOC; c++) {
		register ch;
		if (rusefile[c]) {
			tmpn3[TMPC] = c+'a';
			relfil = fopen(tmpn3, "r");
			if (relfil==NULL) {
				yyerror("cannot reopen temp");
				continue;
			}
			while ((ch = getc(relfil))>=0)
				putc(ch, txtfil);
			fclose(relfil);
		}
	}
	qsort(firstsym, nextsym-firstsym, sizeof(symtab[0]), symcmp);
	for (sp=firstsym; sp->n_name[0]; sp++) {
		sp->n_type &= ~XFORW;
		sp->index = 0;
	}
	fwrite((char *)firstsym, sizeof(firstsym[0]), nextsym-firstsym, txtfil);
	fclose(stabfil);
	if ((stabfil = fopen(tmpns, "r")) == NULL) {
		yyerror("Can't reopen stab file");
		delexit();
	}
	while ((c = getc(stabfil)) != EOF)
		putc(c, txtfil);
	fclose(stabfil);
	delete();
	if (anyerrs==0 && orgwarn)
		fprintf(stderr, "Caution: absolute origins.\n");
	exit(anyerrs!=0);
}

symcmp(p, q)
register struct nlist *p, *q;
{
	if (p->index < q->index)
		return(-1);
	if (p->index > q->index)
		return(1);
	return(p-q);
}

delexit()
{
	delete();
	exit(1);
}

delete()
{
	register c;

	if (tmpns)
		unlink(tmpns);
	if (tmpn1)
		if(debugflag)
			fprintf(stderr, "tempfile = %s\n", tmpn1);
		else
			unlink(tmpn1);
	for (c=0; c<NLOC+NLOC; c++) {
		if (tmpn2) {
			tmpn2[TMPC] = c+'a';
			unlink(tmpn2);
		}
		if (tmpn3) {
			tmpn3[TMPC] = c+'a';
			unlink(tmpn3);
		}
	}
}

struct nlist **
lookup(instflg)
{
	int un;
	unsigned ihash, incr;
	register struct nlist **hp;
	register char *p1, *p2;

	un = usrname;
	usrname = 1;
	ihash = 0;
	p1 = yytext;
	while (*p1) {
		ihash <<= 1;
		ihash += *p1++;
	}
	while (p1<yytext+NCPS)
		*p1++ = 0;
	hp = &hshtab[ihash%NHASH];
	incr = ihash%31 + 1;
	while (*hp) {
		if (instflg==2)
			goto no;
		p2 = (*hp)->n_name;
		for (p1=yytext; p1<yytext+NCPS;)
			if (*p1++ != *p2++)
				goto no;
		if (un == ((*hp)->tag==0))
			return(hp);
	no:
		if ((hp += incr) >= &hshtab[NHASH])
			hp -= NHASH;
	}
	if(++hshused >= NHASH) {
		yyerror("Symbol table overflow");
		delexit();
	}
	if (instflg) {
		for (p1=yytext,p2=nextsym->n_name; p1<yytext+NCPS;)
			*p2++ = *p1++;
		*hp = nextsym++;
	}
	return(hp);
}

outxpr(xp, size)
register struct exp *xp;
{
	if (size==L)
		output(xp->xvalue, xp->xtype, xp->xname);
	else
		outhw((short)xp->xvalue, xp->xtype, xp->xname, 0);
}

output(val, type, xsym)
long val;
struct nlist *xsym;
{

	outhw((short)(val>>16), type, xsym, X2WDS);
	outhw((short)val, N_ABS, SNULL, 0);
}

outhw(val, type, xsym, rel)
struct nlist *xsym;
{
	flushfield(NBPW/2);
	if (passno==1)
		return;
	type &= ~XFORW;
	if (type==N_UNDF)
		yyerror("Undefined reference");
	put2(val, txtfil);
	if (type==N_EXT+N_UNDF && xsym) {
		setindex(xsym);
		put2((xsym->index<<7) | type | rel, relfil);
	} else
		put2((type & XTYPE)|rel, relfil);
}

setindex(xsym)
register struct nlist *xsym;
{
	if (xsym->index == 20000)
		xsym->index = gindex++;
	if (gindex >= 1023)
		yyerror("Too many external symbols");
}

flushfield(n)
{
	if (bitoff) {
		if (n==NBPW/4) {
			if (bitoff%8!=0)
				bitoff = (bitoff+07) & ~07;
			if (bitoff<NBPW/2)
				return;
		}
		bitoff = 0;
		if (n!=NBPW) {
			outhw((short)(bitfield>>16), N_ABS, SNULL, 0);
			dotp->xvalue += 2;
			dotp->xvalue &= ~01;
		} else {
			output(bitfield, N_ABS, SNULL);
			dotp->xvalue += 4;
			dotp->xvalue &= ~01;
		}
		bitfield = 0;
	}
}
