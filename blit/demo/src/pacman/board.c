/*----------------------------------------------------------------------*/
/*									*/
/*	PACMAN for BBN BitGraphs					*/
/*									*/
/*	          File:	board.c68					*/
/*	      Contents:	declaration of game board and operations	*/
/*	        Author: Bob Brown (rlb)					*/
/*			Purdue CS					*/
/*		  Date: May, 1982					*/
/*	   Description:	The Game board is not easily changed.  The	*/
/*			characters used are also defined in pacman.h	*/
/*									*/
/*----------------------------------------------------------------------*/

#include "style.h"
#include "pacman.h"

/*
** Due to memory limitations, the static game board shown below is
** overwritten by the dynamic version.  
*/

bool	Blasted = FALSE;

char *Board[] = {
	"2000000000000320000000000003",
	"1777777777777117777777777771",
	"1720037200037117200037200371",
	"1819917199917117199917199181",
	"1750047500047547500047500471",
	"1777777777777777777777777771",
	"1720037237200000037237200371",
	"1750047117500320047117500471",
	"1777777117777117777117777771",
	"5000037150039119200417200004",
	"999991712004=54=500317199999",
	"999991711;;;;;;;;;;117199999",
	"999991711;20666603;117199999",
	"000004754;19999991;547500000",
	"::::::799;19999991;997::::::",
	"000003723;19999991;237200000",
	"999991711;50000004;117199999",
	"999991711;;;;;;;;;;117199999",
	"9999917119200000039117199999",
	"2000047549500320049547500003",
	"1777777777777117777777777771",
	"1720037200037117200037200371",
	"175031750004<54<500047120471",
	"1877117777777997777777117781",
	"5037117237200000037237117204",
	"2047547117500320047117547503",
	"1777777117777117777117777771",
	"1720000450037117200450000371",
	"1750000000047547500000000471",
	"1777777777777777777777777771",
	"5000000000000000000000000004",
};

/*
** Generate a new board from the fixed board
*/
newboard()
{
	register int i,j;
	register char c;
	for ( i=0 ; i<MAZEROWS ; i++ )
		for ( j=0 ; j<MAZECOLS ; j++ ) {
			Board[i][j] &= TYPEMASK;
			switch ( Board[i][j] ) {
			case FIXPILL:
				Board[i][j] |= PILL;
				break;
			case BLKDOT:
			case FIXGOLD:
				Board[i][j] |= GOLD;
			}
		}
}
char chrmap[] = {"abcdefghi   h "};
drawboard()
{
	register int i,j;
	register char c;
	/*cursinhibit();*/
	for ( i=0 ; i<MAZEROWS ; i++ ) {
		for ( j=0 ; j<MAZECOLS ; j++ ) {
			c = chrmap[Board[i][j] & TYPEMASK];
			if ( c!=' ' )
				blt24(c,MZtoSC(i),MZtoSC(j),REPLACE);
		}
#ifndef BLIT
		rsetdead();
#endif
	}
	/*cursallow();*/
}
