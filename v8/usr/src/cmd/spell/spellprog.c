/*	@(#)spellprog.c	1.1	*/
#include <stdio.h>
#include <ctype.h>
#define Tolower(c) (isupper(c)?tolower(c):c)
#define pair(a,b) (((a)<<8)|(b))
#define DLEV 2

char	*strcat();
int	strip();
int cstrip();
int	subst();
char	*skipv();
int	an();
int	s();
int	es();
int	ily();
int	CCe();
int	VCe();
int	bility();
int	tion();
int	ize();
int	y_to_e();
int	i_to_y();
int	nop();

struct suftab {
	char *suf;
	int (*p1)();
	int n1;
	char *d1;
	char *a1;
	int (*p2)();
	int n2;
	char *d2;
	char *a2;
};
struct suftab stabc[] = {
	{"citsi",strip,2,"","+ic"},
	{"citi",ize,1,"-e+ic",""},
	{"cihparg",i_to_y,1,"-y+ic",""},
	{"cipocs",ize,1,"-e+ic",""},
	{"cirtem",i_to_y,1,"-y+ic",""},
	{"cigol",i_to_y,1,"-y+ic",""},
	0
};
struct suftab stabd[] = {
	{"de",strip,1,"","+d",		i_to_y,2,"-y+ied","+ed"},
	{"dooh",ily,4,"-y+ihood","+hood"},
	0
};
struct suftab stabe[] = {
	{"ecn",subst,1,"-t+ce",""},
	{"elbaif",i_to_y,4,"-y+iable",""},
	{"elba",CCe,4,"-e+able","+able"},
	{"evi",subst,0,"-ion+ive",""},
	{"ezi",CCe,3,"-e+ize","+ize"},
	{"ekil",strip,4,"","+like"},
	0
};
struct suftab stabg[] = {
	{"gni",CCe,3,"-e+ing","+ing"},
	0
};
struct suftab stabl[] = {
	{"laci",strip,2,"","+al"},
	{"latnem",strip,2,"","+al"},
	{"lanoi",strip,2,"","+al"},
	{"luf",ily,3,"-y+iful","+ful"},
	0
};
struct suftab stabm[] = {
	{"msi",CCe,3,"-e+ism","ism"},
	0
};
struct suftab stabn[] = {
	{"noitacifi",i_to_y,6,"-y+ication",""},
	{"noitazi",ize,4,"-e+ation",""},
	{"noit",tion,3,"-e+ion","+ion"},
	{"naino",an,3,"","+ian"},
	{"na",an,1,"","+n"},
	0
};
struct suftab stabp[] = {
	{"pihs",strip,4,"","+ship"},
	0
};
struct suftab stabr[] = {
	{"reta",nop,0,"",""},
	{"retc",nop,0,"",""},
	{"re",strip,1,"","+r",		i_to_y,2,"-y+ier","+er"},
	{"rota",tion,2,"-e+or",""},
	{"rotc",tion,2,"","+or"},
	0
};
struct suftab stabs[] = {
	{"ssen",ily,4,"-y+iness","+ness" },
	{"ssel",ily,4,"-y+i+less","+less" },
	{"se",s,1,"","+s",		es,2,"-y+ies","+es" },
	{"s'",s,2,"","+'s"},
	{"s",s,1,"","+s"},
	0
};
struct suftab stabt[] = {
	{"tnem",strip,4,"","+ment"},
	{"tse",strip,2,"","+st",	i_to_y,3,"-y+iest","+est"},
	{"tsigol",i_to_y,2,"-y+ist",""},
	{"tsi",CCe,3,"-e+ist","+ist"},
	0
};
struct suftab staby[] = {
	{"ycn",subst,1,"-t+cy",""},
	{"ytilb",nop,0,"",""},
	{"ytilib",bility,5,"-le+ility",""},
	{"yti",CCe,3,"-e+ity","+ity"},
	{"ylb",y_to_e,1,"-e+y",""},
	{"ylc",nop,0,"",""},
	{"yl",ily,2,"-y+ily","+ly"},
	{"yrtem",subst,0,"-er+ry",""},
	0
};
struct suftab stabz[] = {
	0
};
struct suftab *suftab[] = {
	stabz,
	stabz,
	stabc,
	stabd,
	stabe,
	stabz,
	stabg,
	stabz,
	stabz,
	stabz,
	stabz,
	stabl,
	stabm,
	stabn,
	stabz,
	stabp,
	stabz,
	stabr,
	stabs,
	stabt,
	stabz,
	stabz,
	stabz,
	stabz,
	staby,
	stabz,
};


