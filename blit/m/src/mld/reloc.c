#include "stdio.h"
#include "a.out.h"
#include "mld.h"
extern struct nlist *outstab;
extern char flg[];

reloc()
{	int i;
	file *f;
	for(i = 0; i < mustuse->lnext; i++) {
		f = (file *)(mustuse->ldata[i]);
		doreloc(f, f->ftext, htextlen(f), f->freloc);
	}
	for(i = 0; i < mustuse->lnext; i++) {
		f = (file *)(mustuse->ldata[i]);
		doreloc(f, f->fdata, hdatlen(f), f->freloc + htextlen(f));
	}
}

doreloc(f, a, len, p)
file *f;
struct reloc *p;
short *a;
{	int i, n;
	struct reloc *svp = p;
	stab *s;
	len /= sizeof(struct reloc);	/* better be sizeof(short) */
	for(i = 0; i < len; p++, i++) {
		if(p->rpc) {
			register t = p->rtype & N_TYPE;
			if(t != N_TEXT && t != N_DATA && t != N_BSS) {
				fprintf(stderr, "doreloc(%s,i=%d) rpc 0x%x rtype 0x%x t 0x%x\n",
					f->name, i, p->rpc, p->rtype, t);
				fprintf(stderr, "a 0x%x ftext 0x%x fdata 0x%x\n",
					a, f->ftext, f->fdata);
				fatal("pjw misunderstood\n");
			}
			continue;
		}
		switch(p->rtype & N_TYPE) {
		default:
			fatal("unexpected reloc type\n");
			break;
		case N_ABS:
			continue;
		case N_TEXT:
			if(p->r2wds)
				lincr(a+i, f->textorg);
			else
				a[i] += f->textorg;
			continue;	/* next will be N_ABS */
		case N_DATA:
			if(p->r2wds)
				lincr(a+i, f->dataorg - htextlen(f));
			else
				a[i] += f->dataorg - htextlen(f);
			continue;
		case N_BSS:
			if(p->r2wds)
				lincr(a+i, f->bssorg-htextlen(f)-hdatlen(f));
			else
				a[i] += f->bssorg - htextlen(f) - hdatlen(f);
			continue;
		case N_UNDF:
			if(!(p->rtype & N_EXT))
				fatal("in reloc, undef & !global\n");
			s = lookup(sym(f->fsym, p->rnum));
			if(s == 0)
				fatal("late unknown sym\n");
			p->rnum = s->ref - outstab;
			if(flg['r'] && type(s->ref) == N_UNDF) {
				if(s->ref - outstab > 512)
					error("symbol table bigger than 512\n");
				continue;
			}
			if(p->r2wds)
				lincr(a+i, s->ref->n_value);
			else
				a[i] += s->ref->n_value;
			if(flg['r']) {
				p->rnum = 0;
				p->rtype |= type(s->ref);
			}
			continue;
		}
	}
}

lincr(p, n)
short *p;
{	long x;
	x = *(p+1) & 0xffff;
	x |= (*p << 16);
	x += n;
	*(p+1) = x & 0xffff;
	*p = (x >> 16) & 0xffff;
}
