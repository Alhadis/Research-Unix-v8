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

extern Bitmap Bitmap24,Bitmap40;

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
#endif
#ifdef BLIT
	bitblt(&Bitmap24,Rect(24*(chr-'a'),0,24*(chr-'a'+1),24),&display,Pt(col*8+40,row*8+176),(opcode==REPLACE)?F_STORE:F_XOR);
#endif
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
	form = chr40[chr-'a'];
	formend = form+BIGFONT*BIGFONT/BYTESIZE;
	switch ( opcode ) {
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
#endif
#ifdef BLIT
	bitblt(&Bitmap40,Rect(40*(chr-'a'),0,40*(chr-'a'+1),40),&display,Pt(col*8+32,row*8+168),F_XOR);
#endif
}

