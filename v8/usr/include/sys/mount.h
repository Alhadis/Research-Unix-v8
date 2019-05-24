/*
 * Mount structure.
 * One allocated on every mount.
 * Used to find the super block.
 */
struct	mount	/* free if !m_bufp && !m_fstyp */
{
	struct	inode *m_inodp;	/* pointer to mounted on inode */
	union {
		struct	buf *M_bufp;	/* pointer to superblock */
		struct	inode *M_idev;	/* communication inode */
	}	m_union;
	dev_t	m_dev;		/* device mounted */
	u_char	m_fstyp;	/* what kind of file system is mounted */
	u_char	m_flags;		/* see definitions below */
};
#define m_bufp m_union.M_bufp
#define m_idev m_union.M_idev

#ifdef KERNEL
struct	mount mount[NMOUNT];
extern struct mount *findmount();
extern struct mount *allocmount();
extern int freemount();
#endif

#define M_MOUNTED 1
