#define ESCAPE		033
#define STATELINE	007

#define PEEK_LONG	'L'
#define PEEK_WORD	'W'
#define PEEK_BYTE	'B'
#define PEEK_STR	'S'

#define	INDEX_BASE	0L
#define TEXT_INDEX	(1+INDEX_BASE)
#define PROC_INDEX	(2+INDEX_BASE)
#define FRAME_INDEX	(3+INDEX_BASE)
#define HATEBS_INDEX	(4+INDEX_BASE)
#define TYPE_INDEX	(5+INDEX_BASE)
#define TLOC_INDEX	(6+INDEX_BASE)
#define SCR_INDEX	(7+INDEX_BASE)
#define LAST_INDEX	(7+INDEX_BASE)

#define POKE_LONG	'l'
#define POKE_WORD	'w'
#define POKE_BYTE	'b'

#define DO_BPTS 	's'
#define DO_BPTC		'c'

#define DO_WRAP		004
#define DO_DEBUG	'N'
#define DO_VIDEO	'V'
#define DO_ACT		'R'
#define DO_HALT		'H'
#define DO_SING		'1'
#define DO_KBD		'K'
#define DO_ANTIC	'A'
#define DO_STACK	'p'
#define DO_FUNC		'f'
#define DO_DELTAS	'd'
#define DO_ENABLE	'E'
#define DO_DISABLE	'D'
#define DO_EVENT	'e'
#define DO_INVOKE	'I'
#define DO_LIMIT	'g'
#define DO_TRACK	't'

#define HI_ADDR 0777774
#define LO_ADDR 8

#define LO_ROM  262152
#define HI_ROM	(LO_ROM+24*1024)

#define NON_JSR		1
#define DIR_JSR		2
#define IND_JSR		3

#define M_MENU		'm'
#define M_ITEM		'i'
#define M_HIT		'h'
#define M_NULL		'z'
#define M_BUTT		'n'
#define M_CONFIRM	'C'
#define M_PENDULUM	'P'
#define M_LIMITS	'#'
#define	M_BITVEC	'|'
#define M_SUGGEST	'@'

#define G_BITMAP	':'
#define G_RECTANGLE	'['
#define G_TEXTURE	'_'
#define G_POINT		'.'


#define I_DATA		1
#define I_ADDR		2