char *ptaba[] = {
	"anti",
	"auto",
	0
};
char *ptabb[] = {
	"bio",
	0
};
char *ptabc[] = {
	"counter",
	0
};
char *ptabd[] = {
	"dis",
	0
};
char *ptabe[] = {
	"electro",
	"en",
	0
};
char *ptabf[] = {
	"femto",
	"fore",
	0
};
char *ptabg[] = {
	"geo",
	"giga",
	0
};
char *ptabh[] = {
	"hyper",
	0
};
char *ptabi[] = {
	"intra",
	"inter",
	"iso",
	0
};
char *ptabj[] = {
	0
};
char *ptabk[] = {
	"kilo",
	0
};
char *ptabl[] = {
	0
};
char *ptabm[] = {
	"magneto",
	"mega",
	"meta",
	"micro",
	"mid",
	"milli",
	"mis",
	"mono",
	"multi",
	0
};
char *ptabn[] = {
	"nano",
	"non",
	0
};
char *ptabo[] = {
	"out",
	"over",
	0
};
char *ptabp[] = {
	"photo",
	"pico",
	"poly",
	"pre",
	"pseudo",
	"psycho",
	0
};
char *ptabq[] = {
	"quasi",
	0
};
char *ptabr[] = {
	"re",
	0
};
char *ptabs[] = {
	"semi",
	"stereo",
	"sub",
	"super",
	0
};
char *ptabt[] = {
	"tele",
	"thermo",
	0
};
char *ptabu[] = {
	"ultra",
	"under",	/*must precede un*/
	"un",
	0
};
char *ptabv[] = {
	0
};
char *ptabw[] = {
	0
};
char *ptabx[] = {
	0
};
char *ptaby[] = {
	0
};
char *ptabz[] = {
	0
};

char **preftab[] = {
	ptaba,
	ptabb,
	ptabc,
	ptabd,
	ptabe,
	ptabf,
	ptabg,
	ptabh,
	ptabi,
	ptabj,
	ptabk,
	ptabl,
	ptabm,
	ptabn,
	ptabo,
	ptabp,
	ptabq,
	ptabr,
	ptabs,
	ptabt,
	ptabu,
	ptabv,
	ptabw,
	ptabx,
	ptaby,
	ptabz,
};

int bflag;
int vflag;
int xflag;
char word[100];
char original[100];
char *deriv[40];
char affix[40];
char errmsg[] = "spell: cannot initialize hash table\n";
FILE *found;
/*	deriv is stack of pointers to notes like +micro +ed
 *	affix is concatenated string of notes
 *	the buffer size 141 stems from the sizes of original and affix.
*/

/*
 *	in an attempt to defray future maintenance misunderstandings, here is an attempt to
 *	describe the input/output expectations of the spell program.
 *
 *	spellprog is intended to be called from the shell file spell.
 *	because of this, there is little error checking (this is historical, not
 *	necessarily advisable).
 *
 *	spellprog hashed-list pass options
 *
 *	the hashed-list is a list of the form made by spellin.
 *	there are 2 types of hashed lists:
 *		1. a stop list: this specifies words that by the rules embodied in
 *		   spellprog would be recognized as correct, BUT are really errors.
 *		2. a dictionary of correctly spelled words.
 *	the pass number determines how the words found in the specified hashed-list
 *	are treated. If the pass number is 1, the hashed-list is treated as the stop-list,
 *	otherwise, it is treated as the regular dictionary list. in this case, the
 *	value of "pass" is a filename. Found words are written to this file.
 *	In the normal case, the filename = /dev/null. However, if the v option is
 *	specified, the derivations are written to this file.
 *	the spellprog looks up words in the hashed-list; if a word is found, it is
 *	printed to the stdout. if the hashed-list was the stop-list, the words found
 *	are presumed to be misspellings. in this case,
 *	a control character is printed ( a "-" is appended to the word.
 *	a hyphen will never occur naturally in the input list because deroff
 *	is used in the shell file before calling spellprog.)
 *	if the regualar spelling list was used (hlista or hlistb), the words are correct,
 *	and may be ditched. (unless the -v option was used - see the manual page).

 *	spellprog should be called twice : first with the stop-list, to flag all
 *	a priori incorrectly spelled words; second with the dictionary.
 *
 *	spellprog hstop 1 |\
 *	spellprog hlista /dev/null 
 *
 *	for a complete scenario, see the shell file: spell.
 *
 */
