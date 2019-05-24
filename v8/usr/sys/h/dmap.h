/*
 * Definitions for the mapping of vitual swap
 * space to the physical swap area - the disk map.
 */

#ifdef BIGMEM
#define	NDMAP 		64	/* size of the swap area map */
#else
#define	NDMAP 		32	/* size of the swap area map */
#endif
#define	DMMIN 		32	/* the initial block size in clicks */
#define	DMMAX		1024	/* max block size alloc on drum = .5M byte */
#define	DMTEXT		1024	/* size of blocks of pure text = .5M byte */

struct	dmap
{
	swblk_t	dm_size;	/* current size used by process */
	swblk_t	dm_alloc;	/* amount of physical swap space allocated */
	swblk_t	dm_map[NDMAP];	/* first disk block number in each chunk */
};
#ifdef KERNEL
struct	dmap zdmap;
#endif

/*
 * The following structure is that ``returned''
 * from a call to vstodb().
 */
struct	dblock
{
	swblk_t	db_base;	/* base of physical contig drum block */
	swblk_t	db_size;	/* size of block */
};
