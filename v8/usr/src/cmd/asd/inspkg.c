#include "asd.h"

#define CHUNK 64

/* type codes for installation subroutine */
#define BACKUP 0
#define INSTALL 1

static int retcode;

void readtemp();

/*
 *	The following declarations and functions manipulate
 *	a list of directory names.  This list is used to decide
 *	which files should be backed up and which have already been.
 */

struct list {
	char *name;
	struct list *next;
};

static struct list *dirs;

/* is the name given a subdirectory of the name on the list? */
static int
subsumed (name)
	register char *name;
{
	register struct list *item;
	register char *p, *q;

	for (item = dirs; item; item = item->next) {
		p = item->name;
		q = name;
		while (*p && *p == *q)
			p++, q++;
		if (*p == '\0' && (*q == '/' || *q == '\0'))
			return 1;
	}

	return 0;
}

/* add the name given to the list */
static void
addlist (name)
	register char *name;
{
	register struct list *l;

	l = new (struct list);
	l->next = dirs;
	l->name = copy (name);
	dirs = l;
}

/* clear the entire list */
static void
clearlist()
{
	register struct list *l;

	while (l = dirs) {
		dirs = l->next;
		free (l->name);
		free ((char *) l);
	}
}

/*
 *	install the given file
 */
process (file, fname)
	FILE *file;
	char *fname;
{
	register int c, rc = 0;

	if (vflag)
		fprintf (stderr, "%s:\n", fname);

	while ((c = getc (file)) != EOF) {
		ungetc (c, file);
		rc += doarch (file);
	}

	return rc;
}

main (argc, argv)
	int argc;
	char **argv;
{
	umask (0);
	return getargs (argc, argv, "nvbD:", process);
}

static char tfname[TMPNAML];
void delete();

/* process a single archive in a concatenation */
doarch (file)
	register FILE *file;
{
	register FILE *tf;
	Sig_typ sigsav;
	register long size;
	register int c;
	char armag[SARMAG];

	/* Make sure the file is an archive */
	if (fread (armag, sizeof (*armag), SARMAG, file) != SARMAG) {
		fprintf (stderr, "inspkg: unexpected EOF\n");
		exit (1);
	}
	if (strncmp (armag, ARMAG, SARMAG) != 0) {
		fprintf (stderr, "inspkg: input not a package\n");
		exit (1);
	}

	/* establish a temporary file */
	(void) tmpname (tfname);
	tf = fopen (tfname, "w");
	sigsav = signal (SIGINT, SIG_IGN);
	if (sigsav != SIG_IGN)
		signal (SIGINT, delete);
	chmod (tfname, 0600);

	/* copy the installation instructions to the temp file */
	size = read_header (instr, file);
	while (--size >= 0) {
		c = getc (file);
		if (c == EOF) {
			fprintf (stderr, "inspkg: premature EOF\n");
			exit (1);
		}
		putc (c, tf);
	}
	fclose (tf);

	next_header (file);

	/* create the optional backup package */
	if (bflag) {
		pkgstart();
		readtemp (BACKUP, file);
		pkgend();
		clearlist();
	}

	/* do the actual work */
	readtemp (INSTALL, file);

	/* delete the temporary file */
	nchk (unlink (tfname));
	signal (SIGINT, sigsav);

	return 0;
}

/*
 *	Make a pass through the temp file.
 */
