#
/*
 * C object code improver-- second part
 */

#include "c2.h"

extern int Func_mask[];
char	nxtregs[4][80];
int	reg_type[80];
int	ccmode;

rmove()
{
	register struct node *p;
	register int r;
	register  r1, flt;

	for (p=first.forw; p!=0; p = p->forw) {
	flt = 0;
	switch (p->op) {

	case MOV:
		if (p->subop==BYTE)
			goto dble;
		dualop(p);
		if ((r = findrand(regs[RT1], flt, p->subop)) >= 0) {
			if (r == flt+isreg(regs[RT2]) && p->forw->op!=CBR
			   && p->forw->op!=SXT
			   && p->forw->op!=CFCC) {
redundant:
				p->forw->back = p->back;
				p->back->forw = p->forw;
				redunm++;
				nchange++;
				continue;
			}
		}
		if (equstr(regs[RT1], regs[RT2]) && p->forw->op!=CBR) {
		    p->forw->back = p->back;
		    p->back->forw = p->forw;
		    nchange++;
		    continue;
		}
		if (p->forw->op==MOV && p->subop==p->forw->subop) {
			nextdual(p->forw);
			if (equstr(regs[RT1], nxtregs[1]) &&
			    equstr(regs[RT2], nxtregs[0]) ) {
				redunm++;
				p->forw->forw->back = p;
				p->forw = p->forw->forw;
		    		nchange++;
				continue;
			}
		}
		if (p->subop==LONG && islit(regs[RT1]) && isd0(regs[RT2]) &&
		    (p->forw->op==ADD || p->forw->op==SUB)) {
			nextdual(p->forw);
			if (isd0(nxtregs[0])) {
				sprintf(nxtregs[0], ",%s", nxtregs[1]);
				p->forw->code = copy(2, regs[RT1], nxtregs[0]);
				p->forw->back = p->back;
				p->back->forw = p->forw;
				nchange++;
				continue;
			}
		}
		if (p->forw->op==AND && p->forw->forw->op==CBR) {
			nextdual(p->forw);
			if (equstr(nxtregs[0], "&32768") && 
			    equstr(regs[RT2], nxtregs[1])) {
				if (p->subop==WORD) {
					p->op = TST;
					p->code = copy(1, regs[RT1]);
					p->forw = p->forw->forw;
					p->forw->back = p;
					if (p->forw->subop==JNE)
						p->forw->subop = JLT;
					else
						p->forw->subop = JGE;
					continue;
				}
			}
		}
		if (p->subop==LONG && ispow2(regs[RT1]) && isd0(regs[RT2])
		  && p->forw->op==MOV) {
		    nextdual(p->forw);
		    if (equstr(nxtregs[0], "&0") && isd1(nxtregs[1])
		      && p->forw->forw->op==MOV) {
			nextdual(p->forw->forw);
			if (isd1(nxtregs[1]) && p->forw->forw->forw->op==AND) {
			    nextdual(p->forw->forw->forw);
			    if (isd0(nxtregs[0]) && isd1(nxtregs[1]) &&
			      p->forw->forw->forw->forw->op==CBR) {
				p->op = BTST;
				p->subop = 0;
				sprintf(regs[RT1],"&%d",log2(regs[RT1])-1);
				nextdual(p->forw->forw);
				strcpy(regs[RT2], nxtregs[0]);
				sprintf(nxtregs[0],",%s",regs[RT2]);
				p->code = copy(2,regs[RT1],nxtregs[0]);
				p->forw = p->forw->forw->forw->forw;
				p->forw->back = p;
		    		nchange++;
				break;
			    }
			}
		    }
		}
		if (equstr(regs[RT1], "&0") && isreg(regs[RT2])==-1) {
			p->op = CLR;
			strcpy(regs[RT1], regs[RT2]);
			regs[RT2][0] = 0;
			p->code = copy(1, regs[RT1]);
			goto sngl;
		}
		repladdr(p, 0, flt);
		r = isreg(regs[RT1]);
		r1 = isreg(regs[RT2]);
		dest(regs[RT2], flt);
		if (r >= 0)
			if (r1 >= 0)
				savereg(r1+flt, regs[r+flt], p->subop);
			else
				savereg(r+flt, regs[RT2], p->subop);
		else
			if (r1 >= 0)
				savereg(r1+flt, regs[RT1], p->subop);
			else
				setcon(regs[RT1], regs[RT2]);
		source(regs[RT1]);
		setcc(regs[RT2]);
		ccmode = p->subop;
		continue;

	case MOVM:
		{
			int functno;

			if( sscanf(p->code, "&M%%%d,", &functno)==1 ||
			   sscanf(p->code,"S%%%*d(%%fp),&M%%%d",&functno) ==1) {
				if( Func_mask[functno]==0 ) {
					p->back->forw = p->forw;
					p->forw->back = p->back;
				}
			}
			break;
		}
		
	case ADDF:
	case SUBF:
	case DIVF:
	case MULF:
		flt = NREG;
		goto dble;

	case AND:
	case ADD:
	case SUB:
	case OR:
	case MUL:
	case DIV:
	case ASL:
	case ASR:
	case LSL:
	case LSR:
	dble:
		dualop(p);
		if (p->op==AND && p->forw->op==CBR) {
			if (equstr(regs[RT1], "&32768")) {
				if (p->subop==WORD) {
					p->op = TST;
					p->code = copy(1, regs[RT2]);
					if (p->forw->subop==JNE)
						p->forw->subop = JLT;
					else
						p->forw->subop = JGE;
					continue;
				}
			}
		}
		if(p->op==ADD && regs[RT1][0]=='&'
		    && equstr(regs[RT2], "%sp") ) {
			int i;

			sscanf(regs[RT1], "&%d", &i);
			if( (i>=-32768 && i<=32768) && (i<0 || i>8) ) {
				p->op = LEA;
				p->subop = 0;
				sprintf(nxtregs[0], "%d(%%sp),", i);
				p->code = copy(2,nxtregs[0],regs[RT2]);
				nchange++;
				continue;
			}
		}
		if(p->op==ASL || p->op==ASR || p->op==LSL || p->op==LSR) {
			if( equstr(regs[RT1], "&0") ) {
				p->back->forw = p->forw;
				p->forw->back = p->back;
				nchange++;
				continue;
			}
		}
		if (p->op==AND && equstr(regs[RT1], "&0")) {
			p->op = CLR;
			strcpy(regs[RT1], regs[RT2]);
			regs[RT2][0] = 0;
			p->code = copy(1, regs[RT1]);
			goto sngl;
		}
		if (p->op==OR && equstr(regs[RT1], "&0")) {
			if (p->forw->op!=CBR) {
				p->back->forw = p->forw;
				p->forw->back = p->back;
				nchange++;
				continue;
			}
		}
		repladdr(p, 0, flt);
		source(regs[RT1]);
		dest(regs[RT2], flt);
		ccloc[0] = 0;
		continue;

	case NEGF:
		flt = NREG;

	case CLR:
	case COM:
	case INC:
	case DEC:
	case NEG:
	case SXT:
		singop(p);
	sngl:
		dest(regs[RT1], flt);
		if (p->op==CLR && flt==0)
			if ((r = isreg(regs[RT1])) >= 0)
				savereg(r, "&0", p->subop);
			else
				setcon("&0", regs[RT1]);
		ccloc[0] = 0;
		continue;

	case TSTF:
		flt = NREG;

	case TST:
		singop(p);
		repladdr(p, 0, flt);
		source(regs[RT1]);
		if (equstr(regs[RT1], ccloc) && p->subop == ccmode) {
			if( regs[RT1][0] != '%' || regs[RT1][1] != 'a' ) {
				p->back->forw = p->forw;
				p->forw->back = p->back;
				p = p->back;
				nrtst++;
				nchange++;
				continue;
			}
		}
		singop(p);
		if( regs[RT1][0] == '%' && regs[RT1][1] == 'a' ) {
			p->op = CMP;
			p->subop = WORD;
			p->code = copy(2, regs[RT1], ",&0");
			nchange++;
		}
		continue;

	case CMPF:
		flt = NREG;

	case CMP:
	case BIT:
		dualop(p);
		source(regs[RT1]);
		source(regs[RT2]);
		if(p->op==BIT) {
			if (equstr(regs[RT1], "&-1") || equstr(regs[RT1], "&177777")) {
				p->op = TST;
				strcpy(regs[RT1], regs[RT2]);
				regs[RT2][0] = 0;
				p->code = copy(1, regs[RT1]);
				nchange++;
				nsaddr++;
			} else if (equstr(regs[RT2], "&-1") || equstr(regs[RT2], "&177777")) {
				p->op = TST;
				regs[RT2][0] = 0;
				p->code = copy(1, regs[RT1]);
				nchange++;
				nsaddr++;
			}
			if (equstr(regs[RT1], "&0")) {
				p->op = TST;
				regs[RT2][0] = 0;
				p->code = copy(1, regs[RT1]);
				nchange++;
				nsaddr++;
			} else if (equstr(regs[RT2], "&0")) {
				p->op = TST;
				strcpy(regs[RT1], regs[RT2]);
				regs[RT2][0] = 0;
				p->code = copy(1, regs[RT1]);
				nchange++;
				nsaddr++;
			}
		}
		repladdr(p, 1, flt);
		ccloc[0] = 0;
		continue;

	case CBR:
		r = -1;
		if (p->back->op==TST || p->back->op==CMP) {
			if (p->back->op==TST) {
				singop(p->back);
				savereg(RT2, "&0", p->subop);
			} else
				dualop(p->back);
			if (equstr(regs[RT1], regs[RT2])
			 && natural(regs[RT1]) && natural(regs[RT2]))
				r = compare(p->subop, "&1", "&1");
			else
				r = compare(p->subop, findcon(RT1), findcon(RT2));
			if (r==0) {
				if (p->forw->op==CBR
				  || p->forw->op==SXT
				  || p->forw->op==CFCC) {
					p->back->forw = p->forw;
					p->forw->back = p->back;
				} else {
					p->back->back->forw = p->forw;
					p->forw->back = p->back->back;
				}
				decref(p->ref);
				p = p->back->back;
				nchange++;
			} else if (r>0) {
				p->op = JBR;
				p->subop = 0;
				p->back->back->forw = p;
				p->back = p->back->back;
				p = p->back;
				nchange++;
			}
		}
	case CFCC:
		ccloc[0] = 0;
		continue;

	case JBR:
		redunbr(p);

	default:
		clearreg();
	}
	}
}

