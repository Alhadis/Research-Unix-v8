#include "a.out.h"
#include "mld.h"

struct nlist *outstab;
int outcnt;
extern char *outfile, flg[];
struct exec outhdr;
extern int textorg;

newstab()
{	int i, n, j, k;
	int cnt;
	stab *s;
	file *f;
	struct nlist *p;

	for(cnt = i = 0; i < mustuse->lnext; i++) {
		f = (file *)(mustuse->ldata[i]);
		n = symcnt(f);
		for(j = 0; j < n; j++) {
			p = sym(f->fsym, j);
			if(willout(p)) {
				cnt++;
				if((p->n_other || p->n_desc) && global(p)) {
					s = lookup(p->n_name);
					if(s == 0)
						continue;
					p->n_type = s->ref->n_type;
				}
			}
		}
	}
	outcnt = cnt;
	outstab = (struct nlist *) malloc(cnt * sizeof(struct nlist));
	k = 0;
	for(i = 0; i < mustuse->lnext; i++) {
		f = (file *)(mustuse->ldata[i]);
		n = symcnt(f);
		for(j = 0; j < n; j++) {
			p = sym(f->fsym, j);
			if(willout(p))
				outstab[k++] = *p;
		}
	}
	if(k != outcnt)
		fatal("inconsistent newstab %d expected %d\n", k, cnt);
	for(i = 0; i < outcnt; i++) {
		s = lookup(outstab[i].n_name);
		if(s && (outstab[i].n_type & N_EXT))
			s->ref = outstab + i;
	}
}

willout(p)	/* which symbols go in the output */
struct nlist *p;
{	stab *s;
	int i;
	if(p->n_other || p->n_desc)	/* sdb */
		return(!flg['s']);
	if(global(p)) {
		s = lookup(p->n_name);
		if(s == 0)
			return(0);
		if(s->ref != p)
			return(0);
		return(1);
	}
	for(i = 0; i < 8; i++)	/* sizeof(n_name) */
		if(p->n_name[i] == '%')
			return(0);
	return(1);
}

output()
{	int i, n, fd;
	file *f;
	if(strcmp("-", outfile) == 0)
		fd = 1;
	else if((fd = creat(outfile, 0666)) < 0) {
		perror(outfile);
		exit(1);
	}
	outhdr.a_unused = textorg;
	if(flg['r'] || flg['R'])
		outhdr.a_flag = 0;
	else
		outhdr.a_flag = 1;
	if(!flg['s'])
		outhdr.a_syms = outcnt * sizeof(struct nlist);
	write(fd, (char *)&outhdr, sizeof(struct exec));
	for(i = 0; i < mustuse->lnext; i++) {
		f = (file *)(mustuse->ldata[i]);
		write(fd, f->ftext, htextlen(f));	/* alignment? */
	}
	for(i = 0; i < mustuse->lnext; i++) {
		f = (file *)(mustuse->ldata[i]);
		write(fd, f->fdata, hdatlen(f));
	}
	if(flg['r']) {
		for(i = 0; i < mustuse->lnext; i++) {
			f = (file *)(mustuse->ldata[i]);
			write(fd, f->freloc, htextlen(f));
		}
		for(i = 0; i < mustuse->lnext; i++) {
			f = (file *)(mustuse->ldata[i]);
			write(fd, f->freloc + htextlen(f), hdatlen(f));
		}
	}
	if(!flg['s'])
		write(fd, (char *)outstab, outcnt * sizeof(struct nlist));
}

prundef()
{	int i, n, j;
	file *f;
	stab *s;
	struct nlist *p;
	for(i = 0; i < mustuse->lnext; i++) {
		f = (file *)(mustuse->ldata[i]);
		n = symcnt(f);
		for(j = 0; j < n; j++) {
			p = sym(f->fsym, j);
			if(type(p) == N_UNDF) {
				s = lookup(p->n_name);
				if(s == 0 || type(s->ref) == N_UNDF)
					prsym(p);
			}
		}
	}
}

prsym(p)
struct nlist *p;
{
	printf("%8.8s undefined\n", p->n_name);
}
