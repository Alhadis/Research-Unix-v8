#ifndef PADS_H
#define PADS_H
typedef unsigned short Attrib;		/* should be private */
>pri
#define CARTE 0x80
#define NUMERIC 1
#undef major
#undef minor
>
#define PRINTF_ARGS char *fmt ...
#define FMT(i) ( ((int*)&fmt)[i] )
#define PRINTF_COPY\
	fmt?fmt:"0",FMT(1),FMT(2),FMT(3),FMT(4),FMT(5),FMT(6),FMT(7),FMT(8)
#ifndef TERM

#ifdef TRACE
#define OK(x)		if( !ok() ) abort();
#define VOK		if( !ok() ) abort();
#define IF_LIVE(x)	{ if( x )   abort(); } if(0)
typedef int  (*PFI)();
PFI trace_fcn(char*,int), trace_ptr;
#define trace  (!(trace_ptr=trace_fcn(__SRC__,__LINE__))) ? 0 : (*trace_ptr)
#else
#define OK(x)		{ if( !ok() ) return x; }
#define VOK		{ if( !ok() ) return ; 	}
#define IF_LIVE(x)	  if( x )
#define trace  if( 0 )
#endif

class Index {
>pub
	char	u_filler[2];
>
public:
>pri
unsigned char	major;
unsigned char	minor;
		Index(void)		{ major = minor = 0;		}
	 	Index(int i)		{ major = i>>8; minor = i&0xFF; }
		Index(int j, int n )	{ major = j;	minor = n;	}
	short	sht(void);		/* don't inline sht() - sht() etc */
>
	int	null(void);
};
extern Index ZIndex;
>pri
class Carte {
public:
	unsigned char	size;		/* host.size != term.size */
	unsigned char	attrib;
	struct Index	bin[1];
			Carte(void) {};
};
>
#else
typedef unsigned short Index;
#define MJR(i)	((i)>>8)
#define MNR(i)	((i)&0xFF)
typedef struct Carte Carte;
typedef enum Protocol Protocol;
struct Carte {
	unsigned char	size;		/* host.size != term.size */
	unsigned char	attrib;
	Index		bin[1];
};
#endif

#define	SELECTLINE	((Attrib)0x0001)
#define SORTED		((Attrib)0x0002)
#define ACCEPT_KBD	((Attrib)0x0004)
#define FOLD		((Attrib)0x0008)
#define TRUNCATE	((Attrib)0x0010)
#define CYCLE		((Attrib)0x0020)
#define USERCLOSE	((Attrib)0x0040)
#define DONT_CUT	((Attrib)0x0080)
#define FLUSHLINE	((Attrib)0x0100)	/* should not be required */

#ifndef TERM
char *sf(char* ...);
typedef void (*Action) (...);
char *PadsInit(char* = 0);
void Pick(Action,long);
long UniqueKey(void);
Index NumericCarte(short,short);

class Univ {
	int	state;
public:
		Univ(void);
	int	isenabled(void);
	int	isdead(void);
	void	kill(void);
	void	enable(void);
	void	disable(void);
virtual	int	disc(void);
virtual	void	kbd(char*);
virtual	void	help(void);
virtual	void	numeric(long);
virtual	void	userclose(void);
virtual	void	cycle(void);
virtual	void	linereq(long,Attrib=0);
virtual	int	accept(Action);
};

class Pad {
>pub
	char	u_filler[20];
>pri
	Univ	*_object;
	char	*_name;
	char	*_banner;
	Attrib	_attributes;
	void	termop(enum Protocol);
	long	errorkey;
	void	nameorbanner(enum Protocol, char* ...);
>
public:
	int	ok(void);
		Pad(Univ *);
		~Pad(void);
	void	alarm(void);
	void	banner(char * ... );
	void	clear(void);
	void	cycle(void);
	void	dump(void);
	void	error(char* ...);
	void	insert(class Line&);
	void	insert(long, Attrib, Univ*, Index, char* ... );
	void	insert(long, Attrib, Univ*, class Menu&, char* ... );
	void	insert(long, Attrib, char* ... );
	void	insert(long, char* ... );
	void	lines(long);
	void	makecurrent(void);
	void	makegap(long,long);
	void	menu(Index);
	void	menu(class Menu&);
	void	name(char * ... );
	void	options(Attrib, Attrib=0);
};

