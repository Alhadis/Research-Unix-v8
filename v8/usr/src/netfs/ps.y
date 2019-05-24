%token MACHINE
%token LP RP NUMBER
%{
#include "stdio.h"
FILE *infd;
#define YYDEBUG
%}
%%
file:
	| machine-list ;
machine-list:	machine
	| machine-list machine
	;
machine:	MACHINE perm-list
	;
perm-list:
	| perm
	| perm-list perm
	;
perm:	LP client host RP
	;
client:	uid
		{ doclient($1, -1); }
	| LP uid gid RP
		{ doclient($2, $3); }
	;
host:	uid
		{ dohost($1, -1); }
	| LP uid gid RP
		{ dohost($2, $3); }
	;
uid:	NUMBER ;
gid:	NUMBER ;
%%
int lineno = 1;
yyerror(s)
char *s;
{	char buf[128];
	sprintf(buf, "%s line %d", s, lineno);
	perror(buf);
}

pparse(fd)
FILE *fd;
{
	infd = fd;
	yyparse();
}

yylex()
{	int c;
again:
	c = getc(infd);
	if(c == '\n')
		lineno++;
loop:
	if(c == EOF)
		return(0);
	if(c == '#') {
		do {
			c = getc(infd);
		} while(c != '\n' && c != EOF);
		if(c == '\n')
			lineno++;
		goto loop;
	}
	if(c == ' ' || c == ',' || c == '\n' || c == '\t')
		goto again;
	if(c == '(')
		return(LP);
	if(c == ')')
		return(RP);
	if(c == '+' || c == '-' || c >= '0' && c <= '9') {
		donumber(c);
		return(NUMBER);
	}
	if(c >= 'a' && c <= 'z' || c == '/') {
		domachine(c);
		lineno++;
		return(MACHINE);
	}
	debug("bad char in people (%c) 0%o ignored\n", c, c);
	goto again;
}

donumber(c)
int c;
{	int n, i, sign;
	n = 0;
	sign = 1;
	if(c == '-')
		sign = -1;
	else if(c != '+')
		n = c - '0';
	for(c = getc(infd); c >= '0' && c <= '9'; c = getc(infd))
		n = 10 * n + c - '0';
	ungetc(c, infd);
	yylval = sign * n;
}

static char nmbuf[128];
domachine(c)
{	char *p = nmbuf;
	*p++ = c;
	for(c = getc(infd); (c >= 'a' && c <= 'z' || c == '/') && p < nmbuf + 127; c=getc(infd))
		*p++ = c;
	*p = 0;
	newhost(nmbuf);
}
