#include	<stdio.h>
#include	<ctype.h>
#include	"search.h"
#include	"warn.h"
#include	"rd.h"

short	database_mode;

struct environment *
read_userfile(filename, database_path, env)
char	*filename, *database_path;
struct environment	*env;
{
	FILE	*userfile;

	/* do i really want to exit on fail, this routine called for
						inserted files */
	if (filename != (char *) 0) {
		if ((userfile = fopenncheck(filename, "r", NO_EXIT))
							== (FILE *) NULL)
			return(env);
	} else
		userfile = stdin;
#ifdef TIMING
	newtime("Start processing userfile %s:\t", filename);
#endif
	if (env == (struct environment *) 0)
		env = read_usertop(userfile, database_path);
	if (env != (struct environment *) 0)
		env = read_usertext(userfile, database_path, env);
	fclosencheck(userfile);
#ifdef TIMING
	newtime("Finished processing userfile %s:\t", filename);
#endif
	return(env);
}

struct environment *
read_usertop(userfile, database_path)
FILE	*userfile;
char	*database_path;
{
	ENTRY	*grabit;
	struct definition	*d;
	struct environment	*env, *newenv;
	int	command;
	int	c, close_delim;
	char	*key, *troff_string;

	if ((c = mygetc(userfile)) == EOF)
		return((struct environment *) 0);
	if (c != BEGIN_PRIMITIVE) {
		myungetc(c, userfile);
		read_databases(NULL, database_path);
		return(init_envir());
	}
	key = read_token(userfile, NO_SPACE);
	if ((command = isacommand(key)) != N_MAKE)
		if (command != N_INSERT) {
			read_databases(NULL, database_path);
			env = init_envir();
		}
	if (key == (char *) EOF || key == (char *) 0) {
		textputc(BEGIN_PRIMITIVE, userfile);
		return(env);
	}
	if ((close_delim = read_userdelim(userfile)) == (char) EOF)
		close_delim = (char) FALSE;
	if (command == NOT_ACOMMAND) {
		newenv = begin_envir(env, key, close_delim, USER_TEXT);
		/* To handle env wo delimiters, eg |paragraph */
		if (newenv != env) {
			env = newenv;
			/* Handle environ wo delimiters, eg |paragraph
					don't look at sub_def */
			if (close_delim != (char) FALSE) {
				if (env->def->sub_def != NULL)
					env = assoc_envir(userfile, env,
							env->def, close_delim,
							END_NOW, USER_TEXT);
			} else
				env = end_envir(env, TOP_LEVEL);
		}
		return(env);
	}
	/* could loop looking for make given a comment.... */
	if (command != N_COMMENT) {
		if (command != N_INSERT)
			key = read_token(userfile, OK_SPACE);
		else
			key = read_filename(userfile, OK_SPACE);
		if ((c = read_nonblank(userfile)) == close_delim)
				close_delim = 0;
		else
			if (c != ITEM_SEP) {
				wrong_delim(key, close_delim, c);
				/* put back any char for health?? */
				if (c == BEGIN_PRIMITIVE)
					myungetc(c, userfile);
			}
	}
	switch(command) {
		case N_INSERT:
			env = read_userfile(key, database_path, NULL);
			if (env == NULL) {
				read_databases(NULL, database_path);
				return(init_envir());
			}
			return(env);
		case N_MAKE:
			read_databases(key, database_path);
			return(begin_envir(init_envir(), DOC_TEXT,
							(char) 0, MONK_DB));
	}
	return(do_envcommands(command, key, database_path, userfile,
							env, close_delim));
}

struct environment *
read_usertext(userfile, database_path, env)
FILE	*userfile;
char	*database_path;
struct environment	*env;
{
	ENTRY	*grabit;
	struct definition	*d;
	struct environment	*newenv;
	int	command;
	int	c, close_delim;
	char	lastchar_in;
	char	*key;

	if ((c = mygetc(userfile)) != NEW_LINE)
		myungetc(c, userfile);
	while ((c = mygetc(userfile)) != EOF) {
		if (lastchar_in == '\\') {
			/* '\' special character only relative to '|' */
			if ((lastchar_in = c) != BEGIN_PRIMITIVE)
				textputc('\\', USER_TEXT);
			if ( c != '\\')
				textputc(c, USER_TEXT);
			continue;
		}
		if (c == '\\') {
			lastchar_in = c;
			continue;
		}
		if (c != BEGIN_PRIMITIVE) {
			if ( env->how_to_end != (char) 0 &&
					(char) c == env->how_to_end ) {
				env = end_envir(env, TOP_LEVEL);
				nl_nlgobble(lastchar_in, userfile);
			} else {
				textputc(c, USER_TEXT);
				lastchar_in = c;
			}
			continue;
		}
		if ((key = read_token(userfile, NO_SPACE)) == (char *) EOF)
			break;
		if (key == (char *) 0) {
			textputc(BEGIN_PRIMITIVE, USER_TEXT);
			lastchar_in = BEGIN_PRIMITIVE;
			continue;
		}
		/* gobbles white space looking for open_delimiter, returns
					matching close_delimiter */
		if ((close_delim = read_userdelim(userfile)) == EOF)
			close_delim = (char) FALSE;
		if ((command = isacommand(key)) == NOT_ACOMMAND) {
			/* short form of environment invocation */
			newenv = begin_envir(env, key, close_delim, USER_TEXT);
			if (newenv != env) {
				env = newenv;
				/* Handle environ wo delimiters, eg |paragraph
					don't look at sub_def */
				if (close_delim != (char) FALSE) {
					if (env->def->sub_def != NULL)
						env = assoc_envir(userfile,
							env, env->def,
							close_delim,
							END_NOW, USER_TEXT);
				} else
					env = end_envir(env, TOP_LEVEL);
			}
			nl_nlgobble(lastchar_in, userfile);
			continue;
		}
		if (command != N_COMMENT) {
			if (command != N_INSERT)
				key = read_token(userfile, OK_SPACE);
			else
				key = read_filename(userfile, OK_SPACE);
			if ((c = read_nonblank(userfile)) == close_delim)
				close_delim = 0;
			else
				if (c != ITEM_SEP) {
					wrong_delim(key, close_delim, c);
					/* put back any char for health?? */
					if (c == BEGIN_PRIMITIVE)
						myungetc(c, userfile);
				}
		}	
		env = do_envcommands(command, key, database_path, userfile,
							env, close_delim);
		nl_nlgobble(lastchar_in, userfile);
	}
	return(env);
}

