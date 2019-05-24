/*----------------------------------------------------------------------*/
/*									*/
/*	PACMAN for BBN BitGraphs					*/
/*									*/
/*	          File: main.c68					*/
/*	      Contents:	controlling and utility routines		*/
/*	        Author: Bob Brown (rlb)					*/
/*			Purdue CS					*/
/*		  Date: May, 1982					*/
/*	   Description: routines for setting up and controlling the	*/
/*			game, plus miscellaneous utilities.		*/
/*									*/
/*----------------------------------------------------------------------*/

#include "style.h"
#include "pacman.h"
#include "host.h"

/*
** Instances of global variables not defined elsewhere
*/
char	*Board[];
bool	Abort;
int	Dotsrem;
int	Game;
long	Highscore[MAXGAMES];
int	Monbase;		/* Base scheduler delay for monsters */
bool	Monseeking;		/* TRUE=>they know where he is */
int	Monvalue;		/* value of captured monster */
bool	Newgame;
int	Pacmen;			/* Number of Pacman remaining */
bool	Running;
long	Score;
long	Nextbonus;
bool	Silent;
int	Timedelay;
int	Wave;
int	clock;
int	Freeze;			/* true if game frozen */

/*
**  Game parameters
*/
int	Seekprob;
int	Seekinc;

/*
** main - game logic control.
**
**  contortions are because there's no exit().
*/

main()
{
	init();
	Running = TRUE;
	Freeze = FALSE;
	retxy(1);
	/*
	** For each game played...
	*/
	while ( Running ) {
		/*
		** For each new wave...
		*/
		newgame();
		while ( Pacmen > 0 && Running ) {
			newwave();
			/*
			** For each game scan...
			*/
			while ( Dotsrem > 0 ) {
				/*
				** If the Pacman died, start a new one.
				*/
				if ( !Pacman.alive ) {
					addmen(-1);
					if ( Pacmen <= 0 )
						break;
					elinit();
					pacinit();
					moninit();
					fruitinit(FALSE);
				}
				if (own()&ALARM) {
					clock++;
					alarm(1);
				}
				poll();
				if ( Abort || Newgame )
					break;
				if(!Freeze)
					elpoll();
			}
			soundoff();
			if ( Abort || Newgame )
				break;
			paccomp();
			moncomp();
			fruitcomp();
			soundoff();
		}
		alarm(300);
		request(KBD|ALARM);
		while (kbdchar()!=-1);
		wait(ALARM|KBD);
		alarm(1);
		request(KBD|MOUSE|ALARM);
		if (Game != 0) {
			sndhost ("%s%s%d\n%s%d %d\n", SYNCSTRING, 
            		       GAMESTRING,  Game, 
	        	       SCORESTRING, encrypt (Score),
			       encrypt (KEY));
			if ( Score > Highscore[Game-1] )
				Highscore[Game-1] = Score;
		}
		if ( Abort )
			break;
	}
	wrapup();
}

/*
** init - set up the terminal
*/

init()
{
	extern int argc, arg0;
	int i, *argp;

#if V1_25 | V1_76
	extern char screenmode;
	do_push();
	screenmode = 0;
	setcontext();
#endif
	/* For non-iconic mpx:
	jinit();
	*/
#if BLIT
	textmode = F_STORE;
#endif

#ifdef V2_0
	extern char screenm;
/*
	dopush();
*/
	screenm = 0;
	setcontext();
#endif
	Abort = FALSE;
	/*
	** High score processing - arguments are the high scores for the
	** games
	*/
/*		Ghod only knows what to do with with these for the Blit

	getscores ();
	argp = &arg0;
	for ( i=0 ; i<MAXGAMES ; i++ )
		if ( i < argc )
			Highscore[i] = *argp++;
		else
			Highscore[i] = NOSCORE;
*/
}

/*
** getscores - get the high score vector from the host.
**
**   Scores are transmitted as ESC : arg0;arg1;...CAN
**   where CAN is the ANSI standard cancel sequence.
**   The cancel ensures the BitGraph will do no processing
**   if the downloaded program doesn't read the arg vector.
**   This routine times out in two seconds in case the host
**   doesn't send the scores.
*/

