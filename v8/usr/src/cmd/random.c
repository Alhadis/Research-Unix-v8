#include <stdio.h>
float fract = 2;
double	atof(), frand();
long time();
int eflag;
char line[BUFSIZ];
main(argc,argv) char **argv;
{
	int i,c;
	for(i=1;i<argc;i++)
	{	if(*argv[i]!='-')
			fract=atof(argv[i]);
		else if((c=argv[i][1])=='e')
			eflag=1;
	}
	srand((int)time(0));
	if(eflag==0)
		while(gets(line)) {
			if(frand()*fract<1)
				puts(line);
		}
	else
		return(nrand((int)fract));
	return(0);
}

