#include	<stdio.h>
#include	<ctype.h>
#include	"search.h"
#include	"warn.h"
#include	"rd.h"
#include	"dbcompress.h"

#define	USAGE	"compress [ -d db_dir ] doctype\n"
#define	OPTIONS	"d:"

/* Read ascii database files and write compressed database buffers:
	hash structures for attributes;
	hash structures for definitions;
	attribute_info structures;
	attribute_case structures;
	definition structures;
	def_el structures;
	value structures;
	text strings;
 */
struct db_buffer	db_struct[BUF_NUMBER];

main(argc, argv)
int	argc;
char	**argv;
{
	extern int	optind;
	extern char	*optarg;

	int	c;
	char	*database_out, *database_path;
	char	*doctype;
	char	*temp;

	database_path = DB_PATH;
	while ((c = getopt(argc, argv, OPTIONS)) != EOF )
		switch (c) {
			case 'd':
				database_path = optarg;
				break;
			case '?':
				warn_user(0, "USAGE: %s; no option %c\n",
								USAGE, c);
		}
	if ((argc - optind) > 0) {
		doctype = argv[optind];
		database_out = strconcat(doctype, DB_COMP_EXT);
	} else {
		doctype = (char *) 0;
		database_out = DB_COMPRESSED;
	}
	init_dbread();
#ifdef TIMING
	newtime("Starting to read databases\n");
#endif
	read_asciidb(DB_ATTRIBUTES, database_path);
#ifdef TIMING
	newtime("Finished reading attribute database:");
#endif
	read_asciidb(DB_DEFINITIONS, database_path);
#ifdef TIMING
	newtime("Finished reading definition database:");
#endif
	read_asciidb(DB_MACROS, database_path);
#ifdef TIMING
	newtime("Finished reading macro database:");
#endif
	if (doctype != (char *) 0) {
		temp = strconcat(doctype, DB_DOC_EXT);
		read_asciidb(temp, database_path);
		myfree(temp, 100);
#ifdef TIMING
		newtime("Finished reading document database:");
#endif
	}
	check_internal_defines(doctype);
	write_buffers(database_out, database_path);
	exit(0);
}

write_buffers(filename, database_path)
char	*filename, *database_path;
{
	FILE	*cdatabase;
	int	buf_size[BUF_NUMBER], buf_number, n;

	/* fix pointers in buffers - need to store offsets for easy read */
	for (buf_number = 0; buf_number < BUF_NUMBER; ++buf_number) {
		switch(buf_number) {
			case BUF_HASH_ATT:
			case BUF_HASH_DEF:
				subhash(buf_number, db_struct, buf_size);
				break;
			case BUF_ATT_INFO:
				subattinfo(buf_number, db_struct, buf_size);
				break;
			case BUF_ATT_CASE:
				subattcase(buf_number, db_struct, buf_size);
				break;
			case BUF_DEF:
				subdef(buf_number, db_struct, buf_size);
				break;
			case BUF_DEF_EL:
				subdef_el(buf_number, db_struct, buf_size);
				break;
			case BUF_COND_DEF_EL:
				subcond_def_el(buf_number, db_struct, buf_size);
				break;
			case BUF_VALUE:
				subvalue(buf_number, db_struct, buf_size);
				break;
			case BUF_TEXT:
				buf_size[buf_number] =
						db_struct[buf_number].current
						- db_struct[buf_number].top;
				break;
			default:
				fprintf(stderr,
				"Compressing db: buffer %d out of range\n",
								buf_number);
		}
	}
	for (buf_number = 0; buf_number < BUF_NUMBER; ++buf_number)
		warn_db(0, "Buffer %d: size %d\n", buf_number,
							buf_size[buf_number]);
	cdatabase = fopendb(filename, database_path, "w", EXIT_ON_FAIL);
	fwrite(buf_size, sizeof(int), BUF_NUMBER, cdatabase);
	for (buf_number = 0; buf_number < BUF_NUMBER; ++buf_number) {
		if ((n = buf_size[buf_number]) == 0)
			continue;
		if (fwrite(db_struct[buf_number].top, 1, n, cdatabase) == 0) {
			fprintf(stderr, "Compress: cannot write buffer %d\n",
							buf_number);
			return(0);
		}
	}
	return(1);
}