class Line {
public:
	int	ok(void);
		Line(void);
	Univ	*object;
	char	*text;
	long	key;
	Attrib	attributes;
	Index	carte;
};
>pri
class IList {
	friend	Menu;
	Index	index;
	IList	*next;
public:
		IList(Index i, IList *n) { index = i; next = n; }
};

class Item {
public:
	char	*text;
	Action	action;
	long	opand;
		Item(char* t,Action a,long o);
		Item(void);			/* ever used ? */
};
>
class Menu {
>pub
	char	u_filler[8];
>pri
	IList	*list;
	int	size;
	void	dump(void);
>
public:
		Menu(void);
		~Menu(void);
		Menu( char*, Action, long=0 );
	Index	carte(void);
	void	first( char*, Action, long=0 );	
	void	first( Index );
	void	last( char*, Action, long=0 );	
	void	last( Index );
	void	sort( char*, Action, long=0 );
};
>pri
class Binary {
public:
	Binary	*left;
	Binary	*right;
	Index	index;
		Binary(void)	{}
};

class Cache {
	friend	ItemCache;
	friend	CarteCache;
	Binary	*root;
	Index	current;
	Index	SIZE;
public:
		Cache(unsigned char,unsigned char);
	int	ok();
};

class ItemCache : public Cache {
	Item	***cache;
public:
		ItemCache(void);
	Index	place(Item);
	Item	*take(Index);
};

class CarteCache : public Cache {
	Carte	***cache;
public:
		CarteCache(void);
	Index	place(Carte*);
	Carte	*take(Index);
	Index	numeric(short,short);
	int	ItemCount(Carte*);
};

ItemCache  *ICache;
CarteCache *CCache;
>
#endif

>pri
#define CARTESIZE(s) (sizeof(Carte) + (s-1)*sizeof(Index))

enum Protocol {
	P_UCHAR		= 1,
	P_SHORT		= 2,
	P_LONG		= 4,

	P_CACHEOP	= 0x10,
	P_I_DEFINE	= 0x11,
	P_I_CACHE	= 0x12,
	P_C_DEFINE	= 0x13,
	P_C_CACHE	= 0x14,

	P_STRING	= 0x20,
	P_INDEX		= 0x21,

	P_PADDEF	= 0x30,
	P_ATTRIBUTE	= 0x31,
	P_BANNER	= 0x32,
	P_CARTE		= 0x33,
	P_LINES		= 0x34,
	P_NAME		= 0x35,

	P_PADOP		= 0x40,
	P_ACTION	= 0x41,
	P_ALARM		= 0x42,
	P_CLEAR		= 0x43,
	P_CYCLE		= 0x44,
	P_DELETE	= 0x45,
	P_KBDSTR	= 0x46,
	P_LINE		= 0x47,
	P_LINEREQ	= 0x48,
	P_MAKECURRENT	= 0x49,
	P_MAKEGAP	= 0x4A,
	P_NEXTLINE	= 0x4B,
	P_NUMERIC	= 0x4C,
	P_USERCLOSE	= 0x4D,

	P_HOSTSTATE	= 0x50,
	P_BUSY		= 0x51,
	P_IDLE		= 0x52,

	P_PICK		= 0x61,
};

long		RcvLong();
short		RcvShort();
unsigned char	RcvUChar();
char		*RcvString();

void		SendLong();
void		SendShort();
void		SendUChar();
void		SendString();

#endif
>
