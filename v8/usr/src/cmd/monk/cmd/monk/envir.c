#include	<stdio.h>
#include	<ctype.h>
#include	"search.h"
#include	"warn.h"
#include	"rd.h"

struct environment *
begin_envir(oldenv, hashkey, close_delim, textwrite)
struct environment	*oldenv;
char	*hashkey;
int	close_delim;
short	textwrite;
{
	struct definition	*def;
	struct environment	*env;

	def = get_envir(hashkey, close_delim, textwrite);
	if (def == (struct definition *) 0)
		return(oldenv);
	env = do_envir(oldenv, def, TOP_LEVEL, close_delim, textwrite);
	return(env);
}

struct definition *
get_envir(hashkey, close_delim, textwrite)
char	*hashkey;
int	close_delim;
short	textwrite;
{
	ENTRY	*entry;
	char	c;

	/* hashentry contains appropriate definition list */
	if ((entry = hashfind(DEF_TABLE, hashkey)) == (ENTRY *) NULL) {
		if (textwrite == USER_TEXT) {
/* ZZ shouldn't all colon environments be called with textwrite == MONK_DB? */
			if (*hashkey != COLON)
				warn_user(PR_FILENAME | PR_LINENUMBER,
			"%s is not defined: use ``\\|'' if not a monk command\n",
								hashkey);
			textputc(BEGIN_PRIMITIVE, USER_TEXT);
			textputs(hashkey, USER_TEXT);
			if ((c = open_match(close_delim)) != (char) FALSE)
				textputc(c, USER_TEXT);
		}
		return((struct definition *) 0);
	}
	return((struct definition *) entry->data);
}

struct environment *
do_envir(oldenv, def, level, close_delim, textwrite)
struct environment	*oldenv;
struct definition	*def;
int	level, close_delim;
short	textwrite;
{
	struct environment	*newenv;

	newenv = mk_envir(oldenv, def, level, close_delim);
	dodef(newenv, def->begin_def, textwrite);
	return(newenv);
}

struct environment *
mk_envir(oldenv, def, level, close_delim)
struct environment	*oldenv;
struct definition	*def;
int	level, close_delim;
{
	struct environment	*newenv;
	struct file_info	*fi;

	newenv = (struct environment *) mymalloc(sizeof(struct environment));
	newenv->def = def;
	newenv->state_list = (struct state *) 0;
	newenv->envname = def->name;
	if (level == TOP_LEVEL) {
		newenv->lastinstance = def->instance;
		def->instance = newenv;
	} else
		newenv->lastinstance = (struct environment *) SUB_LEVEL;
	newenv->previous = oldenv;
	fi = save_file_info();
	newenv->filename = fi->file_name;
	newenv->linenumber = fi->line_number;
	newenv->how_to_end = close_delim;
	return(newenv);
}

/* assoc_envir:
	execute any subenvironments called within the top-level environment
 */

struct environment *
assoc_envir(userfile, env, def, close_delim, endwhen, textwrite)
FILE	*userfile;
struct environment	*env;
struct definition	*def;
int	close_delim, endwhen;
short	textwrite;
{
	struct definition	*subd;
	struct value	*v;
	char	c;

	if ((subd = def->sub_def) == (struct definition *) 0)
		return(env);
	while ((v = read_values(userfile, TRUE)) != (struct value *) 0) {
		env = do_subenv(env, def, v, close_delim, endwhen, textwrite);
		if ((c = read_nonblank(userfile)) == close_delim) {
			myungetc(c, userfile);
			break;
		}
	}
	return(env);
}

struct environment *
do_subenv(env, def, v, close_delim, endwhen, textwrite)
struct environment	*env;
struct definition	*def;
struct value	*v;
int	close_delim, endwhen;
short	textwrite;
{
	struct definition	*d, *subd;

	if ((d = match_assoc(def->sub_def, v)) != (struct definition *) 0) {
		/* if there is a match (and there are no
			expanders) then can pass as definition values */
		d->values = v;
		env = do_envir(env, d, SUB_LEVEL, close_delim, textwrite);
		if (endwhen == END_NOW)
			env = end_envir(env, SUB_LEVEL);
	} else {
		warn_db(PR_FILENAME | PR_LINENUMBER,
				"The %s command does not provide ``%s''\n",
				def->name, str_avalue(v, FALSE));
		textputs(str_avalue(v, FALSE), USER_TEXT);
	}
	return(env);
}

struct definition *
match_assoc(d, values)
struct definition	*d;
struct value	*values;
{
	struct value	*matchthese;
	short	nvalues;

	for (nvalues = 0, matchthese = values; values != (struct value *) 0;
							values = values->next)
		++nvalues;
	if (d != (struct definition *) 0)
		do {
			if (match_arg(matchthese, d->values, nvalues) == TRUE)
				return(d);
		} while ((d = d->sub_def) != (struct definition *) 0);
	return((struct definition *) 0);
}

