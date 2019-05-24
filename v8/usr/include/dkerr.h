#define	HDEAD	1		/* host dead */
#define	NOLIS	2		/* no listener */
#define	NOCHAN	3		/* out of channels */
#define	NOSRV	4		/* server not available */
#define	EXIST	5		/* already exists */

#ifdef	DKMSGS
char *dkmsgs[] ={
	"",
	"host dead\n",
	"no listener\n",
	"out of channels\n",
	"no server\n",
};
#define	NDKERR	4
#endif
