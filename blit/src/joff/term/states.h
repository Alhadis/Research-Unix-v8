#define	RUN		1	/* ready to be scheduled */
#define	BUSY		2	/* active */
#define	BLOCKED		4	/* blocked by user with ^S */
#define	USER		8	/* a user-68ld'd process */
#define	KBDLOCAL	16	/* has requested the KBD */
#define	MOUSELOCAL	32	/* has requested the MOUSE */
#define	GOTMOUSE	64	/* currently owns MOUSE */
#define	WAKEUP		128	/* tell CONTROL to issue setrun(p) */
#define	MOVED		256	/* Don't do mpxublk */
#define	UNBLOCKED	512	/* Has been unblocked */
#define	ZOMBIE		1024	/* proc died horribly */
#define	RESHAPED	2048	/* layer got reshaped */
#define	ZOMBOOT		4096	/* put in ZOMBIE state after booting */
