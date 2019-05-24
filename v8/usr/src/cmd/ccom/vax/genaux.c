#include "gencode.h"
#define NDNUM 400
NODE myt[NDNUM];
int ntree;
extern int bothdebug, nosharp;
#if M32 == 1
char *regnames[] = {"%r0", "%r1", "%r2", "%r3", "%r4", "%r5", "%r6", "%r7",
	"%r8", "%fp", "%ap", "%r11", "%sp", "%r13", "%r14", "%pc"};
char *frameptr = "%fp";
char *argptr = "%ap";
char *jeq = "je";
char *jne = "jne";
char *jgt = "jg";
char *jge = "jge";
char *jlt = "jl";
char *jle = "jle";
char *jugt = "jgu";
char *juge = "jgeu";
char *jult = "jlu";
char *jule = "jleu";
#define CHARCHAR 'b'
#define SHORTCHAR 'h'
#define LONGCHAR 'w'
#define FLOATCHAR 's'
#define DOUBLECHAR 'd'
#else if VAX == 1
char *regnames[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
	"r9", "r10", "r11", "r12", "r13", "r14", "r15"};
char *frameptr = "fp";
char *argptr = "ap";
char *jeq = "jeql";
char *jne = "jneq";
char *jgt = "jgtr";
char *jge = "jgeq";
char *jlt = "jlss";
char *jle = "jleq";
char *jugt = "jgtru";
char *juge = "jgequ";
char *jult = "jlssu";
char *jule = "jlequ";
#define CHARCHAR 'b'
#define SHORTCHAR 'w'
#define LONGCHAR 'l'
#define FLOATCHAR 'f'
#define DOUBLECHAR 'd'
#endif
char *bufend = (char *)bufs + sizeof(bufs);
pr(fmt, list)
char *fmt; long list;
{
	char *sprintxl();
	nosharp = !bothdebug;
	prptr = sprintxl(prptr, fmt, &list);
	if(prptr > prbuf + sizeof(prbuf))
		cerror("prbuf overflow");
	nosharp = 0;
}

outpr()
{
	*prptr = 0;
	printbuf(prbuf, prptr-prbuf);
}
NODE *
gimmenode()
{	NODE *p;
	p = myt + ntree++;
	if(ntree > NDNUM)
		cerror("out of temporary trees");
	return(p);
}

NODE *
tempnode(p, flag)
NODE *p;
{	NODE *x, *q = p->in.left;
	int n;
	extern int minrvar;
	if(p->in.op == CONV)
		q = p;	/* CONV to double of float, versus (CMP double double) */
	x = gimmenode();
	x->in.type = q->in.type;
	n = incrsize(q) == 8? 2: 1;
	if(!(flag & ASADDR) && regvar >= REGVAR - 1 + n) {
		x->in.op = REG;
		x->tn.rval = regvar + 1 - n;
		x->tn.lval = 1;	/* SCRATCH marker !!!!! */
		regvar -= n;
		if(minrvar > regvar)
			minrvar = regvar;	/* in case current routine recursive */
		return(x);
	}
	x->in.op = VAUTO;
	x->tn.lval = gimmetemp(n);
	/* scratch marker? */
	return(x);
}
ret
alloctmp(p)
NODE *p;
{	ret s;
	sprintf(buf, "%d(%s)", gimmetemp(incrsize(p) == 8? 2: 1), frameptr);
	done(s, CANINDIR|SCRATCH, 0);
}
ret
checksize(p, s, regmask)
NODE *p;
ret s;
{	ret t;
	if(p->in.type != TDOUBLE)
		return(s);
	regmask |= s.regmask;
	t = allocreg(p, regmask);
	return(t);
}

ret
allocreg(p, regmask)
NODE *p;
{	int i, n;
	ret s;
	NODE *x;
	if(p->in.type == TDOUBLE)
		n = 2;
	else
		n = 1;
	for(i = 0; i < REGVAR; i++) {
		if(!(regmask & (1 << i)))
			continue;
		if(n == 2 & !(regmask & (1 << (1+i))))
			continue;
		sprintx(buf, "%s", regnames[i]);
		regmask = (1 << i);
		if(n == 2)
			regmask |= (1 << (i+1));
		done(s, SCRATCH|ISREG, regmask);
	}
	x = tempnode(p, 0);
	if(x->in.op == REG) {
		sprintx(buf, "%s", regnames[x->tn.rval]);
		done(s, SCRATCH|ISREG, 0);
	}
	sprintx(buf, "%d(%s)", x->tn.lval, frameptr);
	done(s, SCRATCH|CANINDIR, 0);
}

