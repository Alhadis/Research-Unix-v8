#include	<stdio.h>
#include	<ctype.h>
#include	"search.h"
#include	"warn.h"
#include	"rd.h"
#include	"dbcompress.h"

/* for each definition:
	copy begin and end lists, translate to troff equivalents */

void
read_define(database, name, type, close_delim)
FILE	*database;
char	*name;
short	type;
int	close_delim;
{
	ENTRY	*entry;

	if ((entry = hashfind(DEF_TABLE, name)) != (ENTRY *) 0) {
		warn_db(PR_FILENAME | PR_LINENUMBER,
			"Redefining associate or environment `%s'\n", name);
		entry->data = (char *) read_list(database, name,
				(struct value *) 0, type, close_delim);
		return;
	}
	entry = (ENTRY *) bufmalloc(BUF_HASH_DEF, sizeof(ENTRY));
	/* get key, which is the name of item being defined */
	entry->key = name;
	entry->data = (char *) read_list(database, name,
			(struct value *) 0, type, close_delim);
	hashenter(DEF_TABLE, entry);
	if (strcmp(name, "iftest") == 0) {
		warn_me(0, "Read_define:\n");
		pr_def((struct definition *) entry->data);
	}
#ifdef DEBUG_ENVIRONMENT
	pr_def((struct definition *) entry->data);
#endif
}

struct definition	*
read_list(database, name, values, assoc, close_delim)
FILE	*database;
char	*name;
struct value	*values;
short	assoc;
int	close_delim;
{
	static char	*blankstring;
	struct definition	*def, *d;
	struct value	*v;
	char	inner_delim;
	char	*assoc_name;

	def = (struct definition *)bufmalloc(BUF_DEF,sizeof(struct definition));
	def->name = name;
	def->values = values;
	def->sub_def = (struct definition *) 0;
	def->instance = (struct environment *) 0;
	def->begin_def = trans_list(database, values, LIST_SEP);
	if (assoc == FALSE) {
		def->end_def = trans_list(database, values, close_delim);
		return(def);
	}
	def->end_def = trans_list(database, values, LIST_SEP);
	for (d = def; (inner_delim = read_delim(database)) != (char) FALSE;
							d = d->sub_def) {
		/* BLANKSTRING must be stored for compression routines;
			no use nor harm otherwise... */
		if (blankstring == NULL) {
			blankstring = bufmalloc(BUF_TEXT,strlen(BLANKSTRING)+1);
			strcpy(blankstring, BLANKSTRING);
		}
		assoc_name = blankstring;
		if ((v = read_values(database, FALSE)) == (struct value *) 0) {
			readncheck(database, inner_delim, TRUE, TRUE,
			"Reading %s, null arguments given for associate", name);
			continue;
		}
		readncheck(database, LIST_SEP, FALSE, TRUE, "AReading %s %s",
						name, str_avalue(v, TRUE));
		/* ZZZ need a bubble case:
			not just matching name and pluncking in order
		*/
		d->sub_def = read_list(database,assoc_name,v,FALSE,inner_delim);
	}
	readncheck(database, close_delim, FALSE, TRUE, "CReading closing %s %s",
						name, str_avalue(v, TRUE));
	return(def);
}

/* translist:
	read list of attributes with values, storing it at buffer->current;
	translate each attribute in list into troff equivalent
	generates linked list of def_elements, each one corresponding to
	an attribute
 */

