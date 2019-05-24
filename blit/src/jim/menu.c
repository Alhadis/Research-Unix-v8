#include "r.h"
char *gnames[FIRSTFILE+NFRAME+1] = {
	"new",
	"reshape",
	"close",
	"write",
};
Menu generic = {
	gnames
};
char *lnames[] = {
	"cut",
	"paste",
	"snarf",
	0,	/* will be search string */
	0,	/* will be unix string */
	0,
};
Menu locmenu = {
	lnames
};
keepsearch(which){	/* icky routine */
	static char searchstring[11];
	static char unixstring[11];
	static seenunix=0;
	char *p=searchstring;
	char *cmd=DIAG->str->s;
	int n=3;
	if(which==0){	/* search */
		if(lnames[4]==0 && seenunix)
			lnames[4]=unixstring;
	}else{	/* unix */
		if(lnames[4] || (lnames[3] && !seenunix))
			n=4;
		p=unixstring;
		if(*cmd=='*')
			cmd++;
		seenunix=1;
	}
	movstring(10, cmd, lnames[n]=p);
}
Textframe *
frameoffile(f)
	register f;
{
	register Textframe *t;
	for(t=frame; t<&frame[NFRAME]; t++)
		if(f == t->file)
			return t;
	return 0;
}
setchar(f, n, c)
{
	gnames[f+FIRSTFILE][n]=c;
}
setname(f, s)
	register f;
	register char *s;
{
	register char **p=gnames+f+FIRSTFILE;
	if(*p==0){
		GCalloc(NAMELEN+1+3, p);	/* null plus '* */
		inittags(f);
	}
	strcpy(*p+3, s);
}
inittags(f)
{
	if(f)
		movstring(3, "   ", *(gnames+f+FIRSTFILE));
}
/* right pad file names to look good in menu */
adjustnames(n)
	register n;
{
	register char **p=gnames+FIRSTFILE+1;	/* +1 because we want file 0 not 1 */
	register i, l;
	n+=3;	/* number of tag characters */
	while(*p){
		if((l=strlen(*p)) < n){	/* pad out */
			for(i=l; i<n; i++)
				(*p)[i]=' ';
		}	/* else it's longer; truncate */
		(*p++)[n]=0;
	}
}
modified(t, mod)
	register Textframe *t;
{
	if(t!=DIAG)
		setchar(t->file, PRIME, " '"[mod]);
}
/*ARGSUSED*/
menulocal(t, pt, but)
	register Textframe *t;
	Point pt;
	int but;
{
	int hit=menuhit(&locmenu,2);
	if(t->selecthuge && CUT<=hit && hit<=SNARF){
		mesg("sorry; can't edit huge selection\n", 1);
		buttons(UP);
		return;
	}
	switch(hit) {
	case CUT:
		Send(O_CUT, 0, 0, "");
		cut(t, TRUE, t==DIAG? diagclr : F_CLR);
		break;
	case PASTE:
		if(BUF->n<=0)
			break;
		if(snarfhuge){
			mesg("sorry; can't paste with huge snarf buffer\n", 1);
			return;
		}
		/*
		 * Because selection of source and dest can
		 * be done in arb. order, must send selection.
		 */
		Send(O_SELECT, t->s1, 2, data2(t->s2-t->s1));
		Send(O_PASTE1, 0, 0, (char *)0);
		cut(t, FALSE, t==DIAG? diagclr : F_CLR);
		/*
		 * PASTE goes in two parts because cut() can call loadfile();
		 */
		Send(O_PASTE2, 0, 0, (char *)0);
		if(snarfhuge){
			move(t, Pt(0, t->scrolly), B2);
		}else{
			instext(t, BUF, t->s1);
			t->s2=t->s1+BUF->n;
			t->selecthuge=0;
			selectf(t, F_XOR);
		}
		break;
	case SNARF:
		if(t->s2 > t->s1){
			Send(O_SNARF, 0, 0, "");
			snarf(t->str, t->s1, t->s2, BUF);
		}
		break;
	case -1:
		break;
	default:
		if(workframe){
			send(workframe->file, O_SEARCH, hit-SEARCH, 0, (char *)0);
			waitunix(&diagdone);
		}
		break;
	}
}