/* looking for good ideas for trying to recover from author errors.
	this attempts to handle without adding problems the case where
	one environment is not ended, or two adjacent ones are switched.
	On a mismatch, it will pop two environments if the previous one
	matches the key given.
	which cases will be aggravated? */

env_health(env, key)
struct environment	*env;
char	*key;
{
}

/* newstates loops through def_elements:
		writing troff strings to output file
		adding absolute value troff strings to state stack
	 */

void
dodef(newenv, define, textwrite)
struct environment	*newenv;
register struct def_element	*define;
short	textwrite;
{
	register struct statestack	*statestack;
	ENTRY	*grabit;
	struct state	*newstate, *tempstate;
	struct value	*definevalues;
	register struct def_element	*el;
	struct cond_def_el	*cdl;
	char	*modtext, *p;

	if (define == (struct def_element *) 0)
		return;
	newstate = (struct state *) 0;
	definevalues = newenv->def->values;
	textwrite = (textwrite == USER_TEXT) ? MONK_DB : textwrite;
	do {
		while (define->cdl != (struct cond_def_el *) 0) {
			cdl = define->cdl;
			if (iscondtrue(cdl->attribute,cdl->value,cdl->type))
				break;
			else
				define = cdl->next_on_fail;
		}
#ifdef MONK_SAVE
zzz if monksave,remember
save = name (then if save != 0, call read utext til close_delim)
close_delim, end_envir
#endif
		if (define->troff == (char *) 0) {
			/* should not spew error messages if internal def */
			use_define(newenv, define, textwrite);
			continue;
		}
		if (textwrite != NO_WRITE) {
			if (definevalues == (struct value *) 0
						&& define->special == FALSE)
				textputs(define->troff, textwrite);
			else {
				modtext = replace_args(define->troff,
					define->special, definevalues, newenv);
				textputs(modtext, textwrite);
				if (modtext != define->troff)
					myfree(modtext, 44);
			}
		}
		if ((grabit = hashfind(STACK_TABLE, define->attribute))
							== (ENTRY *) NULL)
			continue;
		/* next is next to undo - need to undo the first states last */
		if (newstate == (struct state *) 0) {
			newstate = (struct state *)
						mymalloc(sizeof(struct state));
			newstate->next = (struct state *) 0;
		} else {
			tempstate = (struct state *)
						mymalloc(sizeof(struct state));
			tempstate->next = newstate;
			newstate = tempstate;
		}
		newstate->attribute = define->attribute;
	/* attribute values are not replaced with define_level values */
		newstate->value = define->value;
		newstate->allocated = FALSE;
		abstroff(newstate);
	/* previous == previous instance of this stacking attribute */
		statestack = (struct statestack *) grabit->data;
		if (statestack->first = (struct state *) 0) {
			newstate->previous = (struct state *) 0;
			statestack->first = newstate;
		} else
			newstate->previous = statestack->last;
		statestack->last = newstate;
	} while ((define = define->next) != (struct def_element *) 0);
	newenv->state_list = newstate;
}


use_define(env, define, textwrite)
struct environment	*env;
register struct def_element	*define;
short	textwrite;
{
	struct environment	*temp_env, *old_env;

	temp_env = begin_envir(env, define->attribute, (char) 0, textwrite);
	if (temp_env != env) {
		old_env=end_envir(temp_env, TOP_LEVEL);
		if (env != old_env)
			warn_me(PR_FILENAME | PR_LINENUMBER,
				"PANIC: temp_env changed ongoing env stack\n");
	}
}

void
abstroff(state)
register struct state	*state;
{
	struct value	*oldv, *v, *newv;
	int	arg, n;
	char	*p;
	char	temp[SAFESIZ];

	if ((v = state->value) == (struct value *) 0)
		return;
	do {
		if (mindex(v->value, "+-") == 0)
			continue;
		state->allocated = TRUE;
	} while ((v = v->next) != (struct value *) 0);
	if (state->allocated == FALSE)
		return;
	oldv = state->value;
	do {
		newv = (struct value *) mymalloc(sizeof(struct value));
		if (state->value == oldv)	
			state->value = newv;
		else
			v->next = newv;
		v = newv;
		/* now could (*oldv->value == '+' || *old->value == '-') */
		if ((n = mindex(oldv->value, "+-")) != 0) {
			arg = atoi(oldv->value);
			if (state->previous != (struct state *) 0) 	
				arg += atoi(state->previous->value->value);
			p = temp;
			sprintf(p, "%d", arg);
		} else
			p = oldv->value;	
		/* always allocate the string; faciliates later free */
		v->value = mymalloc(strlen(p)+1);
		strcpy(v->value, p);
	} while ((oldv = oldv->next) != (struct value *) 0);
	v->next = (struct value *) 0;
}

