/***********************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T Technologies, Inc. All rigths Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T TECHNOLOGIES, INC.

	If you ignore this notice the ghost of Ma Bell will haunt you forever.

lalex.c:

	lexical lookahead
	unravel casts to handle cast/constructor ambiguity
	handle "struct x" => "x" transformation
	make NAME nodes for identifiers
	try to catch missing semi-colons
		(those sets of characters ought to be handled by table lookup)

	This is a mess; it really should have been a recursive decent parser

**************************************************************************/

#include "cfront.h"
#include "yystype.h"
#include "tqueue.h"

/*
	first define a queue of tokens (as a linked list)
*/

toknode* front = 0;
toknode* rear  = 0;

toknode* free_toks = 0;
#define NBITE 16
toknode.toknode(TOK t, YYSTYPE r)
{
	if (free_toks == 0) {
		register toknode* q = (toknode*)malloc( NBITE*sizeof(toknode) );
		free_toks=q;
		for (q+=NBITE-1, q->next=0; q!=free_toks; q--) (q-1)->next = q;
	}
	this = free_toks;
	free_toks = free_toks->next;
	tok = t;
	retval = r;
	next = last = 0;
}

inline toknode.~toknode()
{
	next = free_toks;
	free_toks = this;
	this = 0;
}


void addtok(TOK t, YYSTYPE r)
{
	toknode* T = new toknode(t,r);
	if (front == 0)
		front = rear = T;
	else {
		rear->next = T;
		T->last = rear;
		rear = T;
	}
}


TOK tk;		// last token returned

TOK deltok()
{
	toknode* T = front;
	tk = T->tok;
	yylval = T->retval;
	if (front = front->next) front->last = 0;
	delete T;
	return tk;
}

int scan_type();
int scan_mod();
int scan_tlist();
int scan_suf();
void get_tag();
void scan_e();


toknode* latok = 0;		/* "lookahead" token */

inline TOK  la_start()
{
	if (front == 0) tlex();
	latok=front;
	return latok->tok;
}

inline TOK  lookahead()
{
	if (latok==rear) tlex();
	latok=latok->next;
	return latok->tok;
}

inline void backup()
{
	if (latok->last == 0) error('i', "token q backup");
	latok = latok->last;
}

void insert_tok(TOK t)
{
	toknode* nt = new toknode(t, yylval);
	nt->last = latok->last;
	nt->last->next = nt;
	nt->next = latok;
	latok->last = nt;
	latok = nt;
}

void rep_cast()
/* replace bad cast with cast to ANY */
{
toknode* tt = front->next;	// front->tok == LP
toknode* junk = tt->next;
	if (junk == latok) return;
	tt->tok = TYPE;
	tt->retval.pt = any_type;
	tt->next = latok;
	latok->last->next = 0;
	latok->last = tt;
	for (tt = junk; tt; tt = tt->next) delete tt;
}

/*
	PRE-PARSER -- SCAN AHEAD TO DETERMINE TOKEN TYPE
*/

#define DO_RET goto ret

int bad_cast = 0;

