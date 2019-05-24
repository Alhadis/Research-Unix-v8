#include "regprog.h"

/*
 *	Machine state
 */
#define LISTINCREMENT 8
typedef struct List{
	Inst	*inst;		/* Instruction of the thread */
	Subexp	se;		/* matched subexpressions in this thread */
}List;
static List	*tl, *nl;	/* This list, next list */
static List	*tle, *nle;	/* ends of this and next list */
static List	*list[2];
static List	*liste[2];
static int	listsize = LISTINCREMENT;

static Subexp sempty;		/* empty set of matches */
static int match;		/* true if match is found */

/*
 * Note optimization in addinst:
 * 	*lp must be pending when addinst called; if *l has been looked
 *		at already, the optimization is a bug.
 */
static List *
newthread(lp, ip, sep)
	List *lp;	/* list to add to */
	Inst *ip;	/* instruction to add */
	Subexp *sep;	/* pointers to subexpressions */
{
	register List *p;

	for(p=lp; p->inst != NULL; p++){
		if(p->inst==ip){
			if((sep)->startp[0] < p->se.startp[0])
				p->se = *sep;
			return NULL;
		}
	}
	p->inst = ip;
	p->se = *sep;
	(++p)->inst = NULL;
	return p;
}

static
newmatch(subp, newp)
	Subexp *subp;
	Subexp *newp;
{
	if(subp->startp[0]==0 || newp->startp[0]<subp->startp[0] ||
	   (newp->startp[0]==subp->startp[0] && newp->endp[0]>subp->endp[0]))
		*subp = *newp;
	match = 1;
}

extern char *
regexec(progp, starts)
	Prog *progp;	/* program to run */
	char *starts;	/* string to run machine on */
{
	register flag=0;
	register Inst *inst;
	register List *tlp;
	register char *s;
	int startchar=progp->startinst->type<OPERATOR? progp->startinst->type : 0;
	int i, checkstart;

restart:
	match = 0;
	checkstart = startchar;
	sempty.startp[0] = NULL;
	progp->se = sempty;
	if (list[0] == NULL) {
		list[0] = (List *)malloc(2*listsize*sizeof(List));
		list[1] = list[0] + listsize;
		liste[0] = list[0] + listsize - 1;
		liste[1] = list[1] + listsize - 1;
		if (list[0] == NULL)
			regerror("list overflow");
	}
	list[0][0].inst = list[1][0].inst = NULL;

	/* Execute machine once for each character, including terminal NUL */
	s=starts;
	do{
		/* fast check for first char */
		if(checkstart && *s!=startchar)
			continue;
		tl=list[flag];
		tle=liste[flag];
		nl=list[flag^=1];
		nle=liste[flag];
		nl->inst=0;
		/* Add first instruction to this list */
		sempty.startp[0] = s;
		(void)newthread(tl, progp->startinst, &sempty);
		/* Execute machine until this list is empty */
		for(tlp=tl; inst=tlp->inst; tlp++){	/* assignment = */
	Switchstmt:
			switch(inst->type){
			default:	/* regular character */
				if(inst->type == *s){
	Addinst:
					if(newthread(nl, inst->next, &tlp->se)==nle)
						goto realloc;
				}
				break;
			case LBRA:
				tlp->se.startp[inst->subid] = s;
				inst=inst->next;
				goto Switchstmt;
			case RBRA:
				tlp->se.endp[inst->subid] = s;
				inst=inst->next;
				goto Switchstmt;
			case ANY:
				goto Addinst;
			case BOL:
				if(s == starts){
					inst=inst->next;
					goto Switchstmt;
				}
				break;
			case EOL:
				if(*s=='\0'){
					inst=inst->next;
					goto Switchstmt;
				}
				break;
			case CCLASS:
				if(((char *)inst->right)[*s/8]&(1<<(*s&07)))
					goto Addinst;
				break;
			case OR:
				/* evaluate right choice later */
				if (newthread(tlp, inst->right, &tlp->se) == tle)
					goto realloc;
				/* efficiency: advance and re-evaluate */
				inst=inst->left;
				goto Switchstmt;
			case END:	/* Match! */
				tlp->se.endp[0] = s;
				newmatch(&progp->se, &tlp->se);
				break;
			}
		}
		checkstart = startchar && nl->inst==NULL;
	}while(*s++);
	return match;
realloc:
	free(list[0]);
	list[0] = NULL;
	listsize += LISTINCREMENT;
	goto restart;
}

