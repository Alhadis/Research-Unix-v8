#include "/usr/blit/src/joff/host/a.out.h"
#include "stdio.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "/usr/blit/src/joff/host/sdb.h"
#define MAGIC_REL 0406
#define MAGIC_ABS 0407

extern char *malloc(), *realloc();

char *buf;
int buflen;
struct exec *u;
struct stat stb;

int gflag, vflag;

#define MAX 256
char structids[MAX][8];
limit = 0;

lookup( id )
char *id;
{
	int i;

	for( i = 0; i < limit; ++i )
		if( !strncmp( structids[i], id, 8 ) )
			return 1;
	if( limit < MAX ) strncpy( structids[limit++], id, 8 );
	return 0;
}
	
main(argc, argv)
char **argv;
{	int i, fd, in, insyms, outsyms, skip;
	struct nlist *sin, *sout;

	buf = malloc(buflen = 4096);
	for(i = 1; i < argc; i++) {
		if( argv[i][0] ==  '-' ){
			if( index( argv[i], 'v' ) ) ++vflag;
			if( index( argv[i], 'g' ) ) ++gflag;
			continue;
		}
		if((fd = open(argv[i], 2)) == -1) {
			perror(argv[i]);
			continue;
		}
		if(fstat(fd, &stb) == -1) {
			perror(argv[i]);
			close(fd);
			continue;
		}
		if(stb.st_size > buflen)
			buf = realloc(buf, buflen = stb.st_size);
		if(read(fd, buf, stb.st_size) != stb.st_size) {
			perror(argv[i]);
			close(fd);
			continue;
		}
		close(fd);
		u = (struct exec *) buf;
		if(u->a_magic != MAGIC_ABS && u->a_magic != MAGIC_REL) {
			fprintf(stderr, "%s: magic number 0%o illegal\n",
				argv[i], u->a_magic);
			continue;
		}
		if(u->a_syms == 0) {
			fprintf(stderr, "%s already stripped\n", argv[i]);
			continue;
		}
		sout = sin = (struct nlist *)( &buf[buflen] - u->a_syms );
		if( !gflag ){
			stb.st_size -= u->a_syms;
			u->a_syms = 0;
		}
		insyms = in = u->a_syms/sizeof (struct nlist);
		outsyms = 0;
		skip = 0;
		for( ; in-- > 0; ++sin ){
			if( sin->n_other == S_BSTR && lookup( sin->n_name ) )
				skip = 0xFFFFFF;
			switch( sin->n_other+skip ){
			case 0:
			case S_GSYM:
			case S_STSYM:
			case S_RSYM:
			case S_LSYM:
			case S_PSYM:
			case S_BFUN:
			case S_EFUN:
			case S_SO:
			case S_TYID:
			case S_DIM:
			case S_BSTR:
			case S_SSYM:
			case S_ESTR:
			++outsyms;
				*sout++ = *sin;
				break;
			default:
				u->a_syms -= sizeof (struct nlist);
				stb.st_size -= sizeof (struct nlist);
			}
			if( sin->n_other == S_ESTR ) skip = 0;
		}
		if( gflag && insyms == outsyms ){
			fprintf(stderr, "no symbols removed from %s\n", argv[i]);
			continue;
		}
		if(unlink(argv[i]) == -1) {
			perror(argv[i]);
			continue;
		}
		fd = creat(argv[i], stb.st_mode);
		write(fd, buf, stb.st_size);
		close(fd);
		if( vflag ){
			printf(
				"%s: %d%% of symbols removed\n",
					argv[i], (insyms-outsyms)*100/insyms );
			continue;
		}
	}
	exit(0);
}
