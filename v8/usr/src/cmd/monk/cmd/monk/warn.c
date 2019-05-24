#include	<stdio.h>
#include	"warn.h"

struct missing {
	char	*name;
	char	*filename;
	short	linenumber;
	struct missing	*next;
};

struct filestack	*filestack;
struct missing		*missingstack;

struct missing	*add_missing(), *known_missing();
char	*mymalloc();

FILE *
fopenncheck(filename, mode, exit_on_fail)
char	*filename, *mode;
int	exit_on_fail;
{
	FILE	*file;
	struct filestack	*temp;

	if ((file = fopen(filename, mode)) == NULL) {
		fprintf(stderr, "fopenncheck: cannot open %s file\n", filename);
		if (exit_on_fail)
			exit(exit_on_fail);
		return((FILE *) NULL);
	}
	/* track filenames and line numbers for error messages */
	if (filestack == (struct filestack *) 0) {
		filestack = (struct filestack *)
					mymalloc(sizeof(struct filestack));
		filestack->previous = (struct filestack *) 0;
	} else {
		temp = (struct filestack *) mymalloc(sizeof(struct filestack));
		temp->previous = filestack;
		filestack = temp;
	}
	/* toss path and keep only filename?? */
	filestack->name = mymalloc(strlen(filename)+1);
	strcpy(filestack->name, filename);
	filestack->line_number = 1;
	return(file);
}

fclosencheck(file)
FILE	*file;
{
	if (fclose(file) == EOF)
		warn_me(PR_FILENAME | PR_LINENUMBER, "fclosencheck: cannot close");
	if (filestack == (struct filestack *) 0) {
		warn_me(PR_FILENAME | PR_LINENUMBER,
				"fclosencheck: no file to pop from filestack");
		return;
	}
	/* do not free name;	myfree(filestack->name, 16);
		environments have pointers to their origin filename */	
	myfree(filestack, 17);
	filestack = filestack->previous;
}

struct file_info *
save_file_info()
{
	static struct file_info	fi;

	fi.file_name = filestack->name;
	fi.line_number = filestack->line_number;
	return(&fi);
}

char	*
get_file_name()
{
	if (filestack == (struct filestack *) 0)
		return(":monk");
	return(filestack->name);
}

get_line_number()
{
	if (filestack == (struct filestack *) 0)
		return(0);
	return(filestack->line_number);
}

missing_def(token)
char	*token;
{
	struct missing	*m, *lastm;

	if (missingstack == (struct missing *) 0) {
		missingstack = add_missing(token, (struct missing *) 0);
		return;
	}
	if ((m = known_missing(token, missingstack)) == (struct missing *) 0)
		return;
	lastm = add_missing(token, m);
}

/*	if token to be added to missingstack, returns last structure;
	if token is known to be missing, returns (struct missing *) 0
 */

struct missing *
known_missing(token, m)
char	*token;
struct missing	*m;
{
	if (m != (struct missing *) 0)
		for (;; m = m->next) {
			if (strcmp(m->name, token) == 0)
				return((struct missing *) 0);
			if (m->next == (struct missing *) 0)
				break;
		}
	return(m);
}

struct missing *
add_missing(token, m)
char	*token;
struct missing	*m;
{
	struct file_info	*fi;

	if (m == (struct missing *) 0)
		m = (struct missing *) mymalloc(sizeof(struct missing));
	else {
		m->next = (struct missing *) mymalloc(sizeof(struct missing));
		m = m->next;
	}
	m->name = mymalloc(strlen(token)+1);
	strcpy(m->name, token);
	fi = save_file_info();
	m->filename = mymalloc(strlen(fi->file_name)+1);
	strcpy(m->filename, fi->file_name);
	m->linenumber = fi->line_number;
	m->next = (struct missing *) 0;
	return(m);
}

checkifmissing()
{
	struct missing	*m;

	if ((m = missingstack) != (struct missing *) 0) {
		do {
			if (isdefined(m->name) == 0)
				warn_db(0,
				"%s: line %d: No definition found for %s\n",
				m->filename, m->linenumber, m->name);
		} while ((m = m->next) != (struct missing *) 0);
		free_missing(missingstack);
		missingstack = (struct missing *) 0;
	}
}

