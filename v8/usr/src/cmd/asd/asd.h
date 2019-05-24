#include "ar.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>

/* longest archive component name we will generate */
#define MAXCOMP 14

/* macro to allocate storage of a given type */
#define new(t) ((t *) alloc (sizeof (t)))

/* constants for tmpname function */
#define TMPDIR	"/tmp/"
#define TMPNAML	(sizeof(TMPDIR) + 15)

/* some systems define SIG_TYP, others don't, so we make our own */
typedef int	(*Sig_typ)();

struct replist {
	char *source;
	char *dest;
	struct replist *link;
} *replist;

/* structures to deal with archive headers */
struct ar_hdr ar_hdr;
struct {
	long size;
	int mode;
	long date;
} hdr;

/*
 *	This stat buffer makes it easy to check that none of the
 *	input files is the same as the output.  This will avoid
 *	filling up the entire file system by inadvertently saying
 *
 *		mkpkg . > foo
 */
struct stat outsb;

/*
 *	The following structure helps keep track of things being packaged.
 *	iname is the internal name of the component -- in other words,
 *	the archive element name.  ename is the (short) pathname of the
 *	file.  The structures are chained by the "link" field.  All the
 *	other fields are copies of things returned by "stat" and are
 *	used mostly to make sure nothing changed while we were packaging.
 *	head and tail point to the first and last items in the chain.
 *	The first item is known to refer to the "Instructions" component.
 */
struct pack {
	char *iname;
	char *ename;
	struct pack *link;
	dev_t dev;
	ino_t ino;
	int uid, gid, mode;
	time_t time;
	off_t size;
};

struct pack *pkhead, *pktail;


FILE *popen();
char *alloc();
char *copy();
char *fgets();
char *fullname();
char *getfield();
char *getpass();
char *getpath();
char *hextab;
char *iname();
char *index();
char *instr;
char *keyfile;
char *mktemp();
char *optarg;
char *pwd();
char *ralloc();
char *rindex();
char *strcat();
char *strcpy();
char *strgid();
char *struid();
char *tmpname();
char *transname();
int Kflag;
int bflag;
int consider();
int dflag;
int errno;
int kflag;
int nflag;
int install();
int numgid();
int numuid();
int optind;
int package();
int pkgend();
int retcode;
int seal();
int unseal();
int vflag;
long cvlong();
long read_header();
void delete();
void geteol();
void next_header();
void pkgstart();
void pkgfile();
void putpath();
