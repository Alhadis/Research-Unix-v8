#include "sys/types.h"
#include "sys/stat.h"
extern int errno;
extern char *malloc();

int dbgfd;	/* for debugging output, shared with children */
int cntlfd;	/* fd for new service requests */
int ackfd;	/* for responding to network */
fd_set active;	/* bit map of active channels for select */
int myfd;	/* sole argument to children */
int silent;	/* debugging level */
#define NORMAN 60
struct {
	int pid;	/* pid of child, if it exists */
	int fd;		/* fd of channel to client (duplicates index) */
	int flags;	/* status */
	int dtime;	/* time difference */
	int dev;	/* client's idea of the root */
	long lastheard;	/* for time outs */
	char *who;	/* client's name, for trapping loops */
	int host;	/* correlates with permissions table */
	int silent;
} children[NORMAN];		/* one-one with fd's */
#define CONN	1
#define SPLIT	2
#define UNINIT	3

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
	char how;	/* how opened */
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
