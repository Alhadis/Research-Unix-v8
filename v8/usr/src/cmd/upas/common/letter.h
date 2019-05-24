typedef struct letter {
	long	address;	/* offset into temporary file */
	long	offset;		/* offset into letter */
	long	size;		/* size of letter */
	int	status;		/* status of letter */
}letter;
#define NLETTERS 512

/* letter status flags */
#define L_USED 1		/* 1 if entry used, 0 otherwise */
#define L_DELETED 2		/* 1 if entry has been deleted, 0 otherwise */

/* declare letter routines */
extern void inittmp();
extern void releasetmp();
extern letter * lopen();
extern char *lgets();
extern void lputs();
extern void lputc();
extern long ltell();
extern long lsize();
extern int letseek();
extern int ldelete();
extern int copyto();
extern int copyfrom();