gimmetemp(n)
{
	if(VAX)
		return(freetemp(n)/8 - maxboff/SZCHAR);
	else
		return(freetemp(n)/8 + maxboff/SZCHAR);
}

isfloat(p)
NODE *p;
{
	return(p->in.type == TFLOAT || p->in.type == TDOUBLE);
}

childtype(p)
NODE *p;
{
	return(type(p->in.left));
}

type(p)
NODE *p;
{	int n = p->in.type;
	switch(n) {
	default:
		return('?');
	case TCHAR: case TUCHAR:
		return(CHARCHAR);
	case TSHORT: case TUSHORT:
		return(SHORTCHAR);
	case TINT: case TUNSIGNED: case TLONG: case TULONG:
	case TPOINT: case TSTRUCT:
		return(LONGCHAR);
	case TFLOAT:
		return(FLOATCHAR);
	case TDOUBLE:
		return(DOUBLECHAR);
	}
}

shiftsize(p)
NODE *p;
{
	switch(p->in.type) {
	default:
		return(0);	/* 0 can't occur in an ICON under shift */
	case TCHAR: case TUCHAR:
		return(0);
	case TSHORT: case TUSHORT:
		return(1);
	case TINT: case TUNSIGNED: case TLONG: case TULONG: case TPOINT:
	case TFLOAT:
		return(2);
	case TDOUBLE:
		return(3);
	}
}	

incrsize(p)
NODE *p;
{
	switch(p->in.type) {
	default:
		return(0);	/* 0 can't occur in an icon under incrop */
	case TCHAR: case TUCHAR:
		return(1);
	case TSHORT: case TUSHORT:
		return(2);
	case TINT: case TUNSIGNED: case TLONG: case TULONG: case TPOINT:
	case TFLOAT:
		return(4);
	case TDOUBLE:
		return(8);
	}
}
	
isunsigned(p)
NODE *p;
{
	switch(p->in.type) {
#if VAX==1
	case TCHAR:
#endif
	case TSHORT: case TINT: case TLONG: case TFLOAT: case TDOUBLE:
		return(0);
#if M32==1
	case TCHAR:
#endif
	default:
		return(1);
	}
}

NODE *
copytree(p)
NODE *p;
{	NODE *a, *b, *c;
	switch(p->in.op) {
	case ASG AND: case AND: case CALL: case CMP: case COMOP:
	case DECR: case ASG DIV: case DIV: case ASG ER: case ER:
	case INCR: case ASG LS: case LS: case ASG MINUS: case MINUS:
	case ASG MOD: case MOD: case ASG MUL: case MUL: case ASG OR:
	case OR: case ASG PLUS: case PLUS: case ASG RS: case RS:
	case STASG: case STCALL: case ASSIGN: case CM:
		a = copytree(p->in.left);
		b = copytree(p->in.right);
		c = gimmenode();
		*c = *p;
		c->in.left = a;
		c->in.right = b;
		return(c);
	case COMPL: case CONV: case FLD: case GENBR: case GENLAB:
	case GENUBR: case STAR: case UNARY AND: case UNARY CALL:
	case UNARY MINUS: case UNARY STCALL: case INIT: case FUNARG:
	case STARG:
		a = copytree(p->in.left);
		c = gimmenode();
		*c = *p;
		c->in.left = a;
		return(c);
	case VAUTO: case REG: case NAME: case VPARAM: case ICON:
	case SNODE: case RNODE: case QNODE:
		c = gimmenode();
		*c = *p;
		return(c);
	default:
		cerror("unk node in copytree");
	}
}
/* rewrit A op B into (T = A, T) op B or A op (T = B, T) */
totemp(p, flag)
NODE *p;
{	NODE *a, *b, *c;
	a = gimmenode();
	b = tempnode(p, flag);
	c = gimmenode();
	if(flag & LEFT)
		*a = *p->in.left;
	else
		*a = *p->in.right;
	a->in.op = ASSIGN;
	a->in.left = b;
	if(flag & LEFT)
		a->in.right = p->in.left;
	else
		a->in.right = p->in.right;
	c->in.op = COMOP;
	c->in.left = a;
	c->in.right = b;
	c->in.type = a->in.type;
	if(flag & LEFT)
		p->in.left = c;
	else
		p->in.right = c;
}