free_missing(m)
struct missing	*m;
{
	if (m != (struct missing *) 0)
		do {
			myfree(m->name, 44);
			myfree(m->filename, 45);
			myfree(m, 46);
		} while ((m = m->next) != (struct missing *) 0);
}

/* warn_user:
	spew message onto stderr and exit if exit == nonzero
 */

warn_user(mode, format_string, arg1, arg2, arg3, arg4, arg5)
short	mode;
char	*format_string;
{
	char	c;

	if (filestack != (struct filestack *) 0) {
		if (mode & PR_FILENAME)
			fprintf(stderr, "%s: ", filestack->name);
		if (mode & PR_LINENUMBER)
			fprintf(stderr, "line %d: ", filestack->line_number);
	}
	fprintf(stderr, format_string, arg1, arg2, arg3, arg4, arg5);
}

warn_db(mode, format_string, arg1, arg2, arg3, arg4, arg5)
short	mode;
char	*format_string;
{
	static FILE	*warn_db;

#ifdef PRODUCTION
	char	*filename;

	if (warn_db == (FILE *) 0) {
		filename = mymalloc(strlen(WARN_DB)+10);
		sprintf(filename, "%s%d", WARN_DB, getuid());
		if ((warn_db = fopen(filename, "w")) == NULL) {
			fprintf(stderr, "warn: cant open %s file for writing\n",
						filename);
			exit(-3);
		}
		setbuf(warn_db, NULL);
		myfree(filename, 18);
	}
#else
	warn_db = stderr;
#endif
	if (filestack != (struct filestack *) 0) {
		if (mode & PR_FILENAME)
			fprintf(warn_db, "%s: ", filestack->name);
		if (mode & PR_LINENUMBER)
			fprintf(warn_db, "line %d: ", filestack->line_number);
	}
	fprintf(warn_db, format_string, arg1, arg2, arg3, arg4, arg5);
}

warn_me(mode, format_string, arg1, arg2, arg3, arg4, arg5)
short	mode;
char	*format_string;
{
	static FILE	*warn_me;
	char	*filename;

	if (warn_me == (FILE *) 0) {
		filename = mymalloc(strlen(WARN_ME)+10);
		sprintf(filename, "%s%d", WARN_ME, getuid());
		if ((warn_me = fopen(filename, "w")) == NULL) {
			fprintf(stderr, "warn: cannot open %s file for writing\n",
						filename);
			exit(-3);
		}
		setbuf(warn_me, NULL);
		myfree(filename, 19);
	}
	if (filestack != (struct filestack *) 0) {
		if (mode & PR_FILENAME)
			fprintf(warn_me, "%s: ", filestack->name);
		if (mode & PR_LINENUMBER)
			fprintf(warn_me, "line %d: ", filestack->line_number);
	}
	fprintf(warn_me, format_string, arg1, arg2, arg3, arg4, arg5);
}

mygetc(file)
FILE	*file;
{
	int	c;

	if ((c = getc(file)) == '\n')
		filestack->line_number++;
	return(c);
}

myungetc(c, file)
int	c;
FILE	*file;
{
	if (c != EOF) {
		if (c == '\n')
			filestack->line_number--;
		c = ungetc(c, file);
	}
	return(c);
}

linenumber(change)
int	change;
{
	if (change == INCREMENT)
		filestack->line_number++;
	else
		filestack->line_number--;
}

/* used when isitaloop fails - must change: this won't work on stream input */
myungets(s, file)
char	*s;
FILE	*file;
{
	char	*reverse;

	for (reverse = s + strlen(s); --reverse >= s; )
		myungetc(*reverse, file);
}

wrong_delim(key, close_delim, c)
char	*key, close_delim;
int	c;
{
	warn_user(PR_FILENAME | PR_LINENUMBER,
				"Mismatched delimiters around %s;", key);
	if (c == EOF)
		warn_user(PR_NOTHING, "expecting %c, got EOF\n", close_delim);
	else
		warn_user(PR_NOTHING, "expecting %c, got %c\n", close_delim, c);
}
