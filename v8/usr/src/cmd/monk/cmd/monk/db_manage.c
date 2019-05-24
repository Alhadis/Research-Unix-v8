#include	<stdio.h>
#include	<ctype.h>
#include	"search.h"
#include	"warn.h"
#include	"rd.h"
#include	"dbcompress.h"

char *
bufmalloc(buf_number, size)
int	buf_number;
unsigned	size;
{
	char	*malloc();
	char	*buf, *p;

#ifndef COMPRESS
	if ((p = malloc(size)) == (char *) 0)
		fprintf(stderr, "Malloc: cannot allocate %d bytes\n", size);
	return(p);
#else

	extern struct db_buffer	db_struct[BUF_NUMBER];

	if (buf_number < 0 || buf_number > BUF_NUMBER) {
		fprintf(stderr,
			"Buffer %d requested: only %d buffers available\n",
						buf_number, BUF_NUMBER);
		return((char *) 0);
	}
	if (db_struct[buf_number].top == (char *) 0) {
		/* ZZZZZ - need to realloc, fixing pointers (sub and fix)
			when space buffer all used up */
	/* Health: 4 needs to be 6 on 5.0
			--- need to convert so buffers grow */
		buf = mymalloc(SAFESIZ*5);
		db_struct[buf_number].top = db_struct[buf_number].current = buf;
		db_struct[buf_number].bottom = buf + SAFESIZ*5;
	} else
		buf = db_struct[buf_number].current;
	if ((db_struct[buf_number].current += size) <
						db_struct[buf_number].bottom)
		return(buf);
	else {	int buf_size[BUF_NUMBER];
		fprintf(stderr, "Buffer %d out of space\n", buf_number);
		for (buf_number = 0; buf_number < BUF_NUMBER; ++buf_number) {
			buf_size[buf_number] = db_struct[buf_number].current
						- db_struct[buf_number].top;
			fprintf(stderr, "\nBuffer %d: size %d of %d, ",
				buf_number, buf_size[buf_number],
				db_struct[buf_number].bottom
						- db_struct[buf_number].top);
			fprintf(stderr, "requested %d", size);
		}
			fprintf(stderr, "\n");
		exit(-1);
	}
#endif
}


buffree(pointer, number)
char	*pointer;
int	number;
{
#ifndef COMPRESS
	if (pointer == (char *) 0 || pointer == (char *) EOF) {
		fprintf(stderr, "Free %d: no block to free\n", number);
		return;
	}
	free(pointer);
#endif
}

char *
read_buftoken(file, space)
FILE	*file;
int	space;
{
	register char	*p;
	char	*token;
	char	temp[BUFSIZ];

	p = temp;
	if (space == OK_SPACE) {	
		while ((*p = mygetc(file)) != EOF && isspace(*p));
		while (*p != EOF && isokchar(*p))
			 *++p = mygetc(file);

	} else
		while ((*p = mygetc(file)) != EOF && isokchar(*p))
			 ++p;
	/* return EOF only if read EOF as first character */
	if (*temp == EOF)
		return((char *) EOF);
	myungetc(*p, file);
	if (p == temp)
		return((char *) 0);
	*p = '\0';
	token = bufmalloc(BUF_TEXT, strlen(temp)+1);
	strcpy(token, temp);
	return(token);
}

char *
read_bufarg(file, delimiters)
FILE	*file;
short	delimiters;
{
	register char	*p;
	char	close_delim;
	char	*token;
	char	temp[SAFESIZ];
	
	for (p = temp; (*p = mygetc(file)) != EOF && isspace(*p););
	myungetc(*p, file);
	if (delimiters!=TRUE || (close_delim = read_delim(file))==(char) FALSE){
		while ((*p = mygetc(file)) != EOF && isargchar(*p))
		 	++p;
		if (*p == EOF)
			return((char *) EOF);
		if (!isspace(*p))
			myungetc(*p, file);
		if (p == temp)
			return((char *) 0);
		*p = '\0';
		p = temp;
	} else {
		if ((p = read_until(file, close_delim)) == (char *) EOF)
			return((char *) EOF);
		if (p == (char *) 0)
			return((char *) 0);
	}
	token = bufmalloc(BUF_TEXT, strlen(p)+1);
	strcpy(token, p);
	return(token);
}

struct strings *
strarg(s, delimiters)
register char	*s;
short	delimiters;
{
	static struct strings	str;
	int	length;
	char	close_delim;
	char	*arg, *token;
	
	for (; *s != '\0' && isblank(*s); ++s);
	arg = s;
	if (delimiters != TRUE ||
			(close_delim = close_match(*s)) == (char) FALSE)
		for (;*s != '\0' && isargchar(*s); ++s);
	else
		for (;*s != '\0' && *s != close_delim; ++s);
	if ((length = s - arg) == 0)
		return((struct strings *) 0);
	str.newp = s;
	str.token = bufmalloc(BUF_TEXT, length+1);
	strncpy(str.token, arg, length);
	str.token[length] = '\0';
	return(&str);
}

void
read_macro(database, name, database_path, close_delim)
FILE	*database;
char	*name, *database_path;
int	close_delim;
{
	static FILE	*macrofile;
	int	c, inner_close;

	if ((inner_close = read_delim(database)) == (char) FALSE) {
		warn_db(PR_FILENAME | PR_LINENUMBER,
			"Improper opening delimiter for %s %s\n", MACRO, name);
		return;
	}
#ifndef COMPRESS
	while (((c = mygetc(database)) != EOF) && c != inner_close)
		textputc(c, MONK_DB);
#else
	if (macrofile == NULL)
		macrofile=fopendb(MACRO_COMP, database_path, "w", EXIT_ON_FAIL);
	while (((c = mygetc(database)) != EOF) && c != inner_close)
		fputc(c, macrofile);
#endif
	if ((c = read_nonblank(database)) != close_delim)
		warn_db(PR_FILENAME | PR_LINENUMBER,
			"Improper end delimiter for %s %s\n", MACRO, name);
}