main(argc,argv)
char **argv;
{
	register char *ep, *cp;
	register char *dp;
	int fold;
	int j;
	int pass;
	if(!prime(argc,argv)) {
		fwrite(errmsg, sizeof(*errmsg), sizeof(errmsg), stderr);
		exit(1);
	}
/*
 *	if pass is not 1, it is assumed to be a filename.
 *	found words are written to this file.
 */
	pass = argv[2][0] ;
	if ( pass != '1' )
		found = fopen(argv[2],"w");
	for(argc-=3,argv+=3; argc>0 && argv[0][0]=='-'; argc--,argv++)
		switch(argv[0][1]) {
		case 'b':
			bflag++;
			ise();
			break;
		case 'v':
			vflag++;
			break;
		case 'x':
			xflag++;
			break;
		}
	for(;;) {
		affix[0] = 0;
		for(ep=word;(*ep=j=getchar())!='\n';ep++)
			if(j == EOF)
				exit(0);
		for(cp=word,dp=original; cp<ep; )
			*dp++ = *cp++;
		*dp = 0;
/*
 *	here is the hyphen processing. these words were found in the stop
 *	list. however, if they exist as is, (no derivations tried) in the
 *	dictionary, let them through as correct.
 *
 */
		if(ep[-1]=='-') {
			*--dp = *--ep = 0;
			if(tryword(word,ep,0))
				continue;
			if(!isupper(word[0])) 
				goto notfound;
			if(isupper(word[1])) {
				for(cp=word+1;cp<ep;cp++) {
					if(islower(*cp))
						goto notfound;
					*cp = Tolower(*cp);
				}
			}
			if(tryword(word,ep,0))
				continue;
			word[0] = Tolower(word[0]);
			if(tryword(word,ep,0))
				continue;
			goto notfound;
		}
		fold = 0;
		for(cp=word;cp<ep;cp++)
			if(islower(*cp))
				goto lcase;
		if(trypref(ep,".",0))
			goto foundit;
		++fold;
		for(cp=original+1,dp=word+1;dp<ep;dp++,cp++)
			*dp = Tolower(*cp);
lcase:
		if(trypref(ep,".",0)||trysuff(ep,0))
			goto foundit;
		if(isupper(word[0])) {
			for(cp=original,dp=word; *dp = *cp++; dp++)
				if (fold) *dp = Tolower(*dp);
			word[0] = Tolower(word[0]);
			goto lcase;
		}
notfound:
		printf("%s\n", original);
		continue;

foundit:
		if(pass=='1')
			printf("%s-\n", original);
		else if(affix[0]!=0 && affix[0]!='.') {
			fprintf(found, "%s\t%s\n", affix, original);
		}
	}
}

/*	strip exactly one suffix and do
 *	indicated routine(s), which may recursively
 *	strip suffixes
*/
trysuff(ep,lev)
char *ep;
{
	register struct suftab *t;
	register char *cp, *sp;
	int initchar = ep[-1];
	lev += DLEV;
	deriv[lev] = deriv[lev-1] = 0;
	if(!islower(initchar))
		return(0);
	for(t=suftab[initchar-'a'];sp=t->suf;t++) {
		cp = ep;
		while(*sp)
			if(*--cp!=*sp++)
				goto next;
		for(sp=cp; --sp>=word&&!vowel(*sp); ) ;
		if(sp<word)
			return(0);
		if((*t->p1)(ep-t->n1,t->d1,t->a1,lev+1))
			return(1);
		if(t->p2!=0) {
			deriv[lev] = deriv[lev+1] = 0;
			return((*t->p2)(ep-t->n2,t->d2,t->a2,lev));
		}
		return(0);
next:		;
	}
	return(0);
}

nop()
{
	return(0);
}

cstrip(ep,d,a,lev)
char *ep,*d,*a;
{
	register temp = ep[0];
	if(vowel(temp)&&vowel(ep[-1])) {
		switch(pair(ep[-1],ep[0])) {
		case pair('a', 'a'):
		case pair('a', 'e'):
		case pair('a', 'i'):
		case pair('e', 'a'):
		case pair('e', 'e'):
		case pair('e', 'i'):
		case pair('i', 'i'):
		case pair('o', 'a'):
			return(0);
		}
	} else if(temp==ep[-1]&&temp==ep[-2])
		return(0);
	return(strip(ep,d,a,lev));
}

