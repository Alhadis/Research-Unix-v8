#include "alph.h"

main() {

	cursinhibit();
	param = 0;
	hitkey = false;
	c = '?';

	jinit();
	request(KBD);
	BonW();

	for(;;){
		switch (c) {
		case 'a':
			ripple(); 
			continue;
		case 'b':
			bounce1(); 
			continue;
		case 'c':
			circle1(); 
			continue;
		case 'd':
			lineslide(); 
			continue;
		case 'e':
			munch(); 
			continue;
		case 'f':
			tetra(); 
			continue;
		case 'g':
			localop(); 
			continue;
		case 'h':
			circle2(); 
			continue;
		case 'i':
			bounce2(); 
			continue;
		case 'j':
			fractal(); 
			continue;
		case 'k':
			bounce3(); 
			continue;
		case 'l':
			spheres(); 
			continue;
		case 'm':
			grandmal(); 
			continue;
		case 'n':
			wallpaper(); 
			continue;
		case 'o':
			schwartz(); 
			continue;
		case 'p':
			discs();
			continue;
		case 'q':
		{
			register int (*f)();
			f= *(int(**)())(256*1024+4);
			spl7();
			(*f)();
		}
		case 'u':
			worms(); 
			continue;
		case 'v':
			nkal(); 
			continue;
/*		case 'w':
			rotor(); 
			continue;
		case 'x':
			ncyl(); 
			continue;
		case 'y':
			burst(); 
			continue;
		case 'z':
			frect(); 
			continue;
*/
		default: 
			help(); 
			continue;
		}
	}
}
	
	
#define P       98
#define Q       27
#define Pm1     (P-1)
	
int I=Pm1, J=(Pm1+Q)%P;
int Table[P]={
	0020651, 0147643, 0164707, 0125262, 0104256, 0074760, 0114470,
	0052607, 0045551, 0134031, 0024107, 0030766, 0154073, 0114777,
	0024540, 0111012, 0011042, 0104067, 0056332, 0142244, 0131107,
	0034074, 0052641, 0163046, 0026303, 0131352, 0077724, 0002462,
	0110775, 0127346, 0020100, 0137011, 0136163, 0145552, 0144223,
	0134111, 0075001, 0075221, 0176705, 0000210, 0103625, 0120246,
	0062614, 0016147, 0054723, 0151200, 0105223, 0021001, 0016224,
	0073377, 0150716, 0014557, 0112613, 0037466, 0002677, 0052542,
	0063572, 0105462, 0106436, 0063302, 0053171, 0133243, 0113130,
	0123222, 0072371, 0041043, 0163614, 0037432, 0147330, 0153403,
	0130306, 0056455, 0175640, 0120567, 0100601, 0042371, 0154635,
	0051133, 0074252, 0174525, 0163223, 0052022, 0022564, 0135512,
	0021760, 0006743, 0006451, 0067445, 0106210, 0025417, 0066566,
	0062723, 0124224, 0144643, 0164502, 0025342, 0003521, 0024050,
};

int random() {
	if (I == Pm1) I = 0; 
	else I++;
	if (J == Pm1) J = 0; 
	else J++;
	return (Table[I]^=Table[J]);
}

#include "queue.h"
int keyhit(){
	if(KBDQUEUE.c_cc<=0)
		return false;
	c=qgetc(&KBDQUEUE);
	if (('0'<=c) && (c<='9')) {
		if (hitkey) param=0;
		param=param*10+(c-'0');
		hitkey=false;
	} else
		hitkey=true;
	return hitkey;
}

int r (i, j)
int i, j;
{
	int k;

	k = random() % (j-i+1);
	if (k<0) return(k+j+1);
	else return(k+i);
}


clearscreen() {
	rectf(&display, Drect, F_CLR);
}

sqroot(a)
	register a;
{
	register x, y;
	if(a<=0)
		return 0;
	for(y=a,x=1;y!=0;y>>=2,x<<=1)
		;
	while((y=(a/x+x)>>1)<x)
		x=y;
	return x;
}

help(){
	clearscreen();
	startprintf();
	printf("\n[[[[ Jerqstation graphics demo ]]]]");
	printf("\n");
	printf ("\n        Tom Duff, Dan Silva and John Seamons -- Lucasfilm, Ltd.");
	printf ("\nType:");
	printf ("\n    a for rippling rectangles");
	printf ("\n    b for sliding squares");
	printf ("\n    c for xor circle");
	printf ("\n    d for sliding lines");
	printf ("\n    e for munching squares");
	printf ("\n    f for tetra");
	printf ("\n    g for localop");
	printf ("\n    h for random circles");
	printf ("\n    i for bouncing square");
	printf ("\n    j for fractal contour maps");
	printf ("\n    k for colliding balls");
	printf ("\n    l for shaded spheres");
	printf ("\n    m for grand mal");
	printf ("\n    n for wallpaper");
	printf ("\n    o for a Lillian Schwartz movie");
	printf ("\n    p for random discs");
	printf ("\n    q for quit");
	printf ("\n    r for no effect");
	printf ("\n    s for no effect");
	printf ("\n    t for no effect");
	printf ("\n    u for worms");
	printf ("\n    v for kaleidoscope");
	while (!keyhit());
}

