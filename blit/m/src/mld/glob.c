#include "a.out.h"
#include "mld.h"
extern char flg[];
extern list *mustuse;
int cntundef;
extern int textorg, bssorg;
extern char textset, dataset, bssset;
int dataorg;
extern struct exec outhdr;

globaddr()
{	int i, n, j, k;
	file *f;
	stab *s;
	struct nlist *p;
	n = textorg;
	/* where the text starts */
	for(i = 0; i < mustuse->lnext; i++) {
		f = (file *)(mustuse->ldata[i]);
		f->textorg = n;
		verbose("text for %s starts at %d\n", f->name, f->textorg);
		n += ALIGN(htextlen(f));
	}
	if(dataset == 0)
		dataorg = n;
	if(!dataset && !textset && n > dataorg)
		warn("text overlaps data\n");
	outhdr.a_text = n - textorg;
	n = dataorg;
	/* where the data starts */
	for(i = 0; i < mustuse->lnext; i++) {
		f = (file *)(mustuse->ldata[i]);
		f->dataorg = n;
		verbose("data for %s starts at %d\n", f->name, f->dataorg);
		n += ALIGN(hdatlen(f));
		outhdr.a_data += ALIGN(hdatlen(f));
	}
	if(bssset == 0)
		bssorg = n;
	if(!bssset && bssorg < n && bssorg >= textorg)
		warn("bss overlaps\n");
	/* convert common to bss, perhaps */
	if(!flg['r'] || flg['d'])
		for(i = 0; i < mustuse->lnext; i++) {
			f = (file *)(mustuse->ldata[i]);
			n = symcnt(f);
			for(j = 0; j < n; j++) {
				p = sym(f->fsym, j);
				if(type(p) != N_UNDF)
					continue;
				if(p->n_value == 0)	/* undefined */
					continue;
				if(!global(p)) {	/* local common */
					p->n_type = N_BSS;
					k = hbsslen(f);
					hbsslen(f) += p->n_value;
					hbsslen(f) = ALIGN(hbsslen(f));
					p->n_value = htextlen(f) + hdatlen(f)
						+ k;
					continue;
				}
				if((s = lookup(p->n_name)) == 0)
					fatal("inconsistent sym tab\n");
				if(s->ref != p)
					continue;
				/* this is the ruling use */
				p->n_type = N_BSS | N_EXT;
				k = hbsslen(f);
				hbsslen(f) += p->n_value;
				hbsslen(f) = ALIGN(hbsslen(f));
				p->n_value = htextlen(f) + hdatlen(f) + k;
			}
		}
	/* figure out the bss starts */
	n = bssorg;
	for(i = 0; i < mustuse->lnext; i++) {
		f = (file *)(mustuse->ldata[i]);
		f->bssorg = n;
		verbose("bss of %s starts at %d\n", f->name, f->bssorg);
		n += ALIGN(hbsslen(f));
		outhdr.a_bss += ALIGN(hbsslen(f));
	}
	if(!flg['r'] || flg['d'])
		ldrsyms();	/* text, data, end, etc */
	/* assign addresses to everything */
	for(i = 0; i < mustuse->lnext; i++) {
		f = (file *)(mustuse->ldata[i]);
		verbose("addresses for %s:\n", f->name);
		n = symcnt(f);
		for(j = 0; j < n; j++) {
			p = sym(f->fsym, j);
			switch(type(p)) {
			case N_TEXT:
				p->n_value += f->textorg;
				break;
			case N_DATA:
				p->n_value += f->dataorg - htextlen(f);
				break;
			case N_BSS:
				p->n_value += f->bssorg - htextlen(f) - hdatlen(f);
				break;
			case N_UNDF:
				s = lookup(p->n_name);
				if(s == 0 || type(s->ref) == N_UNDF)
					cntundef++;
				else
					p->n_value = s->ref->n_value;
				break;
			}
			verbose("%s at %d\n", p->n_name, p->n_value);
		}
	}
	if((flg['R'] || !flg['r']) && cntundef > 0) {
		prundef();
		exit(1);
	}
}

ldrx(nm, ty, val)
char *nm;
{	stab *s;
	struct nlist *p;
	file *f;
	int n;
	f = (file *)mustuse->ldata[mustuse->lnext - 1];
	s = lookup(nm);
	if(s == 0)
		return;
	p = s->ref;
	if(type(p) != N_UNDF)
		error("%s (ldr builtin) redefined\n", nm);
	n = symcnt(f);
	s->ref = p = sym(f->fsym, n);
	p->n_type = ty;
	p->n_value = val;
	strcpy(p->n_name, nm);
	f->hdr.a_syms += sizeof(struct nlist);
}

ldrsyms()
{
	ldrx("etext", N_TEXT|N_EXT, 0);
	ldrx("end", N_BSS|N_EXT, 0);
	ldrx("edata", N_DATA|N_EXT, 0);
}
