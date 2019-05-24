typedef struct {
	char *name;	/* header name */
	int size;	/* size of name */
	char *line;	/* header line */
} header;

header hdrs[];		/* important headers */

/* some useful macros */
#define HEADER(s) { s, sizeof(s)-1, NULL }
#define STRCMP(s, p) strncmp((s), (p)->name, (p)->size)
