#include "a.out.h"
#include "sys/types.h"
#include "sys/stat.h"
extern char *malloc();
char *buf;
struct stat stb;

main()
{	struct exec *u;
	struct nlist *p;
	int i, j, n;
	fstat(0, &stb);
	buf = malloc(stb.st_size);
	read(0, buf, stb.st_size);
	u = (struct exec *) buf;
	i = u->a_text + u->a_data;
	if(!u->a_flag)
		i += i;
	p = (struct nlist *) (buf + sizeof(struct exec) + i);
	n = u->a_syms / sizeof(struct nlist);
	for(i = 0; i < n; i++, p++)
		printf("%8.8s\t%x\t%x\t%x\t%x\n", p->n_name,
			p->n_type, p->n_other, p->n_desc, p->n_value);
}