jumpsw()
{
	register struct node *p, *p1;
	register t;
	register struct node *tp;
	int nj;

	t = 0;
	nj = 0;
	for (p=first.forw; p!=0; p = p->forw)
		p->refc = ++t;
	for (p=first.forw; p!=0; p = p1) {
		p1 = p->forw;
		if (p->op == CBR && p1->op==JBR && p->ref && p1->ref
		 && abs(p->refc - p->ref->refc) > abs(p1->refc - p1->ref->refc)) {
			if (p->ref==p1->ref)
				continue;
			p->subop = revbr[p->subop];
			tp = p1->ref;
			p1->ref = p->ref;
			p->ref = tp;
			t = p1->labno;
			p1->labno = p->labno;
			p->labno = t;
			nrevbr++;
			nj++;
		}
	}
	return(nj);
}

addsob()
{
	register struct node *p, *p1;

	for (p = &first; (p1 = p->forw)!=0; p = p1) {
		if (p->op==DEC && isreg(p->code)>=0
		 && p1->op==CBR && p1->subop==JNE) {
			if (p->refc < p1->ref->refc)
				continue;
			if (toofar(p1))
				continue;
			p->labno = p1->labno;
			p->op = SOB;
			p->subop = 0;
			p1->forw->back = p;
			p->forw = p1->forw;
			nsob++;
		}
	}
}

