#include "asd.h"
#include "ftw.h"

FILE *tf;
static Sig_typ sigsav;
static int rc;
static char tfname[TMPNAML];

/*
 *	Prepare to build a package.  The package will appear on stdout.
 *	The argument is nonzero if remarks are to be read.
 */

void
pkgstart()
{
	rc = 0;

	nchk (fstat (fileno (stdout), &outsb));

	/* establish a temporary file to hold the instruction information */
	tmpname (tfname);
	tf = fopen (tfname, "w");
	sigsav = signal (SIGINT, SIG_IGN);
	if (sigsav != SIG_IGN)
		signal (SIGINT, delete);
	nchk (chmod (tfname, 0644));
	schk ((char *) tf);

	/* create the initial element in the component chain */
	pkhead = pktail = new (struct pack);
	pkhead->iname = copy (instr);
	pkhead->ename = copy (tfname);
	pkhead->time = 0;
	pkhead->link = NULL;
	pkhead->uid = getuid();
	pkhead->gid = getgid();
	pkhead->mode = 0100644;
}

/*
 *	put a file (or directory) into a package
 */
void
pkgfile (file)
	register char *file;
{
	register int x;

	/* if the file is truly not present, generate a remove request */
	if (access (file, 0) == -1 && errno == ENOENT) {
		fprintf (tf, "r ");
		putpath (tf, transname (file));
		putc ('\n', tf);
	} else {
		x = ftw (file, consider, 8);
		if (x != 0) {
			rc++;
			if (x == -1)
				perror (file);
		}
	}
}

/*
 *	we are done building a package.  This writes the actual file
 *	contents, so make sure the files are still around
 */
int
pkgend()
{
	register struct pack *pack;
	struct stat s;

	/* we now know how long the first component is */
	pkhead->size = ftell (tf);

	/* we no longer need to write the temporary file */
	fclose (tf);

	/* describe the temp file correctly so it will pass later checks */
	if (pkhead->time == 0)
		(void) time (&pkhead->time);
	nchk (stat (tfname, &s));
	pkhead->dev = s.st_dev;
	pkhead->ino = s.st_ino;

	/*
	 * write the files out into an archive
	 */

	/* first the archive header */
	printf (ARMAG);

	/*
	 *	run through the list, creating the archive components
	 *	and deleting the list entries.  One iteration per component.
	 *	We know there is at least one component, because the
	 *	"Instructions" component must be represented.
	 */
	do {
		struct ar_hdr ah;
		char buf[30];
		register int c;

		/* "pack" is the package component under consideration */
		pack = pkhead;

		/* log it if requested */
		if (vflag)
			fprintf (stderr, "package %s\n",
			    strcmp (pack->iname, instr)? pack->ename: instr);

		/* write the archive element header */

#		define ent(a,x) sprintf(buf, "%-*s", sizeof(ah.a), x); \
		strncpy (ah.a, buf, sizeof (ah.a))
		ent (ar_name, pack->iname);
		ent (ar_fmag, ARFMAG);
#		undef ent

#		define ent(a,x) sprintf(buf, "%*ld", sizeof(ah.a), (long) x); \
		strncpy (ah.a, buf, sizeof (ah.a))
		ent (ar_date, pack->time);
		ent (ar_uid, pack->uid);
		ent (ar_gid, pack->gid);
		ent (ar_size, pack->size);
#		undef ent

#		define ent(a,x) sprintf(buf, "%*o", sizeof(ah.a), x); \
		strncpy (ah.a, buf, sizeof (ah.a))
		ent (ar_mode, pack->mode);
#		undef ent

		fwrite ((char *) &ah, sizeof (ah), 1, stdout);

		/* write the archive element itself */
		tf = fopen (pack->ename, "r");
		if (tf == NULL) {
			perror (pack->iname);
			exit (1);
		}

		while ((c = getc (tf)) != EOF)
			putchar (c);
		
		/* if things now don't match, complain */
		if (fstat (fileno (tf), &s) < 0 || s.st_size != pack->size ||
		    (s.st_mtime != pack->time && strcmp (pack->iname, instr)) ||
		    s.st_dev != pack->dev || s.st_ino != pack->ino ||
		    s.st_uid != pack->uid || s.st_gid != pack->gid ||
		    (s.st_mode & 07777) != (pack->mode & 07777)) {
			fprintf (stderr, "phase error on %s\n",
			    pack->ename);
			rc++;
		}

		fclose (tf);

		if (pack->size & 1)
			putchar ('\n');
		
		/* delete the element, move on to the next */
		free (pack->iname);
		free (pack->ename);
		pkhead = pack->link;
		free ((char *) pack);
	} while (pkhead);

	/* zap the tail pointer for general cleanliness */
	pktail = NULL;

	nchk (unlink (tfname));
	signal (SIGINT, sigsav);

	return rc;
}

