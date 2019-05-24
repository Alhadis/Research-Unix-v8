#include <stdio.h>
#include "mas.h"

insout(sp, a1, a2, size)
struct nlist *sp;
struct arg *a1, *a2;
{
	register struct instab *ip;
	register int c;

	align(HW);
again:
	for (ip = &instab[sp->n_value], c=sp->n_type; c!=0; ip++, c--) {
		if ((size & ip->size) != size)
			continue;
		if (a1 && !argcompat(a1, ip->addr1))
			continue;
		if (a2 && !argcompat(a2, ip->addr2))
			continue;
		if (size==B) {
			if (a1 && a1->atype==AREG && a1->areg1>=AREGS
			 && a1->areg1<=AREGS+7)
				continue;
			if (a2 && a2->atype==AREG && a2->areg1>=AREGS
			 && a2->areg1<=AREGS+7)
				continue;
		}
		putins(ip, a1, a2, size);
		return;
	}
	/* some special dispensations */
	if (strcmp(sp->n_name, "clr")==0) {
		strcpy(yytext, "mov");
		usrname = 0;
		sp = *lookup(0);
		a2 = a1;
		a1 = ap++;
		a1->atype = AIMM;
		a1->xp = xp++;
		a1->xp->xtype = N_ABS;
		a1->xp->xvalue = 0;
		a1->xp->xname = NULL;
		goto again;
	}
	ip = &instab[sp->n_value];
	if (ip->iflag&ISH && a1->atype==AIMM
	 && a1->xp->xtype==N_ABS && a1->xp->xvalue>8) {
		a1->xp->xvalue -= 8;
		insout(sp, a1, a2, size);
		a1->xp->xvalue = 8;
		goto again;
	}
	yyerror("Cannot find op with given args");
}

argcompat(act, exp)
register struct arg *act;
register int exp;
{
	register struct exp *xp;
	register at;
	register ispcrel;

	at = act->atype;
	ispcrel = at==APIC || at==API2 || at==AOFF || at==ANDX;
	switch (exp&AMASK) {

	case AREG:
		if (at != AREG)
			return(0);
		if (exp&D) {
			if (act->areg1>DREGS+7)
				return(0);
			else
				return(1);
		}
		if (exp&A) {
			if (act->areg1<AREGS || act->areg1>AREGS+7)
				return(0);
			else
				return(1);
		}
		if (exp&C) {
			if (act->areg1==CCREG)
				return(1);
			else
				return(0);
		}
		if (exp&U) {
			if (act->areg1==SPREG)
				return(1);
			else
				return(0);
		}
		if (exp&SR) {
			if (act->areg1==SRREG)
				return(1);
			else
				return(0);
		}
		if (exp&SR && act->areg1==SRREG)
			return(1);
		if (act->areg1 > AREGS+7)
			return(0);
		return(1);

	case AIMM:
		if (at != AIMM)
			return(0);
		xp = act->xp;
		if (exp & (O|Q|M)) {
			if (xp->xtype!=N_ABS)
				return(0);
		}
		if (exp&O && (xp->xvalue!=1 || xp->xtype&XFORW))
			return(0);
		if (exp&Q && (xp->xvalue<1 || xp->xvalue>8 || xp->xtype&XFORW))
			return(0);
		if (exp&M && (xp->xvalue<-128 || xp->xvalue>127 || xp->xtype&XFORW))
			return(0);
		if (passno==1)
			return(1);
		if (exp&(V|H) && (xp->xtype&XTYPE)!=N_ABS)
			return(0);
		if (exp&V && (xp->xvalue<0 || xp->xvalue>15))
			return(0);
		return(1);

	case AGEN:
		if (exp&AM) {
			if (act->areg1==PCREG && ispcrel)
				return(0);
			if (at==AREG || at==AIMM)
				return(0);
		}
		if (exp&CT) {
			if (at==AINC || at==ADEC)
				return(0);
			if (at==AREG || at==AIMM)
				return(0);
		}
		if (exp&AL) {
			if (act->areg1==PCREG && ispcrel)
				return(0);
			if (at==AIMM)
				return(0);
		}
		if (exp&DA)
			if (at==AREG && act->areg1>DREGS+7)
				return(0);
		if (at==AREG) {
			if (act->areg1 > AREGS+7) {
				if (act->areg1==SRREG && exp&SP)
					return(1);
				return(0);
			}
		}
		return(1);
	}
	if (exp != at)
		return(0);
	return(1);
}