TOK lalex()
{
static nocast = 0;	// prevent ID CAST, FOR CAST, etc.
//static in_op = 0;	// prevent OPERATOR op CAST
static incast = 0;	// don't scan in already recognized cast
static in_enum;
static fr;
char en = 0;

	switch (la_start()) {
	case VOLATILE:
	case SIGNED:
		error('w',"keyword%k (ignored)",latok->tok);
		return lalex();
	case ENUM:
		en = 1;
	case AGGR:
		switch (tk) {
/*				strictly speaking these ought to be included,
				but they only cause grief
		case ELSE:
		case COLON:	// label
		case RP:	// end of condition
*/
		case 0:
		case LP:	// argument list
		case CM:
		case NEW:
		case CAST:
		case RP:	// yok, old C: f(i) struct s * i; {}
		case OPERATOR:
		case DO:
		case TYPE:	// might be "const"
		case COLON:	// label
		case SM:	// end of statement
		case RC:	// end of statement
		case LC:	// first statement
			break;
		default:
			error(&curloc,"';' missing afterS orD before\"%k\"",latok->tok);
			return tk=SM;
		}

	{	TOK t = lookahead();
		TOK x;
//error('d',"aggr %k %d",t,t);
		switch (t) {
		case TNAME:
			x = lookahead();
			break;
		case ID:			// hidden or undefined
			x = lookahead();
			backup();
			switch (x) {
			case LC:
				in_enum = en;
			case COLON:		// defining: return AGGR ID
				backup();
				fr = 0;
				DO_RET;
			default:
			{	Pname n = ktbl->look(latok->retval.s,HIDDEN);
				if (n == 0) {	// new tag: define it
					n = new name(latok->retval.s);
					n->lex_level = 0;
					n = n->tname(latok->last->retval.t);
					modified_tn = modified_tn->l;
				}
				else {
					switch (n->tp->base) {
					case COBJ:
					case EOBJ:
						break;
					default:
						error('i',"hidden%n:%t",n,n->tp);
					}
				}
				latok->tok = TNAME;
				latok->retval.pn = n;
			}
			}
			(void)lookahead();
			break;
		case LC:
			in_enum = en;
		default:
			fr = 0;
			DO_RET;
		};
		
//error('d',"next %k %d tk %k",x,x,tk);
		switch (x) {
		case LC:		// class x {
			in_enum = en; 
		case COLON:		// class x :
			fr = 0;
			DO_RET;
		case SM:
			if (tk!=NEW && fr==0) {	// no further processing necessary
				deltok();	// class
				deltok();	// x
				deltok();	// ;
				return lalex();
			}
						// new class x ; => new x ;
		default:
			deltok();	// AGGR(?) TNAME(x) => TNAME(x)
			fr = 0;
			DO_RET;
		}	
	}

	case LP:
//error('d',"nocast %d in_op %d incast %d",nocast,in_op,incast);
		fr = 0;
		if (nocast) {
			nocast = 0;
			DO_RET;
//		} else if (in_op) {
//			in_op = 0;
//			DO_RET;
		} else if (incast)
			DO_RET;
		/* possible cast */
		bad_cast = 0;
		if (scan_type()) {
			if (scan_mod()) {
				if (lookahead() != RP) DO_RET;
				switch (lookahead()) {
				case CM:
				case RP:
				case SM:
				case LC:
				case ASSIGN:
					/* in declaration */
					if (tk != SIZEOF) DO_RET;
				}
				backup();
				front->tok = CAST;
				latok->tok = ENDCAST;
				if (bad_cast) {
					error("can't cast to function");
					rep_cast();
				}
				incast = 1;
			}
		}
		DO_RET;
	case CAST:
		incast++;
		DO_RET;
	case ENDCAST:
		if (--incast == 0) nocast = 0;
		DO_RET;
	case ID:
	{	char* s = front->retval.s;
//error('d',"id: %s",front->retval.s);
		fr = 0;
		nocast = 1;
		switch (lookahead()) {
		case ID:
		{	// handle ID ID
			// assume ID is a missing, hidden, or misspelt TNAME
			char* s2 = latok->retval.s;
			backup();
			Pname n = ktbl->look(s,HIDDEN);
			if (n == 0) {	// new tag: define it
				error("%s %s:TX (%s is not a TN)",s,s2,s);
				n = new name(s);
				n->lex_level = 0;
				n = n->tname(0);
				modified_tn = modified_tn->l;
				n->tp = any_type;
			}
			else {
				error("%s %s: %s is hidden",s,s2,s);
			}
			latok->tok = TNAME;
			latok->retval.pn = n;
			break;
		}
		case LC:
			backup();
			front->retval.pn = new name(s);
			front->retval.pn->lex_level--;
			break;
		default:
			backup();
			front->retval.pn = new name(s);
		}
		DO_RET;
	}
	case CASE:
	case DEFAULT:
	case PUBLIC:
	case ELSE:
		fr = 0;
		switch (tk) {
		case COLON:	// label
		case SM:	// end of statement
		case RC:	// end of statement
		case LC:	// first statement
			DO_RET;
		default:
			error(&curloc,"';' missing afterS orD before\"%k\"",latok->tok);
			return tk=SM;
		}
	case DO:
	case GOTO:
	case CONTINUE:
	case BREAK:
	case RETURN:
		fr = 0;
		switch (tk) {
		case ELSE:
		case DO:
		case COLON:	// label
		case RP:	// end of condition
		case SM:	// end of statement
		case RC:	// end of statement
		case LC:	// first statement
			DO_RET;
		default:
			error(&curloc,"';' missing afterS orD before\"%k\"",latok->tok);
			return tk=SM;
		}
	case IF:
	case WHILE:
	case FOR:
	case SWITCH:
		fr = 0;
		switch (tk) {
		case ELSE:
		case DO:
		case COLON:	// label
		case RP:	// end of condition
		case SM:	// end of statement
		case RC:	// end of statement
		case LC:	// first statement
			nocast = 1;
			DO_RET;
		default:
			error(&curloc,"';' missing afterS orD before\"%k\"",latok->tok);
			return tk=SM;
		}
	case TYPE:	// dangerous to diddle with: constructor notation
//fprintf(stderr,"type %d\n",tk); fflush(stderr);
		fr = 0;
		switch (tk) {
		case ID:
		//	case RP: 	old C function definition
		case RB:
			error(&curloc,"';' missing afterS orD before\"%k\"",latok->tok);
			return tk=SM;
		}
		if (latok->retval.t == FRIEND) fr = 1;
		nocast = 1;
		DO_RET;
	case TNAME:	// dangerous to diddle with: name hiding
	{	Pname n = latok->retval.pn;
		if (fr) {	// guard against: TYPE(friend) TNAME(x) SM
			nocast = 1;
			fr = 0;
			DO_RET;
		}
		fr = 0;
// fprintf(stderr,"type or tname %d %s\n",tk,n->string); fflush(stderr);
		switch (tk) {
		case TYPE:	// int TN ? or unsigned TN ?
			// beware of unsigned etc.
			switch (lookahead()) {
			case SM:
			case RB:
			case COLON:
			case ASSIGN:
				goto hid;
		//	case LP:	// the real problem
			default:
				nocast = 1;
				DO_RET;
			}
		case TNAME:	// TN TN ?
			switch (lookahead()) {
			case MEM:	// cl_name::mem_name
			case DOT:	// anachronism: cl_name.mem_name
				nocast = 1;
				DO_RET;
			}
		hid:
			backup();	// undo lookahead after TNAME
			n->hide();
			n = new name(n->string);
			n->n_oper = TNAME;
			latok->tok = ID;
			latok->retval.pn = n;
		}
	}
	case NEW:
		fr = 0;
		nocast = 1;
		DO_RET;
/*
	case OPERATOR:
		switch (lookahead()) {
		case LP:
			in_op = 1;
			if (lookahead() != RP) error("bad operator");
			break;
		case LB:
			if (lookahead() != RB) error("bad operator");
			break;
		case TYPE:
		case TNAME:
			while (lookahead() == MUL) ;
			backup();
		// default : 'regular' operator
		}
		if (lookahead() == LP) in_op = 1;
		DO_RET;
*/
	case RC:
		fr = 0;
		switch (tk) {
		case RC:	// nested } (either stmt or expr)
		case LC:	// empty block: {}
		case SM:	// end of statement
			break;
		default:
		{	TOK t;
			loc x = curloc;
			switch (t = lookahead()) {
			case ELSE:
			case RC:	// } } probably end of initializer
			case CM:	// } , definitely end of initializer
			case SM:	// } ; probably end of initializer or class
			case RP:	// int f( struct { ... } );
				break;
			default:
				// either	"= { ... E } SorD"
				// or		" SorD }"
				// or enum { a, b } c; - yuk
				if (in_enum==0 || t!=ID || (tk!=ID && tk!=ICON) ) {
					error(&x,"';'X at end ofS orD before '}'");
					in_enum = 0;
					return tk=SM;
				}
			}
		}
		}
		in_enum = 0;
	default:
		fr = 0;
		nocast = 0;
		DO_RET;
	}
ret:
// hand optimized return:
//TOK deltok()
//{
	toknode* T = front;
	tk = T->tok;
	yylval = T->retval;
	if (front = front->next) front->last = 0;
	delete T;
	return tk;
//}

}


