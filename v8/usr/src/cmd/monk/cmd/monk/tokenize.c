#include	<stdio.h>
#include	<ctype.h>
			/* should split rd.h, rather than include this here */
#include	"search.h"
#include	"rd.h"

char *
read_key(database, delimiter)
FILE	*database;
char	delimiter;
{
	int	c;

	while (((c = mygetc(database)) != EOF) && isspace(c));
	myungetc(c, database);
	/*  really should leave no trailing white space */
	return(read_until(database, delimiter));
}

char *
read_until(database, delimiter)
FILE	*database;
char	delimiter;
{
	register char	*p;
	char	*key;
	char	temp[SAFESIZ];

	for (p=temp; ((*p = mygetc(database)) != EOF) && *p != delimiter; ++p);
	if (*p == EOF)
		return((char *) EOF);
	if (p == temp)
		return((char *) 0);
	*p = '\0';
	key = mymalloc(strlen(temp)+1);
	strcpy(key, temp);
	return(key);
}

char *
read_token(file, space)
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
	token = mymalloc(strlen(temp)+1);
	strcpy(token, temp);
	return(token);
}

/* Really need one read_token with anm argument selecting character set */
char *
read_filename(file, space)
FILE	*file;
int	space;
{
	register char	*p;
	char	*token;
	char	temp[BUFSIZ];

	p = temp;
	if (space == OK_SPACE) {	
		while ((*p = mygetc(file)) != EOF && isspace(*p));
		while (*p != EOF && isfilenamechar(*p))
			 *++p = mygetc(file);

	} else
		while ((*p = mygetc(file)) != EOF && isfilenamechar(*p))
			 ++p;
	/* return EOF only if read EOF as first character */
	if (*temp == EOF)
		return((char *) EOF);
	myungetc(*p, file);
	if (p == temp)
		return((char *) 0);
	*p = '\0';
	token = mymalloc(strlen(temp)+1);
	strcpy(token, temp);
	return(token);
}

char *
read_group(file)
FILE	*file;
{
	register char	*p;
	char	*token;
	char	temp[BUFSIZ];


	for (p = temp;
		((*p = mygetc(file)) != EOF) && isspace(*p););
	myungetc(*p, file);
	for (p=temp; ((*p=mygetc(file)) != EOF)&&(isargchar(*p)||isspace(*p));++p);
	if (*p == EOF)
		return((char *) EOF);
	myungetc(*p, file);
	if (p == temp)
		return((char *) 0);
	*p = '\0';
	trimwhite(temp, WITHIN_WHITE | TRAIL_WHITE);
	token = mymalloc(strlen(temp)+1);
	strcpy(token, temp);
	return(token);
}

char *
read_arg(file, delimiters)
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
	token = mymalloc(strlen(p)+1);
	strcpy(token, p);
	return(token);
}

char
read_nonblank(database)
FILE	*database;
{
	char	c;

	while ((c = mygetc(database)) != EOF && isspace(c));
	return(c);
}

/* read_delim:
	from paired definitions of delimiters;
	return close character corresponding to one read
		should not gobble \n --- tosses too much of enduser
		white space. e.g what if
|p
(i)
	looks like this would be == |p(i), which seems to crash with too
	many reasonable possibilities --- is this really wanted for db reads?
	also
|p
	can i remove initial white, or coordinate with other routines
		-- should read_user handle this extra white or here
 */

char
read_delim(file)
FILE	*file;
{
	char	open_delim, close_delim;

	while(((open_delim = mygetc(file)) != EOF) && isspace(open_delim));
	if (open_delim == EOF)
		return(EOF);
	if ((close_delim = close_match(open_delim)) == (char) FALSE)
		myungetc(open_delim, file);
	return(close_delim);
}

char
read_userdelim(file)
FILE	*file;
{
	char	open_delim, close_delim;

	/* do not gobble '\n' looking for delimiter in usertext */
	while(((open_delim = mygetc(file)) != EOF) && iswhite(open_delim));
	if (open_delim == EOF)
		return(EOF);
	if ((close_delim = close_match(open_delim)) == (char) FALSE)
		myungetc(open_delim, file);
	return(close_delim);
}

char
close_match(c)
char	c;
{
	switch (c) {
		case '(':
			return(')');
		case '[':
			return(']');
		case '{':
			return('}');
		case '<':
			return('>');
		case '\"':
			return('\"');
		case '\'':
			return('\'');
		case '\`':
			return('\'');
		default:
			return((char) FALSE);
	}
}

char
open_match(c)
char	c;
{
	switch (c) {
		case ')':
			return('(');
		case ']':
			return('[');
		case '}':
			return('{');
		case '>':
			return('<');
		case '\"':
			return('\"');
		case '\'':
			return('\`');
		default:
			return((char) FALSE);
	}
}

/* return structure as ftoken does ??
			static struct got_token	gt;
			gt->delimiter = *p++;
			gt->string = p;
			return(&gt)
 */

