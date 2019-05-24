#include <ctype.h>

/* Get the next token in a string, returning a pointer the the byte
 * following the token.
 */
char *
in_getw(buf, w)
char *buf, *w;
{
	*w = 0;
	while(isspace(*buf)) buf++;
	if(*buf == '\0')
		return(0);
	while(!isspace(*buf) && *buf)
		*w++ = *buf++;
	*w++ = 0;
	return(buf);
}