/* take out the bufmalloc in the read_def and read_att!!! */

subhash(buf_number, db_struct, buf_size)
short	buf_number;
struct db_buffer	db_struct[];
int	buf_size[];
{
	DBENTRY	*entry, *p, *pe;
	struct table_info	*gethashtbl(), *t;
	char	*top;

	entry = (DBENTRY *) db_struct[buf_number].top;
#ifdef OLDMALLOC
#endif
	if (buf_number == BUF_HASH_ATT) {
		t = gethashtbl(ATT_TABLE);
		top = db_struct[BUF_ATT_INFO].top;
	} else
		if (buf_number == BUF_HASH_DEF) {
			t = gethashtbl(DEF_TABLE);
			top = db_struct[BUF_DEF].top;
		} else
			fprintf(stderr,
				"Error in compressed hash %d\n", buf_number);
/* redefinitions are held in hash table and not reflected in my buffer;
		get info from hash and rebuild buffer */
	for (p = (DBENTRY *) t->start, pe = p + t->length; p < pe; ++p)
		if (p->key.p != NULL) {
#ifdef NEWMALLOC
			entry = (DBENTRY *) bufmalloc(buf_number,
							sizeof(DBENTRY));
#endif
			entry->key.i = GET_OFFSET(p->key.p,
						db_struct[BUF_TEXT].top);
			entry->data.i= GET_OFFSET(p->data.p, top);
			++entry;
#ifdef OLDMALLOC
#endif
		}
	buf_size[buf_number] = db_struct[buf_number].current
						- db_struct[buf_number].top;
}

subattinfo(buf_number, db_struct, buf_size)
short	buf_number;
struct db_buffer	db_struct[];
int	buf_size[];
{
	struct dbattribute_info	*dbai;
	short	nitems;

	dbai = (struct dbattribute_info *) db_struct[buf_number].top;
	buf_size[buf_number] = db_struct[buf_number].current
						- db_struct[buf_number].top;
	nitems = buf_size[buf_number] / sizeof(struct dbattribute_info);

	while (nitems--) {
		dbai->firstcase.icase = GET_OFFSET(dbai->firstcase.acase,
			(struct attribute_case *) db_struct[BUF_ATT_CASE].top);
		++dbai;
	}
}

subattcase(buf_number, db_struct, buf_size)
short	buf_number;
struct db_buffer	db_struct[];
int	buf_size[];
{
	struct dbattribute_case	*ac;
	short	nitems;

	ac = (struct dbattribute_case *) db_struct[buf_number].top;
	buf_size[buf_number] = db_struct[buf_number].current
						- db_struct[buf_number].top;
	nitems = buf_size[buf_number] / sizeof(struct dbattribute_case);

	while (nitems--) {
		ac->troff.istring = GET_OFFSET(ac->troff.string,
						db_struct[BUF_TEXT].top);
		ac->value.ivalue = GET_OFFSET(ac->value.value,
				(struct value *) db_struct[BUF_VALUE].top);
		ac->next.icase = GET_OFFSET(ac->next.acase,
			(struct attribute_case *) db_struct[BUF_ATT_CASE].top);
		++ac;
	}
}

subdef(buf_number, db_struct, buf_size)
short	buf_number;
struct db_buffer	db_struct[];
int	buf_size[];
{
	struct dbdefinition	*d;
	short	nitems;

	d = (struct dbdefinition *) db_struct[buf_number].top;
	buf_size[buf_number] = db_struct[buf_number].current
						- db_struct[buf_number].top;
	nitems = buf_size[buf_number] / sizeof(struct dbdefinition);

	while (nitems--) {
		d->name.istring = GET_OFFSET(d->name.string,
						db_struct[BUF_TEXT].top);
		d->values.ivalue = GET_OFFSET(d->values.value,
				(struct value *) db_struct[BUF_VALUE].top);
		d->begin_def.idef_el = GET_OFFSET(d->begin_def.def_el,
			(struct def_element *) db_struct[BUF_DEF_EL].top);
		d->end_def.idef_el = GET_OFFSET(d->end_def.def_el,
			(struct def_element *) db_struct[BUF_DEF_EL].top);
		d->sub_def.idef = GET_OFFSET(d->sub_def.def,
			(struct definition *) db_struct[BUF_DEF].top);
		++d;
	}
}

