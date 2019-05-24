#include <ctype.h>
#include <pads.pri>
#include <CC/sys/types.h>
#include <sys/stat.h>
SRCFILE("term.c")

void Pick( char *s, Action a, long o )
{
	Index ix;

	trace( "Pick(%d,%d)", a, o );
	ix = ICache->place(Item( s, a, o ));
	R->pktstart( P_PICK );
	R->sendshort( ix.sht() );
	R->pktend();
}

char *JerqTERM[] = {	"/usr/jerq/bin/32ld /usr/jerq/mbin/pads.m",
			"5620", "jerq", "JERQ", "Jerq", "DMD", "DMD5620",
			0 };

char *BlitTERM[] = {	"/usr/blit/bin/68ld /usr/blit/mbin/pads.m",
			"blit", "BLIT", "Blit",
			0 };

char **MapTERM[] = { JerqTERM, BlitTERM, 0 };

char *PadsInit(char *loadcmd)
{
	trace( "PadsInit(%s)", loadcmd );
	if( !loadcmd ){
		char *getenv(char*), *TERM = getenv("TERM"), ***map, **term;
		loadcmd = MapTERM[0][0];
		for( map = MapTERM; TERM && *map; ++map )
			for( term = *map, ++term; *term; ++term )
				if( !strcmp(TERM, *term) )
					loadcmd = **map;
	}
	if( system(loadcmd) ) return "terminal download failed";
	R = new Remote("/dev/tty");
	R->pktstart(P_VERSION); R->sendlong(PADS_VERSION); R->pktend();
	R->pktstart(P_BUSY); R->pktend();
	if( !(ICache = new ItemCache) || !(CCache = new CarteCache) )
		PadsError( "cache initialization error" );
	close(2);
	return 0;
}

void WireTap(PRINTF_ARGS)
{
	const LINES = 5, WIDTH = 50;
	static char *log;
	static l;
	stat s;
	char *ctime(long*);
	long t, time(long*);

	if( !TapTo ) return;
	t = time(0);
	if( ::stat("/usr/jerq/lib/.logpads", &s) || ctime(&t)[23]!='4' ){
		TapTo = 0;
		return;
	}
	if( !log ) log = new char[LINES*WIDTH];
	sprintf(log+l*WIDTH, PRINTF_COPY);
	if( ++l >= LINES ){
		l = 0;
		if( ::stat(TapTo, &s) )
			creat(TapTo, 0777); 
		int fd = open(TapTo, 1);
		if( fd<0 || fstat(fd, &s) ){
			TapTo = 0;
			return;
		}
		chmod(TapTo, 0777|s.st_mode);
		if( s.st_size < 32000 ){
			lseek(fd, s.st_size, 0 );
			for( int i = 0; i < LINES; ++i )
				write(fd, log+i*WIDTH, strlen(log+i*WIDTH));
		} else
			TapTo = 0;
		close(fd);
	}
}

int BothValid(PadRcv *p, PadRcv *o)
{
	return p && o;
}


void TermAction(PadRcv *parent, PadRcv *obj, int pick)
{
	Item *item;
	Index ix(R->rcvshort());

	trace( "TermAction(%d,%d,%d)", parent, obj, pick );
	if( ix.null() ) return;
	item = ICache->take(ix);
	if( !BothValid(parent,obj)
	 || (pick && !obj->accept(item->action)) )
		return;
	WireTap("%x:%x->%x(%x)\n", time(0), obj, item->action, item->opand);
	if( item->action) (*item->action)(obj, item->opand);
}

char *DoKbd(PadRcv *obj, char *buf)
{
	WireTap("%x:%x->%x(%x)\n", time(0), obj, &obj->kbd, strlen(buf));
	char *e = obj->kbd(buf);
	if( e ) PadsWarn("%s", e);
	return e;
}

void ShKbd(PadRcv *obj, char *cmd)
{
	trace( "ShKbd(%d,%s)", obj, buf );
	FILE *fp = Popen(cmd, "r");
	if( !fp ){
		PadsWarn("cannot read from pipe");
		return;
	}
	char buf[256];
	while( fgets(buf, sizeof buf, fp) ){
		buf[strlen(buf)-1] = 0;
		if( DoKbd(obj, buf) ) break;
	}
	int x = Pclose(fp);
	if( x ) PadsWarn( "exit(%d): %s", x, cmd );
}

void Kbd(PadRcv *parent, PadRcv *obj)
{
	char buf[256];
	R->rcvstring(buf);
	trace( "Kbd %d %s", obj, buf );
	if( !BothValid(parent,obj) ) return;
	if( !strcmp( buf, "?" ) ){
		char *h = obj->help();
		PadsWarn( "%s", (h && *h) ? h : "error: null help string" );
	} else if( buf[0] == '<' ){
		ShKbd(obj, buf+1);
	} else
		DoKbd(obj, buf);
}

void TermServe()
{
	Protocol p;
	long n, to, pick = 0;

	R->writesize = 0;
	R->pktstart(P_IDLE); R->pktend();	/* flush */
	p = (Protocol) R->get();
	if( p == P_PICK ) {
		trace( "P_PICK" );
		pick = 1;
		p = (Protocol) R->get();
	}
	PadRcv *par = R->rcvobj();
	PadRcv *obj = R->rcvobj();
	if( p != P_CYCLE ) { R->writesize = 0; R->pktstart(P_BUSY); R->pktend(); }
	switch( (int) p ){
		case P_ACTION:
			TermAction(par, obj, pick);
			break;
		case P_KBDSTR:
			Kbd(par, obj);
			break;
		case P_NUMERIC:
		case P_CYCLE:
		case P_USERCLOSE:
		case P_USERCUT:
			n = R->rcvshort();
			trace( "P_%d %d", p, n );
			if( !BothValid(par,obj) ) return;
			switch( (int) p ){
			case P_NUMERIC:		obj->numeric(n);	break;
			case P_CYCLE:		obj->cycle();		break;
			case P_USERCLOSE:	obj->userclose();	break;
			case P_USERCUT	:	obj->usercut();		break;
			default: R->err();
			}
			break;
		case P_LINEREQ:
			n = R->rcvlong();
			to = R->rcvlong();
			trace( "P_LINEREQ %d %d %d", p, n, to );
			if( !BothValid(par,obj) ) return;
			while( n <= to )
				obj->linereq( (long) n++, 0 );
			break;
		default:
			R->err();
	}
}

void PadsServe(long n)
{
	if( n ){
		while( n-->0 ) TermServe();
	} else {
		for( ;; )  TermServe();
	}
}

void PadsWarn(PRINTF_ARGS)
{
	char t[256];
	sprintf( t, PRINTF_COPY );
	R->pktstart( P_HELPSTR );
	R->sendstring( t );
	R->pktend();
}

void PadsError(PRINTF_ARGS)
{
	char t[256];
	sprintf( t, PRINTF_COPY );
	R->sendstring(t);
	abort(1);
}
