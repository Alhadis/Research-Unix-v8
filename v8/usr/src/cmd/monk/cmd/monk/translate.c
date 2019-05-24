#include	<stdio.h>
#include	<ctype.h>
#include	"search.h"
#include	"warn.h"
#include	"dbcompress.h"
#include	"rd.h"

char	*
trans_state(s)
struct state	*s;
{
	static struct def_element	d;

	d.attribute = s->attribute;
	d.value = s->value;
	if (d.allocated == TRUE)
		myfree(d.troff, 20);
	translate(&d, RUN_TIME);
	return(d.troff);
}

translate(d, time_of_call)
struct def_element	*d;
int	time_of_call;
{
	ENTRY	*grabit;
	struct attribute_case	*attcase;

	/* d->attribute == (char *) 0 */
	d->troff = (char *) 0;
	if (*d->attribute == '\0')
		return(FALSE);
	if ((grabit = hashfind(ATT_TABLE, d->attribute)) == (ENTRY *) NULL)
		return(FALSE);
	/* find matching case from linked list of cases */
	if ((attcase = match_case(grabit, d->value)) ==
						(struct attribute_case *) 0)
		return(FALSE);
	d->special = attcase->special;
	/* specials, __LINE__ and __FILE__ get replaced only at run-time */
	if (time_of_call == SETUP_TIME)
		d->troff = replace_args(attcase->troff, FALSE,
					d->value, (struct environment *) 0);
	else
		d->troff = replace_args(attcase->troff, attcase->special,
					d->value, (struct environment *) 0);
	if (d->troff != attcase->troff)
		attcase->allocated = TRUE;
#ifdef DEBUG_ENVIRONMENT
	warn_me(0, "--------------------Akey: `%s'\n", grabit->key);
	if (d->troff != attcase->troff)
		warn_me(0, "Filled ");
	else
		warn_me(0, "Unmodified ");
	warn_me(0, "troff: `%s'\n", d->troff);
#endif
	return(TRUE);
}

/* find arguments listed from value in data from entry */
struct attribute_case *
match_case(entry, value)
ENTRY	*entry;
struct value	*value;
{
	struct attribute_info	*allcase;
	struct attribute_case	*thiscase;
	struct value	*matchthese;
	short	match, nvalues;

	allcase = (struct attribute_info *) entry->data;
	if ((thiscase = allcase->firstcase) == (struct attribute_case *) 0)
		return((struct attribute_case *) 0);
	if (value == (struct value *) 0) {
		if (thiscase->value == (struct value *) 0)
			return(thiscase);
		else
			return((struct attribute_case *) 0);
	}
	for (nvalues = 0, matchthese = value; value != (struct value *) 0;
							value = value->next)
		++nvalues;
	do {
		if (thiscase->expanding == FALSE) {
			if (nvalues == thiscase->nvalues &&
			(match=match_arg(matchthese, thiscase->value, nvalues))
									== TRUE)
					break;
		} else
			if ((match=xpand_arg(matchthese,nvalues,thiscase)) == TRUE)
				break;
	} while ((thiscase = thiscase->next) != (struct attribute_case *) 0);
	/* if there is a match, do a substitution */
	if (match == TRUE)
		return(thiscase);
	return((struct attribute_case *) 0);
}

short
match_arg(matchthese, againstthese, number)
struct value	*matchthese, *againstthese;
short	number;
{
	if (matchthese == (struct value *) 0 ||
					againstthese == (struct value *) 0)
		if (matchthese == againstthese)
			return(TRUE);
		else
			return(FALSE);
	while (matchthese != NULL && againstthese != NULL && --number >= 0) {
		if (match_onearg(matchthese, againstthese) == FALSE)
			return(FALSE);
		matchthese = matchthese->next;
		againstthese = againstthese->next;
	}
	if (number < 0)
		return(TRUE);
	if (matchthese == againstthese)
		return(TRUE);
	else
		return(FALSE);
}

match_onearg(matchthis, againstthis)
struct value	*matchthis, *againstthis;
{
	char	*regex();
	extern char	*loc1;
	char	*p;
	char	matched[BUFSIZ];

	if (againstthis->type == FIXED_ARG) {
		if (strcmp(matchthis->value, againstthis->value) != 0)
			return(FALSE);
		matchthis->type = FIXED_ARG;
	} else
		if (againstthis->type == REGEX_ARG) {
			/* the entire argument must be matched by pattern
			   test if global loc1 points to start of matchthis;
				and if regex returns pointer to end */
			if ((p=regex(againstthis->value, matchthis->value,
							matched)) == 0)
				return(FALSE);
/* how to check where matched string begins; loc1 seems not to be defined and
	nothing interesting returned in matched....
			fprintf(stderr, "Regex match: matched %s; ", matched);
			fprintf(stderr, "from %c to %c; ", *loc1, *p);
			if (strcmp(matched, againstthis->value) != 0)
				return(FALSE);
*/
			if (*p != '\0')
				return(FALSE);
			matchthis->type = REGEX_ARG;
		} else
			matchthis->type = VAR_ARG;
	return(TRUE);
}

xpand_arg(matchthese, nvalues, thiscase)
struct value	*matchthese;
short	nvalues;
struct attribute_case	*thiscase;
{
	struct value	*againstthese;
	short	ndiff, pos;
	char	*p;

