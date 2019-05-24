/*
 * Externals for functions defined in vmsys.c.
 */

int	nosys();
int	nullsys();

int	vfork();		/* later, just fork? */
int	vadvise();		/* later, segadvise */

int	vlimit();
int	vswapon();
int	vtimes();
int	select();

#ifdef TRACE
int	vtrace();
#endif

int	resuba();
int	futz();
int	settod();
