/* @(#) yystype.h 1.1 3/7/85 08:46:02 */
typedef union {
	char*	s;
	TOK	t;
	int	i;
	loc	l;
	Pname	pn;
	Ptype	pt;
	Pexpr	pe;
	Pstmt	ps;
	PP	p;
} YYSTYPE;
extern YYSTYPE yylval;
