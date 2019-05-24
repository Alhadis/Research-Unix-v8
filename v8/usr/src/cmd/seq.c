#include <stdio.h>

double	min=1.0;
double	max=0.0;
double	incr=1.0;
int	width=0;
int	const=0;
char	*format="%.0f\n";
char	*picture;

extern double atof();
extern char *strchr();

main(argc, argv)
	char *argv[];
{
	register i, j, k, n;
	char buf[BUFSIZ];
	double x;

	while(argc>1 && argv[1][0]=='-'){
		switch(argv[1][1]){
		case 'w':
			const++;
			break;
		case 'p':
			picture= &argv[1][2];
			break;
		default:
			goto out;
		}
		--argc;
		argv++;
	}
    out:
	if(argc<2 || argc>4)
		usage();
	max=atof(argv[argc-1]);
	if(argc>2){
		if(argc>3){
			incr=atof(argv[2]);
			argv[2]=argv[3];
		}
		min=atof(argv[1]);
	}
	if(incr==0){
		fprintf(stderr, "seq: zero increment\n");
		exit(1);
	}
	buildfmt();
	n=(max-min)/incr;
	for(i=0; i<=n; i++){
		x=min+i*incr;
		if(width){
			if(const){
				sprintf(buf, "%.0f", x);
				k=width-strlen(buf);
			}else
				k=width;
			if(x<0){
				putchar('-');
				x= -x;
			}
			for(j=0; j<k; j++)
				putchar('0');
		}
		sprintf(buf, format, x);
		fputs(buf, stdout);
	}
	return 0;
}
usage(){
	fprintf(stderr, "usage: seq [-w] [-p10.2] [first [incr]] last\n");
	exit(1);
}
buildfmt()
{
	static char fmt[10]="%.0f";
	register char *t;
	char buf[32];
	if(const){
		sprintf(buf, fmt, min);
		width=strlen(buf);
		sprintf(buf, fmt, max);
		if(strlen(buf)>width)
			width=strlen(buf);
	}
	if(picture)
		while(picture[0]=='0' && picture[1]!='.'){
			width++;
			picture++;
		}
	if(picture==0)
		return;
	if(picture[0]=='-')
		picture++;
	t=strchr(picture, '.');
	if(t>0){
		t++;
		if(*t==0)
			strcat(fmt, ".");
		else
			sprintf(fmt, "%%.%df", strlen(t));
	}
	strcat(fmt, "\n");
	format=fmt;
}
