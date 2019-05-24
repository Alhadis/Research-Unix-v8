#include <stdio.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <dir.h>
#include "defs.h"
#include "load.h"
#include "msg.h"
#include "scanmail.h"

/* globals for mail scanning */
struct scanmail smt[NOFILE/4];
long lastscan = 0;
char doscan = FALSE;
FILE *mlfp = NULL;

/* imported */
extern int errno;
extern char *smnext();
extern void sminit();

/* initialize */
init()
{
    initload();
}

/* return my name */
char *
myname()
{
    return ("mon");
}

/* send the current information to a client */
sendinfo (fd)
int fd;
{
    struct msg m;
    char *p;

    /* send the load */
    m.u.load = load;
    m.size = sizeof(m.u.load) + sizeof(m.size) + sizeof(m.tag);
    m.tag = LOAD;
    if (write (fd, &m, m.size) != m.size)
	return (-1);

    /* check for mail if its time */
    if (doscan) {
	while ((p = smnext (&(smt[fd]), mlfp)) != NULL) {
	    m.size = sizeof(m.size) + sizeof(m.tag) + strlen(p) + 1;
	    m.tag = MAIL;
	    strcpy (m.u.mail, p);
	    if (write (fd, &m, m.size) != m.size)
		return (-1);
	}
    }
    return (0);
}

/* generate the current information */
generate()
{
    long now;

    /* get load info */
    genload ();

    /* find out if we should scanmn the mail */
    now = time (NULL);
    doscan = (now - lastscan) > 60*2;
    if (doscan) {
	if (mlfp != NULL)
	    fclose (mlfp);
	mlfp = fopen (MAILNAME, "r");
	lastscan = now;
    }
}

/* add a client */
void
add (fd, who)
    int fd;
    char *who;
{
    sminit (&(smt[fd]), NULL, who);
}

/* drop a client */
void
drop (fd)
{
}
