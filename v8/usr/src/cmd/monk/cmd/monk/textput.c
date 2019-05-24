#include	<stdio.h>
#include	<ctype.h>
#include	"search.h"
#include	"warn.h"
#include	"rd.h"

textputs(s, whose)
register char	*s;
{
	if (whose == USER_TEXT)
		while (*s != '\0')
			textputc(*s++, whose);
	else /* whose == MONK_DB or FORCE */
		checkputs(s, whose, '\0');
}

char	*
checkputs(s, spew, end_char)
register char	*s;
short	spew;
char	end_char;
{
	static short	layer;
	char	*news;

	if (end_char == '\0')
		layer = 0;
	else
		++layer;
	while (*s != end_char && *s != '\0')
		if (*s == EMBED_IN_TROFF) {
			if ((news = checkif(s++, spew)) == (char *) 0) {
				if (spew != FALSE)
					textputc(EMBED_IN_TROFF, spew);
				continue;
			} else {
				s = news;
			}
		} else {
			if (spew != FALSE)
				textputc(*s, spew);
			++s;
		}
	if (*s != end_char)
		fprintf(stderr, "Reached end_of_string b4 delimiter %c,%o\n",
							end_char, end_char);
	if (*s != '\0') {
		s += 1;
		if (--layer < 0)
			fprintf(stderr, "Negative layer count\n");
	}
	return(s);
}

char	*
checkif(s, spew)
register char	*s;
short	spew;
{
	ENTRY	*grabit;
	struct statestack	*st;
	struct value	*v;
	struct conditional	*cond;
	int	close_if, true;
	char	*iftype, *p;

	if ((cond = isitaconditional(s+1, FALSE)) == (struct conditional *) 0){
		if (spew != FALSE)
			textputc(*s, spew);
		return(s+1);
	}
	if (spew != FALSE) {
		true = iscondtrue(cond->attribute, cond->value, cond->type);
		spew = (true == FALSE) ? FALSE : spew;
	}
	free_cond(cond);
	return(checkputs(cond->newp, spew, cond->close_delim));
}

iscondtrue(attribute, value, type)
char	*attribute;
struct value	*value;
{
	ENTRY	*grabit;
	struct statestack	*st;
	int	true;

	if ((grabit = hashfind(STACK_TABLE, attribute))== (ENTRY *) NULL)
		return(FALSE);
	st = (struct statestack *) grabit->data;
	if (st->last != (struct state *) 0) {
		true = cmp_valu(value, st->last->value);
		if (type == FALSE)
			true = (true == TRUE) ? FALSE : TRUE;
	}
	return(true);
}

/* put text into formatted output, suppressing multiple newlines and white space
	following new-lines --- note: isspace true for newlines! */

/* new version:
	hold over monk char til next call to textputc ????
	if lastchar == newline
		if (lastwhose == USER && whose == USER)
			beginnendenv(usernewlines)
		if (lastchar_whose == MONK_DB && isspace(thischar)
			skip it < or skip last \n >
			continue
 */

textputc(ic, whose)
int	ic, whose;
{
	static int	lastchar_whose = MONK_DB;
	static short	lastchar_nl = TRUE;
	char	c;

	c = ic;
	if (whose != FORCE && lastchar_nl == TRUE) {
		if (whose == USER_TEXT) {
			switch(c) {
			case NEW_LINE:
				if (lastchar_whose == USER_TEXT)
					user_nl_nl();
				return;
			case BLANK:
			case TAB:
				user_nl_white(c);
				lastchar_whose = USER_TEXT;
				/* do not know what actual lastchar is */
				return;
			}
		} else
			/* monk can never put out successive new-lines */
			if (c == NEW_LINE)
				return;
	}
	fputc(c, stdout);
	lastchar_nl = (c == NEW_LINE) ? TRUE : FALSE;
	lastchar_whose = whose;
}

/* routines for handling new-line cases:
	check what i do when definitions are not found
	all touch the environment stack, but all should return to original
		environment
*/

#ifdef MONK_NEWLINE
monknewline()
{
	static struct environment	*newenv;
	static struct value	*vfile, *vline;
	static ENTRY	*entry;
	static int	tried;
	static char	s[BUFSIZ];
	struct environment	*tempenv;
	struct file_info	*fi;

	if (entry == NULL) {
		if (tried == TRUE)
			return;
		tried = TRUE;
		if ((entry = hashfind(DEF_TABLE, MONK_NEWLINE))
							== (ENTRY *) NULL) {
			warn_db(PR_FILENAME | PR_LINENUMBER,
				"Cannot find definition for %s\n",MONK_NEWLINE);
			return;
		}
		/* build an environment structure to pass to do_subenv;
			never invoke begin_envir or end_envir or use true env
			structure hierarchy; make value structures in which
			to stuff filename and linenumber
	 	*/
		newenv = mk_envir(NULL, entry->data, SUB_LEVEL, NULL);
		vfile = make_value(BLANKSTRING);
		vfile->next = make_value(BLANKSTRING);
	}
	fi = save_file_info();
	vfile->value = filestack->name;
	sprintf(s, "%d", filestack->line_number);
	vfile->next->value = s;
			/* textwrite == TRUE, always write newline code */
	do_subenv(newenv, entry->data, vfile, NULL, END_NOW, FORCE);
}
#endif

/* do not use environment stack - saves time 
	must be SUB_LEVEL, so will not affect stack of definition instances */

user_nl_nl()
{
	static ENTRY	*entry;
	static int	tried;
	struct environment	*env;

	if (entry == NULL) {
		if (tried == TRUE)
			return;
		tried = TRUE;
		if ((entry = hashfind(DEF_TABLE, USER_NL_NL))
							== (ENTRY *) NULL) {
			warn_db(PR_FILENAME | PR_LINENUMBER,
				"Cannot find definition for %s\n", USER_NL_NL);
			return(FALSE);
		}
	}
	env = do_envir(NULL, entry->data, SUB_LEVEL, NULL, FORCE);
	return(TRUE);
}

user_nl_white(space_char)
char	space_char;
{
	static ENTRY	*entry;
	static int	tried;
	static struct environment	*env;
	static struct value	*vstring;
	static char	*string;

	if (entry == NULL) {
		if (tried == TRUE)
			return;
		tried = TRUE;
		if ((entry = hashfind(DEF_TABLE, USER_NL_WHITE))
							== (ENTRY *) NULL) {
			warn_db(PR_FILENAME | PR_LINENUMBER,
				"Cannot find definition for %s\n",
							USER_NL_WHITE);
			return(FALSE);
		}
		env = mk_envir(NULL, entry->data, SUB_LEVEL, NULL);
		string = mymalloc(sizeof(char)*2);
		*string = space_char;
		*(string+1) = '\0';
		vstring = make_value(string);
	} else
		*string = space_char;
	do_subenv(env, entry->data, vstring, NULL, END_NOW, FORCE);
	return(TRUE);		
}