toofar(p)
struct node *p;
{
	register struct node *p1;
	int len;

	len = 0;
	for (p1 = p->ref; p1 && p1!=p; p1 = p1->forw)
		len += ilen(p1);
	if (len < 128)
		return(0);
	return(1);
}

ilen(p)
register struct node *p;
{

	switch (p->op) {
	case LABEL:
	case DLABEL:
	case TEXT:
	case EROU:
	case EVEN:
	case STABS:
		return(0);

	case CBR:
		return(6);

	default:
		dualop(p);
		return(2 + adrlen(regs[RT1]) + adrlen(regs[RT2]));
	}
}

adrlen(s)
register char *s;
{
	if (*s == 0)
		return(0);
	if (*s=='r')
		return(0);
	if (*s=='(' && *(s+1)=='r')
		return(0);
	if (*s=='-' && *(s+1)=='(')
		return(0);
	return(2);
}

abs(x)
{
	return(x<0? -x: x);
}

equop(ap1, p2)
struct node *ap1, *p2;
{
	register char *cp1, *cp2;
	register struct node *p1;

	p1 = ap1;
	if (p1->op!=p2->op || p1->subop!=p2->subop)
		return(0);
	if (p1->op>0 && p1->op<MOV)
		return(0);
	cp1 = p1->code;
	cp2 = p2->code;
	if (cp1==0 && cp2==0)
		return(1);
	if (cp1==0 || cp2==0)
		return(0);
	while (*cp1 == *cp2++)
		if (*cp1++ == 0)
			return(1);
	return(0);
}

