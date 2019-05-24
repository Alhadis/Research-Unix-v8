#define	U(a)	u.a
#define	UIO
#define	AUIO
#define	UDCL
#define	GETBLK	geteblk()
#define	IOMOVE(a,n,d)	iomove(a, n, d)
#define	BIOMOVE(a,n,d)	iomove(a, n, d)
#define	CPASS		cpass()
#define	PASSC(a)	passc(a)
#define	RDEV	i_rdev
#define	RVAL	r_val1
#define	GROUP	i_group
#define	FPEND	fileNFILE
#define	paddr(bp)	bp->b_un.b_addr
#define	signal	gsignal
#define	ttioccomm ttioctl
#define	FLUSHTTY(a)	flushtty(a,FREAD|FWRITE)
#define	LDEV	dev,
