
/*
Data structures to help debug tcp.
*/

struct tcpdebug {
	time_t	stamp;		/* time stamp */
	short	inout;		/* flag to note if input (1) or output (0) mess */
	struct tcphdr savhdr;	/* header of the sent/received message */
	};

#define SIZDEBUG	64	/* number of headers to save in debug queue */