static void
readtemp (code, file)
	register int code;
	register FILE *file;
{
	register FILE *tf;
	register int c;

	/* we're done writing the temp file, time to read it */
	tf = fopen (tfname, "r");
	schk ((char *) tf);

	/*
	 *	The main loop -- one iteration per line
	 *	We are careful in use and reuse of storage here;
	 *	if you change this code make sure you understand
	 *	the times at which getfield and transname
	 *	recycle storage or strange things will happen.
	 */
	while ((c = getc (tf)) != EOF) {
		char *param, *path, *path2;
		register FILE *outfd;
		int uid, gid, mode, dmajor, dminor, dev;
		register long size;
		char component[MAXCOMP+1];

		switch (c) {

		/* special files */
		case 'b':
		case 'c':
			/* read the parameters */
			param = getfield (tf);
			mode = cvlong (param, strlen (param), 8) |
			    (c == 'c'? S_IFCHR: S_IFBLK);
			param = getfield (tf);
			dmajor = cvlong (param, strlen (param), 10);
			param = getfield (tf);
			dminor = cvlong (param, strlen (param), 10);
			dev = makedev (dmajor, dminor);
			uid = numuid (getfield (tf));
			gid = numgid (getfield (tf));
			path = transname (getfield (tf));
			geteol (tf);

			switch (code) {

			case BACKUP:
				if (!subsumed (path)) {
					pkgfile (path);
					addlist (path);
				}
				break;

			case INSTALL:
				if (vflag) {
					fprintf (stderr, "special file ");
					putpath (stderr, path);
					fprintf (stderr, "\n");
				}

				if (!nflag) {
					if (mknod (path, mode, dev) >= 0)
						chown (path, uid, gid);
					else
						perror (path);
				}
				break;
			}
			break;

		/* directory */
		case 'd':
			/* read the parameters */
			param = getfield (tf);
			mode = cvlong (param, strlen (param), 8);
			uid = numuid (getfield (tf));
			gid = numgid (getfield (tf));
			path = transname (getfield (tf));
			geteol (tf);

			switch (code) {

			case BACKUP:
				if (!subsumed (path)) {
					pkgfile (path);
					addlist (path);
				}
				break;

			case INSTALL:
				/* make the directory */
				if (vflag) {
					fprintf (stderr, "directory ");
					putpath (stderr, path);
					putc ('\n', stderr);
				}
				if (!nflag) {
					rmall (path);
					mkdir (path);
					chmod (path, mode);
					chown (path, uid, gid);
					chmod (path, mode);
				}
				break;
			}

			break;

		/* file */
		case 'f':
			/* read parameters */
			param = getfield (tf);
			if (strlen (param) > MAXCOMP) {
				fprintf (stderr,
				    "inspkg: impossibly long component name\n");
				delete();
				exit (1);
			}
			strcpy (component, param);
			uid = numuid (getfield (tf));
			gid = numgid (getfield (tf));
			path = transname (getfield (tf));
			geteol (tf);

			switch (code) {

			case BACKUP:
				if (!subsumed (path))
					pkgfile (path);
				break;

			case INSTALL:
				/* read the corresponding archive header */
				size = read_header (component, file);

				if (vflag) {
					fprintf (stderr, "file ");
					putpath (stderr, path);
					putc ('\n', stderr);
				}

				/* create and open the file */
				if (!nflag) {
					rmall (path);
					umask (077);
					outfd = fopen (path, "w");
					umask (0);
					if (outfd == NULL) {
						fprintf (stderr,
						    "inspkg: cannot create ");
						putpath (stderr, path);
						putc ('\n', stderr);
					}
				}
				if (nflag || outfd == NULL) {
					outfd = fopen ("/dev/null", "w");
					schk ((char *) outfd);
				}

				/* copy the file, advance input */
				while (--size >= 0)
					putc (getc (file), outfd);
				next_header (file);

				/* check successful completion, close output */
				fflush (outfd);
				if (feof (file) || ferror (file) ||
				    ferror (outfd)) {
					fprintf (stderr, "inspkg: can't write ");
					putpath (stderr, path);
					putc ('\n', stderr);
				}
				fclose (outfd);
				
				/*
				 *	Update output modification times
				 *	and change mode and owner.
				 *	This is done here to cater to systems
				 *	that allow people to give files away.
				 *	The chmod is done twice because
				 *	systems that let you give files away
				 *	won't let you change the mode after
				 *	you've done so, and some other systems
				 *	turn off the setuid bit as a side
				 *	effect of chown, so the second chmod
				 *	will restore that bit.
				 */
				if (!nflag) {
					time_t timep[2];
					timep[0] = timep[1] = hdr.date;
					utime (path, timep);
					chmod (path, hdr.mode & 07777);
					chown (path, uid, gid);
					chmod (path, hdr.mode & 07777);
				}

				break;
			}
			break;

		/* link */
		case 'l':
			/* read parameters */
			path = copy (transname (getfield (tf)));
			path2 = transname (getfield (tf));
			geteol (tf);

			switch (code) {

			case BACKUP:
				if (!subsumed (path2))
					pkgfile (path2);
				break;

			case INSTALL:
				/* log it if requested */
				if (vflag) {
					fprintf (stderr, "link ");
					putpath (stderr, path);
					fprintf (stderr, " to ");
					putpath (stderr, path2);
					putc ('\n', stderr);
				}

				/* make the link */
				if (!nflag) {
					struct stat sb, sb2;

					/* are we about to link x to x? */
					if (stat (path, &sb) < 0
					    || stat (path2, &sb2) < 0
					    || sb.st_dev != sb2.st_dev
					    || sb.st_ino != sb2.st_ino) {
						rmall (path2);
						if (link (path, path2) < 0)
							perror (path2);
					}
				}

				free (path);
				break;
			}
			break;
		
		/* remove */
		case 'r':
			/* get parameters */
			path = transname (getfield (tf));
			geteol (tf);

			switch (code) {

			case BACKUP:
				if (!subsumed (path))
					pkgfile (path);
				break;

			case INSTALL:
				if (vflag) {
					fprintf (stderr, "remove file ");
					putpath (stderr, path);
					putc ('\n', stderr);
				}
				if (!nflag)
					rmall (path);
				break;
			}

			break;

		default:
			fprintf (stderr, "inspkg: invalid package\n");
			delete();
			exit (1);
		}
	}
	fclose (tf);

}

static void
delete()
{
	unlink (tfname);
	exit (3);
}
