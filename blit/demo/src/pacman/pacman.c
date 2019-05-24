/*----------------------------------------------------------------------*/
/*									*/
/*	PACMAN for BBN BitGraphs					*/
/*									*/
/*	          File: pacman.c68					*/
/*	      Contents:	Pacman control routines				*/
/*	        Author: Bob Brown (rlb)					*/
/*			Purdue CS					*/
/*		  Date: May, 1982					*/
/*	   Description: routines for initialization, movement, and	*/
/*			drawing.					*/
/*									*/
/*----------------------------------------------------------------------*/

#include "style.h"
#include "pacman.h"

/*
**  Initialize the pacman
**
**	pacnew - called a beginning of game.
**	pacinit - called at each board restart.
*/

pacnew()
{
}
pacinit()
{
	Pacman.dir = PACFACE;
	Pacman.moving = FALSE;
	Pacman.mouth = 0;
	Pacman.font = FACECHAR;
	Pacman.row = MZtoSC(PACROW);
	Pacman.col = MZtoSC(PACCOL)-1;
	Pacman.power = 0;
	Pacman.alive = TRUE;
	Pacman.adjusttime = 0;
	paccomp();
	eladd(Pacman.time+STARTDELAY,pacmove);
}
/*
** pacmove - move the pacman based on the description in
**	     global Pacman - called by event-list manager.
**
**	mouthfont is indexed by the direction the pacman is facing and
**	gives the font character to use for each mouth-step
*/

char *mouthfont[] = { "abcdcb", "aefgfe", "ahijih", "aklmlk"};