mindex(string, searchset)
char	*string, *searchset;
{
	register char	*p, *q;

	if (string = (char *) 0)
		return(0);
	for (p=string; *p != '\0'; ++p) {
		for (q=searchset; *q != '\0' && *q != *p; ++q)
			;
		if (*q != '\0')
			return(p-string);
	}
	return(0);
}

isokchar(c)
char	c;
{
	if (isalnum(c) || c == '_' || c == '.' || c == '$' || c == ':')
		return(TRUE);
	else
		return(FALSE);
}

isargchar(c)
char	c;
{
	if (isalnum(c) || c == '_' || c == '.' || c == '$' || c == '\\'
		|| c == '(' || c == '+' || c == '-' || c == '|' || c == '*')
		return(TRUE);
	else
		return(FALSE);
}

isfilenamechar(c)
char	c;
{
	if (isalnum(c) || c == '_' || c == '.' || c == '$'
				|| c == '+' || c == '-' || c == '/')
		return(TRUE);
	else
		return(FALSE);
}


isblank(c)
char	c;
{
	if (isspace(c) && c != '\n')
		return(TRUE);
	else
		return(FALSE);
}

/* trimwhite: mode select the white space to go
		LEAD_WHITE: trim leading white
		WITHIN_WHITE: convert groups of internal white to single blanks
		TRAIL_WHITE: trim trailing white
 */

trimwhite(start, mode)
char	*start;
short	mode;
{
	register char	*p, *q;
	int	length;

	if (mode & LEAD_WHITE) {
		for (p = start; isspace(*p); ++p);
		if (p != start)
			strcpy(start, p);
	}
	if (mode & WITHIN_WHITE) {
		for (p=start; *p != '\0'; ++p)
			if (isspace(*p)) {
				*p++ = ' ';
				for (q = p; isspace(*q); ++q);
				strcpy(p, q);
			}
	}
	if (mode & TRAIL_WHITE) {
		for (p = start + strlen(start)-1; p >= start && isspace(*p); --p);
		if (p >= start)
			*++p = '\0';
	}
}

struct strings *
strtok(s)
char	*s;
{
	register char	*p;
	static struct strings	str;
	int	length;
	char	*token;
	
	for (; *s != '\0' && *s != '\n' && isspace(*s); ++s);
	if (*s == '\n')
		return((struct strings *) 0);
	for (p = s; *p != '\0' && isokchar(*p); ++p);
	if ((length = p - s) == 0)
		return((struct strings *) 0);
	str.newp = p;
	str.token = mymalloc(length+1);
	strncpy(str.token, s, length);
	str.token[length] = '\0';
	return(&str);
}

char	*
strindex(string, segment)
char	*string, *segment;
{
	short	lenstr, lenseg;

	lenstr = strlen(string);
	lenseg = strlen(segment);
	if (lenstr == 0 || lenseg == 0 || lenstr < lenseg)
		return((char *) 0);
	for (; lenstr-- >= lenseg; ++string)
		if (strncmp(string, segment, lenseg) == 0)
			return(string);
	return((char *) 0);
}

char	*
mk_string(c1, c2)
char	c1, c2;
{
	register char	*p;
	static char	string[BUFSIZ];

/* should accept variable arguments */
	p = string;
	*p++ = c1;
	*p++ = c2;
	*p = '\0';
	return(string);
}

char	*
strconcat(s1, s2)
char	*s1, *s2;
{
	char	*new;

	/* must return copy even if one string is null */
	new = mymalloc(strlen(s1) + strlen(s2) + 1);
	strcpy(new, s1);
	strcat(new, s2);
	return(new);
}
	
char	*
strjoin(s1, s2)
char	*s1, *s2;
{
	short	len1, len2;
	char	*new;

	len1 = (s1 == (char *) 0) ? 0 : strlen(s1);
	len2 = (s2 == (char *) 0) ? 0 : strlen(s2);
	new = mymalloc(len1 + len2 + 2);
	if (len1) {
		strcpy(new, s1);
		if (len2) {
			strcat(new, BLANKSTRING);
			strcat(new, s2);
		}
	} else
		if (len2)
			strcpy(new, s2);
		else
			/* both null -> return null string or (char *) 0? */
			*new = '\0';
	return(new);
}

char *
mymalloc(size)
unsigned	size;
{
	char	*malloc();
	char	*p;

	if ((p = malloc(size)) == (char *) 0)
		fprintf(stderr, "Malloc: cannot allocate %d bytes\n", size);
	return(p);
}

char *
myrealloc(p, size)
char	*p;
unsigned	size;
{
	char	*realloc();

	char	*newp;

	if ((newp = realloc(p, size)) == (char *) 0) {
		fprintf(stderr, "Myrealloc: cannot reloc %o for %d bytes\n",
							p, size);
		return((char *) 0);
	}
	return(newp);
}

myfree(pointer, number)
char	*pointer;
short	number;
{
	if (pointer == (char *) 0 || pointer == (char *) EOF) {
		fprintf(stderr, "Free %d: no block to free\n", number);
		return;
	}
	free(pointer);
}
