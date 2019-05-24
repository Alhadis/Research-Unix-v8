/*
 * Declaration of block device
 * switch. Each entry (row) is
 * the only link between the
 * main unix code and the driver.
 * The initialization of the
 * device switches is in the
 * file conf.c.
 */
struct bdevsw
{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_strategy)();
	int	(*d_dump)();
	int	d_flags;
};
#ifdef KERNEL
struct	bdevsw bdevsw[];
#endif

/*
 * Character device switch.
 */
extern struct cdevsw
{
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_read)();
	int	(*d_write)();
	int	(*d_ioctl)();
	int	(*d_reset)();
	struct	streamtab *qinfo;
};
#ifdef KERNEL
struct cdevsw cdevsw[];
#endif

/* file system types */
struct fstypsw {
	int		(*t_put)();
	struct inode	*(*t_get)();
	int		(*t_free)();
	int		(*t_updat)();
	int		(*t_read)();
	int		(*t_write)();
	int		(*t_trunc)();
	int		(*t_stat)();
	int		(*t_nami)();
	int		(*t_mount)();
	int		(*t_ioctl)();
};
#ifdef KERNEL
extern struct fstypsw fstypsw[];
extern nfstyp;
#endif

/*
 * stream processor table
 */
extern	struct streamtab {
	struct	qinit	*rdinit;
	struct	qinit	*wrinit;
} *streamtab[];

/*
 * Swap device information
 */
struct swdevt
{
	dev_t	sw_dev;
	int	sw_freed;
};
#ifdef KERNEL
struct	swdevt swdevt[];
#endif
