#define register

/* database files */

#define	DB_PATH		"../db"			/* default */
#define	SLASH_STRING	"/"
#define	COMPRESSED	1
#define	STANDARD	2
#define	DB_ATTRIBUTES	"attrib.def"
#define	DB_DEFINITIONS	"global.def"
#define	DB_MACROS	"macro.def"
#define	DB_COMPRESSED	"global.comp"
#define	MACRO_COMP	"macro.comp"
#define	DB_COMP_EXT	".comp"
#define	DB_DEF_EXT	".def"			/* extension for document types,
							e.g. letter.def */
#define	DB_DOC_EXT	".doc"			/* extension for document types,
							e.g. letter.doc */
#define	DB_SAMPLE_EXT	".sample"		/* extension for type samples */

/* hash tables: _TABLE is table number, _ENTRIES is maximum # of entries*/
					/* database tables */
#define	ATT_TABLE	0	/* attributes - indent, blank.lines, new.page */
#define	ATT_ENTRIES	200
#define	DEF_TABLE	1	/* definitions - paragraph, address, chapter */
#define	DEF_ENTRIES	200
					/* run-time tables */
#define	STACK_TABLE	2	/* stacking attributes - restore environments */
#define	STACK_ENTRIES	100
#define	STORE_TABLE	3	/* monk saves strings, numbers for db author*/
#define	STORE_ENTRIES	100

#define	SETUP_TIME	0
#define	RUN_TIME	1

/* database_setup keywords: definition primitives */

#define	SAFESIZ		BUFSIZ*4

#define	STACKING	"stack"
#define	DEFAULT		"default"
#define	INIT_STACK	":init_stack"	/* initialize internal stack */
#define	INITIALIZE	"init"
#define	INIT_TROFF	":init_troff"	/* pass to initialize troff */
#define	MONK_TEXT	":common" 	/* monk envir - beg&end usertext*/
#define	DOC_TEXT	":document"	/* doc envir - beg&end of usertext*/
#define	MONK_NEWLINE	"monk_nl"	/* handles multiple user newlines */
#define	SPECIAL_IN_TROFF	'_'	/* first character of all special */
#define	FILENAME	"__FILE__"
#define	LINENUMBER	"__LINE__"
#define	USER_NL_NL	":document_blankline"	/* handles multiple user nls */
#define	USER_NL_WHITE	":document_newline_whitespace"
				/* handles whitespace following newline */
#define	BLANKSTRING	" "
#define	COLON		':'		/* internal names begin with colon */

#define	DEF_ARGUMENT	'$'
#define	REGEX_CHAR	'$'
#define	XPAND_CHAR	'*'
#define	FIXED_ARG	2
#define	VAR_ARG		3
#define	REGEX_ARG	4
#define	XPAND_ARG	999

#define	EMBED_IN_TROFF	'|'
#define	FOR_LOOP	"for"
#define FOR_ARGS	"in"
#define	TOP_OF_FOR_LOOP	"|for "		/* used for reconstruction - must match
					EMBED_IN_TROFF, FOR_LOOP and FOR_ARGS */
#define	IF_VALUE	"ifvalue"
#define	IF_NOTVALUE	"ifnotvalue"

#define	MONKSAVE	"monksave"	/* store string from user text */
#define	MONKREMEMBER	"monkremember"	/* process and write string to output */

#define	ASSOCIATE	"associate"
#define	ATTRIBUTE	"attribute"
#define	COMMENT		"comment"
#define	ENVIRONMENT	"environment"
#define	INSERT		"insert"
#define	MACRO		"macro"

/* integer equivalents for commands */
#define	NOT_ACOMMAND	0
#define	N_BEGIN		1
#define	N_END		2
#define	N_INSERT	3
#define	N_MAKE		4
#define	N_ASSOCIATE	5
#define	N_ATTRIBUTE	6
#define	N_COMMENT	7
#define	N_ENVIRONMENT	8
#define	N_MACRO		9

/* database_use keywords */

#define	BEGIN		"begin"
#define	END		"end"
#define	MAKE		"make"

#define	BEGIN_PRIMITIVE	'|'
#define	LIST_SEP	';'
#define	ITEM_SEP	','
#define	NEW_LINE	'\n'
#define	BLANK		' '
#define	TAB		'\t'

#define	NSUBSTITUTE	4
				/* argument specifies source of text
					0 = donot write it out */
#define	NO_WRITE	0
#define	USER_TEXT	1	/* source for text - enduser file */
#define	MONK_DB		2	/* source for text - monk databases */
#define	FORCE		3	/* unconditionally spew text (omit test \n\n) */

#define	NO_SPACE	FALSE	/* space permitted in parsing: b4 token */
#define	OK_SPACE	TRUE

#define	END_NOW		0	/* when to end a sub-environment */
#define	END_AT_END	1

#define	TRUE		1
#define	FALSE		0
#define	EXIT_ON_FAIL	1
#define	EXIT		1
#define	NO_EXIT		0
#define	NOT(x)		(int) x - 1

#define	STR_SIZE	48

#define	LEAD_WHITE	0001
#define	TRAIL_WHITE	0002
#define	WITHIN_WHITE	0004

#define	ATOI		060

#define	TOP_LEVEL	1
#define	SUB_LEVEL	-1

/* iswhite: isspace but not '\n' */
#define	iswhite(c)		(c == BLANK || c == TAB)

#ifdef FIGURE_IT_OUT
#define	mygetc(file)		((c = getc(file)) == '\n') ?\
					linenumber(INCREMENT), c : c