int scan_type()
{
	int is_type = 0;
	for(;;)
		switch (lookahead()) {
		case AGGR:
		case ENUM:
			get_tag();
		case TYPE:
		case TNAME:
			is_type = 1;
			continue;
		default:
			backup();
			return is_type;
		}
}


int scan_mod()
/*
	  ptr mod
	| "(" mod ")"
	| mod suf
	| ...
*/
{
for(;;)
	switch (lookahead()) {
	case AND:
	case MUL:                     // ptr mod
		continue;
	case LP:                      // "(" mod ")" [suf] | suf
		switch (lookahead()) {
		case AND:
		case MUL:
		case LP:
		case LB:
			backup();
			if (!scan_mod()) return 0;
			if (lookahead() != RP) return 0;
			if (!scan_suf()) return 0;
			return 1;
		case AGGR: case ENUM: case TYPE: case TNAME:
			backup();
			if (!scan_tlist()) return 0;
			if (lookahead() != RP) return 0;
			/* no break */
		case RP:
			bad_cast = 1;	/* possible cast to ftn */
			if (!scan_suf()) return 0;
			return 1;
		default:
			return 0;
		}
	case LB:                      // mod suf
		backup();
		if (!scan_suf()) return 0;
		return 1;
	case RP:
	case CM:
	case ELLIPSIS:
		backup();
		return 1;
	default:
		return 0;
	}
}


