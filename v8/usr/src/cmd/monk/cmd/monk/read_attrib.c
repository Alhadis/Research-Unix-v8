#include	<stdio.h>
#include	<ctype.h>
#include	"search.h"
#include	"warn.h"
#include	"rd.h"
#include	"dbcompress.h"

/* read_attribute:
	add an attribute to hashtable, to state stack if stacking variable,
	and to initialize definition if a default is given
 */

char	*init_troff, *init_stack;

void
read_attribute(database, name, close_delim)
FILE	*database;
char	*name;
char	close_delim;
{
	register ENTRY	*att_entry;
	ENTRY	*exist;
	struct attribute_info	*att_new;
	struct attribute_case	*ac;
	struct init_def	*init_def;
	struct value	*att_values;
	char	c;
	char	*att_format;

	if ((init_def=add_att_type(database, name)) == (struct init_def *)EOF)
		return;
	att_entry = (ENTRY *) bufmalloc(BUF_HASH_ATT, sizeof(ENTRY));
	att_entry->data=bufmalloc(BUF_ATT_INFO,sizeof(struct attribute_info));
	att_entry->key = name;
	att_new = (struct attribute_info *) att_entry->data;
	att_new->stacking = init_def->stacking;
	att_new->firstcase = (struct attribute_case *) 0;
	for (att_values = (struct value *) 0, att_format = (char *) 0;
		(att_values = read_values(database, FALSE))
						!= (struct value *)EOF;) {
		att_format = read_formatcommands(database, name, att_values);
		if (att_format == (char *) 0)
			break;
		make_case(att_new, att_format, att_values);
	}
	if (att_values == (struct value *) EOF || att_format == (char *) EOF)
		return;
	readncheck(database, close_delim, FALSE, TRUE,
					"TReading %s %s", ATTRIBUTE, name);
	if ((exist = hashfind(ATT_TABLE, att_entry->key)) != (ENTRY *) 0) {
		warn_db(PR_FILENAME | PR_LINENUMBER,
				"Redefining attribute `%s'\n", att_entry->key);
		exist->data = att_entry->data;
	} else
		hashenter(ATT_TABLE, att_entry);
	add_init(init_def);
#ifdef DEBUG_ATTRIBUTE
	pr_att(att_entry, TRUE);
#endif
}

char *
read_formatcommands(database, name, att_values)
FILE	*database;
char	*name;
struct value	*att_values;
{
	struct buffer	buffer;
	struct buffer	*b;
	struct file_info	*fi;
	short	len;
	char	inner_close;
	char	*store_format;

	/* not an open delimiter->end of attribute definition */
	if ((inner_close = read_delim(database)) == (char) FALSE)
		return((char *) 0);
	fi = save_file_info();
	create_buffer(&buffer);
	if (read_format(database, &buffer, inner_close, FALSE) != TRUE) {
		warn_db(0," in Attribute-case %s%s starting at line %d\n",
			name, str_avalue(att_values, TRUE), fi->line_number);
		free_buffer(&buffer);
		return((char *) 0);
	}
	map_args(buffer.start, att_values);
	len = strlen(buffer.start)-1;
	store_format = bufmalloc(BUF_TEXT, len+1);
	strncpy(store_format, buffer.start, len);
	store_format[len] = '\0';
	free_buffer(&buffer);
	return(store_format);
}

read_format(database, b, delim, checkopen)
FILE	*database;
struct buffer	*b;
char	delim;
short	checkopen;
{
	struct conditional	*cond;
	struct loop	*loop;
	short	ok;
	char	inner_delim;

	for (;;) {
		if (b->current == b->empty)
			if ((ok = read_into_til(database, b, delim)) != TRUE)
				return(ok);
		if (*b->current == delim) {
			++b->current;
			return(TRUE);
		}
		if (checkopen == TRUE) {
			if ((inner_delim = close_match(*b->current++)) != FALSE)
				if ((ok=read_into_til(database, b, inner_delim))
									!= TRUE)
					return(ok);
			continue;
		}
		if (*b->current++ != EMBED_IN_TROFF)
			continue;
		if ((loop = isitaloop(b->current, TRUE)) != (struct loop *) 0) {
			if((ok = read_inner(database, b, loop->newp,
						loop->close_delim)) != TRUE)
				return(ok);
			continue;
		}
		if ((cond = isitaconditional(b->current, TRUE))
						!= (struct conditional *) 0) {
			if((ok = read_inner(database, b, cond->newp,
						cond->close_delim)) != TRUE)
				return(ok);
			continue;
		}
	}
}

