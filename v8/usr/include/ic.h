struct ICsubs{
	char	*ICsstring;	/* String of subscript */
	short	ICndig;		/*  0 or #of digits for <> subs. */
};
struct ICchip{
	struct ICchip	*ICnextp;	/* Next chip in list */
	struct ICpin	*ICpinp;	/* Linked list of pins */
	struct ICsubs	ICcsubs;
	char	*ICcname;		/* Name of chip (e.g. inverter) */
	char	*ICctype;		/* Type of chip (e.g. 74LS04) */
};
struct ICsignal{
	char	*ICsname;	/* Name of signal */
	struct ICsubs ICssubs;
};
struct ICpin{
	struct ICpin	*ICnextp;	/* Next pin on this chip */
	struct ICsignal	*ICpsig;	/* Signal on this pin */
	char	*ICpname;		/* Pin name */
	struct ICsubs ICpsubs;
	char	*ICpsstr;		/* Parent chip subscript string */
};

struct ICsignal *ICR();
struct ICpin *ICL();
struct ICchip *ICC();
char *ICS();
typedef struct ICchip *ICtchip;
typedef struct ICsignal *ICtsignal;
#ifdef	ICLIB
	/* Definitions needed for actual LIB code */
struct action{
	short	a_type;
	union{
		struct ICchip *uchip;
		struct ICsignal *usig;
		struct ICpin *upin;
	}a_u;
};
#define	a_chip		a_u.uchip
#define	a_sig	a_u.usig
#define	a_pin		a_u.upin

/* The tree nodes */
struct state{
	short	s_char;
	short	s_state;	/* Just for debugging */
	struct action	*s_success;	/* What to do if we have a match */
	struct state	*s_goto;	/* Linked list of accessible states... */
	struct state	*s_next;	/* ... and pointer to manage it */
}*state, *advance();

static char *alloc();
static char *dupstr();
#define	new(s)	(s *)alloc(sizeof (s))
#define	void	int	/* Because PCC mishandles it */
typedef	void (*proc)();

/* What to do on a successful match */
/* The action types: */
#define	SIGNAL	2	/* Signal on pin */
#define	CHIP	4	/* Chip */
#define	AUTO	8	/* Extra flag or'ed in with CHIP or SIGNAL */
#define	PIN	16	/* Pin (unused now) */

#endif
