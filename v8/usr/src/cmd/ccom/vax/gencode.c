#include "gencode.h"
#define fieldbotch(p) if(p->in.left->in.op == FLD) {rewritefld(p); longjmp(back, 1);}
jmp_buf back;
int acnt, Pflag, bbcnt;

gencode(p)
NODE *p;
{	NODE *q;
	ret s;
	int svtemp, svregvar, i, svbb;
	extern int bothdebug;
	svtemp = tmpoff;
	svregvar = regvar;
	svbb = ++bbcnt;
	if(setjmp(back)) {
		pr("#\treg\t%d\n", ++acnt)/*, prtree(q), putchar('\n')*/;
		if(acnt > 20) {
			prtree(q);
			outpr();
			tmpoff = svtemp;
			bbcnt = svbb;
			uerror("expression too complicated");
			return;
		}
	}
	else
		q = copytree(p);
	buf = bufs[1];
	prptr = prbuf;
	if(Pflag && q->in.op != INIT) {
		pr("#\tincl\tlocprof+%d\n", 4*(svbb+3));
	}
	/*printx("#%d ", acnt), prtree(q), printx("\n"), outpr();*/
	s = doit(q, 0, 0, REGMASK);
	if(s.flag & FAIL)
		uerror("codegen failed at top level");
	acnt = ntree = 0;
	if(Pflag && q->in.op != INIT)
		printx("#%d ", svbb), prtree(q), printx("\n");
	outpr();
	tmpoff = svtemp;
	regvar = svregvar;
	if(M32 && !bothdebug)
		for(i = 0; i < NRGS; i++)
			busy[i] = 0;
}

