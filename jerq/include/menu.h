typedef struct NMitem
{
	char	*text;
	char	*help;
	struct NMenu *next;
	void	(*fn)();
} NMitem;
typedef struct NMenu
{
	NMitem	*item;			/* string array, ending with 0 */
	NMitem	*(*generator)();	/* used if item == 0 */
	short	prevhit;		/* private to menuhit() */
	short	prevtop;		/* private to menuhit() */
} NMenu;
NMitem *mhit();