struct def_element *
trans_list(database, values, delimiter)
FILE	*database;
struct value	*values;
char	delimiter;
{
	struct def_element	*d;
	struct cond_def_el	*cond_def_el, *oldcond_def_el;
	struct loop	*loop;
	struct loop	copyloop;
	struct def_element	*firstd, *lv;
	int	c, close_delim, type;
	char	*p;

	for (firstd = (struct def_element *) 0,
				cond_def_el = (struct cond_def_el *) 0; ; ) {
		if ((p = read_buftoken(database, OK_SPACE)) == (char *) 0) {
			if ((c = mygetc(database)) != EMBED_IN_TROFF) {
				myungetc(c, database);
			/* toss or continue ??? */
				readncheck(database, delimiter, FALSE, TRUE,
				"EReading %s %s", ASSOCIATE, ENVIRONMENT);
				break;
			}
			/* can i know name here ??? */
			if ((p = read_token(database, NO_SPACE)) == (char *) 0){
				warn_db(PR_FILENAME | PR_LINENUMBER,
				"Reading %s: `|' with null command\n",
						ASSOCIATE, ENVIRONMENT);
				continue;
			}
#ifdef MONK_SAVE
			/* MONKSAVE, MONKREMEMBER, FOR_LOOP or IF_[NOT]VALUE */
			if (strcmp(p, MONKSAVE) == 0 ||
						strcmp(p, MONKREMEMBER) == 0) {
				lv = (struct def_element *)
						bufmalloc(BUF_DEF_EL,
						sizeof(struct def_element));
				if (firstd == (struct def_element *) 0)
					d = firstd = lv;
				else {
					d->next = lv;
					d = d->next;
					if (cond_def_el != NULL) {
						cond_def_el->next_on_fail = d;
						cond_def_el = NULL;
					}
				}
				d->next = (struct def_element *) 0;
				d->cdl = (struct cond_def_el *) 0;
				d->attribute = p;
				if ((d->value = read_values(database, TRUE))
								== (char *) 0) {
					warn_db(PR_FILENAME | PR_LINENUMBER,
						"Null name to save/remember\n");
					continue;
				}
				if ((c = read_nonblank(database)) == delimiter)
					break;
				continue;
			}
#endif
			if (strcmp(p, FOR_LOOP) == 0 &&
				(loop = readaloop(database, TRUE))
							!= (struct loop *) 0) {
				if (*loop->newp != '\0')
					warn_db(PR_FILENAME | PR_LINENUMBER,
					"Problem in top of loop syntax\n");
				/* here loops cannot be embedded using
							same close_delim */
				copy_struct_loop(loop, &copyloop);
				lv = trans_list(database, values,
							copyloop.close_delim);
				/* allow ITEM_SEP, tho not necessary */
				if ((c = mygetc(database)) != ITEM_SEP)
					myungetc(c, database);
				if ((lv = xpand_loop_valu(lv, &copyloop)) ==
								NULL) {
					warn_db(0,
					"Expanding %c: %s til %c to nothing\n",
						copyloop.loopchar,
						str_avalue(copyloop.args, TRUE),
						copyloop.close_delim);
					continue;
				}
				if (firstd == (struct def_element *) 0)
					d = firstd = lv;
				else {
					d->next = lv;
					d = d->next;
					if (cond_def_el != NULL) {
						cond_def_el->next_on_fail = d;
						cond_def_el = NULL;
					}
				}
				while ((lv = d->next) != NULL)
					d = lv;
				continue;
			}
			if (strcmp(p, IF_VALUE) == 0)
				type = TRUE;
			else
				if (strcmp(p, IF_NOTVALUE) == 0)
					type = FALSE;
				else {
					warn_db(PR_FILENAME | PR_LINENUMBER,
				"%s and %s do not provide a command %s\n",
						ENVIRONMENT, ASSOCIATE, p);
					continue;
				}
			oldcond_def_el = cond_def_el;
			if ((cond_def_el = readacond(database, type, TRUE))
						!= (struct cond_def_el *) 0) {
				if ((close_delim = read_userdelim(database))
									== EOF)
					close_delim = FALSE;
				lv = trans_list(database, values, close_delim);
				/* allow ITEM_SEP, tho not necessary */
				if ((c = mygetc(database)) != ITEM_SEP)
					myungetc(c, database);
				if (lv == NULL) {
					warn_db(PR_FILENAME | PR_LINENUMBER,
						"Empty %s\n", p,
							cond_def_el->attribute);
					cond_def_el = (struct cond_def_el *) 0;
					continue;
				}
				if (firstd == (struct def_element *) 0)
					d = firstd = lv;
				else {
					d->next = lv;
					d = d->next;
					if (oldcond_def_el != NULL)
						oldcond_def_el->next_on_fail=d;
				}
				d->cdl = cond_def_el;
				while ((lv = d->next) != NULL)
					d = lv;
				continue;
			}
		}
		if (firstd == (struct def_element *) 0)
			d =firstd= (struct def_element *) bufmalloc(BUF_DEF_EL,
					 	sizeof(struct def_element));
		else {
			d->next = (struct def_element *) bufmalloc(BUF_DEF_EL,
						sizeof(struct def_element));
			d = d->next;
			if (cond_def_el != (struct cond_def_el *) 0) {
				cond_def_el->next_on_fail = d;
				cond_def_el = (struct cond_def_el *) 0;
			}
		}
		d->next = (struct def_element *) 0;
		d->cdl = (struct cond_def_el *) 0;
		d->attribute = p;
		d->value = read_values(database, TRUE);
		rename_values(d->value, values);
		if (translate(d, SETUP_TIME) == FALSE) {
		/* may be improperly entered attribute or a define 
			- during development, warn_db 
			- need to make list of undefined values and check them
				when databases are completely read!!! */
			if (d->value != (struct value *) 0)
				warn_db(PR_FILENAME | PR_LINENUMBER,
					"No attribute found for %s %s\n",
					d->attribute,str_avalue(d->value,TRUE));
			else
				missing_def(d->attribute);
		}
	/* doesn't check what character separating items in list is */
		if ((c = read_nonblank(database)) == delimiter)
			break;
		if (c != ITEM_SEP)
			warn_db(PR_FILENAME | PR_LINENUMBER,
				"Improper item separator %c in list\n", c);
		/* Health: spit back any `|'. */
	}
	if (c == EOF) {
		free_def_el(firstd);
		return((struct def_element *) 0);
	}
	return(firstd);
}

