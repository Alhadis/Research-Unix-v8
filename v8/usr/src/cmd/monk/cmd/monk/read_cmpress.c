#include	<stdio.h>
#include	<ctype.h>
#include	"search.h"
#include	"warn.h"
#include	"rd.h"
#include	"dbcompress.h"

/* Read compressed database:
	buffers:
		hash structures for attributes;
		hash structures for definitions;
		attribute_info structures;
		attribute_case structures;
		definition structures;
		def_el structures;
		value structures;
		text strings;
 */

read_buffers(database, database_path)
FILE	*database;
char	*database_path;
{
	int	buf_size[BUF_NUMBER], buf_number, n;
	char	*db_buf[BUF_NUMBER];

	fread(buf_size, sizeof(int), BUF_NUMBER, database);
	for (buf_number = 0; buf_number < BUF_NUMBER; ++buf_number) {
		db_buf[buf_number] = mymalloc(buf_size[buf_number]);
		if ((n = buf_size[buf_number]) == 0)
			continue;
		if ((n = fread(db_buf[buf_number], 1, n, database))
						!= buf_size[buf_number]) {
			fprintf(stderr, "Cannot read compressed %d, read %d\n",
						buf_number, n);
			exit(0);
		}
	}
	for (buf_number = 0; buf_number < BUF_NUMBER; ++buf_number) {
	/* fix pointers in buffers */
		switch(buf_number) {
			case BUF_HASH_ATT:
			case BUF_HASH_DEF:
				fixhash(buf_number, db_buf, buf_size);
				break;
			case BUF_ATT_INFO:
				fixattinfo(buf_number, db_buf, buf_size);
				break;
			case BUF_ATT_CASE:
				fixattcase(buf_number, db_buf, buf_size);
				break;
			case BUF_DEF:
				fixdef(buf_number, db_buf, buf_size);
				break;
			case BUF_DEF_EL:
				fixdef_el(buf_number, db_buf, buf_size);
				break;
			case BUF_COND_DEF_EL:
				fixcond_def_el(buf_number, db_buf, buf_size);
				break;
			case BUF_VALUE:
				fixvalue(buf_number, db_buf, buf_size);
				break;
			case BUF_TEXT:
				break;
			default:
				fprintf(stderr,
				"Reading compressed buffer: %d out of range\n",
							buf_number);
		}
	}
	fixmacros(MACRO_COMP, database_path);
}

fixhash(buf_number, db_buf, buf_size)
short	buf_number;
char	*db_buf[BUF_NUMBER];
int	buf_size[BUF_NUMBER];
{
	DBENTRY	*entry;
	int	hashtable;
	short	nitems;
	char	*data, *text;

	entry = (DBENTRY *) db_buf[buf_number];
	nitems = buf_size[buf_number] / sizeof(ENTRY);

	text = db_buf[BUF_TEXT];
	if (buf_number == BUF_HASH_ATT) {
		data = db_buf[BUF_ATT_INFO];
		hashtable = ATT_TABLE;
	} else
		if (buf_number == BUF_HASH_DEF) {
			data = db_buf[BUF_DEF];
			hashtable = DEF_TABLE;
		} else
			fprintf(stderr,
				"Error in reading database %d\n", buf_number);
	while (nitems--) {
		entry->key.p = ADD_OFFSET(entry->key.i, text);
		entry->data.p = ADD_OFFSET(entry->data.i, data);
		hashenter(hashtable, entry++);
	}
}

fixattinfo(buf_number, db_buf, buf_size)
short	buf_number;
char	*db_buf[BUF_NUMBER];
int	buf_size[BUF_NUMBER];
{
	struct dbattribute_info	*dbai;
	short	nitems;

	dbai = (struct dbattribute_info *) db_buf[buf_number];
	nitems = buf_size[buf_number] / sizeof(struct dbattribute_info);

	while (nitems--) {
		dbai->firstcase.acase = ADD_OFFSET(dbai->firstcase.icase,
			(struct attribute_case *) db_buf[BUF_ATT_CASE]);
		dbai++;
	}
}