#define	myungetc(c, file)	(c == '\n') ?\
					linenumber(DECREMENT),ungetc(c, file) :\
					ungetc(c, file)
#endif
/* hash data for attribute definition
 */

struct attribute_info {				/* see dbcompress.h */
	short	stacking;
	struct attribute_case	*firstcase;
};

struct attribute_case {			/* see dbcompress.h */
	short	nvalues;	/* number of arguments - $* counts as one */
	short	expanding;	/* are there any cases with expanding values? */
	short	special;	/* are there any special cookies in troff,
						e.g. __LINE__, __FILE__ */
	short	allocated;
	char	*troff;
	struct value	*value;
	struct attribute_case	*next;
};

/* hash data for each database definition:
	struct definition pointing to linked list of def_elements
 */
struct value {			/* see dbcompress.h */
	char	*value;			/* value identifier */
	short	type;			/* fixed, variable, expanding value */
	struct value	*next;
};

struct def_element {			/* see dbcompress.h */
	char	*attribute;
	char	*troff;
	struct value	*value;
	short	special;
	short allocated;
	short	stacking;
	struct cond_def_el	*cdl;
	struct def_element	*next;
};

struct definition {			/* see dbcompress.h */
	char	*name;
	struct value	*values;
	struct def_element	*begin_def;
	struct def_element	*end_def;
	struct definition	*sub_def;
	struct environment	*instance;	/* envir of this def */
};

/* state information(absolute values) stacked on entering environment;
	on leaving an environment absolute troff strings from previous environment
	used to restore that previous environment
 */

struct state {
	char	*attribute;		/* attribute to restore, e.g. `indent' */
	struct value	*value;		/* absolute argument, eg. `30' */
	short	allocated;		/* space allocated by malloc */
	struct state	*previous;	/* previous instance of this attribute
								to restore */
	struct state	*next;		/* next state in this environment */
};

struct statestack {
	struct state	*first;
	struct state	*last;
};

struct environment {
	struct definition	*def;
	struct state	*state_list;
	struct environment	*lastinstance;	/* last of this envir envname */
	struct environment	*previous;	/* previous envir in time */
	char	*envname;
	char	*filename;
	short	linenumber;	
	char	how_to_end;			/* how to end current envir */
};

struct conditional {
	short	type;
	char	*attribute;
	struct value	*value;
	char	close_delim;
	char	*newp;
};

struct cond_def_el {
	short	type;
	char	*attribute;
	struct value	*value;
	struct def_element	*next_on_fail;
};

struct loop {
	char	loopchar;
	struct value	*args;
	struct value	*current;
	char	close_delim;
	char	*newp;
};

struct buffer {
	char	*start;
	char	*current;
	char	*empty;
	char	*end;
};

struct valu_str {
	struct value	*value;
	char	*newp;
};

struct page_layout {
	short	top_margin;
	short	bottom_margin;
	short	left_margin;
	short	right_margin;
	char	*left_page_header;
	char	*center_page_header;
	char	*right_page_header;
	char	*left_page_footer;
	char	*center_page_footer;
	char	*right_page_footer;
};

struct got_token {
	char	*newp;
	char	delimiter;
};

struct strings {
	char	*token;
	char	*newp;
};

struct	add_att {
	char	*defname;
	struct value	*value;
	struct add_att	*next;
};

struct	init_def {
	char	*attribute;
	short	stacking;
	struct add_att	*add_att;
};

FILE	*fopenncheck(), *fopendb();
struct attribute_case	*match_case();
struct def_element	*copy_def_el(), *mk_def_el(), *merge_def_el();
struct def_element	*trans_list(), *map_def_el();
struct def_element	*xpand_loop_valu();
struct definition	*get_envir();
struct definition	*read_list();
struct environment	*make_doc(), *init_envir(), *do_envcommands();
struct environment	*read_userfile(), *read_usertop(), *read_usertext();
struct environment	*begin_envir(), *assoc_envir(), *do_subenv();
struct environment	*mk_envir(), *do_envir(),*do_end_envir(), *end_envir();
struct file_info	*save_file_info();
struct got_token *ftoken();
struct init_def	*add_att_type();
struct conditional	*isitaconditional();
struct cond_def_el	*readacond();
struct definition	*match_assoc();
struct loop	*readaloop(), *isitaloop();
struct strings	*strtok(), *strarg(), *rename_arg();
struct value	*stratt(), *make_value(), *read_values(), *copy_valu();
struct valu_str	*parse_values();
short	match_arg(), cmp_valu();
char	read_delim(), close_match(), open_match(), read_nonspace();
char	*fillin_loop_valu(), *replace_args(), *bufmalloc();
char	*read_formatcommands(), parsencheck(), *get_file_name();
char	*read_token(), *read_arg(), *read_group(), *read_key(), *read_until();
char	*read_buftoken(), *read_bufarg(), *read_bufname(), *read_filename();
char	*fdelim(), *checkputs(), *checkif(), *strindex(), *trans_state();
char	*str_atype(), *str_aspecial(), *str_acase(), *str_avalue(),*str_vtype();
char	*whichargs(), *xpall(), *copy_text_loop();
char	*read_dbtype(), *index(), *strbuild(), *strreplace(), *mstrcopy();
char	*mymalloc(), *myrealloc(), *mk_string(), *strconcat(), *strjoin();
void	abstroff(), make_case(), bubble_case(), dodef(), newstates();
void	read_gobble(), nl_nlgobble(), read_include(), fgobble(), map_args();
void	read_attribute(), read_comment(), read_define(), read_macro();