funargs(p, regmask)
NODE *p;
{	ret s, t;
	int i;
	switch(p->in.op) {
	case CM:	/* order depends on way stack grows */
#ifdef LTORARGS
		i = funargs(p->in.left, regmask);
		i |= funargs(p->in.right, regmask);
#else
		i = funargs(p->in.right, regmask);
		i |= funargs(p->in.left, regmask);
#endif
		return(i);
	case FUNARG:
#if VAX==1
		t = tostack();
		s = doit(p->in.left, VALUE|TOSTACK, t, regmask);
#else if M32==1
		s = doit(p->in.left, VALUE|TOSTACK, 0, regmask);
#endif
		return(s.flag & FAIL);
	case STARG:
		/* this has to have same cases as STASG */
		if(regmask != REGMASK)
			return(FAILX);
		s = doit(p->in.left, VAX?(ASADDR|VALUE):VALUE, 0, regmask);
		/* this generates expensive code for small structures */
		i = p->stn.stsize / 8;
		if(p->in.left->in.op == STASG) {
			if(VAX && i != 4 && i != 8) {
				pr("#\tsubl2\t$%d,sp\n", i);
				pr("#\tsubl2\t$%d,r3\n", i);
				pr("#\tmovc3\t$%d,(r3),(sp)\n", p->stn.stsize/8);
				return(0);
			}
			else if(M32 && i >= 7 * 4) {
				pr("#\taddw2\t&%d,%%sp\n", i);
				pr("#\tsubw2\t&%d,%%r0\n", i/4);
				goto moveit;
			}
		}
		if(VAX) {
			if(i == 4)
				pr("#\tpushl\t%s\n", str(s));
			else if(i == 8)
				pr("#\tmovq\t%s,-(sp)\n", str(s));
			else {
				pr("#\tsubl2\t$%d,sp\n", i);
				pr("#\tmovc3\t$%d,%s,(sp)\n", p->stn.stsize/8, str(s));
			}
		}
		else if(M32) {
			pr("#\taddw2\t&%d,%%sp\n", i);
			if(strcmp(str(s), "%r0"))
				pr("#\tmovw\t%s,%r0\n", str(s));
moveit:
			pr("#\tmovaw\t-%d(%%sp),%r1\n", p->stn.stsize/8);
			i = p->stn.stsize/32;
			if(i >= 7) {
				pr("#\tmovw\t&%d,%%r2\n", i);
				pr("#\tMOVBLW\n");
			}
			else
				while(--i >= 0)
					pr("#\tmovw\t%d(%r0),%d(%r1)\n", 4*i, 4*i);
		}
		return(s.flag & FAIL);
	}
}

strshift(s, n)
char *s;
{	int i, j;
		i = strlen(s);
	if(n > 0)
		for(j = i; j >= 0; j--)
			s[j + n] = s[j];
	else
		for(j = -n; j <= i; j++)
			s[j + n] = s[j];
}

ret
tostack()
{	ret s;
	sprintx(buf, "-(sp)");
	done(s, 0, 0);
}

ret
specialreg(p, regmask)
NODE *p;
{	ret s;
	int n, i;
	n = p ->in.type == TDOUBLE? 2: 1;
	i = 1;
	if(n == 2)
		i = 3;
	sprintx(buf, "%s", regnames[0]);
	if(i & ~regmask)
		pr("#\specialreg not free\n");
	done(s, SCRATCH|ISREG, i);
}
/* these guys rewrite a1 = a2 = ... an = x
 * as t = x; an = t; an-1 = an */
NODE *
fromtemp(p, temp)
NODE *p, *temp;
{	NODE *q;
	q = gimmenode();
	*q = *p;
	q->in.left = p;
	q->in.op = ASSIGN;
	q->in.right = temp;
	return(q);
}

NODE *
to(p, temp)
NODE *p, *temp;
{	NODE *q;
	q = gimmenode();
	*q = *p;
	q->in.right = p;
	q->in.op = ASSIGN;
	q->in.left = temp;
	return(q);
}

NODE *
comnode(a, p)
NODE *a, *p;
{	NODE *q;
	q = gimmenode();
	*q = *p;
	q->in.op = COMOP;
	q->in.right = p;
	q->in.left = a;
	return(q);
}

