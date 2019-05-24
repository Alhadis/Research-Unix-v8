#include "univ.h"
#include <CC/sys/types.h>
#include <sys/param.h>
#include <CC/sys/stat.h>
#include <sys/dir.h>
SRCFILE("wd.c")

class Wd : public PadRcv {
	long	key;
	Pad	*pad;
	char	*getwd;
	Index	carte();
	void	pwd(Attrib=0);
PUBLIC(Wd,U_WD)
		Wd();
	char	*kbd(char *);
	char	*help();
};

void NewWd() { new Wd; }

Wd.Wd()
{
	pad = new Pad((PadRcv*) this);
	pad->name("pwd/cd");
	pad->banner("Pwd:");
	pwd();
}

Index Wd.carte()
{
	struct direct dirbuf;
	struct stat stbuf;
	Menu m;

	trace( "%d.carte()", this );	OK(ZIndex);
	int  fd = ::open(".", 0);
	if( fd >= 0 ) {
		while( read(fd, (char*) &dirbuf, sizeof dirbuf) > 0 ){
			char dn[16];
			sprintf(dn, "%0.14s", dirbuf.d_name);
			int len = strlen(dn);
			if( len>=3 && dn[len-2]=='.' ) continue;
			if( dirbuf.d_ino == 0 
			 || ::stat(dn, &stbuf)== -1
			 || (stbuf.st_mode&S_IFMT)!=S_IFDIR )
				continue;
			long opand = (long) sf("%s/%s", getwd, dn);
			m.sort(sf("%s\240", dn), (Action)&kbd, opand);
		}
		::close(fd);
	}
	return m.index();
}

char *Getwd()
{
	char	*dot = ".", *dotdot = "..", *name;
	int	file, off = -1;
	struct	stat	d, dd;
	struct	direct	dir;
	int rdev, rino;

	name = "";
	::stat("/", &d);
	rdev = d.st_dev;
	rino = d.st_ino;
	for (;;) {
		::stat(dot, &d);
		if(d.st_ino==rino && d.st_dev==rdev) goto ChDir;
		if((file = open(dotdot,0)) < 0)
			return "can't get working directory";
		fstat(file, &dd);
		chdir(dotdot);
		if(d.st_dev == dd.st_dev) {
			if(d.st_ino == dd.st_ino) goto ChDir;
			do
				if (read(file,(char*)&dir,sizeof(dir))<sizeof(dir))
					return "can't get working directory";
			while (dir.d_ino != d.st_ino);
		}
		else do {
				if(read(file,(char*)&dir,sizeof(dir))<sizeof(dir))
					return "can't get working directory";
				::stat(dir.d_name, &dd);	
			} while(dd.st_ino != d.st_ino || dd.st_dev != d.st_dev);
		close(file);
		name = sf( name[0]?"%s/%s":"%s", dir.d_name, name );
	}
ChDir:
	name =  sf("/%s",name);
	if( chdir(name) == -1 ) return "chdir error";
	return name;
}

char *Wd.help()
{
	trace( "%d.help()", this );
	return "[cd] <path> {change working directory}";
}

char *Wd.kbd(char *s)
{
	trace( "%d.kbd(%s)", this, s );		OK("kbd");
	if( s[0]=='c' && s[1]=='d' && s[2]==' ' ) s += 3;
	while( *s == ' ' ) ++s;
	if( chdir(s) == -1 ) pad->insert(++key, "cannot cd %s", s);
	pwd(SELECTLINE);
	return 0;
}

void Wd.pwd(Attrib a)
{
	static Index zix;
	getwd = Getwd();
	Index ix = strncmp(getwd, "/n/", 3) ? carte() : zix;
	pad->menu(ix);
	pad->insert(++key, a, (PadRcv*)this, ix, "%s", getwd);
}