getscores ()
{
#ifndef BLIT
	register int  c;
	register int  i;
	extern   long  argc;
	extern   long  arg0;
	extern   long  arg1;

	argc = arg0 = arg1 = 0;
	for (i = 0; i < 1000; i++)
	{
	    sleep (2);              /* Wait for max of 2 msec.  */
	    if ((c = hstgetc ()) == '\033')
	    {
		getchar ();         /* Throw away the colon.    */
#if V1_25 | V1_76                                                   
		do_args  ();        /* Old style read arg list. */
#else
		doargs  ();         /* Read the arg list.       */
#endif
		break;
	    }
	}
#endif
}
/*
** settextop - set text operation mode
*/

settextop(i)
{
#ifndef BLIT
	extern long argc, arg0;
	argc = 1;
	arg0 = i;
	setop();
#endif
}
wrapup()
{
	sndhost ("%s", EXITSTRING);
#ifdef V1_25
	do_pop();
#endif

#ifdef V1_76
	do_pop();
#endif

#ifdef V2_0
	dopop();
#endif
}

/*
** newgame - set up for a new game, with instructions.
*/

newgame()
{
	instruct();
	Game = 0;
	Silent = TRUE;
#if BLIT
	request(KBD|ALARM);
	wait(KBD);
	request(KBD|MOUSE|ALARM);
#else
	while(kbdgetc()!=-1) ;
#endif
	while (Game==0 && Running) poll();
	if ( !Running )
		return;
	clear();

	switch ( Game ) {
	case 1:
		Seekprob = 40;
		Seekinc = 10;
		Monbase = 40;
		Pacman.time = 43;
		break;
	case 2:
		Seekprob = 80;
		Seekinc = 2;
		Monbase = 30;
		Pacman.time = 35;
		break;
	case 3:
		Seekprob = 100;
		Seekinc = 0;
		Monbase = MINMONTIME;
		Pacman.time = MINPACTIME;
		break;
	case 4:				/* Flat out, for testing and practice */
		Seekprob = 100;
		Seekinc = 0;
		Monbase = 0;
		Pacman.time = 0;
	}
	Newgame = FALSE;
	Pacman.alive = TRUE;
	Wave = 0;
	Score = 0;
	Nextbonus = (long) SCOREMOD;
	mvprintf(SCOREROW,1,"S C O R E : ");
	addscore(0);
	if ( Highscore[Game-1] != NOSCORE )
		mvprintf(SCOREROW,HIGHCOL,"H I G H   S C O R E : %l",Highscore[Game-1]);
	Pacmen = 1;
	mvprintf(WAVEROW,WAVECOL,"W A V E : ");
	addmen(PACMEN-1);
	pacnew();
	monnew();
}
/*
** Display Game instructions
*/
instruct()
{
	clear();
	/*cursinhibit();*/
	mvprintf(11,1,"P A C M A N");
	mvprintf(13,10,"<h> or <keypad-4>               to move left.");
	mvprintf(14,10,"<j> or <keypad-2> or <keypad-5> to move down.");
	mvprintf(15,10,"<k> or <keypad-8>               to move up.");
	mvprintf(16,10,"<l> or <keypad-6>               to move right.");
	mvprintf(17,10,"< > or <keypad-0>               to stop.");

	mvprintf(19,10,"<q> to quit.");
	mvprintf(20,10,"<n> to start a new game.");
	mvprintf(21,10,"<s> to toggle sound.");
	mvprintf(23,10,"Select game 1, 2, 3, or 4.");
	/*cursallow();*/
}
/*
** start a new wave - also starts the first wave.
**
**  increase the speed on the monsters, but not too fast.
**  However, if already running faster than fastest, leave it there.
*/

