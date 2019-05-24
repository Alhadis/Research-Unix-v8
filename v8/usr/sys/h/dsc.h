/*
 * these values are critical;
 * dsseq depends on da being 0
 * and ad being 1
 */
# define DA		((int)0)
# define AD		((int)1)

/*
 * start address for d/a and a/d converters
 */
#define	ADBASE	((int)0)
#define	DABASE	((int)010)

/*
 * ASC sequence table bit for last entry
 */
#define LAST_SEQ    bit(7)

/*
 * Ioctl commands.
 */
# define DSSEQ		((('s') << 8) | 0)	/* set sequence */
# define DSRATE		((('s') << 8) | 1)	/* set rate */
# define DS08KHZ	((('s') << 8) | 2)	/* set 08kHz filter */
# define DS04KHZ	((('s') << 8) | 3)	/* set 04kHz filter */
# define DSBYPAS	((('s') << 8) | 5)	/* set bypass filter */
# define DSERRS		((('s') << 8) | 6)	/* get errors */
# define DSRESET	((('s') << 8) | 7)	/* reset dsc */
# define DSTRANS	((('s') << 8) | 8)	/* get transit. counts */
# define DSLAST		((('s') << 8) | 12)	/* last seq ram */
# define DSDONE		((('s') << 8) | 14)	/* amnt. done */
# define DSMON		((('s') << 8) | 15)	/* set monitor mode */
# define DSBRD		((('s') << 8) | 16)	/* set broadcast mode */
# define DSDEBUG	((('s') << 8) | 19)	/* debug */
# define DSWAIT		((('s') << 8) | 20)	/* wait for io to finish */
# define DSSTEREO	((('s') << 8) | 21)
# define DSMONO		((('s') << 8) | 22)
# define NDSB		3		/* number of buffers chaining with */

/*
 * reg specifies a sequence register (0-15).
 * conv specifies a converter.
 * dirt specifies the direction when
 * setting up the sequence ram (DSSEQ) or the
 * sampling rate (DSRATE).
 */
struct ds_seq {
	short reg;
	short conv;
	short dirt;			/* shared by DSSEQ and DSRATE */
};
/*
 * Format of returned converter
 * errors.
 */
struct ds_err {
	short dma_csr;
	short asc_csr;
	short errors;
};

/*
 * Format of returned transition counts
 */
struct ds_trans {
	short to_idle;
	short to_active;
};
