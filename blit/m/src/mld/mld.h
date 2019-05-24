#ifndef vax
this file for vax to m68000 cross loading
#endif
typedef struct {
	char *name;	/* full path name */
	struct exec hdr;
	int textorg, dataorg, bssorg;	/* in the a.out */
	char *ftext;
	char *fdata;
	char *freloc;
	char *fsym;
} file;

#define magic(f)	(f->hdr.a_magic)
#define hdatlen(f)	(f->hdr.a_data)
#define htextlen(f)	(f->hdr.a_text)
#define hbsslen(f)	(f->hdr.a_bss)
#define relloccnt(f)	(f->hdr.a_data + f->hdr.a_text)/sizeof(struct reloc)
#define symcnt(f)	(f->hdr.a_syms / sizeof(struct nlist))

typedef struct {
	char **ldata;	/* every pointer can be coerced to (char *) */
	short lnext, lcnt, lincr;
} list;
extern list *mustuse, initlist;

typedef struct x {
	struct x *next;
	struct nlist *ref;
} stab;

#define HLEN	387
extern stab *htab[HLEN];
extern stab *lookup();

#define sym(p, n)	((struct nlist *)(p) + (n))
#define global(p)	(p->n_type & N_EXT)
#define type(p)		((p)->n_type & N_TYPE)

#define ALIGN(x)	(((x) + 1) & ~1)
#define verbose		if(flg['v']) printf
