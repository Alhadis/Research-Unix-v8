#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dev.h"
#include "pp.h"

/*
 * ALL geometry in device units
 */

#define	BMASK	0377	/* because we can't always say unsigned char */
#define	DEFSIZE	10	/* point size of normal text (only used for vert. motion) */
#define	PAGELENGTH	(11*dev.res)
struct dev	dev;
struct font	font;
char fontdir[]="/usr/lib/font";
char *devname="202";
char *fontname=0;
unsigned char width[BMASK+1];
char ligs[BMASK+1];
char codes[BMASK+1];
char fitab[BMASK+1];
unsigned char special[96];
int miwidth;
int havespecial=0;
int pageno, hpos, vpos;
int margin, vspace;
char curfunc[128];
char idchars[]="_1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
char *curfile;
char filetime[32];
int functionmarked;
typedef struct{
	int variant;			/* ROMAN, ITALIC, BOLD or BLACK */
	short size;			/* point size */
	unsigned char width[96];	/* width tables */
}Ftab;
#define	ROMAN	0
#define	ITALIC	1
#define	BOLD	2
#define	BLACK	3
Ftab ftab[]={
#define	R9	0
	{ROMAN,		9},
#define	R10	1
	{ROMAN,		10},
#define	R14	2
	{ROMAN,		14},
#define	I9	3
	{ITALIC,	9},
#define	I10	4
	{ITALIC,	10},
#define	I14	5
	{ITALIC,	14},
#define	B9	6
	{BOLD,	 	9},
#define	B10	7
	{BOLD,		10},
#define	B14	8
	{BOLD,		14},
	{0,		0}	/* 0 size marks end */
};
Ftab *curfont=ftab;
main(argc, argv)
	char *argv[];
{
	struct stat statbuf;
	long timebuf;
	char *ctime();
	char *title=0;
	--argc; argv++;
	while(argc>0 && argv[0][0]=='-'){
		switch(argv[0][1]){
		default:
			error("usage: pp [-t\"Title\"] [-T202] [-fE] [-k] [files]", (char *)0);
		case 'k':
			ckeywords(&argv[0][2]);
			break;
		case 'f':
			fontname= &argv[0][2];
			break;
		case 'b':
			blacken();
			break;
		case 't':
			title= &argv[0][2];
			break;
		case 'T':
			devname= &argv[0][2];
			break;
		}
		--argc; argv++;
	}
	if(fontname==0){
		fontname="Memphis";
		blacken();
	}
	load();
	if(title){
		time(&timebuf);
		coverpage(title, ctime(&timebuf));
	}
	if(argc<=0){
		curfile="<stdin>";
		time(&timebuf);
		strcpy(filetime, ctime(&timebuf)+4);
		process(0);
	}else while(argc-->0){
		stat(*argv, &statbuf);
		strcpy(filetime, ctime(&statbuf.st_mtime)+4);
		process(Open(curfile= *argv++));
	}
	printf("x trailer\nV0\nx stop\n");
	return 0;
}
blacken(){
	register i;
	for(i=B9; i<=B14; i++)
		ftab[i].variant=BLACK;
}
char *fontexcep[][4]={
/*	roman	italic	bold	black	indices match #defines above */
	"R",	"I",	"B",	"B",	/* Times Roman */
	"PA",	"PI",	"PB",	"PB",	/* Palatino */
	"H",	"HI",	"HB",	"HK",	/* Helvetica */
	"CW",	"CS",	"CS",	"CS",	/* constant width Courier */
	"PO",	"PI",	"PB",	"PX",	/* constant width */
	0
};
char *suffix[]={
	"",	"I",	"B",	"BK"
};
char *
fonttag(variant){
	static char buf[512];
	register i;
	for(i=0;fontexcep[i][0];i++)
		if(strcmp(fontexcep[i][0], fontname)==0)
			return fontexcep[i][variant];
	sprintf(buf, "%s%s", fontname, suffix[variant]);
	return buf;
}
load(){
	register fd, i, j, mi;
	register Ftab *f;
	register char *s;
	char file[64];
	char buf[600];	/* should be enough (gulp) */
	long lseek();
	sprintf(file, "%s/dev%s/DESC.out", fontdir, devname);
	fd=Open(file);
	Read(file, fd, &dev, sizeof dev);
	/* Find \(mi to remember its width */
	if(lseek(fd, (dev.nsizes+1+dev.nchtab)*sizeof(short), 1)==-1L)
		error("device file incomplete", file);
	j=read(fd, buf, sizeof buf);
	for(s=buf,mi=0; j>0 && strcmp(s, "mi")!=0; mi++){
		s+=strlen(s)+1;
		j-=strlen(s)+1;
	}
	if(j<=0)
		error("can't find minus in special font on", devname);
	close(fd);
	printf("x T %s\n", devname);
	printf("x res %d %d %d\n", dev.res, dev.hor, dev.vert);
	printf("x init\n");
	sprintf(file, "%s/dev%s/S.out", fontdir, devname);
	readfont(file);
	printf("x font %d S\n", dev.nfonts);	/* Guess? */
	setwidths(special, dev.unitwidth);
	havespecial=1;
	miwidth=width[fitab[mi+128-32]];
	margin=dev.res/2;		/* 1/2 inch */
	vspace=DEFSIZE*dev.res/72;	/* units per line */
	for(f=ftab; f->size; ){
		sprintf(file, "%s/dev%s/%s.out", fontdir, devname,
			fonttag(f->variant));
		readfont(file);
		printf("x font %d %s\n", (f-ftab)/3+1, fonttag(f->variant));
		for(j=0; j<3; j++, f++)
			setwidths(f->width, f->size);
	}
}
setwidths(wt, size)
	register char *wt;
{
	register i, n;
	for(i=0; i<96; i++, wt++){
		if(n=fitab[i])
			n=width[n];
		else if(havespecial)
			n=special[i];
		*wt=size*n/dev.unitwidth;
	}
}
readfont(file)
	char *file;
{
	register fd;
	fd=Open(file);
	Read(file, fd, &font, sizeof font);
	Read(file, fd, width, font.nwfont&BMASK);
	Read(file, fd, ligs, font.nwfont&BMASK);
	Read(file, fd, codes, font.nwfont&BMASK);
	Read(file, fd, fitab, dev.nchtab+128-32);
	close(fd);
}
coverpage(s, t)
	char *s, *t;
{
	printf("p0\nV%d\n", dev.res*4);	/* 3 inches down */
	center(&ftab[B14], s);
	printf("v%d\n", dev.res);	/* another inch */
	center(&ftab[I9], t);
}
Open(s)
	char *s;
{
	register f=open(s, 0);
	if(f<0)
		error("can't open", s);
	return f;
}
Read(s, f, a, n)
	char *s, *a;
{
	if(read(f, a, n)!=n)
		error("read error on file", s);
}
error(s, t)
	char *s, *t;
{
	fprintf(stderr, "pp: %s %s\n", s, t);
	exit(1);
}
char *
extractfn(s)
	register char *s;
{
	extern char *rindex(), *index();
	register char *t=rindex(s, '('), *u;
	if(t==0)
		error("extract can't find function in", s);
	while(index(idchars, *t)==0)
		if(t<=s)
			return "";
		else
			--t;
	for(u=t; u>=s && index(idchars, *u); --u)
		;
	strncpy(curfunc, u+1, t-u);
	curfunc[t-u]=0;
	return curfunc;
}
/*
 * function name should not be in italics
 */ 
