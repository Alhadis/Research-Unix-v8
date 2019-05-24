/*
 *	Package to maintain the temporary file.
 */
#include <stdio.h>
#include <signal.h>
#include "letter.h"
#include "mail.h"
#include "string.h"

/* imports */
extern char *mktemp();
extern int panic();

/* globals */
static letter let[NLETTERS];

#define TMPNAME "/tmp/maXXXXX"
static char tmpfile[sizeof(TMPNAME)];

static FILE *tfp;
static letter *lastp;

#define ASSERT(x,y)	if(!(x)) panic(y)

void
inittmp()
{
	/* make temp file */
	(void)strcpy(tmpfile, TMPNAME);
	(void)mktemp(tmpfile);
	(void)unlink(tmpfile);
	close(creat(tmpfile, 0600));
	chown(tmpfile, getuid(), getgid());
	tfp = fopen(tmpfile, "r+");
	if (tfp == NULL)
		panic("mail: can't open tempfile");
}

void
releasetmp()
{
	if (tfp != NULL)
		fclose(tfp);
	unlink(tmpfile);
}

/*
 *	Modes are:
 *		"w"	to write a letter
 *		"r"	to read a letter
 *		"ru"	to read only undeleted letters
 */
letter *
lopen(n, mode)
	int n;
	char *mode;
{
	letter *lp, *rv = NULL;

	if (n >= NLETTERS)
		return NULL;

	lp = &let[n];
	if (strcmp(mode, "w")==0) {
		lp->status = L_USED;
		(void)fseek(tfp, 0L, 2);
		lp->address = ftell(tfp);
		lp->offset = lp->size = 0;
		rv = lp;

	} else if (strcmp(mode, "r")==0) {
		if (lp->status & L_USED) {
			lp->offset = 0;
			(void)fseek(tfp, lp->address, 0);
			rv = lp;
		}

	} else if (strcmp(mode, "ru")==0) {
		if ((lp->status & L_USED) && !(lp->status & L_DELETED)) {
			lp->offset = 0;
			(void)fseek(tfp, lp->address, 0);
			rv = lp;
		}
	}

	lastp = rv;
	return rv;
}

extern char *
lgets(bp, len, lp)
	char *bp;
	int len;
	letter *lp;
{
	char *rv;

	ASSERT(lp->status & L_USED, "tmpfile botch");
	if (lp != lastp)
		(void)fseek(tfp, lp->address+lp->offset, 0);
	if (lp->offset >= lp->size)
		return NULL;
	if (lp->offset+len > lp->size)
		len = lp->size - lp->offset + 1;
	rv = fgets(bp, len, tfp);
	ASSERT(rv != NULL, "tmpfile botch");
	lp->offset = ftell(tfp) - lp->address;
	lastp = lp;
	return bp;
}

extern void
lputs(bp, lp)
	char *bp;
	letter *lp;
{
	int rv;

	ASSERT(lp->status & L_USED, "tmpfile botch");
	if (lp != lastp)
		(void)fseek(tfp, lp->address+lp->offset, 0);
	rv = fputs(bp, tfp);
	ASSERT(rv != EOF, "can't write to /tmp");
	lp->size = lp->offset = ftell(tfp) - lp->address;
	lastp = lp;
}

extern void
lputc(c, lp)
	char c;
	letter *lp;
{
	char rv;

	ASSERT(lp->status & L_USED, "tmpfile botch");
	if (lp != lastp)
		(void)fseek(tfp, lp->address+lp->offset, 0);
	rv = fputc(c, tfp);
	ASSERT(rv != EOF, "can't write to /tmp");
	lp->size = lp->offset = ftell(tfp) - lp->address;
	lastp = lp;
}

extern long
ltell(lp)
	letter *lp;
{
	ASSERT(lp->status & L_USED, "tmpfile botch");
	return lp->offset;
}

extern long
lsize(lp)
	letter *lp;
{
	ASSERT(lp->status & L_USED, "tmpfile botch");
	return lp->size;
}

extern int
letseek(lp, off)
	letter *lp;
	long off;
{
	ASSERT(lp->status & L_USED, "tmpfile botch");
	lp->offset = off < lp->size ? off : lp->size - 1;
	lastp = NULL;
	return 0;
}

extern int
ldelete(n)
	int n;
{
	ASSERT(n>= 0 && n < NLETTERS && (let[n].status & L_USED), "tmpfile botch");
	let[n].status |= L_DELETED;
	return 1;
}

extern int
lundelete(n)
	int n;
{
	ASSERT(n>= 0 && n < NLETTERS && (let[n].status & L_USED), "tmpfile botch");
	let[n].status &= ~L_DELETED;
	return 1;
}

extern int
copyto(fp, lp, onatty, escape)
	FILE *fp;
	letter *lp;
	int onatty;
	int escape;
{
	char buf[FROMLINESIZE];

	while (fgets(buf, FROMLINESIZE, fp)!=NULL) {
		if (onatty && buf[0]=='.' && buf[1]=='\n' && buf[2]=='\0')
			break;
		if (escape && strncmp(buf, FROM, FSIZE)==0)
			lputc('>', lp);
		(void)lputs(buf, lp);
	}
}

extern int
copyfrom(lp, fp)
	letter *lp;
	FILE *fp;
{
	char buf[FROMLINESIZE];

	while (lgets(buf, FROMLINESIZE, lp)!=NULL)
		(void)fputs(buf, fp);
}
