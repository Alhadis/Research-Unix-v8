/*
 * machine dependent stuff, mostly type structure
 *
 * some of these types belong to adb; some belong to the machine
 * being debugged.  (imagine adb running on one type of cpu, examining
 * a core image from another.)
 *
 * WORD and ADDR belong to adb.
 * WORD holds expression values.  it must hold an ADDR.
 * ADDR is an address in a file.  it should probably be unsigned,
 * unlike off_t.  (think of VAX kernel addresses)
 *
 * other types belong to the target machine:
 * TCHAR is a byte
 * TSHORT is a short integer
 * TLONG is a long integer
 * TADDR is a core address (probably char *)
 * TREG is whatever fits in a register
 * all of these must fit in a WORD, or things won't work right
 *
 * TFLOAT and TDOUBLE are float and double on the target;
 * they needn't fit in a WORD, though.
 *
 * the T* types are, in general, magic cookies, and shouldn't be
 * examined directly, save perhaps to compare them.  the conversions
 * wtol &c should be used first.
 *
 * printf (adb's own) prints adb integers, not target ones.  it prints
 * target floats, though.  this is probably a botch.
 */

/*
 * definitions for a VAX, debugging a VAX
 */

typedef long WORD;
typedef unsigned long ADDR;

#define	HUGE	0x7fffffff	/* enormous WORD */

typedef unsigned char TCHAR;
typedef unsigned short TSHORT;
typedef long TLONG;
typedef char *TADDR;
typedef int TREG;
typedef float TFLOAT;
typedef double TDOUBLE;

/*
 * conversions: WORD to target type
 */

#define	wtoc(x)	((TCHAR)x)
#define	wtos(x)	((TSHORT)x)
#define	wtol(x)	((TLONG)x)
#define	wtoa(x)	((TADDR)x)
#define	wtor(x)	((TREG)x)

/*
 * conversions: target type to WORD
 */

#define	ctow(x)	((WORD)x)
#define	stow(x)	((WORD)x)
#define	ltow(x)	((WORD)x)
#define	atow(x)	((WORD)x)
#define	rtow(x)	((WORD)x)

TCHAR cget();
TSHORT sget();
TLONG lget();
TADDR aget();
TREG rget();

/*
 * is this number seriously negative?
 * hack to make numbers prettier
 */

#define	REALLYNEG(n)	(((n) & 0xFFFF0000) == 0xFFFF0000)

/*
 * miscellany
 */

#define	DEFRADIX	16
