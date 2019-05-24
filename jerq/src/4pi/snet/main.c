#include "snet.h"
#include "term.h"
#include "../protocol.h"

typedef struct Handler {
	int	pkt;
	PFI	handler;
} Handler;

Version(); Spawnee(); Reloc(); Control();
Read(); Write(); String(); LiftBpt(); LayBpt(); CoreP();
Alloc(); Free(); Call();

Handler Handlers[] = {
	{ (int)	CP_STACK,	CoreP	},
	{ (int)	CP_CONTROL,	CoreP	},
	{ (int)	CP_FLASH,	CoreP	},
	{ (int)	CP_REGLOC,	CoreP	},
	{ (int)	CP_RUN,	CoreP	},
	{ (int)	CP_STEP,	CoreP	},
	{ (int)	CP_STOP,	CoreP	},

	{ (int)	C_LAYBPT,	LayBpt	},
	{ (int)	C_LIFTBPT,	LiftBpt	},
	{ (int)	C_READ,	Read	},
	{ (int)	C_SPEE,	Spawnee },
	{ (int)	C_STRING,	String	},
	{ (int)	C_VERSION,	Version },
	{ (int)	C_WRITE,	Write	},

	{ (int)	CF_ALLOC,	Alloc	},
	{ (int)	CF_FREE,	Free	},
	{ (int)	CF_CALL,	Call	},
	{ (int)	0,		0	}
};

void RCVServe()
{
	register pkt;
	register Handler *h;
	register PFI f;

	for( ;; ){
		pkt = GetRemote();
		journal( "RCVServe",  "%x", (long)pkt );
		for( f = 0, h = Handlers; !f && h->pkt; ++h )
			if( h->pkt == pkt )
				f = h->handler;
		assert(f);
		(*f)(pkt);
	}
}

main()
{
	journal( "main", "running" );
	RCVServe();
}

long SpawneePid;
long RcvLong();
Spawnee()		/* new host process spawned ok  */
{
	SpawneePid = RcvLong();
	PutTextf("snet processor agent for pid=%d\n", SpawneePid);
}

long TermVersion = PI_VERSION;
long HostVersion;
Version()
{
	char comment[256];
	HostVersion = RcvLong();
	RcvString(comment);
	PutTextf( "%s\n", comment );
	if( HostVersion == TermVersion ) return;
	PutTextf( "Versions Host=%d Term=%d\n", HostVersion, TermVersion );
	assert(0);
}
