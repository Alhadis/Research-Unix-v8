/*----------------------------------------------------------------------*/
/*									*/
/*	PACMAN for BBN BitGraphs					*/
/*									*/
/*	          File: blt.c68						*/
/*	      Contents:	block transfer routines				*/
/*	        Author: Bob Brown (rlb)					*/
/*			Purdue CS					*/
/*		  Date: May, 1982					*/
/*	   Description:	see below, customized for pacman		*/
/*									*/
/*----------------------------------------------------------------------*/

#include "style.h"
#include "pacman.h"

/*
** block transfer routines - customized for pacman
**
** blt24 - copies a 24x24 font character to the screen
** blt40 - copies a 40x40 font character to the screen
**
** opcode:
**	REPLACE ... replace mode
**	PAINT ..... or in
**	INVERT .... xor in
*/


blt24(chr, row, col, opcode)
char chr;
int row,col;
int opcode;
{
	register unsigned char *mem, *form;
	register unsigned char *formend;
#ifdef V1_25
	mem = SCtoMEM(row-1,col+1);
#endif
#if V1_76 | V2_0
	mem = SCtoMEM(row+1,col-1);
#endif
#ifdef BLIT
	mem = SCtoMEM(row+22,col+5);
#endif
	form = chr24[chr-'a'];
	formend = form+FONTSIZE*FONTSIZE/BYTESIZE;
	switch ( opcode ) {
	case REPLACE:
		while ( form < formend ) {
			*(mem) = *(form);
			*(mem+1) = *(form+1);
			*(mem+2) = *(form+2);
			mem += RASTROWLEN;
			form += FONTSIZE/BYTESIZE;
		}
		break;
	case INVERT:
		while ( form < formend ) {
			*(mem) ^= *(form);
			*(mem+1) ^= *(form+1);
			*(mem+2) ^= *(form+2);
			mem += RASTROWLEN;
			form += FONTSIZE/BYTESIZE;
		}
		break;
	}
}
/*
** Forty-bit font blt.  
*/

blt40(chr, row, col, opcode)
char chr;
int row,col;
int opcode;
{
	register unsigned char *mem, *form;
	register unsigned char *formend;
	register int            i;
#ifdef V1_25
	mem = SCtoMEM(row-2,col+2);
#endif
#if V1_76 | V2_0
	mem = SCtoMEM(row+2,col-2);
#endif
#ifdef BLIT
	mem = SCtoMEM(row+21,col+4);
#endif
	form = chr40[chr-'a'];
	formend = form+BIGFONT*BIGFONT/BYTESIZE;
	switch ( opcode ) {
/*
	case REPLACE:
		while ( form < formend ) {
			*(mem) = *(form);
			*(mem+1) = *(form+1);
			*(mem+2) = *(form+2);
			*(mem+3) = *(form+3);
			*(mem+4) = *(form+4);
			mem += RASTROWLEN;
			form += BIGFONT/BYTESIZE;
		}
		break;
	case PAINT:
		while ( form < formend ) {
			*(mem) |= *(form);
			*(mem+1) |= *(form+1);
			*(mem+2) |= *(form+2);
			*(mem+3) |= *(form+3);
			*(mem+4) |= *(form+4);
			mem += RASTROWLEN;
			form += BIGFONT/BYTESIZE;
		}
		break;
*/
	case INVERT:
		while ( form < formend ) {
			*(mem) ^= *(form);
			*(mem+1) ^= *(form+1);
			*(mem+2) ^= *(form+2);
			*(mem+3) ^= *(form+3);
			*(mem+4) ^= *(form+4);
			mem += RASTROWLEN;
			form += BIGFONT/BYTESIZE;
		}
		break;
	}
}

