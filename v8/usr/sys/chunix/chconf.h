/*
 * This file contains software configuration parameters for the NCP,
 * as well as the definitions
 * for the hardware interfaces in use
 */
/*
 * Software configuration parameters - system independent
 */
#ifdef VMUNIX
#define CHNCONNS	40	/* Maximum number of connections */
#else
#define CHNCONNS	20	/* Maximum number of connections */
#endif VMUNIX
#define CHDRWSIZE	5	/* Default receive window size */
#define CHMINDATA	(32-CHHEADSIZE)
#define CHRFCTRYS	3	/* Try 3 RFC's before giving up */
#define CHNSUBNET	070	/* Number of subnets in routing table */
/*
 * Software configuration parameters - system dependent
 */
#define chroundup(n)	((n) <= (32-CHHEADSIZE) ? (32-CHHEADSIZE) : \
			 (n) <= (128-CHHEADSIZE) ? (128-CHHEADSIZE) : \
			 (512-CHHEADSIZE) )

#define CHBUFTIME	5*Chhz	/* timeout ticks for no buffers (used?) */

/*
 * Hardware configuration parameters
 *
 * Foreach interface type (e.g. xxzzz), you need:

#include "chxx.h"
#if NCHXX > 0
#include "../chaos/xxzzz.h"	* for device register definitions and
				   struct xxxxinfo defining software state *
extern short xxxxhosts[];	* array of actual host numbers
				   (initialized in chconf.c) *
				* only needed for interfaces that don't know
				  their own address *
#endif
 */
/*
 * For dr11-c's
 */
#include "chdr.h"
#if NCHDR > 0
#include "../chaos/dr11c.h"
extern short dr11chosts[];	/* local host address per dr11c interface */
#endif

/*
 * For ch11's - if not VMUNIX we must specify number of interfaces here.
 */
#include "chch.h"
#if NCHCH > 0
#include "../chaos/ch11.h"
#endif
/*
 * For chil's.
 */
#include "chil.h"
#if NCHIL > 0
#include "../h/ilreg.h"
#include "../h/if_il.h"
#include "../chaos/chil.h"
#endif

/*
 * This union should contain all device info structures in use.
 */
union xcinfo {
#if NCHDR > 0
	struct dr11cinfo	xc_dr11c;	/* for dr11c's */
#endif
#if NCHCH > 0
	struct ch11info		xc_ch11;	/* for ch11's */
#endif
#if NCHIL > 0
	struct chilinfo		xc_chil;	/* for chil's */
#define xc_ilinfo xc_info.xc_chil
#endif
	/* other devices go here */
};
