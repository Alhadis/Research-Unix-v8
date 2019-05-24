#define M_MENUS 8
#define M_ITEMS 64
#define M_SHIFT 32		/* can't have \b appear in a menu msg! */

#define M_SLOT ((char *) 1)

#define M_SLOTLEN 15

#define M_MAIN 1

#define M_MEMORY 2

#define M_SCRATCH 3

#define M_CALLRET 4

#define M_NUMERIC 5

#define M_BVSIZE 512

int hinumeric, lonumeric;
char bitvector[M_BVSIZE/8];

typedef struct NewMenu{
	char	**item;			/* string array, ending with 0 */
	char	*(*generator)();	/* used if item == 0 */
	int	prevhit;		/* retained from previous call */
	int	prevtop;		/* ditto */
} NewMenu;

char *limitsgen(), *bvgen();
