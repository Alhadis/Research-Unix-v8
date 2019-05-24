#include "mugs.h"
#include "fb.h"
#include <sgtty.h>
struct sgttyb tbuf, tsave;
int pic[PSIZE][PSIZE];
int pheight, pwidth, shrnk;
char *mugsterm="/usr/jerq/bin/32ld /usr/jerq/mbin/mugsterm.m";
main(argc, argv)
char *argv[];
{
	register i, j, c, x0, y0, x1, y1, size;
	register t, nfile=0;
	char bbbuf[BUFSIZ];
	setbuf(stdout, bbbuf);
	if(argc>1 && strcmp(argv[1], "-t")==0){	/* undoc. flag for testing */
		--argc;
		argv++;
		mugsterm="/usr/jerq/bin/32ld mugsterm.m";
	}
	if(argc<2){
		fprintf(stderr, "Usage: mugs file ...\n");
		exit(1);
	}
	if(!inface(argv[1])){
		fprintf(stderr, "can't read %s\n", argv[1]);
		exit(1);
	}
	gtty(0, &tbuf);
	tsave=tbuf;
	tbuf.sg_flags|=RAW;
	tbuf.sg_flags&=~ECHO;
	stty(0, &tbuf);
	system(mugsterm);
	while((c=getchar())!=QUIT && c!=EOF) switch(c){
	case WHOLE:
		if(nfile==argc){
			putchar(ARGC);
			break;
		}
		if(nfile==0)
			nfile=1;
		else if(!inface(argv[nfile])){
			putchar(NGFILE);
			break;
		}
		sendwhole();
		nfile++;
		break;
	case SQUASH:
		x0=getint();
		y0=getint();
		x1=getint();
		y1=getint();
		size=max(x1-x0, y1-y0);
		x0=(x0+x1-size)/2;
		y0=(y0+y1-size)/2;
		sendsquash(x0, y0, x0+size, y0+size);
		break;
	case SAVEFACE:
		saveface();
		break;
	case QUIT:
		exit(0);
	}
	stty(0, &tsave);
}
inface(f)
char *f;
{
	register unsigned char *vp;
	unsigned char v[4096*8];
	register PICFILE *p;
	register x, y, lum;
	if((p=openpicr(f))==NULL)
		return(0);
	pheight=p->r.co.x-p->r.or.x;
	pwidth=p->r.co.y-p->r.or.y;
	shrnk=max(1, max(pheight/PSIZE, pwidth/PSIZE));
	for(y=0;y!=pheight;y++){
		readpic(p, v);
		for(x=0,vp=v;x!=pwidth;x++,vp+=p->nchan)
			pic[y/shrnk][x/shrnk]+=p->nchan<3?*vp:(vp[0]+vp[1]+vp[2])/3;
	}
	integrate();
	closepic(p);
	return(1);
}
int min(a, b){
	return(a<b?a:b);
}
int max(a, b){
	return(a>b?a:b);
}
sendwhole(){
	register v, h, e, b;
	int floyd[2][PSIZE+1];
	putchar(WHOLE);
	putint(PSIZE);
	putint(PSIZE);
	for(h=0;h!=PSIZE;h++)
		floyd[0][h]=0;
	for(v=0;v!=PSIZE;v++){
		for(h=0;h!=PSIZE;h++)
			floyd[(v+1)&1][h]=0;
		for(h=0;h!=PSIZE;h++){
			if((h&31)==0)
				b=0;
			e=floyd[v&1][h]+sample(h, v, h+1, v+1);
			if(e>RES/2)
				e-=RES;
			else
				b|=1<<(31-(h&31));
			floyd[(v+1)&1][h]+=e*3/8;
			floyd[(v+1)&1][h+1]+=e/4;
			floyd[v&1][h+1]+=e*3/8;
			if((h&31)==31)
				putint(b);
		}
		if((h&31)!=0)
			putint(b);
	}
	fflush(stdout);
}
sendsquash(x0, y0, x1, y1){
	register x, y;
	if(x0<0) x0=0; else if(PSIZE<=x0) x0=PSIZE-1;
	if(y0<0) y0=0; else if(PSIZE<=y0) y0=PSIZE-1;
	if(x1<=x0) x1=x0+1; else if(PSIZE<x1) x1=PSIZE;
	if(y1<=y0) y1=y0+1; else if(PSIZE<y1) y1=PSIZE;
	x1-=x0;
	y1-=y0;
	putchar(SQUASH);
	putint(FSIZE);
	putint(FSIZE);
	for(y=0;y!=FSIZE;y++) for(x=0;x!=FSIZE;x++)
		putchar(sample(x0+x1*x/FSIZE, y0+y1*y/FSIZE,
			x0+((x1+1)*x-1)/FSIZE, y0+((y1+1)*y-1)/FSIZE));
	fflush(stdout);
}
saveface(){
	char name[512];
	register char *s;
	register FILE *f;
	register i, v, w;
	s=name;
	while((*s=getchar())!='\n')
		s++;
	*s='\0';
	if((f=fopen(name, "w"))==NULL){
		putchar(BAD);
		fflush(stdout);
		return;
	}
	putchar(OK);
	fflush(stdout);
	getint();		/* width */
	getint();		/* height */
	for(i=0;i!=48;i++){
		v=getint();
		w=getint();
		fprintf(f, "0x%04x, 0x%04x, 0x%04x,\n", (v>>16)&0xFFFF,
			v&0xFFFF, (w>>16)&0xFFFF);
	}
	fclose(f);
}
getint(){
	register i;
	i=getchar();
	i=(i<<8)+getchar();
	i=(i<<8)+getchar();
	return (i<<8)+getchar();
}
putint(i)
register i;
{
	putchar((i>>24)&0xFF);
	putchar((i>>16)&0xFF);
	putchar((i>>8)&0xFF);
	putchar(i&0xFF);
}
integrate(){
	register i, j, sum;
	for(i=0;i!=PSIZE;i++) for(j=0;j!=PSIZE;j++)
		pic[i][j]/=shrnk*shrnk;
	for(j=1;j!=PSIZE;j++)
		pic[0][j]+=pic[0][j-1];
	for(i=1;i!=PSIZE;i++){
		sum=0;
		for(j=0;j!=PSIZE;j++){
			sum+=pic[i][j];
			pic[i][j]=pic[i-1][j]+sum;
		}
	}
}
sample(x0, y0, x1, y1){
	register v;
	if(x1==x0)
		x1++;
	if(y1==y0)
		y1++;
	if(x0==0){
		if(y0==0)
			v=pic[y1-1][x1-1];
		else
			v=pic[y1-1][x1-1]-pic[y0-1][x1-1];
	}
	else if(y0==0)
		v=pic[y1-1][x1-1]-pic[y1-1][x0-1];
	else
		v=pic[y1-1][x1-1]-pic[y0-1][x1-1]-pic[y1-1][x0-1]+pic[y0-1][x0-1];
	return(v/(y1-y0)/(x1-x0));
}
