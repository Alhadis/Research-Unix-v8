#include "maze.h"

char obuf[40];

getchar()
{
	register char c;
	while ((c = rcvchar()) == -1)
		wait(RCV);
	return(c&0377);
}

getnum()
{
	return(getchar()-' ');
}

State *
getplayer()
{
	register State *p=player+getnum();
	if(p >= &player[N]){
		mess((char *)0);
		mess("bogus player");
		return 0;
	}
	return p;
}

inflush(n)
register n;
{
	do
		getchar();
	while (--n > 0);
}

outchar(c)
{
	sendchar(c);
}

quit()
{
	register char *p = obuf;
	*p++ = 'q';
	*p++ = ME + ' ';
	*p++ = 4;
	sendnchars(3,obuf);
	sleep(30);
	sendchar(0177);
}

myname()
{
	register char *p = obuf, *s = me->name;
	int n = 4;
	*p++ = 'n';
	*p++ = ME + ' ';
	*p++ = me->score + 64;
	while (*p++ = *s++)
		n++;
	sendnchars(n,obuf);
}

killme(n)		/* n got me */
register n;
{
	register char *p = obuf;
	*p++ = 'k';
	*p++ = n + ' ';
	*p++ = ME + ' ';
	sendnchars(3,obuf);
}

fire()
{
	register char *p = obuf;
	*p++ = 'f';
	*p++ = ME + ' ';
	sendnchars(2,obuf);
}

extern char maze[HT][WID];
move()
{
	register char *p = obuf;
	register x,y;
	*p++ = 'm';
	*p++ = ME + ' ';
	*p++ = me->dir + ' ';
	x = (me->pos - maze[0])%WID;
	*p++ = x + ' ';
	y = (me->pos - maze[0])/WID;
	*p++ = y + ' ';
	sendnchars(5,obuf);
}

