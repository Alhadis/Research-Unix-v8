/*
 *	test expression
 *	[ expression ]
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#define EQ(a,b)	((tmp=a)==0?0:(strcmp(tmp,b)==0))

int	ap;
int	ac;
char	**av;
char	*tmp;

main(argc, argv)
char *argv[];
{

	ac = argc; av = argv; ap = 1;
	if(EQ(argv[0],"[")) {
		if(!EQ(argv[--ac],"]"))
			synbad("] missing","");
	}
	argv[ac] = 0;
	if (ac<=1) exit(1);
	exit(exp()?0:1);
}

char *nxtarg(mt) {

	if (ap>=ac) {
		if(mt) {
			ap++;
			return(0);
		}
		synbad("argument expected","");
	}
	return(av[ap++]);
}

exp() {
	int p1;

	p1 = e1();
	if (EQ(nxtarg(1), "-o")) return(p1 | exp());
	ap--;
	return(p1);
}

e1() {
	int p1;

	p1 = e2();
	if (EQ(nxtarg(1), "-a")) return (p1 & e1());
	ap--;
	return(p1);
}

e2() {
	if (EQ(nxtarg(0), "!"))
		return(!e3());
	ap--;
	return(e3());
}

e3() {
	int p1;
	register char *a;
	char *p2;
	int int1, int2;

	a=nxtarg(0);
	if(EQ(a, "(")) {
		p1 = exp();
		if(!EQ(nxtarg(0), ")")) synbad(") expected","");
		return(p1);
	}

	if(EQ(a, "-r"))
		return(tio(nxtarg(0), 4));

	if(EQ(a, "-w"))
		return(tio(nxtarg(0), 2));

	if(EQ(a, "-x"))
		return(tio(nxtarg(0), 1));

	if(EQ(a, "-d"))
		return(ftype(nxtarg(0),S_IFDIR));

	if(EQ(a, "-c"))
		return(ftype(nxtarg(0),S_IFCHR));

	if(EQ(a, "-b"))
		return(ftype(nxtarg(0),S_IFBLK));

	if(EQ(a, "-L"))
		return(ftype(nxtarg(0),S_IFLNK));

	if(EQ(a, "-f"))
		return(ftype(nxtarg(0),S_IFREG));

	if(EQ(a, "-u"))
		return(ftype(nxtarg(0),S_ISUID));

	if(EQ(a, "-g"))
		return(ftype(nxtarg(0),S_ISGID));

	if(EQ(a, "-s"))
		return(fsizep(nxtarg(0)));

	if(EQ(a, "-t"))
		if(ap>=ac)
			return(isatty(1));
		else
			return(isatty(atoi(nxtarg(0))));

	if(EQ(a, "-n"))
		return(!EQ(nxtarg(0), ""));
	if(EQ(a, "-z"))
		return(EQ(nxtarg(0), ""));

	p2 = nxtarg(1);
	if (p2==0)
		return(!EQ(a,""));
	if(EQ(p2, "="))
		return(EQ(nxtarg(0), a));

	if(EQ(p2, "!="))
		return(!EQ(nxtarg(0), a));

	if(EQ(a, "-l")) {
		int1=length(p2);
		p2=nxtarg(0);
	} else{	int1=atoi(a);
	}
	int2 = atoi(nxtarg(0));
	if(EQ(p2, "-eq"))
		return(int1==int2);
	if(EQ(p2, "-ne"))
		return(int1!=int2);
	if(EQ(p2, "-gt"))
		return(int1>int2);
	if(EQ(p2, "-lt"))
		return(int1<int2);
	if(EQ(p2, "-ge"))
		return(int1>=int2);
	if(EQ(p2, "-le"))
		return(int1<=int2);

	synbad("unknown operator ",p2);
}

tio(a, f)
char *a;
int f;
{

	return access (a, f) >= 0;
}

ftype(f, field)
char *f;
int field;
{
	struct stat statb;

	if(stat(f,&statb)<0)
		return(0);
	if((statb.st_mode&S_IFMT)==field)
		return(1);
	return(0);
}

fsizep(f)
char *f;
{
	struct stat statb;
	if(stat(f,&statb)<0)
		return(0);
	return(statb.st_size>0);
}

synbad(s1,s2)
char *s1, *s2;
{
	write(2, "test: ", 6);
	write(2, s1, strlen(s1));
	write(2, s2, strlen(s2));
	write(2, "\n", 1);
	exit(255);
}

length(s)
	char *s;
{
	char *es=s;
	while(*es++);
	return(es-s-1);
}