/* end_envir:
	restore previous environment
	generate end_def troff
 */

struct environment *
end_envir(env, level)
struct environment	*env;
int	level;
{
	struct environment	*newenv;

	if (level == TOP_LEVEL)
		/* pop top_environment and any associated sub_envir*/
		do {
			newenv = do_end_envir(env);
			if (newenv == env)
				break;
			env = newenv;
		} while (env->lastinstance == (struct environment *) SUB_LEVEL);
	else
		env = do_end_envir(env);
	return(env);
}

struct environment *
do_end_envir(env)
struct environment	*env;
{
	ENTRY	*grabit;
	register struct state	*s;
	struct state	*ps;
	struct statestack	*states;
	register struct definition	*d;
	register struct def_element	*define;
	char	*troff_string;

	for (s = env->state_list; s != (struct state *) 0; s = s->next) {
		/* skip if there is another instance of this attribute in the
			list to restore: avoids mulitple restores of font */
		if (isanother(s) == TRUE)
			continue;
		/* restore Previous state */
		grabit = hashfind(STACK_TABLE, s->attribute);
		if (grabit == (ENTRY *) NULL) {
			warn_me(PR_FILENAME | PR_LINENUMBER,
				"End_envir: %s not in stack!\n", s->attribute );
			continue;
		}
		states = (struct statestack *) grabit->data;
		if ((ps = s->previous) == (struct state *) 0) {
			/* no previous to restore */
			states->first = states->last = (struct state *) 0;
			continue;
		}
		states->last = ps;
		troff_string = trans_state(ps);
		textputs(troff_string, MONK_DB);
	}
	d = (struct definition *) env->def;
	define = d->end_def;
	if (define != (struct def_element *) 0)
		do {
			if (define->troff == (char *) 0)
				use_define(env, define, MONK_DB);
			else
				if (d->values == (struct value *) 0
						&& define->special == FALSE)
					textputs(define->troff, MONK_DB);
				else {
					troff_string =
						replace_args(define->troff,
						define->special, d->values,env);
					textputs(troff_string, MONK_DB);
					if (troff_string != define->troff)
						myfree(troff_string, 44);
				}
		} while ((define = define->next) != (struct def_element *) 0);
	/* previous NULL if end invoked before any other command, incl make */
	if (env->previous == (struct environment *) 0)
		return(env);
	if (env->lastinstance != (struct environment *) SUB_LEVEL)
		env->def->instance = env->lastinstance;
	free_env(env);
	return(env->previous);
}

isanother(s)
struct state	*s;
{
	struct state	*s2;

	for (s2 = s->next; s2 != (struct state *) 0; s2 = s2->next)
		if (strcmp(s->attribute, s2->attribute) == 0)
			return(TRUE);
	return(FALSE);
}

/* now use this one stack for both environment structures and state change info */

struct environment *
init_envir()
{
	struct environment	*env;

	/* run constructed initialize */
	checkifmissing();
	init_statestack();
	env = begin_envir((struct environment *)0,INIT_STACK,(char) 0,NO_WRITE);
	env = begin_envir(env, INIT_TROFF, (char) 0, MONK_DB);
	env = begin_envir(env, MONK_TEXT, (char) 0, MONK_DB);
	return(env);
}

end_allenvir(env)
struct environment	*env;
{
	if (env == (struct environment *) 0)
		return;
	for(;;) {
		if (*env->def->name != COLON)
			warn_user(0, "No end for %s begun at line %d\n",
					env->def->name, env->linenumber);
		if (env->previous == (struct environment *) 0) {
			end_envir(env, TOP_LEVEL);
			return;
		}
		env = end_envir(env, TOP_LEVEL);
	}
}

/* build hash table of stacking variables from attribute hash table */
init_statestack()
{
	struct table_info	*gethashtbl(), *t;
	ENTRY	*p, *pe;
	struct attribute_info	*ai;

	t = gethashtbl(ATT_TABLE);
	for (p = t->start, pe = p + t->length; p < pe; ++p)
		if (p->key != NULL) {
			ai = (struct attribute_info *) p->data;
			if (ai->stacking == TRUE)
				add_state(p->key);
		}
}

add_state(attribute)
char	*attribute;
{
	ENTRY	*stack_att;
	struct statestack	*s;

	if (hashfind(STACK_TABLE, attribute) == (ENTRY *) NULL) {
		stack_att = (ENTRY *) mymalloc(sizeof(ENTRY));
		stack_att->key = attribute;
		stack_att->data = mymalloc(sizeof(struct statestack));
		s = (struct statestack *) stack_att->data;
		s->first = (struct state *) 0;
		s->last = (struct state *) 0;
		hashenter(STACK_TABLE, stack_att);
	}
}
