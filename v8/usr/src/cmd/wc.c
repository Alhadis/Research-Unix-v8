#include	<stdio.h>

#define	NL	0
#define	SP	1
#define	ORD	2
#define	JUNK	3
#define	TOKEN	4

char type[]={
     /* 000 nul|001 soh|002 stx|003 etx|004 eot|005 enq|006 ack|007 bel| */
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
     /* 010 bs |011 ht |012 nl |013 vt |014 np |015 cr |016 so |017 si | */
	JUNK,	SP,	NL,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
     /* 020 dle|021 dc1|022 dc2|023 dc3|024 dc4|025 nak|026 syn|027 etb| */
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
     /* 030 can|031 em |032 sub|033 esc|034 fs |035 gs |036 rs |037 us | */
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
     /* 040 sp |041  ! |042  " |043  # |044  $ |045  % |046  & |047  ' | */
	SP,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,
     /* 050  ( |051  ) |052  * |053  + |054  , |055  - |056  . |057  / | */
	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,
     /* 060  0 |061  1 |062  2 |063  3 |064  4 |065  5 |066  6 |067  7 | */
	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,
     /* 070  8 |071  9 |072  : |073  ; |074  < |075  = |076  > |077  ? | */
	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,
     /* 100  @ |101  A |102  B |103  C |104  D |105  E |106  F |107  G | */
	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,
     /* 110  H |111  I |112  J |113  K |114  L |115  M |116  N |117  O | */
	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,
     /* 120  P |121  Q |122  R |123  S |124  T |125  U |126  V |127  W | */
	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,
     /* 130  X |131  Y |132  Z |133  [ |134  \ |135  ] |136  ^ |137  _ | */
	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,
     /* 140  ` |141  a |142  b |143  c |144  d |145  e |146  f |147  g | */
	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,
     /* 150  h |151  i |152  j |153  k |154  l |155  m |156  n |157  o | */
	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,
     /* 160  p |161  q |162  r |163  s |164  t |165  u |166  v |167  w | */
	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,
     /* 170  x |171  y |172  z |173  { |174  | |175  } |176  ~ |177 del| */
	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	ORD,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,	JUNK,
};
char	*opt="lwc";
long	twords;
long	tlines;
long	tchars;
main(argc, argv)
	char *argv[];
{
	register i, fd, status=0;
	if(argc>1 && argv[1][0]=='-'){
		opt=++argv[1];
		--argc, argv++;
	}
	if(argc==1)
		count(0, (char *)0);
	else for(i=1; i<argc; i++){
		fd=open(argv[i], 0);
		if(fd<0){
			fprintf(stderr, "wc: ");
			perror(argv[i]);
			status=1;
			continue;
		}
		count(fd, argv[i]);	
	}
	if(argc>2)
		print(tchars, twords, tlines, "total");
	return status;
}
unsigned char buf[BUFSIZ];
count(fd, name)
	char *name;
{
	register token=0, n;
	register unsigned char *cp;
	register long chars=0, lines=0, words=0;
	while((n=read(fd, buf, sizeof buf))>0){
		chars+=n;
		cp=buf;
		while(--n>=0)
			switch(type[*cp++]|token){
			case NL:
				lines++;
				break;
			case NL|TOKEN:
				lines++;
				token=0;
				break;
			case SP:
				break;
			case SP|TOKEN:
				token=0;
				break;
			case ORD:
				token=TOKEN;
				words++;
				break;
			case ORD|TOKEN:
				break;
			case JUNK:
			case JUNK|TOKEN:
				break;
			}
	}
	close(fd);
	print(chars, words, lines, name);
	tchars+=chars;
	twords+=words;
	tlines+=lines;
}
print(charct, wordct, linect, name)
	long charct, wordct, linect;
	char *name;
{
	register char *wd=opt;
	while (*wd) switch (*wd++) {
	case 'l':
		printf("%7ld ", linect);
		break;

	case 'w':
		printf("%7ld ", wordct);
		break;

	case 'c':
		printf("%7ld", charct);
		break;
	}
	if(name)
		printf(" %s\n", name);
	else
		printf("\n");
}

