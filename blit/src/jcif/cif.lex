%%
B       return(B);
P       return(P);
C       return(C);
T       return(T);
M       return(M);
X       return(X);
Y       return(Y);
R       return(R);
W       return(W);
DS      return(DS);
DF      return(DF);
DD      return(DD);
E       return(E);
[0-9]*  {
	    yylval.intval = atoi(yytext);
	    return(INT);
	}
L       return(L);
NE	return(NE);
NM      return(NM);
SM	return(NM);
ND      return(ND);
SIS	return(ND);
NP      return(NP);
SP	return(NP);
NC      return(NC);
SC	return(NC);
NI      return(NI);
SIM	return(NI);
NG      return(NG);
SG	return(NG);
NB      return(NB);
[ ,\ta-z\#]   ;
\(      {
	    int i,c;
	    i = 1;
	    while (i > 0) {
		if ((c = input()) == '\n')
		    linecnt++;
		else if (c == '(')
		    i++;
		else if (c == ')')
		    i--;
	    }
	}
\n      linecnt++;
\-      return(MINUS);
\;      return(SEMI);
"/"	;
%%
