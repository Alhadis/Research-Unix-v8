#ifdef	MPXSTATS

enum
{
	l0blocked, lblocked, nosuchchan, badckill, nmpxstats
};

#define	L0BLOCKED	(int)l0blocked
#define	LBLOCKED	(int)lblocked
#define	NOSUCHCHAN	(int)nosuchchan
#define	BADCKILL	(int)badckill
#define	NMPXSTATS	(int)nmpxstats

struct
{
	char *	descp;
	int	count;
}
	mpxstats[NMPXSTATS] =
{
	 {"channel 0 blocked"}
	,{"channel blocked"}
	,{"bad channel"}
	,{"bad channel kill"}
};

#undef	MPXSTATS
#define	MPXSTATS(A)	mpxstats[A].count++
#else
#define	MPXSTATS(A)
#endif