int
pacmove()
{

	paccomp();
	if ( Pacman.moving ) {
		/*
		** Check if next move is feasible
		*/
		if ( Pacman.pendcnt > 0 ) {
			Pacman.pendcnt--;
			if ( trymove(Pacman.pending) )
				Pacman.pendcnt = 0;
			else
				trymove(Pacman.dir);
		} else
			trymove(Pacman.dir);
	}
	/*
	** Now have a (potentially new) position
	*/
	if ( ++Pacman.mouth >= 6 ) Pacman.mouth = 0;
	Pacman.font = mouthfont[Pacman.dir-1][Pacman.mouth];
	paccomp();
	/*
	**  The routines that are called below rely on the maze coordinates
	**  having already been computed.
	*/
	Pacman.mrow = SCtoMZ(Pacman.row+2);
	Pacman.mcol = SCtoMZ(Pacman.col+2);
	/*
	** Check for gobbling a dot
	*/
	if ( hasdot() ) {
		Pacman.adjusttime = DOTDELAY;
		Dotsrem--;
		sounddot();
		addscore(VALGOLD);
	} else
		Pacman.adjusttime = 0;
	/*
	** Check for swallowing a power pill, fruit, and in safe zone
	*/
	if ( haspill() )
		pacpower(NULL,ON);
	if ( hasfruit() )
		hitfruit();
	Pacman.safe = issafe();
	/*
	** If powerful, detect collisions with monsters
	*/
	if ( Pacman.power > 0 ) {
		register monster *mptr, *mptr2;
		for ( mptr=Monster ; mptr < &Monster[MAXMONSTER] ; mptr++ ) {
			if ( mptr->state==EDIBLE && collide(mptr) ) {
				/*
				** Captured a monster...
				*/
				int crow, ccol;
				addscore(Monvalue);
#ifdef V1_25
				settextop(INVERT|COMPL);
#endif
#if V1_76 | V2_0
				settextop(RASTSXD);
#endif
#ifdef BLIT
				textmode = F_XOR;
#endif
				crow = SCtoROW(mptr->row)+1;
				ccol = SCtoCOL(min(mptr->col,Pacman.col))-6;
				mvprintf(crow, ccol, "%4d",Monvalue);
				if ( Silent )
					pause(750);
				else
					soundkill();
				mvprintf(crow, ccol, "%4d",Monvalue);
#ifdef V1_25
				settextop(REPLACE|COMPL);
#endif
#if V2_0 | V1_76
				settextop(RASTS);
#endif
#ifdef BLIT
				textmode = F_STORE;
#endif
				Monvalue *= 2;
				mptr->state = DEAD;
				monfont1(mptr,EYESCHAR);
				mptr->movestate = HOMING;
				for ( mptr2=Monster ; mptr2 < &Monster[MAXMONSTER]; mptr2++ )
					if ( mptr2->state == EDIBLE )
						mptr2->canreverse = TRUE;
			}
		}
	}
	eladd(Pacman.time+Pacman.adjusttime,pacmove);
}
paccomp()
{
	blt40(Pacman.font,Pacman.row,Pacman.col,INVERT);
}
/*
** pacpower - handle a transition in the Pacman's power
**		called from scheduler second & third time per pill.
**
** Parameters are set for 10 secs - 0.5 secs per wave, min 1 sec
*/
int
pacpower(junk,type)
char *junk;
int type;
{
	register int powertime;
	register monster *mptr;
	powertime = max(POWERTIME-Wave*POWERDELTA,MINPOWERTIME);
	switch ( type ) {
	case ON:
/*		soundpill():	*/
		addscore(VALPILL);
		Monvalue = VALMONSTER;
		for ( mptr=Monster ; mptr<&Monster[MAXMONSTER] ; mptr++ ) {
			mptr->canreverse = TRUE;
			if ( mptr->state == DANGEROUS ) {
				monfont1(mptr,GHOSTCHAR);
				mptr->state = EDIBLE;
			}
		}
		Pacman.power++;
		eladd(3*powertime/4,pacpower,NULL,CLOSE);
		break;
	case CLOSE:
		if ( Pacman.power == 1 ) {
			for ( mptr=Monster ; mptr<&Monster[MAXMONSTER];mptr++ )
				if ( mptr->state == EDIBLE )
					monfont1(mptr,CLOSECHAR);
		}
		eladd(powertime/4,pacpower,NULL,OFF);
		break;
	case OFF:
		if ( --Pacman.power <= 0 ) {
			for ( mptr=Monster ; mptr<&Monster[MAXMONSTER];mptr++ ) {
				if ( mptr->state == EDIBLE ) {
					mptr->state = DANGEROUS;
					monfont1(mptr,MONSTERCHAR[0]);
				}
			}
		}
		break;
	}
}
/*
** trymove - try to make a move
*/
trymove(newmove)
int newmove;
{
	move try;
	/*
	** If changing direction, row and col mod (FONTSIZE/BYTESIZE) must be 0
	*/
	if ( ((newmove^Pacman.dir)&1)  && !aligns(Pacman.row,Pacman.col)) {
		return 0;
	}
	/*
	** Movement ok, compute next position
	*/
	onestep(Pacman.row, Pacman.col, newmove, &try);
	if ( hitwall(try.row,try.col,newmove)) {
		return 0;
	}
	Pacman.row = try.row;
	Pacman.col = try.col;
	Pacman.dir = newmove;
	return 1;
}
/*
**  check if a screen coordinate hits a wall
*/
hitwall(row,col,dir)
{
	bool wall;
	if ( inslow(row,col) && (dir&1) )
		return(0);
	wall = (Board[SCtoMZ(row+2)][SCtoMZ(col+2)] & TYPEMASK) <= WALLMAX;
	return (wall || ((Board[SCtoMZ(row)][SCtoMZ(col)] & TYPEMASK) <= WALLMAX));
}
hitdoor(row,col)
{
	bool door;
	door = (Board[SCtoMZ(row+2)][SCtoMZ(col+2)] & TYPEMASK) == DOOR;
	return (door || ((Board[SCtoMZ(row)][SCtoMZ(col)] & TYPEMASK) == DOOR));
}
hitblk(row,col)
{
	int type;
	bool blk;
	type = (Board[SCtoMZ(row+2)][SCtoMZ(col+2)] & TYPEMASK);
	blk = type == BLKDOT || type == BLKNODOT;
	type = (Board[SCtoMZ(row)][SCtoMZ(col)] & TYPEMASK);
	return (blk || (type == BLKDOT || type == BLKNODOT));
}
inslow(row,col)
{
	bool slow;
	slow = (Board[SCtoMZ(row+2)][SCtoMZ(col+2)] & TYPEMASK) == SLOW;
	return (slow || ((Board[SCtoMZ(row)][SCtoMZ(col)] & TYPEMASK) == SLOW));
}
aligns(row,col)
{
	int rowmod, colmod;
	rowmod = row%(FONTSIZE/BYTESIZE);
	colmod = col%(FONTSIZE/BYTESIZE);
	return(rowmod==0 && colmod==0);
}
/*
** Check of the pacman collided with a monster
*/
collide(mptr)
register monster *mptr;
{
	return (iabs(mptr->row-Pacman.row)<3 && iabs(mptr->col-Pacman.col)<3 );
}
/*
** Check if the pacman has eaten a dot
*/
hasdot()
{
	if (Board[Pacman.mrow][Pacman.mcol] & GOLD ) {
		Board[Pacman.mrow][Pacman.mcol] &= ~GOLD;
		blt24(GOLDCHAR,MZtoSC(Pacman.mrow),MZtoSC(Pacman.mcol),INVERT);
		return TRUE;
	}
	return FALSE;
}
/*
** Check if the pacman has eaten a pill
*/
haspill()
{
	if (Board[Pacman.mrow][Pacman.mcol] & PILL ) {
		Board[Pacman.mrow][Pacman.mcol] &= ~PILL;
		blt24(PILLCHAR,MZtoSC(Pacman.mrow),MZtoSC(Pacman.mcol),INVERT);
		return TRUE;
	}
	return FALSE;
}
/*
** Check if the Pacman is in the safe corridor
*/
issafe()
{
	return (Board[Pacman.mrow][Pacman.mcol]&TYPEMASK) == SAFE;
}
rempacface()
{
	if (( Pacmen < MAXFACES ) && (Pacmen > 0))
		blt40(FACECHAR,MZtoSC(MAZEROWS+1),MZtoSC(Pacmen*2),INVERT);
}
/*
** Handle the death of the pacman
*/

char *pacdeath = "egstu"; /* font sequence */
int pacdtop[] = {150,170,190,210,0};
int pacdbot[] = {200,220,240,280,0};

pacdie()
{
	int i;
	Pacman.alive = FALSE;
	soundoff();
	sleep(
#ifdef BLIT
		10
#else
		500
#endif
			);
	moncomp();
	fruitcomp();
	elinit();
	for ( i=0 ; pacdeath[i]!='\0' ; i++ ) {
		paccomp();
		Pacman.font = pacdeath[i];
		paccomp();
#ifndef BLIT
		rsetdead();
#endif
		if ( Silent )
			sleep(
#ifdef BLIT
				10
#else
				500
#endif
					);
		else {
			if ( pacdtop[i] ) {
				tonegen(1,pacdtop[i],15-i*2);
				psgsweep(0,pacdtop[i],pacdbot[i],10);
			} else {
				soundoff();
				sleep(
#ifdef BLIT
					10
#else
					500
#endif
						);
			}
		}
	}
	soundoff();
	paccomp();
}