ret
doit(p, flag, dest, regmask)
NODE *p;
ret dest;
{	ret s, t, x, y;
	char *pp;
	NODE snode, *q;
	int i, j, svmask = regmask;
	switch(p->in.op) {
	default:
		pr("#\tweird??? %d\n", p->in.op);
		return(dest);
	case ASG AND:
		fieldbotch(p)
		if(dest.ans && p->in.left->in.op == STAR)
			longjmp(back, mediumstar(p));
		flag |= DESTISLEFT;
		flag &= ~CC;
	case AND:
		if((flag & CC)) {
			if(p->in.left->in.op == CONV && incrsize(p->in.left->in.left) < 4)
				p->in.left = p->in.left->in.left;
			if(p->in.right->in.op == CONV
				&& incrsize(p->in.right->in.left) < 4)
				p->in.right = p->in.right->in.left;
		}
		t = doit(p->in.left, VALUE|USED, 0, regmask);
		if(t.flag & FAIL)
			return(t);
		regmask &= ~t.regmask;
		s = doit(p->in.right, VALUE|USED, 0, regmask);
		if(s.flag & FAIL)
			goto binfail;
		if(flag & CC) {
			if(incrsize(p->in.left) <= incrsize(p->in.right))
				i = childtype(p);
			else
				i = type(p->in.right);
			pr("#\tbit%c\t%s,%s\n", i, str(s), str(t));
			dest.ans = 0;
			dest.flag = CC;
			dest.regmask = 0;
			return(dest);
		}
		regmask &= ~s.regmask;
#if VAX==1
		/* p->in.right->in.op == COMPL is a useful special case */
		if((flag & DESTISLEFT) && dest.ans == 0)
			dest = x = t;
		if(dest.ans == 0)
			if(s.flag & SCRATCH)
				dest = x = s;
			else if(t.flag & SCRATCH) {
				dest = t;
				x = allocreg(p, regmask);
			}
			else
				dest = x = allocreg(p, regmask);
		else
			x = allocreg(p, regmask);
		if(p->in.right->in.op == ICON) {
			x = tostack();	/* to get a buf */
			sprintx(str(x), "$%d", -p->in.right->tn.lval - 1);
		}
		else
			pr("#\tmcom%c\t%s,%s\n", childtype(p), str(s), str(x));
		if(strcmp(str(t), str(dest)) == 0)
			pr("#\tbic%c2\t%s,%s\n", childtype(p), str(x), str(dest));
		else if(flag & DESTISLEFT) {
			pr("#\tbic%c2\t%s,%s\n", childtype(p), str(x), str(t));
			if(strcmp(str(t), str(dest))) {
				x = t;
				goto movexdest;
			}
		}
		else
			pr("#\tbic%c3\t%s,%s,%s\n", childtype(p), str(x),
				str(t), str(dest));
		dest.flag |= CC;
		return(dest);
#endif
	case CALL:
call:
		s.flag = funargs(p->in.right, regmask);
		if(s.flag & FAIL)
			return(s);
		i = p->stn.argsize/32;
called:
		s = doit(p->in.left, VALUE|ASADDR|USED, 0, regmask);
		pp = str(s);
aftercall:
		if(svmask != REGMASK) {
			s.flag = FAIL;
			return(s);
		}
		x = specialreg(p, regmask);
		if(M32 && (p->in.op == STCALL || p->in.op == UNARY STCALL))
			pr("#\tmovaw\t%d(%%fp),%%r2\n", gimmetemp(p->stn.stsize/SZINT));
		pr(VAX? "#\tcalls\t$%d,%s\n": "#\tcall\t&%d,%s\n", i, pp);
		if(flag & ASADDR) {
			strcat(str(x), ")");
			strshift(str(x), 1);
			str(x)[0] = '(';
		}
		if(dest.ans == 0 && (VAX || !(flag & TOSTACK)))
			if(flag & DESTISLEFT)
				dest = doit(p->in.left->in.op == CONV?
					p->in.left->in.left:
					p->in.left, 0/* ? */, 0, regmask & ~x.regmask);
			else
				return(x);
movexdest:	/* type(p), not childtype, for a = a % b */
		if(strcmp(str(x), str(dest)) == 0)
			return(x);
		if((flag & TOSTACK) && incrsize(p) == 4)
			pr("#\tpush%c\t%s\n", type(p), str(x));
		else if(x.flag & ICON0)
			pr("#\tclr%c\t%s\n", type(p), str(dest));
		else if(VAX && isfloat(p) != isfloat(p->in.left)
			&& p->in.op != CALL && p->in.op != UNARY CALL)
			pr("#\tcvt%c%c\t%s,%s\n", childtype(p), type(p), str(x), str(dest));
		else if(VAX && (flag & TOSTACK) && (incrsize(p) != 8))
			pr("#\tcvt%cl\t%s,-(sp)\n", type(p), str(x));
		else {
			pr("#\tmov%c\t%s,%s\n", type(p), str(x), str(dest));
			dest = simpler(x, dest);
		}
		dest.flag |= CC;
		return(dest);
	case CMP:
		if(p->in.left->in.op == CONV && p->in.right->in.op == CONV
			&& childtype(p->in.left) == childtype(p->in.right)) {
			p->in.left = p->in.left->in.left;
			p->in.right = p->in.right->in.left;
		}
		else if(p->in.left->in.op == CONV && p->in.right->in.op == ICON
			&& p->in.right->tn.lval >= 0
			&& ((incrsize(p->in.left->in.left) == 1
				&& p->in.right->tn.lval < 128)
			|| (incrsize(p->in.left->in.left) == 2
				&& p->in.right->tn.lval < 32768)))
			p->in.left = p->in.left->in.left;
		/* the above rewriting depends on childype being of p->left */
		s = doit(p->in.left, VALUE|USED, 0, regmask);
		if(s.flag & FAIL)
			return(s);
		regmask &= ~s.regmask;
		t = doit(p->in.right, VALUE|USED, 0, regmask);
		if(t.flag & FAIL) {
			totemp(p, LEFT);
			longjmp(back, 1);
		}
		pr("#\tcmp%c\t%s,%s\n", childtype(p), str(s), str(t));
		dest.ans = dest.regmask = 0;
		dest.flag = CC;
		return(dest);
	case COMOP:	/* qnodes lurking underneath */
		if(p->in.left->in.op == GENLAB) {
			if(dest.ans == 0) {
				dest = allocreg(p, regmask);
			}
			t = doit(p->in.left, VALUE|(flag & CC), dest, regmask);
		}
		else
			t = doit(p->in.left, 0, 0, regmask);
		if(t.flag & FAIL)
			return(t);
		s = doit(p->in.right, VALUE|(flag & (USED|CC)), dest, regmask);
		if(s.flag & FAIL)
			return(s);
		if(flag & ASADDR) {
			dest.ans = 0;
			i = (s.flag & (ISREG|CANINDIR|SCRATCH));
			j = s.regmask;
			buf = str(s);
			goto convbuf;
		}
		return(s);
	case COMPL:
		s = doit(p->in.left, VALUE|USED, 0, regmask);
		if(s.flag & FAIL)
			return(s);
		if(dest.ans == 0)
			if(s.flag & SCRATCH)
				dest = s;
			else
				dest = allocreg(p, regmask & ~s.regmask);
		pr("#\tmcom%c\t%s,%s\n", childtype(p), str(s), str(dest));
		dest.flag |= CC;
		return(dest);
	case CONV:
		if(p->in.left->in.op == ASSIGN && p->in.left->in.left->in.op == STAR) {
			if(dest.ans == 0) {
				dest = allocreg(p, regmask);
				s = doit(p->in.left, VALUE|USED, dest, regmask);
			}
			else {
				x = allocreg(p, regmask);
				s = doit(p->in.left, VALUE|USED, x, regmask & ~x.regmask);
			}
			if(s.flag & FAIL) {
				asgwrite(p->in.left);
				longjmp(back, 1);
			}
		}
		else
			s = doit(p->in.left, VALUE|USED, 0, regmask);
		if(childtype(p) == type(p) && dest.ans == 0)
			return(s);
		if(s.flag & FAIL)
			return(s);
		if(dest.ans == 0)
			if(s.flag & SCRATCH)
				dest = checksize(p, s, regmask);
			else
				dest = allocreg(p, regmask);
		if(isunsigned(p->in.left) && incrsize(p) > incrsize(p->in.left)) {
			if(type(p) != 'f' && type(p) != 'd')
				pp = "movz";
			else {	/* uns to float or double */
				rewriteconv(p);
				longjmp(back, 1);
			}
		}
		else
			pp = VAX? "cvt": "movb";
		if(isfloat(p) != isfloat(p->in.left))
			goto cvtop;
		if(incrsize(p) < incrsize(p->in.left)) {
			if(isfloat(p))
				goto cvtop;
			pr("#\tmov%c\t%s,%s\n", type(p), str(s), str(dest));
			dest = simpler(s, dest);
		}
		else if(incrsize(p) > incrsize(p->in.left))
cvtop:
			pr("#\t%s%c%c\t%s,%s\n", pp, childtype(p), type(p), str(s), str(dest));
		else {	/* types were the same, but dest.ans != 0 */
			pr("#\tmov%c\t%s,%s\n", type(p), str(s), str(dest));
			dest = simpler(s, dest);
		}
		if(M32 && (flag & TOSTACK))
			pr("#\tpushw\t%s\n", str(dest));
		dest.flag |= CC;
		return(dest);
	case DECR:
		i = -1;
		pp = "sub";
incrop:
		if(p->in.type == TFLOAT)
			uerror("no float ++/--");
		fieldbotch(p)
		/* if the dest uses regs, it may be from a qnode, so that reg
		 * shouldn't be used (i?*a->b++:x()) where b has offset > 0
		 */
		s = doit(p->in.left, USED, 0, dest.ans? regmask & ~dest.regmask: regmask);
		if(s.flag & FAIL)
			return(s);
		t = doit(p->in.right, 0, 0, 0);
		if(flag & VALUE) {
			if(dest.ans == 0)
				dest = allocreg(p, regmask & ~s.regmask);
			pr("#\tmov%c\t%s,%s\n", childtype(p), str(s), str(dest));
			if(t.flag & ICON1) {
				if(i == 1)
					pp = "inc";
				else
					pp = "dec";
				pr("#\t%s%c\t%s\n", pp, childtype(p), str(s));
			}
			else
				pr("#\t%s%c2\t%s,%s\n", pp, childtype(p),
					str(t), str(s));
			if(flag & ASADDR) {
				if(dest.flag & ISREG) {
					sprintx(buf, "0(%s)", str(dest));
					done(dest, CANINDIR, dest.regmask);
				}
				if(dest.flag & CANINDIR) {
					sprintx(buf, "*%s", str(dest));
					done(dest, 0, dest.regmask);
				}
				uerror("weird asaddr in incrop");
			}
			dest.flag &= ~CC;
			return(dest);
		}
		if(t.flag & ICON1) {
			if(i == 1)
				pp = "inc";
			else
				pp = "dec";
			pr("#\t%s%c\t%s\n", pp, childtype(p), str(s));
		}
		else
			pr("#\t%s%c2\t%s,%s\n", pp, childtype(p), str(t), str(s));
		if(dest.ans) {
			x = s;
			goto movexdest;
		}
		s.flag &= ~CC;
		return(s);
	case ASG DIV:
		fieldbotch(p)
		if(rewriteasgop(p))
			goto assign;
		flag |= DESTISLEFT;
	case DIV:
		/* trees wrong: uns/=int and int/=uns */
		if(!isunsigned(p->in.right) && !isunsigned(p->in.left)) {
			pp = "div";
			goto binop;
		}
		pp = "udiv";
#if M32==1
		goto binop;
#else if VAX==1
unsdiv:
		if(incrsize(p->in.right) != 4) {
			snode.in.op = CONV;
			snode.in.left = p->in.right;
			snode.in.type = TULONG;
			s = doit(&snode, VALUE|TOSTACK, tostack(), regmask);
		}
		else
			s = doit(p->in.right, TOSTACK, tostack(), regmask);
		if(s.flag & FAIL)
			return(s);
		regmask &= ~s.regmask;
		if(incrsize(p->in.left) != 4) {		
			snode.in.op = CONV;
			snode.in.left = p->in.left;
			snode.in.type = TULONG;
			t = doit(&snode, VALUE|TOSTACK, tostack(), regmask);
		}
		else
			t = doit(p->in.left, TOSTACK, tostack(), regmask);
		if(t.flag & FAIL) {
			if(dest.ans || svmask != REGMASK) {
				t = doit(p->in.left, VALUE|TOSTACK, tostack(), svmask);
				if(t.flag & FAIL)
					return(t);
			}
			totemp(p, RIGHT);
			longjmp(back, 1);
		}
		i = 2;
		goto aftercall;
#endif
	case ASG ER:
		fieldbotch(p)
		flag |= DESTISLEFT;
	case ER:
		pp = "xor";
binop:
		if((flag & DESTISLEFT) && dest.ans && p->in.left->in.op == STAR)
			longjmp(back, mediumstar(p));
		t = doit(p->in.left, VALUE|USED, 0, regmask);
		if(t.flag & FAIL) {
			return(t);
		}
		regmask &= ~t.regmask;
		s = doit(p->in.right, VALUE|USED, 0, regmask);
		if(s.flag & FAIL) {
binfail:
			if(dest.ans || svmask != REGMASK) {
				s = doit(p->in.right, VALUE|USED, 0, svmask);
				if(s.flag & FAIL)
					return(s);
			}
			/* *foo op= expr  requires care */
			if((flag & DESTISLEFT) && p->in.left->in.op == STAR)
				totemp(p->in.left, LEFT);
			else
				totemp(p, LEFT);
			longjmp(back, 1);
		}
		if(type(p) != childtype(p)) {
			x = allocreg(p, regmask);
			pr("#\t%s%c3\t%s,%s,%s\n", pp, childtype(p), str(s),
				str(t), str(x));
			if(dest.ans == 0)
				dest = x;
			if(!isfloat(p) && !isfloat(p->in.left)
				&& incrsize(p) < incrsize(p->in.left)) {
				pr("#\tmov%c\t%s,%s\n", type(p), str(x), str(dest));
				dest = simpler(x, dest);
			}
			else
				pr("#\tcvt%c%c\t%s,%s\n", childtype(p), type(p),
					str(x), str(dest));
			dest.flag |= CC;
			return(dest);
		}	
		if(dest.ans == 0)
			if((t.flag & SCRATCH) || (flag & DESTISLEFT)) {
twoop:
				dest = t;
				if(*pp == 'a' && (s.flag & ICON1))
					pr("#\tinc%c\t%s\n", childtype(p), str(t));
				else if(*pp == 's' && (s.flag & ICON1))
					pr("#\tdec%c\t%s\n", childtype(p), str(t));
				else
					pr("#\t%s%c2\t%s,%s\n", pp, childtype(p),
						str(s), str(t));
				if(flag & ASADDR)
					goto binopaddr;
				if(flag & TOSTACK)
					pr("#\tpush%c\t%s\n", type(p), str(t));
				dest.flag |= CC;
				return(dest);
			}
			else if(s.flag & SCRATCH)
				dest = s;
			else
				dest = allocreg(p, regmask);
		if(dest.ans && (flag & DESTISLEFT)) {
			pr("#\t%s%c2\t%s,%s\n", pp, childtype(p), str(s), str(t));
			x = t;
			goto movexdest;
		}
		if((dope[p->in.op] & (COMMFLG|MULFLG)) && strcmp(str(s), str(dest)) == 0) {
			dest = s;
			s = t;
			t = dest;
			goto twoop;
		}
		if(strcmp(str(t), str(dest)) == 0)
			goto twoop;
		pr("#\t%s%c3\t%s,%s,%s\n", pp, childtype(p), str(s), str(t), str(dest));
		if(M32 && (flag & TOSTACK)) {
			pr("#\tpushw\t%s\n", str(dest));
			return(dest);
		}
binopaddr:
		if(flag & ASADDR) {
			if(dest.flag & ISREG) {
				strcat(str(dest), ")");
				strshift(str(dest), 1);
				str(dest)[0] = '(';
				return(dest);
			}
			if(dest.flag & CANINDIR) {
				strshift(str(dest), 1);
				str(dest)[0] = '*';
				dest.flag &= ~CANINDIR;
				return(dest);
			}
			dest.flag = FAIL;	/* doubtful */
			return(dest);
		}
		dest.flag |= CC;
		return(dest);	
	case FLD:
		/* this is rvalues */
		s = doit(p->in.left, USED, 0, regmask);
		if(s.flag & FAIL)
			return(s);
		if(dest.ans == 0)
			dest = allocreg(p, regmask);
		if((s.flag & INDEX) && p->in.left->in.op == STAR
			&& incrsize(p->in.left) != 1) {
		/* over-enthusiastic use of index mode */
			if(dest.flag & SCRATCH)
				x = dest;
			else
				x = allocreg(p, regmask);
			pr("#\tmov%c\t%s,%s\n", childtype(p), str(s), str(x));
			s = x;
		}
#if VAX==1
		pr("#\text%sv\t$%d,$%d,%s,%s\n", isunsigned(p->in.left)? "z": "",
			p->tn.rval/64, p->tn.rval%64, str(s), str(dest));
#else if M32==1
		i = p->tn.rval % 64;
		j = 32 - p->tn.rval/64 - i;
		pr("#\textzv\t&%d,&%d,%s,%s\n", j, i, str(s), str(dest));
#endif
		dest.flag |= CC;
		return(dest);
	case GENBR:
		if(p->in.left->in.op == CONV)
			p->in.left = p->in.left->in.left;
		s = doit(p->in.left, CC|VALUE|USED, 0, regmask);
		pp = genjmp(p->bn.lop);
		if(s.flag & CC)
			pr("#\t%s\t", pp);
		else if(VAX)
			pr("#\ttst%c\t%s\n#\t%s\t", childtype(p), str(s), pp);
		else if(M32)
			pr("#\tcmp%c\t%s,&0\n#\t%s\t", childtype(p), str(s), pp);
		pr(VAX? "L%d\n": ".L%d\n", p->bn.label);
		s.flag |= CC;	/* ?? */
		if(Pflag) {
			++bbcnt;
			pr("#\tincl\tlocprof+%d\n", 4*(bbcnt+3));
		}
		return(s);
	case GENLAB:
		s = doit(p->in.left, flag, dest, regmask);
		pr(VAX? "#L%d:\n": "#.L%d:\n", p->bn.label);
		if(M32 && (flag & TOSTACK))
			pr("#\tpushw\t%s\n", str(s));
		if(Pflag) {
			++bbcnt;
			pr("#\tincl\tlocprof+%d\n", 4*(bbcnt+3));
		}
		return(s);
	case GENUBR:
		s = doit(p->in.left, flag & CC, dest, regmask);
		if((flag & CC) && !(s.flag & CC))
			pr("#\ttst%c\t%s\n", childtype(p), str(s));
		pr(VAX? "#\tjbr\tL%d\n": "#\tjmp\t.L%d\n", p->bn.label);
		return(s);
	case ICON:
		if(p->tn.name)
			if(p->tn.lval)
				sprintx(buf, "%s+%d", p->tn.name, p->tn.lval);
			else
				sprintx(buf, "%s", p->tn.name);
		else
			sprintx(buf, "%d", p->tn.lval);
		if(!(flag & ASADDR)) {
			strshift(buf, 1);;
			buf[0] = VAX? '$': '&';
		}
		i = 0;
		if(VAX && p->tn.name == 0)
			if(p->tn.lval == 0)
				i = ICON0;
			else if(p->tn.lval == 1)
				i = ICON1;
		j = 0;
convbuf:	/* the cookie is in buf */
		if(dest.ans == 0 && !(flag & TOSTACK)) {
			if(p->in.op == ICON || !(flag & ASADDR)) {
				done(s, i, j);
			}
			if(i & CANINDIR) {
				strshift(buf, 1);
				buf[0] = '*';
				done(s, i & ~CANINDIR, j);
			}
			if(i & ISREG) {
				strcat(buf, ")");
				strshift(buf, 1);
				buf[0] = '(';
				done(s, i, j);
			}
			pp = buf;
			buf += BUF;
			s = allocreg(p, regmask);
			pr("#\tmov%c\t%s,%s\n", type(p), pp, str(s));
			flag &= ~ASADDR;
			goto inreg;
		}
		if(flag & TOSTACK)
			if(incrsize(p) == 4)
				pr(VAX? "#\tpushl\t%s\n": "#\tpushw\t%s\n", buf);
			else if(incrsize(p) == 8)
				pr("#\tmovd\t%s,%s\n", buf, str(dest));
			else
				pr("#\tcvt%cl\t%s,%s\n", type(p), buf, str(dest));
		else if(i & ICON0)
			pr("#\tclr%c\t%s\n", type(p), str(dest));
		else {	
			pr("#\tmov%c\t%s,%s\n", type(p), buf, str(dest));
			if(!(dest.flag & INDEX) || !index(str(dest), '+')
				&& !index(str(dest), '-'))
				;
			else if(!(i & INDEX) || !index(buf, '+')
				&& !index(buf, '-')) {
				done(s, (i|CC), j);
			}
			else
				dest.flag |= USED;
		}
		dest.flag |= CC;
		return(dest);
	case INCR:
		i = 1;
		pp = "add";
		goto incrop;
	case INIT:	/* knows it is ICON */
		s = doit(p->in.left, ASADDR, 0, 0);
		pr(VAX? "#\t.long\t%s\n": "#\t.word\t%s\n", str(s));
		return(dest);
	case ASG LS:
		if(VAX && incrsize(p) != 4)
			lsconv(p);
		if(rewriteasgop(p))
			goto assign;
		fieldbotch(p)
		flag |= DESTISLEFT;
		if(dest.ans && p->in.left->in.op == STAR)
			longjmp(back, mediumstar(p));
	case LS:	/* stupid vax */
#if M32==1
		pp = "LLS";
shiftop:
#endif
		if(VAX && incrsize(p) != 4)
			lsconv(p);
		s = doit(p->in.left, VALUE|USED, 0, regmask);
		if(s.flag & FAIL)
			return(s);
		regmask &= ~s.regmask;
		t = doit(p->in.right, VALUE|USED, 0, regmask);
		if(t.flag & FAIL)
			goto binfail;
		if(dest.ans == 0)
			if((s.flag & SCRATCH) || (flag & DESTISLEFT))
				dest = s;
			else
				dest = allocreg(p, regmask);
#if VAX==1
		if(incrsize(p) != 4) {	/* stupid vax */
			x = allocreg(p, regmask);
			pr("#\tashl\t%s,%s,%s\n", str(t), str(s), str(x));
			pr("#\tmov%c\t%s,%s\n", type(p), str(x), str(dest));
			if(flag & DESTISLEFT)
				pr("#\tmovl\t%s,%s\n", str(x), str(s));
			dest.flag |= CC;
			return(dest);
		}
		if(p->in.right->in.op == ICON && (i = p->in.right->tn.lval) <= 4
			&& (dest.flag & ISREG)) {
			if(strcmp(str(s), str(dest))) {
				i--;
				pr("#\taddl3\t%s,%s,%s\n", str(s), str(s), str(dest));
			}
			while(i-- > 0)
				pr("#\taddl2\t%s,%s\n", str(dest), str(dest));
		}
		else if(flag & DESTISLEFT)
			pr("#\tashl\t%s,%s,%s\n", str(t), str(s), str(s));
		else
			pr("#\tashl\t%s,%s,%s\n", str(t), str(s), str(dest));
		if((flag & DESTISLEFT) && strcmp(str(dest), str(s)))
			pr("#\tmovl\t%s,%s\n", str(s), str(dest));
#else if M32==1
		pr("#\t%s%c3\t%s,%s,%s\n", pp, childtype(p) + UP, str(t), str(s),
			str(dest));
		if(flag & TOSTACK)
			pr("#\tpush%c\t%s\n", type(p), str(dest));
#endif
		dest.flag |= CC;
		return(dest);
	case ASG MINUS:
		fieldbotch(p)
		if(rewriteasgop(p))
			goto assign;
		flag |= DESTISLEFT;
	case MINUS:
		pp = "sub";
		/* you won't believe this: type x[], *y where type is shorter than
		 * int, generates subx $_x,y which the assembler barfs on.
		 * instead, generate crummy code (stupid assembler)
		 */
		if(incrsize(p) < 4) {
			if(p->in.left->in.op == ICON && p->in.left->tn.name) {
				p->in.left->in.type = TINT;
				totemp(p, LEFT);
			}
			if(p->in.right->in.op == ICON && p->in.right->tn.name) {
				p->in.right->in.type = TINT;
				totemp(p, RIGHT);
			}
		}
		goto binop;
	case ASG MOD:
		fieldbotch(p)
		if(rewriteasgop(p))
			goto assign;
		if(dest.ans && p->in.left->in.op == STAR)
			longjmp(back, mediumstar(p));
		flag |= DESTISLEFT;
	case MOD:
#if M32==1
		if(isunsigned(p->in.right) || isunsigned(p->in.left))
			pp = "umod";
		else
			pp = "mod";
		goto binop;
#else if VAX==1
		if(isunsigned(p->in.right) || isunsigned(p->in.left)) {
			pp = "urem";
			goto unsdiv;
		}
		s = doit(p->in.left, VALUE|USED, 0, regmask);
		if(s.flag & FAIL)
			return(s);
		regmask &= ~s.regmask;
		t = doit(p->in.right, VALUE|USED, 0, regmask);
		if(t.flag & FAIL)
			goto binfail;
		regmask &= ~t.regmask;
		x = allocreg(p, regmask);
		pr("#\tdiv%c3\t%s,%s,%s\n", childtype(p), str(t), str(s), str(x));
		pr("#\tmul%c2\t%s,%s\n", childtype(p), str(t), str(x));
		pr("#\tsub%c3\t%s,%s,%s\n", childtype(p), str(x), str(s), str(x));
		if(dest.ans)
			goto movexdest;
		if(flag & DESTISLEFT) {
			dest = s;
			goto movexdest;
		}
		x.flag |= CC;
		return(x);
#endif
	case ASG MUL:
		fieldbotch(p)
		if(rewriteasgop(p))
			goto assign;
		flag |= DESTISLEFT;
	case MUL:
		pp = "mul";
		goto binop;
	case NAME:	/* ASADDR? */
		if(p->tn.lval && p->tn.name)
			sprintx(buf, "%s+%d", p->tn.name, p->tn.lval);
		else if(p->tn.name)
			sprintx(buf, "%s", p->tn.name);
		else if(p->tn.lval)
			sprintf(buf, "%d", p->tn.lval);
		else
			sprintx(buf, "0");
		j = 0;
		i = CANINDIR;
		goto convbuf;
	case ASG OR:
		fieldbotch(p)
		flag |= DESTISLEFT;
	case OR:
		pp = VAX? "bis": "or";
		goto binop;
	case ASG PLUS:
		fieldbotch(p)
		if(rewriteasgop(p))
			goto assign;
		flag |= DESTISLEFT;
	case PLUS:
		pp = "add";
		goto binop;
	case REG:
		sprintx(buf, "%s", regnames[p->tn.rval]);
		j = 0;
		i = ISREG;
		if(p->tn.lval == 1)
			i |= SCRATCH;
		goto convbuf;
	case RNODE: case SNODE:
		x = specialreg(p, 3);
		x.regmask = 0;
		if(dest.ans)
			goto movexdest;
		return(x);
	case QNODE:
		return(specialreg(p, regmask));
	case ASG RS:
		fieldbotch(p)
		if(VAX && incrsize(p) != 4)
			lsconv(p);
		if(rewriteasgop(p))
			goto assign;
		flag |= DESTISLEFT;
	case RS:	/* all right shifts are unsigned */
		if(incrsize(p) != 4)
			lsconv(p);
#if M32==1
		pp = "LRS";
		goto shiftop;
#else if VAX==1
		s = doit(p->in.left, VALUE|USED, 0, regmask);
		if(s.flag & FAIL)
			return(s);
		if(p->in.left->in.op == ICON ||
			(s.flag & INDEX) && p->in.left->in.op == STAR
			&& incrsize(p->in.left) != 1 ||
			(s.flag & AUTO) && incrsize(p->in.left) != 1) {
		/* over-enthusiastic use of index mode */
		/* 12 >> i generates a byte immediate */
			if((s.flag & INDEX) && (flag & DESTISLEFT)) {/* *a++ >>= 1 */
				lsconv(p);	/* not best */
				longjmp(back, 1);
			}
			if((flag & DESTISLEFT) && dest.ans == 0) {
				dest = s;
				regmask &= ~s.regmask;
			}
			if(dest.flag & SCRATCH)
				x = dest;
			else
				x = allocreg(p, regmask);
			pr("#\tmovl\t%s,%s\n", str(s), str(x));
			s = x;
		}
		regmask &= ~s.regmask;
		if(dest.ans == 0)
			if((flag & DESTISLEFT) || (s.flag & SCRATCH))
				dest = s;
			else
				dest = allocreg(p, regmask);
		dest.flag |= CC;
		regmask &= ~dest.regmask;
		if(incrsize(p) != 4)
			x = allocreg(p, regmask);
		else
			x = dest;
		if(p->in.right->in.op == ICON && (i = p->in.right->tn.lval) >= 0) {
			pr("#\textzv\t$%d,$%d,%s,%s\n", i, 32 - i, str(s), str(x));
			goto movexdest;
		}
		t = doit(p->in.right, VALUE|USED, 0, regmask);
		if(t.flag & FAIL)
			goto binfail;
		y = allocreg(p, regmask & ~t.regmask);
		pr("#\tsubl3\t%s,$32,%s\n", str(t), str(y));
		pr("#\textzv\t%s,%s,%s,%s\n", str(t), str(y), str(s), str(x));
		goto movexdest;
#endif
	case STAR:
		switch(p->in.left->in.op) {
		case ICON:	/* as in foo((a, a)), where a is a struct */
			s = doit(p->in.left, VALUE, 0, regmask);
			sprintx(buf, "%s", str(s)+1);
			i = j = 0;
			goto convbuf;
		case VAUTO:
		case VPARAM:
		case NAME:
			s = doit(p->in.left, VALUE, 0, regmask);
			sprintx(buf, "*%s", str(s));
			j = i = 0;
			goto convbuf;
		case REG:
			s = doit(p->in.left, VALUE, 0, regmask);
inreg:
			if(s.flag & ISREG) {
				sprintx(buf, VAX? "(%s)": "0(%s)", str(s));
				j = s.regmask;
				i = CANINDIR;
				goto convbuf;
			}
			else if(s.flag & CANINDIR) {
				sprintx(buf, "*%s", str(s));
				j = s.regmask;
				i = 0;
				goto convbuf;
			}
			else {
				s.flag = FAIL;
				return(s);
			}
		}
		q = p->in.left;
		if(VAX && q->in.op == INCR && !(flag & INDEX)
			&& q->in.right->in.op == ICON && q->in.left->in.op == REG
			&& incrsize(p) == (int) q->in.right->tn.lval) {
			s = doit(q->in.left, VALUE, 0, regmask);
			sprintx(buf, "(%s)+", str(s));
			i = INDEX|AUTO;
			j = s.regmask;
			goto convbuf;
		}
		if(VAX && q->in.op == PLUS && q->in.left->in.op == LS
			&& !(flag & INDEX)
			&& q->in.left->in.right->in.op == ICON
			&& shiftsize(p) == (int) q->in.left->in.right->tn.lval) {
			s = doit(q->in.left->in.left, VALUE|USED, 0, regmask);
			if(s.flag & FAIL)
				return(s);
			regmask &= ~s.regmask;
			if(!(s.flag & ISREG)) {
				x = allocreg(p, regmask);
				pr("#\tmov%c\t%s,%s\n", childtype(q), str(s), str(x));
				regmask |= s.regmask;
				regmask &= ~x.regmask;
				s = x;
				if(!(s.flag & ISREG)) {
					s.flag = FAIL;
					return(s);
				}
			}
			t = doit(q->in.right, ASADDR|USED, 0, regmask);
			sprintx(buf, "%s[%s]", str(t), str(s));
			i = INDEX;
			j = s.regmask | t.regmask;
			goto convbuf;
		}
		if(q->in.op == PLUS && q->in.right->in.op == ICON) {
			s = doit(q->in.left, VALUE|USED, 0, regmask);
			if(s.flag & FAIL)
				return(s);
			if(!(s.flag & ISREG)) {
				x = allocreg(p, regmask);
				pr("#\tmov%c\t%s,%s\n", childtype(q), str(s), str(x));
				s = x;
				if(!(s.flag & ISREG)) {
					s.flag = FAIL;
					return(s);
				}
			}
			regmask &= ~s.regmask;
			t = doit(q->in.right, ASADDR|USED, 0, regmask);
			sprintx(buf, "%s(%s)", str(t), str(s));
			i = CANINDIR;
			j = s.regmask;
			goto convbuf;
		}
		if(q->in.op == MINUS && q->in.right->in.op == ICON) {
			s = doit(q->in.left, VALUE|USED, 0, regmask);
			if(s.flag & FAIL)
				return(s);
			if(!(s.flag & ISREG)) {
				x = allocreg(p, regmask);
				pr("#\tmov%c\t%s,%s\n", childtype(q), str(s), str(x));
				s = x;
				if(!(s.flag & ISREG)) {
					s.flag = FAIL;
					return(s);
				}
			}
			regmask &= ~s.regmask;
			sprintx(buf, "%d(%s)", -q->in.right->tn.lval, str(s));
			i = CANINDIR;
			j = s.regmask;
			goto convbuf;
		}
		if(VAX && q->in.op == ASG MINUS && !(flag & (INDEX|USED))
			&& q->in.right->in.op == ICON && q->in.left->in.op == REG
			&& incrsize(p) == (int) q->in.right->tn.lval) {
			s = doit(q->in.left, VALUE, 0, regmask);
			sprintx(buf, "-(%s)", str(s));
			i = INDEX|AUTO;
			j = s.regmask;
			goto convbuf;
		}
		if(VAX && q->in.op == PLUS && q->in.left->in.op == REG
			&& !(flag & INDEX) && q->in.right->in.op == REG
			&& incrsize(p) == 1) {
			sprintx(buf, "(r%d)[r%d]", q->in.right->tn.rval,
				q->in.left->tn.rval);
			i = INDEX;
			j = 0;
			goto convbuf;
		}
		s = doit(p->in.left, VALUE|USED, 0, regmask);
		if(s.flag & FAIL)
			return(s);
		regmask &= ~s.regmask;
		if(s.flag & CANINDIR) {
			sprintx(buf, "*%s", str(s));
			i = 0;
			j = s.regmask;
			goto convbuf;
		}
		if(!(s.flag & ISREG)) {
			x = allocreg(p, regmask);
			pr("#\tmov%c\t%s,%s\n", childtype(q), str(s), str(x));
			regmask |= s.regmask;
			regmask &= ~x.regmask;
			s = x;
		}
		if(!(s.flag & ISREG)) {
			s.flag = FAIL;
			return(s);
		}
		sprintx(buf, VAX? "(%s)": "0(%s)", str(s));
		i = 0;
		j = s.regmask;
		goto convbuf;
	case STASG:
		if(regmask != REGMASK) {
			s.flag = FAIL;
			return(s);
		}
		if(p->stn.stsize/8 == 4) {
			i = VAX? 'l': 'w';
stasg:
			s = doit(p->in.left, VALUE|ASADDR|USED, 0, regmask);
			if(s.flag & FAIL)
				return(s);
			regmask &= ~s.regmask;
			t = doit(p->in.right, VALUE|ASADDR|USED, 0, regmask);
			if(t.flag & FAIL) {
				totemp(p, LEFT);
				longjmp(back, 1);
			}
			pr("#\tmov%c\t%s,%s\n", i, str(t), str(s));
			s.regmask = s.flag = 0;
			return(simpler(t, s));
		}
		else if(VAX && p->stn.stsize/8 == 8) {
			i = 'q';
			goto stasg;
		}
#if VAX==1
		s = doit(p->in.right, VALUE|USED, 0, regmask);
		if(s.flag & INDEX) {
		/* over-enthusiastic use of index mode */
			x = allocreg(p, regmask);
			pr("#\tmovl\t%s,%s\n", str(s), str(x));
			s = x;
		}
		regmask &= ~s.regmask;
		t = doit(p->in.left, VALUE|USED, 0, regmask);
		if(t.flag & FAIL) {
			totemp(p, RIGHT);
			longjmp(back, 1);
		}
		if(t.flag & INDEX) {
		/* over-enthusiastic use of index mode */
			x = allocreg(p, regmask);
			pr("#\tmovl\t%s,%s\n", str(t), str(x));
			t = x;
		}
		t = indirit(t);
		if(t.flag & FAIL) {
			if(p->in.left->in.op != STAR)
				uerror("codegen failure in struct asg");
			totemp(p->in.left, LEFT);
			longjmp(back, 1);
		}
		if(p->in.right->in.op == STASG) {	/* secret knowledge */
			pr("#\tsubl2\t$%d,r3\n", p->stn.stsize/8);
			pr("#\tmovc3\t$%d,(r3),%s\n", p->stn.stsize/8, str(t));
		}
		else {
			s = indirit(s);
			if(s.flag & FAIL) {
				if(p->in.right->in.op != STAR)
					uerror("codgen fail in stasg");
				totemp(p->in.right, LEFT);
				longjmp(back, 1);
			}
			pr("#\tmovc3\t$%d,%s,%s\n", p->stn.stsize/8, str(s), str(t));
		}
		t.regmask = t.flag = 0;
#else if M32==1
		x = allocreg(p, regmask);	/* gets reg 0 */
		s = doit(p->in.right, VALUE, x, regmask);
		regmask &= ~x.regmask;
		y = allocreg(p, regmask);	/* gets reg 1 */
		i = p->stn.stsize/32;
		if(p->in.right->in.op == STASG && i >= 7) {	/* secret knowledge */
			pr("#\tsubw3\t&%d,%%r1,%%r0\n", p->stn.stsize/8);
		}
		t = doit(p->in.left, VALUE, y, regmask);
		if(t.flag & FAIL)
			stasgrewrite(p);
		if(i >= 7) {
			pr("#\tmovw\t&%d,%%r2\n", i);
			pr("#\tMOVBLW\n");
		}
		else
			while(--i >= 0)
				pr("#\tmovw\t%d(%r0),%d(%r1)\n", 4 * i, 4 * i);
#endif
		return(t);
	case STCALL:
		goto call;
	case VAUTO:
		sprintx(buf, "%d(%s)", p->tn.lval, frameptr);
		j = 0;
		i = CANINDIR;
		goto convbuf;
	case VPARAM:
		sprintx(buf, "%d(%s)", p->tn.lval, argptr);
		j = 0;
		i = CANINDIR;
		goto convbuf;
	case UNARY AND:
		s = doit(p->in.left, USED, 0, regmask);
		if(s.flag & FAIL)
			return(s);
		regmask &= ~s.regmask;
		if(dest.ans == 0) {
			dest = allocreg(p, regmask);
		}
		if(s.flag & ISREG) {
			x = alloctmp(p);
			pr("#\tmov%c\t%s,%s\n", childtype(p), str(s), str(x));
			s = x;
		}
		if(flag & TOSTACK)
			pr(VAX? "#\tpushal\t%s\n": "#\tpushaw\t%s\n", str(s));
		else
			pr("#\tmova%c\t%s,%s\n", type(p), str(s), str(dest));
		if(flag & ASADDR) {
			if(dest.flag & ISREG) {
				strcat(str(dest), ")");
				strshift(str(dest), 1);
				str(dest)[0] = '(';
			}
			else {
				strshift(str(dest), 1);
				str(dest)[0] = '*';
			}
		}
		return(dest);
	case UNARY CALL:
		i = 0;
		goto called;
	case UNARY MINUS:
		s = doit(p->in.left, VALUE|USED, 0, regmask);
		if(s.flag & FAIL)
			return(s);
		if(dest.ans == 0)
			if(s.flag & SCRATCH)
				dest = s;
			else
				dest = allocreg(p, regmask & ~s.regmask);
		pr("#\tmneg%c\t%s,%s\n", childtype(p), str(s), str(dest));
		dest.flag |= CC;
		return(dest);
	case UNARY STCALL:
		i = 0;
		goto called;
	case ASSIGN:
assign:
		if(p->in.left->in.op == QNODE && dest.ans) {
			s = doit(p->in.right, VALUE|(flag&USED? USED: 0),
				dest, regmask);
			return(s);
		}
		if(p->in.left->in.op == RNODE || p->in.left->in.op == SNODE) {
			s = doit(p->in.right, VALUE|(flag&USED? USED: 0),
				dest, regmask);
			if(s.flag & FAIL)
				return(s);
			x = s;
			dest = doit(p->in.left, USED, 0, 0);
			goto movexdest;
		}
		if(p->in.left->in.op == FLD) {
			s = doit(p->in.left->in.left, INDEX, 0, regmask);
			regmask &= ~s.regmask;
			if(p->in.right->in.op == ASSIGN) {
				x = allocreg(p->in.right, regmask);
				t = doit(p->in.right, VALUE|USED, x, regmask & ~x.regmask);
			}
			else
				t = doit(p->in.right, VALUE|USED, 0, regmask);
			if(t.flag & FAIL) {
				totemp(p->in.left, LEFT);
				longjmp(back, 1);
			}
			i = VAX? '$': '&';
			pr("#\tinsv\t%s,%c%d,%c%d,%s\n", str(t), i,
				p->in.left->tn.rval/64, i,
				p->in.left->tn.rval % 64, str(s));
			if(dest.ans)
				pr("#\tmovl\t%s,%s\n", str(t), str(dest));
			t.flag |= CC;
			return(t);
		}
		if(dest.ans && p->in.left->in.op == STAR) {
			if(dest.flag & SCRATCH)
				x = dest;
			else
				x = allocreg(p, regmask);
			s = doit(p->in.right, VALUE|USED, x, regmask);
			if(s.flag & FAIL)
				return(s);
			regmask &= ~s.regmask;
			t = doit(p->in.left, USED, 0, regmask);
			if(t.flag & FAIL) {
			/* did we fail because of a dest, or was it us? */
				if(dest.ans || svmask != REGMASK) {
					t = doit(p->in.left, USED, 0, svmask);
					if(t.flag & FAIL) {
						return(t);
					}
					starasg(p);
				}
				else
					asgwrite(p);
				longjmp(back, 1);
			}
			pr("#\tmov%c\t%s,%s\n", type(p), str(x), str(t));
			if(strcmp(str(x), str(dest)) == 0) {
				dest.flag |= CC;
				return(dest);
			}
			goto movexdest;
		}
		t = doit(p->in.left, USED, 0, regmask);
		regmask &= ~t.regmask;
		if(p->in.right->in.op == GENLAB)
			extracheck(p);	/* slightly worse code in usual case */
		s = doit(p->in.right, VALUE, t, regmask);
		if(s.flag & NOGOOD) {
		/* did we fail because of a dest, or was it us? */
			if(s.flag & FAIL) {
				if(dest.ans || svmask != REGMASK) {
					s = doit(p->in.right, VALUE, t, svmask);
					if(s.flag & FAIL) {
						return(s);
					}
				}
				asgwrite(p);
				longjmp(back, 1);
			}
			else if((flag & USED) && !((flag & CC) && (s.flag & CC))) {
				/* presumably USED as in *p++=*q++ */
				totemp(p, RIGHT);
				longjmp(back, 1);
			}
		}
		if(dest.ans) {
			x = s;
			goto movexdest;
		}
		else if(M32 && (flag & TOSTACK)) {
			pr("#\tpushw\t%s\n", str(s));
			return(s);
		}
		else if(flag & ASADDR) {
			i = (s.flag & (ISREG|CANINDIR|SCRATCH));
			j = s.regmask;
			buf = str(s);
			goto convbuf;
		}
		return(s);
	}
}