read_inner(database, b, newp, delimiter)
FILE	*database;
struct buffer	*b;
char	*newp;
char	delimiter;
{
	short	ok;
	char	c;

	b->current = newp;
	/* ZZZZ - checkopen??? TRUE or FALSE what for???
			was TRUE */
	if((ok=read_format(database, b, delimiter, FALSE)) != TRUE)
		return(ok);
	return(ok);
}

read_into_til(database, b, delim)
FILE	*database;
struct buffer	*b;
char	delim;
{
	register char	*s;
	char	*se;

	for (s = b->empty, se = b->end;
			(*s = mygetc(database)) != EOF && *s != delim; )
		if (++s > se) {
			b->empty = s;
			if (grow_buffer(b) == FALSE) {
				warn_db(PR_FILENAME,
			"Filled %db buffer searching for delimiter %c\n\t\t",
							10*BUFSIZ, delim);
				return(FALSE);
			}
			s = b->empty;
		}
	if (*s == EOF) {
		warn_db(PR_FILENAME,
			"Reached EOF searching for delimiter %c\n\t\t", delim);
		return(EOF);
	}
	*++s = '\0';
	b->empty = s;
	return(TRUE);
}

void
make_case(attribute_info, text, v)
struct attribute_info	*attribute_info;
char	*text;
struct value	*v;
{
	char	*regcmp(), *strchr();
	struct attribute_case	*newcase;
	int	ifile, iline, len;
	char	*p;

	newcase = (struct attribute_case *)
			bufmalloc(BUF_ATT_CASE, sizeof(struct attribute_case));
	newcase->expanding = newcase->nvalues = 0;
	newcase->value = v;
	if (v != (struct value *) 0)
		do {
			newcase->nvalues++;
			if (v->type == REGEX_ARG) {
				/* compile argument which follows 2 chars:
					DEF_ARGUMENT and REGEX */
				if ((p = regcmp(&v->value[2], 0)) == 0) {
					warn_db(PR_FILENAME | PR_LINENUMBER,
					"Omitting attribute-case: ");
					warn_db(0,
					"can't compile regular expression %s\n",
					v->value);
					buffree(newcase, 40);
					return;
				} else {
					/* Health? HMMMMMM */
					len = strlen(p);
					v->value = bufmalloc(BUF_TEXT, len+1);
					strncpy(v->value, p, len);
				}
			} else
				if (v->type == XPAND_ARG)
					newcase->expanding = newcase->nvalues;
		} while ((v = v->next) != (struct value *) 0);
	newcase->special = FALSE;
	newcase->troff = p = text;
	if ((p = strchr(p, SPECIAL_IN_TROFF)) != 0) {
		ifile = strlen(FILENAME);
		iline = strlen(LINENUMBER);
		do {
			/* look for special cookies, e.g. __FILE__, __LINE__ */
			if (strncmp(p, FILENAME, ifile) == 0) {
				newcase->special = TRUE;
				break;
			} else
				if (strncmp(p, LINENUMBER, iline) == 0) {
					newcase->special = TRUE;
					break;
				}
		} while ((p = strchr(++p, SPECIAL_IN_TROFF)) != 0);
	}
	newcase->next = (struct attribute_case *) 0;
	bubble_case(attribute_info, newcase);
}

/* map_args:
	maps attribute arguments from those used in database to $1, $2, ...
	expands for.loops with fixed arguments, copies those with expanding args
 */

/* REALLY should pass struct buffer and put in overrun protection for text */

void
map_args(text, values)
char	*text;
struct value	*values;
{
	struct loop	*loop;
	struct value	*v;
	struct strings	*strings;
	char	close_delim;
	register char	*p, *q;
	char	*begloop, *newarg, *pe;
	char	buf[SAFESIZ];

	if (values == (struct value *) 0)
		return;
	loop = (struct loop *) 0;
	for (p=text, q=buf, pe=text+strlen(text); p < pe; ) {
		if (*p == DEF_ARGUMENT
				&& (strings = rename_arg(p, values, loop))
							!= (struct strings *) 0){
			strcpy(q, strings->token);
			q += strlen(strings->token);
			p = strings->newp;
			continue;
		}
		if (*p != EMBED_IN_TROFF ||
				(loop=isitaloop(p, FALSE)) == (struct loop *) 0) {
			*q++ = *p++;
			continue;
		}
		if ((p = loop->newp) == '\0')
			continue;
		/* Expand loops for all fixed or variable loop parameters
				(must expand $* for each instance) */
		for (begloop=p, v=loop->args; v != (struct value *) 0; v=v->next) {
			loop->current = v;
			if (v->type == XPAND_ARG) {
				q = copy_text_loop(q, loop);
				*q = *(begloop-1);
			}
			for (p = begloop+1; *p != loop->close_delim;)
				if (*p == DEF_ARGUMENT
					&& (strings = rename_arg(p,values,loop))
							!= (struct strings *)0){
						strcpy(q, strings->token);
						q += strlen(strings->token);
						p = strings->newp;
				} else
					*q++ = *p++;
			if (v->type == XPAND_ARG)
				*q++ = loop->close_delim;
		}
		++p;
	}
	*q = '\0';
	strcpy(text, buf);
}

