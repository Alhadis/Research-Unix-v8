#include <jerq.h>
#include <font.h>

jstrwidth(s)
	char *s;
{
	return(strwidth(&defont,s));
}

strwidth(f,s)
	Font *f;
	register char *s;
{
	register wid=0;
	register Fontchar *info;
	info = f->info;
	for(; *s; s++)
		wid+=info[*s].width;
	return(wid);
}
