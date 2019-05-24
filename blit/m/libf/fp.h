/*
 *	Floating-point numbers are stored in an object of
 *	type "fp" (usually a long integer).  The high-order
 *	bit is the sign, the next EXPSIZE bits are the exponent,
 *	and the remaining FRACSIZE bits are the fraction.
 *	Thus, the number of bits in an fp must be equal to
 *	EXPSIZE + FRACSIZE + 1.
 *
 *	The exponent is stored in excess EXPOFFSET notation,
 *	and the fraction has a hidden bit.
 */

typedef long fp;

/*
 *	parameters of the 68000 implementation
 */
#define EXPSIZE 8
#define EXPOFFSET (1 << (EXPSIZE - 1))
#define MAXEXP ((1 << EXPSIZE) - 1)
#define FRACSIZE 23
#define HIDDENBIT 0x00800000L
#define CARRYBIT (HIDDENBIT << (GBITS + 1))
#define GBITS 2
#define NORMMASK 0xfe000000L

#define SIGNMASK 0x80000000L
#define EXPMASK 0x7f800000L
#define FRACMASK 0x007fffffL

/*
 *	how to pick pieces out of a floating-point number
 */
#define SEXP(x) ((x) & EXPMASK)
#define EXP(x) ((int) (SEXP (x) >> FRACSIZE))
#define SIGN(x) ((x) & SIGNMASK)
#define FRAC(x) ((x) & FRACMASK)
#define SETEXP(x,y) x = (((x) & ~EXPMASK) | ((fp) (y) << FRACSIZE))
#define SETSIGN(x,y) x = (((x) & ~SIGNMASK) | ((fp) (y) << (FRACSIZE + EXPSIZE)))
#define SETFRAC(x,y) x = (((x) & ~FRACMASK) | (y))
#define SETEF(x,e,f) x = \
	(((x) & SIGNMASK) | ((fp) (e) << FRACSIZE) | ((f) & FRACMASK))

/*
 *	macros to pick pieces out of long integers
 *	this must change if FRACSIZE changes
 *	presently we assume two 12-bit pieces
 */
#define CHUNK 12
#define lmul(x,y) ((long) (x) * (long) (y))
#define lo(x) ((x) & 0xfff)
#define hi(x) ((x) >> CHUNK)
#define hibit(x) (((short)(x) >> (CHUNK - 1)) & 1)

static fp zero;
static fp infinity = ~SIGNMASK;

fp fladd(), flsub(), flmul(), fldiv(), flneg(), fp_addmag(), fp_submag();
fp itof(), uitof(), ltof(), ultof();