struct strings	*
rename_arg(s, values, loop)
char	*s;
struct value	*values;
struct loop	*loop;
{
	struct strings	*strings;
	char	*p;

	if (*(s+1) == '*')
		return((struct strings *) 0);
	if ((strings = strtok(s)) == (struct strings *) 0)
		return((struct strings *) 0);
	/* does it match one of the values in the list */
	if (doesargmatch(strings, values) == TRUE)
		return(strings);
	if (loop != (struct loop *) 0 && *(s+1) == loop->loopchar) {
		strings->token = loop->current->value;
		strings->newp = s+2;
		return(strings);
	}
	/* Kludge: becasue now there are no delimiters for argument_tags,
		drop last character working back to check for match of names
		of less than the full token length - this allows the `u' for
		units to sit up against the argument */
	while (--strings->newp > s) {
		strings->token[strlen(strings->token)-1] = '\0';
		if (doesargmatch(strings, values) == TRUE)
			return(strings);
	}
	return((struct strings *) 0);
}

doesargmatch(strings, values)
struct strings	*strings;
struct value	*values;
{
	static char	argpos[STR_SIZE];
	short	n;

	for (n = 0; values != (struct value *) 0; values = values->next) {
		/* BAD - no good reason for this limit of 9..... */
		if (++n > 9) {
			warn_db(PR_FILENAME | PR_LINENUMBER,
			"Limit of nine arguments exceeded: ignoring extras\n");
			break;
		}
		if ((values->type == VAR_ARG || values->type == REGEX_ARG)
				&& strcmp(strings->token, values->value) == 0) {
			sprintf(argpos, "%c%d", DEF_ARGUMENT, n);
			strings->token = argpos;
			return(TRUE);
		}
	}
}

/* bubble_case:
	rule for case ordering
		All non-expanding cases, in order of increasing #arguments
		All expanding cases, in order of decreasing #arguments
 */

void
bubble_case(att_info, newcase)
struct attribute_info	*att_info;
struct attribute_case	*newcase;
{
	struct attribute_case	*lastpos, *pos;
	struct value	*v;
	short	fixed, regexp;

	fixed = regexp = 0;
	if ((v = newcase->value) != (struct value *) 0)
		do {
			if (v->type == FIXED_ARG)
				fixed++;
			else if (v->type == REGEX_ARG)
				regexp++;
		} while ((v = v->next) != (struct value *) 0);
	pos = att_info->firstcase;
	if (nextcase(newcase, pos, fixed, regexp) == TRUE) {
		att_info->firstcase = newcase;
		newcase->next = pos;
		return;
	}
	for (lastpos = pos; (pos = pos->next) != (struct attribute_case *) 0;
								lastpos = pos)
		if (nextcase(newcase, pos, fixed, regexp) == TRUE) {
			lastpos->next = newcase;
			newcase->next = pos;
			return;
		}
	lastpos->next = newcase;
}

/* nextcase:	true if case to be added precedes thiscase */

nextcase(addcase, thiscase, addfixed, addregexp)
struct attribute_case	*addcase, *thiscase;
short	addfixed, addregexp;
{
	struct value	*v;
	short	thisfixed, thisregexp;

	if (thiscase == (struct attribute_case *) 0)
		return(TRUE);
	thisfixed = thisregexp = 0;
	if ((v = thiscase->value) != (struct value *) 0)
		do {
			if (v->type == FIXED_ARG)
				thisfixed++;
			else if (v->type == REGEX_ARG)
				thisregexp++;
		} while ((v = v->next) != (struct value *) 0);
	if (addcase->expanding == FALSE) {
		if (thiscase->expanding == TRUE
					|| thiscase->nvalues > addcase->nvalues)
			return(TRUE);
		if (thiscase->nvalues == addcase->nvalues)
			if (thisfixed < addfixed || thisregexp < addregexp)
				return(TRUE);
	} else
		if (thiscase->expanding == TRUE) {
			if (thiscase->nvalues < addcase->nvalues)
				return(TRUE);
		if (thiscase->nvalues == addcase->nvalues)
			if (thisfixed < addfixed || thisregexp < addregexp)
				return(TRUE);
		}
	return(FALSE);
}

