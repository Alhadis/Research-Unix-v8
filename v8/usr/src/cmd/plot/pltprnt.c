#include <stdio.h>

float deltx;
float delty;

main(argc,argv)  char **argv; {
	int std=1;
	FILE *fin;

	while(argc-- > 1) {
		if(*argv[1] == '-')
			switch(argv[1][1]) {
				case 'l':
					deltx = atoi(&argv[1][2]) - 1;
					break;
				case 'w':
					delty = atoi(&argv[1][2]) - 1;
					break;
			}
		else	{
			std = 0;
			if ((fin = fopen(argv[1], "r")) == NULL) {
				fprintf(stderr, "can't open %s\n", argv[1]);
				exit(1);
			}
			fplt(fin);
		}
		argv++;
	}
	if (std)
		fplt( stdin );
	exit(0);
}


fplt(fin)  FILE *fin; {
	int c;
	char s[256];
	int xi,yi,x0,y0,x1,y1,r,dx,n,i;
	int pat[256];

	while((c=getc(fin)) != EOF){
		switch(c){
		case 'm':
			xi = getsi(fin);
			yi = getsi(fin);
			printf("%c %6d %6d\n", c,xi,yi);
			break;
		case 'l':
			x0 = getsi(fin);
			y0 = getsi(fin);
			x1 = getsi(fin);
			y1 = getsi(fin);
			printf("%c %6d %6d %6d %6d\n", c,x0,y0,x1,y1);
			break;
		case 't':
			gets(s,fin);
			printf("%c %s\n", c,s);
			break;
		case 'e':
			printf("%c\n",c);
			break;
		case 'p':
			xi = getsi(fin);
			yi = getsi(fin);
			printf("%c %6d %6d\n", c,xi,yi);
			break;
		case 'n':
			xi = getsi(fin);
			yi = getsi(fin);
			printf("%c %6d %6d\n", c,xi,yi);
			break;
		case 's':
			x0 = getsi(fin);
			y0 = getsi(fin);
			x1 = getsi(fin);
			y1 = getsi(fin);
			printf("%c %6d %6d %6d %6d\n", c,x0,y0,x1,y1);
			break;
		case 'a':
			xi = getsi(fin);
			yi = getsi(fin);
			x0 = getsi(fin);
			y0 = getsi(fin);
			x1 = getsi(fin);
			y1 = getsi(fin);
			printf("%c %6d %6d %6d %6d %6d %6d\n", c,xi,yi,x0,y0,x1,y1);
			break;
		case 'c':
			xi = getsi(fin);
			yi = getsi(fin);
			r = getsi(fin);
			printf("%c %6d %6d %6d\n", c,xi,yi,r);
			break;
		case 'f':
			gets(s,fin);
			printf("%c %s\n", c,s);
			break;
		case 'd':
			xi = getsi(fin);
			yi = getsi(fin);
			dx = getsi(fin);
			n = getsi(fin);
			for(i=0; i<n; i++)pat[i] = getsi(fin);
			/* dot(xi,yi,dx,n,pat); */
			printf("%c, %6d, %6d, %6d, %6d ???\n", c,xi,yi,dx,n);
			break;
			}
		}
}

getsi(fin)  FILE *fin; {
	/* get an integer stored in 2 ascii bytes. */
	short a, b;
	if((b = getc(fin)) == EOF)
		return(EOF);
	if((a = getc(fin)) == EOF)
		return(EOF);
	a = a<<8;
	return(a|b);
}

gets(s,fin)  char *s;  FILE *fin; {
	for( ; *s = getc(fin); s++)
		if(*s == '\n')
			break;
	*s = '\0';
	return;
}