void
nl_nlgobble(lastchar_in, userfile)
char	lastchar_in;
FILE	*userfile;
{
	int	c;

	if (lastchar_in == NEW_LINE)
		if ((c = mygetc(userfile)) != EOF && (char) c != NEW_LINE)
			myungetc(c, userfile);
}

read_databases(doctype, database_path)
char	*doctype, *database_path;
{
	char	*temp = NULL;

	init_dbread();
#ifdef TIMING
	newtime("Starting to read databases:");
#endif
	if (database_mode == COMPRESSED) {
		if (doctype == NULL)
			read_dbfile(DB_COMPRESSED, database_path, COMPRESSED);
		else {
			temp = strconcat(doctype, DB_COMP_EXT);
			read_dbfile(temp, database_path, COMPRESSED);
		}
#ifdef TIMING
		newtime("Finished reading compressed database:");
#endif
	} else {
		read_dbfile(DB_ATTRIBUTES, database_path, STANDARD);
#ifdef TIMING
		newtime("Finished reading attribute database:");
#endif
		read_dbfile(DB_DEFINITIONS, database_path, STANDARD);
#ifdef TIMING
		newtime("Finished reading definition database:");
#endif
		read_dbfile(DB_MACROS, database_path, STANDARD);
#ifdef TIMING
		newtime("Finished reading macro database:");
#endif
		if (doctype != NULL) {
			temp = strconcat(doctype, DB_DOC_EXT);
			read_dbfile(temp, database_path, STANDARD);
#ifdef TIMING
			newtime("Finished reading document %s database:",
								doctype);
#endif
		}
	}
	if (temp != (char *) 0)
		myfree(temp, 12);
}

read_dbfile(datafilename, database_path, mode)
char	*datafilename, *database_path;
int	mode;
{
	FILE	*database;

	database = fopendb(datafilename, database_path, "r", EXIT_ON_FAIL);
	if (mode == STANDARD)
		read_entries(database, database_path);
	else if (mode == COMPRESSED)
		/* read buffers: reads compressed file and macro file */
		read_buffers(database, database_path);
	fclosencheck(database);
}

struct environment	*
do_envcommands(command, name, database_path, file, env, close_delim)
int	command;
char	*name, *database_path;
FILE	*file;
struct environment	*env;
int	close_delim;
{
	struct definition	*def;
	char	c;

							/* textwrite? */
	switch(command) {
		case N_BEGIN:
			def = get_envir(name, close_delim, USER_TEXT);
			if (def == (struct definition *) 0)
				break;
			if (def->sub_def != (struct definition *) 0
					&& close_delim != 0) {
				env = assoc_envir(file, env, def,
					close_delim, END_AT_END, USER_TEXT);
				if ((c = read_nonblank(file)) !=  close_delim) {
					wrong_delim(name, close_delim, c);
					myungetc(c, file);
				}
			}
			env = do_envir(env, def, TOP_LEVEL, (char)0, USER_TEXT);
			break;
		case N_END:
			/* if key doesn't match next expected ??? */
			if (env->how_to_end != (char) 0
					|| strcmp(name, env->def->name) != 0) {
				if (*env->def->name == COLON)
					warn_user(PR_FILENAME | PR_LINENUMBER,
					"Unexpected end for %s\n", name);
				else {
					warn_user(PR_FILENAME | PR_LINENUMBER,
					"Expecting end for %s begun at line %d",
					env->def->name, env->linenumber);
					warn_user(0,"; got end for %s\n", name);
				}
#ifdef HEALTH
				env_health(env, name);
#endif
			} else
				env = end_envir(env, TOP_LEVEL);
			break;
		case N_INSERT:
			env = read_userfile(name, database_path, env);
			break;
		case N_MAKE:
			make_doc(name, database_path);
			env = begin_envir(env, DOC_TEXT, (char)0, MONK_DB);
			break;
		default:
			do_commands(command, name, database_path, file,
								close_delim);	
	}
	return(env);
}

struct environment *
make_doc(doctype, database_path)
char	*doctype, *database_path;
{
	char	*temp;

	temp = strconcat(doctype, DB_DOC_EXT);
	read_dbfile(temp, database_path, STANDARD);
#ifdef TIMING
	newtime("Finished reading document %s database:", doctype);
#endif
}
