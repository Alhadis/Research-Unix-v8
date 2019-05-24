/*
 * Instrumentation
 */
#define	CPUSTATES	5

#define	CP_USER		0
#define	CP_NICE		1
#define	CP_SYS		2
#define	CP_IDLE		3
#define	CP_QUEUE	4

#define	DK_NDRIVE	8

#ifdef KERNEL
long	cp_time[CPUSTATES];
int	dk_busy;
long	dk_time[DK_NDRIVE];
long	dk_seek[DK_NDRIVE];
long	dk_xfer[DK_NDRIVE];
long	dk_wds[DK_NDRIVE];
float	dk_mspw[DK_NDRIVE];

long	tk_nin;
long	tk_nout;
#endif
