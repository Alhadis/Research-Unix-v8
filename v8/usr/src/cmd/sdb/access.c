static	char sccsid[] = "@(#)access.c 4.1 10/9/80";
#
/*
 *
 *	UNIX debugger
 *
 */

#include "head.h"
struct user u;


MSG		BADDAT;
MSG		BADTXT;
MAP		txtmap;
MAP		datmap;
STRING		errflg;
int		errno;

INT		pid;




/* file handling and access routines */

int dmask[5] = {0, 0xff, 0xffff, 0xffffff, 0xffffffff};

/* get data at loc using descriptor format d */
long
getval(loc, d, space)
ADDR loc;
char d; {
	register int val;
	
	val = get(loc, space);
	val &= dmask[dtol(d)];
	return(val);
}

/* put value at loc using descriptor format d */
putval(loc, d, value)
ADDR loc; char d; long value; {
	register long val;
	
	val = get(loc, DSP);
	val = (val & !dmask[dtol(d)]) | (value & dmask[dtol(d)]);
	put(loc, DSP, val);
}

/* put value in named register using descriptor format d */
putreg(reg, d, value)
ADDR reg; char d; long value; {
	register long val;
	
	val = *(ADDR *)(((ADDR)&u)+R0+WORDSIZE*reg);
	val = (val & !dmask[dtol(d)]) | (value & dmask[dtol(d)]);
	*(ADDR *)(((ADDR)&u)+R0+WORDSIZE*reg) = val;
}

put(adr,space,value)
L_INT	adr;
{
	access(WT,adr,space,value);
}

POS	get(adr, space)
L_INT		adr;
{
	return(access(RD,adr,space,0));
}


access(mode,adr,space,value)
L_INT	adr;
{
	INT	pmode,rd,file;
	ADDR	w;
	if (debug) 
		printf("access(mode=%d,adr=%d,space=%d,value=%d) with pid %d\n",
			mode, adr, space, value, pid);
	rd = mode==RD;

	IF space == NSP THEN return(0); FI
	w = 0;
	IF pid
	THEN file = datmap.ufd;	/* always look in proc, no mapping */
	ELSE IF !chkmap(&adr,space) THEN return(0); FI
	     file=(space&DSP?datmap.ufd:txtmap.ufd);
	FI
	if (longseek(file,adr)==0 ||
	   (rd ? read(file,&w,sizeof(w)) : write(file,&value,sizeof(w))) < 1)
		errflg=(space&DSP?BADDAT:BADTXT);
	return(w);

}

chkmap(adr,space)
	REG L_INT	*adr;
	REG INT		space;
{
	REG MAPPTR amap;
	amap=((space&DSP?&datmap:&txtmap));
	IF space&STAR ORF !within(*adr,amap->b1,amap->e1)
	THEN if (within(*adr,amap->b2,amap->e2))
		*adr += (amap->f2)-(amap->b2);
	     else {
		errflg=(space&DSP?BADDAT:BADTXT); return(0);
	     }
	ELSE *adr += (amap->f1)-(amap->b1);
	FI
	return(1);
}

within(adr,lbd,ubd)
POS	adr, lbd, ubd;
{
	return(adr>=lbd && adr<ubd);
}

/* ------------ */
POS	chkget(n, space)
L_INT		n;
{
#ifndef vax
	REG INT		w;
#else
	REG L_INT	w;
#endif

	w = get(n, space);
	chkerr();
	return(w);
}

POS bchkget(n, space) 
L_INT	n;
{
	return(chkget(n, space) & LOBYTE);
}