strip(ep,d,a,lev)
char *ep,*d,*a;
{
	return(trypref(ep,a,lev)||trysuff(ep,lev));
}

s(ep,d,a,lev)
char *ep,*d,*a;
{
	if(lev>DLEV+1)
		return(0);
	if(*ep=='s') {
		switch(ep[-1]) {
		case 'y':
			if(vowel(ep[-2]))
				break;
		case 'x':
		case 'z':
		case 's':
			return(0);
		case 'h':
			switch(ep[-2]) {
			case 'c':
			case 's':
				return(0);
			}
		}
	}
	return(strip(ep,d,a,lev));
}

an(ep,d,a,lev)
char *ep,*d,*a;
{
	if(!isupper(*word))	/*must be proper name*/
		return(0);
	return(trypref(ep,a,lev));
}

ize(ep,d,a,lev)
char *ep,*d,*a;
{
	register temp = ep[-1];
	register val;
	ep[-1] = 'e';
	val = strip(ep,"",d,lev);
	ep[-1] = temp;
	return(val);
}

y_to_e(ep,d,a,lev)
char *ep,*d,*a;
{
	register val, temp;
	switch(ep[-1]) {
	case 'a':
	case 'e':
	case 'i':
		return 0;
	}
	temp = *ep;
	*ep++ = 'e';
	val = strip(ep,"",d,lev);
	*--ep = temp;
	return(val);
}

ily(ep,d,a,lev)
char *ep,*d,*a;
{
	register temp = ep[0];
	if(temp==ep[-1]&&temp==ep[-2])
		return(0);
	if(ep[-1]=='i')
		return(i_to_y(ep,d,a,lev));
	else
		return(cstrip(ep,d,a,lev));
}

bility(ep,d,a,lev)
char *ep,*d,*a;
{
	*ep++ = 'l';
	return(y_to_e(ep,d,a,lev));
}

i_to_y(ep,d,a,lev)
char *ep,*d,*a;
{
	register val, temp;
	if((temp=ep[-1])=='i'&&!vowel(ep[-2])) {
		ep[-1] = 'y';
		a = d;
	}
	val = cstrip(ep,"",a,lev);
	ep[-1] = temp;
	return(val);
}

es(ep,d,a,lev)
char *ep,*d,*a;
{
	if(lev>DLEV)
		return(0);
	switch(ep[-1]) {
	default:
		return(0);
	case 'i':
		return(i_to_y(ep,d,a,lev));
	case 's':
	case 'h':
	case 'z':
	case 'x':
		return(strip(ep,d,a,lev));
	}
}

subst(ep,d,a,lev)
char *ep, *d, *a;
{
	char *u,*t;
	int val;
	if(skipv(skipv(ep-1))<word)
		return(0);
	for(t=d;*t!='+';t++)
		continue;
	for(u=ep;*--t!='-'; )
		*--u = *t;
	val = strip(ep,"",d,lev);
	while(*++t!='+')
		continue;
	while(*++t)
		*u++ = *t;
	return(val);
}


tion(ep,d,a,lev)
char *ep,*d,*a;
{
	switch(ep[-2]) {
	case 'c':
	case 'r':
		return(trypref(ep,a,lev));
	case 'a':
		return(y_to_e(ep,d,a,lev));
	}
	return(0);
}

/*	possible consonant-consonant-e ending*/
CCe(ep,d,a,lev)
char *ep,*d,*a;
{
	switch(ep[-1]) {
	case 'l':
		if(vowel(ep[-2]))
			break;
		switch(ep[-2]) {
		case 'l':
		case 'r':
		case 'w':
			break;
		default:
			return(y_to_e(ep,d,a,lev));
		}
		break;
	case 's':
		if(ep[-2]=='s')
			break;
	case 'c':
	case 'g':
		if(*ep=='a')
			return(0);
	case 'v':
	case 'z':
		if(vowel(ep[-2]))
			break;
	case 'u':
		if(y_to_e(ep,d,a,lev))
			return(1);
		if(!(ep[-2]=='n'&&ep[-1]=='g'))
			return(0);
	}
	return(VCe(ep,d,a,lev));
}