decref(p)
register struct node *p;
{
	if (--p->refc <= 0) {
		nrlab++;
		p->back->forw = p->forw;
		p->forw->back = p->back;
	}
}

struct node *
nonlab(p)
struct node *p;
{
	CHECK(10);
	while (p && p->op==LABEL)
		p = p->forw;
	return(p);
}

char *
alloc(n)
register n;
{
	register char *p;

#define round(a,b) ((((a)+(b)-1)/(b))*(b))
	n=round(n,sizeof(char *));
	if (alasta+n < alastr) {
		p = alasta;
		alasta += n;
		return(p);
	}
	if (lasta+n >= lastr) {
		if (sbrk(2000) == (char *)-1) {
			fprintf(stderr, "C Optimizer: out of space\n");
			exit(1);
		}
		lastr += 2000;
	}
	p = lasta;
	lasta += n;
	return(p);
}

clearreg()
{
	register int i;

	for (i=0; i<2*NREG; i++)
		regs[i][0] = '\0';
	conloc[0] = 0;
	ccloc[0] = 0;
}

savereg(ai, as, sub)
char *as;
{
	register char *p, *s, *sp;
	int parcnt=0;

	sp = p = regs[ai];
	reg_type[ai] = 0;
	s = as;
	if (source(s))
		return;
	while (*s) {
		switch( *s ) {
		case '(':
			if( *(s+1)=='%' && *(s+2)=='a' && *(s+3)<'5') {
				*sp = 0;
				return;
			}
			parcnt=1;
			break;
		case ')':
			parcnt=0;
			break;
		case ',':
			if( !parcnt )
				goto done;
		}
		*p++ = *s++;
	}
 done:
	*p++ = 0;
	reg_type[ai] = sub;
}

dest(as, flt)
char *as;
{
	register char *s;
	register int i;

	s = as;
	source(s);
	if ((i = isreg(s)) >= 0)
		regs[i+flt][0] = 0;
	for (i=0; i<NREG+NREG; i++)
		if (*regs[i]=='*' && equstr(s, regs[i]+1))
			regs[i][0] = 0;
	if (equstr(s, conloc))
		conloc[0] = '\0';
	while ((i = findrand(s, flt, -1)) >= 0)
		regs[i][0] = 0;
	while (*s) {
		if ((*s=='(' && (*(s+1)!='%' || *(s+2)!='a' 
			  || *(s+2)!='f')) || *s++=='*') {
			for (i=0; i<NREG+NREG; i++) {
				if (regs[i][0] != '&')
					regs[i][0] = 0;
				conloc[0] = 0;
			}
			return;
		}
	}
}