process(fd)
	register fd;
{
	register char *s;
	register unsigned char *w;
	register type;
	register Ftab *f=0, *of;
	register c;
	char buf[32];
	fileno(stdin)=fd;	/* cough */
	pageno=0;
	topofpage();
	curfont= &ftab[R10];
	curfunc[0]=0;
	while(type=yylex()){
		of=f;
		/*
		 * Appropriate font switches, etc.
		 */
		switch(type){
		case FUNCTION:
			rjust(&ftab[I14], extractfn(yytext));
			functionmarked=1;
		case OTHER:
			f= &ftab[R10];
			break;
		case COMMENT:{
			Ftab *oldfont;
			/* gotta do this in place, sigh */
			oldfont = curfont;
			curfont = f= &ftab[I10];
			w=f->width;
			drawstr(f, yytext);
			printf("f2 s10\n");
			for(;;){
				outchar(f, c=yyinput());
				if(c==0)
					break;
				if(c=='*'){
				GotStar:
					outchar(f, c=yyinput());
					if(c=='/')
						break;
					if(c=='*')
						goto GotStar;
				}
				if(c=='\n'){
				GotNewline:
					newline();
					while((c=yyinput())=='\t')
						hpos=tabstop(w);
					if(c==' ')
						printf("H%d\n", hpos+=w['/'-32]);
					else if(c=='\n')
						goto GotNewline;
					else{
						printf("H%d\n", hpos);
						outchar(f, c);
						if(c=='*')
							goto GotStar;
					}
				}
			}
			curfont=f=oldfont;
			printf("f%d s%d\n", (f-ftab)/3+1, f->size);
			continue;
		}
		case KEYWORD:
			f= &ftab[B10];
			break;
		}
		if(functionmarked==0){
			if(curfunc[0]){
				sprintf(buf, "...%s", curfunc);
				rjust(&ftab[I9], buf);
			}
			functionmarked=1;
		}
		if(of!=f){	/* font switch */
			printf("f%d s%d\n", (f-ftab)/3+1, f->size);
			curfont=f;
		}
		w=f->width;
		/*
		 * Draw them.
		 */
		for(s=yytext; *s; s++){
			switch(*s){
			case '\n':
				newline();
				break;
			case '\f':
				vpos = PAGELENGTH;
				newline();
				break;
			case ' ':
				printf("h%d\n", w['n'-32]);
				hpos+=w['n'-32];
				break;
			case '\t':
				hpos=tabstop(w);
				printf("H%d\n", hpos);
				break;
			case '-':
				printf("Cmi h%d\n", miwidth*f->size/dev.unitwidth);
				hpos+=miwidth*f->size/dev.unitwidth;
				break;
			default:
				printf("c%c h%d\n", *s, w[*s-32]);
				hpos+=w[*s-32];
				break;
			}
		}
	}
	bottomofpage();
	close(fd);
}
newline(){
	if((vpos+=vspace)>PAGELENGTH-margin-3*vspace){	/* new page */
		bottomofpage();
		topofpage();
	}else{
		printf("n\n");
		printf("v%d\n", vspace);
	}
	printf("H%d\n", hpos=margin);
}
topofpage(){
	printf("p%d\n", pageno++);
	hpos=margin;
	vpos=margin;
	printf("V%d\n", vpos);
	printf("H%d\n", hpos);
	drawstr(&ftab[B14], curfile);
	rjust(&ftab[B14], curfile);
	printf("v%d\n", 3*vspace);
	vpos+=3*vspace;
	functionmarked=0;
}
bottomofpage(){
	char buf[256];
	printf("H%d\n", margin);
	printf("V%d\n", PAGELENGTH-margin);
	drawstr(&ftab[I9], filetime);
	sprintf(buf, "Page %d of %s", pageno, curfile);
	rjust(&ftab[I9], buf);
}
strwidth(f, s)
	Ftab *f;
	register char *s;
{
	register unsigned char *w=f->width;
	register n=0;
	while(*s){
		if(*s==' ')
			n+=w['n'-32];
		else if(*s>' ')
			n+=w[*s-32];
		s++;
	}
	return n;
}
rjust(f, s)
	register Ftab *f;
	register char *s;
{
	printf("H%d\n", dev.paperwidth-margin-strwidth(f, s));
	drawstr(f, s);
	printf("H%d\n", hpos);
}
center(f, s)
	register Ftab *f;
	register char *s;
{
	printf("H%d\n", (dev.paperwidth-margin-strwidth(f, s))/2);
	drawstr(f, s);
	printf("H%d\n", hpos);
}
drawstr(f, s)
	register Ftab *f;
	register char *s;
{
	register c;
	printf("f%d s%d\n", (f-ftab)/3+1, f->size);
	while(c= *s++)	/* assignment = */
		if(c==' ')
			printf("h%d\n", f->width['n'-32]);
		else
			printf("c%c h%d\n", c, f->width[c-32]);
	printf("f%d s%d\n", (curfont-ftab)/3+1, curfont->size);
}
outchar(f, c)
	register Ftab *f;
	register c;
{
	register w;
	if(c==' ')
		printf("h%d\n", w=f->width['n'-32]);
	else
		printf("c%c h%d\n", c, w=f->width[c-32]);
	hpos+=w;
}
tabstop(w)
	register unsigned char *w;
{
	register c, block;

	block = w['i'-32] == w['m'-32]? (8*w['n'-32]):(dev.res/2);
	c = margin + block*((hpos-margin+block-1)/block);
	if(hpos == c)
		c += block;
	return(c);
}