subdef_el(buf_number, db_struct, buf_size)
short	buf_number;
struct db_buffer	db_struct[];
int	buf_size[];
{
	struct dbdef_el	*d;
	short	nitems;

	d = (struct dbdef_el *) db_struct[buf_number].top;
	buf_size[buf_number] = db_struct[buf_number].current
						- db_struct[buf_number].top;
	nitems = buf_size[buf_number] / sizeof(struct dbdef_el);

	while (nitems--) {
		d->attribute.istring = GET_OFFSET(d->attribute.string,								db_struct[BUF_TEXT].top);
		d->troff.istring = GET_OFFSET(d->troff.string,
						db_struct[BUF_TEXT].top);
		d->value.ivalue = GET_OFFSET(d->value.value,
				(struct value *) db_struct[BUF_VALUE].top);
		d->cdl.icond_def_el = GET_OFFSET(d->cdl.cond_def_el,
			(struct cond_def_el *) db_struct[BUF_COND_DEF_EL].top);
		d->next.idef_el = GET_OFFSET(d->next.def_el,
			(struct def_element *) db_struct[BUF_DEF_EL].top);
		++d;
	}
}

subcond_def_el(buf_number, db_struct, buf_size)
short	buf_number;
struct db_buffer	db_struct[];
int	buf_size[];
{
	struct dbcond_def_el	*d;
	short	nitems;

	d = (struct dbcond_def_el *) db_struct[buf_number].top;
	buf_size[buf_number] = db_struct[buf_number].current
						- db_struct[buf_number].top;
	nitems = buf_size[buf_number] / sizeof(struct dbcond_def_el);

	while (nitems--) {
		d->attribute.istring = GET_OFFSET(d->attribute.string,								db_struct[BUF_TEXT].top);
		d->value.ivalue = GET_OFFSET(d->value.value,
				(struct value *) db_struct[BUF_VALUE].top);
		d->next_on_fail.idef_el = GET_OFFSET(d->next_on_fail.def_el,
			(struct def_element *) db_struct[BUF_DEF_EL].top);
		++d;
	}
}

subvalue(buf_number, db_struct, buf_size)
short	buf_number;
struct db_buffer	db_struct[];
int	buf_size[];
{
	struct dbvalue	*v;
	short	nitems;

	v = (struct dbvalue *) db_struct[buf_number].top;
	buf_size[buf_number] = db_struct[buf_number].current
						- db_struct[buf_number].top;
	nitems = buf_size[buf_number] / sizeof(struct dbvalue);

	while (nitems--) {
		v->value.istring = GET_OFFSET(v->value.string,
					db_struct[BUF_TEXT].top);
		v->next.ivalue = GET_OFFSET(v->next.value,
				(struct value *) db_struct[BUF_VALUE].top);
		++v;
	}
}

/* check that internal definitions are all defined */
	/* monk envir at beginning of usertext*/
	/* monk envir at end of usertext*/
	/* envir at beg of usertext*/
	/* envir invoked at end of usertext */
	/* handles multiple user nls */
	/* handles whitespace following newline */

check_internal_defines(doctype)
char	*doctype;
{
	if (hashfind(DEF_TABLE, MONK_TEXT) == (ENTRY *) NULL)
		warn_db(0, "Internal definition %s is not defined\n",
								MONK_TEXT);
	if (hashfind(DEF_TABLE, USER_NL_NL) == (ENTRY *) NULL)
		warn_db(0, "Internal definition %s is not defined\n",
								USER_NL_NL);
	if (hashfind(DEF_TABLE, USER_NL_WHITE) == (ENTRY *) NULL)
		warn_db(0, "Internal definition %s is not defined\n",
								USER_NL_WHITE);
	if (doctype != (char *) 0) {
		if (hashfind(DEF_TABLE, DOC_TEXT) == (ENTRY *) NULL)
			warn_db(0, "Internal definition %s is not defined\n",
								DOC_TEXT);
	}
}