singop(ap)
struct node *ap;
{
	register char *p1, *p2;

	p1 = ap->code;
	p2 = regs[RT1];
	while (*p2++ = *p1++);
	regs[RT2][0] = 0;
}


dualop(ap)
struct node *ap;
{
	register char *p1, *p2;
	register struct node *p;
	int parcnt = 0;

	p = ap;
	p1 = p->code;
	p2 = regs[RT1];
	while (*p1) {
		switch( *p1 ) {
		case '(':
			parcnt=1;
			break;
		case ')':
			parcnt=0;
			break;
		case ',':
			if( !parcnt )
				goto done;
		}
		*p2++ = *p1++;
	}
 done:
	*p2++ = 0;
	p2 = regs[RT2];
	*p2 = 0;
	if (*p1++ !=',')
		return;
	while (*p1==' ' || *p1=='\t')
		p1++;
	while (*p2++ = *p1++)
		;
}



islit(cp)
char *cp;
{
    if (*cp++=='&' && *cp >= '0' && *cp <= '8' && *++cp==0) {
	return(1);
    }
    return(0);
}

isd01(cp)
char *cp;
{
    if (*cp++=='%' && *cp++=='d' && (*cp=='0' || *cp=='1')) {
	return(1);
    }
    return(0);
}

isd0(cp)
char *cp;
{
    if (*cp++=='%' && *cp++=='d' && *cp=='0') {
	return(1);
    }
    return(0);
}

isd1(cp)
char *cp;
{
    if (*cp++=='%' && *cp++=='d' && *cp=='1') {
	return(1);
    }
    return(0);
}

ispow2(cp)
char *cp;
{
    int i = 0;

    if (*cp++=='&') {
	sscanf(cp,"%d",&i);
	switch (i) {
	case 1:
	case 2:
	case 4:
	case 8:
	case 16:
	case 32:
	case 64:
	case 128:
	    return(1);
	default:
	    return(0);
	}
    }
    return(0);
}

log2(cp)
char *cp;
{
    int i = 0, j;

    if (*cp++=='&') {
	sscanf(cp,"%d",&j);
	switch (j) {
	case 128:
	    i++;
	case 64:
	    i++;
	case 32:
	    i++;
	case 16:
	    i++;
	case 8:
	    i++;
	case 4:
	    i++;
	case 2:
	    i++;
	case 1:
	    i++;
	    return(i);
	default:
	    return(0);
	}
    }
    return(0);
}

nextdual(ap)
struct node *ap;
{
	register char *p1, *p2;
	register struct node *p;
	int parcnt = 0;

	p = ap;
	p1 = p->code;
	p2 = nxtregs[0];
	while (*p1) {
		switch( *p1 ) {
		case '(':
			parcnt=1;
			break;
		case ')':
			parcnt=0;
			break;
		case ',':
			if( !parcnt )
				goto done;
		}
		*p2++ = *p1++;
	}
 done:
	*p2++ = 0;
	p2 = nxtregs[1];
	*p2 = 0;
	if (*p1++ !=',')
		return;
	while (*p1==' ' || *p1=='\t')
		p1++;
	while (*p2++ = *p1++)
		;
}

findrand(as, flt, sub)
char *as;
{
	register int i;
	for (i = flt; i<NREG+flt; i++) {
		if (equstr(regs[i], as) && (sub<0 || reg_type[i] == sub))
			return(i);
	}
	return(-1);
}

isreg(as)
char *as;
{
	register char *s;

	s = as;
	if (s[0]=='%' && s[1]=='d' && s[2]>='0' && s[2]<='7' && s[3]==0)
		return(s[2]-'0');
	if (s[0]=='%' && s[1]=='a' && s[2]>='0' && s[2]<='7' && s[3]==0)
		return(8+s[2]-'0');
	return(-1);
}

check()
{
	register struct node *p, *lp;
	register count;

	lp = &first;
	count = 0;
	for (p=first.forw; p!=0; p = p->forw) {
		if (++count > 10000)
			abort(0);
		if (p->back != lp)
			abort(1);
		lp = p;
	}
}