asgwrite(p)
NODE *p;
{	NODE *q, *a, *temp;
	temp = tempnode(p, 0);
	a = fromtemp(p->in.left, temp);
	for(q = p->in.right; q->in.op == ASSIGN; q = q->in.right)
		a = comnode(fromtemp(q->in.left, temp), a);
	a = comnode(to(q, temp), a);
	*p = *a;
}

char *
genjmp(n)
{
	switch(n) {
	default:
		return("jweird");
	case EQ:
		return(jeq);
	case NE:
		return(jne);
	case GT:
		return(jgt);
	case GE:
		return(jge);
	case LT:
		return(jlt);
	case LE:
		return(jle);
	case UGT:
		return(jugt);
	case UGE:
		return(juge);
	case ULT:
		return(jult);
	case ULE:
		return(jule);
	}
}

lsconv(p)
NODE *p;
{	NODE *lft, *right;
	lft = gimmenode();
	right = gimmenode();
	lft->in.op = right->in.op = CONV;
	lft->in.type = right->in.type = TLONG;
	lft->in.left = p->in.left;
	right->in.left = p->in.right;
	if(p->in.left->in.op != ICON)	/* ICONS are longs anyway (see RS) */
		p->in.left = lft;
	p->in.right = right;
}
/* only some ops have to be rewritten (addb and addl are the same at the bottom) */
rewriteasgop(p)
NODE *p;
{	NODE *a, *newop;
	if(p->in.left->in.op != CONV)
		return(0);
	if(p->in.left->in.left->in.op == STAR)
		longjmp(back, awfulstar(p));
	newop = gimmenode();
	*newop = *p;
	switch(p->in.op) {
	case ASG DIV:
		newop->in.op = DIV;
		break;
	case ASG LS:
		newop->in.op = LS;
		break;
	case ASG MOD:
		newop->in.op = MOD;
		break;
	case ASG RS:
		newop->in.op = RS;
		break;
	case ASG PLUS:
		if(incrsize(p->in.left) != 8)
			return(0);
		newop->in.op = PLUS;
		break;
	case ASG MINUS:
		if(incrsize(p->in.left) != 8)
			return(0);
		newop->in.op = MINUS;
		break;
	case ASG MUL:
		if(incrsize(p->in.left) != 8)
			return(0);
		newop->in.op = MUL;
		break;

	default:
		cerror("codegen: rewriting asgop");
	}
	a = gimmenode();
	*a = *p->in.left->in.left;
	p->in.op = ASSIGN;
	p->in.left = a;
	p->in.right = newop;
	return(1);
}

rewriteconv(p)	/* uns to float or double */
NODE *p;
{	NODE *a;
	a = gimmenode();
	*a = *p;
	a->in.type = TLONG;
	p->in.left = a;
}

mediumstar(p)
NODE *p;
{	NODE *newtop, *tmp, *x, *y;
	newtop = gimmenode();
	newtop->in.op = COMOP;
	newtop->in.type = p->in.type;
	x = gimmenode();
	x->in.op = ASSIGN;
	x->in.type = TPOINT;
	x->in.right = p->in.left->in.left;
	newtop->in.left = x;
	tmp = tempnode(newtop, 0);
	tmp->in.type = TPOINT;
	x->in.left = tmp;
	y = gimmenode();
	y->in.op = COMOP;
	y->in.type = p->in.type;
	x = gimmenode();
	*x = *p;
	newtop->in.right = y;
	y->in.left = x;
	y->in.right = x->in.left;
	x->in.left->in.left = tmp;
	*p = *newtop;
	return(1);
}