	/* expanding argument must match at least one arg in the calling list */
	if ((ndiff = nvalues - thiscase->nvalues) < 0)
		return(FALSE);
	againstthese = thiscase->value;
	if (match_arg(matchthese, againstthese, thiscase->expanding-1) == FALSE)
		return(FALSE);
	for (pos = thiscase->expanding-1; pos--; ) {
		if ((matchthese = matchthese->next) == (struct value *) 0)
			return(FALSE);
		if ((againstthese = againstthese->next) == (struct value *) 0)
			return(FALSE);
	}
	/* check whether againstthese->type == XPAND_ARG ? */
	/* there are ndiff+1 arguments matching the expanding argument */
	if (againstthese->type != XPAND_ARG)
		warn_db(PR_FILENAME | PR_LINENUMBER,
			"Improper definition with expanding argument%d: %s\n",
			thiscase->expanding, str_avalue(thiscase->value, TRUE));
			
	do {
		matchthese->type = XPAND_ARG;
		if ((matchthese = matchthese->next) == (struct value *) 0)
			if (againstthese->next == (struct value *) 0)
				return(TRUE);
			else
				return(FALSE);
	} while (ndiff-- > 0);
	againstthese = againstthese->next;
	pos = thiscase->nvalues - thiscase->expanding;
	if (match_arg(matchthese, againstthese, pos) == FALSE)
		return(FALSE);
	return(TRUE);
}

/* All variable arguments are DEF_ARGUMENT followed by one character,
	1-9 for position variables, a-z for for_loops, * for expanding variables;
   Currently not supporting ${arg}
 */
char	*
whichargs(p, v, loop)
char	*p;
struct value	*v;
struct loop	*loop;
{
	short	n;

	if (isdigit(*p)) {
		for (n=(int) *p-ATOI-1; n-- > 0 && v != (struct value *) 0; )
			if (v->type == XPAND_ARG)
				while ((v = v->next) != (struct value *) 0
					&& v->type == XPAND_ARG);
			else
				v = v->next;
		if (v != (struct value *) 0)
			return(v->value);
		else
			/* error */
			return((char *) 0);
	}
	if (*p == '*')
		return(xpall(v));
	if (loop != (struct loop *) 0 && *p == loop->loopchar)
		return(loop->current->value);
	return((char *) 0);
}

/* Return list of all args matching $* --- when used? */
char	*
xpall(value)
struct value	*value;
{
	static struct value	*lastvalue;
	static char	*lastlist;
	char	temp[BUFSIZ];

	if (value == (struct value *) 0)
		return((char *) 0);
	if (value == lastvalue)
		return(lastlist);
	if (lastlist != (char *) 0)
		myfree(lastlist, 21);
	lastvalue = value;
	*temp = '\0';
	do {
		if (value->type == XPAND_ARG) {
			strcat(temp, BLANKSTRING);
			strcat(temp, value->value);
		}
	} while ((value = value->next) != (struct value *) 0);
	/* copy string into space allocated, omitting leading blank */
	lastlist = mymalloc(strlen(temp));
	strcpy(lastlist, &temp[1]);
	return(lastlist);
}

char	*
replace_args(text, special, values, env)
char	*text;
short	special;
struct value	*values;
struct environment	*env;
{
	struct loop	*loop;
	struct value	*v;
	struct file_info	*fi;
	short	change;
	int	close_delim, ifile, iline, line;
	char	*file;
	register char	*p, *q;
	char	*begloop, *newarg, *pe;
	char	buf[SAFESIZ];

/* Health: check overrun of buf */
	if ((v = values) == (struct value *) 0 && special == FALSE)
		return(text);
	loop = (struct loop *) 0;
	for (change = FALSE, p=text, q=buf, pe=text+strlen(text); p < pe; ) {
		if (*p == DEF_ARGUMENT
			&& (newarg=whichargs(p+1,values,loop))!=(char *)0){
			change = TRUE;
			strcpy(q, newarg);
			q += strlen(newarg);
			/* right now all argument variables are 1character */
			p += 2;
			continue;
		}
		if (special == TRUE && *p == SPECIAL_IN_TROFF) {
			ifile = strlen(FILENAME);
			if (strncmp(p, FILENAME, ifile) == 0) {
				if (env == (struct environment *) 0)
					file = get_file_name();
				else
					file = env->filename;
				strncpy(q, file, strlen(file));
				change = TRUE;
				p += ifile;
				q += strlen(file);
			} else {
				iline = strlen(LINENUMBER);
				if (env == (struct environment *) 0)
					line = get_line_number();
				else
					line = env->linenumber;
				if (strncmp(p, LINENUMBER, iline) == 0) {
					sprintf(q, "%5d", line);
					change = TRUE;
					p += iline;
					q += 5;
				}
			}
		}
		if (*p != EMBED_IN_TROFF ||
			(loop=isitaloop(p+1, FALSE)) == (struct loop *) 0) {
			*q++ = *p++;
			continue;
		}
		if(*(p = loop->newp) == '\0')
			continue;
		/* expand loop for each arg matching expand char */
		for (begloop = p, v=values; v != (struct value *)0; v=v->next){
			if (v->type != XPAND_ARG)
				continue;
			loop->current = v;
			for (p = begloop; *p != loop->close_delim;)
				if (*p == DEF_ARGUMENT
				&& (newarg=whichargs(p+1,values,loop))!=(char *) 0){
						change = TRUE;
						strcpy(q, newarg);
						q += strlen(newarg);
						p += 2;
				} else
					*q++ = *p++;
		}
		++p;
	}
	if (change == TRUE) {
		*q = '\0';
		q = bufmalloc(BUF_TEXT, strlen(buf)+1);
		strcpy(q,buf);
		return(q);
	}
	return(text);
}