source(ap)
char *ap;
{
	register char *p1, *p2;

	p1 = ap;
	p2 = p1;
	if (*p1==0)
		return(0);
	while (*p2++);
	if (*p1=='-' && *(p1+1)=='('
	 || *p1=='*' && *(p1+1)=='-' && *(p1+2)=='('
	 || *(p2-2)=='+') {
		while (*p1 && *p1++!='%');
		if (*p1++=='d' && *p1>='0' && *p1 <='8')
			regs[*p1 - '0'][0] = 0;
		else if (*(p1-1)=='a' && *p1>='0' && *p1 <='5')
			regs[*p1 - '0' + 8][0] = 0;
		return(1);
	}
	return(0);
}

repladdr(p, f, flt)
struct node *p;
{
	register r;
	int r1;
	register char *p1, *p2;
	static char rt1[50], rt2[50];

	if (f)
		r1 = findrand(regs[RT2], flt, p->subop);
	else
		r1 = -1;
	r = findrand(regs[RT1], flt, p->subop);
	if (r1 >= NREG)
		r1 -= NREG;
	if (r >= NREG)
		r -= NREG;
	if (r>=0 || r1>=0) {
		p2 = regs[RT1];
		for (p1 = rt1; *p1++ = *p2++;);
		if (regs[RT2][0]) {
			p1 = rt2;
			*p1++ = ',';
			for (p2 = regs[RT2]; *p1++ = *p2++;);
		} else
			rt2[0] = 0;
		if (r>=0) {
			rt1[0] = '%';
			rt1[1] = (r < 8 ? 'd' : 'a');
			rt1[2] = (r < 8 ? r : r-8) + '0';
			rt1[3] = 0;
			nsaddr++;
		}
		if (r1>=0) {
			rt2[1] = '%';
			rt2[2] = (r1 < 8 ? 'd' : 'a');
			rt2[3] = (r1 < 8 ? r1 : r1-8) + '0';
			rt2[4] = 0;
			nsaddr++;
		}
		p->code = copy(2, rt1, rt2);
	}
}

movedat()
{
	register struct node *p1, *p2;
	struct node *p3;
	register seg;
	register char *segno;
	struct node data;
	struct node *datp;

	if (first.forw == 0)
		return;
	if (lastseg != TEXT && lastseg != -1) {
		p1 = (struct node *)alloc(sizeof(first));
		p1->op = lastseg;
		p1->subop = 0;
		p1->code = lastnseg;
		p1->forw = first.forw;
		p1->back = &first;
		first.forw->back = p1;
		first.forw = p1;
	}
	datp = &data;
	for (p1 = first.forw; p1!=0; p1 = p1->forw) {
		if (p1->op == DATA) {
			p2 = p1->forw;
			while (p2 && p2->op!=TEXT)
				p2 = p2->forw;
			if (p2==0)
				break;
			p3 = p1->back;
			p1->back->forw = p2->forw;
			p2->forw->back = p3;
			p2->forw = 0;
			datp->forw = p1;
			p1->back = datp;
			p1 = p3;
			datp = p2;
		}
		if (p1->op == SET) {
			char *cp = p1->code;
			int functno, mask;

			if(sscanf(p1->code, "M%%%d,0%o", &functno, &mask)==2 ) {
				Func_mask[functno] = mask;
			}
/*
			p2 = p1->forw;
			if (p2==0)
				break;
			p3 = p1->back;
			p1->back->forw = p2->forw;
			p2->forw->back = p3;
			p2->forw = 0;
			datp->forw = p1;
			p1->back = datp;
			p1 = p3;
			datp = p2;
*/
		}
	}
	if (data.forw) {
		datp->forw = first.forw;
		first.forw->back = datp;
		data.forw->back = &first;
		first.forw = data.forw;
	}
	seg = lastseg;
	segno = lastnseg;
	for (p1 = first.forw; p1!=0; p1 = p1->forw) {
		if (p1->op==TEXT||p1->op==DATA||p1->op==BSS) {
			if (p2 = p1->forw) {
				if (p2->op==TEXT||p2->op==DATA||p2->op==BSS) {
					p1->op  = p2->op;
					p1->code = p2->code;
				}
			}
			if ((p1->op==seg && equstr(p1->code,segno)) ||
			 (p1->forw && p1->forw->op==seg && equstr(p1->forw->code,segno))) {
				p1->back->forw = p1->forw;
				p1->forw->back = p1->back;
				p1 = p1->back;
				continue;
			}
			seg = p1->op;
			segno = p1->code;
		}
	}
}