newwave()
{
	register monster *mptr;
	Wave++;
	mvprintf(WAVEROW,WAVECOL+10,"%d",Wave);
	if ( Wave&1 ) {
		for ( mptr=Monster ; mptr < &Monster[MAXMONSTER] ; mptr++ )
			if ( mptr->time > MINMONTIME )
				mptr->time = max(mptr->time-MONDELTA,MINMONTIME);
		if ( Pacman.time > MINPACTIME )
			Pacman.time = max(Pacman.time-PACDELTA,MINPACTIME);
	}
	newboard();
	drawboard();
	Dotsrem = DOTS;
	Seekprob += Seekinc;
	elinit();
	pacinit();
	moninit();
	fruitinit(TRUE);
}
/*
** Keyboard polling routine
*/
poll()
{
	register unsigned char c;
	register int o;

#if BLIT
/*
	if (!(own()&KBD))
		wait(CPU);
	else
	switch ( (c=kbdchar()) ) {
*/
	if(button123()){
		request(0);
		wait(CPU);
		request(ALARM|MOUSE|KBD);
	}
	if((o = own()) & KBD)
		c = kbdchar();
	else if(!Freeze && (o & MOUSE))
		c = retxy(0);
	else
		c = 0;
	if(c == 0)
		wait(CPU);
	else
	switch(c) {
#else
	rsetdead();
	switch ( (c=kbdgetc()) ) {
#endif
	case GAME4:
	case GAME3:
	case GAME2:
	case GAME1:
		if ( Game ) return;
		Game = c - '0';
		break;
	case LEFT:
	case ALTLEFT:
		Pacman.pending = MOVELEFT;
		Pacman.pendcnt = PENDCOUNT;
		Pacman.moving = TRUE;
		break;
	case RIGHT:
	case ALTRIGHT:
		Pacman.pending = MOVERIGHT;
		Pacman.pendcnt = PENDCOUNT;
		Pacman.moving = TRUE;
		break;
	case UP:
	case ALTUP:
		Pacman.pending = MOVEUP;
		Pacman.pendcnt = PENDCOUNT;
		Pacman.moving = TRUE;
		break;
	case DOWN:
	case ALTDOWN:
	case ALTALTDOWN:
		Pacman.pending = MOVEDOWN;
		Pacman.pendcnt = PENDCOUNT;
		Pacman.moving = TRUE;
		break;
	case SILENT:
		Silent = !Silent;
		break;
	case STOP:
	case ALTSTOP:
		Pacman.moving = FALSE;
		Pacman.pendcnt = 0;
		break;
	case ABORT:
		Running = FALSE;
		Abort = TRUE;
		break;
	case NEWGAME:
		Newgame = TRUE;
		break;
	case FREEZE:
		Freeze = ! Freeze;
		break;
	case -1:
		wait(CPU);
		break;
	}
}

/*
** Update the score
*/
addscore(inc)
register int inc;
{
	long oldscore;
	oldscore = Score;
	Score += inc;
	/*cursinhibit();*/
	mvprintf(SCOREROW,SCORECOL,"%l",Score);
	/*cursallow();*/
	if ( oldscore<Nextbonus &&  Score>=Nextbonus ) {
		addmen(1);
		Nextbonus *= 2;
	}
}
/*
** Add multiple men or subtract just one.
*/
addmen(inc)
register int inc;
{
	register int i;
	/*
	** Put another at the bottom of the screen
	*/
	if ( inc > 0 ) {
		for ( i=0 ; i<inc ; i++ ) {
			rempacface();
			Pacmen++;
		}
	} else {
		Pacmen--;
		rempacface();
	}
}
mvprintf(row,col,f,p0,p1,p2,p3)
char *f;
int p0,p1,p2,p3;
{
#ifdef V1_25
	extern long arg0, arg1;
	arg0 = row;
	arg1 = col;
	movexy();
#endif
#ifdef V1_76
	cmovey (row);
	cmovex (col);
#endif
#ifdef V2_0
	cmovey (row);
	cmovex (col);
#endif
#ifdef BLIT
	movexy(col,row);
#endif
	printf(f,p0,p1,p2,p3);
}
#define PRIME 2131
grand(lb,ub)
int lb, ub;
{
	return (rand() % PRIME) * (ub-lb+1) / PRIME + lb;
}
long	randx = 1;

srand(x)
unsigned x;
{
	randx = x;
}

rand()
{
	return(((randx = randx*1103515245 + 12345)>>16) & 077777);
}

pause(ms)
{
	soundoff();
#ifdef BLIT
	sleep((ms+8)/16);
#else
	sleep(ms);
#endif
	eladjust(ms);
}

#ifdef BLIT
clear()
{
	/*cursinhibit();*/
	rectf(&display,Rect(0,0,XMAX,YMAX),F_CLR);
	/*cursallow();*/
}
#endif

#ifdef BLIT
retxy(init)
{
	register ret;
	Point p, dif;
	static Point centre;
#define		FUZZ	50

	if(init)
	{
		centre = div(add(Drect.origin, Drect.corner), 2);
		cursset(centre);
		return;
	}
	p = mouse.xy;
	dif = sub(p, centre);
	ret = 0;
	if(abs(dif.y) > FUZZ) {
		ret = (dif.y<0)? UP : DOWN;
		cursset(Pt(p.x, centre.y));
	} else if(abs(dif.x) > FUZZ) {
		ret = (dif.x>0)? RIGHT : LEFT;
		cursset(Pt(centre.x, p.y));
	}
	return(ret);
}
#endif
