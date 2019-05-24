#include "defs"


warn1(s,t)
char *s, *t;
{
char buff[100];
sprintf(buff, s, t);
warn(buff);
}


warn(s)
char *s;
{
if(nowarnflag)
	return;
fprintf(diagfile, "Warning on line %d of %s: %s\n", lineno, infname, s);
++nwarn;
}


errstr(s, t)
char *s, *t;
{
char buff[100];
sprintf(buff, s, t);
err(buff);
}



erri(s,t)
char *s;
int t;
{
char buff[100];
sprintf(buff, s, t);
err(buff);
}


err(s)
char *s;
{
fprintf(diagfile, "Error on line %d of %s: %s\n", lineno, infname, s);
++nerr;
}


yyerror(s)
char *s;
{ err(s); }



dclerr(s, v)
char *s;
Namep v;
{
char buff[100];

if(v)
	{
	sprintf(buff, "Declaration error for %s: %s", varstr(VL, v->varname), s);
	err(buff);
	}
else
	errstr("Declaration error %s", s);
}



execerr(s, n)
char *s, *n;
{
char buf1[100], buf2[100];

sprintf(buf1, "Execution error %s", s);
sprintf(buf2, buf1, n);
err(buf2);
}


fatal(t)
char *t;
{
fprintf(diagfile, "Compiler error line %d of %s: %s\n", lineno, infname, t);
if(debugflag)
	abort();
done(3);
exit(3);
}




fatalstr(t,s)
char *t, *s;
{
char buff[100];
sprintf(buff, t, s);
fatal(buff);
}



fatali(t,d)
char *t;
int d;
{
char buff[100];
sprintf(buff, t, d);
fatal(buff);
}



badthing(thing, r, t)
char *thing, *r;
int t;
{
char buff[50];
sprintf(buff, "Impossible %s %d in routine %s", thing, t, r);
fatal(buff);
}



badop(r, t)
char *r;
int t;
{
badthing("opcode", r, t);
}



badtag(r, t)
char *r;
int t;
{
badthing("tag", r, t);
}





badstg(r, t)
char *r;
int t;
{
badthing("storage class", r, t);
}




badtype(r, t)
char *r;
int t;
{
badthing("type", r, t);
}


many(s, c, n)
char *s, c;
int n;
{
char buff[250];

sprintf(buff,
"Too many %s.\nTable limit now %d.\nTry recompiling using the -N%c%d option\n",
	s, n, c, 2*n);
fatal(buff);
}


err66(s)
char *s;
{
errstr("Fortran 77 feature used: %s", s);
}



errext(s)
char *s;
{
errstr("F77 compiler extension used: %s", s);
}