/* internal function for package creation */
static int
consider (name, buf, type)
	register char *name;
	register struct stat *buf;
	register int type;
{
	register struct pack *pack;
	register int mode;

	switch (type) {
	case FTW_D:
		fprintf (tf, "d %-*.4o ", MAXCOMP, buf->st_mode & 07777);
		hdrsub (name, buf);
		fprintf (tf, "\n");
		break;

	case FTW_F:
		/* is it a special file? */
		mode = buf->st_mode & S_IFMT;
		if (mode == S_IFBLK || mode == S_IFCHR) {
			fprintf (tf, "%c %#o %d %d ",
			    mode == S_IFBLK? 'b': 'c',
			    buf->st_mode & 07777,
			    major (buf->st_rdev),
			    minor (buf->st_rdev));
			hdrsub (name, buf);
			fprintf (tf, "\n");
			break;
		}

		if (mode != S_IFREG) {
			fprintf (stderr, "%s: unrecognized file type\n",
			    fullname (name));
			return 0;
		}

		/* refuse to package the standard output */
		if (buf->st_dev == outsb.st_dev &&
		    buf->st_ino == outsb.st_ino) {
			fprintf (stderr, "skipping output file %s\n",
			    fullname (name));
			return 0;
		}

		/* Has this file already appeared?  If so, it's a link */
		for (pack = pkhead->link; pack; pack = pack->link) {
			if (buf->st_dev == pack->dev &&
			    buf->st_ino == pack->ino) {
				fprintf (tf, "l %s ", transname (pack->ename));
				fprintf (tf, "%s\n", transname (name));
				return 0;
			}
		}

		/* package the file */
		pack = new (struct pack);
		pack->ename = copy (name);
		pack->dev = buf->st_dev;
		pack->ino = buf->st_ino;
		pack->uid = buf->st_uid;
		pack->gid = buf->st_gid;
		pack->time = buf->st_mtime;
		if (pack->time > pkhead->time)
			pkhead->time = pack->time;
		pack->size = buf->st_size;
		pack->mode = buf->st_mode;
		pack->link = NULL;
		pack->iname = iname (pack->ename);
		pktail->link = pack;
		pktail = pack;
		fprintf (tf, "f ");
		putpath (tf, pack->iname);
		fprintf (tf, " ");
		hdrsub (name, buf);
		fprintf (tf, "\n");
		break;

	case FTW_DNR:
		fprintf (stderr, "cannot read directory %s\n", name);
		return 1;

	case FTW_NS:
		fprintf (stderr, "cannot stat %s\n", name);
		return 1;
	
	case FTW_DP:
		break;

	default:
		fprintf (stderr, "impossible code %d from ftw\n", type);
		exit (1);
	}
	return 0;
}

static
hdrsub (name, buf)
	register char *name;
	register struct stat *buf;
{
	putpath (tf, struid (buf->st_uid));
	putc ('\t', tf);
	putpath (tf, strgid (buf->st_gid));
	putc ('\t', tf);
	putpath (tf, transname (name));
}

/*
 *	generate a unique internal name for a file
 */
static char *
iname (s)
	char *s;
{
	register char *p;
	register char *lastcomp;
	register struct pack *pack;
	char trial[MAXCOMP+1];

	/* point lastcomp at the last pathname component */
	lastcomp = s;
	for (p = s; *p; p++) {
		if (*p == '/')
			lastcomp = p + 1;
	}

	/* if the name is acceptably short, modify it slightly */
	if (strlen (lastcomp) <= MAXCOMP) {

		char prefix[MAXCOMP+1], suffix[MAXCOMP+1], num[30];
		register int n;

		/* split the name, remove unprintables */
		strcpy (prefix, lastcomp);
		for (p = prefix; *p; p++)
			if (*p == ' ' || !isprint (*p))
				*p = '$';
		suffix[0] = '\0';
		p = rindex (prefix, '.');
		if (p != NULL) {
			strcpy (suffix, p);
			*p = '\0';
		}

		/* generate trial names until we run out of space */
		for (n = 0, num[0] = '\0';
		    strlen(prefix) + strlen(suffix) + strlen(num) <= MAXCOMP;
		    n++, sprintf (num, "%.0d", n)) {
			
			/* generate the trial name */
			strcpy (trial, prefix);
			strcat (trial, num);
			strcat (trial, suffix);

			/* if the name is unique, we're done */
			pack = pkhead;
			while (pack != NULL && strcmp (pack->iname, trial) != 0)
				pack = pack->link;
			if (pack == NULL)
				return copy(trial);

		}
	}

	/* punt -- generate a completely new name */
	do {
		static int tempno;

		tempno++;
		sprintf (trial, "Temp%d", tempno);
		pack = pkhead;
		while (pack != NULL && strcmp (trial, pack->iname) != 0)
			pack = pack->link;
	} while (pack != NULL);
	return copy(trial);
}

static void
delete()
{
	unlink (tfname);
	exit (3);
}
