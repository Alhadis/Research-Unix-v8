#include <pads.pub>

class Creator : public PadRcv {
	void	linereq(long,Attrib=0);
public:
	Pad	*pad;
		Creator();
	char	*kbd(char*);
};

class Test : public PadRcv {
	void	linereq(long,Attrib=0);
public:
	Pad	*pad;
		Test();
	void	err(long);
	void	lines(long);
	void	exit();
	void	cycle();
	void	tabs(long t)	{ pad->tabs(t); }
	void	remove(long k)  { pad->removeline(k); }
	char	*kbd(char*);
	char	*help();
	void	usercut();
};

void Test.usercut() {}

int sscanf(char*,char* ...);

char *Test.kbd(char *s)
{
	int i;
	if( !sscanf(s,"%d",&i) || i<=0 || i>=1000 )
		return "out of range";
	linereq(i,SELECTLINE);
	return 0;
}

char *Creator.kbd(char *s)
{
	int lo, hi;
	if( 2 == sscanf(s, "%d %d", &lo, &hi) )
		pad->createline(lo, hi );
	return 0;
}

char *Test.help()
{
	static int i;
	switch( i++%3 ){
		case 0: return 0;
		case 1: return "";
		case 2: return "Test.help";
	}
}

void Test.exit() { ::exit(); }

void Test.cycle()
{
	static i;
	char *ctime(long*);
	long time(long*), t;

	pad->alarm(i++/10);
	time(&t);
	pad->insert( 1, "%s", ctime(&t) );
}

Test.Test()
{
	Menu m( "first 1", (Action) &linereq, 1 );
	Menu sub;
	Menu subsub;
	pad = new Pad( (PadRcv*) this );
	pad->lines(1000);
	pad->insert(1001, "one thouseand and one");
	pad->banner( "%s=%s", "Banner", "Test" );
	pad->name( "%s=%s", "name", "test" );
	pad->options(DONT_CLOSE|NO_TILDE);
	pad->makecurrent();
	subsub.sort( "B", (Action) &linereq, 101 );
	subsub.sort( "A", (Action) &linereq, 102 );
	subsub.sort( "D", (Action) &linereq, 103 );
	subsub.sort( "C", (Action) &linereq, 104 );
	sub.first( subsub.index("subsub") );
	sub.last( "b", (Action) &linereq, 110 );
	sub.last( "a", (Action) &linereq, 120 );
	sub.last( "d", (Action) &linereq, 130 );
	sub.last( "c", (Action) &linereq, 140 );
	m.first( sub.index("sub") );
	m.first( "cycle", (Action) &cycle );
	m.first( "tabs=1", (Action) &tabs, 1 );
	m.first( "tabs=3", (Action) &tabs, 3 );
	m.last( "tabs=5", (Action) &tabs, 5 );
	m.last( "tabs=7", (Action) &tabs, 7 );
	m.last( "tabs=0", (Action) &tabs, 0 );
	m.last( "tabs=128", (Action) &tabs, 128 );
	m.last( "remove 100", (Action) &remove, 100 );
	m.last( NumericRange(1,10) );
	pad->menu(m.index());
	pad->alarm();
}

Creator.Creator()
{
	pad = new Pad( (PadRcv*) this );
	pad->banner( "%s=%s", "Banner", "Creator" );
	pad->name( "%s=%s", "name", "creator" );
	pad->makecurrent();
	for( long i = 1; i <= 100; i += 10 ){
		pad->createline(i);
		pad->createline(i+3, i+7);
	}
}

void Creator.linereq(long i, Attrib a)
{
	pad->insert(i, a, "line\t%d\t[]", i);
}

void Test.linereq(long i, Attrib a)
{
	Menu m;

	switch( i%5 ){
	case 0:
		pad->insert( i, a, "line\t%d\t[]", i  );
		pad->banner( "banner=%d", i );
		break;
	case 1:
		m.last( "150", (Action)&linereq, 150 );
		pad->insert( i, a|USERCUT, (PadRcv*) this, m, "line\t%d", i );
		pad->name( "name=%d", i );
		break;
	case 2:
		m.last("250", (Action)&linereq, 250 );
		pad->insert( i, a|USERCUT, (PadRcv*) this, m, "line\t%d", i );
		break;
	case 3:
		m.last("exit?", (Action)&exit, 0 );
		pad->insert( i, a|USERCUT, (PadRcv*) this, m, "line\t%d", i );
		break;
	case 4:
		pad->insert( i, a|USERCUT, "" );
		break;
	}
	
}

void main(int, char **argv)
{
	if( argv[1] )
		PadsInit( argv[1] );
	else
		PadsInit();
	TapTo = ".tapto";
	new Test;
	new Creator;
	NewPadStats();
	PadsServe();
}