putins(ip, a1, a2, size)
register struct instab *ip;
register struct arg *a1, *a2;
{
	int insword = 0;
	int relword = 0;
	int imoff = 0, a1off = 0, a2off = 0;
	register struct arg *sava2;
	static szcode[] = { 0, 0, 0100, 0, 0200 };

	dotp->xvalue += 2;
	if (size==0 && ip->size && ip->sdisp!=SIG)
		yyerror("Size field missing");
	if (size && ip->size==0)
		yyerror("Superfluous size field");
	sava2 = a2;
	if (ip->a2disp==DIM || ip->a2disp==DIMH) {
		dotp->xvalue += imoff =
		  genadr1(a2, ip->a2disp, size, &insword, &relword, ip->addr1);
		a2 = 0;
	}
	if (a1)
		dotp->xvalue += a1off =
		  genadr1(a1, ip->a1disp, size, &insword, &relword, ip->addr1);
	if (a2)
		dotp->xvalue += a2off =
		  genadr1(a2, ip->a2disp, size, &insword, &relword, ip->addr2);
	if (passno==1)
		return;
	a2 = sava2;
	if (ip->sdisp==SD)
		insword |= szcode[size];
	insword |= ip->opcode;
	if ((relword&TMASK)==TEA0|| (relword&TMASK)==TPC0) {
		if (ip->a1disp==DIM || ip->a1disp==DIMH)
			imoff += a1off;
		relword += 0100*imoff;
	}
	if ((relword&TMASK1)==(TEA0<<TSHFT1) || (relword&TMASK1)==(TPC0<<TSHFT1))
		relword += (0100<<TSHFT1)*a1off;
	if (ip->iflag&ISL) {		/* shrinkable long immed */
		if (a1->atype==AIMM || a2->atype==AIMM)
			relword = TIM0;
	}
	/* detect movm instructions */
	if ((insword&0175700) == 0044300 && (relword&TMASK) == 0)
		relword |= TMM;
	outhw(insword, N_ABS, SNULL, relword);
	if (ip->a2disp==DIM || ip->a2disp==DIMH) {
		genadr2(a2, ip->a2disp, size);
		a2 = 0;
	}
	if (a1) {
		dotp->xvalue -= a2off;
		genadr2(a1, ip->a1disp, size);
		dotp->xvalue += a2off;
	}
	if (a2)
		genadr2(a2, ip->a2disp, size);
}

genadr1(ap, disp, size, modep, relp, addreq)
register struct arg *ap;
int *modep, *relp;
{
	register mode;
	register struct exp *xp;
	int mvflag, naddrs;

	mode = 0;
	naddrs = 0;
	mvflag = 0;
	switch(disp) {

	case DIG:
		return(0);

	case DEAM:
		mvflag = 1;
	case DEA:
		switch(ap->atype) {

		case AREG:
			if (ap->areg1>DREGS+15)
				yyerror("Illegal register");
			mode = ap->areg1;
			break;

		case AIMM:
			xp = ap->xp;
			if (size==L)
				naddrs = 2;
			else {
				if ((xp->xtype & XTYPE)!=N_ABS && passno==2)
					yyerror("long instruction required for immediate");
				naddrs = 1;
			}
			mode = 074;	/* immediate */
			break;

		case AEXP:
			mode = 071;	/* absolute long */
			naddrs = 2;
			if (mvflag)
				*relp |= TEA0<<TSHFT1;
			else
				*relp |= (addreq&(AL|AM))==0?TPC0:TEA0;
			break;

		case AIREG:
			chkareg(ap->areg1);
			mode = 020 + (ap->areg1&07);
			break;

		case AINC:
			chkareg(ap->areg1);
			mode = 030 + (ap->areg1&07);
			break;

		case ADEC:
			chkareg(ap->areg1);
			mode = 040 + (ap->areg1&07);
			break;

		case APIC:
		case AOFF:
			if (ap->areg1==PCREG)
				mode = 072;
			else {
				chkareg(ap->areg1);
				mode = 050 + (ap->areg1&07);
			}
			naddrs = 1;
			break;

		case API2:
		case ANDX:
			if (ap->areg1 == PCREG)
				mode = 073;
			else {
				chkareg(ap->areg1);
				mode = 060 + (ap->areg1&07);
			}
			naddrs = 1;
			break;

		default:
			yyerror("Unimplemented effective address mode");
			break;
		}
		if (mvflag)
			mode = ((mode&07)<<9) | ((mode&070)<<3);
		break;

	case DRG:
		mode = ap->areg1&07;
		break;

	case DRGL:
		mode = (ap->areg1 & 07) << 9;
		break;

	case DAQ:
		xp = ap->xp;
		if (xp->xtype!=N_ABS || xp->xvalue<1 || xp->xvalue>8)
			yyerror("Illegal add-quick");
		mode = (xp->xvalue & 07) << 9;
		break;

	case DMQ:
		xp = ap->xp;
		if (xp->xtype!=N_ABS || xp->xvalue<-256 || xp->xvalue>255)
			yyerror("Illegal move-quick");
		mode = xp->xvalue & 0377;
		break;

	case DIM:
		xp = ap->xp;
		if (size==L)
			naddrs = 2;
		else {
			naddrs = 1;
			if ((xp->xtype & XTYPE)!=N_ABS && passno==2)
				yyerror("Absolute immediate required");
		}
		break;

	case DBR:
	case DBCC:
		xp = ap->xp;
		if (passno==2 && (xp->xtype&XTYPE) != dotp->xtype) {
			yyerror("Branch addr. in wrong segment");
		}
		if (disp==DBR) {
			if (size==B) {
				if (passno==2) {
					register long w = xp->xvalue - dotp->xvalue;
					if (w < -128L || w > 127L || w == 0L)
						yyerror("Branch address too remote");
					*modep|=w&0xFF;
				}
				*relp |= TBR0;
				naddrs=0;
			} else {
				*relp |= TBR1;
				naddrs = 1;
			}
		} else naddrs=1;
		break;

	case DIMH:
		xp = ap->xp;
		if (passno==2 && (xp->xtype&XTYPE)!=N_ABS)
			yyerror("Immediate must be absolute");
		naddrs = 1;
		break;

	default:
		yyerror("Unknown address disp.");
		break;
	}
	*modep |= mode;
	return(2*naddrs);
}