Texture bullseye = {
	 0x07E0, 0x1FF8, 0x399C, 0x63C6, 0x6FF6, 0xCDB3, 0xD99B, 0xFFFF,
	 0xFFFF, 0xD99B, 0xCDB3, 0x6FF6, 0x63C6, 0x399C, 0x1FF8, 0x07E0,
};
/*ARGSUSED*/
menugeneric(t, pt, but)
	Textframe *t;
	Point pt;
	int but;
{
	register hit;
	register Textframe *t;
	Rectangle r;
	extern filedone;
	extern Textframe *obslist[];
	switch(hit=menuhit(&generic,3)){
	case -1:	/* no hit */
		break;
	case NEW:
		/* ask Unix for a file number */
		send(0, O_FILENAME, 0, 0, (char *)0);
		waitunix(&filedone);
		hit=filedone;
		if(hit < NFRAME)
			goto Get_it;
		break;
	case FRAME:
	case CLOSE:
	case WRITE:
		cursswitch(&bullseye);
		buttons(DOWN);
		t=pttoframe(mouse.xy);
		if(!button12() && t && t!=DIAG) switch(hit){
		case WRITE:
			send(t->file, O_WRITE, 0, 0, (char *)0);
			waitunix(&diagdone);
			break;
		case CLOSE:
			setchar(t->file, STARDOT, ' ');
			obscured(t);
			rectf(D, t->totalrect, F_CLR);
			if(t==workframe)
				workframe=0;
			closeframe(t);
			delobs(t);
			if(t==current)
				current=0;
			if(workframe){
				if(workframe->obscured)
					dodraw(workframe);
			}else if((t=obslist[0]) && t->obscured)
				dodraw(workframe=t);
			break;
		case FRAME:
			buttons(UP);
			if(userrect(&r)){
				if(workframe){
					setchar(workframe->file, STARDOT, '*');
					Rectf(workframe->scrollrect, F_XOR);
				}
				obscured(t);
				Rectf(t->totalrect, F_CLR);
				setrects(t, r, 0);
				setcpl(t, 0, t->nlines-1);
				dodraw(t);
				if(current==workframe)
					current=t;
				if(current==DIAG)
					curse(t);
				workframe=t;
				setchar(t->file, STARDOT, '.');
			}
			break;
		}
		cursswitch((Texture *)0);
		break;
	default:	/* must be a file; get it */
		hit-=FIRSTFILE;	/* reduce to file number */
	Get_it:
		t=frameoffile(hit);
		if(t==0){	/* get it */
			if(userrect(&r) == 0){
				buttons(UP);
				return;
			}
			t=newframe(r);
			t->file=hit;
			workinframe(t);
			if(current==DIAG)
				curse(t);
			setchar(hit, STARDOT, '*');
			seek(t, Pt(0, 0), 0);
		}
		if(current==DIAG){
			rXOR(DIAG->rect);
			curse(DIAG);	/* on */
			curse(workframe);	/* off */
		}
		workinframe(t);
		current=t;
		setchar(hit, STARDOT, '.');
		break;
	}
	buttons(UP);	
}
userrect(rp)
	register Rectangle *rp;
{
	*rp=getrect();
	return rectclip(rp, screenrect)
	   && (rp->corner.x-rp->origin.x)>100 && (rp->corner.y-rp->origin.y)>50;
}
extern int move(), seek(), menugeneric(), menulocal(), select();
int (*butfunc[3][3])()={
	select,	menulocal, menugeneric,
	move,	seek,	 move,
	(int (*)())opnull, (int (*)())opnull,  (int (*)())opnull
};
char whichbut[]={	/* depends on def'n of button1(), etc. */
	B1, B3, B2, B2, B1, B1, B1, B1
};
whichrect(t, pt)
	register Textframe *t;
	Point pt;
{
	if(ptinrect(pt, t->scrollrect))
		return SCROLLBAR;
	return FRAMERECT;
}
buttonhit(pt, but)
	Point pt;
	register but;
{
	register Textframe *t=pttoframe(pt);
	but=whichbut[but&7];
	/* icky special case to make scroll bar always active */
	if(t && t==workframe && ptinrect(pt, t->scrollrect)){
		if(current!=t)
			curse(t);
		(*(but==B2? seek : move))(t, pt, but);
		if(current!=t)
			curse(t);
	}else if(but==B1 && t!=current){
		if(t==DIAG || current==DIAG)
			rXOR(DIAG->rect);
		/* get the cursor right first */
		if(current==DIAG || (current==workframe && t==DIAG)){
			curse(DIAG);
			curse(workframe);
		}
		if(t==DIAG)
			diagclr=F_OR;
		else if(t){
			if(t!=workframe)
				workinframe(t);
			diagclr=F_CLR;
		}
		current=t;
		buttons(UP);
	}else{
		if(current==0 && but!=B3)
			return;	/* nothing to do */
	Doit:
		(*butfunc[whichrect(current, pt)][but])(current, pt, but); 
	}
}