awfulstar(p)
NODE *p;
{	NODE *newtop, *doleft, *x, *op, *equals;
	newtop = gimmenode();
	newtop->in.op = COMOP;
	newtop->in.type = p->in.type;
	doleft = gimmenode();
	doleft->in.op = ASSIGN;
	doleft->in.type = TPOINT;
	doleft->in.right = p->in.left->in.left->in.left;
	x = tempnode(doleft, 0);
	doleft->in.left = x;
	op = gimmenode();
	*op = *p;
	op->in.op -= (ASG 0);	/* crap for crap */
	op->in.left->in.left->in.left = x;	/* so it's a dag, but x is harmless */
	equals = gimmenode();
	*equals = *p;
	equals->in.right = op;
	equals->in.left = p->in.left->in.left;
	equals->in.op = ASSIGN;
	*p = *newtop;
	p->in.left = doleft;
	p->in.right = equals;
	return(1);
}
rewritefld(p)
NODE *p;
{	NODE *q, *tmp, *left, *comop;
	if((q = p->in.left->in.left)->in.op != STAR) {
		rewfld(p);
		return;
	}
	q = q->in.left;
	tmp = tempnode(q, 0);
	left = gimmenode();
	left->in.type = q->in.type;
	left->in.op = ASSIGN;
	left->in.left = tmp;
	left->in.right = gimmenode();
	left->in.right = copytree(q);
	*q = *tmp;
	comop = gimmenode();
	comop->in.op = COMOP;
	comop->in.left = left;
	comop->in.right = copytree(p);
	*p = *comop;
	rewfld(p->in.right);
	longjmp(back, 1);
}
rewfld(p)
NODE *p;
{	NODE *x, *y, *z;
	x = gimmenode();
	*x = *copytree(p);
	y = gimmenode();
	*y = *copytree(p);
	z = gimmenode();
	*z = *p;
	z->in.left = x;
	x->in.op = ASSIGN;
	x->in.right = y;
	if(p->in.op == DECR || p->in.op == ASG MINUS)
		z->in.op = PLUS;
	else
		z->in.op = MINUS;
	if(p->in.op == DECR)
		y->in.op = MINUS;
	else if(p->in.op == INCR)
		y->in.op = PLUS;
	else
		y->in.op -= (ASG 0);	/* good grief */
	if(p->in.op == DECR || p->in.op == INCR)
		*p = *z;
	else
		*p = *x;
}

ret
indirit(s)
ret s;
{
	if(s.flag & ISREG) {
		strcat(str(s), ")");
		strshift(str(s), 1);
		str(s)[0] = '(';
		return(s);
	}
	if(s.flag & CANINDIR) {
		strshift(str(s), 1);
		str(s)[0] = '*';
		return(s);
	}
	if(VAX && str(s)[0] == '(') {	/* (r3)[r11] */
		strshift(str(s), 1);
		str(s)[0] = '*';
		return(s);
	}
	if(VAX && str(s)[0] == '$') {	/* an icon for structure returns */
		strshift(str(s), -1);
		return(s);
	}
	s.flag = FAIL;
	return(s);
}
/* not int = (...? int exprs) have a bogus tree */
extracheck(p)
NODE *p;
{	NODE *a;
	if(p->in.right->in.op != GENLAB)
		return;
	a = gimmenode();
	*a = *p;
	a->in.left = p->in.right;
	a->in.op = CONV;
	p->in.right = a;
}

starasg(p)
NODE *p;
{	NODE *tmp, *asg, *x;
	tmp = tempnode(p->in.left->in.left, 0);
	x = gimmenode();
	*x = *p;
	asg = gimmenode();
	*asg = *p->in.left->in.left;
	asg->in.op = ASSIGN;
	asg->in.left = tmp;
	asg->in.right = p->in.left->in.left;
	x->in.left->in.left = tmp;
	p->in.op = COMOP;
	p->in.left = asg;
	p->in.right = x;
}
stasgrewrite(p)
NODE *p;
{	NODE *qa, *qb, *n, *left, *right;
	if(VAX) {
		totemp(p, RIGHT);
		longjmp(back, 1);
	}
	if(M32) {
		qa = tempnode(p->in.right, 0);
		qb = tempnode(p->in.left, 0);
		right = to(p->in.right, qa);
		left = to(p->in.left, qb);
		p->in.right = qa;
		p->in.left = qb;
		qa = gimmenode();
		qb = gimmenode();
		qa->in.op = qb->in.op = COMOP;
		qa->in.left = left;
		qa->in.right = qb;
		qb->in.left = right;
		n = gimmenode();
		*n = *p;
		qb->in.right = n;
		*p = *qa;
		longjmp(back, 1);
	}
}
ret
simpler(a, b)	/* returns b (as dest) preferentially */
ret a, b;
{
	if(b.flag & ISREG)
		return(b);
	if(a.flag & ISREG)
		return(a);
	if(b.flag & SCRATCH)
		return(b);
	if(a.flag & SCRATCH)
		return(a);
	if(!(b.flag & INDEX))
		return(b);
	if(!(a.flag & INDEX))
		return(a);
	/* disallow *p++ = *q++ but not p[i]=q[i]*/
	if(!index(str(b), '-') && !index(str(b), '+'))
		return(b);
	if(!index(str(a), '-') && !index(str(a), '+'))
		return(a);
	b.flag |= USED;
	return(b);
}