chkareg(reg)
{
	if (reg<AREGS || reg>AREGS+7)
		yyerror("Address register required");
}

genadr2(ap, disp, size)
register struct arg *ap;
{
	register struct exp *xp;
	register long w;

	switch(disp) {

	case DEAM:
	case DEA:
		switch(ap->atype) {

		case AIMM:
			outxpr(ap->xp, size);
			return;

		case AEXP:
			outxpr(ap->xp, L);
			return;

		case AOFF:
			xp = ap->xp;
			if (ap->areg1==PCREG && xp->xtype!=N_ABS)
				yyerror("PC offsets not in yet");
			outxpr(xp, W);
			return;

		case APIC:
			xp = ap->xp;
			if (ap->areg1==PCREG) {
				if ((xp->xtype&XTYPE)!=dotp->xtype)
					yyerror("PIC offset in wrong segment");
				size = XPCREL;
				w = xp->xvalue - dotp->xvalue + 2;
			} else {
				size = XOFFS;
				w = xp->xvalue - usedot[((xp->xtype&XTYPE)==N_TEXT) ? 0:NLOC].xvalue;
			}
			if (w<-32768 || w>32767)
				yyerror("Offset not in range");
			outhw((short)w, xp->xtype, xp->xname, size);
			return;

		case API2:
			xp = ap->xp;
			if (ap->areg1==PCREG) {
				if ((xp->xtype&XTYPE)!=dotp->xtype)
					yyerror("PIC offset in wrong segment");
				xp->xvalue -= dotp->xvalue - 2;
			} else
				xp->xvalue -= usedot[((xp->xtype&XTYPE)==N_TEXT) ? 0:NLOC].xvalue;
			xp->xtype = N_ABS;
				/* fall through to ANDX */
		case ANDX:
			xp = ap->xp;
			if ((xp->xtype&XTYPE)!=N_ABS
			 || (xp->xvalue<-128 || xp->xvalue>127))
				yyerror("Illegal double-index");
			w = xp->xvalue & 0377;
			if (ap->areg2<0 || ap->areg2>AREGS+7)
				yyerror("Illegal register");
			w |= (ap->areg2 & 017) << 12;
			if (ap->asize==L)
				w |= 04000;
			else if (ap->asize==W)
				;
			else
				yyerror("Illegal size");
			outhw((short)w, xp->xtype, xp->xname, 0);
			return;

		default:
			return;
		}

	case DIM:
		outxpr(ap->xp, size);
		break;

	case DBR:
	case DBCC:
		xp = ap->xp;
		w = xp->xvalue - dotp->xvalue + 2;	/* +2 ????? */
		if (w < -32768L || w > 32767L)
			yyerror("Branch address too remote");
		if (size!=B) outhw((short)w, xp->xtype, SNULL, XPCREL);
		break;

	case DIMH:
		outxpr(ap->xp, W);
		break;

	}
}

put2(v, f)
register FILE *f;
register short v;
{

	putc(v,  f);
	putc(v>>8, f);
}
