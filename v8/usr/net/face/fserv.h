#include "sys/types.h"
#include "sys/stat.h"
extern int errno;
extern char *malloc();

int dbgfd;	/* for debugging output, shared with children */
int cntlfd;	/* fd for new service requests */
int ackfd;	/* for responding to network */
int myfd;	/* sole argument to children */
int silent;	/* debugging level */

#define NDEV 10
extern struct tdev {
	int ours, his;
} devtab[NDEV];
extern int ndev;

typedef struct aa {
	long tag;
	int dev;	/* his dev */
	int ino;
	char *name;	/* some name */
	struct stat statb;	/* cached */
	short fd;	/* the precious handle */
	long offset;	/* seek offset */
} netf;

extern netf *getnetf(), *gettag(), *newnetf(), *oldnetf();
#define ftype(p) (p->statb.st_mode & S_IFMT)
/* 1 for cmnd outline, 2 for cmnd detail, 4 for permissions */
#define debug1 if(silent & 1) xdebug
#define debug2 if(silent & 2) xdebug
#define debug3 if(silent & 3) xdebug
#define debug4 if(silent & 4) xdebug
#define NDBG 200
int dptr;
char debugbuf[NDBG][128];