/*	possible consonant-vowel-consonant-e ending*/
VCe(ep,d,a,lev)
char *ep,*d,*a;
{
	char c;
	c = ep[-1];
	if(c=='e')
		return(0);
	if(!vowel(c) && vowel(ep[-2])) {
		if(bflag&&c=='l'&&ep[-2]=='e')	/* UK modelled vs US modeled */
			return(0);
		c = *ep;
		*ep++ = 'e';
		if(trypref(ep,d,lev)||trysuff(ep,lev))
			return(1);
		ep--;
		*ep = c;
	}
	return(cstrip(ep,d,a,lev));
}

char *lookuppref(wp,ep)
char **wp;
char *ep;
{
	register char **sp;
	register char *bp,*cp;
	int initchar = Tolower(**wp);
	if(!isalpha(initchar))
		return(0);
	for(sp=preftab[initchar-'a'];*sp;sp++) {
		bp = *wp;
		for(cp= *sp;*cp;cp++,bp++)
			if(Tolower(*bp)!=*cp)
				goto next;
		for(cp=bp;cp<ep;cp++) 
			if(vowel(*cp)) {
				*wp = bp;
				return(*sp);
			}
next:	;
	}
	return(0);
}

/*	while word is not in dictionary try stripping
 *	prefixes. Fail if no more prefixes.
*/
trypref(ep,a,lev)
char *ep,*a;
{
	register char *cp;
	char *bp;
	register char *pp;
	int val = 0;
	char space[20];
	deriv[lev] = a;
	if(tryword(word,ep,lev))
		return(1);
	bp = word;
	pp = space;
	deriv[lev+1] = pp;
	while(cp=lookuppref(&bp,ep)) {
		*pp++ = '+';
		while(*pp = *cp++)
			pp++;
		if(tryword(bp,ep,lev+1)) {
			val = 1;
			break;
		}
	}
	deriv[lev+1] = deriv[lev+2] = 0;
	return(val);
}

tryword(bp,ep,lev)
char *bp,*ep;
{
	register i, j;
	char duple[3];
	if(ep-bp<=1)
		return(0);
	if(vowel(*ep)) {
		if(monosyl(bp,ep))
			return(0);
	}
	i = dict(bp,ep);
	/* doubled consonant in monosyllables and UK 'modelled', etc */
	if(i==0&&vowel(*ep)&&ep[-1]==ep[-2]&&
		(monosyl(bp,ep-1)||bflag&&ep[-1]=='l'&&ep[-3]=='e')) {
		ep--;
		deriv[++lev] = duple;
		duple[0] = '+';
		duple[1] = *ep;
		duple[2] = 0;
		i = dict(bp,ep);
	}
	if(vflag==0||i==0)
		return(i);
	/*	when derivations are wanted, collect them
	 *	for printing
	*/
	j = lev;
	do {
		if(deriv[j])
			strcat(affix,deriv[j]);
	} while(--j>0);
	return(i);
}


monosyl(bp,ep)
char *bp, *ep;
{
	if(ep<bp+2)
		return(0);
	if(vowel(*--ep)||!vowel(*--ep)
		||ep[1]=='x'||ep[1]=='w')
		return(0);
	while(--ep>=bp)
		if(vowel(*ep)&&!(ep[-1]=='q'&&*ep=='u'))
			return(0);
	return(1);
}

char *
skipv(s)
char *s;
{
	if(s>=word&&vowel(*s))
		s--;
	while(s>=word&&!vowel(*s))
		s--;
	return(s);
}

vowel(c)
{
	switch(Tolower(c)) {
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	case 'y':
		return(1);
	}
	return(0);
}

/* crummy way to Britishise */
ise()
{
	register struct suftab *p;
	register i;
	for(i=0; i<26; i++) {
		for(p = suftab[i];p->suf;p++) {
			ztos(p->suf);
			ztos(p->d1);
			ztos(p->a1);
		}
	}
}
ztos(s)
char *s;
{
	for(;*s;s++)
		if(*s=='z')
			*s = 's';
}

dict(bp,ep)
char *bp, *ep;
{
	register temp, result;
	if(xflag)
		fprintf(stderr, "=%.*s\n", ep-bp, bp);
	temp = *ep;
	*ep = 0;
	result = hashlook(bp);
	*ep = temp;
	return(result);
}
