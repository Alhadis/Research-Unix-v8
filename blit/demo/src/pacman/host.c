/*----------------------------------------------------------------------*/
/*									*/
/*	PACMAN for BBN BitGraphs					*/
/*									*/
/*	          File:	hostscore.c68					*/
/*	      Contents:	routines for host score file support		*/
/*	        Author: Rich Fortier (rwf)				*/
/*			Bolt Beranek & Newman				*/
/*		  Date: May, 1982					*/
/*	   Description:	support for keeping a high score file on the	*/
/*			host.						*/
/*									*/
/*----------------------------------------------------------------------*/

#include "style.h"
#include "host.h"

/*****************************************************************************
* encrypt:  trivial encryption routine.                                      *
*                                                                            *
*   Arguments:  num, the number to encrypt.                                  *
*                                                                            *
*   Returns:    the encrypted number.                                        *
*                                                                            *
*   Remarks:   this  routine  is  in no way intended to provide any          *
* security in the numbers transmitted back to the host.  It's  sole          *
* purpose  is  to  make  is sufficiently tedious to type in a bogus          *
* number to the host that most people won't bother.                          *
*                                                                            *
*****************************************************************************/

encrypt (num)
register int num;
{
    return num ^= 0xA5A5;
}


/*****************************************************************************
* sndhost:  formatted print routine.                                         *
*                                                                            *
*   Arguments:  fmt, the print format.                                       *
*               x1,  the first argument.                                     *
*                                                                            *
*   Returns:    nothing.                                                     *
*                                                                            *
*   Remarks:  simply invoke lower level routine with proper display routine. *
*                                                                            *
*****************************************************************************/

/*VARARGS1*/
sndhost (fmt, x1)
char *fmt;
unsigned x1;
{
    extern int hstput ();

#ifndef BLIT
    prf(fmt, &x1, hstput);
#endif
}




/*****************************************************************************
* hstput:  host put a character routine.                                     *
*                                                                            *
*   Arguments:  c, the character to put.                                     *
*                                                                            *
*   Returns:    nothing.                                                     *
*                                                                            *
*   Remarks:  simply invoke the queue put routine, looping until success.    *
*                                                                            *
*****************************************************************************/

hstput (c)
register int c;
{
    extern int hstputc();
    extern int hostpc();

#ifdef V2_0
    while (hstputc (c) == -1);
#endif

#ifdef V1_25
    while (hostpc (c) == -1);
#endif
}





