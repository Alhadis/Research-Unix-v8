#include	<stdio.h>
#include	<ctype.h>
#include	"search.h"
#include	"warn.h"
#include	"rd.h"
#include	"dbcompress.h"

mk_hashtables()
{
	hcreate(ATT_TABLE, ATT_ENTRIES);
	hcreate(DEF_TABLE, DEF_ENTRIES);
	hcreate(STACK_TABLE, STACK_ENTRIES);
}

read_entries(database, database_path)
FILE	*database;
char	*database_path;
{
	int	command;
	short	messages;
	int	close_delim;
	char	*name, *type;

	messages = TRUE;
	while ((type = read_dbtype(database, messages)) != (char *) EOF) {
		if (type == (char *) 0) {
			warn_db(PR_FILENAME | PR_LINENUMBER,
				"Null primitive %s\n", type);
			continue;
		}
		if ((close_delim = read_delim(database)) == (char) FALSE) {
			messages = FALSE;
			continue;
		}
		if (messages == FALSE) {
			warn_db(0, "\n\t\t\tcontinued gobbling til ");
			warn_db(PR_LINENUMBER, "\n");
			messages = TRUE;
		}
		if ((command = isdbcommand(type)) == NOT_ACOMMAND) {
			warn_db(PR_FILENAME | PR_LINENUMBER,
					"Unknown primitive %s\n", type);
			continue;
		}
		if (command != N_COMMENT) {
			if ((name = read_buftoken(database, OK_SPACE)) == 
								(char *) EOF)
				return;
			if (name == (char *) 0) {
				readncheck(database, close_delim, TRUE, TRUE,
					"Reading %s, null name given", type);
				continue;
			}
			readncheck(database, LIST_SEP, FALSE, TRUE,
						"Reading %s %s", type, name);
		}
		do_dbcommands(command, name, database_path, database,
								close_delim);
	}
}

/* read_dbtype:
	returns type of database entry - attribute, comment, define, macro
 */

char *
read_dbtype(database, messages)
FILE	*database;
short	messages;
{
	static char	*dbtype = (char *) 0;

	readncheck(database, BEGIN_PRIMITIVE, TRUE, messages,
						"Looking for primitive");
	if (dbtype != (char *) 0 && dbtype != (char *) EOF)
		myfree(dbtype, 3);
	dbtype = read_token(database, OK_SPACE);
	return(dbtype);	
}

isacommand(key)
char	*key;
{
	if (key == (char *) 0 || key == (char *) EOF)
		return(NOT_ACOMMAND);
	if (strcmp(key, BEGIN) == 0)
		return(N_BEGIN);
	if (strcmp(key, END) == 0)
		return(N_END);
	if (strcmp(key, MAKE) == 0)
		return(N_MAKE);
	return(isdbcommand(key));
}

isdbcommand(type)
char	*type;
{
	if (type == (char *) 0 || type == (char *) EOF)
		return(NOT_ACOMMAND);
	if (strcmp(type, ASSOCIATE) == 0)
		return(N_ASSOCIATE);
	if (strcmp(type, ATTRIBUTE) == 0)
		return(N_ATTRIBUTE);
	if (strcmp(type, COMMENT) == 0)
		return(N_COMMENT);
	if (strcmp(type, ENVIRONMENT) == 0)
		return(N_ENVIRONMENT);
	if (strcmp(type, INSERT) == 0)
		return(N_INSERT);
	if (strcmp(type, MACRO) == 0)
		return(N_MACRO);
	return(NOT_ACOMMAND);
}

do_dbcommands(command, name, database_path, file, close_delim)
int	command;
char	*name, *database_path;
FILE	*file;
int	close_delim;
{
	do_commands(command, name, database_path, file, close_delim);
	if (command == N_INSERT)
		read_asciidb(name, database_path);
}

do_commands(command, name, database_path, file, close_delim)
int	command;
char	*name, *database_path;
FILE	*file;
int	close_delim;
{
	switch(command) {
		case N_ATTRIBUTE:
			read_attribute(file, name, close_delim);
			break;
		case N_COMMENT:
			read_comment(file, close_delim);
			break;
		case N_ENVIRONMENT:
			read_define(file, name, FALSE, close_delim);
			break;
		case N_ASSOCIATE:
			read_define(file, name, TRUE, close_delim);
			break;
		case N_MACRO:
			read_macro(file, name, database_path, close_delim);
			break;
		default:
			return(FALSE);
		}
	return(TRUE);
}

read_asciidb(datafilename, database_path)
char	*datafilename, *database_path;
{
	FILE	*database;

	database = fopendb(datafilename, database_path, "r", EXIT_ON_FAIL);
	read_entries(database, database_path);
	fclosencheck(database);
}

/* read_comment: gobbles comment
 */

void
read_comment(database, close_delim)
FILE	*database;
int	close_delim;
{
	int	c;

	while (((c = mygetc(database)) != EOF) && c != close_delim);
}

struct value *
read_values(database, delimiters)
FILE	*database;
{
	struct value	*firstv, *v;
	char	*arg;

	for(firstv = (struct value *) 0;
		(arg = read_bufarg(database, delimiters)) != (char *) EOF;) {
		if (arg == (char *) 0)
			break;
		if (firstv == (struct value *) 0) {
			firstv = v = make_value(arg);
		} else {
			v->next = make_value(arg);
			v = v->next;
		}
		v->next = (struct value *) 0;
	}
	return(firstv);
}

struct valu_str *
parse_values(s)
char	*s;
{
	static struct valu_str	vs;
	struct value	*v;
	struct strings	*sting;
	char	*arg;

	for(vs.value = (struct value *) 0;
			(sting = strarg(s, FALSE)) != (struct strings *) 0; ) {
		if ((arg = sting->token) == (char *) 0)
			break;
		s = sting->newp;
		if (vs.value == (struct value *) 0) {
			vs.value = v = make_value(arg);
		} else {
			v->next = make_value(arg);
			v = v->next;
		}
		v->next = (struct value *) 0;
	}
	vs.newp = s;
	return(&vs);
}

struct value *
make_value(newarg)
char	*newarg;
{
	struct value	*newv;

	if (newarg == (char *) 0)
		return((struct value *) 0);
	newv = (struct value *) bufmalloc(BUF_VALUE, sizeof(struct value));
	newv->value = newarg;
	newv->next = (struct value *) 0;
	if (*newarg != DEF_ARGUMENT)
		newv->type = FIXED_ARG;
	else
		if (*++newarg == XPAND_CHAR)
			newv->type = XPAND_ARG;
		else
			if (*newarg == REGEX_CHAR)
				/* cannot compile regular expression until
					after argument substitutions in
					formatting commands */
				newv->type = REGEX_ARG;
			else
				newv->type = VAR_ARG;
	return(newv);
}