int scan_suf()
/*
	suf vec | suf arg_type_list | ...
	vec --> "[" [ICON] "]"
	arg_type_list --> "(" [tlist] ")"
*/
{
int found = 0;
for(;;)
	switch (lookahead()) {
	case LB:
		scan_e();
		found = 1;
		continue;
	case LP:
		if (!scan_tlist()) return 0;
		if (lookahead() != RP) return 0;
		if (found) {
			bad_cast = 1;	/* possible cast to ftn */
		} else found = 1;
		continue;
	default:
		backup();
		return 1;
	}
}


int scan_tlist()
/*
	tlist type | type
	type --> (TYPE | [AGGR] TNAME) mod
*/
{
	for(;;) {
		switch (lookahead()) {
		case AGGR:
		case ENUM:
			get_tag();
		case TYPE:
		case TNAME:
			scan_type();
			break;
		case ELLIPSIS:
			if (lookahead() != RP) {
				error("missing ')' after '...'");
				insert_tok(RP);
			}
		case RP:
			backup();
			return 1;
		default:
			return 0;
		}

		/* saw type */
		if (!scan_mod()) return 0;

		switch (lookahead()) {
		case CM:
			continue;
		case ELLIPSIS:
			if (lookahead() != RP) {
				error("missing ')' after '...'");
				insert_tok(RP);
			}
		case RP:
			backup();
			return 1;
		default:
			return 0;
		}
	}
}

void get_tag()
/* saw AGGR or ENUM */
{
	switch (lookahead()) {
	default:
		error('e', "missing tag");
		insert_tok(ID);
		latok->retval.s = "__MISSING__";
	case ID:
	{
		Pname n = ktbl->look(latok->retval.s,HIDDEN);
		if (n == 0) {
			n = new name(latok->retval.s);
			n->lex_level = 0;
			n = n->tname(latok->last->retval.t);
			modified_tn = modified_tn->l;
		}
		else {
			switch (n->tp->base) {
			case COBJ:
			case EOBJ:
				break;
			default:
				error('i',"hidden%n:%t",n,n->tp);
			}
		}
		latok->tok = TNAME;
		latok->retval.pn = n;
		break;
	}
	case TNAME:
		break;
	}

	switch (lookahead()) {
	default:
		backup(); return;
	case COLON:
		switch (lookahead()) {
		case ID: case TNAME: case LC:
			break;
		default:
			backup(); return;
		}
	case LC:{ int level = 1;
		for(;;) switch (lookahead()) {
			case LC: level++; break;
			case RC: if(--level==0) return;
				break;
			case EOFTOK:
				error('i', "unexpected eof");
			}
		} // case LC
	} // switch
}

void scan_e()
/* scan expr in vec */
{
long brcount = 1L;
int localcast = 0;
	for(;;)
		switch (lookahead()) {
		case RB:
			if (--brcount == 0L)  return;
			continue;
		case LB:
			brcount++;
			continue;
		case LP:
			if (localcast)
				continue;
			toknode* mark = latok;
			if (scan_type())
				if (scan_mod())
					if (lookahead() == RP)
						switch (lookahead()) {
						case CM:
						case RP:
						case SM:
						case LC:
						case ASSIGN:
							break;
						default:
							backup();
							mark->tok = CAST;
							latok->tok = ENDCAST;
						}
			continue;
		case CAST:	/* scenario: local cast recognized,
					main cast in lalex fails,
					lalex later recognizes a smaller cast
						containing this one
				*/
			localcast++;
			continue;
		case ENDCAST:
			localcast--;
			continue;
		default:
			continue;
		}
}