redunbr(p)
register struct node *p;
{
	register struct node *p1;
	register char *ap1;
	char *ap2;

	if ((p1 = p->ref) == 0)
		return;
	p1 = nonlab(p1);
	if (p1->op==TST) {
		singop(p1);
		savereg(RT2, "&0", p->subop);
	} else if (p1->op==CMP)
		dualop(p1);
	else
		return;
	if (p1->forw->op!=CBR)
		return;
	ap1 = findcon(RT1);
	ap2 = findcon(RT2);
	p1 = p1->forw;
	if (compare(p1->subop, ap1, ap2)>0) {
		nredunj++;
		nchange++;
		decref(p->ref);
		p->ref = p1->ref;
		p->labno = p1->labno;
		p->ref->refc++;
	}
}

char *
findcon(i)
{
	register char *p;
	register r;

	p = regs[i];
	if (*p=='&')
		return(p);
	if ((r = isreg(p)) >= 0)
		return(regs[r]);
	if (equstr(p, conloc))
		return(conval);
	return(p);
}

compare(oper, cp1, cp2)
register char *cp1, *cp2;
{
	register unsigned n1, n2;

	if (*cp1++ != '&' || *cp2++ != '&')
		return(-1);
	n1 = 0;
	while (*cp2 >= '0' && *cp2 <= '7') {
		n1 <<= 3;
		n1 += *cp2++ - '0';
	}
	n2 = n1;
	n1 = 0;
	while (*cp1 >= '0' && *cp1 <= '7') {
		n1 <<= 3;
		n1 += *cp1++ - '0';
	}
	if (*cp1=='+')
		cp1++;
	if (*cp2=='+')
		cp2++;
	do {
		if (*cp1++ != *cp2)
			return(-1);
	} while (*cp2++);
	switch(oper) {

	case JEQ:
		return(n1 == n2);
	case JNE:
		return(n1 != n2);
	case JLE:
		return((int)n1 <= (int)n2);
	case JGE:
		return((int)n1 >= (int)n2);
	case JLT:
		return((int)n1 < (int)n2);
	case JGT:
		return((int)n1 > (int)n2);
	case JLO:
		return(n1 < n2);
	case JHI:
		return(n1 > n2);
	case JLOS:
		return(n1 <= n2);
	case JHIS:
		return(n1 >= n2);
	}
	return(-1);
}

setcon(ar1, ar2)
char *ar1, *ar2;
{
	register char *cl, *cv, *p;

	cl = ar2;
	cv = ar1;
	if (*cv != '&')
		return;
	if (!natural(cl))
		return;
	p = conloc;
	while (*p++ = *cl++);
	p = conval;
	while (*p++ = *cv++);
}

equstr(ap1, ap2)
char *ap1, *ap2;
{
	char *p1, *p2;

	p1 = ap1;
	p2 = ap2;
	if( p1 == 0 || p2 == 0 )
		return( p1 == p2 );
	do {
		if (*p1++ != *p2)
			return(0);
	} while (*p2++);
	return(1);
}

setcc(ap)
char *ap;
{
	register char *p, *p1;

	p = ap;
	if (!natural(p)) {
		ccloc[0] = 0;
		return;
	}
	p1 = ccloc;
	while (*p1++ = *p++);
}

natural(ap)
char *ap;
{
	register char *p;

	p = ap;
	if (*p=='*' || *p=='(' || *p=='-'&&*(p+1)=='(')
		return(0);
	while (*p++);
	p--;
	if (*--p == '+' || *p ==')' /* && *--p != '5' */ )
		return(0);
	return(1);
}