fixattcase(buf_number, db_buf, buf_size)
short	buf_number;
char	*db_buf[BUF_NUMBER];
int	buf_size[BUF_NUMBER];
{
	struct dbattribute_case	*ac;
	short	nitems;

	ac = (struct dbattribute_case *) db_buf[buf_number];
	nitems = buf_size[buf_number] / sizeof(struct dbattribute_case);

	while (nitems--) {
		ac->troff.string = ADD_OFFSET(ac->troff.istring,
							db_buf[BUF_TEXT]);
		ac->value.value = ADD_OFFSET(ac->value.ivalue,
				(struct value *) db_buf[BUF_VALUE]);
		ac->next.acase = ADD_OFFSET(ac->next.icase,
			(struct attribute_case *) db_buf[BUF_ATT_CASE]);
		++ac;
	}
}

fixdef(buf_number, db_buf, buf_size)
short	buf_number;
char	*db_buf[BUF_NUMBER];
int	buf_size[BUF_NUMBER];
{
	struct dbdefinition	*d;
	short	nitems;

	d = (struct dbdefinition *) db_buf[buf_number];
	nitems = buf_size[buf_number] / sizeof(struct dbdefinition);

	while (nitems--) {
		d->name.string = ADD_OFFSET(d->name.istring, db_buf[BUF_TEXT]);
		d->values.value = ADD_OFFSET(d->values.ivalue,
				(struct value *) db_buf[BUF_VALUE]);
		d->begin_def.def_el = ADD_OFFSET(d->begin_def.idef_el,
				(struct def_element *) db_buf[BUF_DEF_EL]);
		d->end_def.def_el = ADD_OFFSET(d->end_def.idef_el,
				(struct def_element *) db_buf[BUF_DEF_EL]);
		d->sub_def.def = ADD_OFFSET(d->sub_def.idef,
				(struct definition *) db_buf[BUF_DEF]);
		++d;
	}
}

fixdef_el(buf_number, db_buf, buf_size)
short	buf_number;
char	*db_buf[BUF_NUMBER];
int	buf_size[BUF_NUMBER];
{
	struct dbdef_el	*d;
	short	nitems;

	d = (struct dbdef_el *) db_buf[buf_number];
	nitems = buf_size[buf_number] / sizeof(struct dbdef_el);

	while (nitems--) {
		d->attribute.string = ADD_OFFSET(d->attribute.istring,
						db_buf[BUF_TEXT]);
		d->troff.string = ADD_OFFSET(d->troff.istring,
						db_buf[BUF_TEXT]);
		d->value.value = ADD_OFFSET(d->value.ivalue,
				(struct value *) db_buf[BUF_VALUE]);
		d->cdl.cond_def_el = ADD_OFFSET(d->cdl.icond_def_el,
			(struct cond_def_el *) db_buf[BUF_COND_DEF_EL]);
		d->next.def_el = ADD_OFFSET(d->next.idef_el,
				(struct def_element *) db_buf[BUF_DEF_EL]);
		++d;
	}
}

fixcond_def_el(buf_number, db_buf, buf_size)
short	buf_number;
char	*db_buf[BUF_NUMBER];
int	buf_size[BUF_NUMBER];
{
	struct dbcond_def_el	*d;
	short	nitems;

	d = (struct dbcond_def_el *) db_buf[buf_number];
	nitems = buf_size[buf_number] / sizeof(struct dbcond_def_el);

	while (nitems--) {
		d->attribute.string = ADD_OFFSET(d->attribute.istring,
						db_buf[BUF_TEXT]);
		d->value.value = ADD_OFFSET(d->value.ivalue,
				(struct value *) db_buf[BUF_VALUE]);
		d->next_on_fail.def_el = ADD_OFFSET(d->next_on_fail.idef_el,
				(struct def_element *) db_buf[BUF_DEF_EL]);
		++d;
	}
}

fixvalue(buf_number, db_buf, buf_size)
short	buf_number;
char	*db_buf[BUF_NUMBER];
int	buf_size[BUF_NUMBER];
{
	struct dbvalue	*v;
	short	nitems;

	v = (struct dbvalue *) db_buf[buf_number];
	nitems = buf_size[buf_number] / sizeof(struct dbvalue);

	while (nitems--) {
		v->value.string = ADD_OFFSET(v->value.istring,
							db_buf[BUF_TEXT]);
		v->next.value = ADD_OFFSET(v->next.ivalue,
				(struct value *) db_buf[BUF_VALUE]);
		++v;
	}
}

fixmacros(macrofile, database_path)
char	*macrofile, *database_path;
{
	FILE	*macros;
	int	c;

	macros = fopendb(macrofile, database_path, "r", EXIT_ON_FAIL);
	while ((c = mygetc(macros)) != EOF)
		textputc(c, MONK_DB);
	fclosencheck(macros);
}