/* add_att_type:
	if stacking attribute, adds to statestack hashtable
	if default specified, modifies internal initialize definition
 */

struct init_def	*
add_att_type(database, name)
FILE	*database;
char	*name;
{
	static struct init_def	*init_def;
	struct add_att	*a;
	short	i;
	char	*p;

/* interesting: when a definition is added at run-time, i do not want to
			bother with the initialization definitions.
			run these additions now
 */
	/* Again, can gobble ridiculous amounts if LIST_SEP missing... */
	if(init_def == (struct init_def *) 0) {
		init_def =(struct init_def *) mymalloc(sizeof(struct init_def));
	} else
		/*reuse allocated init, but free add_att structure list */
		free_attlist(init_def->add_att);
	init_def->attribute = name;
	init_def->stacking = FALSE;
	init_def->add_att = (struct add_att *) 0;
	do {
		if ((p = read_token(database, OK_SPACE)) == (char *) EOF)
			return;
		if (p == (char *) 0) {
			if (read_nonblank(database) != LIST_SEP)
				warn_db(PR_FILENAME | PR_LINENUMBER,
				"%s attribute has no initialization field\n",
				name);
			break;
		}
		if (strcmp(p, STACKING) == 0) {
			init_def->stacking = TRUE;
			myfree(p, 1);
			continue;
		}
		if ((i= strcmp(p, INITIALIZE)) == 0 || strcmp(p, DEFAULT) == 0){
			if (init_def->add_att == (struct add_att *) 0)
				a = init_def->add_att = (struct add_att *)
					mymalloc(sizeof(struct add_att));
			else {
				a->next = (struct add_att *)
					mymalloc(sizeof(struct add_att));
				a = a->next;
			}
			if (i == 0)
				a->defname = init_troff;
			else
				a->defname = init_stack;
			a->value = read_values(database, FALSE);
			a->next = (struct add_att *) 0;
		}
	} while (read_nonblank(database) != LIST_SEP);
	if (init_def->stacking == TRUE
				&& init_def->add_att == (struct add_att *) 0)
		warn_db(PR_FILENAME | PR_LINENUMBER,
				"Stacking attribute %s needs a default value\n",
					init_def->attribute);
	return(init_def);
}

add_init(init_def)
struct init_def	*init_def;
{
	ENTRY	*entry;
	struct add_att	*a;
	struct def_element	*el;
	struct definition	*d;
	struct value	*v;
	char	*definename;

	if ((a = init_def->add_att) == (struct add_att *) 0)
		return;
	do {
		if ((el = mk_def_el(init_def, a)) == (struct def_element *) 0) {
			warn_db(PR_FILENAME | PR_LINENUMBER,
					"%s: cannot find attribute-case %s",
					a->defname, init_def->attribute);
			if ((v = a->value) != (struct value *) 0)
				do
					warn_db(0, " %s", v->value);
				while ((v = v->next) != (struct value *) 0);
			warn_db(0, "\n");
			continue;
		}
		/* build initialize - get and update hashtable definition */
		if ((entry=hashfind(DEF_TABLE, a->defname)) == (ENTRY *) NULL) {
			entry = (ENTRY *)
					bufmalloc(BUF_HASH_DEF, sizeof(ENTRY));
			entry->key = a->defname;
			/* building begin_def for initialization */
			entry->data =
				bufmalloc(BUF_DEF, sizeof(struct definition));
			d = (struct definition *) entry->data;
			d->name = a->defname;
			d->values = (struct value *) 0;
			d->begin_def = el;
			d->end_def = (struct def_element *) 0;
			d->sub_def = (struct definition *) 0;
		} else {
			d = (struct definition *) entry->data;
			/* warning given if attribute redefined; no reason to
				warn at redefinition of default for attribute */
			d->begin_def = merge_def_el(d->begin_def, el);
		}
		hashenter(DEF_TABLE, entry);
	} while ((a = a->next) != (struct add_att *) 0);
}

init_dbread()
{
	init_stack = bufmalloc(BUF_TEXT, strlen(INIT_STACK)+1);
	strcpy(init_stack, INIT_STACK);
	init_troff = bufmalloc(BUF_TEXT, strlen(INIT_TROFF)+1);
	strcpy(init_troff, INIT_TROFF);
	mk_hashtables();
}
