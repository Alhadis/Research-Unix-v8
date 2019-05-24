/*	@(#)xi.h	1.2	9/21/82	*/
typedef long	signal;
typedef unsigned long	bus;

#define long	int
#define when	/
#define prev	-
#define main	Xi_main
#define NEW `u0`
#define BUS(n,x)	bus x = ( *(int *)&x = (n<<16), x )
#define BUSARRAY(x,m,n)	Xi_array(x, "x", m, n)
#define ELT(x,i)	x[i]
#define AS(x,y)	Xi_as(x,y)
#define bundleval	busval

signal sigval(), busval(), bundle(), bundlename(); 
signal Xifield(), Xi_subs(), Xi_elt(), Xi_mpx(), Xi_lmpx();
signal Xi_pad(), inpad(), outpad(), clockpad();
signal inpin(), outpin(), inconn(), outconn();
signal insig(), outsig(), inbundle(), outbundle();
signal useclock(), usereset(), subclock(), curclock(), curreset();
signal any(), all(), mullerc();
extern signal TRUE, FALSE;
char *useblock(), *signame();
