#include	<stdio.h>
#include	<sys/types.h>
#include	<time.h>
#include	"../comm.h"
#include	"host.h"

static char name[11][32];
int curfont;
int blitfont = 0;
extern char *strcpy();

tfont(name, size, c)
	char *name;
{
	char rname[128], file[128], fbuf[4096];
	register fd, n, spec;
	int osize = size;

	if(debug)fprintf(debug, "tfont(%s,%d,%d) ", name, size, c);
	if(spec = strcmp(name, "S") == 0)
		sprintf(rname, "%s.%d/ALL", name, size);
	else
		sprintf(rname, "%s.%d", name, size);
	send(TFONT);
	sends(rname);
	fd = 0;
	while(size >= 2)
	{
		if(spec)
		{
			sprintf(file, "%s.%d/ALL", name, size);
			if(fd = openf(file)) break;
			sprintf(file, "%s.%d/%d", name, size, c);
			if(fd = openf(file)) break;
		}
		else
		{
			sprintf(file, "%s.%d", name, size);
			if(fd = openf(file)) break;
		}
		size--;
	}
	if(fd)
	{
		if(debug)fprintf(debug, "succeeds with %s ", file);
		if(size != osize) missing(rname);
		sends(file);
		while((n = read(fd, fbuf, sizeof fbuf)) > 0)
			write(jerq, fbuf, n);
		(void)close(fd);
		if(debug)fprintf(debug, "load done\n");
	}
	else
	{
		if(debug)fprintf(debug, "no match\n");
		missing(rname);
		send(0);
	}
}

openf(s)
	char *s;
{
	register n;
	char buf[128];

	sprintf(buf, "%s/%s", JERQFONT, s);
	n = open(buf, 0);
	return(n == -1? 0:n);
}

loadfont(n, s)
	char *s;
{
	(void)strcpy(name[n], s);
}

static
special()
{
	register i;
	static last = 0;

	if(strcmp(name[last], "S") == 0)
		return(last);
	for(i = 0; i < 11; i++)
		if(strcmp(name[i], "S") == 0)
			return(last = i);
	return(0);
}

static
put(x, font)
{

	xygoto();
	if(font != blitfont)
	{
		blitfont = font;
		out(C_FONT + blitfont);
	}
	out(x);
}

dochar(c)
	char *c;
{
	register x;

	if(c[1] == 0)
		put(c[0], curfont);
	else
	{
		if(x = imap(c))
			put(x, curfont);
		else if(x = smap(c))
			put(x, special());
	}
}

missing(name)
	char *name;
{
	static char miss[200][10];
	static next = 0;
	register s;
	register FILE *f;
	time_t clk;
	struct tm *tm;
	extern struct tm *localtime();

	if(debug)fprintf(debug, "missing(%s)\n", name);
	for(s = 0; s != next; s++)
		if(strcmp(miss[s], name) == 0) return;
	(void)strcpy(miss[next], name);
	if(next != 200)
		next++;
	if((f = fopen(JERQMISSING, "a")) != NULL)
	{
		(void)time(&clk);
		tm = localtime(&clk);
		fprintf(f, "%s	%d/%d/%d\n", name, tm->tm_year, tm->tm_mon, tm->tm_mday);
		(void)fclose(f);
	}
}