struct def_element *
xpand_loop_valu(del, loop)
struct def_element	*del;
struct loop	*loop;
{
	struct def_element	*firstl, *l;
	struct value		*arg;
	char	*newtext;

	firstl = (struct def_element *) 0;
	for (arg = loop->args; arg != (struct value *) 0; arg = arg->next) {
		if (firstl == (struct def_element *) 0)
			firstl = l = copy_def_el(del);
		else {
			l->next = copy_def_el(del);
			l = l->next;
		}
		for (loop->current = arg; ; l = l->next) {
			if ((newtext=fillin_loop_valu(l->troff, loop))
								!= l->troff){
				l->troff = newtext;
				l->allocated = TRUE;
			}
			if (l->next == (struct def_element *) 0)
				break;
		}
	}
	return(firstl);
}

char	*
fillin_loop_valu(text, loop)
char	*text;
struct loop	*loop;
{
	short	change;
	char	*p, *q, *newtext;
	char	buf[BUFSIZ];

	for (p = text, q = buf, change = FALSE; *p != '\0'; )
		if (*p == DEF_ARGUMENT && *(p+1) == loop->loopchar) {
			change = TRUE;
			strcpy(q, loop->current->value);
			q += strlen(q);
			p += 2;
		} else
			*q++ = *p++;
	if (change == TRUE) {
		*q = '\0';
		newtext = bufmalloc(BUF_TEXT, strlen(buf)+1);
		strcpy(newtext, buf);
		return(newtext);
	}
	return(text);
}

/* rename each variable argument in args using the choices in valuelist */
rename_values(arglist, valuelist)
struct value	*arglist, *valuelist;
{
	struct value	*arg, *v;
	short	n;
	static char	argpos[STR_SIZE];
	char	*newstr;

	for (v = valuelist, n = 0; v != (struct value *) 0; v = v->next) {
		if (++n > 9) {
			warn_db(PR_FILENAME | PR_LINENUMBER,
			"Limit of nine arguments exceeded: ignoring extras\n");
			break;
		}
		newstr = (char *) 0;
		for (arg = arglist; arg != (struct value *) 0; arg = arg->next)
			if ((arg->type == VAR_ARG || arg->type == REGEX_ARG)
					&& strcmp(arg->value, v->value) == 0){
				if (newstr == (char *) 0) {
					sprintf(argpos, "%c%d", DEF_ARGUMENT,n);
					newstr = bufmalloc(BUF_TEXT,
							strlen(argpos)+1);
					strcpy(newstr, argpos);
				}
				arg->value = newstr;
			}
	}
}

#ifdef OBSOLETE
/* map_values:
	maps arguments to associates; all fixed or variable; no loops, expanders
 */

struct def_element *
map_def_el(del, values)
struct def_element	*del;
struct value	*values;
{
	static struct def_element	*created;
	struct def_element	*d;

	if (values != (struct value *) 0)
		return(del);
	if (created != (struct def_element *) 0)
		free_def_el(created, 43);
	created = copy_def_el(del);
	for (d = created; d != (struct def_element *) 0; d = d->next)
		map_values(d->value, values);
	return(created);
}
#endif

/* need to map values in troff */

map_values(att_values, choices)
struct value	*att_values, *choices;
{
	struct value	*arg, *v;
	char	buf[BUFSIZ];

	if (att_values == (struct value *) 0 || choices == (struct value *) 0)
		return;
	/* FOR NOW, look only for identical values
	struct strings	*strings;
	short	change;
	char	*p, *q, *r;
	for (p = r = v->value, q = buf, change = FALSE; *p != '\0'; ++p)
		if (*p == DEF_ARGUMENT) {
			strings = strtok(p+1);
			for (arg = choices; arg != (struct value *) 0;
							arg = arg->next) {
				if (strcmp(strings->token, 
	 */
	for (arg = att_values; arg != (struct value *) 0; arg = arg->next)
		for (v = choices; v != (struct value *) 0; v = v->next)
			if (strcmp(arg->value, v->value) == 0)
			/* is v->value a safe (not to be deallocated) copy*/
				arg->value = v->value;
}
