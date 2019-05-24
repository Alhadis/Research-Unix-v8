#include "a.out.h"
#include "mld.h"
#include "ar.h"
extern char *malloc(), *ncat(), flg[];
file *
tofile(nm, s)	/* s points to beginning of .o */
char *s, *nm;
{	file *f;
	f = (file *) malloc(sizeof(file));
	f->name = nm;
	f->hdr = *(struct exec *) s;
	f->textorg = f->dataorg = f->bssorg = -1;
	f->ftext = s + sizeof(struct exec);
	f->fdata = f->ftext + f->hdr.a_text;
	if(f->hdr.a_flag)	/* reloc info stripped */
		f->freloc = 0;
	else
		f->freloc = f->fdata + f->hdr.a_data;
	if(f->hdr.a_flag)
		f->fsym = f->fdata + f->hdr.a_data;
	else
		f->fsym = f->freloc + f->hdr.a_text + f->hdr.a_data;
	return(f);
}

doto(nm, s)
char *nm, *s;
{	file *f;
	int i, n;
	struct nlist *p;

	f = tofile(nm, s);
	append(mustuse, (char *)f);
	n = symcnt(f);
	verbose("%s:\n", nm);
	for(i = 0; i < n; i++) {
		p = sym(f->fsym, i);
		if(global(p)) {
			verbose("%8.8s\n", p->n_name);
			if(flg['s'] && (p->n_desc || p->n_other))
				continue;
			merge(p);
		}
	}
}

doarch(nm, p, n)
char *nm, *p;
{	char *x = p + n, *s;
	file *f;
	int i;
	struct ar_hdr *a;
	struct exec *u;
	a = (struct ar_hdr *)(p + SARMAG);
	verbose("archive %s\n", nm);
	while((char *)a < x) {
		u = (struct exec *)((char *)a + sizeof(struct ar_hdr));
		if(u->a_magic != 0407 && u->a_magic != 0406) {
incr:
			i = (int)((char *)a + sizeof(struct ar_hdr));
			i += atoi(a->ar_size);
			if(i%2)
				i++;
			a = (struct ar_hdr *)i;
			continue;
		}
		s = ncat(nm, a->ar_name, NAMESZ);
		f = tofile(s, (char *)u);
		need(f);
		goto incr;
	}
}

need(f)
file *f;
{	int i, n;
	struct nlist *p;
	n = symcnt(f);
	for(i = 0; i < n; i++) {
		p = sym(f->fsym, i);
		if(global(p) && resolves(p))
			goto ok;
	}
	verbose("%s not used\n", f->name);
	return;
ok:
	append(mustuse, (char *)f);
	verbose("%s used because of %8.8s\n", f->name, p->n_name);
	for(i = 0; i < n; i++) {
		p = sym(f->fsym, i);
		if(global(p)) {
			verbose("%8.8s\n", p->n_name);
			merge(p);
		}
	}
}

toundef(s)	/* fake up a file with only one undef in it*/
char *s;
{	struct exec *u;
	struct nlist *n;
	u = (struct exec *) malloc(sizeof(struct exec) + sizeof(struct nlist));
	u->a_magic = 0407;
	u->a_text = u->a_data = u->a_bss = u->a_entry = u->a_unused = 0;
	u->a_syms = sizeof(struct nlist);
	n = (struct nlist *) (u + 1);
	strncpy(n->n_name, s, NAMESZ);
	n->n_type = N_UNDF | N_EXT;
	n->n_other = n->n_desc = n->n_value = 0;
	doto("(fake)", (char *)u);
}
